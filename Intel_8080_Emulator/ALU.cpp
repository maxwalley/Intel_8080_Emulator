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

uint8_t ALU::createStatusByte() const
{
    //Set to 01000000
    uint8_t statusByte = 0x40;
    
    statusByte |= uint8_t(getFlag(Flag::Carry));
    statusByte |= uint8_t(getFlag(Flag::Parity)) << 2;
    statusByte |= uint8_t(getFlag(Flag::AuxillaryCarry)) << 4;
    statusByte |= uint8_t(getFlag(Flag::Zero)) << 6;
    statusByte |= uint8_t(getFlag(Flag::Sign)) << 7;
    
    return statusByte;
}

void ALU::setFromStatusByte(uint8_t statusByte)
{
    setFlag(Flag::Carry, statusByte & 0x1);
    setFlag(Flag::Parity, statusByte & 0x4);
    setFlag(Flag::AuxillaryCarry, statusByte & 0x10);
    setFlag(Flag::Zero, statusByte & 0x40);
    setFlag(Flag::Sign, statusByte & 0x80);
    
    return;
}

void ALU::checkCarryCheck()
{
    //Check that carry check works
    uint8_t first = 0xFF;
    uint8_t second = 0xFF;
    
    operateAndSetFlags(first, second);
    
    assert(getFlag(Flag::Carry));
    
    setFlag(Flag::All, false);
}

ALU::Flag operator|(ALU::Flag first, ALU::Flag second)
{
    return static_cast<ALU::Flag>(static_cast<int>(first) | static_cast<int>(second));
}

bool operator&(ALU::Flag first, ALU::Flag second)
{
    return static_cast<int>(first) & static_cast<int>(second);
}
