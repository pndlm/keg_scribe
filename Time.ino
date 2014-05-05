
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
//sntp mysntp = sntp(NULL, "time.nist.gov", (short)(0 * 60), (short)(0 * 60), false);

// Type SNTP_Timestamp is 64-bit NTP time. High-order 32-bits is seconds since 1/1/1900
//   Low order 32-bits is fractional seconds
//SNTP_Timestamp_t sntpTime;

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
//NetTime_t timeExtract;


const unsigned long
  connectTimeout  = 15L * 1000L, // Max time to wait for server connection
  responseTimeout = 15L * 1000L; // Max time to wait for data from server
  
unsigned long
  lastPolledTime  = 0L, // Last value retrieved from time server
  sketchTime      = 0L; // CPU milliseconds since last server query

void initTime() {
  setSyncProvider(getNtpTime);
  setSyncInterval(NTP_INTERVAL);
}

// Minimalist time server query; adapted from Adafruit Gutenbird sketch,
// which in turn has roots in Arduino UdpNTPClient tutorial.
time_t getNtpTime(void) {
  
  return 1;
  
  Adafruit_CC3000_Client client;
  Adafruit_CC3000* cc3000 = getCC3000();

  uint8_t       buf[48];
  unsigned long ip, startTime, t = 0L;

  Serial.print(F("time"));

  // Hostname to IP lookup; use NTP pool (rotates through servers)
  if(cc3000->getHostByName(NTP_SERVER, &ip)) {
    static const char PROGMEM
      timeReqA[] = { 227,  0,  6, 236 },
      timeReqB[] = {  49, 78, 49,  52 };

    startTime = millis();
    client = cc3000->connectUDP(ip, 123);
    
    while((!client.connected()) &&
            ((millis() - startTime) < connectTimeout));

    if(client.connected()) {
      // Assemble and issue request packet
      memset(buf, 0, sizeof(buf));
      memcpy_P( buf    , timeReqA, sizeof(timeReqA));
      memcpy_P(&buf[12], timeReqB, sizeof(timeReqB));
      client.write(buf, sizeof(buf));

      memset(buf, 0, sizeof(buf));
      startTime = millis();
      while((!client.available()) &&
            ((millis() - startTime) < responseTimeout));
      if(client.available()) {
        client.read(buf, sizeof(buf));
        t = (((unsigned long)buf[40] << 24) |
             ((unsigned long)buf[41] << 16) |
             ((unsigned long)buf[42] <<  8) |
              (unsigned long)buf[43]) - NTP_TO_UNIX;
        Serial.print(t);
        Serial.print(OK_MSG);
      }
      client.close();
    }
  }
  if(!t) Serial.println(FAIL_MSG);
  return t;
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
