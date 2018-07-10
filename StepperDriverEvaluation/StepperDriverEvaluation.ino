/*
 * Simple demo, should work with any driver board
 *
 * Connect STEP, DIR as indicated
 *
 * Copyright (C)2015-2017 Laurentiu Badea
 *
 * This file may be redistributed under the terms of the MIT license.
 * A copy of this license has been included with this distribution in the file LICENSE.
 */
#include <Arduino.h>
#include "BasicStepperDriver.h"
#include <DirectIO.h>

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 120

// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 1=full step, 2=half step etc.
#define MICROSTEPS 16

Output<8> enablePin;

//Output<2> xStepBit;
Output<3> yStepBit;
Output<4> zStepBit;
Output<12> aStepBit;

//Output<5> xDirBit;
Output<6> yDirBit;
Output<7> zDirBit;
Output<11> aDirBit;

// All the wires needed for full functionality
#define DIR 5
#define STEP 2
//Uncomment line to use enable/disable functionality
//#define ENABLE 13

// 2-wire basic config, microstepping is hardwired on the driver
BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);

//Uncomment line to use enable/disable functionality
//BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP, ENABLE);

void setup()
{
    Serial.begin(115200);
    Serial.setTimeout(20);

    //xStepBit = LOW;
    yStepBit = LOW;
    zStepBit = LOW;
    aStepBit = LOW;

    //xDirBit = LOW;
    yDirBit = LOW;
    zDirBit = LOW;
    aDirBit = LOW;

    enablePin = HIGH;
    stepper.begin(RPM, MICROSTEPS);
}

void loop()
{

    // energize coils - the motor will hold position
    // stepper.enable();

    /*
     * Moving motor one full revolution using the degree notation
     */
    stepper.rotate(360);

    /*
     * Moving motor to original position using steps
     */
    stepper.move(-MOTOR_STEPS * MICROSTEPS);

    // pause and allow the motor to be moved by hand
    // stepper.disable();

    delay(5000);
    Serial.println("did one rot.");
}
