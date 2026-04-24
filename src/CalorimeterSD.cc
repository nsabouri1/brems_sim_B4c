/// \file B4/B4c/src/CalorimeterSD.cc
/// \brief Implementation of the B4c::CalorimeterSD class

#include "CalorimeterSD.hh"
#include "DetectorConstruction.hh"

#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Threading.hh"

namespace B4c
{

CalorimeterSD::CalorimeterSD(const G4String& name,
                             const G4String& hitsCollectionName,
                             G4int nofCells)
  : G4VSensitiveDetector(name), fNofCells(nofCells)
{
  collectionName.insert(hitsCollectionName);

  auto detConst = static_cast<const B4c::DetectorConstruction*>(
      G4RunManager::GetRunManager()->GetUserDetectorConstruction());

  G4String baseFilename = detConst->GetOutputFileName();

  // In multithreaded mode each worker thread gets its own file to avoid
  // race conditions. Files are named e.g. loweroutput_G4_W_1mm_t0.txt
  // The plotting script merges them automatically.
  G4String filename = baseFilename;
  if (G4Threading::IsWorkerThread()) {
    // Insert thread ID before .txt
    G4String threadSuffix = "_t" + std::to_string(G4Threading::G4GetThreadId());
    auto dotPos = baseFilename.rfind('.');
    if (dotPos != G4String::npos)
      filename = baseFilename.substr(0, dotPos) + threadSuffix + baseFilename.substr(dotPos);
    else
      filename = baseFilename + threadSuffix;
  }

  outputFile.open(filename, std::ios::out | std::ios::trunc);
  if (outputFile.is_open()) {
    outputFile << "EventID,TrackID,ParentID,Particle,KineticEnergy,Volume,DetectorID\n";
    G4cout << "[CalorimeterSD] Thread " << G4Threading::G4GetThreadId()
           << " writing to " << filename << G4endl;
  } else {
    G4cerr << "[CalorimeterSD] Warning: Could not open " << filename << G4endl;
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

  fLoggedTracks.clear();
}

G4bool CalorimeterSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  auto* track = step->GetTrack();
  if (!track) return false;

  // Only photons
  if (track->GetDefinition()->GetParticleName() != "gamma") return true;

  // Only record each track once (first entry into the detector plane)
  G4int trackID = track->GetTrackID();
  if (fLoggedTracks.count(trackID) > 0) return true;
  fLoggedTracks.insert(trackID);

  auto* event = G4RunManager::GetRunManager()->GetCurrentEvent();
  if (!event) return true;

  auto eventID       = event->GetEventID();
  auto parentID      = track->GetParentID();
  auto kineticEnergy = track->GetKineticEnergy();

  if (outputFile.is_open()) {
    outputFile << eventID       << ","
               << trackID       << ","
               << parentID      << ","
               << "gamma"       << ","
               << kineticEnergy / CLHEP::MeV << ","
               << "Detector"    << ","
               << 0             << "\n";
  }

  return true;
}

void CalorimeterSD::EndOfEvent(G4HCofThisEvent*)
{
  // NOTE: flush removed — OS buffers writes automatically and flushes
  // on close, which is far faster than flushing every single event.
  // The file is safely closed in the destructor when the run ends.

  if (verboseLevel > 1) {
    auto nofHits = fHitsCollection->entries();
    G4cout << G4endl << "-------->Hits Collection: in this event they are "
           << nofHits << " hits in the calorimeter: " << G4endl;
    for (std::size_t i = 0; i < nofHits; ++i)
      (*fHitsCollection)[i]->Print();
  }
}

} // namespace B4c
