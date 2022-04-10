//
//  RegisterManager.hpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 09/04/2022.
//

#pragma once

#include <array>
#include <optional>

class RegisterManager
{
public:
    RegisterManager();
    
    enum class Register
    {
        B,
        C,
        D,
        E,
        H,
        L,
        W,
        Z,
        A
    };
    
    enum class RegisterPair
    {
        BC,
        DE,
        HL
    };
    
    uint8_t getRegisterValue(Register reg) const;
    void setRegisterValue(Register reg, uint8_t newValue);
    
    uint16_t getValueFromRegisterPair(RegisterPair pair) const;
    void setRegisterPair(RegisterPair pair, uint8_t highOrderVal, uint8_t lowOrderVal);
    
    static std::optional<Register>  getRegFromEncodedValue(uint8_t value);
    static RegisterPair getPairFromEncodedValue(uint8_t value);
    
private:
    
    //Returns the first register in the pair
    Register regFromPair(RegisterPair pair) const;
    Register nextReg(Register reg) const;
    
    std::array<uint8_t, 8> registers;
};
