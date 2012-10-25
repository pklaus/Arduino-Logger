/*
  NTC - Temperature - Serial Output - Exponential Moving Average
  Based on the example script AnalogReadSerial
  
  For help with the
  Analog input pins: http://arduino.cc/en/Tutorial/AnalogInputPins 
  Serial connection: http://arduino.cc/en/Reference/Serial 
  Serial Realtime Plot: http://www.blendedtechnologies.com/realtime-plot-of-arduino-serial-data-using-python 
  Easy example for exponential moving average: http://forums.adafruit.com/viewtopic.php?p=57826#p57826 
*/

#include <math.h>

// enumarating 3 major temperature scales
enum {
  T_KELVIN=0,
  T_CELSIUS,
  T_FAHRENHEIT
};

#define BAUD 115200 // baud rate of serial port
#define INPUT_PIN 8 // NTC connected to analog input #8
#define R_BALANCE 470.0f // The balance resistance
#define NUM_ANALOG_PINS 1 // Set the number of analog input pins
// manufacturer data for the thermistor
#define EPCOS_G1550_10k 3625.0f,298.15f,10000.0f  // B,T0,R0

int i, j;
//int input_pins[NUM_ANALOG_PINS] = {A0, A1, A2, A3, A4, A5};  // the analog input pins to read
int input_pins[NUM_ANALOG_PINS] = {A8};  // the analog input pins to read
float input[NUM_ANALOG_PINS] = {0.}; // where the input is being written to
float ema[NUM_ANALOG_PINS] = {0.}; // where the exp moving average is being stored
char* sep = "\t";  // separating the values in the output lines
float output_frequency = 2.; // Output frequency in Hertz
int output_every_x_values = 100; // Size of the moving average samples
int moving_average_samples = 50;
float weight;
int delay_time;

float Temperature(int AnalogInputNumber,int OutputUnit,float B,float T0,float R0,float R_Balance) {
  // Temperature function outputs float, the actual temperature
  // Temperature function inputs
  // 1.AnalogInputNumber - analog input to read from
  // 2.OuputUnit - output in celsius, kelvin or fahrenheit
  // 3.Thermistor B parameter - found in datasheet
  // 4.Manufacturer T0 parameter - found in datasheet (kelvin)
  // 5. Manufacturer R0 parameter - found in datasheet (ohms)
  // 6. Your balance resistor resistance in ohms
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


// the setup routine runs once when you press reset:
void setup() {
  // Setting the loop to zero:
  i = 0;
  delay_time = 1000/(output_frequency*output_every_x_values) - 1; // we substract a couple of ms (as the execution takes time too).
  weight = 2./(1.+float(moving_average_samples)); // we multiply the output_every_x_values by two to have more overlap with the previous output
  // initialize serial communication at BAUD bits per second:
  Serial.begin(BAUD);
  // Setup the analog reference:
  analogReference(DEFAULT);
  //  DEFAULT: the default analog reference of 5 volts (on 5V Arduino boards) or 3.3 volts (on 3.3V Arduino boards)
  //  INTERNAL: an built-in reference, equal to 1.1 volts on the ATmega168 or ATmega328 and 2.56 volts on the ATmega8 (not available on the Arduino Mega)
  //  INTERNAL1V1: a built-in 1.1V reference (Arduino Mega only)
  //  INTERNAL2V56: a built-in 2.56V reference (Arduino Mega only)
  //  EXTERNAL: the voltage applied to the AREF pin (0 to 5V only) is used as the reference. 
}

// the loop routine runs over and over again forever:
void loop() {
  i += 1;
  // read the inputs
  for (j = 0; j < NUM_ANALOG_PINS; j += 1) {
    input[j] = Temperature(input_pins[j],T_CELSIUS,EPCOS_G1550_10k,R_BALANCE);
    // Calculate the exponential moving average:
    ema[j] = (input[j] - ema[j]) * weight + ema[j];
  }
  if (i % output_every_x_values == 0)
  {
    // print the input
    for (j = 0; j < NUM_ANALOG_PINS; j += 1) {
      Serial.print(input[j]);
      Serial.print(sep);
    }
    // print the ema
    for (j = 0; j < NUM_ANALOG_PINS; j += 1) {
      Serial.print(ema[j]);
      Serial.print(sep);
    }
    Serial.println("");
  }
  
  delay(delay_time);        // delay (in ms) in between reads
}
