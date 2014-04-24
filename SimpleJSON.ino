
// What page to grab!
#define WEBSITE      "mikazuki.local"
#define WEBPAGE      "/import/simpleJSON"

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

char* createJSONString(char importCode[], char utcTimeStamp[], float value) {
  char* result = new char[strlen(importCode) + strlen(utcTimeStamp) + 100];
  
  int valueA = floor(value);
  int valueB = floor(value-valueA * 10000);
  
  
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
    
  char timeStampString[50];
  sprintTimeStamp(timeStampString, t);  
  
  Serial.println(timeStampString);
  
  char contentSizeString[50];
  char* content = createJSONString(importCode, timeStampString, value);
  
  Serial.println(content);
  
  sprintf(contentSizeString, "%i", strlen(content));
  
  Serial.println(contentSizeString);
  
  uint32_t ip = 0;
  
  // hard coded for now. :(
  ip = cc3000->IP2U32(HARD_CODED_IP);
  
  // Try looking up the website's IP address
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000->getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    free(content);
    return 1;
  }

  cc3000->printIPdotsRev(ip);
  Serial.println();
  
  /* Try connecting to the website.
     Note: HTTP/1.1 protocol is used to keep the server from closing the connection before all data is read.
  */
  Adafruit_CC3000_Client www = cc3000->connectTCP(ip, 80);
  if (www.connected()) {
    www.fastrprint(F("POST "));
    www.fastrprint(WEBPAGE);
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Accept: */*\r\n"));
    www.fastrprint(F("Cache-Control: no-cache\r\n"));
    www.fastrprint(F("Authorization: Basic a2Vnc2NyaWJlOnRlc3Q=\r\n"));
    www.fastrprint(F("Content-Length: ")); www.fastrprint(contentSizeString); www.fastrprint(F("\r\n"));
    www.fastrprint(F("Content-Type: */*; charset=UTF-8\r\n"));
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

  Serial.println(F("-------------------------------------"));
  
  /* Read data until either the connection is closed, or the idle timeout is reached. */ 
  unsigned long lastRead = millis();
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (www.available()) {
      char c = www.read();
      Serial.print(c);
      lastRead = millis();
    }
  }
  www.close();
  Serial.println(F("-------------------------------------"));
  
  /* You need to make sure to clean up after yourself or the CC3000 can freak out */
  /* the next time your try to connect ... */
  //Serial.println(F("\n\nDisconnecting"));
  // should only do this when we don't need wifi anymore
  //cc3000.disconnect(); 
  
  return 0;
}


