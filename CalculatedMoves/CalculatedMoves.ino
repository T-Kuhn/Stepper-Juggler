#include <DirectIO.h>
#include "Encoder.h"

// 9 and 10 are inputs
Encoder Encoder1(9, 10);

// Max input size expected for one command
#define INPUT_SIZE 16

int counter = 0;
bool isMovingUpwards = true;
bool isStartingUp = true;
bool isCalibratingHorizontally = true;
bool isCalibratingVertically = true;

bool moveDownRequest = false;
bool moveUpRequest = false;
bool moveUpFlag = true;
bool moveDownFlag = false;

bool plateIsTopPos = false;
bool plateIsBottomPos = false;

const float FREQ = 0.0055f;
const float BASE_AMPLITUDE = 170.0;

float xAmplitude = BASE_AMPLITUDE;
float yAmplitude = BASE_AMPLITUDE;
float zAmplitude = BASE_AMPLITUDE;
float aAmplitude = BASE_AMPLITUDE;

float oldVertical = 0.0;
float oldHorizontal = 0.0;

float xOldCorrection = 0.0;
float yOldCorrection = 0.0;
float zOldCorrection = 0.0;
float aOldCorrection = 0.0;

float xNewCorrection = 0.0;
float yNewCorrection = 0.0;
float zNewCorrection = 0.0;
float aNewCorrection = 0.0;

Input<A0> button;

Output<8> enablePin;

Output<2> xStepBit;
Output<3> yStepBit;
Output<4> zStepBit;
Output<12> aStepBit;

Output<5> xDirBit;
Output<6> yDirBit;
Output<7> zDirBit;
Output<11> aDirBit;

void setup()
{
    Serial.begin(115200);
    Serial.setTimeout(20);
    enablePin = HIGH;
    delay(2000);

    // - - - - - - - - - - - - - - - - - - -
    // - - - - -  SET UP TIMER 1 - - - - - -
    // - - - - - - - - - - - - - - - - - - -
    noInterrupts(); // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    OCR1A = 16; // compare match register 16MHz/256/4kHz
    //OCR1A = 3;// compare match register 16MHz/256/20kHz
    TCCR1B |= (1 << WGM12);  // CTC mode
    TCCR1B |= (1 << CS12);   // 256 prescaler
    TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt

    interrupts(); // enable all interrupts
}

// - - - - - - - - - - - - - - - - - - -
// - - TIMER 1 INTERRUPT FUNCTION  - - -
// - - - - - every 50 us - - - - - - - -
// - - - - - - - - - - - - - - - - - - -
ISR(TIMER1_COMPA_vect)
{
    interrupts();

    dealWithRequests();
    moveAllUpOrDown();
}

void dealWithRequests()
{
    if (moveUpRequest && plateIsBottomPos)
    {
        moveUpFlag = true;
        isMovingUpwards = true;

        moveUpRequest = false;
        plateIsBottomPos = false;
    }
    else if (moveDownRequest && plateIsTopPos)
    {
        moveDownFlag = true;
        isMovingUpwards = false;

        moveDownRequest = false;
        plateIsTopPos = false;
    }
}

void moveAllUpOrDown()
{
    if (isStartingUp)
    {
        counter++;
        // Move all steppers up a little on start-up
        int pulseLevel = counter % 100;

        xStepBit = pulseLevel;
        yStepBit = pulseLevel;
        zStepBit = pulseLevel;
        aStepBit = pulseLevel;

        setDirection();

        if (counter > 12000)
        {
            // Finished starting up.
            counter = 0;
            isStartingUp = false;
            plateIsBottomPos = true;
            moveUpRequest = true;
        }
    }
    else if (isCalibratingHorizontally)
    {
        Encoder1.update();

        int rotDir = Encoder1.currentRot;
        xDirBit = 1 - rotDir; // X dir bit
        yDirBit = rotDir;     // Y dir bit
        zDirBit = 1 - rotDir; // Z dir bit
        aDirBit = rotDir;     // A dir bit

        int pulseLevel = Encoder1.count % 4;
        xStepBit = pulseLevel;
        yStepBit = pulseLevel;
        zStepBit = pulseLevel;
        aStepBit = pulseLevel;
    }
    else if (isCalibratingVertically)
    {
        Encoder1.update();

        int rotDir = Encoder1.currentRot;
        xDirBit = rotDir;     // X dir bit
        yDirBit = rotDir;     // Y dir bit
        zDirBit = 1 - rotDir; // Z dir bit
        aDirBit = 1 - rotDir; // A dir bit

        int pulseLevel = Encoder1.count % 4;
        xStepBit = pulseLevel;
        yStepBit = pulseLevel;
        zStepBit = pulseLevel;
        aStepBit = pulseLevel;
    }
    else if (moveUpFlag)
    {
        counter++;
        float r = counter * FREQ;
        float c = cos(r) + 1.0;

        int pulseLevel = pulseFromAmplitude(BASE_AMPLITUDE, c);
        xStepBit = pulseLevel;
        yStepBit = pulseLevel;
        zStepBit = pulseLevel;
        aStepBit = pulseLevel;

        setDirection();

        if (r >= PI)
        {
            // Finished moving up.
            counter = 0;
            moveUpFlag = false;
            plateIsTopPos = true;
        }
    }
    else if (moveDownFlag)
    {
        counter++;
        float r = counter * FREQ;
        float c = cos(r) + 1.0;

        xStepBit = pulseFromAmplitude(xAmplitude, c);
        yStepBit = pulseFromAmplitude(yAmplitude, c);
        zStepBit = pulseFromAmplitude(zAmplitude, c);
        aStepBit = pulseFromAmplitude(aAmplitude, c);

        setDirection();

        if (r >= PI)
        {
            // Finished moving down.
            counter = 0;
            moveDownFlag = false;
            plateIsBottomPos = true;
        }
    }
}

void setDirection()
{
    xDirBit = isMovingUpwards;     // X dir bit
    yDirBit = 1 - isMovingUpwards; // Y dir bit
    zDirBit = 1 - isMovingUpwards; // Z dir bit
    aDirBit = isMovingUpwards;     // A dir bit
}

int pulseFromAmplitude(float ampl, float c)
{
    int s = (int)(ampl * c);
    return s % 2;
}

void loop()
{
    if (!button && isCalibratingHorizontally)
    {
        isCalibratingHorizontally = false;
        delay(500);
    }
    if (!button && isCalibratingVertically)
    {
        isCalibratingVertically = false;
        delay(500);
    }

    if (Serial.available() > 0 && !isCalibratingHorizontally && !isCalibratingVertically)
    {
        // Get next command from Serial (add 1 for final 0)
        char input[INPUT_SIZE + 1];
        byte size = Serial.readBytes(input, INPUT_SIZE);
        // Add the final 0 to end the C string
        input[size] = 0;

        // Read command pair
        char *command = strtok(input, "&");
        // Split the command in two values
        char *separator = strchr(command, ':');
        if (separator != 0)
        {
            // Actually split the string in 2: replace ':' with 0
            *separator = 0;
            int ver = atoi(command);
            ++separator;
            int hor = atoi(separator);

            float horizontal = (float)hor;
            float vertical = (float)ver;

            // DEBUG
            Serial.print("v: ");
            Serial.println(ver);
            Serial.print("h: ");
            Serial.println(hor);
            // DEBUG

            // - - - PD - - -
            // - - - P
            float h_P = constrain(horizontal / 6.0, -20.0, 20.0);
            float v_P = constrain(vertical / 6.0, -20.0, 20.0);

            // - - - PD - - -
            // - - - D
            float h_D = constrain((horizontal - oldHorizontal) / 2, -20.0, 20.0);
            float v_D = constrain((vertical - oldVertical) / 2, -20.0, 20.0);
            oldHorizontal = horizontal;
            oldVertical = vertical;

            // - - - PID - - -
            // - - - ADD THEM TOGETHER
            float horizontalCor = -h_P - h_D;
            float verticalCor = -v_P - v_D;

            // DEBUG
            Serial.print("v cor: ");
            Serial.println(verticalCor);
            Serial.print("h cor: ");
            Serial.println(horizontalCor);
            // DEBUG

            // - - - ADD UP CORRECTION - - -
            // horizontal
            xNewCorrection = -horizontalCor;
            yNewCorrection = -horizontalCor;

            zNewCorrection = horizontalCor;
            aNewCorrection = horizontalCor;
            // vertical
            aNewCorrection -= verticalCor;
            yNewCorrection -= verticalCor;

            xNewCorrection += verticalCor;
            zNewCorrection += verticalCor;

            // - - - APPLY CORRECTION - - -
            xAmplitude = BASE_AMPLITUDE - xOldCorrection + xNewCorrection;
            yAmplitude = BASE_AMPLITUDE - yOldCorrection + yNewCorrection;
            zAmplitude = BASE_AMPLITUDE - zOldCorrection + zNewCorrection;
            aAmplitude = BASE_AMPLITUDE - aOldCorrection + aNewCorrection;

            xOldCorrection = xNewCorrection;
            yOldCorrection = yNewCorrection;
            zOldCorrection = zNewCorrection;
            aOldCorrection = aNewCorrection;

            moveDownRequest = true;
            delay(200);
            moveUpRequest = true;
        }
    }
}
