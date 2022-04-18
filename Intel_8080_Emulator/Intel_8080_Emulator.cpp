//
//  Intel_8080_Emulator.cpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 09/04/2022.
//

#include "Intel_8080_Emulator.hpp"

Intel_8080_Emulator::Intel_8080_Emulator()
{
    
}

Intel_8080_Emulator::~Intel_8080_Emulator()
{
    
}

void Intel_8080_Emulator::fetch()
{
    
}

void Intel_8080_Emulator::decodeAndExecute()
{
    //Check first two bits
    switch(currentOpcode & 0xc0)
    {
        //00
        case 0x00:
            
            switch(currentOpcode & 0xFF)
            {
                //00110110 - Move to memory immediate
                case 0x36:
                {
                    uint16_t destMemLocation = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    uint8_t dataByte = memory[programCounter + 1];
                    
                    memory[destMemLocation] = dataByte;
                    
                    programCounter += 2;
                    return;
                }
                    
                //00111010 - Load Accumulator Direct
                case 0x3A:
                {
                    uint8_t data = memory[getAddressInDataBytes()];
                    registers.setRegisterValue(RegisterManager::Register::A, data);
                    
                    programCounter += 3;
                    return;
                }
                    
                //00110010 - Store Accumulator Direct
                case 0x32:
                {
                    uint8_t data = registers.getRegisterValue(RegisterManager::Register::A);
                    memory[getAddressInDataBytes()] = data;
                    
                    programCounter += 3;
                    return;
                }
                    
                //00101010 - Load H and L direct
                case 0x2A:
                {
                    uint16_t sourceMemoryAddress = getAddressInDataBytes();
                    
                    registers.setRegisterValue(RegisterManager::Register::L, memory[sourceMemoryAddress]);
                    registers.setRegisterValue(RegisterManager::Register::H, memory[sourceMemoryAddress + 1]);
                    
                    programCounter += 3;
                    return;
                }
                    
                //00100010 - Store H and L direct
                case 0x22:
                {
                    uint16_t destMemoryAddress = getAddressInDataBytes();
                    
                    memory[destMemoryAddress] = registers.getRegisterValue(RegisterManager::Register::L);
                    memory[destMemoryAddress + 1] = registers.getRegisterValue(RegisterManager::Register::H);
                    
                    programCounter += 3;
                    return;
                }
                    
                //00110100 - Increment Memory
                case 0x34:
                {
                    uint16_t address = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint8_t result = alu.operateAndSetFlags(memory[address], uint8_t(1), true, ALU::Flag::Carry, false);
                    
                    memory[address] = result;
                    
                    ++programCounter;
                    return;
                }
                    
                //00110101 - Decrement Memory
                case 0x35:
                {
                    uint16_t address = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint8_t result = alu.operateAndSetFlags(memory[address], uint8_t(1), false, ALU::Flag::Carry, false);
                    
                    memory[address] = result;
                    
                    ++programCounter;
                    return;
                }
                    
                //00100111 - Decimal Adjust Accumulator
                case 0x27:
                {
                    uint8_t accumulatorVal = registers.getRegisterValue(RegisterManager::Register::A);
                    
                    //If the value of the least significant 4 bits of the accumulator is greater than 9 or if the AC flag is set, 6 is added to the accumulator.
                    if(((accumulatorVal & 0xF) > 0x9) | alu.getFlag(ALU::Flag::AuxillaryCarry))
                    {
                        accumulatorVal += 0x6;
                    }
                    
                    //If the value of the most significant 4 bits of the accumulator is now greater than 9, or if the CY flag is set, 6 is added to the most significant 4 bits of the accumulator.
                    if((((accumulatorVal & 0xF0) >> 4) > 0x9) | alu.getFlag(ALU::Flag::Carry))
                    {
                        accumulatorVal += 0x60;
                    }
                    
                    registers.setRegisterValue(RegisterManager::Register::A, accumulatorVal);
                    ++programCounter;
                    return;
                }
                 
                //00111111 - Complement Carry
                case 0x3F:
                {
                    alu.setFlag(ALU::Flag::Carry, !alu.getFlag(ALU::Flag::Carry));
                    ++programCounter;
                    return;
                }
                    
                //00110111 - Set Carry
                case 0x37:
                {
                    alu.setFlag(ALU::Flag::Carry, true);
                    ++programCounter;
                    return;
                }
                    
                default:
                    break;
            }
            
            //Look at last 4 bits
            switch(currentOpcode & 0xCF)
            {
                //00RP0001 - Load Register Pair Immediate
                case 0x1:
                {
                    RegisterManager::RegisterPair destPair = getRegisterPair();
                    uint8_t lowOrderByte = memory[programCounter + 1];
                    uint8_t highOrderByte = memory[programCounter + 2];
                    
                    registers.setRegisterPair(destPair, highOrderByte, lowOrderByte);
                    
                    programCounter += 3;
                    return;
                }
                
                //00RP1010 - Load accumulator indirect
                case 0xA:
                {
                    RegisterManager::RegisterPair pair = getRegisterPair();
                
                    if(pair != RegisterManager::RegisterPair::BC && pair != RegisterManager::RegisterPair::DE)
                    {
                        assert(false);
                        return;
                    }
                    
                    uint8_t data = memory[registers.getValueFromRegisterPair(pair)];
                    
                    registers.setRegisterValue(RegisterManager::Register::A, data);
                    
                    ++programCounter;
                    return;
                }
                
                //00RP0010 - Store accumulator indirect
                case 0x2:
                {
                    RegisterManager::RegisterPair pair = getRegisterPair();
                    
                    if(pair != RegisterManager::RegisterPair::BC && pair != RegisterManager::RegisterPair::DE)
                    {
                        assert(false);
                        return;
                    }
                    
                    uint16_t destAddress = registers.getValueFromRegisterPair(pair);
                    
                    memory[destAddress] = registers.getRegisterValue(RegisterManager::Register::A);
                    
                    ++programCounter;
                    return;
                }
                    
                //00RP0011 - Increment Register Pair
                case 0x3:
                {
                    uint16_t incrementedVal = registers.getValueFromRegisterPair(getRegisterPair()) + 1;
                    
                    registers.setRegisterPair(getRegisterPair(), incrementedVal >> 8, incrementedVal);
                    
                    ++programCounter;
                    return;
                }
                    
                //00RP1011 - Decrement Register Pair
                case 0xB:
                {
                    uint16_t decrementedVal = registers.getValueFromRegisterPair(getRegisterPair()) - 1;
                    
                    registers.setRegisterPair(getRegisterPair(), decrementedVal >> 8, decrementedVal);
                    
                    ++programCounter;
                    return;
                }
                    
                //00RP1001 - Add Register Pair to H and L
                case 0x9:
                {
                    ALU::Flag flagsToExclude = ALU::Flag::Zero | ALU::Flag::Sign | ALU::Flag::Parity | ALU::Flag::AuxillaryCarry;
                    
                    uint16_t result = alu.operateAndSetFlags(registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL), registers.getValueFromRegisterPair(getRegisterPair()), true, flagsToExclude);
                    
                    registers.setRegisterPair(RegisterManager::RegisterPair::HL, result >> 8, result);
                    
                    ++programCounter;
                    return;
                }
            }
            
            //Look at the last 3 bits
            switch(currentOpcode & 0xC7)
            {
                //00DDD110 - Move Immediate
                case 0x6:
                {
                    RegisterManager::Register destReg = getFirstRegister();
                    uint8_t dataByte = memory[programCounter + 1];

                    registers.setRegisterValue(destReg, dataByte);
                        
                    programCounter += 2;
                    return;
                }
                    
                //00DDD100 - Increment Register
                case 0x4:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(getFirstRegister()), uint8_t(1), true, ALU::Flag::Carry, false);
                    
                    registers.setRegisterValue(getFirstRegister(), result);
                    
                    ++programCounter;
                    return;
                }
                
                //00DDD101 - Decrement Register
                case 0x5:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(getFirstRegister()), uint8_t(1), false, ALU::Flag::Carry, false);
                    
                    registers.setRegisterValue(getFirstRegister(), result);
                    
                    ++programCounter;
                    return;
                }
                    
            }
            
        //01
        case 0x40:
            
            //01DDD110 - Move from memory
            if((currentOpcode & 0x7) == 0x6)
            {
                uint16_t sourceMemoryLocation = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                
                RegisterManager::Register destReg = getFirstRegister();
                
                registers.setRegisterValue(destReg, memory[sourceMemoryLocation]);
                
                ++programCounter;
                return;
            }
            
            //01110SSS - Move to memory
            else if((currentOpcode & 0x70) == 0x70)
            {
                uint16_t destMemoryLocation = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                
                RegisterManager::Register sourceReg = getSecondRegister();
                
                memory[destMemoryLocation] = registers.getRegisterValue(sourceReg);
                
                ++programCounter;
                return;
            }
            
            //01DDDSSS - Move register
            else
            {
                RegisterManager::Register firstReg = getFirstRegister();
                RegisterManager::Register secondReg = getSecondRegister();
                
                registers.setRegisterValue(firstReg, registers.getRegisterValue(secondReg));
                
                ++programCounter;
                return;
            }
            
            break;
    
        //10
        case 0x80:
            
            switch (currentOpcode & 0xFF)
            {
                //10000110 - Add Memory
                case 0x86:
                {
                    uint16_t location = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[location]);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10001110 - Add memory with carry
                case 0x8E:
                {
                    uint16_t location = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[location], true, ALU::Flag::None, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10010110 - Subtract Memory
                case 0x96:
                {
                    uint16_t location = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[location], false);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10011110 - Subtract Memory with Borrow
                case 0x9E:
                {
                    uint16_t location = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[location], false, ALU::Flag::None, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                default:
                    break;
            }
            
            switch(currentOpcode & 0xF8)
            {
                //10000SSS - Add Register
                case 0x80:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(getSecondRegister()), registers.getRegisterValue(RegisterManager::Register::A));
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                
                //10001SSS - Add Register with carry
                case 0x88:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(getSecondRegister()), registers.getRegisterValue(RegisterManager::Register::A), true, ALU::Flag::None, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10010SSS - Subtract Register
                case 0x90:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), registers.getRegisterValue(getSecondRegister()), false);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10011SSS - Subtract Register with borrow
                case 0x91:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), registers.getRegisterValue(getSecondRegister()), false, ALU::Flag::None, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                
                default:
                    break;
            }
            return;
            
        //11
        case 0xc0:
            
            switch (currentOpcode & 0xFF)
            {
                //11000110 - Add Immediate
                case 0xC6:
                {
                    uint8_t result = alu.operateAndSetFlags(memory[programCounter + 1], registers.getRegisterValue(RegisterManager::Register::A));
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    programCounter += 2;
                    return;
                }
                    
                //11001110 - Add Immediate with Carry
                case 0xCE:
                {
                    uint8_t result = alu.operateAndSetFlags(memory[programCounter + 1], registers.getRegisterValue(RegisterManager::Register::A), true, ALU::Flag::None, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    programCounter += 2;
                    return;
                }
                
                //11101011 - Exchange H and L with D and E
                case 0xEB:
                {
                    uint16_t hlVal = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint16_t deVal = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::DE);
                    
                    registers.setRegisterPair(RegisterManager::RegisterPair::DE, hlVal >> 8, hlVal);
                    registers.setRegisterPair(RegisterManager::RegisterPair::HL, deVal >> 8, deVal);
                    
                    ++programCounter;
                    return;
                }
                
                //11010110 - Subtract Immediate
                case 0xD6:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[programCounter + 1], false);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    programCounter += 2;
                    return;
                }
                    
                //11011110 - Subtract Immediate with Borrow
                case 0xDE:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[programCounter + 1], false, static_cast<ALU::Flag>(0), true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    programCounter += 2;
                    return;
                }
                    
                //11000011 - Jump
                case 0xC3:
                {
                    programCounter = getAddressInDataBytes();
                    return;
                }
                    
                //11001101 - Call
                case 0xCD:
                {
                    assert(false);
                }
                    
                default:
                    break;
            }
            
            //Check last 3 bits
            switch (currentOpcode & 0x7)
            {
                //11CCC010 - Conditional Jump
                case 0x2:
                {
                    if(checkCurrentCondition())
                    {
                        programCounter = getAddressInDataBytes();
                    }
                    else
                    {
                        programCounter += 3;
                    }
                    
                    return;
                }
                    
                default:
                    break;
            }
            
    }
}

RegisterManager::Register Intel_8080_Emulator::getFirstRegister() const
{
    uint8_t regIndex = (currentOpcode & 0x38) >> 3;
    
    if(const auto reg = RegisterManager::getRegFromEncodedValue(regIndex); reg)
    {
        return *reg;
    }
    
    assert(false);
    return RegisterManager::Register::A;
}

RegisterManager::Register Intel_8080_Emulator::getSecondRegister() const
{
    uint8_t regIndex =  currentOpcode & 0x6;
    
    if(const auto reg = RegisterManager::getRegFromEncodedValue(regIndex); reg)
    {
        return *reg;
    }
    
    assert(false);
    return RegisterManager::Register::A;
}

RegisterManager::RegisterPair Intel_8080_Emulator::getRegisterPair() const
{
    uint8_t pairIndex = (currentOpcode & 0x30) >> 4;
    
    return RegisterManager::getPairFromEncodedValue(pairIndex);
}

uint16_t Intel_8080_Emulator::getAddressInDataBytes() const
{
    uint8_t lowOrderAddrData = memory[programCounter + 1];
    uint8_t highOrderAddrData = memory[programCounter + 2];
    
    return lowOrderAddrData << 8 | highOrderAddrData;
}

bool Intel_8080_Emulator::checkCurrentCondition() const
{
    uint8_t conditionVal = (currentOpcode & 0x38) >> 3;
    
    switch(conditionVal & 0x7)
    {
        case 0x0:
            return !alu.getFlag(ALU::Flag::Zero);
            
        case 0x1:
            return alu.getFlag(ALU::Flag::Zero);
            
        case 0x2:
            return !alu.getFlag(ALU::Flag::Carry);
            
        case 0x3:
            return alu.getFlag(ALU::Flag::Carry);
            
        case 0x4:
            return !alu.getFlag(ALU::Flag::Parity);
            
        case 0x5:
            return alu.getFlag(ALU::Flag::Parity);
            
        case 0x6:
            return !alu.getFlag(ALU::Flag::Sign);
            
        case 0x7:
            return alu.getFlag(ALU::Flag::Sign);
    }
    
    return false;
}
