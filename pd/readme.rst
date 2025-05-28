`osc_net` Pd library 
====================

Devices with firmware that uses the `OSC_networking` library can be configured and controlled via Puredata [Pd] patches.
The configuration patch `osc_net_configuration.pd` provides automatic discovery of the devices in the local network, automatic connection and monitoring of the IoTs.

As an `osc_net` abstraction library, it is intended for the creation of patches to control OSC_networking devices.

Concept
----------

Boards with the same firmware can be distinguished from others by a base name and are addressed by their ID. This ID does not have to be the device number and can be configured.
This means that a series of boards can be addressed and queried via identification numbers (`ID`).
For each device, the IP and the port for controlling the device are queried and made available so that it is addressed directly and not broadcast or multicast.
The devices regularly report information via an `/info` message, which is received on the broadcast port.

Each device can send to the same receiver (host), so that on the receiver side there is an object for receiving from all boards via the same OSC port, whereby the devices are differentiated in the OSC address.

Each card can be configured with an ID number, called ‘board ID’.
This ID of the board must be added to the `base name` with the OSC `/`, such as ``/networker/7`` and prefixed to every message sent and received.

Received messages with the correct board ID can be forwarded to the board recipients using the ``osc_net/parse_device`` object.
To send messages, they can either be sent to all devices or directly to the known IP address via a ``osc_net/osc_send`` object.

Pd objects are therefore indexed with a `ID`, let's call it control-ID.
Each object with the same control ID receives the board ID via ``r /osc/<id>/id`` and the base name via ``r /osc/<id>/base_address``, so that a board can be assigned to a single control ID.

See examples for more information.

Installation
............

To include this library in an application patch, the `osc_net` folder should be placed in the project and the path to the pd library folder should be declared in Pd, for example with a `declare` object: ``[declare -path libs/OSC_networking/pd]``.

.. [Pd] Pure data, see http://puredata.info/

:author: Winfried Ritsch, Atelier Algorythmics
:version: see library version
:license: GPL V3
:URL: http://git.iem.at/uC/OSC_networking