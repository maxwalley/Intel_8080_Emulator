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
        Zero = 1 << 0,
        Sign = 1 << 1,
        Parity = 1 << 2,
        Carry = 1 << 3,
        AuxillaryCarry = 1 << 4
    };
    
    void setFlag(Flag f, bool val);
    bool getFlag(Flag f) const;
    
    //Will work on zero, sign and parity bits
    template<unsigned_integral IntType>
    void setFlagsBasedOnValue(IntType val)
    {
        //Zero flag
        setFlag(Flag::Zero, val == 0);
            
        //Sign flag
        const int numBits = std::numeric_limits<IntType>::digits;
        uint8_t test = val >> (numBits - 1);
        setFlag(Flag::Sign, test);
            
        //Parity flag
        setFlag(Flag::Parity, checkParity(val));
    }
    
    //Checks and sets appropiate flag for carrys and borrows, if add is false performs a subtraction (first - second)
    template<unsigned_integral IntType>
    IntType operateAndSetCarryFlags(IntType first, IntType second, bool add = true, bool affectCarryFlag = true, bool useCarryFlag = false)
    {
        if(add)
        {
            //This seems to check for carry in apple clang
            setFlag(Flag::Carry, first + second + (useCarryFlag ? getFlag(Flag::Carry) : 0) > std::numeric_limits<IntType>::max());
        
            IntType result = first + second;
        
            //Get the bits which weren't set in first or second but were set in result?
            IntType carryBits = result ^ first ^ second;
        
            //Check the 4th bit
            IntType carryInFourthBit = (1 << 4) & carryBits;
        
            setFlag(Flag::AuxillaryCarry, carryInFourthBit > 0);
        
            return result;
        }
    
        //Carry turns into borrow flag
        if(useCarryFlag)
        {
            setFlag(Flag::Carry, getFlag(Flag::Carry) > second || (second - getFlag(Flag::Carry)) > first);
            return first - second - getFlag(Flag::Carry);
        }
        
        setFlag(Flag::Carry, second > first);
        return first - second;
    }
    
    template<unsigned_integral IntType>
    IntType operateAndSetAllFlags(IntType first, IntType second, bool add = true, bool carryFlag = false)
    {
        const auto result = operateAndSetCarryFlags(first, second, add, carryFlag);
        
        setFlagsBasedOnValue(result);
        
        return result;
    }
    
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
