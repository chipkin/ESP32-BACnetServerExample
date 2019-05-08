# ESP32-BACnetServerExample

In this project we are using the CAS BACnet stack (https://www.bacnetstack.com/) to generate a simple BACnet server with one Multi-state-value (MSV) object. The MSV shows the current mode of the built-in LED. A BACnet client (such as the [CAS BACnet Explorer](https://store.chipkin.com/products/tools/cas-bacnet-explorer)) can be used to write to this MSV to change the mode.

In this example the MSV has three possible values

- 1 = Off
- 2 = On
- 3 = Blink

![Preview of the CAS BACnet Explorer](/docs/CASBACnetExplorer.png?raw=true "Preview of the CAS BACnet Explorer")

## Example output

The example output from the serial terminal of the ESP32

```txt
FYI: ESP32 CAS BACnet Stack example version: 0.0.1
FYI: ESP32 Chip ID: 30C6, (08A4AE30)
FYI: Connecting to wifi...
..
FYI: Connected to XXXXXXXXXX
IP address: 192.168.1.66
FYI: Connecting UDP resource to port=[47808]
FYI: Connected to UDP port.
FYI: CAS BACnet Stack version: 3.8.1
FYI: BACnet device created: Device instanse=389001
FYI: Enabled WriteProperty for Device 389001
FYI: Enabled Read Property Multiple for Device 14
FYI: Added multi-state-output (1) to Device (389001)
FYI: CallbackGetPropertyUInt deviceInstance=389001, objectType=8, objectInstance=389001, propertyIdentifier=62
FYI: CallbackGetPropertyUInt deviceInstance=389001, objectType=8, objectInstance=389001, propertyIdentifier=120
FYI: Sent message with 25 bytes to 192.168.1.255:47808
FYI: Sent broadcast IAm message
FYI: FreeHeap: 270784 / 336136 (80.59 %)
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
FYI: Recived message with 19 bytes from 192.168.1.77:47808
FYI: CallbackGetPropertyUInt deviceInstance=389001, objectType=19, objectInstance=1, propertyIdentifier=74
FYI: CallbackGetPropertyCharString deviceInstance=389001, objectType=19, objectInstance=1, propertyIdentifier=77
FYI: CallbackGetPropertyUInt deviceInstance=389001, objectType=19, objectInstance=1, propertyIdentifier=85
FYI: CallbackGetPropertyUInt deviceInstance=389001, objectType=19, objectInstance=1, propertyIdentifier=110
FYI: CallbackGetPropertyCharString deviceInstance=389001, objectType=19, objectInstance=1, propertyIdentifier=110
FYI: CallbackGetPropertyCharString deviceInstance=389001, objectType=19, objectInstance=1, propertyIdentifier=110
FYI: CallbackGetPropertyCharString deviceInstance=389001, objectType=19, objectInstance=1, propertyIdentifier=110
FYI: Sent message with 98 bytes to 192.168.1.77:47808
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

```

## Tested hardware

- [Adafruit HUZZAH32 – ESP32 Feather Board](https://www.adafruit.com/product/3405)
