/**

Copy the defines below into your own Config.ino

Config.ino is in .gitignore and shouldn't be added
to the repository.

/*

// Fill this with your Wifi Credentials
#define WLAN_SSID           "Mai Wifu"           // cannot be longer than 32 characters!
#define WLAN_PASS           "Mai Wifu Password"
#define WLAN_SECURITY       WLAN_SEC_WPA2

#define USE_HARD_CODED_IP   true
#define HARD_CODED_IP       172, 16, 25, 63
//#define HARD_CODED_IP     10, 0, 1, 9

// change this to match your SD shield or module;
//     Arduino Ethernet shield: pin 4
//     Adafruit SD shields and modules: pin 10
//     Sparkfun SD shield: pin 8
#define SD_CHIP_SELECT_PIN  4

// Webserver Address
#define WEBSITE             "my.webserver.com"

// NTP Server Address
#define NTP_SERVER          "pool.ntp.org"

// Path on the Server to report to
#define CSV_WEBPAGE         "/import/simpleCSV"
#define JSON_WEBPAGE        "/import/simpleJSON"

#define IDLE_TIMEOUT_MS     3000      // Amount of time to wait (in milliseconds) with no data 
                                      // received before closing the connection.  If you know the server
                                      // you're accessing is quick to respond, you can reduce this value.                               

*/
