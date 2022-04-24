//
//  SpaceInvaders.cpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 24/04/2022.
//

#include "SpaceInvaders.hpp"

SpaceInvaders::SpaceInvaders()
{
    
}

SpaceInvaders::~SpaceInvaders()
{
    
}

uint8_t SpaceInvaders::inputOperation(uint8_t port)
{
    switch (port)
    {
        //Shift the data and read
        case 0x3:
        {
            uint8_t realShiftAmount = 0x8 - currentShiftOffset;
            return currentShiftVal >> realShiftAmount;
        }
            
        default:
            return 0x0;
    }
}

void SpaceInvaders::outputOperation(uint8_t port, uint8_t value)
{
    switch (port)
    {
        //Set Shift Amount (3 Bits)
        case 0x2:
        {
            currentShiftOffset = value & 0x7;
            return;
        }
            
        //Set New Shift Data
        case 0x4:
        {
            currentShiftVal = (currentShiftVal >> 8) | (value << 8);
            return;
        }
            
        default:
            return;
    }
}
