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
        Zero,
        Sign,
        Parity,
        Carry,
        AuxillaryCarry
    };
    
    void setFlag(Flag f, bool val);
    bool getFlag(Flag f) const;
    
    //Will work on zero, sign and parity bits
    template<unsigned_integral IntType>
    void setFlagsBasedOnValue(IntType val);
    
    //Checks and sets appropiate flag for carrys and borrows, if add is false performs a subtraction (first - second)
    template<unsigned_integral IntType>
    IntType operateAndSetCarryFlag(IntType first, IntType second, bool add);
    
private:
    
    //Returns true if even parity
    template<unsigned_integral IntType>
    bool checkParity(IntType val) const;

    //Called on init. Just checks that the method used to check for carrys works
    void checkCarryCheck();
    
    std::bitset<5> flags;
};
