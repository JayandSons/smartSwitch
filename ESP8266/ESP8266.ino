#include <ESP8266WiFi.h>
#include <EEPROM.h>

/************************************
Details sent to the chip from the app
************************************/
String homeWiFiName;          //Name of the network that the chip and the app will be in
int homeWiFiNameLength;
String homeWiFiPassword;      //Password of the above network
int homeWiFiPasswordLength; 
const int NUMBER_OF_THINGS_TO_BE_RECEIVED = 2; 


/*********************************************
Variables that is created and used in the chip
*********************************************/
String uuid;                           //uuid(MAC address) for the chip that will be used to control the chip over the cloud
unsigned char eepromStorage[100];      //Used as a buffer for the EEPROM functions
int userConnectedFlag;                 //Prevents from accessing two users getting connected to chip WiFi
int chipHasADefinedState;              //When this is one that means the chip has all the details to get connected to home WiFi and there is a user connected to it
WiFiServer server(80);                 //When the chip is an accesspoint it listens to port 80 for client requests
WiFiClient client;                     //To talk to the client connected
String request;                        //Buffer for what is sent from the app
String response;                       //Buffer for what is sent to the app
int checksum;                          //Storage for the checksum received with the request

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
  if(chipHasADefinedState)
  {
    //Acquiring the home WiFi network name from the EEPROM and storing it in the correponding variable
    eepromRead(1, homeWiFiNameLength, eepromStorage);
    homeWiFiName = charArrayToString(eepromStorage, homeWiFiNameLength);

    //Acquiring the home WiFi network name from the EEPROM and storing it in the correponding variable
    eepromRead(1, homeWiFiPasswordLength, eepromStorage);
    homeWiFiPassword = charArrayToString(eepromStorage, homeWiFiPasswordLength);

    //Acquiring the uuid of the chip from the EEPROM and storing it in the correponding variable
    eepromRead(1, WL_MAC_ADDR_LENGTH, eepromStorage);
    uuid = charArrayToString(eepromStorage, WL_MAC_ADDR_LENGTH);

    digitalWrite(0, HIGH);    //Make the LED solid and let the user know that the switch is online
  }
  //No previously defined state, so connect to the user app and get the details to go online
  else
  {
    int i;
    
    setupWiFi();              //Make the chip an access point acquire the uuid
    for(i = 0; i < NUMBER_OF_THINGS_TO_BE_RECEIVED; i++)
    {
      //Iterate till the right thing is received
      while(1)
      {
        LEDBlink();           //Letting the user know the switch is waiting to receive data from the app
        
        // Check if a client has connected
        client = server.available();
        if (!client)
        {
          continue;
        }

        //Expecting the name of the home WiFi
        if(i == 0)
        {
          //Check if what was sent was what was actually received, if it is update the corresponding variable, send confimraiton response, and break the loop
          if(requestCleanUp(&homeWiFiNameLength))
          {
            homeWiFiName = request;   

            //Send the response to the client
            response = "HTTP/1.1 406 OK\r\n";
            client.print(response);
            
            break;
          }
          //Checksum difference, ask for it again
          else
          {
            //Send the response to the client
            response = "HTTP/1.1 406 Not Acceptable\r\n";
            client.print(response);
            continue; 
          }
        }
        //Expecting the password of the home WiFi
        else
        {
          
        }
      }
    }
  }
}
void setup() {
  initComs();
  initPins();
}

int calcChecksum()
{
  int i;
  int localChecksum = 0;

  for(i = 0; i < request.length(); i++)
  {
    localChecksum += int(request.charAt(i));
  }

  return (localChecksum % 10);
}

bool requestCleanUp(int * howLong)
{
  int localChecksum;
  
  // Read the first line of the request
  request = client.readStringUntil('\r'); 

  request.remove(0, 5);                                           //Remove the http part
  request.remove(request.length() - 9, request.length());         //Remove the unnecessary tail
  checksum = request.charAt(request.length() - 1);          //Last character is the checksum value
  request.remove(request.length() - 1, request.length());         //Remove the checksum character to get the required data
  * howLong = request.length();                                   //Storing the length in the corresponding variable
  
  client.flush();

  localChecksum = calcChecksum();

  return (checksum == localChecksum);
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

