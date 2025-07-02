#include <PortentaEthernet.h> 
#include <Ethernet.h>
#include <PubSubClient.h>

int port = 4000;

byte mac[] = {0x00,0x0A,0xAB,0x1A,0x1B,0x1D};

IPAddress IP(192,167,1,3);

IPAddress gateway(192,167,1,1);

IPAddress subnet(255,255,255,0);

IPAddress dns(192,167,1,1);

IPAddress server(192,167,1,2);

EthernetServer myserver(port);

EthernetClient client;

void setup() {
  Serial.begin(115200);

  if(Ethernet.begin(mac,IP,dns,subnet) == 0)
  {
    Serial.print("Fail to connect Ethernet");
  }
  else
  {
    Serial.println("Connected Sucess !");
    Serial.print("IP : ");
    Serial.println(Ethernet.localIP());
    Serial.print("MAC Address : ");
    Serial.println(Ethernet.macAddress());
  }


  Serial.println("TCP/IP Server started......//");
  Serial.print("Server IP : ");
  Serial.print(IP);
  Serial.print(" : LISTNING PORT : ");
  Serial.println(port);

  myserver.begin();
}

void loop() {

  EthernetClient client = myserver.available();
  if (client) {
    Serial.println("Client connected!");
    while (client.connected()) {
      // Serial.println("this is client connected state");
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        client.write(c); // Echo back
      }
    }
    Serial.println("Client disconnected.");
    client.stop();
  }

  // put your setup code here, to run once:

  // if(client.available() > 0)
  // {
  //   Serial.println(client.read());
  // }
  // unsigned long Result = Ethernet.ping(server);
  // if(Result > 0)
  // {
  //   Serial.print("Ping to ");
  //   Serial.print(server);
  //   Serial.print(" Sucessful . Round-trip time : ");
  //   Serial.print(Result);
  //   Serial.println("ms");
  // }
  // else
  // {
  //   Serial.print("Ping to ");
  //   Serial.print(server);
  //   Serial.println("Fail !");
  // }

  // Serial.print("Connecting to Server : ");
  // Serial.print(server);
  // Serial.print("PORT : ");
  // Serial.println(port);

  // if(client.connect(server,port))
  // {
    // Serial.println("Connected ....!");
    // client.println("Hello form Portenta");
  // }
  // else
  // {
  //   Serial.println("Connection Failed .......!");
  // }

  // if(client.available())
  // {
  // if(client.connected())
  // {
  //   Serial.println("Connected ....!");
  // if(client.available() > 0)
  // {
  //   Serial.println(client.read());
  // }
  //   // client.println("Hello form Portenta");
  // }
  // else
  // {
  //   Serial.println("Connection Failed .......!");
  // }
  
  // }
  // delay(1000);

}
