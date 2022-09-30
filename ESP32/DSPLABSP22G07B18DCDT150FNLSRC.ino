#include <FreeRTOS.h>
#include <Arduino.h>
#include "DHT.h"
//#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "ArduinoJson.h"
#include <Wire.h>

#define DHTPIN D7
#define ssid "iot"
#define password "11111111"
#define mqtt_server "broker.mqtt-dashboard.com"
const uint16_t mqtt_port = 1883;
#define topic_main "mackcest/data/sensor"
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  pinMode(A0, INPUT);
  pinMode(D0, INPUT_PULLUP);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  if (!client.connected())
  { // Kiểm tra kết nối
    reconnect();
  }
  client.subscribe("light");
  client.subscribe("pump");
  client.subscribe("fan");
  client.subscribe("heater");

  pinMode(D1, OUTPUT); // light
  pinMode(D2, OUTPUT); // pump
  pinMode(D3, OUTPUT); // pump
  pinMode(D4, OUTPUT); // heater
  dht.begin();
}

// Hàm kết nối wifi
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
  // in ra thông báo đã kết nối và địa chỉ IP của ESP8266
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Hàm call back để nhận dữ liệu
void callback(char *topic, byte *payload, unsigned int length)
{
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);

  if (String(topic) == "light")
  {
    if (message == "lightOn")
    {
      digitalWrite(D1, HIGH);
    }
    if (message == "lightOff")
    {
      digitalWrite(D1, LOW);
    }
  }

  if (String(topic) == "pump")
  {
    if (message == "pumpOn")
    {
      digitalWrite(D2, HIGH);
    }
    if (message == "pumpOff")
    {
      digitalWrite(D2, LOW);
    }
  }

  if (String(topic) == "fan")
  {
    if (message == "fanOn")
    {
      digitalWrite(D3, HIGH);
    }
    if (message == "fanOff")
    {
      digitalWrite(D3, LOW);
    }
  }

  if (String(topic) == "heater")
  {
    if (message == "heaterOn")
    {
      digitalWrite(D4, HIGH);
    }
    if (message == "heaterOff")
    {
      digitalWrite(D4, LOW);
    }
  }

  Serial.println(message);
  Serial.println();
  //-------------------------------------------------------------------------
}

// Hàm reconnect thực hiện kết nối lại khi mất kết nối với MQTT Broker
void reconnect()
{
  while (!client.connected()) // Chờ tới khi kết nối
  {
    String clientId = "ESP8266Client-Mackcest";
    if (client.connect("ESP8266Client-Mackcest")) // kết nối vào broker
    {
      Serial.println("Đã kết nối:");
      client.subscribe("light");
      client.subscribe("pump");
      client.subscribe("fan");
      client.subscribe("heater");
    }
    else
    {
      // in ra trạng thái của client khi không kết nối được với broker
      Serial.print("Lỗi:, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Đợi 5s
      delay(5000);
    }
  }
}

unsigned long lastMsg = 0;
void loop()
{
  if (!client.connected())
  { // Kiểm tra kết nối
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    int h = dht.readHumidity();
    int t = dht.readTemperature();
    //    h = 80;
    //    t = 30;
    if (isnan(h) || isnan(t))
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    float luminance = 0.00, ADC_value = 0.0048828125, LDR_value = 500;
    // LDR_value = analogRead(A0);
    luminance = (250 / (ADC_value * LDR_value)) - 50;
    //    int real_value = 0;
    //    int value = 0;
    //    int i = 0;
    int SoilMisture = 62;
    //    for (i = 0; i < 9; i++)
    //    {
    //      real_value += analogRead(A0); //Đọc giá trị cảm biến độ ẩm đất   // Đợi đọc giá trị ADC
    //    }
    //    value = real_value / 10;
    //    SoilMisture = map(value, 350, 1023, 0, 100);
    //    SoilMisture = 100 - SoilMisture;
    //    if (SoilMisture < 85 ) {
    //      digitalWrite(D2, HIGH);
    //    }
    //    else {
    //      digitalWrite(D2, LOW);
    //    }
    char tempString[10];
    sprintf(tempString, "%d", t);
    char humiString[10];
    sprintf(humiString, "%d", h);
    char lightString[10];
    sprintf(lightString, "%f", luminance);

    StaticJsonDocument<100> doc;
    doc["Temperature"] = t;
    doc["Humidity"] = h;
    doc["Light"] = luminance;
    doc["SoilMisture"] = SoilMisture;

    char buffer[256];
    serializeJson(doc, buffer);
    client.publish(topic_main, buffer);
    Serial.println("buffer");
    Serial.println(buffer);
  }
}

/// RTOS -------------

void Sensor_ReadData(void *pvParameters)
{
  while (1)
  {
    /*DHT11*/
    dht.temperature().getEvent(&event);
    sr_temp = 20; // event.temperature;
    dht.humidity().getEvent(&event);
    sr_humi = 70; // event.relative_humidity;
    /*MQ2*/
    sr_mq2 = 10; // analogRead(PIN_MQ2);
    vTaskDelay(1000);
  }
  vTaskDelete(NULL);
}

void Publish_Data(void *pvParameters)
{
  while (1)
  {
    client.loop();
    Data_to_Json(String(sr_temp), String(sr_humi), String(sr_mq2));
    client.publish(topic, Buffer);
    vTaskDelay(2000);
  }
  vTaskDelete(NULL);
}

void Subscribe_Data(void *pvParameters)
{
  while (1) 
  {
    if (messageTemp == "LEDON")
    {
      digitalWrite(PIN_LED, HIGH);
      messageTemp.remove(0, messageTemp.length());
    }
    else if (messageTemp == "LEDOFF")
    {
      digitalWrite(PIN_LED, LOW);
      messageTemp.remove(0, messageTemp.length());
    }
  }
  vTaskDelete(NULL);
}
