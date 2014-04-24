
// What page to grab!
#define WEBSITE      "mikazuki.local"
#define WEBPAGE      "/import/simpleJSON"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "Pad"           // cannot be longer than 32 characters!
#define WLAN_PASS       "star*buck$"

// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

// secondary SSID to try if the first is unavailable
#define WLAN_FALLBACK_SSID       "hotcrossbuns"           // cannot be longer than 32 characters!
#define WLAN_FALLBACK_PASS       "cafecafe12"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_FALLBACK_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.


void ConnectWifi() {

  Serial.begin(115200);
  Serial.println(F("Hello, KegScribe!\n")); 

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  
  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  // Optional SSID scan
  // listSSIDResults();
  
  Serial.print(F("\nAttempting to connect to SSID: ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.print(F("\nAttempting to connect to fallback SSID: ")); Serial.println(WLAN_FALLBACK_SSID);
    if (!cc3000.connectToAP(WLAN_FALLBACK_SSID, WLAN_FALLBACK_PASS, WLAN_FALLBACK_SECURITY)) {
      Serial.println(F("Failed!"));
      while(1);
    }
  }
   
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  /* Display the IP address DNS, Gateway, etc. */  
  while (! displayConnectionDetails()) {
    delay(1000);
  }
  
}


/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

char* createJSONString(char importCode[], char utcTimeStamp[], float value) {
     char* result = new char[strlen(importCode) + strlen(utcTimeStamp) + 100];
  
     sprintf(result,
             "{"
                "\"importCode\": \"%s\", "
                "\"utcTimeStamp\": %s, "
                "\"value\":\"%.5g\""
             "}",
             importCode, utcTimeStamp, value);
  
    return result;
}

void reportValue(char importCode[], float value) {
  
  char contentSizeString[50];
  char* content = createJSONString(importCode, "1398292028", value);
  
  sprintf(contentSizeString, "%i", strlen(content));
  
  uint32_t ip = 0;
  // Try looking up the website's IP address
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  }

  cc3000.printIPdotsRev(ip);
  
  /* Try connecting to the website.
     Note: HTTP/1.1 protocol is used to keep the server from closing the connection before all data is read.
  */
  Adafruit_CC3000_Client www = cc3000.connectTCP(ip, 80);
  if (www.connected()) {
    www.fastrprint(F("POST "));
    www.fastrprint(WEBPAGE);
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Accept: */*\r\n"));
    www.fastrprint(F("Cache-Control: no-cache\r\n"));
    www.fastrprint(F("\r\n"));
    www.fastrprint(F("\r\n"));
    www.fastrprint(F("\r\n"));
    www.fastrprint(F("Content-Length: ")); www.fastrprint(contentSizeString); www.fastrprint(F("\r\n"));
    www.fastrprint(F("Content-Type: */*; charset=UTF-8\r\n"));
    www.fastrprint(F("User-Agent: KegScribe\r\n"));
    www.fastrprint(F("\r\n"));
    www.fastrprint(F("Host: ")); www.fastrprint(WEBSITE); www.fastrprint(F("\r\n"));
    www.println();
    www.fastrprint(content);
  } else {
    Serial.println(F("Connection failed"));
    free(content);
    return;
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
  Serial.println(F("\n\nDisconnecting"));
  cc3000.disconnect(); 
  
}


