#include <ESP8266WiFi.h>
char uuid[WL_MAC_ADDR_LENGTH];

WiFiServer server(80);
void setup() {
  initHardware();
  setupWiFi();
  server.begin();

}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  req.remove(0, 5);
  req.remove(req.length() - 9, req.length());
  Serial.println(req);
  client.flush();

  if (req.indexOf("/led/0") != -1)
    Serial.println(req);
  
  client.flush();

  // Prepare the response. Start with the common header:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";

  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  Serial.println("Client disonnected");
}

void initHardware()
{
  Serial.begin(115200);
}

void setupWiFi()
{
  //Making the ESP an access point 
  WiFi.mode(WIFI_AP);

  //Obtaining the mac in its raw form
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);

  //Converting the above to a upper case hex string
  String macID;
  String temp;
  for(int i = 0; i < WL_MAC_ADDR_LENGTH; i++){
    temp = "0";
    if(mac[i] < 16){
      temp += String(mac[i], HEX);
      macID += temp; 
    }
    else
      macID += String(mac[i], HEX);       
  }
  macID.toUpperCase();

  Serial.println(macID);  
  
  WiFi.softAP("Water Pump");
}
