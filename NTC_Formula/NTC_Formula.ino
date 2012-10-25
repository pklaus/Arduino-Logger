#include <math.h>
// enumarating 3 major temperature scales
enum {
  T_KELVIN=0,
  T_CELSIUS,
  T_FAHRENHEIT
};

#define INPUT_PIN 8 // NTC connected to analog input #8
#define R_BALANCE 470.0f // The balance resistance

// manufacturer data for the thermistor
#define EPCOS_G1550_10k 3625.0f,298.15f,10000.0f  // B,T0,R0

// Temperature function outputs float , the actual 
// temperature
// Temperature function inputs
// 1.AnalogInputNumber - analog input to read from 
// 2.OuputUnit - output in celsius, kelvin or fahrenheit
// 3.Thermistor B parameter - found in datasheet 
// 4.Manufacturer T0 parameter - found in datasheet (kelvin)
// 5. Manufacturer R0 parameter - found in datasheet (ohms)
// 6. Your balance resistor resistance in ohms  

float Temperature(int AnalogInputNumber,int OutputUnit,float B,float T0,float R0,float R_Balance) {
  float R,T;

  R = (1023.0f*R_Balance/float(analogRead(AnalogInputNumber))) - R_Balance;
  T = 1.0f/(1.0f/T0+(1.0f/B)*log(R/R0));

  switch(OutputUnit) {
    case T_CELSIUS :
      T-=273.15f;
      break;
    case T_FAHRENHEIT :
      T=9.0f*(T-273.15f)/5.0f+32.0f;
      break;
    default:
      break;
  };
  return T;
}

void setup() {
 Serial.begin(115200);
}

void loop() {

 Serial.println(Temperature(INPUT_PIN,T_CELSIUS,EPCOS_G1550_10k,R_BALANCE));

 delay(250);
}
