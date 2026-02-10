
/// \file B4/B4c/include/DetectorConstruction.hh
/// \brief Definition of the B4c::DetectorConstruction class

#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH 1

#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"

namespace B4c {


class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction() = default;
    ~DetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;
    G4LogicalVolume* GetBremsVolume() const { return fBremsVolume; }

  private:
    // methods
    //
    G4LogicalVolume* logicDetector = nullptr;  // Logical volume for the detector   
    G4LogicalVolume* logicTarget = nullptr;  // $ Logical volume for the target
    G4LogicalVolume* fBremsVolume = nullptr;   // Logical volume for the bremsstrahlung target
    G4double fTargetBackZ = 0;                 // Position of the back face of the target

};
} // namespace B4c
#endif