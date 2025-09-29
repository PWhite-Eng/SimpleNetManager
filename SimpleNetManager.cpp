#include "SimpleNetManager.h"

// All library code is placed within the SimpleNet namespace
namespace SimpleNet {

/**
 * @brief Constructor: Initializes member variables.
 */
SimpleNetManager::SimpleNetManager(byte mac[], Stream* debugStream) {
    memcpy(_mac, mac, 6);
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
    // Set last attempt time to ensure an immediate connection attempt on the first call to loop().
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
    _lastConnectionAttempt = millis() - _connectionInterval;
    if (_debugStream) {
        _debugStream->println(F("[NetManager] Initialized for Static IP."));
    }
}

/**
 * @brief The main state machine loop to be called repeatedly.
 */
NetState SimpleNetManager::loop() {
    // Keep a record of the state before this loop iteration
    NetState previousState = _currentState;

    switch (_currentState) {
        case NET_DISCONNECTED:
            // It's time to try connecting again.
            if (millis() - _lastConnectionAttempt >= _connectionInterval) {
                _currentState = NET_CONNECTING;
                connect(); // Attempt to connect
            }
            break;

        case NET_CONNECTING:
            // This state is now transient. The connect() method will either succeed,
            // fail, or transition to NET_AWAITING_LINK for static IPs. This case
            // is kept for logical clarity but should not persist across loops.
            break;

        case NET_CONNECTED:
            // For DHCP, we must maintain the lease. For Static, this does no harm.
            // A return of 1 (renew fail) or 3 (rebind fail) indicates a lease failure.
            // A return of 0 means it wasn't time to renew yet.
            uint8_t maintain_status = Ethernet.maintain();
            if (maintain_status == 1 || maintain_status == 3) {
                // DHCP lease failed, we are effectively disconnected.
                if (_debugStream) _debugStream->println(F("[NetManager] DHCP lease lost."));
                _currentState = NET_DISCONNECTED;
            }

            // Check if the physical link is still active.
            if (Ethernet.linkStatus() != LinkON) {
                if (_debugStream) _debugStream->println(F("[NetManager] Physical link lost."));
                _currentState = NET_DISCONNECTED;
            }
            break;
    }

    // --- Callback Execution Logic ---
    // Check if a state change occurred and trigger the appropriate callback.
    if (_currentState != previousState) {
        if (_currentState == NET_CONNECTED) {
            // We have just connected.
            if (_onConnectCallback) {
                _onConnectCallback();
            }
        } else if (_currentState == NET_DISCONNECTED && previousState == NET_CONNECTED) {
            // We have just disconnected from a previously connected state.
            if (_onDisconnectCallback) {
                _onDisconnectCallback();
            }
            // Reset timer to start reconnection attempts immediately on the next valid loop.
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
        // We can't check link status immediately. Instead, we wait non-blockingly.
        // The check and transition to NET_CONNECTED will happen in the loop() after a short delay.
        // For now, we assume it's "connecting" until the link is verified.
        // This is a placeholder; real state change happens in the main loop.

    } else {
        // For DHCP, begin() returns 1 on success (got a lease), 0 on failure.
        if (Ethernet.begin(_mac) == 1) {
            success = true;
        }
    }
    
    // For DHCP, we can check success immediately.
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