/*
 * Time_NTP.pde
 * Example showing time sync to NTP time source
 * 
 * This file is a combination of the NTP Time Example found in the
 * Arduino Time library http://arduino.cc/playground/Code/Time 
 * and the better NTP code in the Arduino Ethernet example
 * UdpNtpClient http://arduino.cc/en/Tutorial/UdpNtpClient 
 *
 */
 
#include <Time.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE0 };

unsigned int localPort = 8888;      // local port to listen for UDP packets

//IPAddress timeServer(193, 149, 10, 65); // time.nist.gov NTP server
IPAddress timeServer(192, 168, 2, 1); // local NTP server

const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

time_t prevDisplay = 0; // when the digital clock was displayed
const  long timeZoneOffset = -7200L; // set this to the offset in seconds to your local time;

char isodate[25]; // The current time in ISO format is being stored here

void setup() 
{
  Serial.begin(9600);
  
  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  Udp.begin(localPort);
  
  Serial.println("waiting for sync");
  
  setSyncProvider(getNtpTime);
  while(timeStatus()== timeNotSet)   
     ; // wait until the time is set by the sync provider
}

void loop()
{  
  if( now() != prevDisplay) //update the display only if the time has changed
  {
    prevDisplay = now();
    digitalClockDisplay();
    makeISOdate();
    Serial.println(isodate);
  }
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
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
  sprintf(isodate, "%4d-%02d-%02dT%02d:%02d:%02d%+05d\n",
      year(), month(), day(), hour(), minute(), second(), timeZoneOffset/36 );
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
    return epoch - timeZoneOffset;
  }
  return 0; // return 0 if unable to get the time
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
