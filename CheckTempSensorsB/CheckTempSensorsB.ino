#include <OneWire.h> 
#include <DallasTemperature.h>
 
float temp1;
float temp2;
float temp3;
float temp4;
float temp5;
 
 
#define ONE_WIRE_BUS_PIN A5
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS_PIN);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
 
DeviceAddress Probe2 = { 0x28, 0x52, 0xB1, 0x8F, 0x05, 0x00, 0x00, 0xAA }; //Master Bedroom 1
DeviceAddress Probe8 = { 0x28, 0x95, 0x0D, 0x92, 0x05, 0x00, 0x00, 0x7B }; //Dining Room 7
DeviceAddress Probe11 = { 0x28, 0xAA, 0xFF, 0x7C, 0x05, 0x00, 0x00, 0xCB }; //Hallway 10
DeviceAddress Probe9 = { 0x28, 0x3D, 0x1A, 0x8F, 0x05, 0x00, 0x00, 0x0E }; //Conservatory
DeviceAddress Probe3 = { 0x28, 0xA4, 0x0F, 0x92, 0x05, 0x00, 0x00, 0x22 }; //Guest Bedroom 2
DeviceAddress Probe10 = { 0x28, 0x99, 0xFB, 0x91, 0x05, 0x00, 0x00, 0xB4 }; //Tank 9
 
 
void setup() {
 
  Serial.begin(9600);    // initialize serial  
 
 
 
  
 // Initialize the Temperature measurement library
  sensors.begin();
  
  // set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
  sensors.setResolution(Probe2, 10);
   sensors.setResolution(Probe11, 10);
    sensors.setResolution(Probe8, 10);
     sensors.setResolution(Probe9, 10);
      sensors.setResolution(Probe3, 10);
      sensors.setResolution(Probe10, 10);
  
 
}
 
 
void loop() {
  sensors.requestTemperatures();  
  temp1 = sensors.getTempC(Probe2);
  temp2 = sensors.getTempC(Probe11);
  temp3 = sensors.getTempC(Probe8);
  temp4 = sensors.getTempC(Probe9);
  temp5 = sensors.getTempC(Probe3);
  temp5 = sensors.getTempC(Probe10);
 
  
 Serial.println(temp1);
 Serial.println(temp2);
 Serial.println(temp3);
 Serial.println(temp4);
 Serial.println(temp5);
 
 
}

