//

//
/// \file B4/B4c/include/CalorimeterSD.hh
/// \brief Definition of the B4c::CalorimeterSD class

#ifndef B4cCalorimeterSD_h
#define B4cCalorimeterSD_h 1

#include "CalorHit.hh"

#include "G4VSensitiveDetector.hh"
#include "globals.hh"
#include <set>
#include <fstream>

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

namespace B4c
{

/// Calorimeter sensitive detector class
///
/// In Initialize(), it creates one hit for each calorimeter layer and one more
/// hit for accounting the total quantities in all layers.
///
/// The values are accounted in hits in ProcessHits() function which is called
/// by Geant4 kernel at each step.

class CalorimeterSD : public G4VSensitiveDetector
{
  public:
    CalorimeterSD(const G4String& name, const G4String& hitsCollectionName, G4int nofCells);
    ~CalorimeterSD() override;

    // methods from base class
    void Initialize(G4HCofThisEvent* hitCollection) override;
    G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hitCollection) override;
    std::ofstream outputFile; // Output file stream
  private:
    CalorHitsCollection* fHitsCollection = nullptr;
    G4int fNofCells = 0;

        //std::set<G4int> fLoggedTracks; 

};

}  // namespace B4c

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
