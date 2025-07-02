/****************************************************************************************************************************
  FullyFeatured_PortentaH7_Ethernet.ino

  AsyncMqttClient_Generic is a library for ESP32, ESP8266, Protenta_H7, STM32F7, etc. with current AsyncTCP support

  Based on and modified from :

  1) async-mqtt-client (https://github.com/marvinroger/async-mqtt-client)

  Built by Khoi Hoang https://github.com/khoih-prog/AsyncMqttClient_Generic
 *****************************************************************************************************************************/


#include<Arduino.h>
#include "defines.h"
#include <AsyncMqtt_Generic.h>
#include <ArduinoJson.h>
#include <Adafruit_MAX31865.h>
#include <Dynamixel2Arduino.h>
#include "mbed.h"
#include "rtos.h"




 Dynamixel2Arduino dxl(Serial3, 5);
int motor_degree = 0;

//Adafruit_MAX31865 thermo = Adafruit_MAX31865(7);

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      4300.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  1000.0

 using namespace rtos;
 Thread connectThread;
 Thread publish_data_thread;
Thread dxl_thead;
 
Mutex mtx,dxl_mtx;
ConditionVariable condVar(mtx);
 #define DXL_ID 1


 #define MQTT_HOST         IPAddress(192, 168, 29, 120)
 //#define MQTT_HOST         IPAddress(97, 74, 86, 232)
 //#define MQTT_HOST         "mqtt.eclipseprojects.io"        // Broker address
 #define MQTT_PORT         1883
 
 const char *PubTopic  = "async-mqtt/Portenta_H7_Ethernet_Pub";               // Topic to publish
 const char *sensTopic = "sensor_sub";               // Topic to send sensor data

 #define PUB_DELAY 100
 AsyncMqttClient mqttClient;
 
 bool connectedEthernet  = false;
 bool connectedMQTT      = false;
 
 // Check connection every 1s
 #define MQTT_CHECK_INTERVAL_MS     1000
 
 void connectToMqttLoop()
 {
   while (true)
   {
     if (Ethernet.linkStatus() == LinkON)
     {
       if (!connectedMQTT)
       {
         mqttClient.connect();
       }
 
       if (!connectedEthernet)
       {
         Serial.println("Ethernet reconnected.");
         connectedEthernet = true;
       }
     }
     else
     {
       if (connectedEthernet)
       {
         Serial.println("Ethernet disconnected");
         connectedEthernet = false;
       }
     }
 
     delay(MQTT_CHECK_INTERVAL_MS);
   }
 }
 
 void connectToMqtt()
 {
   Serial.println("Connecting to MQTT...");
   mqttClient.connect();
 }
 
 void printSeparationLine()
 {
   Serial.println("************************************************");
 }
 

 void onMqttConnect(bool sessionPresent)
 {
   Serial.print("Connected to MQTT broker: ");
   Serial.print(MQTT_HOST);
   Serial.print(", port: ");
   Serial.println(MQTT_PORT);
   Serial.print("PubTopic: ");
   Serial.println(PubTopic);
 
   connectedMQTT = true;
 
   printSeparationLine();
   Serial.print("Session present: ");
   Serial.println(sessionPresent);
 
   uint16_t packetIdSub = mqttClient.subscribe(PubTopic, 2);
   Serial.print("Subscribing at QoS 2, packetId: ");
   Serial.println(packetIdSub);
 
   mqttClient.publish(PubTopic, 0, true, "Portenta_H7_Ethernet Test1");
   Serial.println("Publishing at QoS 0");
 
   uint16_t packetIdPub1 = mqttClient.publish(PubTopic, 1, true, "Portenta_H7_Ethernet Test2");
   Serial.print("Publishing at QoS 1, packetId: ");
   Serial.println(packetIdPub1);
 
   uint16_t packetIdPub2 = mqttClient.publish(PubTopic, 2, true, "Portenta_H7_Ethernet Test3");
   Serial.print("Publishing at QoS 2, packetId: ");
   Serial.println(packetIdPub2);
 
   printSeparationLine();
   
 }
 
 void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
 {
   (void) reason;
 
   connectedMQTT = false;
 
   Serial.println("Disconnected from MQTT.");
 }
 
 void onMqttSubscribe(const uint16_t& packetId, const uint8_t& qos)
 {
   Serial.println("Subscribe acknowledged.");
   Serial.print("  packetId: ");
   Serial.println(packetId);
   Serial.print("  qos: ");
   Serial.println(qos);
 }
 
 void onMqttUnsubscribe(const uint16_t& packetId)
 {
   Serial.println("Unsubscribe acknowledged.");
   Serial.print("  packetId: ");
   Serial.println(packetId);
 }
 
 void onMqttMessage(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties,
                    const size_t& len, const size_t& index, const size_t& total)
 {
  
  //Serial.println("Message received.");
  
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  String command = doc["CMD"];

  //Serial.println("Command: " + command);
  //Serial.println("Args: " + doc["ARGS"].as<String>());
  //JsonDocument Args = doc["ARGS"];
  

  

  if(command == "test")
  {int GPIO = doc["ARGS"]["GPIO"];
    bool State = doc["ARGS"]["VALUE"];
    //Serial.println("GPIO: " + String(GPIO));
  //Serial.println("State: " + String(State));

switch (GPIO)
{
case 1:
  digitalWrite(D0, State); //MOTOR
  break;

case 2:
  digitalWrite(D1, State);
  break;

case 3:
  digitalWrite(D2, State);
  break;

case 4:
  digitalWrite(D3, State);
  break;

  case 5:
  digitalWrite(D4, State);
  break;

default:
  break;
}

  }


if(command == "motor")
{
 
 mtx.lock();
 motor_degree = doc["ARGS"]["VALUE"];
 //Serial.println("Wake up led thread");
 condVar.notify_all();
mtx.unlock();
   
} 
  
  }
 
 void onMqttPublish(const uint16_t& packetId)
 {
   //Serial.println("Publish acknowledged.");
   //Serial.print("  packetId: ");
   //Serial.println(packetId);
 }
 
 void publishData()
 { int pin_A0 =0;
   float current = 0.0;
   float temp = 0.0;
   float out_current = 0.0;
   float out_temp = 0.0;
   Adafruit_MAX31865 thermo = Adafruit_MAX31865(7);
     
   thermo.begin(MAX31865_4WIRE);
   thermo.clearFault();
 
   std::vector<float> tempVector = {0};
   std::vector<float> currentVector;
 
   //dxl.begin(1000000);
   //dxl.setPortProtocolVersion(1.0);
   //dxl.torqueOn(DXL_ID);
   
 
   while(true)
   {//thermo.begin(MAX31865_4WIRE);  // set to 2WIRE or 4WIRE as necessary
     
     try{
       //dxl_test();
     if (connectedMQTT)
     {
       // Publish data to the topic
       JsonDocument pubdoc;
       pin_A0 = analogRead(A0);
       current = ((pin_A0 * 3.1) / 1023.0 - 1.47)*6.66667*1.667; // Convert to voltage -
       if(current < 0)
       {
         current = 0;
       }
       
       temp = thermo.temperature(RNOMINAL, RREF);
       /*if(temp < -100)
       {
         thermo = Adafruit_MAX31865(7);
         thermo.begin(MAX31865_4WIRE);  // set to 2WIRE or 4WIRE as necessary
 
       }
 */
 
 
       pubdoc["Current"] = current;
       pubdoc["Power"] = current * 11;
       pubdoc["Temperature"] = temp;
       pubdoc["RAW"] = pin_A0;
       dxl_mtx.lock();
       pubdoc["Motor_Position"] = dxl.getPresentPosition(DXL_ID, UNIT_DEGREE);
       //pubdoc["Motor_Current"] = dxl.getPresentCurrent(DXL_ID);
       //pubdoc["Motor_Velocity"] = dxl.getPresentVelocity(DXL_ID);
       dxl_mtx.unlock();
       mqttClient.publish(sensTopic, 0, false, pubdoc.as<String>().c_str());
       //Serial.println("Data published to topic: " + String(sensTopic));
     }
     ThisThread::sleep_for(100);
     //delay(2000); // Adjust the delay as needed
     //Serial.println("Executing publishData() thread...");
   }
   catch (const std::exception& e)
   {
     Serial.print("Error: ");
     Serial.println(e.what());
   }
   catch (...)
   {
     Serial.println("Unknown error occurred.");
   }
 }
 }
 
void dxl_loop()
{
  dxl.begin(1000000);
  dxl.setPortProtocolVersion(1.0);
  dxl.torqueOn(DXL_ID);
  mtx.lock();
  while (true)
  {

    condVar.wait();
    dxl_mtx.lock();
    //dxl.ledOn(DXL_ID);
    dxl.setGoalPosition(DXL_ID, motor_degree, UNIT_DEGREE);
    //delay(1000);
    //dxl.setGoalPosition(DXL_ID, 0);
    //dxl.ledOff(DXL_ID);
    //delay(1000);
    dxl_mtx.unlock();
  }
  mtx.unlock();
}

 void setup()
 {

  //Begin RTD
  //thermo.begin(MAX31865_4WIRE);  // set to 2WIRE or 4WIRE as necessary
  
  //Setup PINOUT
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  


   Serial.begin(9600);
 
   Serial.print("\nStarting FullyFeatured_PortentaH7_Ethernet on ");
   Serial.println(BOARD_NAME);
   Serial.println(ASYNC_MQTT_GENERIC_VERSION);
 
   // start the ethernet connection and the server
 
   // Use Static IP
   //Ethernet.begin(mac[index], ip);
   // Use DHCP dynamic IP and built-in mac
   Ethernet.begin();
   // Use DHCP dynamic IP and random mac
   //uint16_t index = micros() % NUMBER_OF_MAC;
   //Ethernet.begin(mac[index]);
 
   Serial.print("Connected to network. IP = ");
   Serial.println(Ethernet.localIP());
 
   connectedEthernet = true;
 
   mqttClient.onConnect(onMqttConnect);
   mqttClient.onDisconnect(onMqttDisconnect);
   mqttClient.onSubscribe(onMqttSubscribe);
   mqttClient.onUnsubscribe(onMqttUnsubscribe);
   mqttClient.onMessage(onMqttMessage);
   mqttClient.onPublish(onMqttPublish);
 
   mqttClient.setServer(MQTT_HOST, MQTT_PORT);
   delay(5000);
   Serial.println("Starting service");
   // Add "connectToMqttLoop" loop to control connection To Mqtt
   connectThread.start(connectToMqttLoop);
   publish_data_thread.start(publishData);

   dxl_thead.start(dxl_loop);
   connectToMqtt();
 }
 
 void loop()
 {
  
  
  //delay(5000);
  //mtx.lock();
  //Serial.println("Wake up led thread");
  //condVar.notify_all();
  //mtx.unlock();
  //delay(5000);
  //dxl_test();

  //dxl.setGoalPosition(DXL_ID, 512);
  
  //dxl_test();
  //dxl.setGoalPosition(DXL_ID, 0);
  //Serial.println("Motor Position set");
  //delay(1000);
/*
delay(1000);
 
   if (connectedMQTT)
   {
     digitalWrite(LEDR, HIGH);
     digitalWrite(LEDG, LOW);
     digitalWrite(LEDB, LOW);
   }
   else
   {
     digitalWrite(LEDR, LOW);
     digitalWrite(LEDG, HIGH);
     digitalWrite(LEDB, LOW);
   }
  delay(1000);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, HIGH);
  delay(1000);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW);
  delay(1000);
*/
 }
 