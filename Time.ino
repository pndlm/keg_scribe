
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
  
  Serial.println(F("UpdateNTPTime"));
  mysntp.UpdateNTPTime();
  
  Serial.println(F("Current local time is:"));
  mysntp.ExtractNTPTime(mysntp.NTPGetTime(&sntpTime, true), &timeExtract);
  
  char time[50];
  Serial.println(sprintTime(time, sntpTime.seconds - NTP_TO_UNIX));

  // seconds since 1900 - (seconds between 1900 and 1970)
  return sntpTime.seconds - NTP_TO_UNIX;
}

int sprintTimeStamp(char* s, time_t t) {
  return sprintf(s, "%lu", t);
}

int sprintTime(char* s, time_t t) {
  return sprintf(s, "%02i-%02i-%02i %02i:%02i:%02i", year(t), month(t), day(t), hour(t), minute(t), second(t));
}
