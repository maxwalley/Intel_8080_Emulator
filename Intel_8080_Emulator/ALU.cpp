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
    flags[static_cast<int>(f)] = val;
}

bool ALU::getFlag(Flag f) const
{
    return flags[static_cast<int>(f)];
}

template<unsigned_integral IntType>
void ALU::setFlagsBasedOnValue(IntType val)
{
    //Zero flag
    setFlag(Flag::Zero, val == 0);
    
    //Sign flag
    const int numBits = std::numeric_limits<IntType>::digits;
    uint8_t test = val >> numBits - 1;
    setFlag(Flag::Sign, test);
    
    //Parity flag
    setFlag(Flag::Parity, checkParity(val));
}

template<unsigned_integral IntType>
IntType ALU::operateAndSetCarryFlag(IntType first, IntType second, bool add)
{
    if(add)
    {
        //This seems to check for carry in apple clang
        setFlag(Flag::Carry, first + second > std::numeric_limits<IntType>::max());
        return first + second;
    }
    
    setFlag(Flag::Carry, second > first);
    return first - second;
}

template<unsigned_integral IntType>
bool ALU::checkParity(IntType val) const
{
    //This might be too slow to construct the bitset
    const int numBits = std::numeric_limits<IntType>::digits;
    std::bitset<numBits> bits(val);
    return bits.count() % 2 == 0;
}

void ALU::checkCarryCheck()
{
    //Check that carry check works
    uint8_t first = 0xFF;
    uint8_t second = 0xFF;
    
    operateAndSetCarryFlag(first, second, true);
    
    assert(getFlag(Flag::Carry));
    
    setFlag(Flag::Carry, false);
}
