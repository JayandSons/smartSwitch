/*
  This a simple example of the aREST Library for the ESP32 WiFi chip.
  This example illustrate the cloud part of aREST that makes the board accessible from anywhere
  See the README file for more details.

  Written in 2017 by Marco Schwartz under a GPL license.
  
*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <aREST.h>

// Clients
WiFiClient client;
PubSubClient aRestClient(client);

// Create aREST instance
aREST rest = aREST(aRestClient);

// Unique ID to identify the device for cloud.arest.io
char* device_id = "MAC";

// WiFi parameters
char* ssid = "MyResNet Legacy";
char* password = "";

// Variables to be exposed to the API
int relayStatus;



void setup()
{
  pinMode(2, OUTPUT);
  // Start Serial
  Serial.begin(115200);

  // Set callback
  aRestClient.setCallback(callback);

  // Init variables and expose them to REST API
  relayStatus = 0;
  rest.variable("Relay Status", &relayStatus);


  // Function to be exposed
  rest.function("relay",relayControl);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id(device_id);
  rest.set_name("Farmette");

  // Connect to WiFi
  if(strlen(password))
  {
    WiFi.begin(ssid, password);
  }
  else
  {
    WiFi.begin(ssid);
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

}

void loop() {
  
  // Connect to the cloud
  rest.handle(aRestClient);
  
}

// Handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {

  rest.handle_callback(aRestClient, topic, payload, length);

}

// Custom function accessible by the API
int relayControl(String command) {

  // Get state from command
  int relayStatus = command.toInt();

  digitalWrite(2, relayStatus);
  return 1;
}
