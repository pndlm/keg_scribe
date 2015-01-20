/**

Copy the defines below into your own Config.ino

Config.ino is in .gitignore and shouldn't be added
to the repository.

/*

// Fill this with your Wifi Credentials
#define WLAN_SSID           "Mai Wifu"           // cannot be longer than 32 characters!
#define WLAN_PASS           "Mai Wifu Password"
#define WLAN_SECURITY       WLAN_SEC_WPA2

#define USE_HARD_CODED_IP_PRIMARY    true
#define HARD_CODED_IP_PRIMARY        172, 16, 25, 63
#define USE_HARD_CODED_IP_SECONDARY  false
#define HARD_CODED_IP_SECONDARY      10, 0, 0, 1

// change this to match your SD shield or module;
//     Arduino Ethernet shield: pin 4
//     Adafruit SD shields and modules: pin 10
//     Sparkfun SD shield: pin 8
#define SD_CHIP_SELECT_PIN  4

// Webserver Address
#define PRIMARY_SERVER             "server1.webserver.com"
#define SECONDARY_SERVER           "server2.webserver.com"

// key to pass for HTTP Basic Authorization
#define BASIC_AUTH_KEY        "somehashedkeygoeshere"

// NTP Server Address
#define NTP_SERVER          "pool.ntp.org"

// Path on the Server to report to
#define CSV_WEBPAGE         "/import/simpleCSV"
#define JSON_WEBPAGE        "/import/simpleJSON"

#define IDLE_TIMEOUT_MS     3000      // Amount of time to wait (in milliseconds) with no data 
                                      // received before closing the connection.  If you know the server
                                      // you're accessing is quick to respond, you can reduce this value.                               

*/
