#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>


// Local Network Settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // BIA or Physical mac address of W5100 Shield     
byte ip[] = { 192, 168, 1, 96 };              
byte gateway[] = { 192, 168, 1, 1 };                   // internet access via router
byte subnet[] = { 255, 255, 255, 0 };                  // 24bit Subnet Mask
//EthernetServer server(80);                           //server port     

//byte gateway[] = { 192, 168, 0, 1 }; //Internet access via router
//byte subnet[] = { 255, 255, 255, 0 }; //24 Bit Subnet Mask 
//byte myserver[] = { 208, 104, 2, 86 }; // zoomkat web page server IP addre
// ThingSpeak Settings

char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "W9B41J0P1TTBFL85"; 
const int updateThingSpeakInterval = 15 * 1000;      // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

// Setup the variable

long lastConnectionTime = 0; 
boolean lastConnected = false;
int failedCounter = 0;

// Initialize Ethernet Client
EthernetClient client;

Adafruit_BMP085 bmp;

int VbatPin1 = A0;
int VbatPin2 = A1;
float vout1 = 0.0;
float vin1 = 0.0;
float vout2 = 0.0;
float vin2 = 0.0;
float R1 = 30000.0; 
float R2 = 7500.0;  
int value1 = 0;
int value2 = 0;

long pulseCount = 1;   //Number of pulses, used to measure energy.
unsigned long pulseTime,lastTime; //Used to measure power.

volatile float power, elapsedkWh;  //power and energy
volatile float ppwh = 1; //Number of pulses per wh - found or set



void onPulse() {
  lastTime = pulseTime;
  pulseTime = micros();
  pulseCount++;
  power = (3600000000.0 / (pulseTime - lastTime))/ppwh;   //Calculate power
  elapsedkWh = (1.0*pulseCount/(ppwh /*  *1000*/)); //Find kwh elapsed  //multiply by 1000 to convert pulses per wh to kwh
  Serial.print("Power: ");
  Serial.println(power,0);
 // Serial.print(" ");
  //Serial.println(elapsedkWh,0); 
  
}

void setup()
{
  // Start Serial for debugging on the Serial Monitor
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(3), onPulse, FALLING);
  if (!bmp.begin()) {
  Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  while (1) {}
  }
 // start the Ethernet connection and the server:
  Ethernet.begin(ip,mac,gateway, subnet);
//  server.begin();
  Serial.println("Arduino is ready to work!");
  pinMode(VbatPin1, INPUT);
  pinMode(VbatPin2, INPUT);
  // Start Ethernet on Arduino
  
  startEthernet();
}

void loop()
{
   value1 = analogRead(VbatPin1);
   vout1 = (value1 * 4.16) / 1024.0; // see text
   vin1 = (vout1 / (R2/(R1+R2))); //-2.67
   String Vbat1 = String(vin1, 2);
   
   value2 = analogRead(VbatPin2);
   vout2 = (value2 * 4.16) / 1024.0; // see text
   vin2 = (vout2 / (R2/(R1+R2)));// -2.6
   String Vbat2 = String(vin2, 2);
   Serial.print("Voltage1: ");
   Serial.println(Vbat1);
   Serial.print("Voltage2: ");
   Serial.println(Vbat2);
  
   float temp=bmp.readTemperature();
   float pres=bmp.readPressure()/100;
   Serial.print("Temp: ");
   Serial.println(temp);
   Serial.print("Pressure: ");
   Serial.println(pres);
  
  if (client.available())
  {
    char c = client.read();
    Serial.print(c);
  }

  // Disconnect from ThingSpeak
  if (!client.connected() && lastConnected)
  {
    Serial.println("...disconnected");
    Serial.println();
    
    client.stop();
  }
  
  // Update ThingSpeak
  if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
  {
    //updateThingSpeak("field1="+Vbat);
     updateThingSpeak("&field1="+Vbat1+"&field2="+Vbat2+"&field3="+power+"&field4="+temp+"&field5="+pres);
     
  }
  
  // Check if Arduino Ethernet needs to be restarted
  if (failedCounter > 3 ) {startEthernet();}
  
  lastConnected = client.connected();
}

void updateThingSpeak(String tsData)
{
  if (client.connect(thingSpeakAddress, 80))
  {         
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+ writeAPIKey +"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");

    client.print(tsData);
    
    lastConnectionTime = millis();
    
    if (client.connected())
    {
      Serial.println("Connecting to ThingSpeak Server");
      Serial.println();
      
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
  
      Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");   
      Serial.println();
    }
    
  }
  else
  {
    failedCounter++;
    
    Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");   
    Serial.println();
    
    lastConnectionTime = millis(); 
  }
}

void startEthernet()
{
  
  client.stop();

  Serial.println("Connecting to current network please wait...");
  Serial.println();  

  delay(1000);
  
  // Connect to network amd obtain an IP address using DHCP
  
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("DHCP Failed, reset the device and try again");
    Serial.println();
  }
  else
  {
    Serial.println("Connected to network w/d DHCP"); 
    Serial.println();
  }
  
  delay(1000); // Delay at 1 Seconds
}
