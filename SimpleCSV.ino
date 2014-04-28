                                   
#define FORM_BOUNDARY "KegScribeCSVFile"

void initSD() {
  pinMode(SS, OUTPUT);
  Serial.print("init ");
  
  if (!SD.begin(CHIP_SELECT)) {
    Serial.println("fail!");
    return;
  }
  Serial.println("ok."); 
}

// provide a string buffer of at least 16 characters
// the buffer will be filled like: 2012-03-29T17:00:00
char* sprintFloat(char* buffer, float value) {
  int valueA = floor(value);
  int valueB = floor((value-valueA) * 10000);
  
  sprintf(buffer, "%010i.015%i", valueA, valueB);         
}

int recordValue(char importCode[], time_t t, float value) {
  
  char filename[16];
  sprintf(filename, "%4i-2%i-2%i.csv", year(t), month(t), day(t));
  
  char timestamp[20];
  sprintTime(timestamp, t);
  
  char valueString[16];
  sprintFloat(valueString, value);
  
  // if the file didn't open, print an error:
  Serial.print(F("open file "));
  Serial.println(filename);
  
  File file = SD.open(filename, FILE_WRITE);
  
  if (file) {
    
    if(file.size() == 0) {
      Serial.println(F("new file"));
      file.print(F("ImportCode,Timestamp,Value\n")); 
    }
    
    file.print(importCode); file.print(",");
    file.print(timestamp); file.print(",");
    file.print(valueString); file.print("\n");
    
    // close the file:
    file.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file.");
    return 1;
  }
  
  return 0;
}

int reportFile(File file) {
  Adafruit_CC3000* cc3000 = getCC3000();
  
  uint32_t ip = getWebServerIP(cc3000);
  
  if (ip == 0) {
    return 1;
  }
  
  /* Try connecting to the website.
     Note: HTTP/1.1 protocol is used to keep the server from closing the connection before all data is read.
  */
  Adafruit_CC3000_Client www = cc3000->connectTCP(ip, 80);
  
  if (www.connected()) {
    www.fastrprint(F("POST "));
    www.fastrprint(CSV_WEBPAGE);
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Accept: */*\r\n"));
    www.fastrprint(F("Cache-Control: no-cache\r\n"));
    www.fastrprint(F("Authorization: Basic a2Vnc2NyaWJlOnRlc3Q=\r\n"));
    www.fastrprint(F("Content-Length: "));
    www.print(file.size());
    www.fastrprint(F("\r\nContent-Type: multipart/form-data; boundary=")); www.fastrprint(FORM_BOUNDARY);
    www.fastrprint(F("\r\nUser-Agent: KegScribe\r\n"));
    www.fastrprint(F("Host: ")); www.fastrprint(CSV_WEBPAGE); www.fastrprint(F("\r\n"));
    www.println();
    www.fastrprint("--"); www.fastrprint(FORM_BOUNDARY);
    www.fastrprint(F("Content-Disposition: form-data; name=\"file\"; filename=\""));
    www.fastrprint(F("\"\r\n"));
    www.fastrprint(F("Content-Type: application/octet-stream\r\n"));
    www.fastrprint(F("Content-Transfer-Encoding: binary\r\n"));
    www.println();
    
    while (file.available()) {
      www.fastrprint((char*)(file.read()));
    }
    
    www.fastrprint("--"); www.fastrprint(FORM_BOUNDARY); www.fastrprint("--");
    
  } else {
    Serial.println(F("Connection failed"));
    www.close();
    return 1;
  }

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
  
  return 0;
}

void reportFiles() {
  
  File dir = SD.open("/");
  
  // Begin at the start of the directory
  dir.rewindDirectory();
  
  while(true) {
    File file = dir.openNextFile();
    if (!file) {
      // no more files
      break;
    }
   
    if(reportFile(file) == 0) {
      // successfully sent the data
      // so we can remove this file
      SD.remove(file.name());
      file.close();
    }
    
    file.close();
  }
}


