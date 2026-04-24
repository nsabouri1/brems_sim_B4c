/// \file B4/B4c/src/PrimaryGeneratorAction.cc
/// \brief Implementation of the B4::PrimaryGeneratorAction class

#include "PrimaryGeneratorAction.hh"

#include "G4Box.hh"
#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4RandomTools.hh"
#include "G4ThreeVector.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4GeneralParticleSource.hh"
#include "G4AnalysisManager.hh"

#include "Randomize.hh"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <numeric>

namespace B4
{

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  // Create GPS, General Particle Source, for position and direction control
  fParticleGun = new G4GeneralParticleSource;

  // Particle type: electron
  const G4String particleName = "e-";
  auto* particle = G4ParticleTable::GetParticleTable()->FindParticle(particleName);
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->GetCurrentSource()->GetAngDist()->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));

  // Load spectrum — path is relative to the build directory where you run from.
  // spectrum_new.mac lives in macros/ alongside your other macro files.
  LoadSpectrum("macros/spectrum_new.mac");

  G4cout << "[PrimaryGeneratorAction] Loaded " << fEnergies.size()
         << " energy points from macros/spectrum_new.mac" << G4endl;

  if (fEnergies.empty() || fProbabilities.empty()) {
    G4cerr << "Error: Spectrum data not loaded correctly from macros/spectrum_new.mac" << G4endl;
    G4cerr << "Make sure you are running from the build/ directory and macros/spectrum_new.mac exists." << G4endl;
  }

  // Build cumulative distribution function for fast thread-safe sampling
  double total = 0.0;
  for (auto p : fProbabilities) total += p;
  fCDF.reserve(fProbabilities.size());
  double cumulative = 0.0;
  for (auto p : fProbabilities) {
    cumulative += p / total;
    fCDF.push_back(cumulative);
  }
  fCDF.back() = 1.0; // ensure last bin catches rounding
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
}

void PrimaryGeneratorAction::LoadSpectrum(const std::string& filename)
{
  std::ifstream infile(filename);
  if (!infile.is_open()) {
    G4cerr << "Error: Could not open spectrum file: " << filename << G4endl;
    return;
  }

  std::string line;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    std::string cmd;
    double E, P;
    if (iss >> cmd >> E >> P) {
      fEnergies.push_back(E * MeV);
      fProbabilities.push_back(P);
    }
  }

  if (fEnergies.empty())
    G4cerr << "WARNING: No energy points found in spectrum file." << G4endl;
}

double PrimaryGeneratorAction::SampleEnergy() const
{
  // Protect against empty spectrum
  if (fEnergies.empty() || fCDF.empty()) return 1.0 * MeV;

  // Use Geant4's thread-safe RNG (G4UniformRand) — safe in MT mode.
  // std::mt19937 with static storage is NOT thread-safe and corrupts
  // the random state across threads, causing all samples to collapse
  // to low energies.

  // Build cumulative distribution once (fCDF is built in constructor)
  double r = G4UniformRand();

  // Binary search for the bin
  auto it = std::lower_bound(fCDF.begin(), fCDF.end(), r);
  int index = static_cast<int>(std::distance(fCDF.begin(), it));
  if (index >= static_cast<int>(fEnergies.size()))
    index = static_cast<int>(fEnergies.size()) - 1;

  return fEnergies[index];
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  // Sample an energy from the loaded spectrum
  G4double sampledEnergy = SampleEnergy();
  fParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(sampledEnergy);

  // Generate the event
  fParticleGun->GeneratePrimaryVertex(event);

  // Optional: fill analysis histogram
  auto* analysisManager = G4AnalysisManager::Instance();
  if (analysisManager)
    analysisManager->FillH1(0, sampledEnergy / MeV);
}

} // namespace B4