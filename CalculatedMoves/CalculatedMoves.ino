#include <DirectIO.h>

int counter = 0;
int pulseLevel;
bool dir = true;
bool isStartingUp = true;
const double freq = 0.004f;
const double amplitude = 200.0;

Output<14> pin;

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
    enablePin = LOW;

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
// - - - - - every 60 us - - - - - - - -
// - - - - - - - - - - - - - - - - - - -
ISR(TIMER1_COMPA_vect)
{
    counter++;

    if (isStartingUp)
    {
        pulseLevel = counter % 100;
        if (counter > 10000)
        {
            counter = 0;
            isStartingUp = false;
        }
    }
    else
    {
        double r = counter * freq;
        int s = (int)(amplitude * (cos(r) + 1.0));

        pulseLevel = s % 2;
        if (r >= PI)
        {
            counter = 0;
            dir = !dir;
        }
    }

    xDirBit = 1 - dir; // X dir bit
    yDirBit = dir;     // Y dir bit
    zDirBit = dir;     // Z dir bit
    aDirBit = 1 - dir; // A dir bit

    xStepBit = pulseLevel; // X step bit
    yStepBit = pulseLevel; // Y step bit
    zStepBit = pulseLevel; // Z step bit
    aStepBit = pulseLevel; // A step bit
}

void loop()
{
    while (1)
    {
        pin = HIGH;
        pin = LOW;
    }
}
