
import numpy as np
import matplotlib.pyplot as plt
import glob
import os
import re
from collections import defaultdict, OrderedDict

# --- Plot Style (paper-like) ---
plt.style.use('seaborn-v0_8-paper')
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

# --- Parameters you can tweak ---
data_folder = "build/Data"                 # directory with your Geant4 text files
file_glob   = "loweroutput_G4_*_*.txt"     # filename pattern to match your outputs
target_thicknesses = ["0.1", "0.25", "0.5", "1.0"]
bin_width_keV = 2.0
xmax_keV = 100.0
require_volume_detector = True             # set False to ignore the Volume filter
use_logy = True                            # y-axis log-scale for dynamic range

# --- Discover files ---
files = sorted(glob.glob(os.path.join(data_folder, file_glob)))
print("Found files:", files)

# --- Parse material and thickness from filename: ..._G4_<Material>_<thickness>mm.txt ---
fname_re = re.compile(r"_G4_([A-Za-z0-9]+)_([0-9]*\.?[0-9]+)mm\.txt$")

def parse_material_thickness(path):
    m = fname_re.search(os.path.basename(path))
    if not m:
        return None, None
    material = m.group(1)               # e.g., Ta, W, Cu
    thickness = m.group(2)              # e.g., 0.1, 0.25, 0.5, 1 or 1.0
    if thickness == "1":                # normalize
        thickness = "1.0"
    return material, thickness

# --- Helpers for robust CSV reading ---
def load_structured_csv(path):
    """Load CSV with header; return structured numpy array.
    Handles single-row files (0-d arrays) and preserves strings."""
    arr = np.genfromtxt(path, delimiter=",", names=True, dtype=None, encoding=None)
    if arr.size == 0:
        return np.array([], dtype=arr.dtype)
    if arr.shape == ():  # single row -> make it 1D
        arr = np.array([arr])
    return arr

def get_energy_mev(arr):
    """Return energy in MeV from either 'KineticEnergy' or 'KineticEnergy(MeV)'."""
    names = arr.dtype.names or ()
    if "KineticEnergy" in names:
        return arr["KineticEnergy"].astype(float)
    if "KineticEnergy(MeV)" in names:
        return arr["KineticEnergy(MeV)"].astype(float)
    raise KeyError("Missing energy column: expected 'KineticEnergy' or 'KineticEnergy(MeV)'")

# --- Collect energies grouped by material & thickness ---
# data_by_material[material][thickness] = 1D array of energies (keV)
data_by_material = defaultdict(lambda: defaultdict(list))

for fp in files:
    material, thickness = parse_material_thickness(fp)
    if not material or not thickness:
        print(f"Skipping (name parse failed): {fp}")
        continue
    if thickness not in target_thicknesses:
        # skip non-target thicknesses
        continue

    try:
        arr = load_structured_csv(fp)
    except Exception as e:
        print(f"Could not read {fp}: {e}")
        continue

    if arr.size == 0:
        print(f"Empty or unreadable: {fp}")
        continue

    # Column checks
    names = arr.dtype.names or ()
    if "Particle" not in names:
        print(f"Missing 'Particle' column in {fp}. Skipping.")
        continue
    if require_volume_detector and "Volume" not in names:
        print(f"Missing 'Volume' column in {fp} while require_volume_detector=True. Skipping.")
        continue

    # Filter gamma (and optionally Detector volume)
    particle_mask = (arr["Particle"] == "gamma")
    if require_volume_detector:
        particle_mask &= (arr["Volume"] == "Detector")

    if not np.any(particle_mask):
        print(f"No matching gamma entries in {fp}.")
        continue

    try:
        energies_mev = get_energy_mev(arr)
    except KeyError as e:
        print(f"{e} in {fp}. Skipping.")
        continue

    energies_keV = energies_mev[particle_mask] * 1000.0
    # Keep 0–xmax_keV
    energies_keV = energies_keV[(energies_keV >= 0.0) & (energies_keV <= xmax_keV)]
    if energies_keV.size == 0:
        print(f"All gamma energies in {fp} are outside 0–{xmax_keV} keV.")
        continue

    data_by_material[material][thickness].append(energies_keV)

# Concatenate lists per (material, thickness)
for mat in list(data_by_material.keys()):
    for thk in list(data_by_material[mat].keys()):
        if len(data_by_material[mat][thk]) == 0:
            data_by_material[mat].pop(thk)
        else:
            data_by_material[mat][thk] = np.concatenate(data_by_material[mat][thk])

# --- Plot per material ---
bins = np.arange(0.0, xmax_keV + bin_width_keV, bin_width_keV)
bin_centers = (bins[:-1] + bins[1:]) / 2.0

for material, thk_dict in data_by_material.items():
    if not thk_dict:
        print(f"No data to plot for material {material}.")
        continue

    # Order curves by numeric thickness
    ordered = OrderedDict(
        sorted(thk_dict.items(), key=lambda kv: float(kv[0]))
    )

    fig, ax = plt.subplots(figsize=(8, 6))
    cmap = plt.cm.viridis(np.linspace(0, 1, len(ordered)))

    for (color, (thk, energies_keV)) in zip(cmap, ordered.items()):
        if energies_keV.size == 0:
            continue
        counts, _ = np.histogram(energies_keV, bins=bins)
        label = f"{thk} mm"
        ax.step(bin_centers, counts, where="mid", lw=2, label=label, color=color)

    ax.set_xlim(0, xmax_keV)
    ax.set_xlabel("Photon Energy [keV]")
    ax.set_ylabel(f"Number of Photons (counts per {int(bin_width_keV)} keV bin)")
    ax.set_title(f"Bremsstrahlung Counts vs Thickness — {material}")

    if use_logy:
        # avoid log(0) by setting a small floor where needed
        ymin = 0.9
        ax.set_yscale("log")
        ax.set_ylim(bottom=max(ymin, 1e-1))

    ax.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)
    ax.legend(title="Foil Thickness", loc="best", frameon=True)
    plt.tight_layout()

    out_name = f"spectrum_counts_{material}.png"
    plt.savefig(out_name, dpi=300, bbox_inches="tight")
    print(f"Saved: {out_name}")

plt.show()