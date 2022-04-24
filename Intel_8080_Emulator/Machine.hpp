//
//  Machine.hpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 24/04/2022.
//

#pragma once

#include <cstdint>

class Machine
{
public:
    Machine() {};
    virtual ~Machine() {};
    
    virtual uint8_t inputOperation(uint8_t port)=0;
    virtual void outputOperation(uint8_t port, uint8_t value)=0;
};
