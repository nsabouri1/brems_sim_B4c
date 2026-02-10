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

#include <fstream>
#include <sstream>
#include <random>
#include <iostream>

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

  // Load GALADRIEL spectrumc from macro: energy (MeV), probability
  LoadSpectrum("spectrum_new.mac");

  G4cout << "[PrimaryGeneratorAction] Loaded " << fEnergies.size()
         << " energy points from spectrum_new.mac" << G4endl;

  if (fEnergies.empty() || fProbabilities.empty()) {
    G4cerr << "Error: Spectrum data not loaded correctly." << G4endl;
  }
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
  if (fEnergies.empty()) return 1.0 * MeV;

  // Normalize probabilities
  double total = 0.0;
  for (auto p : fProbabilities) total += p;

  std::vector<double> norm_probs;
  norm_probs.reserve(fProbabilities.size());
  for (auto p : fProbabilities) norm_probs.push_back(p / total);

  // Random sampling according to probability distribution
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::discrete_distribution<> dist(norm_probs.begin(), norm_probs.end());
  int index = dist(gen);

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