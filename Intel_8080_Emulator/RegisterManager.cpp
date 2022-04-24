//
//  RegisterManager.cpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 09/04/2022.
//

#include "RegisterManager.hpp"

RegisterManager::RegisterManager()
{
    
}

uint8_t RegisterManager::getRegisterValue(Register reg) const
{
    return registers[static_cast<int>(reg)];
};

void RegisterManager::setRegisterValue(Register reg, uint8_t newValue)
{
    int regIndex = static_cast<int>(reg);
    
    registers[regIndex] = newValue;
}

uint16_t RegisterManager::getValueFromRegisterPair(RegisterPair pair) const
{
    Register firstReg = regFromPair(pair);
    Register secondReg = nextReg(firstReg);
    
    uint8_t higherOrderBits = getRegisterValue(firstReg);
    uint8_t LowerOrderBits = getRegisterValue(secondReg);
    
    return LowerOrderBits << 8 | higherOrderBits;
}

void RegisterManager::setRegisterPair(RegisterPair pair, uint8_t highOrderVal, uint8_t lowOrderVal)
{
    Register firstReg = regFromPair(pair);
    Register secondReg = nextReg(firstReg);
    
    setRegisterValue(firstReg, highOrderVal);
    setRegisterValue(secondReg, lowOrderVal);
}

std::optional<RegisterManager::Register> RegisterManager::getRegFromEncodedValue(uint8_t value)
{
    uint8_t index = value & 0x7;
    
    if(index <= 5)
    {
        return static_cast<Register>(index);
    }
    else if(index == 0x07)
    {
        return Register::A;
    }
    
    return std::nullopt;
}

RegisterManager::RegisterPair RegisterManager::getPairFromEncodedValue(uint8_t value)
{
    uint8_t index = value & 0x3;
    
    if(index == 3)
    {
        //We don't use a stack pointer
        assert(false);
        return RegisterPair::BC;
    }
    
    return static_cast<RegisterPair>(index);
}

RegisterManager::Register RegisterManager::regFromPair(RegisterPair pair) const
{
    switch (pair)
    {
        case RegisterPair::BC:
            return Register::B;
            
        case RegisterPair::DE:
            return Register::D;
            
        case RegisterPair::HL:
            return Register::H;
    };
}

RegisterManager::Register RegisterManager::nextReg(Register reg) const
{
    if(reg == Register::A)
    {
        assert(false);
        return Register::A;
    }
    
    return static_cast<Register>(static_cast<int>(reg) + 1);
}
