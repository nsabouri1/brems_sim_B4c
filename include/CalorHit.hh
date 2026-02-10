
/// \file B4/B4c/include/CalorHit.hh
/// \brief Definition of the B4c::CalorHit class

#ifndef B4cCalorHit_h
#define B4cCalorHit_h 1

#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "G4Threading.hh"
#include "G4ThreeVector.hh"
#include "G4VHit.hh"
#include "globals.hh"

namespace B4c
{

/// Calorimeter hit class
///
/// It defines data members to store the the energy deposit and track lengths
/// of charged particles in a selected volume:
/// - fEdep, fTrackLength

class CalorHit : public G4VHit
{
  public:
    CalorHit() = default;
    CalorHit(const CalorHit&) = default;
    ~CalorHit() override = default;

    // operators
    CalorHit& operator=(const CalorHit&) = default;
    G4bool operator==(const CalorHit&) const;

    inline void* operator new(size_t);
    inline void operator delete(void*);

    // methods from base class
    void Draw() override {}
    void Print() override;

    // methods to handle data
    void Add(G4double de, G4double dl);

    // get methods
    G4double GetEdep() const;
    G4double GetTrackLength() const;

  private:
    G4double fEdep = 0.;  ///< Energy deposit in the sensitive volume
    G4double fTrackLength = 0.;  ///< Track length in the  sensitive volume
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

using CalorHitsCollection = G4THitsCollection<CalorHit>;

extern G4ThreadLocal G4Allocator<CalorHit>* CalorHitAllocator;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

inline void* CalorHit::operator new(size_t)
{
  if (!CalorHitAllocator) {
    CalorHitAllocator = new G4Allocator<CalorHit>;
  }
  void* hit;
  hit = (void*)CalorHitAllocator->MallocSingle();
  return hit;
}

inline void CalorHit::operator delete(void* hit)
{
  if (!CalorHitAllocator) {
    CalorHitAllocator = new G4Allocator<CalorHit>;
  }
  CalorHitAllocator->FreeSingle((CalorHit*)hit);
}

inline void CalorHit::Add(G4double de, G4double dl)
{
  fEdep += de;
  fTrackLength += dl;
}

inline G4double CalorHit::GetEdep() const
{
  return fEdep;
}

inline G4double CalorHit::GetTrackLength() const
{
  return fTrackLength;
}

}  // namespace B4c

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
