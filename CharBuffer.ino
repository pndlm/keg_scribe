

/*
 * Write an unsigned number to file
 */
int cbPrintInt(char* buf, int n) {
  char b[8 * sizeof (int) + 1];
  itoa(n,b,10);
  int l = strlen(b);
  
  memcpy(buf, b, l+1);
  return l;
}

/*
 * Write an unsigned number to file
 */
int cbPrintULong(char* buf, unsigned long n) {
  char b[16 * sizeof (int) + 1];
  ultoa(n,b,10);
  int l = strlen(b);
  
  memcpy(buf, b, l+1);
  return l;
}

// provide a string buffer of at least 20 characters
// the buffer will be filled like: ####.####
int sprintFloat(char* buffer, float value) {
  int valueA = floor(value);
  int valueB = floor((value-valueA) * 10000);
  short offset = cbPrintInt(buffer, valueA);
  buffer[offset++] = '.';
  offset += cbPrintInt(&buffer[offset], valueB);
  buffer[offset++] = 0;
  return offset;
}

int sprintTimeStamp(char* buffer, time_t t) {
  return cbPrintULong(buffer, t);
}

// provide a string buffer of at least 20 characters
// the buffer will be filled like: 2012-03-29T17:00:00
int sprintTime(char* buffer, time_t* t, bool shortMode) {
  short offset = 0;
  offset += cbPrintInt(&buffer[offset], year(*t));
  if(!shortMode) { buffer[offset++] = '-'; }
  offset += cbPrintInt(&buffer[offset], month(*t));
  if(!shortMode) { buffer[offset++] = '-'; }
  offset += cbPrintInt(&buffer[offset], day(*t));
  if (!shortMode) { 
    buffer[offset++] = 'T';
    offset += cbPrintInt(&buffer[offset], hour(*t));
    buffer[offset++] = ':';
    offset += cbPrintInt(&buffer[offset], minute(*t));
    buffer[offset++] = ':';
    offset += cbPrintInt(&buffer[offset], second(*t));
  }
  buffer[offset++] = 0;
  return offset;
}

// provide a string buffer of at least 20 characters
// the buffer will be filled like: ####.####
int sprintFilename(char* buffer, time_t* t) {
  short offset = sprintTime(buffer, t, true);
  buffer[offset++] = '.';
  buffer[offset++] = 'c';
  buffer[offset++] = 's';
  buffer[offset++] = 'v';
  buffer[offset++] = 0;
  return offset;
}
