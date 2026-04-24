/// \file B4/B4c/include/CalorimeterSD.hh
/// \brief Definition of the B4c::CalorimeterSD class


#ifndef B4cCalorimeterSD_h
#define B4cCalorimeterSD_h 1


#include "CalorHit.hh"


#include "G4VSensitiveDetector.hh"
#include "globals.hh"


#include <fstream>
#include <set>


class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;


namespace B4c
{


class CalorimeterSD : public G4VSensitiveDetector
{
  public:
    CalorimeterSD(const G4String& name, const G4String& hitsCollectionName, G4int nofCells);
    ~CalorimeterSD() override;


    void Initialize(G4HCofThisEvent* hitCollection) override;
    G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hitCollection) override;


  private:
    CalorHitsCollection* fHitsCollection = nullptr;
    G4int fNofCells = 0;


    std::ofstream outputFile;
    std::set<G4int> fLoggedTracks;
};


}  // namespace B4c


#endif
