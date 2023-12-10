/**
 * @file encoder.ino
 * @author SeanKwok (shaoxiang@m5stack.com)
 * @brief M5Dial Encoder Test
 * @version 0.2
 * @date 2023-10-18
 *
 *
 * @Hardwares: M5Dial
 * @Platform Version: Arduino M5Stack Board Manager v2.0.7
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 */

#include "M5Dial.h"
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("AFIndustries_VC", "AFIndustries", 100);

static m5::touch_state_t prevTouchState;

long encoderNewValue = 0;
long encoderPreviousValue = 0;
unsigned long lastInteractionTimeStamp = 0;
bool bleConnectedStatus = true;

// Define the minimum time interval since last rotation before turning off in milliseconds
const unsigned long minTimeBetweenMovesToTurnOff = 60000;

void setup()
{
  Serial.begin(9600);
  Serial.println("running set up...");
  pinMode(G46, INPUT_PULLUP);
  // gpio_deep_sleep_hold_en();
  // gpio_hold_en(GPIO_NUM_46);
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  M5Dial.Display.setTextColor(GREENYELLOW);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.setTextFont(&fonts::Roboto_Thin_24);
  M5Dial.Display.setTextSize(1);
  bleKeyboard.begin();
}

void loop()
{
  Serial.println(F("looping..."));
  M5Dial.update();

  if (bleConnectedStatus != bleKeyboard.isConnected())
  {
    M5Dial.Display.clear();
    bleConnectedStatus = bleKeyboard.isConnected();
    if (bleConnectedStatus)
    {
      M5Dial.Display.drawString(F("Connected"),
                                M5Dial.Display.width() / 2,
                                M5Dial.Display.height() / 3);
    }
    else
    {
      M5Dial.Display.drawString(F("Disconnected"),
                                M5Dial.Display.width() / 2,
                                M5Dial.Display.height() / 3);
    }
  }

  auto t = M5Dial.Touch.getDetail();
  if (prevTouchState != t.state)
  {
    M5Dial.Display.wakeup();
    lastInteractionTimeStamp = millis();
    prevTouchState = t.state;
    static constexpr const char *state_name[16] = {
        "none", "touch", "touch_end", "touch_begin",
        "___", "hold", "hold_end", "hold_begin",
        "___", "flick", "flick_end", "flick_begin",
        "___", "drag", "drag_end", "drag_begin"};

    if (state_name[t.state] == state_name[2] || state_name[t.state] == state_name[6])
    {
      M5Dial.Speaker.tone(8000, 20);
      if (bleConnectedStatus)
      {
        bleKeyboard.write(KEY_MEDIA_MUTE);
      }
    }
  }

  if (M5Dial.BtnA.wasPressed())
  {
    M5Dial.Display.wakeup();
    M5Dial.Speaker.tone(6000, 20);
    if (bleConnectedStatus)
    {
      bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
    }
    lastInteractionTimeStamp = millis();
  }

  encoderNewValue = M5Dial.Encoder.read();
  long encoderIncrement = encoderNewValue - encoderPreviousValue;
  encoderPreviousValue = encoderNewValue;
  if (encoderIncrement != 0)
  {
    M5Dial.Display.wakeup();
    int tone = encoderIncrement > 0 ? 4000 : 10000;
    const uint8_t *keyToPress = encoderIncrement > 0 ? KEY_MEDIA_VOLUME_UP : KEY_MEDIA_VOLUME_DOWN;
    encoderIncrement = min(abs(encoderIncrement), 100L);

    while (encoderIncrement > 0)
    {
      encoderIncrement--;
      M5Dial.Speaker.tone(tone, 20);
      if (bleConnectedStatus)
      {
        bleKeyboard.write(keyToPress);
      }
    }
    lastInteractionTimeStamp = millis();
  }

  if (millis() - lastInteractionTimeStamp >= minTimeBetweenMovesToTurnOff)
  {
    Serial.println(F("Going to sleep..."));
    // M5Dial.Power.lightSleep();
    Serial.println(F("back from sleep in same location..."));

    // M5Dial.Power.lightSleep(minTimeBetweenMovesToTurnOff  * 1000, true);
    M5Dial.Display.sleep();
  }
}
