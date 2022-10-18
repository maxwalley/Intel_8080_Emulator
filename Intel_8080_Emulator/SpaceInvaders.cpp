//
//  SpaceInvaders.cpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 24/04/2022.
//

#include "SpaceInvaders.hpp"
#include <thread>

SpaceInvaders::SpaceInvaders()
{
    if(debugMode ? !loadTest() : !loadGame())
    {
        std::cout << "Warning game failed to load" << std::endl;
        assert(false);
    }
    
    while(true)
    {
        runCycle();
        //std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

SpaceInvaders::~SpaceInvaders()
{
    
}

void SpaceInvaders::triggerKeyDown(int keycode, int x, int y)
{
    if(std::find(currentlyDownKeys.cbegin(), currentlyDownKeys.cend(), keycode) == currentlyDownKeys.cend())
    {
        currentlyDownKeys.push_back(keycode);
    }
}

void SpaceInvaders::triggerKeyUp(int keycode, int x, int y)
{
    currentlyDownKeys.erase(std::remove(currentlyDownKeys.begin(), currentlyDownKeys.end(), keycode), currentlyDownKeys.end());
}

uint8_t SpaceInvaders::inputOperation(uint8_t port)
{
    switch (port)
    {
        case 0x1:
        {
            uint8_t retVal = 0x8;
            
            //Space Key
            if(checkKeyDown(32))
            {
                retVal |= 0x15;
            }
            
            //Left Key
            if(checkKeyDown(100))
            {
                retVal |= 0x20;
            }
            
            //Right Key
            if(checkKeyDown(102))
            {
                retVal |= 0x40;
            }
        }
            
        //Shift the data and read
        case 0x3:
        {
            uint8_t realShiftAmount = 0x8 - currentShiftOffset;
            return currentShiftVal >> realShiftAmount;
        }
            
        default:
            return 0x0;
    }
}

void SpaceInvaders::outputOperation(uint8_t port, uint8_t value)
{
    switch (port)
    {
        //Set Shift Amount (3 Bits)
        case 0x2:
        {
            currentShiftOffset = value & 0x7;
            return;
        }
            
        //Discrete Sounds
        case 0x3:
        {
            if(value & 0x80)
            {
                std::cout << "UFO SOUND" << std::endl;
            }
            
            if(value & 0x40)
            {
                std::cout << "SHOT SOUND" << std::endl;
            }
            
            if(value & 0x20)
            {
                std::cout << "FLASH SOUND" << std::endl;
            }
            
            if(value & 0x10)
            {
                std::cout << "DEATH SOUND" << std::endl;
            }
            return;
        }
        
        //Set New Shift Data
        case 0x4:
        {
            currentShiftVal = (currentShiftVal >> 8) | (value << 8);
            return;
        }
            
        default:
            return;
    }
}

bool SpaceInvaders::loadGame()
{
    //Memory load locations found at: https://www.emutalk.net/threads/space-invaders.38177/
    
    std::filesystem::path gameFilesDir = "/Users/maxwalley/Documents/Personal_Projects/Intel_8080_Emulator/invaders";
    
    for(int fileIndex = 0; fileIndex < 4; ++fileIndex)
    {
        std::string extensionName;
        int16_t destinationMemoryLocation = 0x0;
        
        switch (fileIndex)
        {
            case 0:
                extensionName = "h";
                destinationMemoryLocation = 0x0;
                break;
                
            case 1:
                extensionName = "g";
                destinationMemoryLocation = 0x800;
                break;
                
            case 2:
                extensionName = "f";
                destinationMemoryLocation = 0x1000;
                break;
                
            case 3:
                extensionName = "e";
                destinationMemoryLocation = 0x1800;
                break;
        }
        
        std::filesystem::path childFileName = gameFilesDir / std::filesystem::path("invaders." + extensionName);
        
        if(const std::filesystem::directory_entry& file{childFileName}; file.exists())
        {
            std::ifstream fileStream(file);
            
            if(!fileStream.is_open())
            {
                return false;
            }
            
            fileStream.unsetf(std::ios_base::skipws);
            
            //Load program into memory
            std::copy(std::istream_iterator<uint8_t>(fileStream), std::istream_iterator<uint8_t>(), memory.begin() + destinationMemoryLocation);
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool SpaceInvaders::loadTest()
{
    std::filesystem::path testFile = "/Users/maxwalley/Documents/Personal_Projects/Intel_8080_Emulator/cpudiag.bin";
    
    std::ifstream fileStream(testFile);
    
    if(!fileStream.is_open())
    {
        return false;
    }
    
    fileStream.unsetf(std::ios_base::skipws);
    
    //Load program into memory
    std::copy(std::istream_iterator<uint8_t>(fileStream), std::istream_iterator<uint8_t>(), memory.begin() + 0x100);
    
    //std::cout << int(memory[368]) << std::endl;
    
    //Fix the stack pointer from 0x6ad to 0x7ad
    // this 0x06 byte 112 in the code, which is
    // byte 112 + 0x100 = 368 in memory
    // memory[368] = 0x7;
    
    programCounter = 0x100;
    
    return true;
}

bool SpaceInvaders::checkKeyDown(uint8_t keycode) const
{
    return std::find(currentlyDownKeys.cbegin(), currentlyDownKeys.cend(), keycode) != currentlyDownKeys.cend();
}
