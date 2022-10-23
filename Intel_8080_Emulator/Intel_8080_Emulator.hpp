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
#include <sstream>

class Intel_8080_Emulator
{
public:
    Intel_8080_Emulator();
    virtual ~Intel_8080_Emulator();
    
    static constexpr bool debugMode = false;
    
protected:
    void runCycle();
    void performInterrupt(uint8_t opcode);
    
    std::array<uint8_t, 65535> memory;
    
    uint16_t programCounter;
    
private:
    virtual uint8_t inputOperation(uint8_t port)=0;
    virtual void outputOperation(uint8_t port, uint8_t value)=0;
    
    void fetch();
    void decodeAndExecute(uint8_t opcode);
    
    RegisterManager::Register getFirstRegister(uint8_t opcode) const;
    RegisterManager::Register getSecondRegister(uint8_t opcode) const;
    RegisterManager::RegisterPair getRegisterPair(uint8_t opcode) const;
    
    uint16_t getAddressInDataBytes() const;
    
    bool checkCurrentCondition() const;
    
    void call();
    void ret();
    
    std::string getCurrentOpName() const;
    std::string getCurrentConditionName() const;
    std::string getFlagValuesStr() const;
    std::string getCurrentRegValuesStr() const;
    
    uint8_t currentOpcode;
    
    RegisterManager registers;
    ALU alu;
    
    bool haltFlag = false;
    bool interrupts = false;
    
    uint64_t opCounter = 0;
};
