import numpy as np
import matplotlib.pyplot as plt
import glob
import os
import re
import csv
from collections import defaultdict, OrderedDict

plt.style.use("seaborn-v0_8-paper")
plt.rcParams.update({
    "font.family": "serif",
    "font.size": 12,
    "axes.labelsize": 14,
    "xtick.labelsize": 12,
    "ytick.labelsize": 12,
    "legend.fontsize": 12,
    "figure.titlesize": 16,
    "figure.dpi": 300,
})

# =========================================================
# USER SETTINGS
# =========================================================
data_folder = "build/Data"
file_glob = "loweroutput_G4_*_*.txt"

# Put your thicknesses here exactly as they appear in filenames
target_thicknesses = ["0.1", "0.25", "0.5", "1.0"]

# Plot mode:
# "full"  -> MeV log-log spectrum like your nice tungsten plot
# "low"   -> low-energy zoom in keV, log-y only
plot_mode = "full"

# Detector filtering
# False = all gamma rows
# True  = only gamma rows in Detector volume
require_volume_detector = False
detector_volume_name = "Detector"

# Binning / range
bin_width_MeV = 0.002   # 2 keV bins
max_energy_MeV = 10.0
min_energy_MeV_plot = 0.01

# Low-energy view settings (used if plot_mode == "low")
xmax_keV_low = 100.0

# Output settings
save_binned_csv = True
output_plot_dir = "plots"
output_binned_dir = "binned_data"

# =========================================================
# HELPERS
# =========================================================
fname_re = re.compile(r"_G4_([A-Za-z0-9]+)_([0-9]*\.?[0-9]+)mm\.txt$")

def parse_material_thickness(path):
    basename = os.path.basename(path)
    m = fname_re.search(basename)
    if not m:
        return None, None
    material = m.group(1)
    thickness = m.group(2)
    if thickness == "1":
        thickness = "1.0"
    return material, thickness

def load_structured_csv(path):
    arr = np.genfromtxt(path, delimiter=",", names=True, dtype=None, encoding=None, invalid_raise=False)
    if getattr(arr, "size", 0) == 0:
        return np.array([], dtype=arr.dtype if hasattr(arr, "dtype") else [])
    if getattr(arr, "shape", None) == ():
        arr = np.array([arr])
    return arr

def get_energy_mev(arr):
    names = arr.dtype.names or ()
    if "KineticEnergy" in names:
        return arr["KineticEnergy"].astype(float)
    if "KineticEnergy(MeV)" in names:
        return arr["KineticEnergy(MeV)"].astype(float)
    if "KineticEnergy_MeV" in names:
        return arr["KineticEnergy_MeV"].astype(float)
    raise KeyError("Missing energy column")

def get_gamma_mask(arr):
    names = arr.dtype.names or ()
    if "Particle" not in names:
        raise KeyError("Missing 'Particle' column")
    mask = (arr["Particle"] == "gamma")
    if require_volume_detector:
        if "Volume" not in names:
            raise KeyError("Missing 'Volume' column")
        mask &= (arr["Volume"] == detector_volume_name)
    return mask

def safe_material_name(material):
    return re.sub(r"[^A-Za-z0-9_-]", "_", material)

# =========================================================
# FIND FILES
# =========================================================
files = sorted(glob.glob(os.path.join(data_folder, file_glob)))
print("Found files:")
for f in files:
    print("  ", f)

if not files:
    raise FileNotFoundError(f"No files found matching {os.path.join(data_folder, file_glob)}")

os.makedirs(output_plot_dir, exist_ok=True)
os.makedirs(output_binned_dir, exist_ok=True)

# =========================================================
# COLLECT RAW ENERGIES BY MATERIAL & THICKNESS
# data_by_material[material][thickness] = 1D array of energies in MeV
# =========================================================
data_by_material = defaultdict(lambda: defaultdict(list))

for fp in files:
    material, thickness = parse_material_thickness(fp)
    if not material or not thickness:
        print(f"Skipping (filename parse failed): {fp}")
        continue

    if target_thicknesses and thickness not in target_thicknesses:
        print(f"Skipping (thickness not in target list): {fp}")
        continue

    try:
        arr = load_structured_csv(fp)
    except Exception as e:
        print(f"Could not read {fp}: {e}")
        continue

    if arr.size == 0:
        print(f"Skipping empty file: {fp}")
        continue

    try:
        energies_mev = get_energy_mev(arr)
        gamma_mask = get_gamma_mask(arr)
    except KeyError as e:
        print(f"Skipping {fp}: {e}")
        continue

    if not np.any(gamma_mask):
        print(f"No matching gamma entries in {fp}")
        continue

    energies = energies_mev[gamma_mask]
    energies = energies[np.isfinite(energies)]
    energies = energies[(energies >= 0.0) & (energies <= max_energy_MeV)]

    if energies.size == 0:
        print(f"No usable energies in {fp}")
        continue

    data_by_material[material][thickness].append(energies)
    print(f"Loaded {fp}: material={material}, thickness={thickness} mm, count={energies.size}")

# Concatenate per material/thickness
for material in list(data_by_material.keys()):
    for thickness in list(data_by_material[material].keys()):
        chunks = data_by_material[material][thickness]
        if len(chunks) == 0:
            del data_by_material[material][thickness]
        else:
            data_by_material[material][thickness] = np.concatenate(chunks)

# =========================================================
# BINNING SETUP
# =========================================================
bins = np.arange(0.0, max_energy_MeV + bin_width_MeV, bin_width_MeV)
bin_centers_mev = (bins[:-1] + bins[1:]) / 2.0

# =========================================================
# PROCESS EACH MATERIAL
# =========================================================
for material, thk_dict in data_by_material.items():
    if not thk_dict:
        print(f"No usable datasets for material {material}")
        continue

    ordered = OrderedDict(sorted(thk_dict.items(), key=lambda kv: float(kv[0])))

    # Bin counts for each thickness
    counts_by_thickness = {}
    for thk, energies_mev in ordered.items():
        counts, _ = np.histogram(energies_mev, bins=bins)
        counts_by_thickness[thk] = counts

    # Save binned CSV
    if save_binned_csv:
        csv_path = os.path.join(output_binned_dir, f"binned_{safe_material_name(material)}.csv")
        with open(csv_path, "w", newline="") as f:
            writer = csv.writer(f)
            header = ["Energy_MeV"] + [f"{material}_{thk}mm" for thk in ordered.keys()]
            writer.writerow(header)

            for i, e in enumerate(bin_centers_mev):
                row = [f"{e:.6f}"]
                for thk in ordered.keys():
                    row.append(int(counts_by_thickness[thk][i]))
                writer.writerow(row)

        print(f"Saved binned CSV: {csv_path}")

    # Plot
    fig, ax = plt.subplots(figsize=(8, 6))
    cmap = plt.cm.viridis(np.linspace(0, 1, len(ordered)))

    if plot_mode == "full":
        max_found_energy = min_energy_MeV_plot

        for color, (thk, counts) in zip(cmap, counts_by_thickness.items()):
            valid = (counts > 0) & (bin_centers_mev >= min_energy_MeV_plot)
            if not np.any(valid):
                continue
            max_found_energy = max(max_found_energy, np.max(bin_centers_mev[valid]))
            ax.plot(bin_centers_mev[valid], counts[valid], lw=2, color=color, label=f"{thk} mm")

        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.set_xlim(min_energy_MeV_plot, min(max_energy_MeV, max_found_energy * 1.05))
        ax.set_xlabel("Photon Energy [MeV]")
        ax.set_ylabel("Photon Counts per 2 keV Bin")
        ax.set_title(f"Bremsstrahlung Spectrum vs {material} Foil Thickness")

        suffix = "full"

    elif plot_mode == "low":
        xmax_mev_low = xmax_keV_low / 1000.0

        for color, (thk, counts) in zip(cmap, counts_by_thickness.items()):
            mask = (bin_centers_mev <= xmax_mev_low)
            x_keV = bin_centers_mev[mask] * 1000.0
            y = counts[mask]
            ax.step(x_keV, y, where="mid", lw=2, color=color, label=f"{thk} mm")

        ax.set_xlim(0, xmax_keV_low)
        ax.set_yscale("log")
        ax.set_ylim(bottom=0.9)
        ax.set_xlabel("Photon Energy [keV]")
        ax.set_ylabel("Photon Counts per 2 keV Bin")
        ax.set_title(f"Low-Energy Bremsstrahlung vs Thickness — {material}")

        suffix = "low"

    else:
        raise ValueError("plot_mode must be 'full' or 'low'")

    ax.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)
    ax.legend(title="Foil Thickness", loc="best", frameon=True)
    plt.tight_layout()

    plot_path = os.path.join(output_plot_dir, f"spectrum_{safe_material_name(material)}_{suffix}.png")
    plt.savefig(plot_path, dpi=300, bbox_inches="tight")
    print(f"Saved plot: {plot_path}")

plt.show()


