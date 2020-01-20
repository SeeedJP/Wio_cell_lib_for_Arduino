#pragma once

#include "../NectisCellularConfig.h"

class WioSK6812 {
    private:
    void SetBit(bool on);
    void SetByte(uint8_t val);
    
    public:
    void Reset();
    void SetSingleLED(uint8_t r, uint8_t g, uint8_t b);
    
};
