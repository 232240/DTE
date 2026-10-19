#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

#define SERVO_PIN 18

Servo steering;

typedef struct
{
  int steeringAngle;
} ControllerData;

ControllerData data = {90};

float currentAngle = 90;

bool receivedPacket = false;

void onReceive(const esp_now_recv_info *info,
               const uint8_t *incomingData,
               int len)
{
  memcpy(&data, incomingData, sizeof(data));

  receivedPacket = true;
}

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW Failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);

  steering.setPeriodHertz(50);

  steering.attach(SERVO_PIN, 500, 2500);

  steering.write(90);

  Serial.println("Waiting for controller...");
}

void loop()
{
  if (receivedPacket)
  {
    currentAngle += (data.steeringAngle - currentAngle) * 0.20;

    steering.write((int)currentAngle);

    Serial.print("Received: ");
    Serial.print(data.steeringAngle);

    Serial.print("  Servo: ");
    Serial.println((int)currentAngle);
  }

  delay(20);
}