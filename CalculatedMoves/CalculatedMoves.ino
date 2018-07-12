#include <DirectIO.h>
#include "Encoder.h"
#include "cos_fix.h"

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

const uint16_t FREQ = 36;           //36
const float BASE_AMPLITUDE = 0.002; // 0.009

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

int posX = 0;
int posY = 0;
int posZ = 0;
int posA = 0;

int posSnapShotTopX = 0;
int posSnapShotTopY = 0;
int posSnapShotTopZ = 0;
int posSnapShotTopA = 0;

int posSnapShotTopXinit = 0;
int posSnapShotTopYinit = 0;
int posSnapShotTopZinit = 0;
int posSnapShotTopAinit = 0;

int idleCounter = 0;

bool calibrationStepLevel;
bool calibDebugFlag;

bool isInitialMoveToTop = true;
bool idleCycleRequest = false;

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

Output<13> ISRIsActive;

void setup()
{
    Serial.begin(115200);
    Serial.setTimeout(20);
    enablePin = HIGH;
    delay(1000);
    isMovingUpwards = true;
    setDirection();
    delay(1000);

    // - - - - - - - - - - - - - - - - - - -
    // - - - - -  SET UP TIMER 1 - - - - - -
    // - - - - - - - - - - - - - - - - - - -
    noInterrupts(); // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    OCR1A = 16; // compare match register 16MHz/256/4kHz

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
    ISRIsActive = true;

    if (idleCycleRequest)
    {
        idleCounter++;
        if (idleCounter > 50)
        {
            idleCycleRequest = false;
            idleCounter = 0;
        }
    }
    else
    {
        dealWithRequests();
        moveAllUpOrDown();
    }
    ISRIsActive = false;
}

void dealWithRequests()
{
    if (moveUpRequest && plateIsBottomPos)
    {
        moveUpFlag = true;
        isMovingUpwards = true;
        setDirection();

        moveUpRequest = false;
        plateIsBottomPos = false;
    }
    else if (moveDownRequest && plateIsTopPos)
    {
        moveDownFlag = true;
        isMovingUpwards = false;
        setDirection();

        moveDownRequest = false;
        plateIsTopPos = false;
    }
}

void moveAllUpOrDown()
{
    if (idleCycleRequest)
    {
        return;
    }

    if (isStartingUp)
    {
        counter++;
        // Move all steppers up a little on start-up
        int pulseLevel = counter % 100;
        xStepBit = pulseLevel;
        yStepBit = pulseLevel;
        zStepBit = pulseLevel;
        aStepBit = pulseLevel;

        if (counter > 6000)
        {
            // Finished starting up.
            counter = 0;
            isStartingUp = false;
            plateIsBottomPos = true;
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

        int pulseLevel = Encoder1.count % 10;

        if (pulseLevel == 0 && calibDebugFlag)
        {
            calibrationStepLevel = !calibrationStepLevel;
            calibDebugFlag = false;
        }
        if (pulseLevel != 0)
        {
            calibDebugFlag = true;
        }

        xStepBit = calibrationStepLevel;
        yStepBit = calibrationStepLevel;
        zStepBit = calibrationStepLevel;
        aStepBit = calibrationStepLevel;
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
        uint16_t r = counter * FREQ;
        int16_t c = cos_fix(r) + 16384;

        xStepBit = pulseFromAmplitudeX(BASE_AMPLITUDE, c);
        yStepBit = pulseFromAmplitudeY(BASE_AMPLITUDE, c);
        zStepBit = pulseFromAmplitudeZ(BASE_AMPLITUDE, c);
        aStepBit = pulseFromAmplitudeA(BASE_AMPLITUDE, c);

        if (r >= 32768)
        {
            // Finished moving up.
            counter = 0;
            moveUpFlag = false;
            if (isInitialMoveToTop)
            {
                takeInitialPosSnapShot();
                isInitialMoveToTop = false;
            }
            takePosSnapShot();
            plateIsTopPos = true;
            idleCycleRequest = true;
        }
    }
    else if (moveDownFlag)
    {
        counter++;
        uint16_t r = counter * FREQ;
        int16_t c = cos_fix(r) + 16384;

        xStepBit = pulseFromAmplitudeX(xAmplitude, c);
        yStepBit = pulseFromAmplitudeY(yAmplitude, c);
        zStepBit = pulseFromAmplitudeZ(zAmplitude, c);
        aStepBit = pulseFromAmplitudeA(aAmplitude, c);

        if (r >= 32768)
        {
            // Finished moving down.
            counter = 0;
            moveDownFlag = false;
            plateIsBottomPos = true;
            idleCycleRequest = true;
        }
    }
}

void takePosSnapShot()
{
    posSnapShotTopX = posX;
    posSnapShotTopY = posY;
    posSnapShotTopZ = posZ;
    posSnapShotTopA = posA;
}

void takeInitialPosSnapShot()
{
    posSnapShotTopXinit = posX;
    posSnapShotTopYinit = posY;
    posSnapShotTopZinit = posZ;
    posSnapShotTopAinit = posA;
}

void setDirection()
{
    xDirBit = isMovingUpwards;     // X dir bit
    yDirBit = 1 - isMovingUpwards; // Y dir bit
    zDirBit = 1 - isMovingUpwards; // Z dir bit
    aDirBit = isMovingUpwards;     // A dir bit

    idleCycleRequest = true;
}

// unused
int pulseFromAmplitude(float ampl, int16_t c)
{
    int s = (int)(c * ampl);
    return s % 2;
}

int pulseFromAmplitudeX(float ampl, int16_t c)
{
    int s = (int)(c * ampl);
    int res = s % 2;
    if (res > xStepBit)
    {
        // count the positive flank.
        posX += isMovingUpwards ? 1 : -1;
    }
    return res;
}

int pulseFromAmplitudeY(float ampl, int16_t c)
{
    int s = (int)(c * ampl);
    int res = s % 2;
    if (res > yStepBit)
    {
        // count the positive flank.
        posY += isMovingUpwards ? 1 : -1;
    }
    return res;
}

int pulseFromAmplitudeZ(float ampl, int16_t c)
{
    int s = (int)(c * ampl);
    int res = s % 2;
    if (res > zStepBit)
    {
        // count the positive flank.
        posZ += isMovingUpwards ? 1 : -1;
    }
    return res;
}

int pulseFromAmplitudeA(float ampl, int16_t c)
{
    int s = (int)(c * ampl);
    int res = s % 2;
    if (res > aStepBit)
    {
        // count the positive flank.
        posA += isMovingUpwards ? 1 : -1;
    }
    return res;
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
        moveUpRequest = true;
        isMovingUpwards = true;
        setDirection();
        delay(500);
    }

    // DEBUG
    //moveDownRequest = true;
    //moveUpRequest = true;
    // DEBUG

    if (Serial.available() > 0 && !isCalibratingHorizontally && !isCalibratingVertically && isMovingUpwards)
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
            float horizontalCor = h_P; // h_D +
            float verticalCor = v_P;   // v_D +

            // DEBUG
            Serial.print("v cor: ");
            Serial.println(verticalCor);
            Serial.print("h cor: ");
            Serial.println(horizontalCor);
            // DEBUG

            // DEBUG
            Serial.print("posX: ");
            Serial.println(posSnapShotTopX);
            Serial.print("posY: ");
            Serial.println(posSnapShotTopY);
            Serial.print("posZ: ");
            Serial.println(posSnapShotTopZ);
            Serial.print("posA: ");
            Serial.println(posSnapShotTopA);
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

            // - - - BRING VALUES INTO RIGHT RANGE - - -
            xNewCorrection = xNewCorrection * 0.00001;
            yNewCorrection = yNewCorrection * 0.00001;
            zNewCorrection = zNewCorrection * 0.00001;
            aNewCorrection = aNewCorrection * 0.00001;

            // - - - APPLY CORRECTION - - -
            xAmplitude = BASE_AMPLITUDE - xOldCorrection + xNewCorrection;
            yAmplitude = BASE_AMPLITUDE - yOldCorrection + yNewCorrection;
            zAmplitude = BASE_AMPLITUDE - zOldCorrection + zNewCorrection;
            aAmplitude = BASE_AMPLITUDE - aOldCorrection + aNewCorrection;

            // DEBUG
            /*
            xAmplitude += (posSnapShotTopX - posSnapShotTopXinit) * 0.00001;
            yAmplitude += (posSnapShotTopY - posSnapShotTopYinit) * 0.00001;
            zAmplitude += (posSnapShotTopZ - posSnapShotTopZinit) * 0.00001;
            aAmplitude += (posSnapShotTopA - posSnapShotTopAinit) * 0.00001;
            */
            // DEBUG

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
// MEMO
// - implement position monitoring for all axis.
// - clamp result of PD.
// - make random number test.
