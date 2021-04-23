
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>

#include "System.h"
#include "arduino_secrets.h"
#include "Sensor.h"
#include "SensorMHZ14A.h"

static const int SENSORS_COUNT = 8;

int count = 0;

System ArduinoSystem;

Sensor* Sensors[SENSORS_COUNT];

WiFiMulti WifiMulti;
WebSocketsClient WebSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length)
{ 
  switch(type) 
  {
    case WStype_DISCONNECTED:
    {
      Serial.printf("[WSc] Disconnected!\n");
      break;
    }
    case WStype_CONNECTED:
    {
      Serial.printf("[WSc] Connected to url: %s\n", payload);      
      ArduinoSystem.WebSocketConnection(&WebSocket);      
      break;
    }
    case WStype_TEXT:
    {          
      Serial.printf("[WSc] get text: %s\n", payload);         
      ArduinoSystem.ReadCommand(String((char*)payload));
      break;
    }
    case WStype_BIN:
    {
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
    }
    case WStype_ERROR:      
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting");
  
  for(uint8_t t = 4; t > 0; t--) 
  {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }  

  SensorMHZ14A* sensorMHZ14A = new SensorMHZ14A(SENSOR_NAME, SENSOR_CHANNEL, DESCRIPTION, &ArduinoSystem);
  Sensors[count++] = sensorMHZ14A;
 
  ArduinoSystem.Initialize(&WifiMulti, Sensors, count, &WebSocket);

  // server address, port and URL
  WebSocket.begin(IP_SERVER, PORT, "/ws");
  
  // event handler
  WebSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  WebSocket.setReconnectInterval(5000);
}

void loop() 
{   
  ArduinoSystem.Run(&WebSocket);
  WebSocket.loop();
  delay(500);  
}
