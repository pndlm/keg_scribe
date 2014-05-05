
/* scope used to save space */
char cbBuf[11 * sizeof (int) + 1];

/*
 * Write an unsigned number to file
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
  memcpy(&buf[pad], cbBuf, l+1);
  
  return (l+pad);
}

byte cbPrintInt(char* buf, int n) {
  return cbPrintInt(buf, n, 0);
}

/*
 * Write an unsigned number to file
 */
/*
int cbPrintULong(char* buf, unsigned long n, int pad) {
  ultoa(n,cbBuf,10);
  return 0;
  //return cbPrintBuf(buf, n, pad);
}
*/

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

/*
int sprintTimeStamp(char* buffer, time_t t) {
  return cbPrintULong(buffer, t, 0);
}
*/

// provide a string buffer of at least 20 characters
// the buffer will be filled like: 2012-03-29T17:00:00
byte sprintTime(char* buffer, time_t* t, bool shortMode) {
  byte offset = 0;
  offset += cbPrintInt(&buffer[offset], year(*t), 4);
  if(!shortMode) { buffer[offset++] = '-'; }
  offset += cbPrintInt(&buffer[offset], month(*t), 2);
  if(!shortMode) { buffer[offset++] = '-'; }
  offset += cbPrintInt(&buffer[offset], day(*t), 2);
  if (!shortMode) { 
    buffer[offset++] = 'T';
    offset += cbPrintInt(&buffer[offset], hour(*t), 2);
    buffer[offset++] = ':';
    offset += cbPrintInt(&buffer[offset], minute(*t), 2);
    buffer[offset++] = ':';
    offset += cbPrintInt(&buffer[offset], second(*t), 2);
  }
  buffer[offset] = 0;
  return offset;
}

// provide a string buffer of at least 20 characters
// the buffer will be filled like: ####.####
byte sprintFilename(char* buffer, time_t* t) {
  byte offset = sprintTime(buffer, t, true);
  buffer[offset++] = '.';
  buffer[offset++] = 'C';
  buffer[offset++] = 'S';
  buffer[offset++] = 'V';
  buffer[offset] = 0;
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
