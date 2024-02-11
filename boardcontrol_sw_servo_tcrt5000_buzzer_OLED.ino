#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <ESP32_Servo.h>

Adafruit_SH1106 oled(21, 22); // Specify the correct pins for SDA and SCK

char ssid[] = "thiramanat_2.4G";
char pass[] = "newin11111";

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     7
#define UTC_OFFSET_DST 0

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, UTC_OFFSET);

#define buzzer 5 // Buzzer
int sensorPin = 32; // TCRT5000
int threshold = 500; // กำหนดค่าสะสมต่ำสุดที่ถือว่าตรวจจับได้

int sw1 = 33;
int sw2 = 25;
int sw3 = 26;
int sw4 = 27;
int LED = 18;

Servo myservo;
int servoPin = 13;

bool ledState = LOW; // เพิ่มตัวแปรเพื่อเก็บสถานะ LED

void setup() {
  Serial.begin(115200); // Open Serial Monitor

  // SW,Servo,LED
  pinMode(sw1, INPUT);
  pinMode(sw2, INPUT);
  pinMode(sw3, INPUT);
  pinMode(sw4, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(13, OUTPUT);
  myservo.attach(13, 544, 2400); // Servo
  digitalWrite(13, LOW);
  // SW,Servo,LED

  // Start Buzzer
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  // END Buzzer

  // Start OLED
  oled.begin(SH1106_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay(); // Clear the display
  connectWiFi();
  timeClient.begin();
  timeClient.setTimeOffset(UTC_OFFSET * 3600);
  // END OLED
}

void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void printTime() {
  timeClient.update();
  oled.setTextColor(WHITE, BLACK); // Set text color to white on a black background
  oled.setCursor(0, 0); // Set cursor position
  oled.setTextSize(1); // Set text size
  oled.print("Time : ");
  oled.println(timeClient.getFormattedTime());
}

void printDetectionStatus(bool detected) {
  
  oled.setTextColor(WHITE, BLACK); // Set text color to white on a black background
  oled.setCursor(0, 15); // Set cursor position
  oled.setTextSize(1); // Set text size
  oled.print("Fish feed : ");

  oled.setCursor(0, 30); // Set cursor position for detection status
  oled.setTextSize(1); // Set text size for detection status
  if (detected) {
    oled.setTextColor(WHITE, BLACK);
    oled.println("Detected");
  } else {
    oled.setTextColor(WHITE, BLACK);
    oled.println("Not Detected");
  }
  oled.display(); // Display the content
  delay(100);
  oled.clearDisplay();
}

void checkSwitches() {
  if (digitalRead(sw1) == LOW) {
    Serial.println("SW1 pressed");
  }
  if (digitalRead(sw2) == LOW) {
    Serial.println("SW2 pressed");
    myservo.write(180);
  }
  if (digitalRead(sw3) == LOW) {
    Serial.println("SW3 pressed");
    myservo.write(0);
  }
  if (digitalRead(sw4) == LOW) {
    Serial.println("SW4 pressed");
  }
}

void loop() {
  printTime();
  checkSwitches();

  // Start TCRT5000
  int sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);
  if (sensorValue > threshold) {
    Serial.println("Not Detected");
    printDetectionStatus(false);
  } else {
    Serial.println("Detected");
    printDetectionStatus(true);
  }
  // END TCRT5000
  delay(1000); // Adjust the delay as needed
}
