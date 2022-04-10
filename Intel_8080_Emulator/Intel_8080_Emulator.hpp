//
//  Intel_8080_Emulator.hpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 09/04/2022.
//

#pragma once

#include <cstdint>
#include <vector>
#include "RegisterManager.hpp"
#include "ALU.hpp"

class Intel_8080_Emulator
{
public:
    Intel_8080_Emulator();
    ~Intel_8080_Emulator();
    
private:
    void fetch();
    void decodeAndExecute();
    
    RegisterManager::Register getFirstRegister() const;
    RegisterManager::Register getSecondRegister() const;
    RegisterManager::RegisterPair getRegisterPair() const;
    
    uint16_t getAddressInDataBytes() const;
    
    uint8_t currentOpcode;
    
    uint16_t programCounter;
    
    uint8_t stackPointer;
    std::vector<uint16_t> stack;
    
    std::array<uint8_t, 16384> memory;
    
    RegisterManager registers;
    ALU alu;
};
