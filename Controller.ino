#include <WiFi.h>
#include <esp_now.h>

#define JOYSTICK_PIN 34

typedef struct
{
  int steeringAngle;
} ControllerData;

ControllerData data;

// CHANGE THIS TO YOUR CAR ESP32 MAC ADDRESS
uint8_t receiverMAC[] = {
  0x20, 0xE7, 0xC8, 0x68, 0x06, 0x48
};

int centre = 0;

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW Failed");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Calibrating joystick...");
  delay(1000);

  long total = 0;

  for (int i = 0; i < 200; i++)
  {
    total += analogRead(JOYSTICK_PIN);
    delay(5);
  }

  centre = total / 200;

  Serial.print("Joystick Centre = ");
  Serial.println(centre);
}

void loop()
{
  long total = 0;

  for (int i = 0; i < 8; i++)
  {
    total += analogRead(JOYSTICK_PIN);
  }

  int reading = total / 8;

  // Dead zone
  if (abs(reading - centre) < 20)
    reading = centre;

  int angle;

  if (reading >= centre)
  {
    angle = map(reading,
                centre,
                4095,
                90,
                135);
  }
  else
  {
    angle = map(reading,
                0,
                centre,
                45,
                90);
  }

  angle = constrain(angle, 45, 135);

  data.steeringAngle = angle;

  esp_now_send(receiverMAC,
               (uint8_t *)&data,
               sizeof(data));

  Serial.print("ADC: ");
  Serial.print(reading);

  Serial.print("  Angle: ");
  Serial.println(angle);

  delay(20);
}