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
        AuxillaryCarry = 1 << 4,
        CarryFlags = 0x18,
        All = 0x1F
    };
    
    enum class Operation
    {
        Addition,
        Subtraction,
        And,
        Or,
        Xor,
        RotateRight,
        RotateLeft
    };
    
    void setFlag(Flag f, bool val);
    bool getFlag(Flag f) const;
    
    template<unsigned_integral IntType>
    IntType operateAndSetFlags(IntType first, IntType second, Operation op = Operation::Addition, Flag flagsToExclude = static_cast<Flag>(0), bool useCarry = false);
    
    uint8_t createStatusByte() const;
    void setFromStatusByte(uint8_t statusByte);
    
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
IntType ALU::operateAndSetFlags(IntType first, IntType second, Operation op, Flag flagsToExclude, bool useCarry)
{
    IntType result;
    
    switch(op)
    {
        case Operation::Addition:
            
            if(!(flagsToExclude & Flag::Carry))
            {
                //This seems to check for carry in apple clang
                setFlag(Flag::Carry, first + second + (useCarry ? getFlag(Flag::Carry) : 0) > std::numeric_limits<IntType>::max());
            }
        
            result = first + second + (useCarry ? getFlag(Flag::Carry) : 0);
            
            break;
        
        case Operation::Subtraction:
            
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
            
            break;
            
        case Operation::And:
            result = first & second;
            break;
            
        case Operation::Or:
            result = first | second;
            break;
            
        case Operation::Xor:
            result = first ^ second;
            break;
            
        case Operation::RotateRight:
        {
            IntType lowOrderBit = first & IntType(0x1);
            
            if(useCarry)
            {
                //Set the high order bit to the carry flag value
                result = (first >> 1) | (IntType(getFlag(Flag::Carry)) << (std::numeric_limits<IntType>::digits - 1));
            }
            else
            {
                //Set the high order bit to the low order bit
                result = (first >> 1) | (lowOrderBit << (std::numeric_limits<IntType>::digits - 1));
            }
            
            if(!(flagsToExclude & Flag::Carry))
            {
                setFlag(Flag::Carry, bool(lowOrderBit));
            }
            
            break;
        }
            
        case Operation::RotateLeft:
        {
            IntType highOrderBit = first >> (std::numeric_limits<IntType>::digits - 1);
            
            if(useCarry)
            {
                //Set the low order bit to the carry flag value
                result = (first << 1) | IntType(getFlag(Flag::Carry));
            }
            else
            {
                //Set the low order bit to the high order bit
                result = (first << 1) | highOrderBit;
            }
            
            if(!(flagsToExclude & Flag::Carry))
            {
                setFlag(Flag::Carry, bool(highOrderBit));
            }
            
            break;
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
