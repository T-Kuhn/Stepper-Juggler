#include <DirectIO.h>

int micDiffCounter = 0;

Input<2> mic1;
Input<3> mic2;

void setup()
{
    Serial.begin(115200);
    Serial.println("start.");
}

void loop()
{
    int currentMic1, currentMic2;

    while (!currentMic1 && !currentMic2)
    {
        currentMic1 = mic1;
        currentMic2 = mic2;
    }
    while (!currentMic1)
    {
        currentMic1 = mic1;
        micDiffCounter--;
    }
    while (!currentMic2)
    {
        currentMic2 = mic2;
        micDiffCounter++;
    }

    Serial.print("mic diff: ");
    Serial.println(micDiffCounter);

    while (currentMic1 || currentMic2)
    {
        currentMic1 = mic1;
        currentMic2 = mic2;
    }

    micDiffCounter = 0;
}
