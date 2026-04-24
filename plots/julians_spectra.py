import numpy as np
import matplotlib.pyplot as plt
import glob
import os
import re

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

data_folder = "build/Data"
files = sorted(glob.glob(os.path.join(data_folder, "loweroutput_G4_W_*.txt")))
print("Found files:", files)

bin_width_MeV = 0.002
max_energy = 10

fig, ax = plt.subplots(figsize=(8, 6))
colors = plt.cm.viridis(np.linspace(0, 1, len(files)))

max_found_energy = 0.01

for color, file_path in zip(colors, files):
    data = np.genfromtxt(file_path, delimiter=",", dtype=None, encoding=None, names=True)

    mask = data["Particle"] == "gamma"
    energies = data["KineticEnergy"][mask]

    if len(energies) == 0:
        continue

    max_found_energy = max(max_found_energy, np.max(energies))

    bins = np.arange(0, max_energy + bin_width_MeV, bin_width_MeV)
    counts, edges = np.histogram(energies, bins=bins)

    basename = os.path.basename(file_path)
    match = re.search(r"W_(\d*\.?\d+)mm", basename)
    label = f"{match.group(1)} mm" if match else basename.replace(".txt", "")

    valid = counts > 0
    ax.plot(edges[:-1][valid], counts[valid], lw=2, color=color, label=label)

ax.set_xscale("log")
ax.set_yscale("log")
ax.set_xlim(0.01, max_found_energy * 1.1)
ax.set_xlabel("Photon Energy [MeV]")
ax.set_ylabel("Number of Photons (counts per 2 keV bin)")
ax.set_title("Bremsstrahlung Spectrum vs Tungsten Foil Thickness")
ax.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)
ax.legend(title="Foil Thickness", loc="best", frameon=True)

plt.tight_layout()
plt.savefig("tungsten_thickness_spectrum.png", dpi=200, bbox_inches='tight')
plt.show()