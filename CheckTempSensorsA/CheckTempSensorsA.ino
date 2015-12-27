#include <OneWire.h> 
#include <DallasTemperature.h>
 
float temp1;
float temp2;
float temp3;
float temp4;
float temp5;
float temp6;
 
 
#define ONE_WIRE_BUS_PIN A2 //OneWire sensor pin (middle pin) wired into pin A2 on Arduino
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS_PIN);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
 
DeviceAddress Probe06 = { 0x28, 0x50, 0xAD, 0x8F, 0x05, 0x00, 0x00, 0x99 };
DeviceAddress Probe11 = { 0x28, 0x26, 0xDA, 0x91, 0x05, 0x00, 0x00, 0xA2 };
DeviceAddress Probe8 = { 0x28, 0x11, 0xC1, 0x8F, 0x05, 0x00, 0x00, 0xEB };
DeviceAddress Probe4 = { 0x28, 0xE3, 0x79, 0x7C, 0x05, 0x00, 0x00, 0x7F };
DeviceAddress Probe5 = { 0x28, 0x19, 0x7C, 0x8E, 0x05, 0x00, 0x00, 0x66 };
DeviceAddress Probe13 = { 0x28, 0x7C, 0xD3, 0x91, 0x05, 0x00, 0x00, 0xB0 }; //Lounge on valve 8
 
void setup() {
 
  Serial.begin(9600);    // initialize serial  
 
 
 
  
 // Initialize the Temperature measurement library
  sensors.begin();
  
  // set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
  sensors.setResolution(Probe06, 10);
   sensors.setResolution(Probe11, 10);
    sensors.setResolution(Probe8, 10);
     sensors.setResolution(Probe4, 10);
      sensors.setResolution(Probe5, 10);
        sensors.setResolution(Probe13, 10);
  
 
}
 
 
void loop() {
  sensors.requestTemperatures();  
  temp1 = sensors.getTempC(Probe06);
  temp2 = sensors.getTempC(Probe11);
  temp3 = sensors.getTempC(Probe8);
  temp4 = sensors.getTempC(Probe4);
  temp5 = sensors.getTempC(Probe5);
  temp6 = sensors.getTempC(Probe13);
 
  
 Serial.println(temp1);
 Serial.println(temp2);
 Serial.println(temp3);
 Serial.println(temp4);
 Serial.println(temp5);
 Serial.println(temp6);
 
 
}
