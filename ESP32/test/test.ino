#include <FreeRTOS.h>
#include <Arduino.h>
#include "DHT.h"
//#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "ArduinoJson.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>

#define DHTPIN 13
#define ssid "Full House"
#define password "11111111"
#define mqtt_server "broker.mqtt-dashboard.com"
const uint16_t mqtt_port = 1883;
#define topic_main "mackcest/data/sensor"
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

void Publish_Data(void *pvParameters);
void Subscribe_Data(void *pvParameters);
void Sensor_ReadData(void *pvParameters);

/*Creart QUEUE*/
QueueHandle_t queue_temp;
QueueHandle_t queue_humi;
QueueHandle_t queue_light;

int temp_queuex1, humi_queuex1, light_queuex1;
int t, h, SoilMisture, luminance;
char tempString[10];
char humiString[10];
char lightString[10];
char SoilMistureString[10];
char buffer[256];
String messageTemp;
char *topic1;


void setup()
{
  Serial.begin(115200);
  Wire.begin();
  pinMode(4, INPUT);
  //    pinmode(5, INPUT_PULLUP);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  if (!client.connected())
  {
    reconnect();
  }
  client.subscribe("light");
  client.subscribe("pump");
  client.subscribe("fan");
  client.subscribe("heater");

  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(19, OUTPUT);
  digitalWrite(21, LOW);
  digitalWrite(22, LOW);
  dht.begin();
//  dht.readHumidity();
//  dht.readTemperature();
  queue_temp = xQueueCreate(200, sizeof(int));
  queue_humi = xQueueCreate(200, sizeof(int));
  queue_light = xQueueCreate(200, sizeof(int));
  xTaskCreate(Sensor_ReadData, "read_data", 10000, NULL, 2, NULL);
  xTaskCreate(Publish_Data, "pub_data", 10000, NULL, 1, NULL);
  xTaskCreate(Subscribe_Data, "sub_data", 10000, NULL, 1, NULL);
}
void loop() {}
void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
// reconect
void reconnect()
{
  while (!client.connected())
  {
    String clientId = "ESP8266Client-Mackcest";
    if (client.connect("ESP8266Client-Mackcest"))
    {
      Serial.println("Đã kết nối:");
      client.subscribe("light");
      client.subscribe("pump");
      client.subscribe("fan");
      client.subscribe("heater");
    }
    else
    {
      Serial.print("Lỗi:, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{


    messageTemp = message;
//  Serial.print("Message arrived in topic: ");
//  Serial.println(topic);
//  Serial.print("Message:");
//  for (int i = 0; i < length; i++) {
//    Serial.print((char) payload[i]);
//    messageTemp += (char)payload[i];
//  }
//  topic1 = topic;
//  Serial.println();
//  Serial.println("DATA MQTT->ESP32:" + String(messageTemp));
//  Serial.println("-----------------------");
}
unsigned long lastMsg = 0;
void Sensor_ReadData(void *pvParameters)
{
  while (1)
  {
    if (!client.connected())
    {
      reconnect();
    }
    long now = millis();
    if (now - lastMsg > 2000)
    {
      lastMsg = now;
      if (isnan(h) || isnan(t))
      {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
      }
      h = dht.readHumidity();
      t = dht.readTemperature();
      //            h = 30;
      //            t = 80;
      SoilMisture = 62;
      luminance = 50;
    }
    vTaskDelay(1000);
  }
  vTaskDelete(NULL);
}
void Publish_Data(void *pvParameters)
{
  while (1)
  {
    client.loop();
    tempString[10];
    sprintf(tempString, "%d", t);
    humiString[10];
    sprintf(humiString, "%d", h);
    lightString[10];
    sprintf(lightString, "%f", luminance);
    SoilMistureString[10];
    sprintf(SoilMistureString, "%f", SoilMisture);
    StaticJsonDocument<100> doc;
    doc["Temperature"] = t;
    doc["Humidity"] = h;
    doc["Light"] = luminance;
    doc["SoilMisture"] = SoilMisture;
    buffer[256];
    serializeJson(doc, buffer);
    client.publish(topic_main, buffer);
    Serial.println("buffer");
    Serial.println(buffer);
    vTaskDelay(2000);
  }
  vTaskDelete(NULL);
}

void Subscribe_Data(void *pvParameters)
{
  while (1)
  {
    if (messageTemp == "lightOn")
    {
      digitalWrite(21, HIGH);
      vTaskDelay(100);
    }
    else if (messageTemp == "lightOff")
    {
      digitalWrite(21, LOW);
      vTaskDelay(100);
    }



    else if (messageTemp == "pumpOn")
    {
      digitalWrite(22, HIGH);
      vTaskDelay(100);
    }
    else if (messageTemp == "pumpOff")
    {
      digitalWrite(22, LOW);
      vTaskDelay(100);
    }


    else if (messageTemp == "fanOn")
    {
      digitalWrite(23, HIGH);
      vTaskDelay(100);
    }
    else if (messageTemp == "fanOff")
    {
      digitalWrite(23, LOW);
      vTaskDelay(100);
    }


    else if (messageTemp == "heaterOn")
    {
      digitalWrite(19, HIGH);
      vTaskDelay(100);
    }
    else if (messageTemp == "heaterOff")
    {
      digitalWrite(19, LOW);
      vTaskDelay(100);
    }
    else {
      
    }
  }
  vTaskDelete(NULL);
}
