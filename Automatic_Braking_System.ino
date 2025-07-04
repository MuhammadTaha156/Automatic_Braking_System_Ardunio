#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define Pin Connections
#define trigPin 2    // Ultrasonic Sensor Trigger Pin
#define echoPin 3    // Ultrasonic Sensor Echo Pin
#define enA 9        // Motor Driver Enable Pin (PWM)
#define in1 8        // Motor Driver Input 1
#define in2 7        // Motor Driver Input 2
#define ledPin 4     // LED Pin (for status indication)

// I2C LCD Address and Dimensions (usually 0x27 or 0x3F for 16x2 LCDs)
LiquidCrystal_I2C lcd(0x20, 16, 2);

// Constants for calculations
const float SOUND_SPEED_CM_US = 0.0343; // Speed of sound in cm per microsecond (approx 343 m/s)
const unsigned long ULTRASONIC_TIMEOUT_US = 25000; // Timeout for pulseIn in microseconds (covers ~400cm)

// Global Variables
long duration;      // Duration of ultrasonic pulse
int distance;       // Calculated distance in cm
int speedValue;     // Motor speed (0-255 for analogWrite)

void setup() {
  // Initialize Pin Modes
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(ledPin, OUTPUT); // Set the LED pin as an OUTPUT

  // Initialize Serial Communication for debugging
  Serial.begin(9600);

  // Initialize LCD
  lcd.init();      // Initialize the LCD
  lcd.backlight(); // Turn on LCD backlight
  lcd.setCursor(0, 0);
  lcd.print("Smart Stop Ready");
  delay(2000); // Display message for 2 seconds
  lcd.clear(); // Clear LCD screen
}

// Function to get an average distance reading from the ultrasonic sensor
int getAverageDistance() {
  long totalDistance = 0;
  int numReadings = 5; // Number of readings to average
  int validReadings = 0; // To count valid readings

  for (int i = 0; i < numReadings; i++) {
    // Clear the trigPin by setting it LOW for 2 microseconds
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Set the trigPin HIGH for 10 microseconds to send a pulse
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Measure the duration of the pulse on the echoPin
    duration = pulseIn(echoPin, HIGH, ULTRASONIC_TIMEOUT_US);

    // Calculate the distance
    int currentReading = (int)(duration * SOUND_SPEED_CM_US / 2);

    // Filter out potentially bad readings (e.g., 0 or very large due to timeout)
    // Assuming max practical range of 400cm. Adjust if your application needs more.
    if (currentReading > 0 && currentReading < 400) {
      totalDistance += currentReading;
      validReadings++; // Increment valid readings count
    }
    delay(10); // Small delay between readings for stability
  }

  // Avoid division by zero if no valid readings were obtained
  if (validReadings > 0) {
    return totalDistance / validReadings;
  } else {
    // Return a high value if no valid readings, implying clear path/out of range
    return 400;
  }
}

void loop() {
  distance = getAverageDistance(); // Get the current average distance

  // Display distance on Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Display distance on LCD
  lcd.setCursor(0, 0);
  lcd.print("Distance:        "); // Clear previous numbers/text
  lcd.setCursor(10, 0);
  lcd.print(distance);
  lcd.print("cm ");

  // Motor and LED Control Logic based on new requirements
  // LED Logic: Turn LED ON if an object is detected within 100cm (or any threshold you define)
  if (distance <= 100) { // Agar object 100 cm ya us se kam distance par hai
    digitalWrite(ledPin, HIGH); // LED ON karo
  } else {
    digitalWrite(ledPin, LOW); // LED OFF karo
  }

  // Motor Control Logic (as previously defined)
  if (distance <= 50) { // Jab distance 50 cm ya us se kam ho
    // Motor stop
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(enA, 0);    // Set motor speed to 0
    speedValue = 0;         // Update speedValue variable

    lcd.setCursor(0, 1);
    lcd.print("Status: STOP    "); // Display motor status
    Serial.println("Motor Stopped (<=50cm)");
  }
  else if (distance > 50 && distance <= 100) { // Jab distance 50 cm aur 100 cm ke beech ho
    // Slow speed
    speedValue = 100; // Aap yeh value adjust kar sakte hain (e.g., 80, 120)
    digitalWrite(in1, HIGH); // Set motor direction (e.g., forward)
    digitalWrite(in2, LOW);
    analogWrite(enA, speedValue); // Apply the determined speed

    lcd.setCursor(0, 1);
    lcd.print("Speed: ");
    lcd.print(speedValue);
    lcd.print("      "); // Clear previous numbers/text
    Serial.print("Motor Speed: ");
    Serial.println(speedValue);
    Serial.println("Slow Speed (50-100cm)");
  }
  else { // Jab distance 100 cm se zyada ho
    // Full speed (normal speed)
    speedValue = 255; // Full speed
    digitalWrite(in1, HIGH); // Set motor direction (e.g., forward)
    digitalWrite(in2, LOW);
    analogWrite(enA, speedValue); // Apply the determined speed

    lcd.setCursor(0, 1);
    lcd.print("Speed: ");
    lcd.print(speedValue);
    lcd.print("      "); // Clear previous numbers/text
    Serial.print("Motor Speed: ");
    Serial.println(speedValue);
    Serial.println("Full Speed (>100cm)");
  }

  delay(200); // Small delay to allow stable readings and updates
}
