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
#include <stack>
#include "Machine.hpp"

class Intel_8080_Emulator
{
public:
    Intel_8080_Emulator(std::unique_ptr<Machine> encapsulatingMachine);
    ~Intel_8080_Emulator();
    
private:
    void fetch();
    void decodeAndExecute();
    
    RegisterManager::Register getFirstRegister() const;
    RegisterManager::Register getSecondRegister() const;
    RegisterManager::RegisterPair getRegisterPair() const;
    
    uint16_t getAddressInDataBytes() const;
    
    bool checkCurrentCondition() const;
    
    uint8_t currentOpcode;
    
    uint16_t programCounter;
    
    std::stack<uint16_t> stack;
    
    std::array<uint8_t, 16384> memory;
    
    RegisterManager registers;
    ALU alu;
    
    bool haltFlag = false;
    
    std::unique_ptr<Machine> machine;
};
