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


#include <fstream>
#include <sstream>


namespace B4c {


G4VPhysicalVolume* DetectorConstruction::Construct()
{
   G4bool checkOverlaps = true;
   G4NistManager* nist = G4NistManager::Instance();


   std::string materialName = "G4_W";
   G4double foilThickness = 0.1 * mm;


   std::ifstream infile("geometry.txt");
   if (infile.is_open()) {
       std::string line;
       while (std::getline(infile, line)) {
           std::istringstream iss(line);
           std::string key;
           iss >> key;


           if (key == "material") {
               std::string val;
               if (iss >> val) {
                   materialName = "G4_" + val;
               } else {
                   G4cerr << "[DetectorConstruction] Could not read material from line: "
                          << line << G4endl;
               }
           }
           else if (key == "thickness") {
               double val = -1.0;
               if (iss >> val) {
                   foilThickness = val * mm;
               } else {
                   G4cerr << "[DetectorConstruction] Could not read thickness from line: "
                          << line << G4endl;
               }
           }
       }
       infile.close();
   }
   else {
       G4cerr << "Warning: geometry.txt not found. Using default material and thickness." << G4endl;
   }


   if (foilThickness <= 0.) {
       G4cerr << "[DetectorConstruction] Invalid foil thickness. Resetting to 0.1 mm." << G4endl;
       foilThickness = 0.1 * mm;
   }


   G4cout << "[DetectorConstruction] Material: " << materialName
          << ", thickness: " << foilThickness / mm << " mm" << G4endl;


   G4Material* foilMat = nist->FindOrBuildMaterial(materialName);
   if (!foilMat) {
       G4cerr << "Material " << materialName << " not found. Using default G4_W." << G4endl;
       foilMat = nist->FindOrBuildMaterial("G4_W");
       materialName = "G4_W";
   }


   fMaterialName = materialName;
   fThicknessMM = foilThickness / mm;


   std::ostringstream filename;
   filename << "data/loweroutput_"
            << fMaterialName
            << "_"
            << fThicknessMM
            << "mm.txt";


   fOutputFileName = filename.str();


   G4cout << "[DetectorConstruction] Output file: "
          << fOutputFileName << G4endl;


   // World
   auto worldMat = nist->FindOrBuildMaterial("G4_Galactic");
   auto solidWorld = new G4Box("World", 0.5 * m, 0.5 * m, 0.5 * m);
   auto logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");
   auto physWorld = new G4PVPlacement(
       nullptr, G4ThreeVector(), logicWorld, "World", nullptr, false, 0, checkOverlaps);


   // Target
   auto solidTarget = new G4Box("Target", 0.5 * cm, 0.5 * cm, foilThickness / 2.0);
   logicTarget = new G4LogicalVolume(solidTarget, foilMat, "Target");


   auto targetVis = new G4VisAttributes(G4Colour(0.8, 0.5, 0.2));
   targetVis->SetVisibility(true);
   logicTarget->SetVisAttributes(targetVis);


   new G4PVPlacement(
       nullptr, G4ThreeVector(), logicTarget, "Target", logicWorld, false, 0, checkOverlaps);


   fTargetBackZ = foilThickness / 2.0;
   fBremsVolume = logicTarget;


   // Thin detector plane behind foil (can stay for visualization, but not sensitive)
   G4double detectorXY = 2.0 * cm;
   G4double detectorThicknessZ = 1.0 * um;


   auto solidDetector = new G4Box(
       "Detector",
       detectorXY / 2.0,
       detectorXY / 2.0,
       detectorThicknessZ / 2.0);


   logicDetector = new G4LogicalVolume(solidDetector, worldMat, "Detector");


   auto detectorVis = new G4VisAttributes(G4Colour(0., 0., 1., 0.3));
   detectorVis->SetForceSolid(true);
   logicDetector->SetVisAttributes(detectorVis);


   new G4PVPlacement(
       nullptr,
       G4ThreeVector(0, 0, fTargetBackZ + detectorThicknessZ / 2.0),
       logicDetector,
       "Detector",
       logicWorld,
       false,
       1,
       checkOverlaps);


   logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());


   return physWorld;
}


void DetectorConstruction::ConstructSDandField()
{
   auto calorSD = new CalorimeterSD("DetectorSD", "CalorHitsCollection", 1);
   G4SDManager::GetSDMpointer()->AddNewDetector(calorSD);

   // Make the thin detector plane behind the foil sensitive
   logicDetector->SetSensitiveDetector(calorSD);
}


} // namespace B4c