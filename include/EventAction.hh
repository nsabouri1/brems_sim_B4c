//
/// \file B4/B4c/include/EventAction.hh
/// \brief Definition of the B4c::EventAction class

#ifndef B4cEventAction_h
#define B4cEventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class G4Event;

namespace B4c
{

/// Event action class
///
/// For now, this just prints event IDs at the end of each event.
/// Later you can extend it to record scoring or analysis results.
class EventAction : public G4UserEventAction
{
  public:
    EventAction();
    ~EventAction() override;

    void BeginOfEventAction(const G4Event* event) override;
    void EndOfEventAction(const G4Event* event) override;

  private:
    // Placeholder: add scoring IDs or counters here in the future
};

}  // namespace B4c

#endif
