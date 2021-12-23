# ESP32 BACnet Server Example

In this project we are using the [CAS BACnet stack](https://www.bacnetstack.com/) to generate a simple BACnet server with one Multi-state-value (MSV) object. The MSV shows the current mode of the built-in LED. A BACnet client (such as the [CAS BACnet Explorer](https://store.chipkin.com/products/tools/cas-bacnet-explorer)) can be used to write to this MSV to change the mode.

The MSV allows for three possaible values to be written.

- 1 = **Off**
- 2 = **On**
- 3 = **Blink** - Blinks the LED on and off at a rate of 500 ms.

## Quick start

1. Download and install [Platform/io](https://platformio.org/) for [Visual studios code](https://code.visualstudio.com/)
2. Add the ESP32 board to Platform/io
3. Move the [CAS BACnet stack](https://www.bacnetstack.com/) source and adapter files to the */lib/src/* folder:
```
From:
|--lib
|  |
|  |--cas-bacnet-stack
|  |  |--adapters
|  |  |  |--cpp
|  |  |  |  |- *.cpp
|  |  |  |  |- *.h
|  |  |--source
|  |  |  |- *.cpp
|  |  |  |- *.h
|  |  |--submodules
|  |  |  |--cas-common
|  |  |  |  |--source
|  |  |  |  |  |- *.cpp
|  |  |  |  |  |- *.c
|--src
|  |- main.c

To:
|--lib
|  |
|  |--cas-bacnet-stack
|  |  |--src
|  |  |  |- *.cpp
|  |  |  |- *.h
|--src
|  |- main.c
```
4. Use Platform/io to *Build* the project.
5. Use Platform/io to *Upload and Monitor*.

## Supported BIBBs

The CAS BACnet stack supports many other BIBBs, this minumial example only supports the following:

- **DS-RP-B**: Data Sharing - ReadProperty-B
- **DS-RPM-B**: Data Sharing - ReadPropertyMultiple-B
- **DS-WP-B**: Data Sharing - WriteProperty-B
- **DM-DDB-B**: Device and Network Management - Dynamic Device Binding-B (Who-Is)
- **DM-DOB-B**: Device and Network Management - Dynamic Object Binding-B (Who-Has)

## BACnet objects supported

The CAS BACnet stack supports many other object types this minumial example only supports the following:

- Device (8)
- Multi-state Value (19)

## Device Tree

Below is the device tree exported from the CAS BACnet Explorer.

```txt
device: 389001  (ESP32 BACnet Example Server)
    object_identifier: device (389001)
    object_type: device
    vendor_identifier: Chipkin Automation Systems (0x185)
    application_software_version: v1
    firmware_revision: 3.25.0.0
    model_name: CAS BACnet Stack
    object_name: ESP32 BACnet Example Server
    protocol_services_supported: readProperty (1), readPropertyMultiple (1), writeProperty (1), who_Has (1), who_Is (1), 
    protocol_version: 1
    vendor_name: Chipkin Automation Systems
    protocol_revision: 14
    multi_state_value: 1  (LED State)
        object_identifier: multi_state_value (1)
        object_type: multi_state_value (0x13)
        event_state: normal (0x0)
        number_of_states: 3
        object_name: LED State
        out_of_service: False
        present_value: 3
        status_flags: in_alarm (0), fault (0), overridden (0), out_of_service (0), 
        state_text: Off, On, Blink

```

## Example in the CAS BACnet Explorer

![Preview of the CAS BACnet Explorer](/docs/CASBACnetExplorer.png?raw=true "Preview of the CAS BACnet Explorer")

## Example output

The example output from the serial terminal of the ESP32

```txt
FYI: ESP32 CAS BACnet Stack example version: 0.0.1
FYI: ESP32 Chip ID: 30C6, (08A4AE30)
FYI: Connecting to wifi...
FYI: Connected to XXXXXXXXXX
FYI: IP address: 192.168.1.66
FYI: Connecting UDP resource to port=[47808]
FYI: Connected to UDP port.
FYI: CAS BACnet Stack version: 3.25.0
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
FYI: FreeHeap: 270784 / 336136 (80.59 %)
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

```

## Tested hardware

- [Adafruit HUZZAH32 – ESP32 Feather Board](https://www.adafruit.com/product/3405)

## FAQ

### Can't build the project. *CASBACnetStackAdapter.h: No such file or directory*

Please see this issue [CASBACnetStackAdapter.h: No such file or directory](https://github.com/chipkin/ESP32-BACnetServerExample/issues/1)
