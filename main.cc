/// \file exampleB4c.cc
/// \brief Main program of the B4c example

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "G4PhysListFactory.hh"

#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

int main(int argc, char** argv)
{
    // Verbose units for stepping
    G4SteppingVerbose::UseBestUnit(4);

    // Construct the run manager
    auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);

    // Set mandatory initialization classes
    auto detConstruction = new B4c::DetectorConstruction();
    runManager->SetUserInitialization(detConstruction);

    G4PhysListFactory factory;
    auto physicsList = factory.GetReferencePhysList("FTFP_BERT_LIV");
    runManager->SetUserInitialization(physicsList);

    auto actionInitialization = new B4c::ActionInitialization();
    runManager->SetUserInitialization(actionInitialization);

    // Initialize visualization manager
    auto visManager = new G4VisExecutive;
    visManager->Initialize();

    // Get the UI manager
    auto UImanager = G4UImanager::GetUIpointer();

    // Start GUI if no macro is provided
    G4UIExecutive* ui = nullptr;
    if (argc == 1) {
        ui = new G4UIExecutive(argc, argv);

        // Execute your run macro first
        UImanager->ApplyCommand("/control/execute run1.mac");

        // Initialize visualization
        UImanager->ApplyCommand("/control/execute macros/init_vis.mac");
        UImanager->ApplyCommand("/control/execute macros/vis.mac");

        // Optional GUI macros
        if (ui->IsGUI()) {
            UImanager->ApplyCommand("/control/execute macros/gui.mac");
        }

        // Start interactive session
        ui->SessionStart();
        delete ui;
    }
    else if (argc == 3 && std::string(argv[1]) == "-m") {
        // Batch mode
        G4String command = "/control/execute ";
        UImanager->ApplyCommand(command + G4String(argv[2]));
    }
    else {
        G4cerr << "Usage: " << G4endl;
        G4cerr << "./brems_sim_b4c          (interactive GUI)" << G4endl;
        G4cerr << "./brems_sim_b4c -m macro (batch mode)" << G4endl;
        return 1;
    }

    // Clean up
    delete visManager;
    delete runManager;
}
