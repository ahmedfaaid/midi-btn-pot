#include <Arduino.h>
#include <MIDI.h>
#include <ResponsiveAnalogRead.h>

// Buttons
const int NUM_BTNS = 3;
const int BTN_PINS[NUM_BTNS] = {2, 3, 4};

int CURR_BTN_ST[NUM_BTNS] = {};
int PREV_BTN_ST[NUM_BTNS] = {};

// Debounce
unsigned long LAST_DEB_TM[NUM_BTNS] = {0};
unsigned long DEB_DELAY = 50;

// Pots
const int NUM_POTS = 1;
const int POT_PINS[NUM_POTS] = {A0};

int CURR_POT_ST[NUM_POTS] = {0};
int PREV_POT_ST[NUM_POTS] = {0};
int POT_ST_DIFF = 0;

int CURR_MIDI_ST[NUM_POTS] = {0};
int PREV_MIDI_ST[NUM_POTS] = {0};

const int TIMEOUT = 300;
const int DIFF_THRESH = 20;
boolean POT_MOVE = true;
unsigned long POT_TIME[NUM_POTS] = {0};
unsigned long TIMER[NUM_POTS] = {0};

int READING = 0;

float SNAP_MULT = 0.01;
ResponsiveAnalogRead RESP_POT[NUM_POTS] = {};

int POT_MIN = 10;
int POT_MAX = 1023;

// MIDI
byte MIDI_CH = 0;
byte NOTE = 36;
byte CC = 1;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println("Debug mode");
  Serial.println();

  for (int i = 0; i < NUM_BTNS; i++)
  {
    pinMode(BTN_PINS[i], INPUT_PULLUP);
  }

  for (int i = 0; i < NUM_POTS; i++)
  {
    RESP_POT[i] = ResponsiveAnalogRead(POT_PINS[i], true, SNAP_MULT);
    RESP_POT[i].setAnalogResolution(1023);
  }
}

void buttons()
{
  for (int i = 0; i < NUM_BTNS; i++)
  {
    CURR_BTN_ST[i] = digitalRead(BTN_PINS[i]);

    if ((millis() - LAST_DEB_TM[i]) > DEB_DELAY)
    {
      if (PREV_BTN_ST[i] != CURR_BTN_ST[i])
      {
        LAST_DEB_TM[i] = millis();

        if (CURR_BTN_ST[i] == LOW)
        {
          Serial.print(i);
          Serial.println(": button on");
        }
        else
        {
          Serial.print(i);
          Serial.println(": button off");
        }

        PREV_BTN_ST[i] = CURR_BTN_ST[i];
      }
    }
  }
}

void pots()
{
  for (int i = 0; i < NUM_POTS; i++)
  {
    READING = analogRead(POT_PINS[i]);
    RESP_POT[i].update(READING);
    CURR_POT_ST[i] = RESP_POT[i].getValue();

    CURR_POT_ST[i] = analogRead(POT_PINS[i]);

    CURR_MIDI_ST[i] = map(CURR_POT_ST[i], POT_MIN, POT_MAX, 0, 127);

    if (CURR_MIDI_ST[i] < 0)
    {
      CURR_MIDI_ST[i] = 0;
    }
    if (CURR_MIDI_ST[i] > 127)
    {
      CURR_MIDI_ST[i] = 0;
    }

    POT_ST_DIFF = abs(CURR_POT_ST[i] - PREV_POT_ST[i]);

    if (POT_ST_DIFF > DIFF_THRESH)
    {
      POT_TIME[i] = millis();
    }

    TIMER[i] = millis();

    if (TIMER[i] < TIMEOUT)
    {
      POT_MOVE = true;
    }
    else
    {
      POT_MOVE = false;
    }

    if (POT_MOVE == true)
    {
      if (PREV_MIDI_ST[i] != CURR_MIDI_ST[i])
      {
        Serial.print("Pot: ");
        Serial.print(i);
        Serial.print(" ");
        Serial.println(CURR_MIDI_ST[i]);

        PREV_POT_ST[i] = CURR_POT_ST[i];
        PREV_MIDI_ST[i] = CURR_MIDI_ST[i];
      }
    }
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  buttons();
  pots();
}
