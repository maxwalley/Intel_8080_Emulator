//
//  ALU.hpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 10/04/2022.
//

#pragma once

#include <bitset>
#include <concepts>

//Reimplementation of the c++20 template since xcode doesn't have it
template<typename T>
concept unsigned_integral = std::is_unsigned_v<T>;

class ALU
{
public:
    ALU();
    
    enum class Flag
    {
        None = 0,
        Zero = 1 << 0,
        Sign = 1 << 1,
        Parity = 1 << 2,
        Carry = 1 << 3,
        AuxillaryCarry = 1 << 4
    };
    
    void setFlag(Flag f, bool val);
    bool getFlag(Flag f) const;
    
    template<unsigned_integral IntType>
    IntType operateAndSetFlags(IntType first, IntType second, bool add = true, Flag flagsToExclude = static_cast<Flag>(0), bool useCarry = false);
    
private:
    //Returns true if even parity
    template<unsigned_integral IntType>
    bool checkParity(IntType val) const
    {
        //This might be too slow to construct the bitset
        const int numBits = std::numeric_limits<IntType>::digits;
        std::bitset<numBits> bits(val);
        return bits.count() % 2 == 0;
    }

    //Called on init. Just checks that the method used to check for carrys works
    void checkCarryCheck();
    
    std::bitset<5> flags;
};

ALU::Flag operator|(ALU::Flag first, ALU::Flag second);
bool operator&(ALU::Flag first, ALU::Flag second);

template<unsigned_integral IntType>
IntType ALU::operateAndSetFlags(IntType first, IntType second, bool add, Flag flagsToExclude, bool useCarry)
{
    IntType result;
    
    if(add)
    {
        if(!(flagsToExclude & Flag::Carry))
        {
            //This seems to check for carry in apple clang
            setFlag(Flag::Carry, first + second + (useCarry ? getFlag(Flag::Carry) : 0) > std::numeric_limits<IntType>::max());
        }
    
        result = first + second + (useCarry ? getFlag(Flag::Carry) : 0);
    }
    else
    {
        //Carry turns into borrow flag
        if(useCarry)
        {
            if(!(flagsToExclude & Flag::Carry))
            {
                setFlag(Flag::Carry, getFlag(Flag::Carry) > second || (second - getFlag(Flag::Carry)) > first);
            }
            
            result = first - second - getFlag(Flag::Carry);
        }
        else
        {
            if(!(flagsToExclude & Flag::Carry))
            {
                setFlag(Flag::Carry, second > first);
            }
            
            result = first - second;
        }
    }
    
    if(!(flagsToExclude & Flag::AuxillaryCarry))
    {
        //Get the bits which weren't set in first or second but were set in result?
        IntType carryBits = result ^ first ^ second;

        //Check the 4th bit
        IntType carryInFourthBit = (1 << 4) & carryBits;

        setFlag(Flag::AuxillaryCarry, carryInFourthBit > 0);
    }
    
    //Zero flag
    if(!(flagsToExclude & Flag::Zero))
    {
        setFlag(Flag::Zero, result == 0);
    }
        
    //Sign flag
    if(!(flagsToExclude & Flag::Sign))
    {
        const int numBits = std::numeric_limits<IntType>::digits;
        uint8_t test = result >> (numBits - 1);
        setFlag(Flag::Sign, test);
    }
        
    //Parity flag
    if(!(flagsToExclude & Flag::Parity))
    {
        setFlag(Flag::Parity, checkParity(result));
    }
    
    return result;
}
