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
 */

#include <Time.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SdFat.h>
#include <SdFatUtil.h>  // define FreeRam()

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  
  0xDE, 0xAD, 0xBC, 0xEF, 0xFE, 0xE0 };
  
IPAddress timeServer(193, 149, 10, 65); // time.nist.gov NTP server

#define CHIP_SELECT     4 // was: SS  // SD chip select pin
#define LOG_INTERVAL   2000  // mills between entries
#define SENSOR_COUNT     6  // number of analog pins to log
#define ECHO_TO_SERIAL   1  // echo data to serial port if nonzero
#define WAIT_TO_START    0  // Wait for serial input in setup()
#define ADC_DELAY       10  // ADC delay for high impedence sensors

#define TZ_OFFSET -7200L  // time zone offset - set this to the offset in seconds to your local time;
#define TIME_SYNC_INTERVAL 86400 // = 60*60*24 seconds = 1 day  until time is synced again

#define LOCAL_PORT 8888   // local port to listen for UDP packets

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

time_t prevDisplay = 0; // when the digital clock was displayed

char isodate[25]; // The current time in ISO format is being stored here

// file system object
SdFat sd;

// text file for logging
ofstream logfile;

// Serial print stream
ArduinoOutStream cout(Serial);

// buffer to format data - makes it eaiser to echo to Serial
char buf[80];
//------------------------------------------------------------------------------
#if SENSOR_COUNT > 6
#error SENSOR_COUNT too large
#endif  // SENSOR_COUNT
//------------------------------------------------------------------------------
// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
// call back for file timestamps
void dateTime(uint16_t* date, uint16_t* time) {
  // Ask only once for date and time:
  time_t t = now(); // store the current time in time variable t
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(year(t), month(t), day(t));
  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour(t), minute(t), second(t));
}
//------------------------------------------------------------------------------
void setup() 
{
  
  pinMode(SS, OUTPUT);     // set the SS pin as an output (necessary!)
  //digitalWrite(10, HIGH);
  
  Serial.begin(9600);
  Serial.println("Hello there.");

  Serial.println("Trying to initialize the SD card.");
  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  if (!sd.begin(CHIP_SELECT, SPI_FULL_SPEED)) sd.initErrorHalt();
  
  // pstr stores strings in flash to save RAM
  cout << endl << pstr("FreeRam: ") << FreeRam() << endl;
  
#if WAIT_TO_START
  cout << pstr("Type any character to start\n");
  while (Serial.read() < 0) {}
#endif  // WAIT_TO_START
  
  Serial.println("Getting an IP via DHCP.");
  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP.");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  Udp.begin(LOCAL_PORT);
  
  Serial.println("Starting to sync the time via NTP.");
  
  setSyncProvider(getNtpTime);
  while(timeStatus() == timeNotSet)   
     ; // wait until the time is set by the sync provider
  setSyncInterval(TIME_SYNC_INTERVAL);
  
    makeISOdate();
  Serial.print("OK. It's now ");
  Serial.println(isodate);
  
  // set date time callback function
  SdFile::dateTimeCallback(dateTime);

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

  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
    bout << pstr(",sens") << int(i);
  }
  logfile << buf << endl;

#if ECHO_TO_SERIAL
  cout << buf << endl;
#endif  // ECHO_TO_SERIAL
}

void loop()
{
  uint32_t m;
  
  if( now() != prevDisplay) //update the display only if the time has changed
  {
    prevDisplay = now();
    //digitalClockDisplay();
    makeISOdate();
    //Serial.println(isodate);
  }

  // wait for time to be a multiple of interval
  do {
    m = millis();
  } while (m % LOG_INTERVAL);

  // use buffer stream to format line
  obufstream bout(buf, sizeof(buf));

  // start with time in millis
  bout << m;

  bout << ',' << isodate;

  // read analog pins and format data
  for (uint8_t ia = 0; ia < SENSOR_COUNT; ia++) {
#if ADC_DELAY
    analogRead(ia);
    delay(ADC_DELAY);
#endif  // ADC_DELAY
    bout << ',' << analogRead(ia);
  }
  bout << endl;

#if ECHO_TO_SERIAL
  cout << buf;
#endif  // ECHO_TO_SERIAL

  // log data and flush to SD
  logfile << buf << flush;

  // check for error
  if (!logfile) error("write data failed");

  // don't log two points in the same millis
  if (m == millis()) delay(1);
}

void digitalClockDisplay(){
  // Ask only once for date and time:
  time_t t = now();
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
  // Ask only once for date and time:
  time_t t = now();
  sprintf(isodate, "%4d-%02d-%02dT%02d:%02d:%02d%+05d",
      year(t), month(t), day(t), hour(t), minute(t), second(t), TZ_OFFSET/36 );
}

/*-------- NTP code ----------*/

unsigned long getNtpTime()
{
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  if ( Udp.parsePacket() ) {
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    
    // now convert NTP time into Unix time
    // starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;  
    return epoch - TZ_OFFSET;
  }
  return 0; // return 0 if u`````  qqnable to get the time
}

// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
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
