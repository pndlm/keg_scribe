/*
Functions to initialize the Wifi connection
and lookup the web server IP address.
*/

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

void initWifi() {
  
  /* Initialise the module */
  Serial.print(F("wifi"));
  if (!cc3000.begin())
  {
    Serial.print(FAIL_MSG);
    while(1);
  }
  Serial.print(OK_MSG);
  
  ensureWifiConnection();
  
}

boolean ensureWifiConnection() {
  
  if (!cc3000.checkConnected()) {
  
    Serial.print(F("ssid ")); Serial.print(WLAN_SSID);
    if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
      Serial.print(FAIL_MSG);
      return false;
    }
    Serial.print(OK_MSG);
    
    /* Wait for DHCP to complete */
    do
    {
      delay(100); // ToDo: Insert a DHCP timeout!
    } while (!cc3000.checkDHCP()); 
  
    /* Display the IP address DNS, Gateway, etc. */  
    //while (! displayConnectionDetails()) {
    //  delay(1000);
    //}
    
    return true;
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
    Serial.print(FAIL_MSG);
    return false;
  }
  else
  {
    Serial.print(OK_MSG);
    
    Serial.print(F("\nip addr  ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nnetmask  ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\ngateway  ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\ndhcp srv ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\ndns srv  ")); cc3000.printIPdotsRev(dnsserv);
    Serial.print(F("\n"));
    
    return true;
  }
}

Adafruit_CC3000* getCC3000() {
  return &cc3000;
}

uint32_t getPrimaryServerIP(Adafruit_CC3000* cc3000) {
  uint32_t ip = 0;
  
  #if USE_HARD_CODED_IP_PRIMARY
    Serial.print(F("fixed ip "));
    ip = cc3000->IP2U32(HARD_CODED_IP_PRIMARY);
  #else
    // Try looking up the website's IP address
    if (ip == 0) {
      Serial.print(F("nslookup "));
      Serial.print(PRIMARY_SERVER);
      if (! cc3000->getHostByName(PRIMARY_SERVER, &ip)) {
        Serial.print(FAIL_MSG);
        return 0;
      }
    }
  #endif
  
  cc3000->printIPdotsRev(ip);
  Serial.print(NEWLINE_MSG);
  
  return ip;
}

/*
uint32_t getSecondaryServerIP(Adafruit_CC3000* cc3000) {
  uint32_t ip = 0;
  
  #if USE_HARD_CODED_IP_SECONDARY
    Serial.print(F("fixed ip "));
    ip = cc3000->IP2U32(HARD_CODED_IP_SECONDARY);
  #else
    // Try looking up the website's IP address
    if (ip == 0) {
      Serial.print(F("nslookup "));
      Serial.print(SECONDARY_SERVER);
      if (! cc3000->getHostByName(SECONDARY_SERVER, &ip)) {
        Serial.print(FAIL_MSG);
        return 0;
      }
    }
  #endif
  
  cc3000->printIPdotsRev(ip);
  Serial.print(F("\r\n"));
  
  return ip;
}
*/

