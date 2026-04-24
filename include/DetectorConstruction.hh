/// \file B4/B4c/include/DetectorConstruction.hh

#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH 1

#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"
#include "globals.hh"

namespace B4c {

class DetectorConstruction : public G4VUserDetectorConstruction
{
 public:
 DetectorConstruction() = default;
 ~DetectorConstruction() override = default;

 G4VPhysicalVolume* Construct() override;
 void ConstructSDandField() override;

 G4LogicalVolume* GetBremsVolume() const { return fBremsVolume; }

 G4String GetOutputFileName() const { return fOutputFileName; }
 G4double GetThicknessMM() const { return fThicknessMM; }
 G4String GetMaterialName() const { return fMaterialName; }

 private:
 G4LogicalVolume* logicDetector = nullptr;
 G4LogicalVolume* logicTarget = nullptr;
 G4LogicalVolume* fBremsVolume = nullptr;
 G4double fTargetBackZ = 0;

 G4String fOutputFileName = "";
 G4double fThicknessMM = 0.0;
 G4String fMaterialName = "";
};

} // namespace B4c

#endif