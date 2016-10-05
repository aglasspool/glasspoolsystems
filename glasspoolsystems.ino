#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Process.h>


Process date;                 // process used to get the date
int hours, minutes, seconds;  // for the results
int lastSecond = -1;          // need an impossible value for comparison
boolean isHomeTime = false;
boolean isNurseryTime = false;
float hysteresis = 0.5;     //Ian - this is where you change the hysterisis. It's currently set to 0.5 degrees either side of desired room temp (desiredTemp)


/*-----( Declare Constants and Pin Numbers )-----*/
#define ONE_WIRE_BUS_A A4
#define ONE_WIRE_BUS_B A5
#define ONE_WIRE_BUS_C A2

/*-----( Declare objects )-----*/
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWireA(ONE_WIRE_BUS_A);
OneWire oneWireB(ONE_WIRE_BUS_B);
OneWire oneWireC(ONE_WIRE_BUS_C);


// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensorsA(&oneWireA);
DallasTemperature sensorsB(&oneWireB);
DallasTemperature sensorsC(&oneWireC);


/*-----( Declare Variables )-----*/
// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html

DeviceAddress Probe6 = { 0x28, 0x50, 0xAD, 0x8F, 0x05, 0x00, 0x00, 0x99 }; //Office on valve 5
DeviceAddress Probe7 = { 0x28, 0x26, 0xDA, 0x91, 0x05, 0x00, 0x00, 0xA2 }; //Nursery on valve 6
DeviceAddress Probe12 = { 0x28, 0x11, 0xC1, 0x8F, 0x05, 0x00, 0x00, 0xEB }; //Upstairs Bathroom on valve 11
DeviceAddress Probe5 = { 0x28, 0xE3, 0x79, 0x7C, 0x05, 0x00, 0x00, 0x7F }; //Utility on valve 4
DeviceAddress Probe4 = { 0x28, 0x19, 0x7C, 0x8E, 0x05, 0x00, 0x00, 0x66 }; //Kitchen on valve 3
DeviceAddress Probe13 = { 0x28, 0x7C, 0xD3, 0x91, 0x05, 0x00, 0x00, 0xB0 }; //Lounge on valve 8

DeviceAddress Probe2 = { 0x28, 0x52, 0xB1, 0x8F, 0x05, 0x00, 0x00, 0xAA }; //Master Bedroom on valve 1
DeviceAddress Probe8 = { 0x28, 0x95, 0x0D, 0x92, 0x05, 0x00, 0x00, 0x7B }; //Dining Room on valve 7
DeviceAddress Probe11 = { 0x28, 0xAA, 0xFF, 0x7C, 0x05, 0x00, 0x00, 0xCB }; //Hallway on valve 10
DeviceAddress Probe9 = { 0x28, 0x3D, 0x1A, 0x8F, 0x05, 0x00, 0x00, 0x0E }; //Conservatory on valve 10
DeviceAddress Probe3 = { 0x28, 0xA4, 0x0F, 0x92, 0x05, 0x00, 0x00, 0x22 }; //Guest Bedroom on valve 2
DeviceAddress Probe10 = { 0x28, 0x99, 0xFB, 0x91, 0x05, 0x00, 0x00, 0xB4 }; //Tank on ?? 9


int dataStore[13] = {0, //[0]Dummy
                     0,    //[1]Dummy
                     0,   //[2]Master Bedroom
                     0,    //[3]Guest bedroom
                     0,    //[4]Kitchen
                     0,    //[5]Utility
                     0,    //[6]Office
                     0,    //[7]Nursery
                     0,    //[8]Dining Room
                     0,    //[9]Lounge (posh room)
                     0,    //[10]hallway
                     0,    //[11]Bathroom
                     0};   //[12]Conservatory



// Listen on default port 5555, the webserver on the Yun
// will forward there all the HTTP requests for us.
YunServer server;

void setup() {
  // use a for loop to initialize each pin as an output:
  for (int digitalPins = 2; digitalPins <= 12; digitalPins++)
  {
    pinMode(digitalPins, OUTPUT);
  }
  for (int analogPins = A0; analogPins <= A3; analogPins++)
  {
    pinMode(analogPins, OUTPUT);
  }
  
  digitalWrite(A1, HIGH); // we are using A1 as a supplementary 5v power for the lounge sensor

  // Bridge startup
  Bridge.begin();
 // Serial.begin(9600);    // initialize serial

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();


  // Initialize the Temperature measurement library
  sensorsA.begin();
  sensorsB.begin();
  sensorsC.begin();

  // set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
  sensorsA.setResolution(Probe6, 10);
  sensorsA.setResolution(Probe7, 10);
  sensorsA.setResolution(Probe12, 10);
  sensorsA.setResolution(Probe5, 10);
  sensorsA.setResolution(Probe4, 10);
  sensorsC.setResolution(Probe13, 10);

  sensorsB.setResolution(Probe2, 10);
  sensorsB.setResolution(Probe8, 10);
  sensorsB.setResolution(Probe11, 10);
  sensorsB.setResolution(Probe9, 10);
  sensorsB.setResolution(Probe3, 10);
  sensorsB.setResolution(Probe10, 10);
  
  digitalWrite(A0, HIGH);

}


void loop() {

  updateTimeParameters();

 


  // this is the main script
  sensorsA.requestTemperatures();
sensorsB.requestTemperatures();
sensorsC.requestTemperatures();

 
    //The tempArray gets the current temperature values for each room and puts them into an Array
    float tempArray[13] = {sensorsB.getTempC(Probe2), //Dummy
                           sensorsB.getTempC(Probe3),    //Dummy
                           sensorsB.getTempC(Probe2),  //Master Bedroom on digital 2
                           sensorsB.getTempC(Probe3),  //Guest Bedroom on digital 3
                           sensorsA.getTempC(Probe4),  //Kitchen on digital 4
                           sensorsA.getTempC(Probe5),  //Utility on digital 5
                           sensorsA.getTempC(Probe6),  //Office on digital 6
                           sensorsA.getTempC(Probe7),  //Nursery on digital 7
                           sensorsB.getTempC(Probe8),  //Dining Room on digital 8
                           sensorsC.getTempC(Probe13),  //Lounge on digital 9
                           sensorsB.getTempC(Probe11), //Hallway on digital 10
                           sensorsA.getTempC(Probe12),  //Upstairs Bathroom on digital 11
                           sensorsB.getTempC(Probe9)  //Conservatory on digital12
                          };  
    //Ian - this is where you set the room temps. The desiredTemp sets the desired temperature in each room              
    float desiredTemp[13] = {0, //Dummy
                           0,    //Dummy
                           18,  //Master Bedroom on digital 2
                           20,  //Guest Bedroom on digital 3
                           19,  //Kitchen on digital 4
                           18,  //Utility on digital 5
                           19,  //Office on digital 6
                           17,  //Nursery on digital 7
                           19,  //Dining Room on digital 8
                           19,  //Lounge on digital 9
                           17, //Hallway on digital 10
                           18,  //Upstairs Bathroom on digital 11
                           18  //Conservatory on digital12
                          };   
 
 

 
                       
  float tankTemp = (sensorsB.getTempC(Probe10));


  YunClient client = server.accept();
  if (client) {
     String command = client.readStringUntil('/');
   command.trim();
    
  if (command == "temperature") {
      client.print("Tank Temp: ");
      client.print(tankTemp);
      client.print("</br>Master Bed: ");
      client.print(tempArray[2]);
      client.print("</br>Guest Bed: ");
      client.print(tempArray[3]);
      client.print("</br>Kitchen: ");
      client.print(tempArray[4]);
      client.print("</br>Utility: ");
      client.print(tempArray[5]);
      client.print("</br>Office: ");
      client.print(tempArray[6]);
      client.print("</br>Nursery: ");
      client.print(tempArray[7]);
      client.print("</br>Dining Rm: ");
      client.print(tempArray[8]);
      client.print("</br>Lounge: ");
      client.print(tempArray[9]);
      client.print("</br>Hallway: ");
      client.print(tempArray[10]);
      client.print("</br>Bathroom: ");
      client.print(tempArray[11]);
      client.print("</br>Conservatory: ");
      client.print(tempArray[12]);
    }   
    
  else if (command == "digital") {
   digitalCommand(client);
  }
      
   client.stop();
  }
 
//Ian - when setting the time parameters for each room remember to set them both in the 'if' and also their negative in the second 'else if' statement 
                                                          
    //Master Bedroom                                          
    if ((hours >= 11) && (hours < 24) && (tankTemp >= 25) && (tempArray[2] < (desiredTemp[2] - hysteresis)) && (dataStore[2] == 1)) //if its between 11h and 22h and the tank is above 25 degrees and room temp is below 21 and room turned on

    {
      digitalWrite(2, LOW); //then turn on the master bedroom valve on digital pin 1
      digitalWrite(A0, LOW); //and Start the circulating pump on Analog Pin A0

    }
    else if ((tempArray[2] > (desiredTemp[2] + hysteresis)) || (tankTemp < 25) || (dataStore[2] == 0) || (hours < 11))
    {
      digitalWrite(2, HIGH); //turn the master bedroom off 
    }
    
    //Guest Bedroom
    if ((hours >= 18) && (hours < 23) && (tankTemp >= 25) && (tempArray[3] < (desiredTemp[3] - hysteresis)) && (dataStore[3] == 1 ))
    {
      digitalWrite(3, LOW);
      digitalWrite(A0, LOW);
    }
    else if ((tempArray[3] > (desiredTemp[3] + hysteresis)) || (tankTemp < 25) || (dataStore[3] == 0) || (hours < 18) || (hours >= 23))
    {
      digitalWrite(3, HIGH);
    }
    
    //Kitchen
    if ((isHomeTime) && (tankTemp >= 25) && (tempArray[4] < (desiredTemp[4] - hysteresis)) && (dataStore[4] == 1))
    {
      digitalWrite(4, LOW);
      digitalWrite(A0, LOW);
    }
    else if ((tempArray[4] > (desiredTemp[4] + hysteresis)) || (tankTemp < 25) || (dataStore[4] == 0) || (!isHomeTime))
    {
      digitalWrite(4, HIGH);
    }
   
    
    //Utility room
    if ((isHomeTime) && (tankTemp >= 25) && (tempArray[5] < (desiredTemp[5] - hysteresis)) && (dataStore[5] == 1))
    {
      digitalWrite(5, LOW);
      digitalWrite(A0, LOW);
    }
    else if ((tempArray[5] > (desiredTemp[5] + hysteresis)) || (tankTemp < 25) || (dataStore[5] == 0) || (!isHomeTime))
    {
      digitalWrite(5, HIGH);
    }
    
    
    //Office
    if ((isHomeTime) && (tankTemp >= 25) && (tempArray[6] < (desiredTemp[6] - hysteresis)) && (dataStore[6] == 1))
    {
      digitalWrite(6, LOW);
      digitalWrite(A0, LOW);
    }
    else if ((tempArray[6] > (desiredTemp[6] + hysteresis)) || (tankTemp < 25) || (dataStore[6] == 0) || (!isHomeTime))
    {
      digitalWrite(6, HIGH);
    }
    
    
    //Nursery
    if ((isNurseryTime) && (tankTemp >= 25) && (tempArray[7] < (desiredTemp[7] - hysteresis)) && (dataStore[7] == 1))
    {
      digitalWrite(7, LOW);
      digitalWrite(A0, LOW);
    }
    else if ((tempArray[7] > (desiredTemp[7] + hysteresis)) || (tankTemp < 25) || (dataStore[7] == 0) || (!isNurseryTime))
    {
      digitalWrite(7, HIGH);
    }
   
    //Dining Room
    if ((isHomeTime) && (tankTemp >= 25) && (tempArray[8] < (desiredTemp[8] - hysteresis)) && (dataStore[8] == 1))
    {
      digitalWrite(8, LOW);
      digitalWrite(A0, LOW);
    }
    else if ((tempArray[8] > (desiredTemp[8] + hysteresis)) || (tankTemp < 25) || (dataStore[8] == 0) || (!isHomeTime))
    {
      digitalWrite(8, HIGH);
    }
   
    
    //Lounge (posh room)
    if ((isHomeTime) && (tankTemp >= 25) && (tempArray[9] < (desiredTemp[9] - hysteresis)) && (dataStore[9] == 1))
    {
      digitalWrite(9, LOW);
      digitalWrite(A0, LOW);
    }
    else if ((tempArray[9] > (desiredTemp[9] + hysteresis)) || (tankTemp < 25) || (dataStore[9] == 0) || (!isHomeTime))
    {
      digitalWrite(9, HIGH);
    }
  
    
    //Hallway
    if ((isHomeTime) && (tankTemp >= 25) && (tempArray[10] < (desiredTemp[10] - hysteresis)) && (dataStore[10] == 1))
    {
      digitalWrite(10, LOW);
      digitalWrite(A0, LOW);
    }
    else if ((tempArray[10] > (desiredTemp[10] + hysteresis)) || (tankTemp < 25) || (dataStore[10] == 0) || (!isHomeTime))
    {
      digitalWrite(10, HIGH);
    }
    
    
    //Bathroom Towel radiators
    if ((isHomeTime) && (tankTemp >= 25) && (tempArray[11] < (desiredTemp[11] - hysteresis)) && (dataStore[11] == 1))
    {
      digitalWrite(11, LOW);
      digitalWrite(A0, LOW);
    }
    else if ((tempArray[11] > (desiredTemp[11] + hysteresis)) || (tankTemp < 25) || (dataStore[11] == 0) || (!isHomeTime))
    {
      digitalWrite(11, HIGH);
    }
    
    
    //Conservatory
    if ((isHomeTime) && (tankTemp >= 25) && (tempArray[12] < (desiredTemp[12] - hysteresis)) && (dataStore[12] == 1))
    {
      digitalWrite(12, LOW);
      digitalWrite(A0, LOW);
    }
    else if ((tempArray[12] > (desiredTemp[12] + hysteresis)) || (tankTemp < 25) || (dataStore[12] == 0) || (!isHomeTime))
    {
      digitalWrite(12, HIGH);
    }
    



  if (tankTemp >= 70)
  {
    digitalWrite(11, LOW); //turn on towel rails
    digitalWrite(A0, LOW); //and Start the circulating pump on Analog Pin A0
    delay(120000); //added two minute delay to stop flickering problem
  }
  
//The following checks to see if all valves are off - if true then it shuts off the circulating valve     
if (digitalRead(2) && digitalRead(3) && digitalRead(4) && digitalRead(5) && digitalRead(6) && digitalRead(7) && digitalRead(8) && digitalRead(9) && digitalRead(10) && digitalRead(11) && digitalRead(12) == HIGH) 
{
digitalWrite(A0, HIGH);
}


  // end of main script
}


void digitalCommand(YunClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/digital/13/1"
  if (client.read() == '/') {
    value = client.parseInt();

  }
  else {
    value = digitalRead(pin);
  }

  // Send feedback to client
  client.print(F("Pin D"));
  client.print(pin);
  client.print(F(" set to "));
  client.println(value);

  // Update datastore key with the current pin value
  String key = "D";
  key += pin;
  Bridge.put(key, String(value));
  dataStore[pin] = (value);
}



void updateTimeParameters()
{

  // start the date process:
  if (!date.running())  {
    date.begin("date");
    date.addParameter("+%T");
    date.run();


    while (date.available() > 0) {
      // get the result of the date process (should be hh:mm:ss):
      String timeString = date.readString();

      // find the colons:
      int firstColon = timeString.indexOf(":");
      int secondColon = timeString.lastIndexOf(":");

      // get the substrings for hour, minute second:
      String hourString = timeString.substring(0, firstColon);
      String minString = timeString.substring(firstColon + 1, secondColon);
      String secString = timeString.substring(secondColon + 1);

      // convert to ints,saving the previous second:
      hours = hourString.toInt();
      minutes = minString.toInt();
      lastSecond = seconds;          // save to do a time comparison
      seconds = secString.toInt();


      //change below for timer hours settings
      if ((hours >= 5) && (hours < 22))
      {
        isHomeTime = true;
      }
      else
      {
        isHomeTime = false;
      }
      
       if (((hours >= 18) && (hours < 23)) || ((hours >= 2) && (hours < 9)))
      {
        isNurseryTime = true;
      }
      else
      {
        isNurseryTime = false;
      }



   }
   }
 }




