/*
  NTC - Temperature - Serial Output
  Based on the example script AnalogReadSerial
  
  For help with the
  Analog input pins: http://arduino.cc/en/Tutorial/AnalogInputPins 
  Serial connection: http://arduino.cc/en/Reference/Serial 
  Serial Realtime Plot: http://www.blendedtechnologies.com/realtime-plot-of-arduino-serial-data-using-python 
*/

#define BAUD 115200 // Baud Rate for serial port
#define INTERVAL 1000 // ms between writes to serial
#define LED_PIN 13 // Pin that should blink

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(BAUD);
  // Setup the analog reference:
  analogReference(DEFAULT);
  //  DEFAULT: the default analog reference of 5 volts (on 5V Arduino boards) or 3.3 volts (on 3.3V Arduino boards)
  //  INTERNAL: an built-in reference, equal to 1.1 volts on the ATmega168 or ATmega328 and 2.56 volts on the ATmega8 (not available on the Arduino Mega)
  //  INTERNAL1V1: a built-in 1.1V reference (Arduino Mega only)
  //  INTERNAL2V56: a built-in 2.56V reference (Arduino Mega only)
  //  EXTERNAL: the voltage applied to the AREF pin (0 to 5V only) is used as the reference.
  // Blinking LED on PIN 13:
  pinMode(LED_PIN, OUTPUT);
}


int i;
const int numPins = 16;
//int input[6] = {A0, A1, A2, A3, A4, A5};  // the analog input pins to read
int input[numPins] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15};  // for the MEGA
int output[numPins]; // where the output gets written to
char* sep = "\t";  // separating the values in the output lines

// the loop routine runs over and over again forever:
void loop() {
  
  // read the inputs
  for (i = 0; i < numPins; i = i + 1) {
    Serial.print(analogRead(input[i]));
    Serial.print(sep);
  }
  Serial.println("");
  digitalWrite(13, HIGH);
  delay(INTERVAL/5);        // delay (in ms) in between reads
  digitalWrite(13, LOW);
  delay(INTERVAL/5*4);        // delay (in ms) in between reads
}
