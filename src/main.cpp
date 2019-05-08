/**
 * ESP32 BACnet Server Example 
 * --------------------------------------
 * In this project we are using the CAS BACnet stack (https://www.bacnetstack.com/) to generate a 
 * simple BACnet server with one Multi-state-value (MSV) object. The MSV shows the current mode of 
 * the build in LED. A BACnet client (such as the CAS BACnet Explorer) can be used to write to this 
 * MSV to change the mode. 
 * 
 * In this example the MSV has three possible values 
 * - 1 = Off
 * - 2 = On
 * - 3 = Blink
 * 
 * Created by: Steven Smethurst 
 * Created on: May 8, 2019 
 * Last updated: May 8, 2019 
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Missing file
// This file is part of the CAS BACnet stack and is not included in this repo
// More information about the CAS BACnet stack can be found here https://www.bacnetstack.com/
// https://github.com/chipkin/ESP32-BACnetServerExample/issues/1
#include <CASBACnetStackAdapter.h>

// Application Version
// -----------------------------
const uint32_t APPLICATION_VERSION_MAJOR = 0;
const uint32_t APPLICATION_VERSION_MINOR = 0;
const uint32_t APPLICATION_VERSION_PATCH = 1;

// Application settings
// -----------------------------
// ToDo: Update these prameters with your local wifi SSID and password.
const char* APPLICATION_WIFI_SSID = "Chipkin";
const char* APPLICATION_WIFI_PASSWORD = "revenues vital config four";

const uint32_t APPLICATION_BACNET_DEVICE_INSTANCE = 389001;
const uint32_t APPLICATION_BACNET_OBJECT_MSV_LED_INSTANCE = 1;
const char* APPLICATION_BACNET_OBJECT_DEVICE_OBJECT_NAME = "ESP32 BACnet Example Server";
const char* APPLICATION_BACNET_OBJECT_MSV_LED_OBJECT_NAME = "LED State";
const char* APPLICATION_BACNET_OBJECT_MSV_LED_STATE_TEXT[] = { "Off", "On", "Blink" };
const uint16_t APPLICATION_BACNET_UDP_PORT = 47808;
const uint32_t APPLICATION_LED_PIN = LED_BUILTIN;
const uint32_t APPLICATION_SERIAL_BAUD_RATE = 115200;
const long APPLICATION_LED_BLINK_RATE_MS = 500; // interval at which to blink (milliseconds)

// BACnet constants
// -----------------------------
// This is a sub list of BACnet constants. A full list can be found in the documentation
const uint16_t BACNET_SERVICE_READ_PROPERTY_MULTIPLE = 14;
const uint16_t BACNET_SERVICE_WRITE_PROPERTY = 15;
const uint16_t BACNET_OBJECT_TYPE_DEVICE = 8;
const uint16_t BACNET_OBJECT_TYPE_MULTI_STATE_VALUE = 19;
const uint16_t BACNET_NETWORK_TYPE_IP = 0;
const uint32_t BACNET_PROPERTY_IDENTIFIER_NUMBER_OF_STATES = 74;
const uint32_t BACNET_PROPERTY_IDENTIFIER_OBJECT_NAME = 77;
const uint32_t BACNET_PROPERTY_IDENTIFIER_PRESENT_VALUE = 85;
const uint32_t BACNET_PROPERTY_IDENTIFIER_STATE_TEXT = 110;
const uint32_t BACNET_ERROR_CODE_VALUE_OUT_OF_RANGE = 37;

// Wifi
// create UDP instance
WiFiUDP gUDP;

// LED
// -----------------------------
// LED Mode
const uint16_t LED_MODE_OFF = 1;
const uint16_t LED_MODE_ON = 2;
const uint16_t LED_MODE_BLINK = 3;
uint32_t gLEDMode = LED_MODE_BLINK;
const uint16_t LED_MODE_STATE_COUNT = LED_MODE_BLINK;

// LED State
uint16_t gLEDState = LOW;

// Callback functions
// -----------------------------
uint16_t CallbackReceiveMessage(uint8_t* message, const uint16_t maxMessageLength, uint8_t* receivedConnectionString, const uint8_t maxConnectionStringLength, uint8_t* receivedConnectionStringLength, uint8_t* networkType);
uint16_t CallbackSendMessage(const uint8_t* message, const uint16_t messageLength, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, bool broadcast);
time_t CallbackGetSystemTime();
bool CallbackGetPropertyCharString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, char* value, uint32_t* valueElementCount, const uint32_t maxElementCount, uint8_t* encodingType, const bool useArrayIndex, const uint32_t propertyArrayIndex);
bool CallbackGetPropertyUInt(uint32_t deviceInstance, uint16_t objectType, uint32_t objectInstance, uint32_t propertyIdentifier, uint32_t* value, bool useArrayIndex, uint32_t propertyArrayIndex);
bool CallbackSetPropertyUInt(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode);

// Helpers
// -----------------------------
bool GetBroadcastAddress(uint8_t* broadcastAddress, size_t maxbroadcastAddressSize);

void setup()
{
    // Hardware setup
    // ==========================================
    // Set the digital pin as output:
    pinMode(APPLICATION_LED_PIN, OUTPUT);
    Serial.begin(APPLICATION_SERIAL_BAUD_RATE);
    delay(3000); // Let the Serial port connect, and give us some time to reprogram if needed.

    Serial.print("FYI: ESP32 CAS BACnet Stack example version: ");
    Serial.print(APPLICATION_VERSION_MAJOR);
    Serial.print(".");
    Serial.print(APPLICATION_VERSION_MINOR);
    Serial.print(".");
    Serial.println(APPLICATION_VERSION_PATCH);
    uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
    Serial.printf("FYI: ESP32 Chip ID: %04X, (%08X)\n", (uint16_t)(chipid >> 32), (uint32_t)chipid);

    // WiFi connection
    // ==========================================
    Serial.println("FYI: Connecting to wifi...");
    WiFi.begin(APPLICATION_WIFI_SSID, APPLICATION_WIFI_PASSWORD);

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("FYI: Connected to ");
    Serial.println(APPLICATION_WIFI_SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Set up UDP port.
    // Connect the UDP resource to the BACnet Port
    Serial.printf("FYI: Connecting UDP resource to port=[%u]\n", APPLICATION_BACNET_UDP_PORT);
    gUDP.begin(APPLICATION_BACNET_UDP_PORT);
    Serial.println("FYI: Connected to UDP port.");

    // Set up the CAS BACnet stack.
    // ==========================================
    LoadBACnetFunctions();
    Serial.print("FYI: CAS BACnet Stack version: ");
    Serial.print(fpGetAPIMajorVersion());
    Serial.print(".");
    Serial.print(fpGetAPIMinorVersion());
    Serial.print(".");
    Serial.println(fpGetAPIPatchVersion());
    Serial.print(".");
    Serial.println(fpGetAPIBuildVersion());

    // Set up CallBack functions
    // ------------------------------------------
    // There are many call back functions that we could implement. These are the minimum required for the demo.

    // Message Callback Functions
    fpRegisterCallbackReceiveMessage(CallbackReceiveMessage);
    fpRegisterCallbackSendMessage(CallbackSendMessage);

    // System Time Callback Functions
    fpRegisterCallbackGetSystemTime(CallbackGetSystemTime);

    // Get Property Callback Functions
    fpRegisterCallbackGetPropertyCharacterString(CallbackGetPropertyCharString);
    fpRegisterCallbackGetPropertyUnsignedInteger(CallbackGetPropertyUInt);

    // Set Property Callback Functions
    fpRegisterCallbackSetPropertyUnsignedInteger(CallbackSetPropertyUInt);

    // Set up the BACnet device
    // ------------------------------------------
    if (!fpAddDevice(APPLICATION_BACNET_DEVICE_INSTANCE)) {
        Serial.printf("Error: Could not add device. Device instanse=%u\n", APPLICATION_BACNET_DEVICE_INSTANCE);
        return;
    }
    Serial.printf("FYI: BACnet device created: Device instanse=%u\n", APPLICATION_BACNET_DEVICE_INSTANCE);

    // By default the write service is not enabled. We have to enable it to allow users to write to points.
    if (!fpSetServiceEnabled(APPLICATION_BACNET_DEVICE_INSTANCE, BACNET_SERVICE_WRITE_PROPERTY, true)) {
        Serial.printf("Error: Failed to enabled the WriteProperty service=[%u] for Device %u\n", BACNET_SERVICE_WRITE_PROPERTY, APPLICATION_BACNET_DEVICE_INSTANCE);
        return;
    }
    Serial.printf("FYI: Enabled WriteProperty for Device %u\n", APPLICATION_BACNET_DEVICE_INSTANCE);
    if (!fpSetServiceEnabled(APPLICATION_BACNET_DEVICE_INSTANCE, BACNET_SERVICE_READ_PROPERTY_MULTIPLE, true)) {
        Serial.printf("Error: Failed to enabled the WriteProperty service=[%u] for Device %u\n", BACNET_SERVICE_READ_PROPERTY_MULTIPLE, APPLICATION_BACNET_DEVICE_INSTANCE);
        return;
    }
    Serial.printf("FYI: Enabled Read Property Multiple for Device %u\n", BACNET_SERVICE_READ_PROPERTY_MULTIPLE);

    // Add Objects
    if (!fpAddObject(APPLICATION_BACNET_DEVICE_INSTANCE, BACNET_OBJECT_TYPE_MULTI_STATE_VALUE, APPLICATION_BACNET_OBJECT_MSV_LED_INSTANCE)) {
        Serial.printf("Error: Failed to add multi-state-output (%u) to Device (%u)\n", APPLICATION_BACNET_OBJECT_MSV_LED_INSTANCE, APPLICATION_BACNET_DEVICE_INSTANCE);
        return;
    }
    fpSetPropertyWritable(APPLICATION_BACNET_DEVICE_INSTANCE, BACNET_OBJECT_TYPE_MULTI_STATE_VALUE, APPLICATION_BACNET_OBJECT_MSV_LED_INSTANCE, BACNET_PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
    fpSetPropertyEnabled(APPLICATION_BACNET_DEVICE_INSTANCE, BACNET_OBJECT_TYPE_MULTI_STATE_VALUE, APPLICATION_BACNET_OBJECT_MSV_LED_INSTANCE, BACNET_PROPERTY_IDENTIFIER_STATE_TEXT, true);

    Serial.printf("FYI: Added multi-state-output (%u) to Device (%u)\n", APPLICATION_BACNET_OBJECT_MSV_LED_INSTANCE, APPLICATION_BACNET_DEVICE_INSTANCE);

    // Send a IAm Message to announse to the network that a new BACnet device has started.
    uint8_t connectionString[6];
    if (!GetBroadcastAddress(connectionString, 6)) {
        Serial.println("Error: Could not get the broadcast IP address");
        return;
    }
    connectionString[4] = APPLICATION_BACNET_UDP_PORT / 256;
    connectionString[5] = APPLICATION_BACNET_UDP_PORT % 256;
    if (!fpSendIAm(APPLICATION_BACNET_DEVICE_INSTANCE, connectionString, 6, BACNET_NETWORK_TYPE_IP, true, 65535, NULL, 0)) {
        Serial.printf("Error: Unable to send IAm for Device %u", APPLICATION_BACNET_DEVICE_INSTANCE);
        return;
    }
    Serial.println("FYI: Sent broadcast IAm message");
}

void loop()
{
    fpLoop();
    unsigned long currentMillis = millis();

    static unsigned long lastMemoryCheck = 0;
    if (lastMemoryCheck < currentMillis) {
        lastMemoryCheck = currentMillis + 30000;
        Serial.print("FYI: FreeHeap: ");
        Serial.print(ESP.getFreeHeap());
        Serial.print(" / ");
        Serial.print(ESP.getHeapSize());
        Serial.print(" (");
        Serial.print(((float)ESP.getFreeHeap() / (float)ESP.getHeapSize()) * 100.0f);
        Serial.println(" %)");
    }

    switch (gLEDMode) {
        default:
        case LED_MODE_OFF:
            if (gLEDState != LOW) {
                gLEDState = LOW;
                digitalWrite(APPLICATION_LED_PIN, gLEDState);
            }
            break;
        case LED_MODE_ON:
            if (gLEDState != HIGH) {
                gLEDState = HIGH;
                digitalWrite(APPLICATION_LED_PIN, gLEDState);
            }
            break;
        case LED_MODE_BLINK:
            // check to see if it's time to blink the LED; that is, if the difference
            // between the current time and last time you blinked the LED is bigger than
            // the interval at which you want to blink the LED.
            static long lastBlink = 0;
            if (currentMillis - lastBlink >= APPLICATION_LED_BLINK_RATE_MS) {
                // save the last time you blinked the LED
                lastBlink = currentMillis;

                // if the LED is off turn it on and vice-versa:
                if (gLEDState == LOW) {
                    gLEDState = HIGH;
                    Serial.print("+");
                } else {
                    gLEDState = LOW;
                    Serial.print("-");
                }

                // set the LED with the ledState of the variable:
                digitalWrite(APPLICATION_LED_PIN, gLEDState);
            }
            break;
    }
}

// Helper
// ---------------------------------------------------------------------------
bool GetBroadcastAddress(uint8_t* broadcastAddress, size_t maxbroadcastAddressSize)
{
    if (broadcastAddress == NULL || maxbroadcastAddressSize < 4) {
        return false; // Not enugh space.
    }
    IPAddress localIP = WiFi.localIP();
    IPAddress subnetMask = WiFi.subnetMask();

    broadcastAddress[0] = (localIP[0] & subnetMask[0]) == 0 ? 255 : localIP[0];
    broadcastAddress[1] = (localIP[1] & subnetMask[1]) == 0 ? 255 : localIP[1];
    broadcastAddress[2] = (localIP[2] & subnetMask[2]) == 0 ? 255 : localIP[2];
    broadcastAddress[3] = (localIP[3] & subnetMask[3]) == 0 ? 255 : localIP[3];
    return true;
}

// Callback used by the BACnet Stack to check if there is a message to process
// ---------------------------------------------------------------------------

uint16_t CallbackReceiveMessage(uint8_t* message, const uint16_t maxMessageLength, uint8_t* receivedConnectionString, const uint8_t maxConnectionStringLength, uint8_t* receivedConnectionStringLength, uint8_t* networkType)
{
    // Check parameters
    if (message == NULL || maxMessageLength == 0) {
        Serial.println("Error: Invalid input buffer");
        return 0;
    }
    if (receivedConnectionString == NULL || maxConnectionStringLength == 0) {
        Serial.println("Error: Invalid connection string buffer");
        return 0;
    }
    if (maxConnectionStringLength < 6) {
        Serial.println("Error: Not enough space for a UDP connection string");
        return 0;
    }

    // processing incoming packet, must be called before reading the buffer
    gUDP.parsePacket();

    int32_t bytesRead = gUDP.read(message, maxMessageLength);
    if (bytesRead <= 0) {
        return 0;
    }

    // We got a message.
    IPAddress remoteIP = gUDP.remoteIP();
    receivedConnectionString[0] = remoteIP[0];
    receivedConnectionString[1] = remoteIP[1];
    receivedConnectionString[2] = remoteIP[2];
    receivedConnectionString[3] = remoteIP[3];

    uint16_t remotePort = gUDP.remotePort();
    receivedConnectionString[4] = remotePort / 256;
    receivedConnectionString[5] = remotePort % 256;

    *receivedConnectionStringLength = 6;
    *networkType = BACNET_NETWORK_TYPE_IP;

    Serial.printf("FYI: Recived message with %u bytes from %u.%u.%u.%u:%u\n", bytesRead, receivedConnectionString[0], receivedConnectionString[1], receivedConnectionString[2], receivedConnectionString[3], remotePort);
    // Serial.printf("Message first 10 bytes: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n", message[0], message[1], message[2], message[3], message[4], message[5], message[6], message[7], message[8], message[9]);
    return bytesRead;
}
uint16_t CallbackSendMessage(const uint8_t* message, const uint16_t messageLength, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, bool broadcast)
{
    if (message == NULL || messageLength == 0) {
        Serial.println("FYI: Nothing to send");
        return 0;
    }
    if (connectionString == NULL || connectionStringLength == 0) {
        Serial.println("FYI: No connection string");
        return 0;
    }

    // Verify Network Type
    if (networkType != BACNET_NETWORK_TYPE_IP) {
        Serial.println("FYI: Message for different network");
        return 0;
    }

    // Prepare the IP Address
    char destiationIPAddressAsString[32];
    if (broadcast) {
        uint8_t broadcastIPAddress[4];
        if (!GetBroadcastAddress(broadcastIPAddress, 4)) {
            Serial.println("Error: Could not get the broadcast iIP address");
            return 0;
        }
        snprintf(destiationIPAddressAsString, 32, "%u.%u.%u.%u", broadcastIPAddress[0], broadcastIPAddress[1], broadcastIPAddress[2], broadcastIPAddress[3]);
    } else {
        snprintf(destiationIPAddressAsString, 32, "%u.%u.%u.%u", connectionString[0], connectionString[1], connectionString[2], connectionString[3]);
    }

    // Get the port
    uint16_t udpPort = 0;
    udpPort += connectionString[4] * 256;
    udpPort += connectionString[5];

    // Send
    gUDP.beginPacket(destiationIPAddressAsString, udpPort);
    gUDP.write(message, messageLength);
    gUDP.endPacket();

    Serial.printf("FYI: Sent message with %u bytes to %s:%u\n", messageLength, destiationIPAddressAsString, udpPort);
    // Serial.printf("Message first 10 bytes: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n", message[0], message[1], message[2], message[3], message[4], message[5], message[6], message[7], message[8], message[9]);
    return messageLength;
}
time_t CallbackGetSystemTime()
{
    return millis() / 1000;
}
bool CallbackGetPropertyCharString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, char* value, uint32_t* valueElementCount, const uint32_t maxElementCount, uint8_t* encodingType, const bool useArrayIndex, const uint32_t propertyArrayIndex)
{
    Serial.printf("FYI: CallbackGetPropertyCharString deviceInstance=%d, objectType=%d, objectInstance=%d, propertyIdentifier=%d\n", deviceInstance, objectType, objectInstance, propertyIdentifier);

    if (deviceInstance != APPLICATION_BACNET_DEVICE_INSTANCE) {
        return false;
    }

    if (objectType == BACNET_OBJECT_TYPE_MULTI_STATE_VALUE && objectInstance == APPLICATION_BACNET_OBJECT_MSV_LED_INSTANCE) {
        if (propertyIdentifier == BACNET_PROPERTY_IDENTIFIER_OBJECT_NAME && useArrayIndex == false) {
            size_t objectNameStringLength = strlen(APPLICATION_BACNET_OBJECT_MSV_LED_OBJECT_NAME);
            if (objectNameStringLength <= maxElementCount) {
                memcpy(value, APPLICATION_BACNET_OBJECT_MSV_LED_OBJECT_NAME, objectNameStringLength);
                *valueElementCount = objectNameStringLength;
                return true;
            }
        } else if (propertyIdentifier == BACNET_PROPERTY_IDENTIFIER_STATE_TEXT && useArrayIndex == true) {
            if (propertyArrayIndex > LED_MODE_STATE_COUNT || propertyArrayIndex <= 0) {
                return false; // Invalid range
            }

            size_t objectNameStringLength = strlen(APPLICATION_BACNET_OBJECT_MSV_LED_STATE_TEXT[propertyArrayIndex - 1]);
            if (objectNameStringLength <= maxElementCount) {
                memcpy(value, APPLICATION_BACNET_OBJECT_MSV_LED_STATE_TEXT[propertyArrayIndex - 1], objectNameStringLength);
                *valueElementCount = objectNameStringLength;
                return true;
            }
        }
    } else if (objectType == BACNET_OBJECT_TYPE_DEVICE && objectInstance == APPLICATION_BACNET_DEVICE_INSTANCE && propertyIdentifier == BACNET_PROPERTY_IDENTIFIER_OBJECT_NAME && useArrayIndex == false) {
        size_t objectNameStringLength = strlen(APPLICATION_BACNET_OBJECT_DEVICE_OBJECT_NAME);
        if (objectNameStringLength <= maxElementCount) {
            memcpy(value, APPLICATION_BACNET_OBJECT_DEVICE_OBJECT_NAME, objectNameStringLength);
            *valueElementCount = objectNameStringLength;
            return true;
        }
    }

    return false;
}

bool CallbackGetPropertyUInt(uint32_t deviceInstance, uint16_t objectType, uint32_t objectInstance, uint32_t propertyIdentifier, uint32_t* value, bool useArrayIndex, uint32_t propertyArrayIndex)
{
    Serial.printf("FYI: CallbackGetPropertyUInt deviceInstance=%d, objectType=%d, objectInstance=%d, propertyIdentifier=%d\n", deviceInstance, objectType, objectInstance, propertyIdentifier);

    if (deviceInstance == APPLICATION_BACNET_DEVICE_INSTANCE && objectType == BACNET_OBJECT_TYPE_MULTI_STATE_VALUE && objectInstance == APPLICATION_BACNET_OBJECT_MSV_LED_INSTANCE) {
        if (propertyIdentifier == BACNET_PROPERTY_IDENTIFIER_PRESENT_VALUE && useArrayIndex == false) {
            *value = gLEDMode;
            return true;
        } else if (propertyIdentifier == BACNET_PROPERTY_IDENTIFIER_NUMBER_OF_STATES && useArrayIndex == false) {
            *value = LED_MODE_STATE_COUNT;
            return true;
        } else if (propertyIdentifier == BACNET_PROPERTY_IDENTIFIER_STATE_TEXT && useArrayIndex == true && propertyArrayIndex == 0) {
            *value = LED_MODE_STATE_COUNT;
            return true;
        }
    }

    return false;
}
bool CallbackSetPropertyUInt(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode)
{
    Serial.printf("FYI: CallbackSetPropertyUInt deviceInstance=%d, objectType=%d, objectInstance=%d, propertyIdentifier=%d, value=%d\n", deviceInstance, objectType, objectInstance, propertyIdentifier, value);

    if (deviceInstance == APPLICATION_BACNET_DEVICE_INSTANCE && objectType == BACNET_OBJECT_TYPE_MULTI_STATE_VALUE && objectInstance == APPLICATION_BACNET_OBJECT_MSV_LED_INSTANCE && propertyIdentifier == BACNET_PROPERTY_IDENTIFIER_PRESENT_VALUE && useArrayIndex == false) {
        switch (value) {
            case LED_MODE_OFF:
            case LED_MODE_ON:
            case LED_MODE_BLINK:
                gLEDMode = value;
                return true;
            default:
                // Out of range
                *errorCode = BACNET_ERROR_CODE_VALUE_OUT_OF_RANGE;
                return false;
        }
    }

    return false;
}