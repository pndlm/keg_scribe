SdCard card;
Fat16 file;

#define FORM_BOUNDARY "KegScribeCSVFile"

void initSD() {
  pinMode(SS, OUTPUT);
  
  // initialize the SD card
  Serial.print(F("sd"));
  if (!card.init()) {
    Serial.print(FAIL_MSG);
    return;
  }
  
  // initialize a FAT16 volume
  Serial.print(F("fat16"));
  if (!Fat16::init(&card)) {
    Serial.print(FAIL_MSG);
    return;
  }
  
  Serial.print(OK_MSG);
}

// provide a string buffer of at least 20 characters
// the buffer will be filled like: ####.####
int sprintFilename(char* buffer, time_t* t) {
  short offset = 0;
  offset += cbPrintInt(&buffer[offset], year(*t));
  offset += cbPrintInt(&buffer[offset], month(*t));
  offset += cbPrintInt(&buffer[offset], day(*t));
  buffer[offset++] = '.';
  buffer[offset++] = 'c';
  buffer[offset++] = 's';
  buffer[offset++] = 'v';
  return offset;
}

// provide a string buffer of at least 20 characters
// the buffer will be filled like: ####.####
int sprintFloat(char* buffer, float value) {
  int valueA = floor(value);
  int valueB = floor((value-valueA) * 10000);
  short offset = cbPrintInt(buffer, valueA);
  buffer[offset++] = '.';
  offset += cbPrintInt(&buffer[offset], valueB);
  return offset;
}

int recordValue(char importCode[], time_t* t, float value) {
  
  char filename[12];
  sprintFilename(filename, t);
  
  char timestamp[20];
  sprintTime(timestamp, t);
  
  char valueString[16];
  sprintFloat(valueString, value);
  
  // if the file didn't open, print an error:
  Serial.print(F("open file "));
  Serial.print(filename);
  Serial.print("\r\n");
  
  // O_CREAT - create the file if it does not exist
  // O_WRITE - open for write
  file.open(filename, O_CREAT | O_WRITE);
  
  if (!file.isOpen()) {
    // if the file didn't open, print an error:
    Serial.print(FAIL_MSG);
    return 1;
  }
    
  if(file.fileSize() == 0) {
    Serial.print(F("new file\r\n"));
    file.print(F("ImportCode,Timestamp,Value\r\n"));
  }
    
  file.print(importCode); file.print(",");
  file.print(timestamp); file.print(",");
  file.print(valueString); file.print("\r\n");
  
  // close the file:
  file.close();
  
  return 0;
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
  Adafruit_CC3000_Client www = cc3000->connectTCP(ip, 80);
  
  if (www.connected()) {
    www.fastrprint(F("POST "));
    www.fastrprint(CSV_WEBPAGE);
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Accept: */*\r\n"));
    //www.fastrprint(F("Cache-Control: no-cache\r\n"));
    www.fastrprint(F("Authorization: Basic a2Vnc2NyaWJlOnRlc3Q=\r\n"));
    www.fastrprint(F("Content-Length: "));
    www.print(file->fileSize());
    www.fastrprint(F("\r\nContent-Type: multipart/form-data; boundary=")); www.fastrprint(FORM_BOUNDARY);
    //www.fastrprint(F("\r\nUser-Agent: KegScribe\r\n"));
    www.fastrprint(F("Host: ")); www.fastrprint(CSV_WEBPAGE); www.fastrprint(F("\r\n"));
    www.print("\r\n");
    www.fastrprint("--"); www.fastrprint(FORM_BOUNDARY);
    www.fastrprint(F("Content-Disposition: form-data; name=\"f\"; filename=\""));
    www.fastrprint(F("filename.csv"));
    www.fastrprint(F("\"\r\n"));
    www.fastrprint(F("Content-Type: application/octet-stream\r\n"));
    www.fastrprint(F("Content-Transfer-Encoding: binary\r\n"));
    www.print("\r\n");
    
    /*
    int16_t n;
    uint8_t buf[7];// nothing special about 7, just a lucky number.
    while ((n = file->read(buf, sizeof(buf))) > 0) {
      for (uint8_t i = 0; i < n; i++) {
        www.print(buf[i]);
      }
    }
    */
    
    int16_t c;
    while ((c = file->read()) > 0) www.print((char)c);
    
    www.fastrprint("--"); www.fastrprint(FORM_BOUNDARY); www.fastrprint("--");
    
  } else {
    Serial.print(FAIL_MSG);
    www.close();
    return 1;
  }
  
  www.close();
  
  return 0;
}

void reportFiles() {
  
  dir_t dir;
  for (uint16_t index = 0; file.readDir(&dir, &index, DIR_ATT_VOLUME_ID); index++) {
    //for (uint8_t i = 0; i < 11; i++) {
    //  if (dir.name[i] == ' ') { continue; }
    // todo: only use valid file names
    //}
    
    if (!DIR_IS_FILE(&dir)) {
      continue;
    }
    
    Serial.print("read ");
    Serial.print((char*)(dir.name));

    file.open((char*)(dir.name), O_READ);
    if (!file.isOpen()) {
      // no more files
      Serial.print(FAIL_MSG);
      continue;
    }
    
    Serial.print(OK_MSG);
   
    if(reportFile(&file) == 0) {
      // successfully sent the data
      // so we can remove this file
      file.remove((char*)(dir.name));
    }
    
    file.close();
  }
}


