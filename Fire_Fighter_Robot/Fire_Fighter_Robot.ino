#include <ESP32Servo.h>      // Library for controlling servo motors with ESP32

// ----- Pin Definitions -----
#define IN1 27               // Motor A input pin 1
#define IN2 26               // Motor A input pin 2
#define IN3 25               // Motor B input pin 1
#define IN4 33               // Motor B input pin 2

#define ENA 14               // PWM control pin for Motor A
#define ENB 32               // PWM control pin for Motor B

// Pump relay was on GPIO 12, which is a boot strapping pin: if the relay
// pulls it HIGH at power-up the ESP32 can fail to boot. GPIO 4 is safe.
// >>> Move the relay signal wire from GPIO 12 to GPIO 4. <<<
#define pump 4               // Pin connected to relay controlling the water pump
#define ServoPin 13          // Servo motor signal pin

#define FlameLeft 34         // Analog input pin for left flame sensor
#define FlameMiddle 39       // Analog input pin for middle flame sensor
#define FlameRight 35        // Analog input pin for right flame sensor

#define inbuilt_led 2        // On-board LED pin for ESP32

// Time constants (in milliseconds)
#define move_forward 300     // Forward movement time
#define turning_time 300     // Turning duration
#define move_backward 150    // Backward movement time
#define stop_delay 100       // Delay after stopping
#define servo_step_delay 150 // Delay between servo steps while spraying

int fireThreshold = 500;     // Analog value threshold to detect fire (lower = fire)
int motorSpeed = 200;        // Speed of motors (PWM value from 0-255)

// ----- Objects -----
Servo myServo;              // Create servo motor object

void setup() {
  Serial.begin(115200);     // Start serial communication

  // Set motor control pins as output
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Set other component pins
  pinMode(pump, OUTPUT);
  pinMode(inbuilt_led, OUTPUT);
  pinMode(FlameLeft, INPUT);
  pinMode(FlameMiddle, INPUT);
  pinMode(FlameRight, INPUT);

  // Ensure pump and LED are off initially
  digitalWrite(pump, LOW);
  digitalWrite(inbuilt_led, LOW);
  stopMoving();

  // Attach servo motor to pin
  myServo.attach(ServoPin);
  myServo.write(90);  // Center servo

  Serial.println("Firefighter Robot ready.");
}

// Returns true if any of the three flame sensors sees fire
bool fireVisible() {
  return analogRead(FlameLeft) < fireThreshold ||
         analogRead(FlameMiddle) < fireThreshold ||
         analogRead(FlameRight) < fireThreshold;
}

void loop() {
  // Read analog values from flame sensors
  int leftSensor = analogRead(FlameLeft);
  int middleSensor = analogRead(FlameMiddle);
  int rightSensor = analogRead(FlameRight);

  bool fireDetected = leftSensor < fireThreshold ||
                      middleSensor < fireThreshold ||
                      rightSensor < fireThreshold;

  if (fireDetected) {
    firefightMode(leftSensor, middleSensor, rightSensor);  // Start firefighting
  } else {
    stopMoving();               // Stop robot
    digitalWrite(pump, LOW);    // Turn off pump
    digitalWrite(inbuilt_led, LOW);  // Turn off LED
    myServo.write(90);          // Center the servo
  }
}

void firefightMode(int left, int middle, int right) {
  digitalWrite(inbuilt_led, HIGH);  // Turn on LED as alarm indicator

  // ******************** Robot Movement Logic ********************

  // Fire detected on right and middle sensors
  if (middle < fireThreshold && right < fireThreshold) {
    moveBackward();
    delay(move_backward);
    stopMoving();
    delay(stop_delay);
    slightRight();             // Slight right turn
    delay(turning_time);
    stopMoving();
    delay(stop_delay);
  }
  // Fire detected on left and middle sensors
  else if (middle < fireThreshold && left < fireThreshold) {
    moveBackward();
    delay(move_backward);
    stopMoving();
    delay(stop_delay);
    slightLeft();              // Slight left turn
    delay(turning_time);
    stopMoving();
    delay(stop_delay);
  }
  // Fire detected in the middle only
  else if (middle < fireThreshold) {
    moveForward();             // Move straight
    delay(move_forward);
    stopMoving();
    delay(stop_delay);
  }
  // Fire detected on left only
  else if (left < fireThreshold) {
    moveBackward();
    delay(move_backward);
    stopMoving();
    delay(stop_delay);
    turnLeft();                // Full left turn
    delay(turning_time);
    stopMoving();
  }
  // Fire detected on right only
  else if (right < fireThreshold) {
    moveBackward();
    delay(move_backward);
    stopMoving();
    delay(stop_delay);
    turnRight();               // Full right turn
    delay(turning_time);
    stopMoving();
  }

  spray();   // Spray water while the fire is still visible
  reset();   // Reset robot after spraying
}

// Moves the servo to an angle, then checks the sensors.
// Returns false if the fire is gone, so the sweep can stop early.
bool sprayStep(int angle) {
  myServo.write(angle);
  delay(servo_step_delay);
  return fireVisible();
}

// Sweep the nozzle left/right with the pump on.
// Sensors are checked at every step, so spraying stops as soon as the fire is out.
void spray() {
  digitalWrite(pump, HIGH);    // Activate pump to spray water

  bool fire = true;
  for (int angle = 45; angle <= 135 && fire; angle += 5) fire = sprayStep(angle);
  for (int angle = 135; angle >= 45 && fire; angle -= 5) fire = sprayStep(angle);
  for (int angle = 45; angle <= 90 && fire; angle += 5) fire = sprayStep(angle);

  digitalWrite(pump, LOW);     // Turn off pump
  myServo.write(90);           // Re-center nozzle
}

void reset() {
  moveBackward();              // Move back a bit
  delay(move_backward);
  stopMoving();                // Stop
  digitalWrite(pump, LOW);     // Make sure pump is off
  digitalWrite(inbuilt_led, LOW);  // Turn off LED
}

// ---- Motor Control Functions ----
void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed);  // Set motor speed
  analogWrite(ENB, motorSpeed);
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void turnLeft() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void turnRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void slightLeft() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed * 0.6);  // Reduce speed of right motor
}

void slightRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed * 0.6);  // Reduce speed of left motor
  analogWrite(ENB, motorSpeed);
}

void stopMoving() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);         // Stop both motors
  analogWrite(ENB, 0);
}
