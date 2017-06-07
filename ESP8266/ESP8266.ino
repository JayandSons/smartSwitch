#include <ESP8266WiFi.h>
#include <EEPROM.h>

/************************************
Details sent to the chip from the app
************************************/
String homeWiFiName;                                      //Name of the network that the chip and the app will be in
int homeWiFiNameLength;
String homeWiFiPassword;                                  //Password of the above network
int homeWiFiPasswordLength; 
const int NUMBER_OF_THINGS_COMMUNICATED = 3;              //How many things the phone app and the chip share 


/*********************************************
Variables that is created and used in the chip
*********************************************/
String uuid;                           //uuid(MAC address) for the chip that will be used to control the chip over the cloud
unsigned char eepromStorage[100];      //Used as a buffer for the EEPROM functions
int userConnectedFlag;                 //Prevents from accessing two users getting connected to chip WiFi
unsigned char chipHasADefinedState;    //When this is one that means the chip has all the details to get connected to home WiFi and there is a user connected to it
WiFiServer server(80);                 //When the chip is an accesspoint it listens to port 80 for client requests
WiFiClient client;                     //To talk to the client connected
String request;                        //Buffer for what is sent from the app
String response;                       //Buffer for what is sent to the app


void LEDBlink()
{
  //If HIGH, turn LOW
  if(digitalRead(0))
    digitalWrite(0, LOW); 
  //If LOW, turn HIGH
  else                                   
    digitalWrite(0, HIGH);

  delay(750);
}

void defineAState()
{
  //Reading the flag for the state of the chip and updating the correponding state variable
  eepromRead(0, 1, eepromStorage);    
  chipHasADefinedState = eepromStorage[0];
  
  //This means the chip has a defined state and no need to connect to user app
  if(chipHasADefinedState == 1)
  {
    //Acquiring the home WiFi network name and length of it from the EEPROM and storing them in the correponding variables
    eepromRead(1, 1, eepromStorage);
    homeWiFiNameLength = eepromStorage[0];
    eepromRead(2, homeWiFiNameLength, eepromStorage);
    homeWiFiName = charArrayToString(eepromStorage, homeWiFiNameLength);
    
    //Acquiring the home WiFi network password and length of it from the EEPROM and storing them in the correponding variables
    eepromRead((2 + homeWiFiNameLength), 1, eepromStorage);
    homeWiFiPasswordLength = eepromStorage[0];
    eepromRead((3 + homeWiFiNameLength), homeWiFiPasswordLength, eepromStorage);
    homeWiFiPassword = charArrayToString(eepromStorage, homeWiFiPasswordLength);
    
    //Acquiring the uuid of the chip from the EEPROM and storing it in the correponding variable
    eepromRead((3 + homeWiFiNameLength + homeWiFiPasswordLength), (2 * WL_MAC_ADDR_LENGTH), eepromStorage);
    uuid = charArrayToString(eepromStorage, (2 * WL_MAC_ADDR_LENGTH));

    //Saving the changes made to EEPROM
    EEPROM.end();
  }
  //No previously defined state, so connect to the user app and get the details to go online
  else
  {
    int i = 0;
    
    setupWiFi();             //Letting the user know the chip has to be set up since it didn't have a previous state
    
    //Iterate till the right thing is received
    while(1)
    {
      LEDBlink();            //Letting the user know the switch is waiting to receive data from the app
      
      // Check if a client has connected
      client = server.available();
      if (!client)
      {
        continue;
      }
      
      //Expecting the name of the home WiFi
      if(i == 0)
      {
        requestCleanUp(&homeWiFiNameLength);
        homeWiFiName = request;   

        //Send the response to the client
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html\r\n\r\n";
        response += "<!DOCTYPE HTML>\r\n<html>\r\n";
        response += "Received";
        response += "</html>\n";
        client.print(response);
        client.stop();
        i++;
      }
      //Expecting the password of the home WiFi
      else if(i == 1)
      {
        requestCleanUp(&homeWiFiPasswordLength);
        homeWiFiPassword = request;   
     
        //Send the same previous response to the client
        client.print(response);
        client.stop();
        i++;
      }
      //Reponding the phone's request to obtain the uuid
      else
      {
        //Send the response to the client
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html\r\n\r\n";
        response += "<!DOCTYPE HTML>\r\n<html>\r\n";
        response += uuid;
        response += "</html>\n";
        client.print(response);
        client.stop();
        i++;
      }
      //Received both things, break out of the loop
      if(i == NUMBER_OF_THINGS_COMMUNICATED)
      {
        chipHasADefinedState = 1;
        break;
      }
    }

    //Communication between the phone and chip is done. Write the data to EEPROM
    eepromStorage[0] = chipHasADefinedState;
    eepromWrite(0, 1, eepromStorage);
    
    eepromStorage[0] = homeWiFiNameLength;
    eepromWrite(1, 1, eepromStorage);
    eepromStorageLoad(homeWiFiName, eepromStorage);
    eepromWrite(2, homeWiFiNameLength, eepromStorage);

    eepromStorage[0] = homeWiFiPasswordLength;
    eepromWrite((2 + homeWiFiNameLength), 1, eepromStorage);
    eepromStorageLoad(homeWiFiPassword, eepromStorage);
    eepromWrite((3 + homeWiFiNameLength), homeWiFiPasswordLength, eepromStorage);

    //No need to store the size of the MAC ID because it is a constant 
    eepromStorageLoad(uuid, eepromStorage);
    eepromWrite((3 + homeWiFiNameLength + homeWiFiPasswordLength), (2 * WL_MAC_ADDR_LENGTH), eepromStorage);
    
    //Saving the changes made to EEPROM
    EEPROM.end();
  }
}

void setup() {
  initComs();
  initPins();
  defineAState();
  Serial.println("Going in here");
}


void requestCleanUp(int * howLong)
{
  //Read the first line of the request
  request = client.readStringUntil('\r'); 
  client.flush();
  
  request.remove(0, 5);                                           //Remove the http part
  request.remove(request.length() - 9, request.length());         //Remove the unnecessary tail
  * howLong = request.length();                                   //Storing the length in the corresponding variable
}

void loop() {
 
}

void initComs()
{
  Serial.begin(115200);       //This is for debugging purposes?????????????????????????????????????????????
  EEPROM.begin(512);          //Starting the link with the EEPROM
}

void initPins()
{
  //LED
  pinMode(0, OUTPUT);         
  digitalWrite(0, LOW);

  //Relay
  pinMode(2, OUTPUT); 

  //Reset switch
  pinMode(3, INPUT);               
}

void setupWiFi()
{ 
  WiFi.mode(WIFI_AP);         //Making the chip an access point
  obtainingMACID();           //Acquring MAC ID
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
    eepromStorage[i] = EEPROM.read(i + startingAddress);
  }
}

void eepromWrite(int startingAddress, int numberOfBytes, unsigned char * eepromStorage)
{
  int i;
  
  for(i = 0; i < numberOfBytes; i++)
  {
    EEPROM.write((i + startingAddress), eepromStorage[i]);
  }
}

void eepromStorageLoad(String source, unsigned char * eepromStorage)
{
  int i;
  
  for(i = 0; i < source.length(); i++)
  {
    eepromStorage[i] = source.charAt(i);
  }
}

String charArrayToString(unsigned char * charArrayToBeConverted, int numberOfCharacters)
{
  String temp;
  for(int i = 0; i < numberOfCharacters; i++)
  {
    temp += String(char (charArrayToBeConverted[i]));       
  }
  return temp;
}
