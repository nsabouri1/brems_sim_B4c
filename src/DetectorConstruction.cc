#include "DetectorConstruction.hh"
#include "CalorimeterSD.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SDManager.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"
#include <ctime>
#include <fstream>

namespace B4c {

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    G4bool checkOverlaps = true;
    G4NistManager* nist = G4NistManager::Instance();
// Read geometry parameters from file
    std::string materialName = "G4_W";        // default
    G4double foilThickness = 0.1 * mm;       // default

    std::ifstream infile("geometry.txt");
    if (infile.is_open()) {
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            std::string key;
            iss >> key;
            if (key == "material") {
                std::string val;
                iss >> val;
                materialName = "G4_" + val;
            } else if (key == "thickness") {
                double val;
                iss >> val;
                foilThickness = val * mm;
            }
        }
        infile.close();
    } else {
        G4cerr << "Warning: geometry.txt not found. Using default material and thickness." << G4endl;
    }
    G4cout << "[DetectorConstruction] Using foil material: " << materialName 
       << ", thickness: " << foilThickness/mm << " mm" << G4endl;

    G4Material* foilMat = nist->FindOrBuildMaterial(materialName);
    if (!foilMat) {
        G4cerr << "Material " << materialName << " not found. Using default G4_W." << G4endl;
        foilMat = nist->FindOrBuildMaterial("G4_W");
    }

    // World
    G4double worldSize = 1.0*m;
    G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic");
    auto solidWorld = new G4Box("World", worldSize/2, worldSize/2, worldSize/2);
    auto logicWorld = new G4LogicalVolume(solidWorld, vacuum, "World");
    auto physWorld = new G4PVPlacement(nullptr, G4ThreeVector(), logicWorld, "World", nullptr, false, 0, checkOverlaps);

    // Target / Foil
    G4double sizeXY = 1.0*cm;
  
    auto solidTarget = new G4Box("Target", sizeXY/2, sizeXY/2, foilThickness/2);
    logicTarget = new G4LogicalVolume(solidTarget, foilMat, "Target");

    auto targetVis = new G4VisAttributes(G4Colour(0.8, 0.5, 0.2));
    targetVis->SetVisibility(true);
    logicTarget->SetVisAttributes(targetVis);

    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), logicTarget, "Target",
                      logicWorld, false, 0, checkOverlaps);

    fTargetBackZ = foilThickness/2;
    fBremsVolume = logicTarget;


// Log material and thickness info
std::ostringstream filename;
filename << "output_"
         << foilMat->GetName()
         << "_" << foilThickness/mm << "mm.txt";

std::ofstream log("runlog.txt", std::ios::app);
if (log.is_open()) {
    log << "Material: " << foilMat->GetName()
        << ", Thickness: " << foilThickness/mm << " mm\n"
        << "Output file: " << filename.str() << "\n\n";
    log.close();
}

    // Detector behind target
    G4double detectorXY = 2.0*cm;
    G4double detectorThicknessZ = 3.0*mm; 
    auto solidDetector = new G4Box("Detector", detectorXY, detectorXY, detectorThicknessZ/2);
    logicDetector = new G4LogicalVolume(solidDetector, vacuum, "Detector");

    auto detectorVis = new G4VisAttributes(G4Colour(0.,0.,1.,0.3)); // blue, semi-transparent
    detectorVis->SetForceSolid(true);
    logicDetector->SetVisAttributes(detectorVis);

    // Place detector behind target with small gap
    G4double gap = 0.1*mm;
    new G4PVPlacement(nullptr, G4ThreeVector(0,0,fTargetBackZ + detectorThicknessZ/2 + gap),
                      logicDetector, "Detector", logicWorld, false, 1, checkOverlaps);

    // Make world invisible
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    return physWorld;
}

void DetectorConstruction::ConstructSDandField()
{
    auto calorSD = new CalorimeterSD("DetectorSD", "CalorHitsCollection", 2);
    G4SDManager::GetSDMpointer()->AddNewDetector(calorSD);

    // Assign the same SD to both volumes
    logicTarget->SetSensitiveDetector(calorSD);
    logicDetector->SetSensitiveDetector(calorSD);

    G4cout << "[DetectorConstruction] CalorimeterSD assigned to Target and Detector." << G4endl;
}
}
// namespace B4c
