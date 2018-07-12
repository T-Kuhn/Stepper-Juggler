#include <DirectIO.h>

int mic12DiffCounter = 0;
int mic34DiffCounter = 0;

Output<2> ff_reset;

Input<8> mic1;
Input<9> mic2;
Input<10> mic3;
Input<11> mic4;

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    // DEBUG
    while (1)
    {
        int ranVertical = random(-250, 250);
        int ranHorizontal = random(-250, 250);

        if (random(1, 10) >= 4)
        {
            ranVertical = 0;
        }
        if (random(1, 10) >= 4)
        {
            ranHorizontal = 0;
        }

        Serial.print(constrain(ranVertical, -200, 200));
        Serial.print(":");
        Serial.print(constrain(ranHorizontal, -200, 200));
        Serial.println("&");

        delay(500);
    }
    // DEBUG

    boolean micFlag1 = false;
    boolean micFlag2 = false;
    boolean micFlag3 = false;
    boolean micFlag4 = false;

    while (1)
    {

        while (!micFlag1 || !micFlag2 || !micFlag3 || !micFlag4)
        {
            // update micFlag values.
            // only update the ones which aren't high yet for faster computation speed.
            if (!micFlag1)
                micFlag1 = mic1;
            if (!micFlag2)
                micFlag2 = mic2;
            if (!micFlag3)
                micFlag3 = mic3;
            if (!micFlag4)
                micFlag4 = mic4;

            // track diff between mic 1 and 2
            if (micFlag1 && !micFlag2)
            {
                mic12DiffCounter--;
            }
            if (micFlag2 && !micFlag1)
            {
                mic12DiffCounter++;
            }

            // track diff between mic 2 and 3
            if (micFlag3 && !micFlag4)
            {
                mic34DiffCounter--;
            }
            if (micFlag4 && !micFlag3)
            {
                mic34DiffCounter++;
            }
        }

        Serial.print(constrain(mic12DiffCounter, -200, 200));
        Serial.print(":");
        Serial.print(constrain(mic34DiffCounter, -200, 200));
        Serial.println("&");

        ff_reset = false;
        delay(100);

        // wait until all mics are low again.
        while (micFlag1 || micFlag2 || micFlag3 || micFlag4)
        {
            micFlag1 = mic1;
            micFlag2 = mic2;
            micFlag3 = mic3;
            micFlag4 = mic4;
        }

        ff_reset = true;
        //Serial.println("reset done.");

        mic12DiffCounter = 0;
        mic34DiffCounter = 0;
        micFlag1 = micFlag2 = micFlag3 = micFlag4 = false;
    }
}
