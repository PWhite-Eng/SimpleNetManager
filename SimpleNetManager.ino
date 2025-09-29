#include <SPI.h>
#include <Ethernet.h>
#include "SimpleNetManager.h"

// The library is now in a namespace to prevent naming conflicts.
using namespace SimpleNet;

//-----------------------------------------------------
// Network Configuration & Objects
//-----------------------------------------------------
// Define an override for the Chip Select (CS) pin if you aren't using the default (10).
const uint8_t ETHERNET_CS_PIN = 4;

// MAC address for the Ethernet shield.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// The server we want to connect to.
const char server[] = "example.com";

// --- Instantiation Options ---
// Choose ONE of the following three ways to create the network manager object.

// OPTION 1: Basic (MAC address only, CS pin defaults to 10)
// SimpleNetManager netManager(mac);

// OPTION 2: MAC address and custom CS pin (overrides the default)
// SimpleNetManager netManager(mac, ETHERNET_CS_PIN);

// OPTION 3: MAC, custom CS pin, and debug output (Recommended for development)
SimpleNetManager netManager(mac, ETHERNET_CS_PIN, &Serial);

//-----------------------------------------------------
// Timing for periodic HTTP request
//-----------------------------------------------------
unsigned long previousRequestMillis = 0;
const long requestInterval = 15000; // Make a request every 15 seconds

//-----------------------------------------------------
// Function Prototypes
//-----------------------------------------------------
void makeHttpRequest();
void onNetworkConnect();
void onNetworkDisconnect();

//-----------------------------------------------------
// SETUP
//-----------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) { ; } // Wait for serial port to connect.

  Serial.println("--- SimpleNetManager Full Feature Example ---");
  Serial.println("Demonstrating callbacks, debugging, and custom retry interval.");

  // DEMO 2: Set a custom reconnection interval.
  // Here we'll try to reconnect every 5 seconds instead of the 10-second default.
  netManager.setConnectionRetryInterval(5000);

  // DEMO 3: Register callback functions (OPTIONAL).
  // Callbacks allow you to react to network events automatically.
  // If you don't need this, you can comment out or delete the next two lines
  // and just use netManager.isConnected() in your main loop to check the status.
  netManager.onConnect(onNetworkConnect);
  netManager.onDisconnect(onNetworkDisconnect);

  // Initialize the Ethernet manager using DHCP.
  // To use a static IP, comment the line below and uncomment the static block.
  netManager.begin();

  /*
  // --- Static IP Configuration Example ---
  IPAddress ip(192, 168, 1, 177);
  IPAddress dns(8, 8, 8, 8);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  netManager.begin(ip, dns, gateway, subnet);
  */

  Serial.println("\nSetup complete. The manager will now handle the connection.");
}

//-----------------------------------------------------
// LOOP
//-----------------------------------------------------
void loop() {
  // The most important line: call the manager's loop on every iteration.
  // This runs the internal state machine and triggers our callbacks when events occur.
  netManager.loop();

  // The main application logic can run independently. Our onNetworkConnect() and
  // onNetworkDisconnect() callbacks now handle connection status changes automatically.

  // We can still use isConnected() for tasks that should only run while connected,
  // like making periodic requests.
  if (netManager.isConnected()) {

    // Check if it's time to make another HTTP request
    if (millis() - previousRequestMillis >= requestInterval) {
      previousRequestMillis = millis(); // Save the last time we made a request
      makeHttpRequest();
    }
  }
}

//-----------------------------------------------------
// Function Definitions
//-----------------------------------------------------

/**
 * @brief Makes a simple HTTP GET request.
 * This is called by the onNetworkConnect callback and then periodically by the main loop.
 */
void makeHttpRequest() {
  // To make a connection, get the underlying EthernetClient from the manager.
  // This demonstrates the getClient() function.
  EthernetClient& client = netManager.getClient();

  Serial.println("\n---------------------------------");
  Serial.print("Making HTTP request to: ");
  Serial.println(server);

  if (client.connect(server, 80)) {
    Serial.println("=> Connected to server. Sending GET request.");
    client.println("GET / HTTP/1.1");
    client.println("Host: example.com");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("=> Connection to server failed.");
    Serial.println("---------------------------------");
    return;
  }

  // Reading the response from the server.
  unsigned long timeout = millis();
  while (client.connected() && (millis() - timeout < 2000)) {
    if (client.available()) {
      char c = client.read();
      Serial.write(c);
      timeout = millis();
    }
  }

  if (!client.connected()) {
    Serial.println();
    Serial.println("=> Disconnected from server.");
    client.stop();
  }
  Serial.println("---------------------------------");
}

//-----------------------------------------------------
// Callback Functions (Only used if registered in setup)
//-----------------------------------------------------

/**
 * @brief This function is called AUTOMATICALLY by the library when a network connection is established.
 */
void onNetworkConnect() {
  Serial.println("\n>>> EVENT: Network Connected! <<<");
  Serial.print("    IP Address: ");
  Serial.println(Ethernet.localIP());
  // Let's make a request immediately upon connecting.
  makeHttpRequest();
}

/**
 * @brief This function is called AUTOMATICALLY by the library when the network connection is lost.
 */
void onNetworkDisconnect() {
  Serial.println("\n>>> EVENT: Network Disconnected! <<<");
  Serial.println("    The library will now attempt to reconnect automatically.");
}