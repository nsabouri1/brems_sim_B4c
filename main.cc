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
#include "Randomize.hh"
#include <ctime>

int main(int argc, char** argv)
{
    G4SteppingVerbose::UseBestUnit(4);
    CLHEP::HepRandom::setTheSeed(std::time(nullptr));

    // GUI mode (no args) → Serial to avoid MT/vis crash on beamOn
    // Batch mode (-m macro) → MT for full speed
    bool batchMode = (argc == 3 && std::string(argv[1]) == "-m");

    G4RunManager* runManager = nullptr;
    if (batchMode) {
        runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::MTOnly);
        static_cast<G4MTRunManager*>(runManager)->SetNumberOfThreads(0); // all cores
        G4cout << "[main] Batch mode: multithreaded" << G4endl;
    } else {
        runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);
        G4cout << "[main] GUI mode: serial (MT disabled to prevent vis crash)" << G4endl;
    }

    auto detConstruction = new B4c::DetectorConstruction();
    runManager->SetUserInitialization(detConstruction);

    G4PhysListFactory factory;
    auto physicsList = factory.GetReferencePhysList("FTFP_BERT_LIV");
    runManager->SetUserInitialization(physicsList);

    auto actionInitialization = new B4c::ActionInitialization();
    runManager->SetUserInitialization(actionInitialization);

    auto visManager = new G4VisExecutive;
    visManager->Initialize();

    auto UImanager = G4UImanager::GetUIpointer();

    if (!batchMode) {
        // GUI / interactive mode — serial
        auto ui = new G4UIExecutive(argc, argv);
        UImanager->ApplyCommand("/control/execute macros/run1.mac");
        UImanager->ApplyCommand("/control/execute macros/init_vis.mac");
        UImanager->ApplyCommand("/control/execute macros/vis.mac");
        if (ui->IsGUI())
            UImanager->ApplyCommand("/control/execute macros/gui.mac");
        ui->SessionStart();
        delete ui;
    } else {
        // Batch mode — MT, no vis
        UImanager->ApplyCommand("/control/execute " + G4String(argv[2]));
    }

    delete visManager;
    delete runManager;
    return 0;
}