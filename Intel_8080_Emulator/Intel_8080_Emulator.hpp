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
    
    void runCycle();
    
    static constexpr bool debugMode = true;
    
protected:
    std::array<uint8_t, 65535> memory;
    
    uint16_t programCounter;
    
private:
    virtual uint8_t inputOperation(uint8_t port)=0;
    virtual void outputOperation(uint8_t port, uint8_t value)=0;
    
    void fetch();
    void decodeAndExecute();
    
    RegisterManager::Register getFirstRegister() const;
    RegisterManager::Register getSecondRegister() const;
    RegisterManager::RegisterPair getRegisterPair() const;
    
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
    
    uint64_t opCounter = 0;
};
