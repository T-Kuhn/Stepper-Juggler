#include "Encoder.h"

// 9 and 10 are inputs
Encoder Encoder1(9, 10);
int counter= 0;

void setup() {
  Serial.begin(115200);
  pinMode(8, OUTPUT);
  pinMode(2, OUTPUT); // X step bit
  pinMode(3, OUTPUT); // Y step bit
  pinMode(4, OUTPUT); // Z step bit
  pinMode(5, OUTPUT); // X dir bit
  pinMode(6, OUTPUT); // Y dir bit
  pinMode(7, OUTPUT); // Z dir bit
  pinMode(12, INPUT); // Z limit bit
  digitalWrite(8, 0);

  // - - - - - - - - - - - - - - - - - - -
  // - - - - -  SET UP TIMER 1 - - - - - -
  // - - - - - - - - - - - - - - - - - - - 
  noInterrupts();               // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 3;                   // compare match register 16MHz/256/20kHz
  TCCR1B |= (1 << WGM12);      // CTC mode
  TCCR1B |= (1 << CS12);       // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);     // enable timer compare interrupt

  interrupts();                // enable all interrupts
}

// - - - - - - - - - - - - - - - - - - -
// - - TIMER 1 INTERRUPT FUNCTION  - - -
// - - - - - every 60 us - - - - - - - -
// - - - - - - - - - - - - - - - - - - - 
ISR(TIMER1_COMPA_vect)
{
  Encoder1.update();
  
  digitalWrite(5, Encoder1.currentRot); // X dir bit
  digitalWrite(6, Encoder1.currentRot); // Y dir bit
  //digitalWrite(7, Encoder1.currentRot);  // Z dir bit
  counter++;
      
  int pin9VoltageLevel;
  if(Encoder1.count < 1200)
  {
    pin9VoltageLevel = digitalRead(9);
  }
  else
  {
    pin9VoltageLevel = counter%20;
  }

    
  digitalWrite(2, pin9VoltageLevel); // X step bit
  digitalWrite(3, pin9VoltageLevel); // Y step bit
  //digitalWrite(4, pin9VoltageLevel); // Z step bit
  
  //digitalWrite(2, Encoder1.count%2);
}

void loop() {
  serialPrintf("val:%d", Encoder1.count);
      
  //send all 20ms. Thus, 50 times per second.
  delay(20);
}

// Printing Values to the Serial port.
// serialPrintf("someInt:%d,someFloat:%f,someString:%s", someInt, someFloat, someString);
int serialPrintf(char *str, ...) {
    int i, j, count = 0;

    va_list argv;
    va_start(argv, str);
    for(i = 0, j = 0; str[i] != '\0'; i++) {
        if (str[i] == '%') {
            count++;

            Serial.write(reinterpret_cast<const uint8_t*>(str+j), i-j);

            switch (str[++i]) {
                case 'd': Serial.print(va_arg(argv, int));
                    break;
                case 'l': Serial.print(va_arg(argv, long));
                    break;
                case 'f': Serial.print(va_arg(argv, double));
                    break;
                case 'c': Serial.print((char) va_arg(argv, int));
                    break;
                case 's': Serial.print(va_arg(argv, char *));
                    break;
                case '%': Serial.print("%");
                    break;
                default:;
            };

            j = i+1;
        }
    };
    va_end(argv);

    if(i > j) {
        Serial.write(reinterpret_cast<const uint8_t*>(str+j), i-j);
    }

    Serial.println("");
    return count;
}
