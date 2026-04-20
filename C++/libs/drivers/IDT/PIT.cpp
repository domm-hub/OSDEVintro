#include "PIT.h"
#include "IO.h"
#include "BasicRenderer.h"

namespace PIT {
    volatile uint_64 TimeSinceBootMS = 0;
    volatile uint_64 TicksSinceBoot = 0;
    
    uint_16 Divisor = 65535;

    void Sleep(uint_64 milliseconds){
        uint_64 startTime = TimeSinceBootMS;
        while (TimeSinceBootMS < startTime + milliseconds){
            asm("hlt");
        }
    }

    void io_wait() {
        outb(0x80, 0);
    }

    void SetDivisor(uint_16 divisor){
        if (divisor < 100) divisor = 100;
        Divisor = divisor;
        outb(0x43, 0x36); // Command port
        io_wait();
        outb(0x40, (uint_8)(divisor & 0x00ff));
        io_wait();
        outb(0x40, (uint_8)((divisor & 0xff00) >> 8));
        io_wait();
    }

    uint_16 GetDivisor(){
        return Divisor;
    }

    void SetFrequency(uint_64 frequency){
        if (frequency == 0) return;
        SetDivisor(1193182 / frequency);
    }

    void Tick(){
        TicksSinceBoot++;
        TimeSinceBootMS += 1000 / (1193182 / Divisor);
        
        uint_64 freq = 1193182 / Divisor;
        uint_64 blinkTicks = freq / 2;
        if (blinkTicks == 0) blinkTicks = 1;

        if (TicksSinceBoot % blinkTicks == 0) { 
            if (GlobalRenderer) {
                GlobalRenderer->ToggleCursor();
            }
        }
        
    }
}
