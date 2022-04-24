//
//  SpaceInvaders.hpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 24/04/2022.
//

#pragma once

#include "Machine.hpp"

class SpaceInvaders  : public Machine
{
public:
    SpaceInvaders();
    ~SpaceInvaders() override;
    
private:
    uint8_t inputOperation(uint8_t port) override;
    void outputOperation(uint8_t port, uint8_t value) override;
    
    uint8_t currentShiftOffset = 0x0;
    uint16_t currentShiftVal = 0x0;
};
