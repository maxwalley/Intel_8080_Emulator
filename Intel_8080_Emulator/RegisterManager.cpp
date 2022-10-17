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
    return registers.at(static_cast<int>(reg));
};

void RegisterManager::setRegisterValue(Register reg, uint8_t newValue)
{
    int regIndex = static_cast<int>(reg);
    
    registers.at(regIndex) = newValue;
}

uint16_t RegisterManager::getValueFromRegisterPair(RegisterPair pair) const
{
    if(pair == RegisterPair::SP)
    {
        return stackPointer;
    }
    
    Register firstReg = regFromPair(pair);
    Register secondReg = nextReg(firstReg);
    
    uint8_t higherOrderBits = getRegisterValue(firstReg);
    uint8_t LowerOrderBits = getRegisterValue(secondReg);
    
    return higherOrderBits << 8 | LowerOrderBits;
}

void RegisterManager::setRegisterPair(RegisterPair pair, uint8_t highOrderVal, uint8_t lowOrderVal)
{
    setRegisterPair(pair, (highOrderVal << 8) | lowOrderVal);
}

void RegisterManager::setRegisterPair(RegisterPair pair, uint16_t val)
{
    if(pair == RegisterPair::SP)
    {
        stackPointer = val;
        return;
    }
    
    Register firstReg = regFromPair(pair);
    Register secondReg = nextReg(firstReg);
    
    setRegisterValue(firstReg, val >> 8);
    setRegisterValue(secondReg, val);
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
        return RegisterPair::SP;
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
            
        case RegisterPair::SP:
            assert(false);
            return Register::B;
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
