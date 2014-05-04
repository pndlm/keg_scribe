SdCard card;
Fat16 file;

#define FORM_BOUNDARY "KegScribeCSVFile"

void initSD() {
  pinMode(SD_CHIP_SELECT_PIN, OUTPUT);
  
  // initialize the SD card
  Serial.print(F("sd"));
  if (!card.init(0, SD_CHIP_SELECT_PIN)) {
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



int recordValue(char importCode[], time_t* t, float value) {
  
  Serial.print(F("filename "));
  
  char filename[13];
  sprintFilename(filename, t);
  
  Serial.print(filename); Serial.print("\r\ntimestamp ");
  
  char timestamp[20];
  sprintTime(timestamp, t, false);
  
  Serial.print(timestamp); Serial.print("\r\nvalue ");
  
  char valueString[16];
  sprintFloat(valueString, value);
  
  Serial.print(valueString); Serial.print("\r\n");
  
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
    www.fastrprint(F("POST ")); www.fastrprint(CSV_WEBPAGE); www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Host: ")); www.fastrprint(CSV_WEBPAGE); www.fastrprint(F("\r\n"));
    www.fastrprint(F("\r\nContent-Type: multipart/form-data; boundary=")); www.fastrprint(FORM_BOUNDARY);
    //www.fastrprint(F("\r\nUser-Agent: KegScribe\r\n"));
    www.fastrprint(F("Content-Length: ")); //www.print(file->fileSize());
    www.fastrprint(F("\r\nAuthorization: Basic a2Vnc2NyaWJlOnRlc3Q=\r\n"));
    www.fastrprint("\r\n--"); www.fastrprint(FORM_BOUNDARY);
    www.fastrprint(F("Content-Disposition: form-data; name=\"f\"; filename=\"filename.csv\"\r\n"));
    www.fastrprint(F("Content-Type: text/csv\r\n\r\n"));
    
    int16_t n;
    uint8_t buf[7] = {0,0,0,0,0,0,0};// nothing special about 7, just a lucky number.
    // read sizeof(buf)-1 bytes into buf
    while ((n = file->read(buf, sizeof(buf)-1)) > 0) {
      www.fastrprint((char*)buf);
    }
    
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


