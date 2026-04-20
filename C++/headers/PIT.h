#pragma once
#include "TypeDefs.h"

namespace PIT {
    extern volatile uint_64 TimeSinceBootMS;
    extern volatile uint_64 TicksSinceBoot;
    
    void Sleep(uint_64 milliseconds);
    
    void SetDivisor(uint_16 divisor);
    uint_16 GetDivisor();
    void SetFrequency(uint_64 frequency);
    void Tick();
}
