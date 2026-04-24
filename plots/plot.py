"""
plot_bremsstrahlung.py
Matches the style of the original good plot exactly.
Auto-merges per-thread files produced by multithreaded Geant4 runs.
"""

import numpy as np
import matplotlib.pyplot as plt
import glob, os, re
from collections import defaultdict

# ── SETTINGS ──────────────────────────────────────────────────────────────────
DATA_FOLDER   = "build/data"
MATERIAL      = "W"                  # change to Ta, Cu etc. for other materials
BIN_WIDTH_MEV = 0.002                # 2 keV bins — matches original
MIN_ENERGY    = 0.00                # MeV — start just below W K-edge
MAX_ENERGY    = 1.05                 # MeV — 1 MeV beam endpoint
OUTPUT_PNG    = f"brems_{MATERIAL}_spectrum.png"
MIN_ROWS      = 5000                 # skip tiny/broken files
# ──────────────────────────────────────────────────────────────────────────────

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

# Matches both:  loweroutput_G4_W_1mm.txt
#                loweroutput_G4_W_1mm_t0.txt  (threaded)
#                loweroutput_G4_W_0_1mm.txt
fname_re = re.compile(
    r"loweroutput_G4_" + MATERIAL + r"_(\d+[_\.]?\d*)mm(?:_t\d+)?\.txt$"
)

def parse_thickness(path):
    m = fname_re.search(os.path.basename(path))
    if not m: return None
    try: return float(m.group(1).replace('_', '.'))
    except: return None

def thickness_label(t):
    s = f"{t:.4f}".rstrip('0').rstrip('.')
    return f"{s} mm"

def load_gammas(path):
    n = sum(1 for _ in open(path)) - 1
    if n < MIN_ROWS:
        print(f"  skip {os.path.basename(path)} ({n} rows)")
        return None
    arr = np.genfromtxt(path, delimiter=',', names=True, dtype=None, encoding=None, invalid_raise=False)
    if arr.size == 0: return None
    if arr.shape == (): arr = np.array([arr])
    e = arr['KineticEnergy'][arr['Particle'] == 'gamma'].astype(float)
    e = e[(e > 0) & (e <= MAX_ENERGY)]
    print(f"  {os.path.basename(path)}: {len(e):,} gammas, "
          f"E=[{e.min()*1000:.1f}, {e.max()*1000:.0f}] keV")
    return e

# ── Discover and group ────────────────────────────────────────────────────────
pattern = os.path.join(DATA_FOLDER, f"loweroutput_G4_{MATERIAL}_*.txt")
all_files = sorted(glob.glob(pattern))

if not all_files:
    raise FileNotFoundError(f"No files found: {pattern}")
print(f"Found {len(all_files)} file(s):\n")

groups = defaultdict(list)
for fp in all_files:
    t = parse_thickness(fp)
    if t is not None:
        groups[t].append(fp)
    else:
        print(f"  (skipping unparseable: {os.path.basename(fp)})")

# ── Load and merge ────────────────────────────────────────────────────────────
thickness_energies = {}
for t in sorted(groups.keys()):
    print(f"Loading {thickness_label(t)}:")
    chunks = [e for fp in sorted(groups[t])
              if (e := load_gammas(fp)) is not None and len(e) > 0]
    if chunks:
        merged = np.concatenate(chunks)
        thickness_energies[t] = merged
        print(f"  → total: {len(merged):,} gammas\n")

if not thickness_energies:
    raise RuntimeError("No usable data loaded.")

# ── Plot ──────────────────────────────────────────────────────────────────────
bins     = np.arange(0.0, MAX_ENERGY + BIN_WIDTH_MEV, BIN_WIDTH_MEV)
bin_left = bins[:-1]

fig, ax = plt.subplots(figsize=(8, 6))

# viridis from dark purple → yellow, same as original good plot
colors = plt.cm.viridis(np.linspace(0, 1, len(thickness_energies)))

for color, t in zip(colors, sorted(thickness_energies.keys())):
    e = thickness_energies[t]
    counts, _ = np.histogram(e, bins=bins)
    valid = counts > 0
    ax.plot(bin_left[valid], counts[valid],
            lw=2, color=color, label=thickness_label(t))

ax.set_xscale("log")
ax.set_yscale("log")
ax.set_xlim(MIN_ENERGY, MAX_ENERGY)
ax.set_xlabel("Photon Energy [MeV]")
ax.set_ylabel(f"Number of Photons (counts per {int(BIN_WIDTH_MEV*1000)} keV bin)")
ax.set_title(f"Bremsstrahlung Spectrum vs {MATERIAL} Foil Thickness")
ax.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)
ax.legend(title="Foil Thickness", loc="upper right", frameon=True)

plt.tight_layout()
plt.savefig(OUTPUT_PNG, dpi=300, bbox_inches="tight")
print(f"\nSaved: {OUTPUT_PNG}")
plt.show()
