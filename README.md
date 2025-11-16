# **SimpleNetManager for Arduino**

A simple, non-blocking, event-driven manager for Arduino Ethernet connections. This library provides a robust state machine to handle network connections, automatically managing reconnections for both DHCP and Static IP configurations.

It now features an event-driven design using callbacks, allowing your main sketch to react to network status changes without polling, keeping your code clean and efficient.

## **Key Features**

* **Fully Non-Blocking:** Manages network state without using `delay()`, even during static IP initialization.  
* **Event-Driven with Callbacks:** Register functions to run automatically when the network connects or disconnects.  
* **Automatic Reconnection:** If the network connection is lost, the manager will periodically attempt to reconnect.  
* **DHCP & Static IP:** Easily configure the network connection using either method.  
* **Customizable Chip Select (CS) Pin:** Specify the CS pin for your Ethernet hardware (defaults to pin 10).
* **Configurable & Debuggable:** Adjust settings like the connection retry interval and enable serial debugging output.

## **Installation**

1. **Arduino Library Manager:**  
   * Open the Arduino IDE.  
   * Go to `Sketch` \> `Include Library` \> `Manage Libraries...`.  
   * Search for "SimpleNetManager" and click `Install`.
2. **Install from .ZIP:**  
   * Download this repository as a .ZIP file.  
   * In the Arduino IDE, go to `Sketch` \> `Include Library` \> `Add .ZIP Library...`.  
   * Select the downloaded ZIP file.

## **Basic Usage**

The library is now event-driven. You can register callback functions in `setup()` that will automatically execute when the network status changes. The `netManager.loop()` function must still be called on every iteration of your sketch's main `loop()`.
```cpp
#include <SPI.h>
#include <Ethernet.h>
#include "SimpleNetManager.h"

// The library is now in a namespace to prevent naming conflicts.
using namespace SimpleNet;

// Define the MAC address for your Ethernet shield.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Define the Chip Select (CS) pin if you are not using the default (pin 10).
// const uint8_t ETHERNET_CS_PIN = 4;

// --- Instantiation ---
// Create an instance of the network manager. Choose one of the following options.

// Option 1: Basic (MAC only, CS pin defaults to 10)
// SimpleNetManager netManager(mac);

// Option 2: MAC and a custom CS pin
// SimpleNetManager netManager(mac, ETHERNET_CS_PIN);

// Option 3: MAC and debug output (CS pin defaults to 10))
SimpleNetManager netManager(mac, &Serial);

// Option 4: MAC, custom CS pin, and debug output 
// SimpleNetManager netManager(mac, ETHERNET_CS_PIN, &Serial);


// --- Callback Functions ---
// This function will be called automatically when the network connects.
void onNetworkConnect() {
  Serial.println("\n--- Network Connected! ---");
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());
}

// This function will be called automatically when the network disconnects.
void onNetworkDisconnect() {
  Serial.println("\n--- Network Disconnected! ---");
  Serial.println("Attempting to reconnect...");
}

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  // Register our callback functions
  netManager.onConnect(onNetworkConnect);
  netManager.onDisconnect(onNetworkDisconnect);

  // Initialize using DHCP. For static IP, use the other begin() method.
  netManager.begin();
}

void loop() {
  // The most important line: this runs the manager's state machine
  // and triggers callbacks when the state changes.
  netManager.loop();

  // Your main application logic can now run independently.
  if (netManager.isConnected()) {
    // --- Your networking code goes here ---
    // This part will only run when connected.
  }
}
```
## **API Reference**

All components are within the `SimpleNet` namespace.

### **Constructors**

The library provides three flexible ways to create a `SimpleNetManager` object.

1. `SimpleNetManager(byte mac[])`

    - The most basic constructor.

    - `mac[]`: A 6-byte array for the MAC address.

    - The Chip Select (CS) pin will default to **pin 10**.

1. `SimpleNetManager(byte mac[], uint8_t csPin)`

    - Constructor to specify a custom CS pin.

    - `mac[]`: A 6-byte array for the MAC address.

    - `csPin`: The pin to use for the Ethernet module's Chip Select.

1. `SimpleNetManager(byte mac[], Stream* debugStream)`

    - Constructor to enable debug output.

    - `mac[]`: A 6-byte array for the MAC address.

    - `debugStream`: A pointer to a Stream object (like &Serial) to print debug output.

1. `SimpleNetManager(byte mac[], uint8_t csPin, Stream* debugStream)`

    - The full constructor.

    - `mac[]`: A 6-byte array for the MAC address.

    - `csPin`: The pin to use for the Ethernet module's Chip Select.

    - `debugStream`: A pointer to a Stream object (like &Serial) to print debug output.

`void begin()` & `void begin(...)`

Initializes the manager for DHCP or Static IP. This must be called in `setup()`.

`NetState loop()`

The main work function; must be called in your sketch's `loop()`. It runs the state machine and triggers callbacks.

* **Returns**: The current `NetState`('NET_DISCONNECTED', 'NET_CONNECTING', or 'NET_CONNECTED').

'bool isConnected()'

Checks if the network is fully connected.

* **Returns**: `true` if in the `NET_CONNECTED` state, otherwise `false`.

`EthernetClient& getClient()`

Provides access to the underlying `EthernetClient` object, which is needed to make TCP connections.

`void setConnectionRetryInterval(long interval)`

Sets the time in milliseconds to wait between reconnection attempts. The default is 10,000ms.

`void onConnect(void (\*callback)())`

Registers a function to be called once when the network connection is established.

`void onDisconnect(void (\*callback)())`

Registers a function to be called once when the network connection is lost.

## Acknowledgments

This library's event-driven approach was inspired by the design patterns found in the [Arduino_ConnectionHandler](https://github.com/arduino-libraries/Arduino_ConnectionHandler) library.

This project was developed with the assistance of Google's Gemini.

## **License**

This project is licensed under the MIT License \- see the LICENSE file for details.
