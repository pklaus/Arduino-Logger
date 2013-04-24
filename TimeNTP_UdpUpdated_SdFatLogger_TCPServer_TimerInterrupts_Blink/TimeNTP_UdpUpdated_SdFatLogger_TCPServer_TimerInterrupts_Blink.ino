/*
 * Time_NTP.pde
 * Example showing time sync to NTP time source
 * 
 * This file is a combination of the NTP Time Example found in the
 * Arduino Time library http://arduino.cc/playground/Code/Time 
 * and the better NTP code in the Arduino Ethernet example
 * UdpNtpClient http://arduino.cc/en/Tutorial/UdpNtpClient 
 * Now including a simple data logger based on AnalogLogger
 * found in SdFat http://code.google.com/p/sdfatlib/ 
 *
 
 TODO: All the NTC integration.
 
 */

// ##################   The dependencies first   ################
#include <Time.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SdFat.h>
#include <SdFatUtil.h>  // define FreeRam()

// ##################   Now the configuration section  ################

#define BAUD 115200 // Baud rate of serial port

#define LOG_FREQ   2  // How often the log data should be written to output in Hz
#define ADC_FREQ       10  // How many times all the ADC values should be read in Hz
#define EMA_SAMPLES 20     // Number of samples for exponential moving average
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  //configuration values  for EMA Timer Interrupt
  #define NUM_ADC_INPUTS 14
  #define ADC_PINS A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15
#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  #define NUM_ADC_INPUTS 4
  #define ADC_PINS A2, A3, A4, A5
#endif

#define ECHO_TO_SERIAL   1  // echo data to serial port if nonzero
#define WAIT_TO_START    0  // Wait for serial input in setup()

byte mac[] = { 0xDE, 0xAD, 0xBC, 0xEF, 0xFE, 0xE0 }; // The MAC address of the Ethernet controller

//IPAddress timeServer(192, 53, 103, 108); // ptbtime1.ptb.de NTP server
IPAddress timeServer(192, 168, 10, 1); // local NTP server
#define TZ_OFFSET -7200L  // time zone offset - set this to the offset in seconds to your local time;
#define TIME_SYNC_INTERVAL 43200UL // = 60*60*12 seconds = 1 day  until time is synced again

// Port for net console
#define TELNET_PORT 23 // "telnet" defaults to port 23
// You can connect via Netcat (`nc 193.149.10.108 23`) or via Telnet (`telnet 193.149.10.108`).

#define USE_STATUS_LED   1 // Shall we flash an LED when we write to the SD card?
#define STATUS_LED       9 // Connect an LED that flashes whenever you write to the SD card

// ##################   should not need to change configuration below this line ################

#define TIMER_FREQ ADC_FREQ * NUM_ADC_INPUTS

const uint8_t log_interval = 1000 / LOG_FREQ;

#define CHIP_SELECT     4 // was: SS  // SD chip select pin

#define LOCAL_UDP_PORT 8888   // local port to listen for UDP packets

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

EthernetServer server = EthernetServer(TELNET_PORT);

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

time_t cur_time = 0;
char isodate[25]; // The current time in ISO format is being stored here

// file system object
SdFat sd;

// text file for logging
ofstream logfile;

// Serial print stream
ArduinoOutStream cout(Serial);

// buffer to format data - makes it eaiser to echo to Serial
char buf[160];
//------------------------------------------------------------------------------
// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
// call back for file timestamps
void dateTime(uint16_t* date, uint16_t* time) {
  // Ask only once for date and time:
  time_t t = cur_time; // store the current time in time variable t
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(year(t), month(t), day(t));
  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour(t), minute(t), second(t));
}
//------------------------------------------------------------------------------

// Variables for Timer Interrupt
const uint8_t N_ADC = NUM_ADC_INPUTS;
float adc_values_ema[N_ADC];
const uint8_t adc_read_pins[N_ADC] = {ADC_PINS};
const uint8_t moving_average_samples = EMA_SAMPLES;
float weight;

// ------------ Clock stuff

void digitalClockDisplay(){
  // Ask only once for date and time:
  time_t t = cur_time;
  // digital clock display of the time
  Serial.print(hour(t));
  printDigits(minute(t));
  printDigits(second(t));
  Serial.print(" ");
  Serial.print(day(t));
  Serial.print(" ");
  Serial.print(month(t));
  Serial.print(" ");
  Serial.print(year(t)); 
  Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void makeISOdate(){
  // Uses the global time variable cur_time (user's responsibility to call before this func).
  time_t t = cur_time;
  sprintf(isodate, "%4d-%02d-%02dT%02d:%02d:%02d%+05d",
      year(t), month(t), day(t), hour(t), minute(t), second(t), TZ_OFFSET/36 );
}

/*-------- NTP code ----------*/

unsigned long getNtpTime()
{
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(400);
  if ( Udp.parsePacket() == NTP_PACKET_SIZE) {
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    
    if ( secsSince1900 < 3559411850UL || secsSince1900 > 4294967295UL )
      // If the date is not in the range between 2012 and 2036, there's something wrong
      return 0;
    
    // now convert NTP time into Unix time
    // starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;  
    return epoch - TZ_OFFSET;
  }
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 		   
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket(); 
}

void set_timer_interrupt(){
  cli();//stop interrupts

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 100 Hz
  //OCR1A = 155;// = (16*10^6) / (100 * 1 * 1024) - 1 = 155   (must be < 65536)
  OCR1A = (16000000UL) / (TIMER_FREQ * 1 * 1024UL) - 1;
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
}

// The routine that's being called by the timer interrupt.
// It fetches the current ADC reading,
// calculates the moving average and then
// gets the reading of the next ADC channel (to set up the MUX for this one for the next call).
uint8_t counter = 0;
ISR(TIMER1_COMPA_vect){
  // Might replace analogRead by the actual stuff from:
  // https://github.com/arduino/Arduino/blob/master/hardware/arduino/cores/arduino/wiring_analog.c#L40
  uint8_t the_one = counter%N_ADC;
  // Calculate the exponential moving average:
  adc_values_ema[the_one]  = (analogRead(adc_read_pins[the_one]) - adc_values_ema[the_one]) * weight + adc_values_ema[the_one];
  counter++;
  if (counter == N_ADC) counter = 0; // reset the counter (not really needed IF N_ADC is a power of 2.
  // prepare next read operation:
  analogRead(adc_read_pins[counter%N_ADC]);
}

void initialize_adc_interrupts_ema() {
  // Setting the timer interrupts for exp moving averaging of ADC values
  set_timer_interrupt();
  weight = 2./(1.+float(moving_average_samples));
  for(int i = 0; i < N_ADC; i++) {
    // Initialize ema
    adc_values_ema[i] = (float) analogRead(adc_read_pins[i]);
    delay(2);
  }
}

void start_ntp_time() {
  Serial.println("Starting to sync the time via NTP.");
  Udp.begin(LOCAL_UDP_PORT);
  
  setSyncProvider(getNtpTime);
  while(timeStatus() == timeNotSet)   
     ; // wait until the time is set by the sync provider
  setSyncInterval(TIME_SYNC_INTERVAL);
  
  cur_time = now();
  makeISOdate();
  Serial.print("OK. It's now ");
  Serial.println(isodate);
  
  // set date time callback function
  SdFile::dateTimeCallback(dateTime);
}

void start_log_file() {
  // create a new file in root, the current working directory
  char name[] = "LOG0000.CSV";

  for (uint8_t i = 0; i < 1000; i++) {
    name[4] = i/100 + '0';
    name[5] = i/10 + '0';
    name[6] = i%10 + '0';
    if (sd.exists(name)) continue;
    logfile.open(name);
    break;
  }
  if (!logfile.is_open()) error("file.open");

  cout << pstr("Logging to: ") << name << endl;

  // format header in buffer
  obufstream bout(buf, sizeof(buf));

  bout << pstr("millis");

  bout << pstr(",iso8601datetime");

  for (int i = 0; i < N_ADC; i++) {
    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
      bout << pstr(",sens") << adc_read_pins[i]-52;
    #else
      bout << pstr(",sens") << adc_read_pins[i];
    #endif
  }
  logfile << buf << endl;

  #if ECHO_TO_SERIAL
    cout << buf << endl;
  #endif  // ECHO_TO_SERIAL
}

void initialize_sdcard() {
  Serial.println("Trying to initialize the SD card.");
  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  if (!sd.begin(CHIP_SELECT, SPI_FULL_SPEED)) sd.initErrorHalt();
}

void initialize_networking() {
  Serial.println("Getting an IP via DHCP.");
  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP.");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
}

// --------------------------  Setup  --------------------------

void setup() {
  
  // For SD-Card / Ethernet Shield:
  pinMode(SS, OUTPUT);     // set the SS pin as an output (necessary!)
  
  Serial.begin(BAUD);
  
  #if WAIT_TO_START
    cout << pstr("Type any character to start\n");
    while (Serial.read() < 0) {}
  #endif  // WAIT_TO_START
  
  cout << endl << pstr("FreeRam: ") << FreeRam() << endl; // pstr stores strings in flash to save RAM
  
  initialize_sdcard();
  
  initialize_networking();
  
  start_ntp_time();

  start_log_file();
  
  initialize_adc_interrupts_ema();
  
  server.begin(); // start listening for clients on net console
  
  #if USE_STATUS_LED
    // initialize status LED pin as an output.
    pinMode(STATUS_LED, OUTPUT);
  #endif
}

// --------------------------  Loop  --------------------------

void loop()
{
  uint32_t m;
  time_t prev_time = 0; // when the digital clock was displayed

  // wait for time to be a multiple of interval
  do {
    m = millis();
  } while (m % log_interval);

  // Get date&time and format it
  cur_time = now();
  if( cur_time != prev_time) {
    // update the date only if has changed
    prev_time = cur_time;
    //digitalClockDisplay();
    makeISOdate(); // acts on cur_time and writes to isodate
    //Serial.println(isodate);
  }
  
  // use buffer stream to format line
  obufstream bout(buf, sizeof(buf));
  
  bout << m; // start with time in millis
  bout << ',' << isodate; // add the date and time

  for(int i = 0; i < N_ADC; i++) {
    // write averaged ADC values
    bout << ',' << adc_values_ema[i];
  }
  bout << endl;

  #if USE_STATUS_LED
    digitalWrite(STATUS_LED, HIGH);   // turn the LED on
  #endif
  
  #if ECHO_TO_SERIAL
    cout << buf;
  #endif

  // Write to net console
  server.write(buf);

  // log data and flush to SD
  logfile << buf << flush;
  
  #if USE_STATUS_LED
    digitalWrite(STATUS_LED, LOW);    // turn the LED off
  #endif

  // check for error
  if (!logfile) error("write data failed");

  // don't log two points in the same millis
  if (m == millis()) delay(1);
}
