//
//  Intel_8080_Emulator.cpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 09/04/2022.
//

#include "Intel_8080_Emulator.hpp"

#include <iostream>

Intel_8080_Emulator::Intel_8080_Emulator()
{
    programCounter = 0x0;
}

Intel_8080_Emulator::~Intel_8080_Emulator()
{
    
}

void Intel_8080_Emulator::runCycle()
{
    if(!haltFlag)
    {
        ++opCounter;
        
        fetch();
        
        if(debugMode)
        {
            std::cout << "------------------" << std::endl << "Opcode: " << std::bitset<8>(currentOpcode) << std::endl
                      << "Op Name: " << getCurrentOpName() << std::endl
                      << "Op Number: " << opCounter << std::endl
                      << "Current Data Bytes: " << getAddressInDataBytes() << std::endl
                      << "Current Condition: " << getCurrentConditionName() << std::endl
                      << "Current Flag Values: " << getFlagValuesStr() << std::endl
                      << "Current Reg Values: " << getCurrentRegValuesStr() << std::endl
                      << "Current Stack Pointer Val: " << registers.getValueFromRegisterPair(RegisterManager::RegisterPair::SP) << std::endl;
        }
        
        decodeAndExecute();
        
        const auto startGraphicsIt = memory.cbegin() + 0x2400;
        const auto endGraphicsIt = memory.cbegin() + 0x3FFF + 1;
        
        if(std::any_of(startGraphicsIt, endGraphicsIt, [](const uint8_t memVal)
        {
            return memVal != 0x0;
        }))
        {
            std::cout << "Graphics" << std::endl;
        }
    }
}

void Intel_8080_Emulator::fetch()
{
    currentOpcode = memory[programCounter];
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
                    
                    uint8_t result = alu.operateAndSetFlags(memory[address], uint8_t(1), ALU::Operation::Addition, ALU::Flag::Carry, false);
                    
                    memory[address] = result;
                    
                    ++programCounter;
                    return;
                }
                    
                //00110101 - Decrement Memory
                case 0x35:
                {
                    uint16_t address = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint8_t result = alu.operateAndSetFlags(memory[address], uint8_t(1), ALU::Operation::Subtraction, ALU::Flag::Carry, false);
                    
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
                    
                //00000111 - Rotate Left
                case 0x7:
                {
                    ALU::Flag flagsToIgnore = ALU::Flag::Zero | ALU::Flag::Sign | ALU::Flag::Parity | ALU::Flag::AuxillaryCarry;
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), uint8_t(0), ALU::Operation::RotateLeft, flagsToIgnore);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //00001111 - Rotate Right
                case 0xF:
                {
                    ALU::Flag flagsToIgnore = ALU::Flag::Zero | ALU::Flag::Sign | ALU::Flag::Parity | ALU::Flag::AuxillaryCarry;
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), uint8_t(0), ALU::Operation::RotateRight, flagsToIgnore);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //00010111 - Rotate Left Through Carry
                case 0x17:
                {
                    ALU::Flag flagsToIgnore = ALU::Flag::Zero | ALU::Flag::Sign | ALU::Flag::Parity | ALU::Flag::AuxillaryCarry;
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), uint8_t(0), ALU::Operation::RotateLeft, flagsToIgnore, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //00011111 - Rotate Right Through Carry
                case 0x1F:
                {
                    ALU::Flag flagsToIgnore = ALU::Flag::Zero | ALU::Flag::Sign | ALU::Flag::Parity | ALU::Flag::AuxillaryCarry;
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), uint8_t(0), ALU::Operation::RotateRight, flagsToIgnore, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //00101111 - Complement Accumulator
                case 0x2F:
                {
                    registers.setRegisterValue(RegisterManager::Register::A, ~registers.getRegisterValue(RegisterManager::Register::A));
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
                    
                //00000000 - No Op
                case 0x0:
                {
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
                    registers.setRegisterPair(getRegisterPair(), registers.getValueFromRegisterPair(getRegisterPair()) + 1);
                    
                    ++programCounter;
                    return;
                }
                    
                //00RP1011 - Decrement Register Pair
                case 0xB:
                {
                    registers.setRegisterPair(getRegisterPair(), registers.getValueFromRegisterPair(getRegisterPair()) - 1);
                    
                    ++programCounter;
                    return;
                }
                    
                //00RP1001 - Add Register Pair to H and L
                case 0x9:
                {
                    ALU::Flag flagsToExclude = ALU::Flag::Zero | ALU::Flag::Sign | ALU::Flag::Parity | ALU::Flag::AuxillaryCarry;
                    
                    uint16_t result = alu.operateAndSetFlags(registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL), registers.getValueFromRegisterPair(getRegisterPair()), ALU::Operation::Addition, flagsToExclude);
                    
                    registers.setRegisterPair(RegisterManager::RegisterPair::HL, result);
                    
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
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(getFirstRegister()), uint8_t(1), ALU::Operation::Addition, ALU::Flag::Carry, false);
                    
                    registers.setRegisterValue(getFirstRegister(), result);
                    
                    ++programCounter;
                    return;
                }
                
                //00DDD101 - Decrement Register
                case 0x5:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(getFirstRegister()), uint8_t(1), ALU::Operation::Subtraction, ALU::Flag::Carry, false);
                    
                    registers.setRegisterValue(getFirstRegister(), result);
                    
                    ++programCounter;
                    return;
                }
            }
            
            break;
            
        //01
        case 0x40:
            
            //01110110 - Halt
            if((currentOpcode & 0xFF) == 0x76)
            {
                haltFlag = true;
            }
            
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
            else if((currentOpcode & 0xF8) == 0x70)
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
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[location], ALU::Operation::Addition, ALU::Flag::None, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10010110 - Subtract Memory
                case 0x96:
                {
                    uint16_t location = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[location], ALU::Operation::Subtraction);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10011110 - Subtract Memory with Borrow
                case 0x9E:
                {
                    uint16_t location = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[location], ALU::Operation::Subtraction, ALU::Flag::None, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10100110 - AND Memory
                case 0xA6:
                {
                    uint16_t location = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[location], ALU::Operation::And, ALU::Flag::Carry);
                    
                    //Clear the carry flag
                    alu.setFlag(ALU::Flag::Carry, false);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10101110 - Exclusive OR Memory
                case 0xAE:
                {
                    uint16_t location = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[location], ALU::Operation::Xor, ALU::Flag::CarryFlags);
                    
                    //Clear the carry and aux carry flags
                    alu.setFlag(ALU::Flag::CarryFlags, false);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10110110 - OR Memory
                case 0xB6:
                {
                    uint16_t location = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[location], ALU::Operation::Or, ALU::Flag::CarryFlags);
                    
                    //Clear the carry and aux carry flags
                    alu.setFlag(ALU::Flag::CarryFlags, false);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10111110 - Compare Memory
                case 0xBE:
                {
                    uint8_t accVal = registers.getRegisterValue(RegisterManager::Register::A);
                    uint8_t memVal = memory[registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL)];
                    
                    alu.operateAndSetFlags(accVal, memVal, ALU::Operation::Subtraction, ALU::Flag::Carry | ALU::Flag::Zero);
                    
                    //The Z flag is set to 1 if (A) = ((H) (L))
                    alu.setFlag(ALU::Flag::Zero, accVal == memVal);
                    
                    //The CY flag is set to 1 if (A) < ((H) (L))
                    alu.setFlag(ALU::Flag::Carry, accVal < memVal);
                    
                    ++programCounter;
                    return;
                }
                    
                default:
                    break;
            }
            
            //Look at the first 5 bits
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
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(getSecondRegister()), registers.getRegisterValue(RegisterManager::Register::A), ALU::Operation::Addition, ALU::Flag::None, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10010SSS - Subtract Register
                case 0x90:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), registers.getRegisterValue(getSecondRegister()), ALU::Operation::Subtraction);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10011SSS - Subtract Register with borrow
                case 0x98:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), registers.getRegisterValue(getSecondRegister()), ALU::Operation::Subtraction, ALU::Flag::None, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                
                //10100SSS - AND Register
                case 0xA0:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), registers.getRegisterValue(getSecondRegister()), ALU::Operation::And, ALU::Flag::Carry);
                    
                    //Clear the carry flag
                    alu.setFlag(ALU::Flag::Carry, false);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10101SSS - Exclusive OR Register
                case 0xA8:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), registers.getRegisterValue(getSecondRegister()), ALU::Operation::Xor, ALU::Flag::CarryFlags);
                    
                    //Clear Carry and Aux Carry flags
                    alu.setFlag(ALU::Flag::CarryFlags, false);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10110SSS - OR Register
                case 0xB0:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), registers.getRegisterValue(getSecondRegister()), ALU::Operation::Or, ALU::Flag::CarryFlags);
                    
                    //Clear Carry and Aux Carry flags
                    alu.setFlag(ALU::Flag::CarryFlags, false);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    ++programCounter;
                    return;
                }
                    
                //10111SSS - Compare Register
                case 0xB8:
                {
                    uint8_t accVal = registers.getRegisterValue(RegisterManager::Register::A);
                    uint8_t regVal = registers.getRegisterValue(getSecondRegister());
                    
                    alu.operateAndSetFlags(accVal, regVal, ALU::Operation::Subtraction, ALU::Flag::Zero | ALU::Flag::Carry);
                    
                    //The Z flag is set to 1 if (A) = (r)
                    alu.setFlag(ALU::Flag::Zero, accVal == regVal);
                    
                    //The CY flag is set to 1 if (A) < (r)
                    alu.setFlag(ALU::Flag::Carry, accVal < regVal);
                    
                    ++programCounter;
                    return;
                }
                    
                default:
                    break;
            }
            break;
            
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
                    uint8_t result = alu.operateAndSetFlags(memory[programCounter + 1], registers.getRegisterValue(RegisterManager::Register::A), ALU::Operation::Addition, ALU::Flag::None, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    programCounter += 2;
                    return;
                }
                
                //11101011 - Exchange H and L with D and E
                case 0xEB:
                {
                    uint16_t hlVal = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    uint16_t deVal = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::DE);
                    
                    registers.setRegisterPair(RegisterManager::RegisterPair::DE, hlVal);
                    registers.setRegisterPair(RegisterManager::RegisterPair::HL, deVal);
                    
                    ++programCounter;
                    return;
                }
                
                //11010110 - Subtract Immediate
                case 0xD6:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[programCounter + 1], ALU::Operation::Subtraction);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    programCounter += 2;
                    return;
                }
                    
                //11011110 - Subtract Immediate with Borrow
                case 0xDE:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[programCounter + 1], ALU::Operation::Subtraction, ALU::Flag::None, true);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    programCounter += 2;
                    return;
                }
                    
                //11100110 - AND Immediate
                case 0xE6:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[programCounter + 1], ALU::Operation::And, ALU::Flag::CarryFlags);
                    
                    //Clear Carry and Aux Carry flags
                    alu.setFlag(ALU::Flag::CarryFlags, false);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    programCounter += 2;
                    return;
                }
                    
                //11101110 - Exclusive OR Immediate
                case 0xEE:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[programCounter + 1], ALU::Operation::Xor, ALU::Flag::CarryFlags);
                    
                    //Clear Carry and Aux Carry flags
                    alu.setFlag(ALU::Flag::CarryFlags, false);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    programCounter += 2;
                    return;
                }
                    
                //11110110 - OR Immediate
                case 0xF6:
                {
                    uint8_t result = alu.operateAndSetFlags(registers.getRegisterValue(RegisterManager::Register::A), memory[programCounter + 1], ALU::Operation::Or, ALU::Flag::CarryFlags);
                    
                    //Clear Carry and Aux Carry flags
                    alu.setFlag(ALU::Flag::CarryFlags, false);
                    
                    registers.setRegisterValue(RegisterManager::Register::A, result);
                    
                    programCounter += 2;
                    return;
                }
                    
                //11111110 - Compare Immediate
                case 0xFE:
                {
                    uint8_t accVal = registers.getRegisterValue(RegisterManager::Register::A);
                    uint8_t dataVal = memory[programCounter + 1];
                    
                    alu.operateAndSetFlags(accVal, dataVal, ALU::Operation::Subtraction, ALU::Flag::Zero | ALU::Flag::Carry);
                    
                    //The Z flag is set to 1 if (A) = (byte 2)
                    alu.setFlag(ALU::Flag::Zero, accVal == dataVal);
                    
                    //The CY flag is set to 1 if (A) < (byte 2)
                    alu.setFlag(ALU::Flag::Carry, accVal < dataVal);
                    
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
                    if(debugMode)
                    {
                        if(5 == ((memory[programCounter + 2] << 8) | memory[programCounter + 1]))
                        {
                            if(registers.getRegisterValue(RegisterManager::Register::C) == 9)
                            {
                                uint16_t offset = (registers.getRegisterValue(RegisterManager::Register::D) << 8) | (registers.getRegisterValue(RegisterManager::Register::E));
                            
                                const auto messageStart = memory.begin() + offset+3;
                                const auto messageEnd = std::find(messageStart, memory.end(), '$');
                            
                                std::for_each(messageStart, messageEnd, [](uint8_t val)
                                {
                                    std::cout << val;
                                });
                            
                                std::cout << std::endl;
                            }
                            else if (registers.getRegisterValue(RegisterManager::Register::C) == 2)
                            {
                                std::cout << "print char routine called" << std::endl;
                            }
                        }
                        else if (0 == ((memory[programCounter + 2] << 8) | memory[programCounter + 1]))
                        {
                            exit(0);
                        }
                        else
                        {
                            call();
                        }
                        return;
                    }
                    
                    call();
                    return;
                }
                    
                //11001001 - Return
                case 0xC9:
                {
                    ret();
                    return;
                }
                    
                    
                //11101001 - Jump H and L indirect - move H and L to Program Counter
                case 0xE9:
                {
                    programCounter = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    return;
                }
                    
                //11110101 - Push processor status word
                case 0xF5:
                {
                    uint16_t sp = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::SP);
                    
                    memory[--sp] = registers.getRegisterValue(RegisterManager::Register::A);
                    memory[--sp] = alu.createStatusByte();
                    
                    registers.setRegisterPair(RegisterManager::RegisterPair::SP, sp);
                    ++programCounter;
                    return;
                }
                    
                //11110001 - Pop processor status word
                case 0xF1:
                {
                    uint16_t sp = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::SP);
                    
                    alu.setFromStatusByte(memory[sp++]);
                    registers.setRegisterValue(RegisterManager::Register::A, memory[sp++]);
                    
                    registers.setRegisterPair(RegisterManager::RegisterPair::SP, sp);
                    ++programCounter;
                    return;
                }
                    
                //11100011 - Exchange stack top with H and L
                case 0xE3:
                {
                    const uint16_t oldStackVal = memory[registers.getValueFromRegisterPair(RegisterManager::RegisterPair::SP)];
                    const uint16_t hlRegVal = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL);
                    
                    registers.setRegisterPair(RegisterManager::RegisterPair::SP, hlRegVal);
                    registers.setRegisterPair(RegisterManager::RegisterPair::HL, oldStackVal);
                    
                    ++programCounter;
                    return;
                }
                    
                //11111001 - Move HL to SP
                case 0xF9:
                {
                    registers.setRegisterPair(RegisterManager::RegisterPair::SP, registers.getValueFromRegisterPair(RegisterManager::RegisterPair::HL));
                    
                    ++programCounter;
                    return;
                }
                    
                //11011011 - Input
                case 0xDB:
                {
                    registers.setRegisterValue(RegisterManager::Register::A, inputOperation(memory[programCounter + 1]));
                    ++programCounter;
                }
                    
                //11010011 - Output
                case 0xD3:
                {
                    outputOperation(memory[programCounter + 1], registers.getRegisterValue(RegisterManager::Register::A));
                    ++programCounter;
                }
                    
                //11111011 - Enable Interrupts
                case 0xFB:
                {
                    assert(false);
                }
                    
                //11110011 - Disable Interrupts
                case 0xF3:
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
                
                //11CCC100 - Conditional Call
                case 0x4:
                {
                    if(checkCurrentCondition())
                    {
                        //This is where we leave the script
                        //assert(false);
                        call();
                    }
                    else
                    {
                        programCounter += 3;
                    }
                    
                    return;
                }
                    
                //11CCC000 - Conditional Return
                case 0x0:
                {
                    if(checkCurrentCondition())
                    {
                        ret();
                    }
                    else
                    {
                        ++programCounter;
                    }
                    
                    return;
                }
                    
                //11NNN111 - Restart
                case 0x7:
                {
                    uint16_t sp = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::SP);
                    
                    memory[--sp] = programCounter >> 8;
                    memory[--sp] = programCounter;
                    
                    registers.setRegisterPair(RegisterManager::RegisterPair::SP, sp);
                    
                    uint8_t restartNumber = (currentOpcode & 0x38) >> 3;
                    programCounter = restartNumber * 8;
                    return;
                }
                    
                default:
                    break;
            }
            
        //Check the last 4 bits
        switch (currentOpcode & 0xF)
        {
            //11RP0101 - Push
            case 0x5:
            {
                const uint16_t val = registers.getValueFromRegisterPair(getRegisterPair());
                uint16_t sp = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::SP);
                
                memory[--sp] = val >> 8;
                memory[--sp] = val;
                
                registers.setRegisterPair(RegisterManager::RegisterPair::SP, sp);
                ++programCounter;
                return;
            }
                
            //11RP0001 - Pop
            case 0x1:
            {
                uint16_t sp = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::SP);
                
                registers.setRegisterPair(getRegisterPair(), memory[sp + 1], memory[sp]);
                
                sp += 2;
                registers.setRegisterPair(RegisterManager::RegisterPair::SP, sp);
                ++programCounter;
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
    uint8_t regIndex =  currentOpcode & 0x7;
    
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
    
    return highOrderAddrData << 8 | lowOrderAddrData;
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

void Intel_8080_Emulator::call()
{
    uint16_t sp = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::SP);
    
    uint16_t nextInstructionPos = programCounter + 3;
    
    std::cout << "Pos Stored: " << nextInstructionPos << std::endl;
    
    memory[--sp] = nextInstructionPos >> 8;
    memory[--sp] = nextInstructionPos;
    
    registers.setRegisterPair(RegisterManager::RegisterPair::SP, sp);
    
    programCounter = getAddressInDataBytes();
}

void Intel_8080_Emulator::ret()
{
    const uint16_t sp = registers.getValueFromRegisterPair(RegisterManager::RegisterPair::SP);
    
    programCounter = (memory[sp + 1] << 8) | memory[sp];
    
    std::cout << "Pos Restored: " << programCounter << std::endl;
    
    registers.setRegisterPair(RegisterManager::RegisterPair::SP, sp + 2);
}

std::string Intel_8080_Emulator::getCurrentOpName() const
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
                    return "Move to memory immediate";
                    
                //00111010 - Load Accumulator Direct
                case 0x3A:
                    return "Load accumulator direct";
                    
                //00110010 - Store Accumulator Direct
                case 0x32:
                    return "Store accumulator direct";
                    
                //00101010 - Load H and L direct
                case 0x2A:
                    return "Load H and L direct";
                    
                //00100010 - Store H and L direct
                case 0x22:
                    return "Store H and L direct";
                    
                //00110100 - Increment Memory
                case 0x34:
                    return "Increment memory";
                    
                //00110101 - Decrement Memory
                case 0x35:
                    return "Decrement memory";
                    
                //00100111 - Decimal Adjust Accumulator
                case 0x27:
                    return "Decimal adjust accumulator";
                    
                //00000111 - Rotate Left
                case 0x7:
                    return "Rotate left";
                    
                //00001111 - Rotate Right
                case 0xF:
                    return "Rotate right";
                    
                //00010111 - Rotate Left Through Carry
                case 0x17:
                    return "Rotate left through carry";
                    
                //00011111 - Rotate Right Through Carry
                case 0x1F:
                    return "Rotate right through carry";
                    
                //00101111 - Complement Accumulator
                case 0x2F:
                    return "Complement accumulator";
                 
                //00111111 - Complement Carry
                case 0x3F:
                    return "Complement carry";
                    
                //00110111 - Set Carry
                case 0x37:
                    return "Set carry";
                    
                //00000000 - No Op
                case 0x0:
                    return "No Op";
                    
                default:
                    break;
            }
            
            //Look at last 4 bits
            switch(currentOpcode & 0xCF)
            {
                //00RP0001 - Load Register Pair Immediate
                case 0x1:
                    return "Load register pair immediate";
                
                //00RP1010 - Load accumulator indirect
                case 0xA:
                    return "Load accumulator indirect";
                
                //00RP0010 - Store accumulator indirect
                case 0x2:
                    return "Store accumulator indirect";
                    
                //00RP0011 - Increment Register Pair
                case 0x3:
                    return "Increment register pair";
                    
                //00RP1011 - Decrement Register Pair
                case 0xB:
                    return "Decrement Register Pair";
                    
                //00RP1001 - Add Register Pair to H and L
                case 0x9:
                    return "Add Register Pair to H and L";
            }
            
            //Look at the last 3 bits
            switch(currentOpcode & 0xC7)
            {
                //00DDD110 - Move Immediate
                case 0x6:
                    return "Move immediate";
                    
                //00DDD100 - Increment Register
                case 0x4:
                    return "Increment register";
                
                //00DDD101 - Decrement Register
                case 0x5:
                    return "Decrement register";
                    
            }
            
        //01
        case 0x40:
            
            //01110110 - Halt
            if((currentOpcode & 0xFF) == 0x76)
                return "Halt";
            
            //01DDD110 - Move from memory
            if((currentOpcode & 0x7) == 0x6)
                return "Move from memory";
            
            //01110SSS - Move to memory
            else if((currentOpcode & 0xF8) == 0x70)
                return "Move to memory";
            
            //01DDDSSS - Move register
            else
                return "Move register";
            
            break;
    
        //10
        case 0x80:
            
            switch (currentOpcode & 0xFF)
            {
                //10000110 - Add Memory
                case 0x86:
                    return "Add memory";
                    
                //10001110 - Add memory with carry
                case 0x8E:
                    return "Add memory with carry";
                    
                //10010110 - Subtract Memory
                case 0x96:
                    return "Subtract memory";
                    
                //10011110 - Subtract Memory with Borrow
                case 0x9E:
                    return "Subtract memory with borrow";
                    
                //10100110 - AND Memory
                case 0xA6:
                    return "AND memory";
                    
                //10101110 - Exclusive OR Memory
                case 0xAE:
                    return "Exclusive OR memory";
                    
                //10110110 - OR Memory
                case 0xB6:
                    return "OR memory";
                    
                //10111110 - Compare Memory
                case 0xBE:
                    return "Compare Memory";
                    
                default:
                    break;
            }
            
            //Look at the first 5 bits
            switch(currentOpcode & 0xF8)
            {
                //10000SSS - Add Register
                case 0x80:
                    return "Add register";
                
                //10001SSS - Add Register with carry
                case 0x88:
                    return "Add register with carry";
                    
                //10010SSS - Subtract Register
                case 0x90:
                    return "Subtract register";
                    
                //10011SSS - Subtract Register with borrow
                case 0x98:
                    return "Subtract register with borrow";
                
                //10100SSS - AND Register
                case 0xA0:
                    return "AND register";
                    
                //10101SSS - Exclusive OR Register
                case 0xA8:
                    return "Exclusive OR register";
                    
                //10110SSS - OR Register
                case 0xB0:
                    return "OR register";
                    
                //10111SSS - Compare Register
                case 0xB8:
                    return "Compare register";
                    
                default:
                    break;
            }
            break;
            
        //11
        case 0xc0:
            
            switch (currentOpcode & 0xFF)
            {
                //11000110 - Add Immediate
                case 0xC6:
                    return "Add immediate";
                    
                //11001110 - Add Immediate with Carry
                case 0xCE:
                    return "Add immediate with Carry";
                
                //11101011 - Exchange H and L with D and E
                case 0xEB:
                    return "Exchange H and L with D and E";
                
                //11010110 - Subtract Immediate
                case 0xD6:
                    return "Subtract immediate";
                    
                //11011110 - Subtract Immediate with Borrow
                case 0xDE:
                    return "Subtract immediate with borrow";
                    
                //11100110 - AND Immediate
                case 0xE6:
                    return "AND immediate";
                    
                //11101110 - Exclusive OR Immediate
                case 0xEE:
                    return "Exclusive OR immediate";
                    
                //11110110 - OR Immediate
                case 0xF6:
                    return "OR immediate";
                    
                //11111110 - Compare Immediate
                case 0xFE:
                    return "Compare immediate";
                    
                //11000011 - Jump
                case 0xC3:
                    return "Jump";
                    
                //11001101 - Call
                case 0xCD:
                    return "Call";
                    
                //11001001 - Return
                case 0xC9:
                    return "Return";
                    
                //11101001 - Jump H and L indirect - move H and L to Program Counter
                case 0xE9:
                    return "Jump H and L indirect - move H and L to program counter";
                    
                //11110101 - Push processor status word
                case 0xF5:
                    return "Push processor status word";
                    
                //11110001 - Pop processor status word
                case 0xF1:
                    return "Pop processor status word";
                    
                //11100011 - Exchange stack top with H and L
                case 0xE3:
                    return "Exchange stack top with H and L";
                    
                //11111001 - Move HL to SP
                case 0xF9:
                    return "Move HL to SP";
                    
                //11011011 - Input
                case 0xDB:
                    return "Input";
                    
                //11010011 - Output
                case 0xD3:
                    return "Output";
                    
                //11111011 - Enable Interrupts
                case 0xFB:
                    return "Enable interrupts";
                    
                //11110011 - Disable Interrupts
                case 0xF3:
                    return "Disable interrupts";
                    
                default:
                    break;
            }
            
            //Check last 3 bits
            switch (currentOpcode & 0x7)
            {
                //11CCC010 - Conditional Jump
                case 0x2:
                    return "Conditional jump";
                
                //11CCC100 - Conditional Call
                case 0x4:
                    return "Conditional call";
                    
                //11CCC000 - Conditional Return
                case 0x0:
                    return "Conditional return";
                    
                //11NNN111 - Restart
                case 0x7:
                    return "Restart";
                    
                default:
                    break;
            }
            
        //Check the last 4 bits
        switch (currentOpcode & 0xF)
        {
            //11RP0101 - Push
            case 0x5:
                return "Push";
                
            //11RP0001 - Pop
            case 0x1:
                return "Pop";
                
            default:
                break;
        }
    }
    
    return "Unrecognised Op";
}

std::string Intel_8080_Emulator::getCurrentConditionName() const
{
    uint8_t conditionVal = (currentOpcode & 0x38) >> 3;
    
    switch(conditionVal & 0x7)
    {
        case 0x0:
            return "Not Zero";
            
        case 0x1:
            return "Zero";
            
        case 0x2:
            return "No Carry";
            
        case 0x3:
            return "Carry";
            
        case 0x4:
            return "Parity Odd";
            
        case 0x5:
            return "Parity Even";
            
        case 0x6:
            return "Plus";
            
        case 0x7:
            return "Minus";
    }
    
    return "";
}

std::string Intel_8080_Emulator::getFlagValuesStr() const
{
    std::stringstream strm;
    
    strm << "Z: " << int(alu.getFlag(ALU::Flag::Zero)) << ", S: " << int(alu.getFlag(ALU::Flag::Sign)) << ", P: " << int(alu.getFlag(ALU::Flag::Parity)) << ", C: " << int(alu.getFlag(ALU::Flag::Carry)) << ", AC: " << int(alu.getFlag(ALU::Flag::AuxillaryCarry));
    
    return strm.str();
}

std::string Intel_8080_Emulator::getCurrentRegValuesStr() const
{
    std::stringstream strm;
    
    strm << "B: " << int(registers.getRegisterValue(RegisterManager::Register::B)) <<
          ", C: " << int(registers.getRegisterValue(RegisterManager::Register::C)) <<
          ", D: " << int(registers.getRegisterValue(RegisterManager::Register::D)) <<
          ", E: " << int(registers.getRegisterValue(RegisterManager::Register::E)) <<
          ", H: " << int(registers.getRegisterValue(RegisterManager::Register::H)) <<
          ", L: " << int(registers.getRegisterValue(RegisterManager::Register::L)) <<
          ", W: " << int(registers.getRegisterValue(RegisterManager::Register::W)) <<
          ", Z: " << int(registers.getRegisterValue(RegisterManager::Register::Z)) <<
          ", A: " << int(registers.getRegisterValue(RegisterManager::Register::A));
    
    return strm.str();
}
