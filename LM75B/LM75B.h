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

#ifndef LM75B_H
#define LM75B_H

#include "mbed.h"

/** LM75B class.
 *  Used for controlling an LM75B temperature sensor connected via I2C.
 *
 * Example:
 * @code
 * #include "mbed.h"
 * #include "LM75B.h"
 *
 * //Create an LM75B object at the default address (ADDRESS_0)
 * LM75B sensor(p28, p27);
 *
 * int main()
 * {
 *     //Try to open the LM75B
 *     if (sensor.open()) {
 *         printf("Device detected!\n");
 *
 *         while (1) {
 *             //Print the current temperature
 *             printf("Temp = %.3f\n", (float)sensor);
 *
 *             //Sleep for 0.5 seconds
 *             wait(0.5);
 *         }
 *     } else {
 *         error("Device not detected!\n");
 *     }
 * }
 * @endcode
 */
class LM75B
{
public:
    /** Represents the different I2C address possibilities for the LM75B
     */
    enum Address {
        ADDRESS_0 = (0x48 << 1),    /**< A[2:0] pins = 000 */
        ADDRESS_1 = (0x49 << 1),    /**< A[2:0] pins = 001 */
        ADDRESS_2 = (0x4A << 1),    /**< A[2:0] pins = 010 */
        ADDRESS_3 = (0x4B << 1),    /**< A[2:0] pins = 011 */
        ADDRESS_4 = (0x4C << 1),    /**< A[2:0] pins = 100 */
        ADDRESS_5 = (0x4D << 1),    /**< A[2:0] pins = 101 */
        ADDRESS_6 = (0x4E << 1),    /**< A[2:0] pins = 110 */
        ADDRESS_7 = (0x4F << 1)     /**< A[2:0] pins = 111 */
    };

    /** Represents the power mode of the LM75B
     */
    enum PowerMode {
        POWER_NORMAL,   /**< Chip is enabled and samples every 100ms */
        POWER_SHUTDOWN  /**< Chip is in low-power shutdown mode */
    };

    /** Represents OS pin mode of the LM75B
     */
    enum OSMode {
        OS_COMPARATOR,  /**< OS is asserted when the temperature reaches the alert threshold, and de-asserted when the temperature drops below the alert hysteresis threshold */
        OS_INTERRUPT    /**< OS is asserted when the temperature reaches the alert threshold, or drops below the alert hysteresis threshold, and only de-asserted when a register has been read */
    };

    /** Represents OS pin polarity of the LM75B
     */
    enum OSPolarity {
        OS_ACTIVE_LOW,  /**< OS is a logic low when asserted, and a logic high when de-asserted */
        OS_ACTIVE_HIGH  /**< OS is a logic high when asserted, and a logic low when de-asserted */
    };

    /** Represents OS pin fault queue length of the LM75B
     */
    enum OSFaultQueue {
        OS_FAULT_QUEUE_1,   /**< OS is asserted after 1 fault */
        OS_FAULT_QUEUE_2,   /**< OS is asserted after 2 consecutive faults */
        OS_FAULT_QUEUE_4,   /**< OS is asserted after 4 consecutive faults */
        OS_FAULT_QUEUE_6    /**< OS is asserted after 6 consecutive faults */
    };

    /** Create an LM75B object connected to the specified I2C pins with the specified I2C slave address
     *
     * @param sda The I2C data pin.
     * @param scl The I2C clock pin.
     * @param addr The I2C slave address (defaults to ADDRESS_0).
     */
    LM75B(PinName sda, PinName scl, Address addr = ADDRESS_0);

    /** Probe for the LM75B and reset it to default configuration if present
     *
     * @returns
     *   'true' if the device exists on the bus,
     *   'false' if the device doesn't exist on the bus.
     */
    bool open(void);

    /** Get the current power mode of the LM75B
     *
     * @returns The current power mode as a PowerMode enum.
     */
    LM75B::PowerMode powerMode(void);

    /** Set the power mode of the LM75B
     *
     * @param mode The new power mode as a PowerMode enum.
     */
    void powerMode(PowerMode mode);

    /** Get the current OS pin mode of the LM75B
     *
     * @returns The current OS pin mode as an OSMode enum.
     */
    LM75B::OSMode osMode(void);

    /** Set the OS pin mode of the LM75B
     *
     * @param mode The new OS pin mode as an OSMode enum.
     */
    void osMode(OSMode mode);

    /** Get the current OS pin polarity of the LM75B
     *
     * @returns The current OS pin polarity as an OSPolarity enum.
     */
    LM75B::OSPolarity osPolarity(void);

    /** Set the OS pin polarity of the LM75B
     *
     * @param polarity The new OS pin polarity as an OSPolarity enum.
     */
    void osPolarity(OSPolarity polarity);

    /** Get the current OS pin fault queue length of the LM75B
     *
     * @returns The current OS pin fault queue length as an OSFaultQueue enum.
     */
    LM75B::OSFaultQueue osFaultQueue(void);

    /** Set the OS pin fault queue length of the LM75B
     *
     * @param queue The new OS pin fault queue length as an OSFaultQueue enum.
     */
    void osFaultQueue(OSFaultQueue queue);

    /** Get the current alert temperature threshold of the LM75B
     *
     * @returns The current alert temperature threshold in °C.
     */
    float alertTemp(void);

    /** Set the alert temperature threshold of the LM75B
     *
     * @param temp The new alert temperature threshold in °C.
     */
    void alertTemp(float temp);

    /** Get the current alert temperature hysteresis threshold of the LM75B
     *
     * @returns The current alert temperature hysteresis threshold in °C.
     */
    float alertHyst(void);

    /** Set the alert temperature hysteresis threshold of the LM75B
     *
     * @param temp The new alert temperature hysteresis threshold in °C.
     */
    void alertHyst(float temp);

    /** Get the current temperature measurement of the LM75B
     *
     * @returns The current temperature measurement in °C.
     */
    float temp(void);

#ifdef MBED_OPERATORS
    /** A shorthand for temp()
     *
     * @returns The current temperature measurement in °C.
     */
    operator float() {
        return temp();
    }
#endif

private:
    //I2C register addresses
    enum Register {
        REG_TEMP    = 0x00,
        REG_CONF    = 0x01,
        REG_THYST   = 0x02,
        REG_TOS     = 0x03
    };

    //Member variables
    I2C m_I2C;
    int m_Addr;

    //Internal functions
    char read8(char reg);
    void write8(char reg, char data);
    unsigned short read16(char reg);
    void write16(char reg, unsigned short data);
    float readAlertTempHelper(char reg);
    void writeAlertTempHelper(char reg, float temp);
};

#endif
