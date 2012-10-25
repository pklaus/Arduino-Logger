// Interrupt Timers inspired by
// http://www.instructables.com/id/Arduino-Timer-Interrupts/ and by
// http://stackoverflow.com/questions/10393955/multiple-analogread-calls-at-timed-intervals 

//configuration values
#define EMA_SAMPLES 50
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  #define NUM_ADC_INPUTS 16
  #define ADC_PINS A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15
#else
  #define NUM_ADC_INPUTS 6;
  #define ADC_PINS A0, A1, A2, A3, A4, A5
#endif

// do not need to configure below this line
const uint8_t N = NUM_ADC_INPUTS;
float adc_values_ema[N];
const uint8_t adc_read_pins[N] = {ADC_PINS};
const uint8_t moving_average_samples = EMA_SAMPLES;
float weight;

void set_timer_interrupt(){
  cli();//stop interrupts

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 100 Hz
  OCR1A = 155;// = (16*10^6) / (100 * 1 * 1024) - 1 = 155   (must be < 65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
}

void setup(){
  set_timer_interrupt();
  
  weight = 2./(1.+float(moving_average_samples));
  
  for(int i = 0; i < N; i++) {
    adc_values_ema[i] = (float) analogRead(adc_read_pins[i]);
    delay(2);
  }
  
  Serial.begin(115200);
}

uint8_t counter = 0;
ISR(TIMER1_COMPA_vect){
  // Might replace analogRead by the actual stuff from:
  // https://github.com/arduino/Arduino/blob/master/hardware/arduino/cores/arduino/wiring_analog.c#L40
  uint8_t the_one = counter%N;
  int adc_input_buffer = analogRead(adc_read_pins[the_one]);
  // Calculate the exponential moving average:
  adc_values_ema[the_one]  = (adc_input_buffer - adc_values_ema[the_one]) * weight + adc_values_ema[the_one];
  counter++;
  if (counter == N) counter = 0; // reset the counter (not really needed IF N is a power of 2.
  // prepare next read operation:
  analogRead(adc_read_pins[counter%N]);
}

void loop(){
  for(int i = 0; i < N; i++) {
    Serial.print(adc_values_ema[i]);
    Serial.print(" ");
  }
  Serial.println("");
  delay(150);
}

