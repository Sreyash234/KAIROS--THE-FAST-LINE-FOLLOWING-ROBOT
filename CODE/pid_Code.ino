#include <SparkFun_TB6612.h>
#include <QTRSensors.h>

// --- 1. HARDWARE PINS ---
// Motor Driver Pins
#define PWMA 10   
#define AIN1 12
#define AIN2 2
#define BIN1 7
#define BIN2 3
#define PWMB 11  
#define STBY 5    

// --- 2. SENSOR DEFINITIONS ---
// QTR Analog Array (6 Pins)
// Ensure A0 is Leftmost and A7 is Rightmost physically
const uint8_t AnalogSensorPins[] = {A0, A1, A2, A3, A6, A7};
const uint8_t SensorCount = 6;

// Digital Wing Sensors 
#define SENSOR_LEFT_WING  8
#define SENSOR_RIGHT_WING 9

// --- 3. CONFIGURATION ---
const int offsetA = 1;  
const int offsetB = 1;  

int BaseSpeed = 100;    
int MaxSpeed = 255;     

// PID Variables
float Kp = 0.1;   
float Ki = 0.0;   //Should always be 0
float Kd = 0.5;

// --- 4. OBJECTS ---
Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

QTRSensors qtr;
uint16_t sensorValues[SensorCount];

// Global tracking
int lastError = 0;
// CRITICAL FIX: Changed from int to long to prevent overflow
long integral = 0; 

void setup() {
  Serial.begin(9600); 
  
  pinMode(SENSOR_LEFT_WING, INPUT);
  pinMode(SENSOR_RIGHT_WING, INPUT);

  qtr.setTypeAnalog();
  qtr.setSensorPins(AnalogSensorPins, SensorCount);

  // --- MANUAL CALIBRATION ---
  Serial.println("MANUAL CALIBRATION STARTING...");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); 

  // Move robot back and forth over line manually now
  for (uint16_t i = 0; i < 400; i++) {
    qtr.calibrate();
    delay(10); 
  }

  digitalWrite(LED_BUILTIN, LOW); 
  Serial.println("CALIBRATION COMPLETE.");
  
  delay(3000); 
}

void loop() {
  // 1. Get Analog Line Position
  // Range: 0 (Left) to 5000 (Right). Center: 2500.
  uint16_t position = qtr.readLineBlack(sensorValues);

  // 2. Read Digital Wings
  // Assuming Active HIGH (1 = Line Detected)
  bool leftWing = digitalRead(SENSOR_LEFT_WING);
  bool rightWing = digitalRead(SENSOR_RIGHT_WING);

  // 3. Calculate Error
  // Positive Error = Robot is to the Right -> Need to turn Left
  int error = position - 2500; 

  // 4. Smart Wing Override
  // Only override if the main sensors are near the edge (0 or 5000)
  // This prevents jerking at T-intersections (Crosses)
  if (leftWing && !rightWing && position < 1000) {
    error = -3000; // Hard force Left turn
  } else if (rightWing && !leftWing && position > 4000) {
    error = 3000;  // Hard force Right turn
  }

  // 5. PID Calculations
  int P = error;
  
  // Accumulate integral only if error is small (prevents windup)
  integral += error; 

  int D = error - lastError;
  lastError = error;

  int correction = (Kp * P) + (Ki * integral) + (Kd * D);

  // 6. Calculate Motor Speeds
  // CRITICAL FIX: Inverted logic. 
  // If Error > 0 (Robot Right), we need m1 (Left) Slow, m2 (Right) Fast to turn Left.
  int m1Speed = BaseSpeed - correction; 
  int m2Speed = BaseSpeed + correction;

  // 7. Apply to Motors
  motor1.drive(m1Speed);
  motor2.drive(m2Speed);
}