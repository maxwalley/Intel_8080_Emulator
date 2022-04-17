//
//  ALU.cpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 10/04/2022.
//

#include "ALU.hpp"

ALU::ALU()
{
    checkCarryCheck();
}

void ALU::setFlag(Flag f, bool val)
{
    for(int index = 0; index < flags.size(); ++index)
    {
        if(f & Flag(1 << index))
        {
            flags[index] = val;
        }
    }
}

bool ALU::getFlag(Flag f) const
{
    for(int index = 0; index < flags.size(); ++index)
    {
        if(f & Flag(1 << index))
        {
            return flags[index];
        }
    }
    
    return false;
}

void ALU::checkCarryCheck()
{
    //Check that carry check works
    uint8_t first = 0xFF;
    uint8_t second = 0xFF;
    
    operateAndSetFlags(first, second);
    
    assert(getFlag(Flag::Carry));
    
    setFlag(Flag::Carry, false);
}

ALU::Flag operator|(ALU::Flag first, ALU::Flag second)
{
    return static_cast<ALU::Flag>(static_cast<int>(first) | static_cast<int>(second));
}

bool operator&(ALU::Flag first, ALU::Flag second)
{
    return static_cast<int>(first) & static_cast<int>(second);
}
