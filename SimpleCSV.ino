SdCard card;
Fat16 file;

#define FORM_BOUNDARY_BASE       F("KegScribeCSVFile")
#define FORM_BOUNDARY            F("--KegScribeCSVFile")
#define FORM_BOUNDARY_END        F("--KegScribeCSVFile--\r\n")
#define FORM_BOUNDARY_END_SIZE   22
#define FORM_BOUNDARY_START      F("--KegScribeCSVFile\r\n")
#define FORM_BOUNDARY_START_SIZE 20 

#define FILE_HEADER              F("Content-Disposition: form-data; name=\"file\"; filename=\"filename.csv\"\r\nContent-Type: text/csv\r\n\r\n")
#define FILE_HEADER_SIZE         96
//#define SUCCESS_RESPONSE         F("HTTP/1.1 200 OK\r\n")
const char SUCCESS_RESPONSE[] PROGMEM = "HTTP/1.1 200 OK\r\n";

#define SUCCESS_RESPONSE_SIZE    17

void initSD() {
  pinMode(SD_CHIP_SELECT_PIN, OUTPUT);
  
  // initialize the SD card
  Serial.print(F("sd"));
  if (!card.init(false, SD_CHIP_SELECT_PIN)) {
    Serial.print(" error ");
    Serial.print(card.errorCode);
    Serial.print(FAIL_MSG);
    return;
  }
  
  // initialize a FAT16 volume
  Serial.print(F("fat16"));
  if (!Fat16::init(&card)) {
    Serial.print(" error ");
    Serial.print(card.errorCode);
    Serial.print(FAIL_MSG);
    return;
  }
  
  Serial.print(OK_MSG);
}



int recordValue(const char importCode[], time_t* t, float value) {
  
  char filename[13];
  sprintFilename(filename, t);
  
  char timestamp[20];
  sprintTime(timestamp, t, false);
  
  char valueString[16];
  sprintFloat(valueString, value);
  
  // if the file didn't open, print an error:
  Serial.print(F("writing "));
  Serial.print(filename);
  
  // O_CREAT - create the file if it does not exist
  // O_WRITE - open for write
  file.open(filename, O_CREAT | O_APPEND | O_WRITE );
  
  if (!file.isOpen()) {
    // if the file didn't open, print an error:
    Serial.print(FAIL_MSG);
    return 1;
  }
    
  if(file.fileSize() == 0) {
    file.print(F("ImportCode,Timestamp,Value\r\n"));
  }
    
  file.print(importCode); file.print(F(","));
  file.print(timestamp); file.print(F(","));
  file.print(valueString); file.print(F("\r\n"));
  
  // close the file:
  file.close();
  Serial.print(OK_MSG);
  
  return 0;
}

int safeprint(Adafruit_CC3000_Client www, const __FlashStringHelper* string) {
  delay(CC3000_DELAY);
  return www.fastrprint(string);
}

int safeprint(Adafruit_CC3000_Client www, const char* string) {
  delay(CC3000_DELAY);
  return www.fastrprint(string);
}

int reportFile(Fat16* file) {
  Adafruit_CC3000* cc3000 = getCC3000();
  
  uint32_t ip = getWebServerIP(cc3000);
  
  if (ip == 0) {
    return 1;
  }
  
  /* Try connecting to the website.
     Note: HTTP/1.1 protocol is used to keep the server from closing the connection before all data is read.
  */
  Serial.print(F("tcp connect"));
  Adafruit_CC3000_Client www = cc3000->connectTCP(ip, 80);
        
  // calculate the content-length in bytes
  uint32_t totalContentLength = 
    FORM_BOUNDARY_START_SIZE +
    FILE_HEADER_SIZE +
    (uint32_t)file->fileSize() + 2 +// file data + crlf
    FORM_BOUNDARY_END_SIZE;
  
  Serial.print(F("len: "));
  Serial.println(totalContentLength);
        
  if (www.connected()) {
    safeprint(www, F("POST "));
    safeprint(www, CSV_WEBPAGE);
    safeprint(www, F(" HTTP/1.1"));
    safeprint(www, F("\r\nHost: "));
    safeprint(www, WEBSITE);
    safeprint(www, F("\r\nAuthorization: Basic a2Vnc2NyaWJlOnRlc3Q="));
    safeprint(www, F("\r\nUser-Agent: KegScribe"));
    safeprint(www, F("\r\nConnection: Close"));
    safeprint(www, F("\r\nContent-Length: "));
    www.print(totalContentLength);
    safeprint(www, F("\r\nContent-Type: multipart/form-data; boundary="));
    safeprint(www, FORM_BOUNDARY_BASE);
    safeprint(www, F("\r\n"));
    safeprint(www, F("\r\n"));
    
    safeprint(www, FORM_BOUNDARY_START);
    safeprint(www, FILE_HEADER);
    
    int16_t c;
    while ((c = file->read()) > 0) {
      Serial.print(c);
      www.write(&c, 1, 0);
      delay(CC3000_DELAY);
    }
    
    safeprint(www, F("\r\n"));
    safeprint(www, FORM_BOUNDARY_END);
    
    Serial.print(" sent");
    
    // Read data until either the connection is closed, or the idle timeout is reached.
    byte i = 0;
    unsigned long lastRead = millis();
    while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
      delay(CC3000_DELAY);
      while (www.available() && (i < SUCCESS_RESPONSE_SIZE)) {
        char c = www.read();
        Serial.print(c);
        if ((i < SUCCESS_RESPONSE_SIZE) && c != SUCCESS_RESPONSE[i++]) {
          Serial.print(FAIL_MSG);
          www.close();
          return 2;
        }
        lastRead = millis();
        delay(CC3000_DELAY);
      }
    }
    
    Serial.print(OK_MSG);
    
  } else {
    Serial.print(FAIL_MSG);
    www.close();
    return 1;
  }
  
  www.close();
  
  return 0;
}

void reportFiles() {
  
  char filename[13];
  dir_t dir;
  for (uint16_t index = 0; file.readDir(&dir, &index, DIR_ATT_VOLUME_ID); index++) {
    //for (uint8_t i = 0; i < 11; i++) {
    //  if (dir.name[i] == ' ') { continue; }
    // todo: only use valid file names
    //}
    
    if (!DIR_IS_FILE(&dir)) {
      continue;
    }
    
    // skip anything that is not a csv
    if (dir.name[8]  != 'C' ||
        dir.name[9]  != 'S' ||
        dir.name[10] != 'V' ) {
      continue; 
    }
    
    Serial.print("read ");
    cbPrintFilename(filename, dir);
    Serial.print(filename);
    
    file.open(filename, O_READ);
    if (!file.isOpen()) {
      // no more files
      Serial.print(FAIL_MSG);
      continue;
    }
    
    Serial.print(OK_MSG);
   
    if(reportFile(&file) == 0) {
      // successfully sent the data
      // so we can remove this file
      file.remove(filename);
    }
    
    file.close();
  }
}


