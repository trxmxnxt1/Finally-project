#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <ESP32_Servo.h>

char ssid[] = "thiramanat_2.4G";
char pass[] = "newin11111";
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "ef7491af-610c-4c0a-9a28-1b4139cea365";
const char* mqtt_username = "jkBuwJ2uW3F7Kg9DDzhkrF9z3DbBLCe7";
const char* mqtt_password = "dzR6f2Q3UEAjdZdNAQwqUC8cz5TBor33";

WiFiClient espClient;
PubSubClient client(espClient);

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     7
#define UTC_OFFSET_DST 0

int relayPin = 27;  // กำหนดขา Relay
int setHour, setMinute, setSec, onSet;
int hr, minute, sec;
char msg[1000];

Servo myservo;
int servoPin = 13;

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  hr = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  sec = timeinfo.tm_sec;
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("@msg/timer/setHr");
      client.subscribe("@msg/timer/setMinute");
      client.subscribe("@msg/timer/setSec");
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


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  pinMode(relayPin, OUTPUT);

  configTime(UTC_OFFSET * 3600, UTC_OFFSET_DST, NTP_SERVER);

   pinMode(13, OUTPUT);
  myservo.attach(13, 544, 2400); // Servo
  digitalWrite(13, LOW);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  printLocalTime();
  relayControl();

  Serial.println(String(hr) + ":" + String(minute) + ":" + String(sec));
  String dateTime = "{\"data\": {\"hr\":" + String(hr) + ", \"min\":" + String(minute) + ", \"sec\": " +  String(sec) + ", \"from\": \"Thiramanat1\"" + "}}";
  dateTime.toCharArray(msg, (dateTime.length() + 1));
  client.publish("@shadow/data/update", msg);

  Serial.println(String(setHour) + ":" + String(setMinute) + ":" + String(setSec));
  delay(1000);
}

void relayControl() {
  int setTime = timeInSec(setHour, setMinute, setSec);
  int realTime = timeInSec(hr, minute, sec);

  if (realTime >= setTime ) {
    myservo.write(180);
    Serial.println("Turn off the relay");
  }
  if (realTime >= setTime + 10 && realTime <= setTime + 11) {
    myservo.write(0);
    Serial.println("Turn on the relay");
  }

}

int timeInSec(int hr, int minute, int sec) {
  return hr * 3600 + minute * 60 + sec;
}
