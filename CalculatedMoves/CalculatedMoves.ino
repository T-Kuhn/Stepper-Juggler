#include <DirectIO.h>

int counter = 0;
int pulseLevel;
bool isMovingUpwards = true;
bool isStartingUp = true;

bool moveDownRequest = false;
bool moveUpRequest = false;
bool moveUpFlag = true;
bool moveDownFlag = false;

bool plateIsTopPos = false;
bool plateIsBottomPos = false;

const double freq = 0.001f; // value lowered for debug. real value: 0.004f
const double amplitude = 200.0;

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
    enablePin = LOW;
    //enablePin = HIGH;

    // - - - - - - - - - - - - - - - - - - -
    // - - - - -  SET UP TIMER 1 - - - - - -
    // - - - - - - - - - - - - - - - - - - -
    noInterrupts(); // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    OCR1A = 3;               // compare match register 16MHz/256/20kHz
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
    dealWithRequests();
    generatePulsLevels();
    SetOutputs();
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

void generatePulsLevels()
{
    if (isStartingUp)
    {
        // Move all steppers up a little on start-up
        counter++;
        pulseLevel = counter % 100;
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
        double r = counter * freq;
        int s = (int)(amplitude * (cos(r) + 1.0));

        counter++;

        pulseLevel = s % 2;
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
        double r = counter * freq;
        int s = (int)(amplitude * (cos(r) + 1.0));

        counter++;

        pulseLevel = s % 2;
        if (r >= PI)
        {
            // Finished moving down.
            counter = 0;
            moveDownFlag = false;
            plateIsBottomPos = true;
        }
    }
}

void SetOutputs()
{
    xDirBit = 1 - isMovingUpwards; // X dir bit
    yDirBit = isMovingUpwards;     // Y dir bit
    zDirBit = isMovingUpwards;     // Z dir bit
    aDirBit = 1 - isMovingUpwards; // A dir bit

    xStepBit = pulseLevel; // X step bit
    yStepBit = pulseLevel; // Y step bit
    zStepBit = pulseLevel; // Z step bit
    aStepBit = pulseLevel; // A step bit
}

void loop()
{
    while (1)
    {
        if (Serial.available())
        {
            //String command = Serial.readStringUntil(';');
            // clear buffer of any data left after the terminator char.
            Serial.readString();
            // This needs to be fixed.
            //int x = Serial.parseInt();
            //int y = Serial.parseInt();
            // 1. parse
            // 2. pid
            // 3. correction
            // 4. set moveDownRequest instantly (current goal)
            moveDownRequest = true;
            //Serial.println(command);
            // 5. set moveUpRequest after a certain delay (current goal)
            //delay(50);
            moveUpRequest = true;
            //Serial.println("UP");
        }
    }
}
