/**
 * @file RuntimeMeterExample.cpp
 * @author Gabriel Sieben (gsieben@geogab.net)
 * @brief This is a quck and dirty example for a webserver integration. 
 * @version 0.1
 * @date 2021-02-14
 * 
 * @copyright Copyright (c) 2021
 * 
 * @licenze MIT license (see LICENSE file)
 */
#include "WebserverExample.h"

//RuntimeMeter rtmeter(3,RT_MEASURE_MICROS);          // 3 Slots but measure in micro seconds instead of cpu ticks (RT_MEASURE_TICKS) which is default. This is less accurate but longer time intervals can be measured.
RuntimeMeter rtmeter(5);
ESP8266WebServer server(80);

// Global Variables
#define MAXSLOTS 5

struct data_t {
  struct measurePoint_t {
    String Name="";                   // Slot Name
    uint32_t Runtime=0;               // Runtime
  } slots[MAXSLOTS];
  uint8_t No;
  uint16_t CPUf;
  uint32_t Runtime=0;
} data;

/****** Setup ******/
void setup() {
  Serial.begin(115200);
  Serial.println("\r\n### G e o G a b ###");

  WiFi.begin("Your Wifi Router", "Wifi Password");     // Please insert your network settings
  
  Serial.print("Connecting to WiFi: ");
  while (WiFi.status() != WL_CONNECTED) {       // Wait for the WiFi connection
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.print("Webpage: ");
  Serial.print("http://");
  Serial.println(WiFi.localIP());               //Print the local IP
  
  server.on("/", SendIndex);                    //Associate the handler function to the path
  server.on("/json", JsonAnswer);
  server.begin();                               //Start Webserver
}

/****** Main Loop ******/
void loop() {
  rtmeter.Add("Code A");              // Measurement Point 1 (Start)
  // Your Code A
  uint32_t b;
    for (uint32_t a=0;a<50;a++) {
      b=acos(a);
    } 

  rtmeter.Add("Code B");              // Measurement Point 2 (caculate runtime between Point1 and Point2)
  // Your Code B
    for (uint32_t a=0;a<200;a++) {
      b=acos(a);
    } 

  rtmeter.Add("Webserver");           // Measurement Point 2 (caculate runtime between Point1 and Point2)
  // Webserver Code
  server.handleClient();              // Handling of incoming requests

  rtmeter.Finalize();                 // Last Measure Point and finalize calculations. The final() is mandatory!!! 

  // Save the data
  uint8_t index=0;
  // Measure Point 1 (Code A)
  data.slots[index].Runtime = rtmeter.mpslots[index].Runtime;
  data.slots[index].Name = rtmeter.mpslots[index].Name;
  
  // Measure Point 2 (Code B)
  index++;  // 1
  data.slots[index].Runtime = rtmeter.mpslots[index].Runtime;
  data.slots[index].Name = rtmeter.mpslots[index].Name;

  // Measure Point 3 (Webserver)
  index++;  // 2
  data.slots[index].Runtime = rtmeter.mpslots[index].Runtime;
  data.slots[index].Name = rtmeter.mpslots[index].Name;

  // Measure Point 3 (Kernel)
  index++;  // 3
  data.slots[index].Runtime = rtmeter.overall.kernel.Runtime;
  data.slots[index].Name = "Kernel";

  data.No= index+1;
  data.CPUf= rtmeter.overall.cpuf;
  data.Runtime=rtmeter.loop.Runtime;    // overall runtime

  rtmeter.loop.Runtime=b;     // This does not make any sense. But it still serves a purpose in this example code. The purpose of this line is only to make the optimizer think the above loops are not meaningless. This variable will be deleted in the next step anyway. 
}

void JsonAnswer() {
  // Prepare Data
  StaticJsonDocument<512> jout;
  jout["no"]=data.No;
  jout["CPUf"]=data.CPUf;
  jout["Runtime"]=data.Runtime;
  JsonArray slots = jout.createNestedArray("slots");

  for (uint8_t i=0;i<data.No;i++) {
      JsonObject sub = slots.createNestedObject();
      sub["Name"] = data.slots[i].Name;
      sub["Runtime"] = data.slots[i].Runtime;
  }

  // Send Data
  WiFiClient client = server.client();
  // Write response headers
  client.println(F("HTTP/1.0 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.print(F("Content-Length: "));
  client.println(measureJson(jout));
  client.println();

  //serializeJsonPretty(jout, Serial);                      // Just for testing
  serializeJson(jout, client);                              // Write JSON document
  client.stop();                                            // Disconnect from Client
  jout.clear();                                             // Clear Json memory
}

void SendIndex() {
  server.send(200, "text/html", MAIN_page);
} 