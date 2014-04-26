/**

Copy the defines below into your own Config.ino

Config.ino is in .gitignore and shouldn't be added
to the repository.

/*

// Add it and put the following defines in it:
#define WLAN_SSID       "Mai Wifu"           // cannot be longer than 32 characters!
#define WLAN_PASS       "Mai Wifu Password"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

// Comma, separated, hard coded IP of the Hakase Server
#define USE_HARD_CODED_IP   true
#define HARD_CODED_IP       172, 16, 25, 63

// When not using a hard coded IP, use this address:
#define WEBSITE      "my.hakase.server.com"

// Use this path to report data on the Hakase server
#define WEBPAGE      "/import/simpleJSON"

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

*/
