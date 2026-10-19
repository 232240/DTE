#include <WiFi.h>
#include <esp_now.h>

// =====================================================
// JOYSTICKS
// =====================================================

// LEFT joystick
#define THROTTLE_PIN 33

// RIGHT joystick
#define STEERING_PIN 34


// =====================================================
// CAR ESP32 MAC ADDRESS
// =====================================================

uint8_t receiverMAC[] = {
  0x20, 0xE7, 0xC8, 0x67, 0x64, 0xBC
};


// =====================================================
// DATA SENT TO CAR
// =====================================================

typedef struct
{
  int steeringAngle;
  int throttle;

} ControllerData;

ControllerData data;


// =====================================================
// JOYSTICK CENTRES
// =====================================================

int throttleCentre;
int steeringCentre;


// =====================================================
// READ JOYSTICK
// =====================================================

int readJoystick(int pin)
{
  long total = 0;

  for (int i = 0; i < 10; i++)
  {
    total += analogRead(pin);
  }

  return total / 10;
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW FAILED");
    return;
  }

  esp_now_peer_info_t peerInfo = {};

  memcpy(
    peerInfo.peer_addr,
    receiverMAC,
    6
  );

  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("FAILED TO ADD CAR");
    return;
  }


  // ===================================================
  // CALIBRATION
  // ===================================================

  Serial.println("CALIBRATING...");
  Serial.println("LEAVE BOTH JOYSTICKS CENTRED!");

  delay(1000);

  long throttleTotal = 0;
  long steeringTotal = 0;

  for (int i = 0; i < 200; i++)
  {
    throttleTotal += analogRead(THROTTLE_PIN);
    steeringTotal += analogRead(STEERING_PIN);

    delay(5);
  }

  throttleCentre = throttleTotal / 200;
  steeringCentre = steeringTotal / 200;

  Serial.println("CALIBRATION COMPLETE");

  Serial.print("Throttle centre: ");
  Serial.println(throttleCentre);

  Serial.print("Steering centre: ");
  Serial.println(steeringCentre);
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
  int throttleRaw = readJoystick(THROTTLE_PIN);
  int steeringRaw = readJoystick(STEERING_PIN);


  // ===================================================
  // THROTTLE
  // ===================================================

  int throttleDifference =
    throttleRaw - throttleCentre;

  int throttle = 0;

  // Dead zone

  if (abs(throttleDifference) < 180)
  {
    throttle = 0;
  }
  else
  {
    throttle = map(
      throttleDifference,
      -throttleCentre,
      4095 - throttleCentre,
      255,
      -255
    );
  }

  throttle = constrain(
    throttle,
    -255,
    255
  );


  // ===================================================
  // STEERING
  // ===================================================

  int steeringDifference =
    steeringRaw - steeringCentre;

  int steeringAngle = 90;


  // Dead zone around centre

  if (abs(steeringDifference) < 180)
  {
    steeringAngle = 90;
  }
  else if (steeringRaw > steeringCentre)
  {
    steeringAngle = map(
      steeringRaw,
      steeringCentre + 180,
      4095,
      90,
      135
    );
  }
  else
  {
    steeringAngle = map(
      steeringRaw,
      0,
      steeringCentre - 180,
      45,
      90
    );
  }

  steeringAngle = constrain(
    steeringAngle,
    45,
    135
  );


  // ===================================================
  // SEND DATA
  // ===================================================

  data.throttle = throttle;
  data.steeringAngle = steeringAngle;

  esp_now_send(
    receiverMAC,
    (uint8_t *)&data,
    sizeof(data)
  );


  // ===================================================
  // DEBUG
  // ===================================================

  Serial.print("Throttle raw: ");
  Serial.print(throttleRaw);

  Serial.print(" | Throttle: ");
  Serial.print(throttle);

  Serial.print(" | Steering raw: ");
  Serial.print(steeringRaw);

  Serial.print(" | Angle: ");
  Serial.println(steeringAngle);

  delay(30);
}