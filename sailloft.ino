// This #include statement was automatically added by the Particle IDE.
#include "RelayShield/RelayShield.h"
// Create an instance of the RelayShield library, so we have something to talk to
RelayShield myRelays;

// Create prototypes of the Spark.functions we'll declare in setup()
int relayOn(String command);
int relayOff(String command);
int isRelayOn(String command);
float hysteresis = 0.5;
int led2 = D7;
/*
The following is required to repair a problem with the photon when it is flashing cyan/occasionally also with red:
particle keys new
particle keys load device.pem
particle keys send <device_id> device.pub.pem

// grab this file https://s3.amazonaws.com/spark-website/cloud_public.der
particle keys server cloud_public.der
*/

/*
Use this sketch to read the temperature from 1-Wire devices
you have attached to your Particle device (core, p0, p1, photon, electron)

Temperature is read from: DS18S20, DS18B20, DS1822, DS2438

Expanding on the enumeration process in the address scanner, this example
reads the temperature and outputs it from known device types as it scans.

I/O setup:
These made it easy to just 'plug in' my 18B20

D3 - 1-wire ground: black wire from temp sensors which'll be connected to Digital Pin 3 on the Photon (driven LOW to GND)
D4 - 1-wire signal, with a 2K-10K pull up resistor bridged across to D5 (which is driven HIGH to provide 5v PWR)
D5 - 1-wire power, connected to Digital pin 5 on the Photon (driven to HIGH)

A pull-up resistor is required on the signal line. The spec calls for a 4.7K.
I have used 1K-10K depending on the bus configuration and what I had out on the
bench. If you are powering the device, they all work. If you are using parisidic
power it gets more picky about the value.

*/

// Only include One of the following depending on your environment!
#include "OneWire/OneWire.h"  // Use this include for the Web IDE:
// #include "OneWire.h" // Use this include for Particle Dev where everything is in one directory.


// Comment this out for normal operation
// SYSTEM_MODE(SEMI_AUTOMATIC);  // skip connecting to the cloud for (Electron) testing

OneWire ds = OneWire(A4);  // 1-wire signal on pin D4

unsigned long lastUpdate = 0;
char zone1temp[40];
char zone2temp[40];
char zone3temp[40];
char zone4temp[40];
char zone5temp[40];



void setup() {
  Serial.begin(9600);
  // Set up 'power' pins, comment out if not used!
  pinMode(A3, OUTPUT);
  pinMode(A5, OUTPUT);
  digitalWrite(A3, HIGH);
  digitalWrite(A5, LOW);
  Particle.variable("zone1temp", zone1temp);
  Particle.variable("zone2temp", zone2temp);
  Particle.variable("zone3temp", zone3temp);
  Particle.variable("zone4temp", zone4temp);
  Particle.variable("zone5temp", zone5temp);
  
  pinMode(led2, OUTPUT);
  
     myRelays.begin(2);

    // Use myRelays.begin(2); if you have the square, white RelayShield (from the Core)
    // to use, just add a '2' between the parentheses in the code above.

    // Register Spark.functions and assign them names
    Spark.function("relayOn", relayOn);
    Spark.function("relayOff", relayOff); 
    Spark.function("isRelayOn", isRelayOn);
  
  
}

// up to here, it is the same as the address acanner
// we need a few more variables for this example

void loop(void) {
  byte i;
  byte present = 0;
  byte type_s = 0;
  byte data[12];
  byte addr[8];
  float celsius;
  

  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }

  // The order is changed a bit in this example
  // first the returned address is printed

  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  // second the CRC is checked, on fail,
  // print error and just return to try again

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();

  // we have a good address at this point

  // this device has temp so let's read it

  ds.reset();               // first clear the 1-wire bus
  ds.select(addr);          // now select the device we just found
  // ds.write(0x44, 1);     // tell it to start a conversion, with parasite power on at the end
  ds.write(0x44, 0);        // or start conversion in powered mode (bus finishes low)

  // just wait a second while the conversion takes place
  // different chips have different conversion times, check the specs, 1 sec is worse case + 250ms
  // you could also communicate with other devices if you like but you would need
  // to already know their address to select them.

  delay(1000);     // maybe 750ms is enough, maybe not, wait 1 sec for conversion
  
  // we might do a ds.depower() (parasite) here, but the reset will take care of it.

  // first make sure current values are in the scratch pad

  present = ds.reset();
  ds.select(addr);
  ds.write(0xB8,0);         // Recall Memory 0
  ds.write(0x00,0);         // Recall Memory 0

  // now read the scratch pad

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE,0);         // Read Scratchpad
  if (type_s == 2) {
    ds.write(0x00,0);       // The DS2438 needs a page# to read
  }

  // transfer and print the values

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s == 2) raw = (data[2] << 8) | data[1];
  byte cfg = (data[4] & 0x60);

  switch (type_s) {
    case 1:
      raw = raw << 3; // 9 bit resolution default
      if (data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - data[6];
      }
      celsius = (float)raw * 0.0625;
      break;
    case 0:
      // at lower res, the low bits are undefined, so let's zero them
      if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
      if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
      if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
      // default is 12 bit resolution, 750 ms conversion time
      celsius = (float)raw * 0.0625;
      break;

    case 2:
      data[1] = (data[1] >> 3) & 0x1f;
      if (data[2] > 127) {
        celsius = (float)data[2] - ((float)data[1] * .03125);
      }else{
        celsius = (float)data[2] + ((float)data[1] * .03125);
      }
  }

  Serial.print("  Temperature = ");
  Serial.print(celsius);
  
  
    switch (addr[2]) {
    case 0xA4:
      Serial.println("  Sensor number 5");
      sprintf(zone5temp, "%.2f", celsius);
      if ((Time.hour()>=8 && Time.hour()<=18) && (Time.weekday()!=1 || Time.weekday()!=7) && (celsius < (28 - hysteresis)))
      {
          digitalWrite(led2, HIGH);    // if it's between 8am and 5pm and not a saturday(1) or a sunday(7) weekday and below 24.5 degrees, then turn on led2. 
      }
      else if ((Time.hour()>=8 && Time.hour()<=18) && (Time.weekday()!=1 || Time.weekday()!=7) && (celsius > (28 + hysteresis)))
      {
          digitalWrite(led2, LOW);   // else if its working hours and too hot then turn it off.
      }
      else if ((Time.hour()>18) || (Time.weekday()==1) || (Time.weekday()==7))
      {
          digitalWrite(led2, LOW);   // else if it's past home time or it's Sat or Sun then turn it off.
      }
      break;
    case 0x16:
      Serial.println("  Sensor number 4");
      sprintf(zone4temp, "%.2f", celsius);
      break;
    case 0x22:
      Serial.println("  Sensor number 3");
      sprintf(zone3temp, "%.2f", celsius);
      break;
    case 0x31:
      Serial.println("  Sensor number 2");
      sprintf(zone2temp, "%.2f", celsius);
      break;
    case 0x44:
      Serial.println("  Sensor number 1");
      sprintf(zone1temp, "%.2f", celsius);
      break;
    default:
      Serial.println("Unknown device type.");
      return;
  }
  
}


int isRelayOn(String command){
    // Ritual incantation to convert String into Int
    char inputStr[64];
    command.toCharArray(inputStr,64);
    int i = atoi(inputStr);
    
    
    if(myRelays.isOn(i))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int relayOn(String command){
    // Ritual incantation to convert String into Int
    char inputStr[64];
    command.toCharArray(inputStr,64);
    int i = atoi(inputStr);
    
    // Turn the desired relay on
    myRelays.on(i);
    
    // Respond
    return 1;    
}

int relayOff(String command){
    // Ritual incantation to convert String into Int
    char inputStr[64];
    command.toCharArray(inputStr,64);
    int i = atoi(inputStr);
    
    // Turn the desired relay off
    myRelays.off(i);
    
    // Respond
    return 1;    
}
