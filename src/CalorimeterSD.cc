/// \file B4/B4c/src/CalorimeterSD.cc
/// \brief Implementation of the B4c::CalorimeterSD class

#include "CalorimeterSD.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include <fstream>
#include <iomanip>

namespace B4c
{

CalorimeterSD::CalorimeterSD(const G4String& name,const G4String& hitsCollectionName, G4int nofCells)
  : G4VSensitiveDetector(name), fNofCells(nofCells)
{
  collectionName.insert(hitsCollectionName);
outputFile.open("outputdata.txt",std::ios::out | std::ios::trunc);
 if (outputFile.is_open()) {
   // Write header only if file is empty
   outputFile.seekp(0, std::ios::end);
   if (outputFile.tellp() == 0) {
       outputFile << "EventID,TrackID,ParentID,Particle,KineticEnergy,Volume,DetectorID\n";
       G4cout << "[CalorimeterSD] Opened 'outputdata.txt' for logging." << G4endl;
       G4cout.flush();
     }
 } else {
   G4cerr << "[CalorimeterSD] Warning: Could not open 'outputdata.txt'." << G4endl;
 }
}

CalorimeterSD::~CalorimeterSD()
{
  if (outputFile.is_open())
    outputFile.close();
}

void CalorimeterSD::Initialize(G4HCofThisEvent* hce)
{
  fHitsCollection = new CalorHitsCollection(SensitiveDetectorName, collectionName[0]);

  auto hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(hcID, fHitsCollection);

  for (G4int i = 0; i < fNofCells + 1; ++i) 
    fHitsCollection->insert(new CalorHit());
  }

G4bool CalorimeterSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    // Particle info
    auto particle = step->GetTrack()->GetDefinition()->GetParticleName();
    auto kineticEnergy = step->GetTrack()->GetKineticEnergy();
    auto currentVol = step->GetPreStepPoint()->GetPhysicalVolume();
    if (!currentVol) return false;  // ignore steps outside world

    auto volumeName = currentVol->GetName();
    auto trackID = step->GetTrack()->GetTrackID();
    auto parentID = step->GetTrack()->GetParentID();
    G4double energy_keV = kineticEnergy / CLHEP::keV;

    // ======================
    // DEBUG: print first few gammas inside detector
    static G4int gammaCounter = 0;
    const G4int maxGammaDebug = 10;  // only first 10 gammas
    if (particle == "gamma" && volumeName == "Detector" && gammaCounter < maxGammaDebug) {
        G4cout << "[DEBUG-GAMMA] Event "
               << G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID()
               << " particle=" << particle
               << " E(MeV)=" << kineticEnergy / CLHEP::MeV
               << " volume=" << volumeName
               << G4endl;
        gammaCounter++;
    }
    // ======================

    // Log gammas inside detector in the 10–100 keV window
    if (volumeName == "Detector" && particle == "gamma")
    {
        int detectorID = 1;

        if (outputFile.is_open()) {
            outputFile << G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID() << ","
                       << trackID << ","
                       << parentID << ","
                       << particle << ","
                       << kineticEnergy / CLHEP::MeV << ","
                       << volumeName << ","
                       << detectorID << "\n";
            outputFile.flush();  // ensures live updates
        }
    }

    return true;
}





void CalorimeterSD::EndOfEvent(G4HCofThisEvent*)
{
  if (verboseLevel > 1) {
    auto nofHits = fHitsCollection->entries();
    G4cout << G4endl << "-------->Hits Collection: in this event they are "
           << nofHits << " hits in the calorimeter: " << G4endl;
    for (std::size_t i = 0; i < nofHits; ++i)
      (*fHitsCollection)[i]->Print();
  }

  if (outputFile.is_open()) outputFile.flush();
}
} // namespace B4c
