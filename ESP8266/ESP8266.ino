#include <ESP8266WiFi.h>
#include <EEPROM.h>

/************************************
Details sent to the chip from the app
************************************/
String homeWiFiName;          //Name of the network that the chip and the app will be in
int homeWiFiNameLength;
String homeWiFiPassword;      //Password of the above network
int homeWiFiPasswordLength;  
String appPassword;           //Password of the user app which will be combined with the MAC of the chip to produce the uuid
int appPasswordLength;

/*********************************************
Variables that is created and used in the chip
*********************************************/
String uuid;                           //uuid(MAC + password of the app) for the chip that will be used to control the chip over the cloud
unsigned char eepromStorage[100];      //Used as a buffer for the EEPROM functions
int userConnectedFlag;                 //Prevents from accessing two users getting connected to chip WiFi
WiFiServer server(80);                 //When the chip is an accesspoint it listens to port 80 for client requests

void setup() {
  String temp = "SomethingElse";
  
  initPins();
  Serial.println(temp);
  setupWiFi();
  EEPROM.begin(512);
  homeWiFiName = "myass";
  eepromStorageLoad(homeWiFiName, eepromStorage, 0);
  eepromWrite(0, homeWiFiName.length(), eepromStorage);
  eepromRead(0, homeWiFiName.length(), eepromStorage);
  temp = charArrayToString(eepromStorage, homeWiFiName.length());
  Serial.println(temp);
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

void initPins()
{
  Serial.begin(115200);       //This is for debugging purposes?????????????????????????????????????????????
  pinMode(0, OUTPUT);         //Making GPIO0 an output for the LED
  digitalWrite(0, LOW);
  pinMode(2, OUTPUT);         //Making GPIO2 an output for the relay       
}

void setupWiFi()
{ 
  WiFi.mode(WIFI_AP);         //Making the chip an access point
  obtainingMACID();           //Acquring MAC ID
  Serial.println(uuid);       //This is for debugging purposes?????????????????????????????????????????????
  WiFi.softAP("Farmette");    //Creating a WiFi network without any password
  server.begin();             //Starting the WiFi network
}

void obtainingMACID()
{
  //Obtaining the mac in its raw form
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);

  //Converting the above to a upper case hex string
  String temp;
  for(int i = 0; i < WL_MAC_ADDR_LENGTH; i++){
    temp = "0";
    //When converted to binary if the 4 MSBs are zeroes include the leading zero manually 
    if(mac[i] < 16){
      temp += String(mac[i], HEX);
      uuid += temp; 
    }
    else
      uuid += String(mac[i], HEX);       
  }
  uuid.toUpperCase();
}

void eepromRead(int startingAddress, int numberOfBytes, unsigned char * eepromStorage)
{
  int i;
  
  for(i = 0; i < numberOfBytes; i++)
  {
    eepromStorage[i] = EEPROM.read(i);
  }
}

void eepromWrite(int startingAddress, int numberOfBytes, unsigned char * eepromStorage)
{
  int i;
  
  for(i = 0; i < numberOfBytes; i++)
  {
    EEPROM.write(i, eepromStorage[i]);
  }
}

void eepromStorageLoad(String source, unsigned char * eepromStorage, int clear)
{
  int i;
  
  for(i = 0; i < source.length(); i++)
  {
    //Clearing the EEPROM
    if(clear)
    {
      eepromStorage[i] = 255;   //Clear value is 255 because no ASCII character is 255
    }
    //Writing source buffer
    else
      eepromStorage[i] = source.charAt(i);
  }
}

String charArrayToString(unsigned char * temp, int big)
{
  String tempi;
  for(int i = 0; i < big; i++)
  {
    tempi += String(char (temp[i]));       
  }
  return tempi;
}

