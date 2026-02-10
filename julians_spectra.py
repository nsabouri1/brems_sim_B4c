import numpy as np
import matplotlib.pyplot as plt
import glob
import os
import re

# --- Plot Style ---
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

# --- Parameters ---
data_folder = "build/Data"  # directory where your Geant4 .txt files live
files = sorted(glob.glob(os.path.join(data_folder, "loweroutput_G4_W_*.txt")))
print("Found files:", files)

bin_width_MeV = 0.002  # 1 keV = 0.001 MeV
max_energy = 10       # MeV

# --- Figure Setup ---
fig, ax = plt.subplots(figsize=(8, 6))
colors = plt.cm.viridis(np.linspace(0, 1, len(files)))

# --- Loop through files ---
for color, file_path in zip(colors, files):
    # Read data
    
    data = np.genfromtxt(file_path, delimiter=",", dtype=None, encoding=None, names=True)
    mask = data["Particle"] == "gamma"
    energies = data["KineticEnergy"][mask]  # already in MeV

    # Histogram binning (10 keV bins)
    bins = np.arange(0, max_energy + bin_width_MeV, bin_width_MeV)
    counts, edges = np.histogram(energies, bins=bins)

    # Label from filename
    basename = os.path.basename(file_path)
    match = re.search(r"W_(\d*\.?\d+)mm", basename)
    label = f"{match.group(1)} mm" if match else basename.replace(".txt", "")

    # Plot
    ax.plot(edges[:-1], counts, lw=2, color=color, label=label)

# --- Log–Log scaling & Formatting ---
ax.set_xscale("log")
ax.set_yscale("log")
ax.set_xlim(0.01, max([np.max(np.genfromtxt(f, delimiter=",", names=True)["KineticEnergy"]) for f in files]) * 1.1)
ax.set_ylim(auto=True)
ax.set_xlabel("Photon Energy [MeV]")
ax.set_ylabel("Number of Photons (counts per 2 keV bin)")
ax.set_title("Bremsstrahlung Spectrum vs Tungsten Foil Thickness")
ax.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)
ax.legend(title="Foil Thickness", loc="best", frameon=True)
valid = counts > 0
ax.plot(edges[:-1][valid], counts[valid], lw=2, color=color, label=label)
plt.tight_layout()
plt.tight_layout()
plt.savefig("tungsten_thickness_spectrum.png", dpi=200, bbox_inches='tight')
plt.show()
