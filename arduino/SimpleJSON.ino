/*
Functions to report data as JSON format to
to a web server.

This code is a bit outdated, but I am leaving
it as an example if someone wants to use it.
The downside is that it does not store data
on an SD card so network failures result
in data loss.

It is also more complicated to append to a JSON
file on an SD card, so I opted to use CSV instead.
*/

/*

char* createJSONString(char importCode[], char utcTimeStamp[], float value) {
  char* result = new char[strlen(importCode) + strlen(utcTimeStamp) + 100];
  
  int valueA = floor(value);
  int valueB = floor((value-valueA) * 10000);
  
     sprintf(result,
             "[{"
                "\"importCode\": \"%s\", "
                "\"utcTimeStamp\": %s, "
                "\"value\":\"%i.%i\""
             "}]",
             importCode, utcTimeStamp, valueA, valueB);
  
    return result;
}

int reportValue(char importCode[], time_t t, float value) {
  
  Adafruit_CC3000* cc3000 = getCC3000();
  
  uint32_t ip = getWebServerIP(cc3000);
  
  if (ip == 0) {
    return 1;
  }
    
  char timeStampString[50];
  sprintTimeStamp(timeStampString, t);
  
  char contentSizeString[50];
  char* content = createJSONString(importCode, timeStampString, value);
  
  //sprintf(contentSizeString, "%i", strlen(content));
  

  // Try connecting to the website.
  //   Note: HTTP/1.1 protocol is used to keep the server from closing the connection before all data is read.
  ..
  Adafruit_CC3000_Client www = cc3000->connectTCP(ip, 80);
  if (www.connected()) {
    www.fastrprint(F("POST "));
    www.fastrprint(JSON_WEBPAGE);
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Accept: *\r\n"));
    www.fastrprint(F("Cache-Control: no-cache\r\n"));
    www.fastrprint(F("Authorization: Basic a2Vnc2NyaWJlOnRlc3Q=\r\n"));
    www.fastrprint(F("Content-Length: ")); www.fastrprint(contentSizeString); www.fastrprint(F("\r\n"));
    www.fastrprint(F("Content-Type: *; charset=UTF-8\r\n"));
    www.fastrprint(F("User-Agent: KegScribe\r\n"));
    www.fastrprint(F("Host: ")); www.fastrprint(WEBSITE); www.fastrprint(F("\r\n"));
    www.println();
    www.fastrprint(content);
  } else {
    Serial.println(F("Connection failed"));
    free(content);
    www.close();
    return 1;
  }
  
  free(content);

  // Read data until either the connection is closed, or the idle timeout is reached.
  unsigned long lastRead = millis();
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (www.available()) {
      char c = www.read();
      Serial.print(c);
      lastRead = millis();
    }
  }
  www.close();
  
  return 0;
}

*/


