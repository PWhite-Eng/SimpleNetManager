#ifndef SIMPLE_NET_MANAGER_H
#define SIMPLE_NET_MANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

/**
 * @brief A namespace to contain all SimpleNetManager library components
 * and prevent potential naming conflicts with other libraries.
 */
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
     * @brief Constructor for the network manager.
     * @param mac A byte array of length 6 containing the MAC address for the device.
     * @param debugStream An optional pointer to a Stream object (like Serial) for debug output.
     */
    SimpleNetManager(byte mac[], Stream* debugStream = nullptr);

    /**
     * @brief Initializes the manager to use DHCP for obtaining an IP address.
     */
    void begin();

    /**
     * @brief Initializes the manager to use a static IP configuration.
     * @param ip The static IP address to assign to the device.
     * @param dns The DNS server address.
     * @param gateway The network gateway address.
     * @param subnet The subnet mask.
     */
    void begin(IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet);

    /**
     * @brief The main loop function that manages the connection state.
     * @details This MUST be called on every iteration of the sketch's main loop() function.
     * @return The current connection state (NetState).
     */
    NetState loop();

    /**
     * @brief Checks if the device is fully connected to the network.
     * @return true if the state is NET_CONNECTED, otherwise false.
     */
    bool isConnected();

    /**
     * @brief Provides access to the underlying EthernetClient object.
     * @details This is essential for making HTTP requests or establishing other TCP connections.
     * @return A reference to the internal EthernetClient instance.
     */
    EthernetClient& getClient();

    /**
     * @brief Sets the time interval between connection retry attempts.
     * @param interval The time in milliseconds to wait between retries. Default is 10000.
     */
    void setConnectionRetryInterval(long interval);

    /**
     * @brief Registers a callback function to be executed when a network connection is established.
     * @param callback A pointer to a void function with no arguments, e.g., `void myConnectHandler()`.
     */
    void onConnect(void (*callback)());

    /**
     * @brief Registers a callback function to be executed when the network connection is lost.
     * @param callback A pointer to a void function with no arguments, e.g., `void myDisconnectHandler()`.
     */
    void onDisconnect(void (*callback)());

private:
    // Internal state and logic
    byte          _mac[6];
    IPAddress     _ip;
    IPAddress     _dns;
    IPAddress     _gateway;
    IPAddress     _subnet;
    bool          _use_static_ip;

    NetState      _currentState;
    unsigned long _lastConnectionAttempt;
    long          _connectionInterval = 10000; // Default 10 seconds

    EthernetClient _client;
    Stream* _debugStream;

    // Pointers to the callback functions
    void (*_onConnectCallback)();
    void (*_onDisconnectCallback)();

    // Private method to handle the connection logic
    void connect();
};

} // namespace SimpleNet

#endif // SIMPLE_NET_MANAGER_H