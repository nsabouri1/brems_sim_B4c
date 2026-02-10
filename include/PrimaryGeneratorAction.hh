/// \file B4/B4c/include/PrimaryGeneratorAction.hh
/// \brief Definition of the B4::PrimaryGeneratorAction class

#ifndef B4PrimaryGeneratorAction_h
#define B4PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

#include <vector>
#include <string>

class G4GeneralParticleSource;
class G4Event;

namespace B4
{

/// PrimaryGeneratorAction: responsible for generating the initial particles for each event.
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction();
  ~PrimaryGeneratorAction() override;

  // Called at the beginning of each event to generate the primary vertex
  void GeneratePrimaries(G4Event* event) override;

private:
  // Loads the energy spectrum (energy–probability pairs) from a text file
  void LoadSpectrum(const std::string& filename);

  // Samples a random energy according to the loaded probability distribution
  double SampleEnergy() const;

private:
  G4GeneralParticleSource* fParticleGun;   // Geant4’s flexible particle source
  std::vector<double> fEnergies;           // Energies (in MeV)
  std::vector<double> fProbabilities;      // Corresponding probabilities
};

} // namespace B4
#endif
