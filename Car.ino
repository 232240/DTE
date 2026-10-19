#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>


// =====================================================
// SERVO
// =====================================================

#define SERVO_PIN 18

Servo steering;


// =====================================================
// L298N MOTOR PINS
// =====================================================

// MOTOR 1

#define ENA 25
#define IN1 26
#define IN2 27


// MOTOR 2

#define ENB 32
#define IN3 33
#define IN4 22


// =====================================================
// CONTROLLER DATA
// =====================================================

typedef struct
{
  int steeringAngle;
  int throttle;

} ControllerData;


// Start in a safe state

ControllerData data =
{
  90,
  0
};


// =====================================================
// CURRENT SERVO POSITION
// =====================================================

float currentAngle = 90;


// =====================================================
// ESP-NOW RECEIVE CALLBACK
// =====================================================

void onReceive(
  const esp_now_recv_info *info,
  const uint8_t *incomingData,
  int len
)
{
  // Make sure we received the correct packet

  if (len == sizeof(data))
  {
    memcpy(
      &data,
      incomingData,
      sizeof(data)
    );
  }
}


// =====================================================
// MOTOR CONTROL FUNCTION
// =====================================================

void setMotors(int speed)
{
  // Make sure speed stays between -255 and +255

  speed = constrain(
    speed,
    -255,
    255
  );


  // ===================================================
  // STOP
  // ===================================================

  if (speed == 0)
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    analogWrite(ENA, 0);
    analogWrite(ENB, 0);

    return;
  }


  // ===================================================
  // FORWARD
  // ===================================================

  if (speed > 0)
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }


  // ===================================================
  // REVERSE
  // ===================================================

  else
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }


  // ===================================================
  // MOTOR SPEED
  // ===================================================

  int motorSpeed = abs(speed);

  analogWrite(
    ENA,
    motorSpeed
  );

  analogWrite(
    ENB,
    motorSpeed
  );
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  Serial.println();
  Serial.println("CAR STARTING");


  // ===================================================
  // MOTOR PINS
  // ===================================================

  pinMode(ENA, OUTPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);


  // Make absolutely sure motors start stopped

  setMotors(0);


  // ===================================================
  // SERVO
  // ===================================================

  steering.setPeriodHertz(50);
  ESP32PWM::timerCount[0] = 4;
ESP32PWM::timerCount[1] = 4;

  steering.attach(
    SERVO_PIN,
    500,
    2500
  );

  steering.write(90);


  // ===================================================
  // ESP-NOW
  // ===================================================

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW INITIALISATION FAILED");
    return;
  }


  // Register receive callback

  esp_now_register_recv_cb(onReceive);


  Serial.println("CAR READY");
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
  // ===================================================
  // SMOOTH STEERING
  // ===================================================

  currentAngle +=
    (data.steeringAngle - currentAngle)
    * 0.15;


  steering.write(
    (int)currentAngle
  );


  // ===================================================
  // CONTROL MOTORS
  // ===================================================

  setMotors(
    data.throttle
  );


  // ===================================================
  // SERIAL MONITOR
  // ===================================================

  Serial.print("Steering: ");
  Serial.print(data.steeringAngle);

  Serial.print(" | Throttle: ");
  Serial.println(data.throttle);


  delay(20);
}