import numpy as np
import matplotlib.pyplot as plt
import glob
import os

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

# --- PARAMETERS ---
data_folder = "data"  # folder containing your text files
output_folder = "histograms"
bin_width = 2  # keV
max_energy = None
#target_energy_min = 80  # keV
#target_energy_max = 120  # keV

os.makedirs(output_folder, exist_ok=True)

files = glob.glob(os.path.join(data_folder, "*.txt"))

# Dictionary to store results
yields = {}
spectrum_scores = {}

for file_path in files:
    # Read the file and extract relevant columns
    data = np.genfromtxt(file_path, delimiter=",", dtype=None, encoding=None, names=True)
    # names=True automatically uses the header row
    print("Column names:", data.dtype.names)
    # Optional: Filter only gamma particles (in case there are other types)
    mask = data["Particle"] == "gamma"
    energies_mev s= data["KineticEnergy"][mask]

    # Convert to keV
    energies = energies_mev * 1000

    # Total yield
    total_yield = len(energies)
    yields[os.path.basename(file_path)] = total_yield

    # Spectrum score (80–120 keV)
   # target_count = np.sum((energies >= target_energy_min) & (energies <= target_energy_max))
   # spectrum_score = target_count / total_yield if total_yield > 0 else 0
    spectrum_scores[os.path.basename(file_path)] = spectrum_score

    # Histogram
if max_energy:
    bins = np.arange(0, max_energy + bin_width, bin_width)
else:
    bins = np.arange(0, np.max(energies) + bin_width, bin_width)
    counts, edges = np.histogram(energies, bins=bins)

    # Plot
    fig, ax = plt.subplots(figsize=(8,6))
    ax.bar(edges[:-1], counts, width=bin_width, align='edge', color='#0072B2', edgecolor='black', alpha=0.7)

    ax.set_xlabel("Energy (keV)")
    ax.set_ylabel("Counts")
    ax.set_title(f"X-ray Spectrum: {os.path.basename(file_path)}")
    ax.grid(True, which='both', linestyle='--', linewidth=0.5)
    ax.set_ylim(bottom=0)
    ax.set_xlim(left=0)

    plt.tight_layout()
    plt.savefig(os.path.join(output_folder, f"{os.path.basename(file_path).replace('.txt','.png')}"))
    plt.close()

# --- Ranking ---
sorted_scores = sorted(spectrum_scores.items(), key=lambda x: x[1], reverse=True)
#print("Top 5 foil+thickness combinations by ~100 keV spectrum fraction:")
#for i, (filename, score) in enumerate(sorted_scores[:5], 1):
   # print(f"{i}: {filename} -> {score:.3f} fraction in 80-120 keV")

sorted_yields = sorted(yields.items(), key=lambda x: x[1], reverse=True)
print("\nTop 5 foil+thickness combinations by total X-ray yield:")
for i, (filename, y) in enumerate(sorted_yields[:5], 1):
    print(f"{i}: {filename} -> {y} photons")

print("Histograms saved and top combinations ranked.")
