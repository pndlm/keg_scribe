
// seconds between 1900-01-01 and 1970-01-01 
// also known as 2208988800UL
#define NTP_TO_UNIX ((70 * 365 + 17) * 86400)

//Arguments to SNTP client constructor:
//	1 - Primary Network Time Server URL (can be NULL)
//	2 - Secondary Network Time Server URL (also can be NULL)
//	3 - Local UTC offset in minutes (US Eastern Time is UTC - 5:00
//	4 - Local UTC offset in minutes for Daylight Savings Time (US Eastern DST is UTC - 4:00
//	5 - Enable Daylight Savings Time adjustment (not implemented yet)
//
sntp mysntp = sntp(NULL, "time.nist.gov", (short)(0 * 60), (short)(0 * 60), false);

// Type SNTP_Timestamp is 64-bit NTP time. High-order 32-bits is seconds since 1/1/1900
//   Low order 32-bits is fractional seconds
SNTP_Timestamp_t sntpTime;

// Type NetTime_t contains NTP time broken out to human-oriented values:
//	uint16_t millis; ///< Milliseconds after the second (0..999)
//	uint8_t	 sec;    ///< Seconds after the minute (0..59)
//	uint8_t	 min;    ///< Minutes after the hour (0..59)
//	uint8_t	 hour;   ///< Hours since midnight (0..23)
//	uint8_t	 mday;   ///< Day of the month (1..31)
//	uint8_t	 mon;    ///< Months since January (0..11)
//	uint16_t year;   ///< Year.
//	uint8_t	 wday;	 ///< Days since Sunday (0..6)
//	uint8_t	 yday;   ///< Days since January 1 (0..365)
//	bool	 isdst;  ///< Daylight savings time flag, currently not supported	
NetTime_t timeExtract;


time_t getNtpTime() {
  
  mysntp.UpdateNTPTime();
  
  mysntp.NTPGetTime(&sntpTime, false);

//  Serial.println(F("time "));
//  Serial.print(timeExtract.year); Serial.print(F("-")); Serial.print(timeExtract.mon); Serial.print(F("-")); Serial.print(timeExtract.mday); Serial.print(F(" "));
//  Serial.print(timeExtract.hour); Serial.print(F(":")); Serial.print(timeExtract.min); Serial.print(F(":")); Serial.print(timeExtract.sec); Serial.print(F(".")); Serial.print(timeExtract.millis);
  
  // seconds since 1900 - (seconds between 1900 and 1970)
  return sntpTime.seconds - NTP_TO_UNIX;
}

/*-------- NTP code ----------*/

/*
//33674

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; 
// NTP Servers:
IPAddress timeServer(132, 163, 4, 101);

EthernetUDP Udp;
unsigned int localPort = 8888;

const int timeZone = 1;

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
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
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

*/

// ----- My code

int sprintTimeStamp(char* buffer, time_t t) {
  return cbPrintInt(buffer, t);
}

// provide a string buffer of at least 20 characters
// the buffer will be filled like: 2012-03-29T17:00:00
int sprintTime(char* buffer, time_t* t, bool withMinutes) {
  short offset = 0;
  offset += cbPrintInt(&buffer[offset], year(*t));
  offset += cbPrintInt(&buffer[offset], month(*t));
  offset += cbPrintInt(&buffer[offset], day(*t));
  if (withMinutes) { 
    buffer[offset++] = 'T';
    offset += cbPrintInt(&buffer[offset], hour(*t));
    offset += cbPrintInt(&buffer[offset], minute(*t));
    offset += cbPrintInt(&buffer[offset], second(*t));
  }
  return offset;
}
