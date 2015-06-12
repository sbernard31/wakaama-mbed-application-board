/* LM75B Driver Library
 * Copyright (c) 2013 Neil Thiessen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "LM75B.h"

LM75B::LM75B(PinName sda, PinName scl, Address addr) : m_I2C(sda, scl)
{
    //Set the internal device address
    m_Addr = (int)addr;
}

bool LM75B::open(void)
{
    //Probe for the LM75B using a Zero Length Transfer
    if (!m_I2C.write(m_Addr, NULL, 0)) {
        //Reset the device to default configuration
        write8(REG_CONF, 0x00);
        write16(REG_THYST, 0x4B00);
        write16(REG_TOS, 0x5000);

        //Return success
        return true;
    } else {
        //Return failure
        return false;
    }
}

LM75B::PowerMode LM75B::powerMode(void)
{
    //Read the 8-bit register value
    char value = read8(REG_CONF);

    //Return the status of the SHUTDOWN bit
    if (value & (1 << 0))
        return POWER_SHUTDOWN;
    else
        return POWER_NORMAL;
}

void LM75B::powerMode(PowerMode mode)
{
    //Read the current 8-bit register value
    char value = read8(REG_CONF);

    //Set or clear the SHUTDOWN bit
    if (mode == POWER_SHUTDOWN)
        value |= (1 << 0);
    else
        value &= ~(1 << 0);

    //Write the value back out
    write8(REG_CONF, value);
}

LM75B::OSMode LM75B::osMode(void)
{
    //Read the 8-bit register value
    char value = read8(REG_CONF);

    //Return the status of the OS_COMP_INT bit
    if (value & (1 << 1))
        return OS_INTERRUPT;
    else
        return OS_COMPARATOR;
}

void LM75B::osMode(OSMode mode)
{
    //Read the current 8-bit register value
    char value = read8(REG_CONF);

    //Set or clear the OS_COMP_INT bit
    if (mode == OS_INTERRUPT)
        value |= (1 << 1);
    else
        value &= ~(1 << 1);

    //Write the value back out
    write8(REG_CONF, value);
}

LM75B::OSPolarity LM75B::osPolarity(void)
{
    //Read the 8-bit register value
    char value = read8(REG_CONF);

    //Return the status of the OS_POL bit
    if (value & (1 << 2))
        return OS_ACTIVE_HIGH;
    else
        return OS_ACTIVE_LOW;
}

void LM75B::osPolarity(OSPolarity polarity)
{
    //Read the current 8-bit register value
    char value = read8(REG_CONF);

    //Set or clear the OS_POL bit
    if (polarity == OS_ACTIVE_HIGH)
        value |= (1 << 2);
    else
        value &= ~(1 << 2);

    //Write the value back out
    write8(REG_CONF, value);
}

LM75B::OSFaultQueue LM75B::osFaultQueue(void)
{
    //Read the 8-bit register value
    char value = read8(REG_CONF);

    //Return the status of the OS_F_QUE bits
    if ((value & (1 << 3)) && (value & (1 << 4)))
        return OS_FAULT_QUEUE_6;
    else if (!(value & (1 << 3)) && (value & (1 << 4)))
        return OS_FAULT_QUEUE_4;
    else if ((value & (1 << 3)) && !(value & (1 << 4)))
        return OS_FAULT_QUEUE_2;
    else
        return OS_FAULT_QUEUE_1;
}

void LM75B::osFaultQueue(OSFaultQueue queue)
{
    //Read the current 8-bit register value
    char value = read8(REG_CONF);

    //Clear the old OS_F_QUE bits
    value &= ~(3 << 3);

    //Set the new OS_F_QUE bits
    if (queue == OS_FAULT_QUEUE_2)
        value |= (1 << 3);
    else if (queue == OS_FAULT_QUEUE_4)
        value |= (2 << 3);
    else if (queue == OS_FAULT_QUEUE_6)
        value |= (3 << 3);

    //Write the value back out
    write8(REG_CONF, value);
}

float LM75B::alertTemp(void)
{
    //Use the 9-bit helper to read the TOS register
    return readAlertTempHelper(REG_TOS);
}

void LM75B::alertTemp(float temp)
{
    //Use the 9-bit helper to write to the TOS register
    return writeAlertTempHelper(REG_TOS, temp);
}

float LM75B::alertHyst(void)
{
    //Use the 9-bit helper to read the THYST register
    return readAlertTempHelper(REG_THYST);
}

void LM75B::alertHyst(float temp)
{
    //Use the 9-bit helper to write to the THYST register
    return writeAlertTempHelper(REG_THYST, temp);
}

float LM75B::temp(void)
{
    //Signed return value
    short value;

    //Read the 11-bit raw temperature value
    value = read16(REG_TEMP) >> 5;

    //Sign extend negative numbers
    if (value & (1 << 10))
        value |= 0xFC00;

    //Return the temperature in °C
    return value * 0.125;
}

char LM75B::read8(char reg)
{
    //Select the register
    m_I2C.write(m_Addr, &reg, 1);

    //Read the 8-bit register
    m_I2C.read(m_Addr, &reg, 1);

    //Return the byte
    return reg;
}

void LM75B::write8(char reg, char data)
{
    //Create a temporary buffer
    char buff[2];

    //Load the register address and 8-bit data
    buff[0] = reg;
    buff[1] = data;

    //Write the data
    m_I2C.write(m_Addr, buff, 2);
}

unsigned short LM75B::read16(char reg)
{
    //Create a temporary buffer
    char buff[2];

    //Select the register
    m_I2C.write(m_Addr, &reg, 1);

    //Read the 16-bit register
    m_I2C.read(m_Addr, buff, 2);

    //Return the combined 16-bit value
    return (buff[0] << 8) | buff[1];
}

void LM75B::write16(char reg, unsigned short data)
{
    //Create a temporary buffer
    char buff[3];

    //Load the register address and 16-bit data
    buff[0] = reg;
    buff[1] = data >> 8;
    buff[2] = data;

    //Write the data
    m_I2C.write(m_Addr, buff, 3);
}

float LM75B::readAlertTempHelper(char reg)
{
    //Signed return value
    short value;

    //Read the 9-bit raw temperature value
    value = read16(reg) >> 7;

    //Sign extend negative numbers
    if (value & (1 << 8))
        value |= 0xFF00;

    //Return the temperature in °C
    return value * 0.5;
}

void LM75B::writeAlertTempHelper(char reg, float temp)
{
    //Range limit temp
    if (temp < -55.0)
        temp = -55.0;
    else if (temp > 125.0)
        temp = 125.0;

    //Extract and shift the signed integer
    short value = temp * 2;
    value <<= 7;

    //Send the new value
    write16(reg, value);
}
