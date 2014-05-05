
/* scope used to save space */
char cbBuf[11 * sizeof (int) + 1];

/*
 * Write an integer to a character buffer
 * the buffer will be padded with `pad`
 * number of 0 characters.
 * Returns the number of characters written
 * a null character is not written and must
 * be manually added.
 */
byte cbPrintInt(char* buf, int n, int pad) {
  itoa(n,cbBuf,10);
  byte l = strlen(cbBuf);
  if (l < pad) {
    pad -= l;
    memset(buf, '0', pad);
    l = pad;
  } else {
    pad = 0;
  }
  memcpy(&buf[pad], cbBuf, l);
  return (l+pad);
}

byte cbPrintInt(char* buf, int n) {
  return cbPrintInt(buf, n, 0);
}

// provide a string buffer of at least 20 characters
// the buffer will be filled like: ####.####
byte sprintFloat(char* buffer, float* ptrValue) {
  int valueA = floor(*ptrValue);
  int valueB = floor((*ptrValue-valueA) * 10000);
  byte offset = cbPrintInt(buffer, valueA);
  buffer[offset++] = '.';
  offset += cbPrintInt(&buffer[offset], valueB);
  buffer[offset] = 0;
  return offset;
}

// provide a string buffer of at least 20 characters
// the buffer will be filled like: 2012-03-29T17:00:00
byte sprintTime(char* buffer, time_t* t, bool shortMode) {
  
  if (shortMode) {
    cbPrintInt(buffer, year(*t), 4);
    cbPrintInt(&buffer[4], month(*t), 2);
    cbPrintInt(&buffer[6], day(*t), 2);
    return 8;
  }
  
  // first copy our template
  memcpy(buffer, "0000-00-00T00:00:00", 20);
  cbPrintInt(buffer,      year(*t),   4);
  cbPrintInt(&buffer[5],  month(*t),  2);
  cbPrintInt(&buffer[8],  day(*t),    2);
  cbPrintInt(&buffer[11], hour(*t),   2);
  cbPrintInt(&buffer[14], minute(*t), 2);
  cbPrintInt(&buffer[17], second(*t), 2);
  return 19;
}

// provide a string buffer of at least 13 characters
// the buffer will be filled like: ########.CSV
byte sprintFilename(char* buffer, time_t* t) {
  byte offset = sprintTime(buffer, t, true);
  memcpy(&buffer[offset], ".CSV", 5);
  offset += 5;
  return offset;
}

// buf must be 13 character buffer
byte cbPrintFilename(char* buf, dir_t dir) {
  byte offset = 0;
  for(uint8_t i = 0; i < 11; i++){
    if(dir.name[i] == ' ') continue;
    if (i == 8) {
        buf[offset++] = '.';
    }
    buf[offset++] = (char)(dir.name[i]);
  }
  buf[offset] = 0;
  return offset;
}
