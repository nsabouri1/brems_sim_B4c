import os
import re
import glob
import numpy as np
import pandas as pd

# -------- Settings --------
DATA_FOLDER = "Data"              # folder containing loweroutput_G4_W_*.txt
PATTERN = "loweroutput_G4_W_*.txt"      # adjust if needed
BIN_WIDTH_MEV = 0.002                   # 2 keV = 0.002 MeV
E_MIN = 0.0
E_MAX = 10.0                            # MeV

OUT_DIR = "binned_csv"
# -------------------------

def thickness_from_filename(path: str) -> float | None:
    """Extract thickness from filenames like loweroutput_G4_W_0.25mm.txt"""
    m = re.search(r"W_(\d*\.?\d+)mm", os.path.basename(path))
    return float(m.group(1)) if m else None

def load_gamma_energies(file_path: str) -> np.ndarray:
    """Load gamma KineticEnergy column from CSV-like txt with header."""
    df = pd.read_csv(file_path)  # your file is comma-delimited with header
    # Filter only gammas
    df = df[df["Particle"] == "gamma"]
    # Ensure numeric energies
    energies = pd.to_numeric(df["KineticEnergy"], errors="coerce").dropna().to_numpy()
    return energies

def bin_counts(energies: np.ndarray, edges: np.ndarray) -> np.ndarray:
    counts, _ = np.histogram(energies, bins=edges)
    return counts

def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    files = glob.glob(os.path.join(DATA_FOLDER, PATTERN))
    if not files:
        raise FileNotFoundError(f"No files found at {DATA_FOLDER}/{PATTERN}")

    # Sort by numeric thickness if possible
    files_sorted = sorted(files, key=lambda f: (thickness_from_filename(f) is None, thickness_from_filename(f) or 1e9))

    # Shared bin edges for all files
    edges = np.arange(E_MIN, E_MAX + BIN_WIDTH_MEV, BIN_WIDTH_MEV)
    centers = 0.5 * (edges[:-1] + edges[1:])

    combined = pd.DataFrame({"Energy_MeV": centers})

    print("Binning settings:")
    print(f"  Bin width: {BIN_WIDTH_MEV} MeV ({BIN_WIDTH_MEV*1000:.1f} keV)")
    print(f"  Range: {E_MIN} to {E_MAX} MeV")
    print("\nProcessing files:")

    for fp in files_sorted:
        th = thickness_from_filename(fp)
        th_label = f"{th}mm" if th is not None else os.path.splitext(os.path.basename(fp))[0]

        energies = load_gamma_energies(fp)
        counts = bin_counts(energies, edges)

        # Save individual binned CSV (small!)
        out_path = os.path.join(OUT_DIR, f"W_{th_label}_binned_{int(BIN_WIDTH_MEV*1000)}keV.csv")
        out_df = pd.DataFrame({"Energy_MeV": centers, "Counts": counts})
        out_df.to_csv(out_path, index=False)

        # Add to combined wide table
        col_name = f"W_{th_label}"
        combined[col_name] = counts

        # Quick check for W K-lines (optional console info)
        ka = np.sum((energies >= 0.058) & (energies < 0.060))
        kb = np.sum((energies >= 0.066) & (energies < 0.068))

        print(f"  {os.path.basename(fp)} -> {out_path}")
        print(f"    gammas: {len(energies)} | Kα(0.058-0.060): {ka} | Kβ(0.066-0.068): {kb}")

    # Save combined CSV for easy Excel plotting (one sheet, many curves)
    combined_path = os.path.join(OUT_DIR, f"W_all_thicknesses_binned_{int(BIN_WIDTH_MEV*1000)}keV.csv")
    combined.to_csv(combined_path, index=False)

    print("\nDone.")
    print(f"Combined CSV for Excel: {combined_path}")
    print(f"Individual CSVs saved in: {OUT_DIR}/")

if __name__ == "__main__":
    main()
