#include "pit.h"
#include "../IO.h"

namespace PIT
{
    double TimeSinceBoot = 0;

    uint16_t Divisor = 65535;

    double freq = (double)GetFrequency();

    void InitPIT()
    {
        TimeSinceBoot = 0;
        SetDivisor(65535);
        freq = (double)GetFrequency();
    }

    void SetDivisor(uint16_t divisor)
    {
        if (divisor < 100)
            divisor = 100;

        Divisor = divisor;
        outb(0x40, (uint8_t)(divisor & 0x00ff));
        io_wait();
        outb(0x40, (uint8_t)((divisor & 0xff00) >> 8));
        io_wait();
        freq = (double)GetFrequency();
    }

    void Sleepd(double seconds)
    {
        double startTime = TimeSinceBoot;
        while (TimeSinceBoot < startTime + seconds)
            ;//asm("hlt");
    }

    void Sleep(uint64_t milliseconds)
    {
        Sleepd(milliseconds / 1000.0);
    }

    uint64_t GetFrequency()
    {
        return BaseFrequency / Divisor;
    }

    void SetFrequency(uint64_t frequency)
    {
        SetDivisor(BaseFrequency / frequency);
        freq = (double)GetFrequency();
    }

    void Tick()
    {
        TimeSinceBoot += 1/freq;
    }
}