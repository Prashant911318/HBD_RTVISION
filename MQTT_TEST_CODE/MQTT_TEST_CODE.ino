#include <PortentaEthernet.h>
#include <Ethernet.h>
#include <PubSubClient.h>

const char *ClientID = "ArduinoPortenta";

byte mac[] = {0x00, 0x0A, 0xAB, 0x1A, 0x1B, 0x1D};

IPAddress MyIp(192, 167, 1, 3);
IPAddress gateway(192, 167, 1, 1);
IPAddress dns(192, 167, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress brokerIp(192,167,1,2);

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Ethernet connection starting...");

  Ethernet.begin(mac, MyIp, dns, gateway, subnet);

  Serial.println("Ethernet connected.");
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("MAC: ");
  Serial.println(Ethernet.macAddress());

  mqttClient.setServer(brokerIp, 1884);
}

void loop() {
  if (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (mqttClient.connect(ClientID)) {
      Serial.println("Connected to MQTT broker!");

      // Publish IP string
      char ipStr[16];
      sprintf(ipStr, "%d.%d.%d.%d", MyIp[0], MyIp[1], MyIp[2], MyIp[3]);
      mqttClient.publish("Test_massage/massage", ipStr);
    } else {
      Serial.print("MQTT connect failed. State: ");
      Serial.println(mqttClient.state());
      delay(5000);
      return;
    }
  }
  mqttClient.publish("Test_massage/massage", "Hello i am portenta IP : 192.167.1.3 :: ");
  mqttClient.loop();
  delay(1000);
}

