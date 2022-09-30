#include <Arduino.h>
#include <FreeRTOS.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <SPI.h>
#include <ArduinoJson.h>

WiFiClient espClient;
const char *ssid  = "Full House";
const char *password = "11111111";
int flag = 0;

PubSubClient client(espClient);
const char *mqtt_broker      = "broker.hivemq.com";
const char *topic            = "mackcest/data/sensor";
//const char *topic1           = "MQTT/Led";
//const char *mqtt_username    = "emqx";
//const char *mqtt_password    = "public";
const int   mqtt_port        = 1883;
String messageTemp; `

/*Variable Json*/
DynamicJsonDocument doc(1024);
char Buffer[1000];
int Len_Buffer;
int js_temp = 0, js_humi = 0, js_mq2 = 0, js_led = 0;
String ESP32_to_Web_Json = "";

/*Variable Sensor*/

#define PIN_LED 21
#define PIN_MQ2 36
sensor_t sensor;
sensors_event_t event;
float sr_temp, sr_humi, sr_mq2, sr_led;

/*DHT11*/
#define DHTPIN     4     
#define DHTTYPE    DHT11     
DHT_Unified dht(DHTPIN, DHTTYPE);


void callback(char *topic, byte *payload, unsigned int length);
void Data_to_Json(String js_temp, String js_humi, String js_mq2);

void Publish_Data(void *pvParameters);
void Subscribe_Data(void *pvParameters);
void Sensor_ReadData(void *pvParameters);

/*Creart QUEUE*/
QueueHandle_t queue_temp;
QueueHandle_t queue_humi;
QueueHandle_t queue_mq2;

int temp_queuex1, humi_queuex1, mq2_queuex1; /*bien tam*/

void setup() {
  Serial.begin(9600);
  pinMode(PIN_LED,OUTPUT);
  /*connect Wifi*/
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
      delay(200);
      Serial.print(".");
  }
  Serial.println("Connected to the WiFi network");
  
  /*DHT11*/
  dht.begin();
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);

  /*MQTT*/
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if(client.connect(client_id.c_str(), mqtt_username, mqtt_password)){
      Serial.println("Public emqx mqtt broker connected");
    } 
    else{
      Serial.print("failed with state ");
      Serial.print(client.state());
    }
  }
  client.subscribe(topic1);
  queue_temp = xQueueCreate(200, sizeof(int));
  queue_humi = xQueueCreate(200, sizeof(int));
  queue_mq2  = xQueueCreate(200, sizeof(int));
  xTaskCreate(Sensor_ReadData, "read_data", 10000, NULL, 2, NULL );
  xTaskCreate(Publish_Data, "pub_data", 10000, NULL, 1, NULL );
  xTaskCreate(Subscribe_Data, "sub_data", 10000, NULL, 1, NULL );
}

void loop() {
  
}

void Data_to_Json(String js_temp, String js_humi, String js_mq2) {
  ESP32_to_Web_Json = "{\"Temp\":\"" + String(js_temp) + "\"," +
                      "\"Humi\":\"" + String(js_humi) + "\"," +
                      "\"Mq2\":\"" + String(js_mq2) + "\"}";
                      
  //Serial.println(ESP32_to_Web_Json);
  Len_Buffer = ESP32_to_Web_Json.length()+1;
  ESP32_to_Web_Json.toCharArray(Buffer,Len_Buffer);
  Serial.println(Buffer);
}

void callback(char *topic, byte *payload, unsigned int length) { 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();
  Serial.println("DATA MQTT->ESP32:" + String(messageTemp));
  Serial.println("-----------------------");
}

void Sensor_ReadData(void *pvParameters){
  while(1){ 
    /*DHT11*/
    dht.temperature().getEvent(&event);
    sr_temp = 20;//event.temperature;
    dht.humidity().getEvent(&event);
    sr_humi =70;// event.relative_humidity;
    /*MQ2*/
    sr_mq2 = 10;//analogRead(PIN_MQ2);
    vTaskDelay(1000);
  }
  vTaskDelete(NULL);
}


void Publish_Data(void *pvParameters){
  while(1){
    client.loop(); 
    Data_to_Json(String(sr_temp), String(sr_humi), String(sr_mq2));
    client.publish(topic, Buffer);
    vTaskDelay(2000);
  }
  vTaskDelete(NULL);
}

void Subscribe_Data(void *pvParameters){
  while(1){
    if(messageTemp == "LEDON"){
      digitalWrite(PIN_LED,HIGH);
      messageTemp.remove(0,messageTemp.length());
    }
    else if(messageTemp == "LEDOFF"){
      digitalWrite(PIN_LED,LOW);
      messageTemp.remove(0,messageTemp.length());
    }
  }
  vTaskDelete(NULL);
}
