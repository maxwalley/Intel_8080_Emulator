//
//  SpaceInvaders.hpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 24/04/2022.
//

#pragma once

#include "Intel_8080_Emulator.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

class SpaceInvaders  : public Intel_8080_Emulator
{
public:
    SpaceInvaders();
    ~SpaceInvaders() override;
    
    void run();
    
    void triggerKeyDown(int keycode, int x, int y);
    void triggerKeyUp(int keycode, int x, int y);
    
private:
    uint8_t inputOperation(uint8_t port) override;
    void outputOperation(uint8_t port, uint8_t value) override;
    
    bool loadGame();
    bool loadTest();
    
    bool checkKeyDown(uint8_t keycode) const;
    
    uint8_t currentShiftOffset = 0x0;
    uint16_t currentShiftVal = 0x0;
    
    std::vector<int> currentlyDownKeys;
    
    static constexpr double halfFrameTimeMS = 1000.0 / 60.0;
    std::chrono::steady_clock::time_point lastHalfFrameTime;
    bool onFirstHalf;
};
