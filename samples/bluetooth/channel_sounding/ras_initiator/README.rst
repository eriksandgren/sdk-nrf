.. _channel_sounding_ras_initiator:

Bluetooth: Channel Sounding Initiator with Ranging Requestor
############################################################

.. contents::
   :local:
   :depth: 2

This sample demonstrates how to use the ranging service to request ranging data from a server.
Distance estimates are then computed from the ranging data and logged to the terminal.

Requirements
************

The sample supports the following development kits:

.. table-from-sample-yaml::

The sample also requires a device running a Channel Sounding Reflector with Ranging Responder to connect to, such as the :ref:`channel_sounding_ras_reflector` sample.

Overview
********

The sample demonstrates a basic Bluetooth® Low Energy Central role functionality that acts as a GATT Ranging Requestor client and configures the Channel Sounding initiator role.
Regular Channel Sounding procedures are set up, local subevent data is stored, and peer ranging data is fetched.

User interface
**************

The sample does not require user input and will scan for a device advertising with the GATT Ranging Service UUID.
The first LED on the development kit will be lit when a connection has been established.

Configuration
*************

|config|

Configuration options
=====================

The following Kconfig options are specific to the Channel Sounding RAS Initiator sample:

.. _RAS_INITIATOR_STEP_MODE:

RAS_INITIATOR_STEP_MODE
   This option selects the Channel Sounding step modes to be used in the RAS initiator sample.
   Depending on the selected option, the sample will estimate the distance using Phase Based
   Ranging (PBR) and or Round Trip Time (RTT).
   The available options are:
   -:kconfig:option:`CONFIG_RAS_INITIATOR_STEP_MODE_2_SUB_MODE_1` Main: 2, Sub: 1
     Mode 2 steps are used for PBR and mode 1 steps are used for RTT. This is the default option.
   -:kconfig:option:`CONFIG_RAS_INITIATOR_STEP_MODE_1` Main: 1, Sub: unused
     Mode 1 steps are used for RTT.
   -:kconfig:option:`CONFIG_RAS_INITIATOR_STEP_MODE_2` Main: 2, Sub: unused
     Mode 2 steps are used for PBR.
   -:kconfig:option:`CONFIG_RAS_INITIATOR_STEP_MODE_3` Main: 3, Sub: unused
     Mode 3 steps are used for PBR and RTT.

Building and running
********************
.. |sample path| replace:: :file:`samples/bluetooth/channel_sounding/ras_initiator`

.. include:: /includes/build_and_run.txt

Testing
=======

After programming the sample to your development kit, you can test it by connecting to another device programmed with a Channel Sounding Reflector role with Ranging Responder, such as the :ref:`channel_sounding_ras_reflector` sample.

1. |connect_terminal_specific|
#. Reset both kits.
#. Wait until the scanner detects the Peripheral.
   In the terminal window, check for information similar to the following::

      I: Starting Channel Sounding Initiator Sample
      I: SoftDevice Controller build revision:
      I: bd 8a c7 d2 9b 7c 24 05 |.....|$.
      I: d3 20 24 a2 60 b9 62 44 |. $.`.bD
      I: 1a dc cc 22             |..."
      I: HW Platform: Nordic Semiconductor (0x0002)
      I: HW Variant: nRF54Lx (0x0005)
      I: Firmware: Standard Bluetooth controller (0x00) Version 189.51082 Build 612146130
      I: Identity: XX:XX:XX:XX:XX:XX (random)
      I: HCI: version 6.0 (0x0e) revision 0x30d5, manufacturer 0x0059
      I: LMP: version 6.0 (0x0e) subver 0x30d5
      I: Filters matched. Address: XX:XX:XX:XX:XX:XX (random) connectable: 1
      I: Connecting
      I: Connected to XX:XX:XX:XX:XX:XX (random) (err 0x00)
      I: Security changed: XX:XX:XX:XX:XX:XX (random) level 2
      I: MTU exchange success (498)
      I: The discovery procedure succeeded
      I: CS capability exchange completed.
      I: CS config creation complete. ID: 0
      I: CS security enabled.
      I: CS procedures enabled:
       - config ID: 0
       - antenna configuration index: 0
       - TX power: 0 dbm
       - subevent length: 28198 us
       - subevents per event: 1
       - subevent interval: 0
       - event interval: 2
       - procedure interval: 10
       - procedure count: 0
       - maximum procedure length: 1000
      I: Distance estimates on antenna path 0: ifft: 1.039173, phase_slope: 1.581897, rtt: 3.075647
      I: Sleeping for a few seconds...

Dependencies
************

This sample uses the following |NCS| libraries:

* :ref:`dk_buttons_and_leds_readme`
* :file:`include/bluetooth/gatt_dm.h`
* :file:`include/bluetooth/services/ras.h`

This sample uses the following Zephyr libraries:

* :file:`include/sys/printk.h`
* :file:`include/zephyr/types.h`
* :ref:`zephyr:kernel_api`:

  * :file:`include/kernel.h`

* :ref:`zephyr:bluetooth_api`:

* :file:`include/bluetooth/bluetooth.h`
* :file:`include/bluetooth/conn.h`
* :file:`include/bluetooth/cs.h`
