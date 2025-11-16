#ifndef SIMPLE_NET_MANAGER_H
#define SIMPLE_NET_MANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

namespace SimpleNet {

/**
 * @brief Defines the possible network connection states.
 */
enum NetState {
    NET_DISCONNECTED, ///< The device is not connected to the network.
    NET_CONNECTING,   ///< A connection attempt is currently in progress.
    NET_CONNECTED     ///< The device has a stable network connection.
};

/**
 * @brief Manages an Arduino Ethernet connection in a non-blocking way.
 * @details This class handles the state machine for connecting, maintaining,
 * and reconnecting an Ethernet shield using either DHCP or a static IP.
 * It is designed to be called repeatedly in the main sketch loop.
 */
class SimpleNetManager {
public:
    /**
     * @brief Constructor (Basic): Initializes with only the MAC address. CS pin defaults to 10.
     * @param mac A byte array of length 6 for the MAC address.
     */
    SimpleNetManager(byte mac[]);

    /**
     * @brief Constructor (MAC + CS Pin): Initializes with MAC address and a custom Chip Select pin.
     * @param mac A byte array of length 6 for the MAC address.
     * @param csPin The chip select (CS) pin for the Ethernet module.
     */
    SimpleNetManager(byte mac[], uint8_t csPin);

    /**
     * @brief Constructor (MAC + Debug): Initializes with MAC address and a debug stream. CS pin defaults to 10.
     * @param mac A byte array of length 6 for the MAC address.
     * @param debugStream A pointer to a Stream object (like &Serial) for debug output.
     */
    SimpleNetManager(byte mac[], Stream* debugStream);

    /**
     * @brief Constructor (MAC + CS Pin + Debug): Initializes with MAC, CS pin, and a debug stream.
     * @param mac A byte array of length 6 for the MAC address.
     * @param csPin The chip select (CS) pin for the Ethernet module.
     * @param debugStream A pointer to a Stream object (like &Serial) for debug output.
     */
    SimpleNetManager(byte mac[], uint8_t csPin, Stream* debugStream);

    // --- Public Methods ---
    void begin();
    void begin(IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet);
    NetState loop();
    bool isConnected();
    EthernetClient& getClient();
    void setConnectionRetryInterval(long interval);
    void onConnect(void (*callback)());
    void onDisconnect(void (*callback)());

private:
    // --- Private Members ---
    byte          _mac[6];
    IPAddress     _ip;
    IPAddress     _dns;
    IPAddress     _gateway;
    IPAddress     _subnet;
    bool          _use_static_ip;
    uint8_t       _csPin;
    Stream* _debugStream;

    NetState      _currentState;
    unsigned long _lastConnectionAttempt;
    long          _connectionInterval = 10000;

    EthernetClient _client;
    
    void (*_onConnectCallback)();
    void (*_onDisconnectCallback)();

    void connect();
};

} // namespace SimpleNet

#endif // SIMPLE_NET_MANAGER_H