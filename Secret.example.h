
// ##### VARIABLES NEEDED FOR OTA #####

const int NETWORKS_COUNT = 1;

// Network(s) setup
// You can add more than one if you plan
// to use device in multiple networks
const char* SSIDs[NETWORKS_COUNT] = {
  "SSID1"
  // "SSID2"
  // "SSID3"
};
const char* passwords[NETWORKS_COUNT] = {
  "password1",
  // "password2"
  // "password3"
};

// OTA upgrade page basic auth credentials
const char* authUser = "admin";
const char* authPass = "password";
