//inspired by http://www.instructables.com/id/Arduino-Timer-Interrupts/


//configuration values
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  #define NUM_ADC_INPUTS 16
  #define ADC_PINS A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15
#else
  #define NUM_ADC_INPUTS 6;
  #define ADC_PINS A0, A1, A2, A3, A4, A5
#endif

// do not need to configure below this line
const uint8_t N = NUM_ADC_INPUTS;
const uint8_t adc_read_pins[N] = {ADC_PINS};
int adc_values[N];

void setup(){
  cli();//stop interrupts

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15;//OCR1A = 155;// = (16*10^6) / (100 * 1 * 1024) - 1 = 155   (must be < 65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
  
  for(int i = 0; i < N; i++) {
    adc_values[i] = 0;
  }
  
  Serial.begin(115200);
}

uint8_t counter = 0;

ISR(TIMER1_COMPA_vect){
  // Might replace analogRead by the actual stuff from:
  // https://github.com/arduino/Arduino/blob/master/hardware/arduino/cores/arduino/wiring_analog.c#L40
  adc_values[counter%N] = analogRead(adc_read_pins[counter%N]);
  counter++;
}

void loop(){
  for(int i = 0; i < N; i++) {
    Serial.print(adc_values[i]);
    Serial.print(" ");
  }
  Serial.println("");
  delay(100);
}

