#include <DirectIO.h>

// Max input size expected for one command
#define INPUT_SIZE 16

int counter = 0;
bool isMovingUpwards = true;
bool isStartingUp = true;

bool moveDownRequest = false;
bool moveUpRequest = false;
bool moveUpFlag = true;
bool moveDownFlag = false;

bool plateIsTopPos = false;
bool plateIsBottomPos = false;

const float freq = 0.004f; // value lowered for debug. real value: 0.004f
const float baseAmplitude = 200.0;

float xAmplitude = 200.0;
float yAmplitude = 200.0;
float zAmplitude = 200.0;
float aAmplitude = 200.0;

float xOldCorrection = 0.0;
float yOldCorrection = 0.0;
float zOldCorrection = 0.0;
float aOldCorrection = 0.0;

float xNewCorrection = 0.0;
float yNewCorrection = 0.0;
float zNewCorrection = 0.0;
float aNewCorrection = 0.0;

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
    enablePin = LOW;

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

    xDirBit = 1 - isMovingUpwards; // X dir bit
    yDirBit = isMovingUpwards;     // Y dir bit
    zDirBit = isMovingUpwards;     // Z dir bit
    aDirBit = 1 - isMovingUpwards; // A dir bit
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

        if (counter > 10000)
        {
            // Finished starting up.
            counter = 0;
            isStartingUp = false;
            plateIsBottomPos = true;
            moveUpRequest = true;
        }
    }
    else if (moveUpFlag)
    {
        counter++;
        float r = counter * freq;
        float c = cos(r) + 1.0;

        int pulseLevel = pulseFromAmplitude(baseAmplitude, c);
        xStepBit = pulseLevel;
        yStepBit = pulseLevel;
        zStepBit = pulseLevel;
        aStepBit = pulseLevel;

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
        float r = counter * freq;
        float c = cos(r) + 1.0;

        xStepBit = pulseFromAmplitude(xAmplitude, c);
        yStepBit = pulseFromAmplitude(yAmplitude, c);
        zStepBit = pulseFromAmplitude(zAmplitude, c);
        aStepBit = pulseFromAmplitude(aAmplitude, c);

        if (r >= PI)
        {
            // Finished moving down.
            counter = 0;
            moveDownFlag = false;
            plateIsBottomPos = true;
        }
    }
}

int pulseFromAmplitude(float ampl, float c)
{
    int s = (int)(ampl * c);
    return s % 2;
}

void loop()
{
    // DEBUG
    moveDownRequest = true;
    moveUpRequest = true;
    // DEBUG

    if (Serial.available() > 0)
    {
        // Get next command from Serial (add 1 for final 0)
        char input[INPUT_SIZE + 1];
        byte size = Serial.readBytes(input, INPUT_SIZE);
        // Add the final 0 to end the C string
        input[size] = 0;

        // Read each command pair
        char *command = strtok(input, "&");
        // Split the command in two values
        char *separator = strchr(command, ':');
        if (separator != 0)
        {
            // Actually split the string in 2: replace ':' with 0
            *separator = 0;
            int x = atoi(command);
            ++separator;
            int y = atoi(separator);
            Serial.println(x);
            Serial.println(y);

            // Correcting things.
            xAmplitude = baseAmplitude - xOldCorrection + xNewCorrection;
            yAmplitude = baseAmplitude - yOldCorrection + yNewCorrection;
            zAmplitude = baseAmplitude - zOldCorrection + zNewCorrection;
            aAmplitude = baseAmplitude - aOldCorrection + aNewCorrection;

            xOldCorrection = xNewCorrection;
            yOldCorrection = yNewCorrection;
            zOldCorrection = zNewCorrection;
            aOldCorrection = aNewCorrection;

            moveDownRequest = true;
            //delay(100);
            moveUpRequest = true;
        }
    }
}
