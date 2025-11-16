#include "SimpleNetManager.h"

namespace SimpleNet {

/**
 * @brief Constructor (Basic): Delegates to the full constructor with CS pin 10 and no debug stream.
 */
SimpleNetManager::SimpleNetManager(byte mac[])
    : SimpleNetManager(mac, 10, nullptr) { // Default CS pin is 10
}

/**
 * @brief Constructor (MAC + CS Pin): Delegates to the full constructor with no debug stream.
 */
SimpleNetManager::SimpleNetManager(byte mac[], uint8_t csPin)
    : SimpleNetManager(mac, csPin, nullptr) {
}

/**
 * @brief Constructor (MAC + Debug): Delegates to the full constructor with default CS pin 10.
 */
SimpleNetManager::SimpleNetManager(byte mac[], Stream* debugStream)
    : SimpleNetManager(mac, 10, debugStream) { // Default CS pin is 10
}

/**
 * @brief Constructor (Full): This is the main constructor that does all the setup work.
 */
SimpleNetManager::SimpleNetManager(byte mac[], uint8_t csPin, Stream* debugStream) {
    memcpy(_mac, mac, 6);
    _csPin = csPin;
    _debugStream = debugStream;
    
    _currentState = NET_DISCONNECTED;
    _use_static_ip = false;
    _lastConnectionAttempt = 0;
    _onConnectCallback = nullptr;
    _onDisconnectCallback = nullptr;
}

/**
 * @brief Initializes for DHCP.
 */
void SimpleNetManager::begin() {
    _use_static_ip = false;

    // Always initialize the Ethernet CS pin based on the constructor used.
    Ethernet.init(_csPin);
    if (_debugStream) {
        _debugStream->print(F("[NetManager] Using CS pin: "));
        _debugStream->println(_csPin);
    }
    
    _lastConnectionAttempt = millis() - _connectionInterval;
    if (_debugStream) {
        _debugStream->println(F("[NetManager] Initialized for DHCP."));
    }
}

/**
 * @brief Initializes for Static IP.
 */
void SimpleNetManager::begin(IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet) {
    _use_static_ip = true;
    _ip = ip;
    _dns = dns;
    _gateway = gateway;
    _subnet = subnet;

    // Always initialize the Ethernet CS pin based on the constructor used.
    Ethernet.init(_csPin);
    if (_debugStream) {
        _debugStream->print(F("[NetManager] Using CS pin: "));
        _debugStream->println(_csPin);
    }
    
    _lastConnectionAttempt = millis() - _connectionInterval;
    if (_debugStream) {
        _debugStream->println(F("[NetManager] Initialized for Static IP."));
    }
}

/**
 * @brief The main state machine loop to be called repeatedly.
 */
NetState SimpleNetManager::loop() {
    NetState previousState = _currentState;

    switch (_currentState) {
        case NET_DISCONNECTED:
            if (millis() - _lastConnectionAttempt >= _connectionInterval) {
                _currentState = NET_CONNECTING;
                connect();
            }
            break;

        case NET_CONNECTING:
            // This state is transient and handled within the connect() method.
            break;

        case NET_CONNECTED:
            uint8_t maintain_status = Ethernet.maintain();
            if (maintain_status == 1 || maintain_status == 3) {
                if (_debugStream) _debugStream->println(F("[NetManager] DHCP lease lost."));
                _currentState = NET_DISCONNECTED;
            }

            if (Ethernet.linkStatus() != LinkON) {
                if (_debugStream) _debugStream->println(F("[NetManager] Physical link lost."));
                _currentState = NET_DISCONNECTED;
            }
            break;
    }

    if (_currentState != previousState) {
        if (_currentState == NET_CONNECTED) {
            if (_onConnectCallback) {
                _onConnectCallback();
            }
        } else if (_currentState == NET_DISCONNECTED && previousState == NET_CONNECTED) {
            if (_onDisconnectCallback) {
                _onDisconnectCallback();
            }
            _lastConnectionAttempt = millis();
        }
    }

    return _currentState;
}

/**
 * @brief Private method to handle the actual connection attempt.
 */
void SimpleNetManager::connect() {
    _lastConnectionAttempt = millis();
    if (_debugStream) {
        _debugStream->print(F("[NetManager] Attempting connection... Mode: "));
        _debugStream->println(_use_static_ip ? "Static" : "DHCP");
    }

    bool success = false;
    if (_use_static_ip) {
        Ethernet.begin(_mac, _ip, _dns, _gateway, _subnet);
        // For static, success is assumed until link status check fails in loop().
        // A short delay might be needed for link to establish, which the non-blocking
        // loop naturally handles. We tentatively move to CONNECTED.
        if (Ethernet.linkStatus() == LinkON) {
             _currentState = NET_CONNECTED;
        } else {
            // If link is not on, we go back to disconnected to retry.
             _currentState = NET_DISCONNECTED;
        }
    } else { // DHCP
        if (Ethernet.begin(_mac) == 1) {
            success = true;
        }
    }
    
    if (!_use_static_ip) {
        if (success && Ethernet.localIP() != IPAddress(0,0,0,0)) {
            _currentState = NET_CONNECTED;
            if (_debugStream) {
                _debugStream->print(F("[NetManager] DHCP connection successful. IP: "));
                _debugStream->println(Ethernet.localIP());
            }
        } else {
            _currentState = NET_DISCONNECTED;
            if (_debugStream) _debugStream->println(F("[NetManager] DHCP connection failed."));
        }
    }
}


/**
 * @brief Returns true if the current state is CONNECTED.
 */
bool SimpleNetManager::isConnected() {
    return _currentState == NET_CONNECTED;
}

/**
 * @brief Returns a reference to the internal EthernetClient object.
 */
EthernetClient& SimpleNetManager::getClient() {
    return _client;
}

/**
 * @brief Sets the connection retry interval.
 */
void SimpleNetManager::setConnectionRetryInterval(long interval) {
    _connectionInterval = interval;
}

/**
 * @brief Registers the onConnect callback function.
 */
void SimpleNetManager::onConnect(void (*callback)()) {
    _onConnectCallback = callback;
}

/**
 * @brief Registers the onDisconnect callback function.
 */
void SimpleNetManager::onDisconnect(void (*callback)()) {
    _onDisconnectCallback = callback;
}

} // namespace SimpleNet