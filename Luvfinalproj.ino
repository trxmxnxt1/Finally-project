#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <ESP32_Servo.h>

Adafruit_SH1106 oled(21, 22); // Specify the correct pins for SDA and SCK

char ssid[] = "Luv";
char pass[] = "12345678";

const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "ef7491af-610c-4c0a-9a28-1b4139cea365";
const char* mqtt_username = "jkBuwJ2uW3F7Kg9DDzhkrF9z3DbBLCe7";
const char* mqtt_password = "dzR6f2Q3UEAjdZdNAQwqUC8cz5TBor33";

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     7
#define UTC_OFFSET_DST 0

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, UTC_OFFSET);
WiFiClient espClient;
PubSubClient client(espClient);

#define buzzer 5 // Buzzer
#define led 18
int sensorPin = 32; // TCRT5000
int threshold = 500; // กำหนดค่าสะสมต่ำสุดที่ถือว่าตรวจจับได้

int sw1 = 33;
int sw2 = 25;
int sw3 = 26;
int sw4 = 27;
bool servo;
int ledState = 0; // เพิ่มตัวแปรเพื่อเก็บสถานะ LED

Servo myservo;
int servoPin = 13;
unsigned long previousMillis = 0; // เก็บค่าเวลาก่อนหน้า
unsigned long interval = 1000; // ระยะเวลาในการรอ (เป็นมิลลิวินาที)

int setHour, setMinute, setSec, onSet;
int hr, minute, sec;
char msg[1000];

void setup() {
  Serial.begin(115200); // Open Serial Monitor
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  configTime(UTC_OFFSET * 3600, UTC_OFFSET_DST, NTP_SERVER);

  // SW,Servo,LED
  pinMode(sw1, INPUT);
  pinMode(sw2, INPUT);
  pinMode(sw3, INPUT);
  pinMode(sw4, INPUT);
  pinMode(led, OUTPUT);
  pinMode(13, OUTPUT);
  myservo.attach(13, 544, 3000); // Servo
  myservo.write(0);
  // SW,Servo,LED

  // Start Buzzer
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  // END Buzzer

  sec += 6;

  // Start OLED
  oled.begin(SH1106_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay(); // Clear the display
  connectWiFi();
  timeClient.begin();
  timeClient.setTimeOffset(UTC_OFFSET * 3600);
  printLocalTime();
  // END OLED
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  hr = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  sec = timeinfo.tm_sec;
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

void reconnect() {

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("@msg/timer/setHr");
      client.subscribe("@msg/timer/setMinute");
      client.subscribe("@msg/timer/setSec");
      client.subscribe("@msg/timer/onSet");
      client.subscribe("@msg/timer/servo");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  
  if (String(topic) == "@msg/timer/servo") {
    Serial.println("Feed the fish");
    myservo.write(180); // กำหนดให้ servo หมุนไปที่ 180 องศา
    buzzForDuration(100); // เรียกใช้งาน buzzer เป็นเวลา 0.1 วินาที
    delay(1800); // รอเป็นเวลา 1.8 วินาที
    myservo.write(0); // กำหนดให้ servo หมุนกลับไปที่ 0 องศา
    buzzForDuration(100); // เรียกใช้งาน buzzer เป็นเวลา 0.1 วินาที
  }
  
  // print arrive msg
  if (strcmp(topic, "@msg/timer/onSet") == 0) {
    if (message == "on") {
      onSet = 1;
      digitalWrite(led, HIGH);
      Serial.println("Timer mode on");
    } else {
      onSet = 0;
      digitalWrite(led, LOW);
      Serial.println("Timer mode off");
    }
  }

  if (onSet) {
    if (strcmp(topic, "@msg/timer/setHr") == 0) {
      setHour = message.toInt();
      Serial.println(setHour);
    }
    if (strcmp(topic, "@msg/timer/setMinute") == 0) {
      setMinute = message.toInt();
      Serial.println(setMinute);
    }
    if (strcmp(topic, "@msg/timer/setSec") == 0) {
      setSec = message.toInt();
      Serial.println(setSec);
    }

  }
  
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

  oled.setTextColor(WHITE, BLACK); // Set text color to white on a black background
  oled.setCursor(0, 45); // Set cursor position
  oled.setTextSize(1); // Set text size
  oled.print("Timer mode : ");

  oled.setCursor(75, 45); // Set cursor position for detection status
  oled.setTextSize(1); // Set text size for detection status
  if (onSet == 1) {
    oled.setTextColor(WHITE, BLACK);
    oled.println("ON");
  } else {
    oled.setTextColor(WHITE, BLACK);
    oled.println("OFF");
  }
  oled.display(); // Display the content
  delay(100);
  oled.clearDisplay();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  unsigned long currentMillis = millis(); // อ่านค่าเวลาปัจจุบัน
  // ตรวจสอบว่ามีการกดสวิตซ์หรือไม่และเวลาผ่านไปตามระยะเวลาที่กำหนดหรือไม่
  printLocalTime();
  relayControl();
  printTime();

  if (digitalRead(sw1) == LOW) {
    if (ledState == 0) {
      onSet = 1;
      digitalWrite(led, 1);
      ledState = 1;
      Serial.println("Switch 1 has pressed - Timer mode on");
      delay(100);
    } else {
      onSet = 0;
      digitalWrite(led, 0);
      ledState = 0;
      Serial.println("Switch 1 has pressed - Timer mode off");
      delay(100);
    }

  }

  if (digitalRead(sw2) == LOW && currentMillis - previousMillis >= interval) {
    myservo.write(180); // กำหนดให้ servo หมุนไปที่ 180 องศา
    buzzForDuration(100); // เรียกใช้งาน buzzer เป็นเวลา 0.1 วินาที
    previousMillis = currentMillis; // บันทึกค่าเวลาปัจจุบัน
    Serial.println("SW2 pressed");

  }

  if (digitalRead(sw3) == LOW && currentMillis - previousMillis >= interval) {
    myservo.write(0); // กำหนดให้ servo หมุนไปที่ 0 องศา
    buzzForDuration(100); // เรียกใช้งาน buzzer เป็นเวลา 0.1 วินาที
    previousMillis = currentMillis; // บันทึกค่าเวลาปัจจุบัน
    Serial.println("SW3 pressed");
  }

  if ((digitalRead(sw4) == LOW || servo ) && currentMillis - previousMillis >= interval) {
    myservo.write(180); // กำหนดให้ servo หมุนไปที่ 180 องศา
    buzzForDuration(100); // เรียกใช้งาน buzzer เป็นเวลา 0.1 วินาที
    delay(1800); // รอเป็นเวลา 1.8 วินาที
    myservo.write(0); // กำหนดให้ servo หมุนกลับไปที่ 0 องศา
    buzzForDuration(100); // เรียกใช้งาน buzzer เป็นเวลา 0.1 วินาที
    previousMillis = currentMillis; // บันทึกค่าเวลาปัจจุบัน
    Serial.println("SW4 pressed");

  }

  Serial.println(onSet);

  Serial.println(String(hr) + ":" + String(minute) + ":" + String(sec));
  String dateTime = "{\"data\": {\"hr\":" + String(hr) + ", \"min\":" + String(minute) + ", \"sec\": " +  String(sec) + ", \"from\": \"Thiramanat1\"" + "}}";
  dateTime.toCharArray(msg, (dateTime.length() + 1));
  client.publish("@shadow/data/update", msg);

  Serial.println(String(setHour) + ":" + String(setMinute) + ":" + String(setSec));

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

void relayControl() {

  int setTime = timeInSec(setHour, setMinute, setSec);
  int realTime = timeInSec(hr, minute, sec);
  if (!onSet) {
    if (realTime <= setTime ) {
      servo = 0;
      Serial.println("Real time");
      // เรียกใช้ฟังก์ชันให้ buzzer ทำงานเมื่อ servo หมุนที่ 180 องศา
    }
     if (realTime >= setTime && realTime <= setTime + 5) {
      servo = 1;
      Serial.println("Set time");
    }
    else {
      servo = 0;
      Serial.println("None time");
    }
  }
}

void buzzForDuration(int duration) {
  digitalWrite(buzzer, HIGH);
  delay(duration);
  digitalWrite(buzzer, LOW);
}

int timeInSec(int hr, int minute, int sec) {
  return hr * 3600 + minute * 60 + sec;
}
