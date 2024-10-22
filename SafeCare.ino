#include <SoftwareSerial.h>
#include <TinyGPS++.h>

SoftwareSerial sim800l(11, 12); // RX, TX pins for the SIM800L module
SoftwareSerial gpsSerial(6, 7); // RX, TX pins for the GPS module
TinyGPSPlus gps; // Create a TinyGPS++ object
const int buttonPin = 2; // Button connected to pin 2
const int ledPin = 13; // LED connected to pin 13
const int buzzerPin = 9; // Buzzer connected to pin 9
const int xPin = A0; // X axis of accelerometer connected to A0
const int yPin = A1; // Y axis of accelerometer connected to A1
const int zPin = A2; // Z axis of accelerometer connected to A2

int xZero, yZero, zZero;       // Calibrated zero values for X, Y, and Z axes
int xAcc, yAcc, zAcc;          // Acceleration values for X, Y, and Z axes

// SIM800L module configurations
String sim800lNumbers[] = {
  "+111111111111",  // Replace with your desired phone numbers
  "",
  ""
};
int numNumbers = sizeof(sim800lNumbers) / sizeof(sim800lNumbers[0]);

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  sim800l.begin(9600);

  Serial.begin(9600);
  Serial.println("System Started...");
}

void loop() {
  if (digitalRead(buttonPin) == LOW || suddenMovementDetected()) {
    sendAlertMessage();
  }
  
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isUpdated()) {
        Serial.print("Latitude: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Longitude: ");
        Serial.println(gps.location.lng(), 6);
      }
    }
  }
}

bool suddenMovementDetected() {
  int xAcc = analogRead(xPin) - xZero; // Read X axis accelerometer value
  int yAcc = analogRead(yPin) - yZero; // Read Y axis accelerometer value
  int zAcc = analogRead(zPin) - zZero; // Read Z axis accelerometer value

  // Set threshold values for detecting sudden movement
  int threshold = 480; // Adjust this value based on the sensitivity of your accelerometer

  // Check if any of the axes exceeds the threshold
  if (abs(xAcc) > threshold || abs(yAcc) > threshold || abs(zAcc) > threshold) {
    return true; // Sudden movement detected
  } else {
    return false; // No sudden movement detected
  }
}

void sendAlertMessage() {
  digitalWrite(ledPin, HIGH);
  tone(buzzerPin, 1000);

  for (int i = 0; i < numNumbers; i++) {
    // Initialize SMS message
    sim800l.println("AT+CMGF=1"); // Set SMS mode to text
    delay(100);

    // Specify recipient number
    sim800l.print("AT+CMGS=\"");
    sim800l.print(sim800lNumbers[i]);
    sim800l.println("\"");
    delay(100);

   // Compose and send message with latitude and longitude
  String message = "This person could be in danger!\nLatitude: " + String(gps.location.lat(), 6) +
                   "\nLongitude: " + String(gps.location.lng(), 6);
  sim800l.print(message);
  sim800l.write(0x1A); // Send Ctrl+Z
  delay(1000);

    // Clear message buffer
    while (sim800l.available()) {
      sim800l.read();
    }

    // Delay after sending each message to allow SIM800L to process
    delay(1000);
  }

  digitalWrite(ledPin, LOW);
  noTone(buzzerPin);
}
