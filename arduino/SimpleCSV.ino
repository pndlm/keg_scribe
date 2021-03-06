/*
Functions to write a CSV file to an SD card,
read those files from the SD card, and
report the files with HTTP POST multipart form
to a web server.
I used Fat16lib to access the SD card due to
the small memory footprint it requires.

See https://code.google.com/p/fat16lib/ for details.
*/

SdCard card;
Fat16 file;

#define FORM_BOUNDARY_BASE       F("KSFile")
#define FORM_BOUNDARY_END        F("--KSFile--\r\n")
#define FORM_BOUNDARY_END_SIZE   12
#define FORM_BOUNDARY_START      F("--KSFile\r\n")
#define FORM_BOUNDARY_START_SIZE 10

#define FILE_HEADER              F("Content-Disposition: form-data; name=\"file\"; filename=\"f.csv\"\r\nContent-Type: text/csv\r\n\r\n")
#define FILE_HEADER_SIZE         89 //       '         '         '          '          '          '            '         '            9

#define SUCCESS_RESPONSE         "HTTP/1.1 200 OK\r\n"
#define SUCCESS_RESPONSE_SIZE    16

void initSD() {
  pinMode(SD_CHIP_SELECT_PIN, OUTPUT);
  
  // initialize the SD card
  Serial.print(F("sd "));
  if (!card.init(false, SD_CHIP_SELECT_PIN)) {
    Serial.print(card.errorCode);
    Serial.print(FAIL_MSG);
    return;
  }
  
  // initialize a FAT16 volume
  Serial.print(F("fat16"));
  if (!Fat16::init(&card)) {
    Serial.print(card.errorCode);
    Serial.print(FAIL_MSG);
    return;
  }
  
  Serial.print(OK_MSG);
}

bool recordValue(const __FlashStringHelper importCode[], time_t* t, float ptrValue) {
  
  char filename[13];
  sprintFilename(filename, t);
  
  char timestamp[20];
  sprintTime(timestamp, t, false);
  
  char valueString[16];
  sprintFloat(valueString, &ptrValue);
  
  // if the file didn't open, print an error:
  //Serial.print(F("write "));
  //Serial.print(filename);
  
  Serial.print(timestamp); Serial.print(' '); Serial.print(valueString); 
  Serial.print(' '); Serial.print(importCode); Serial.print(NEWLINE_MSG);
  
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
    
  file.print(importCode); file.print(CSV_SEPARATOR);
  file.print(timestamp); file.print(CSV_SEPARATOR);
  file.print(valueString); file.print(NEWLINE_MSG);
  
  // close the file:
  file.close();
  //Serial.print(OK_MSG);
  
  return 0;
}

bool reportFile(Fat16* file, uint32_t ip, const char* hostname) {
  
  Adafruit_CC3000* cc3000 = getCC3000();
  
  if (!ensureWifiConnection() || ip == 0) {
    return 1;
  }
  
  /* Try connecting to the website.
     Note: HTTP/1.1 protocol is used to keep the server from closing the connection before all data is read.
  */
  Adafruit_CC3000_Client www = cc3000->connectTCP(ip, 80);
  
  // calculate the content-length in bytes
  uint32_t totalContentLength = 
    FORM_BOUNDARY_START_SIZE +
    FILE_HEADER_SIZE +
    (uint32_t)file->fileSize() + 2 + // file data + crlf
    FORM_BOUNDARY_END_SIZE;
    
  boolean success = false;
  
  //char totalContentLengthString[12];
  //byte length = cbPrintInt(totalContentLengthString, (int)totalContentLength, 0);
  //totalContentLengthString[length] = '\0';
  
  if (www.connected()) {
    
    www.fastrprint(F("POST "));
    
    www.fastrprint(CSV_WEBPAGE);
    
    www.fastrprint(F(" HTTP/1.1\r\nHost: "));
    
    www.fastrprint(hostname);
    
    www.fastrprint(F("\r\nAuthorization: Basic " BASIC_AUTH_KEY "\r\nContent-Length: "));
    //www.fastrprint(totalContentLengthString);
    www.print(totalContentLength);
    
    www.fastrprint(F("\r\nContent-Type: multipart/form-data; boundary="));
    www.fastrprint(FORM_BOUNDARY_BASE);
    
    www.fastrprint(NEWLINE_MSG);
    www.fastrprint(NEWLINE_MSG);
    
    www.fastrprint(FORM_BOUNDARY_START);
    www.fastrprint(FILE_HEADER);
    
    uint16_t n;    
    char buf[64];
    while ((n = uint16_t(file->read(&buf, sizeof(buf)))) > 0) {
      www.write(buf, n, 0);
    }
    
    www.fastrprint(NEWLINE_MSG);
    www.fastrprint(FORM_BOUNDARY_END);
    
    // Read data until either the connection is closed, or the idle timeout is reached.
    byte i = 0;
    
    success = true;
    
    unsigned long lastRead = millis();
    while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
      // read to the first white space
      while (www.available()) {
        char c = www.read();
        //Serial.print(c);
        if (success && i < SUCCESS_RESPONSE_SIZE) {
          if (c != SUCCESS_RESPONSE[i++]) {
            success = false;
          }
        }
        lastRead = millis();
      }
    }
  } 
  
  if (!success) {
    www.close();
    return 1;
  }
  
  www.close();
  
  return 0;
}

void reportFiles() {
  dir_t dir;
  char filename[13];
  uint16_t index = 0;
  
  Adafruit_CC3000* cc3000 = getCC3000();
  
  if(!ensureWifiConnection()) {
    return;
  }
  
  uint32_t primaryIP = getPrimaryServerIP(cc3000);
  //uint32_t secondaryIP = getSecondaryServerIP(cc3000);
  
  boolean success = false;
  
  for (uint16_t index = 0; file.readDir(&dir, &index, DIR_ATT_VOLUME_ID); index++) {
    
    if (!DIR_IS_FILE(&dir)) {
      continue;
    }
    
    for (byte i = 0; i < 3; i++) {
      if (dir.name[i+8] != "CSV"[i]) {
        // continue outside loop;
        goto nextfile;
      }
    }
    
    Serial.print(F("read "));
    cbPrintFilename(filename, dir);
    Serial.print(filename);
    
    file.open(filename, O_READ);
    if (!file.isOpen()) {
      // no more files
      Serial.print(FAIL_MSG);
      continue;
    }
    
    Serial.print(OK_MSG);
 
    // try to report to secondary
    // but don't care if it fails   
    //Serial.print("srv2 ");
    //Serial.print(reportFile(&file, secondaryIP, SECONDARY_SERVER) == 0 ? OK_MSG : FAIL_MSG);
    
    //file.rewind();

    Serial.print(F("http"));
  
    success = (reportFile(&file, primaryIP, PRIMARY_SERVER) == 0);

    Serial.print(F(" close"));

    file.close();
    
    if(success) {
      Serial.print(OK_MSG);
      
      // successfully sent the data
      // so we can remove this file
      Serial.print(F("rm"));
      
      if (file.remove(filename)) {
        Serial.print(OK_MSG);
      } else {
        Serial.print(FAIL_MSG);
      }
      
    } else {
      Serial.print(FAIL_MSG);
    }
    
nextfile:;
  }
  
}


