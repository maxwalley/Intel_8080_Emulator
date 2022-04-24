//
//  main.cpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 09/04/2022.
//

#include "Intel_8080_Emulator.hpp"
#include "SpaceInvaders.hpp"

int main(int argc, const char * argv[])
{
    Intel_8080_Emulator emulator(std::make_unique<SpaceInvaders>());
    
    return 0;
}
