#include "Encoder.h"

// 9 and 10 are inputs
Encoder Encoder1(9, 10);
int counter = 0;
bool dir = true;
bool isStartingUp = true;

void setup()
{
    Serial.begin(115200);
    pinMode(8, OUTPUT);
    pinMode(2, OUTPUT); // X step bit
    pinMode(3, OUTPUT); // Y step bit
    pinMode(4, OUTPUT); // Z step bit
    pinMode(5, OUTPUT); // X dir bit
    pinMode(6, OUTPUT); // Y dir bit
    pinMode(7, OUTPUT); // Z dir bit

    pinMode(11, OUTPUT); // A dir bit
    pinMode(12, OUTPUT); // A step bit

    digitalWrite(8, 0);

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
    Encoder1.update();
    int pin9VoltageLevel = digitalRead(9);

    counter++;

    if (isStartingUp)
    {
        pin9VoltageLevel = counter % 100;
        if (counter > 10000)
        {
            counter = 0;
            isStartingUp = false;
        }
    }
    else
    {
        double freq = 0.004f;
        double amplitude = 200.0;
        double r = counter * freq;
        int s = (int)(amplitude * (cos(r) + 1.0));

        pin9VoltageLevel = s % 2;
        if (r >= PI)
        {
            counter = 0;
            dir = !dir;
        }
    }

    digitalWrite(5, 1 - dir);  // X dir bit
    digitalWrite(6, dir);      // Y dir bit
    digitalWrite(7, dir);      // Z dir bit
    digitalWrite(11, 1 - dir); // A dir bit

    digitalWrite(2, pin9VoltageLevel);  // X step bit
    digitalWrite(3, pin9VoltageLevel);  // Y step bit
    digitalWrite(4, pin9VoltageLevel);  // Z step bit
    digitalWrite(12, pin9VoltageLevel); // A step bit
}

int calculateCurrentPulseLevel()
{
}

void loop()
{
    //send all 20ms. Thus, 50 times per second.
    delay(20);
    // serialPrintf("val:%d", Encoder1.count);
}

// Printing Values to the Serial port.
// serialPrintf("someInt:%d,someFloat:%f,someString:%s", someInt, someFloat, someString);
int serialPrintf(char *str, ...)
{
    int i, j, count = 0;

    va_list argv;
    va_start(argv, str);
    for (i = 0, j = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '%')
        {
            count++;

            Serial.write(reinterpret_cast<const uint8_t *>(str + j), i - j);

            switch (str[++i])
            {
            case 'd':
                Serial.print(va_arg(argv, int));
                break;
            case 'l':
                Serial.print(va_arg(argv, long));
                break;
            case 'f':
                Serial.print(va_arg(argv, double));
                break;
            case 'c':
                Serial.print((char)va_arg(argv, int));
                break;
            case 's':
                Serial.print(va_arg(argv, char *));
                break;
            case '%':
                Serial.print("%");
                break;
            default:;
            };

            j = i + 1;
        }
    };
    va_end(argv);

    if (i > j)
    {
        Serial.write(reinterpret_cast<const uint8_t *>(str + j), i - j);
    }

    Serial.println("");
    return count;
}
