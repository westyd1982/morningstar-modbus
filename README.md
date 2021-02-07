# morningstar-modbus
Linux (and Mac OS X) Software to Read Data from Morningstar Products that Support the MODBUS Protocol

It is relatively easy to write software to read data from a Morningstar product that supports the MODBUS protocol. I have tested the software below with two SunSaver MPPTs and a SureSine-300 using their MODBUS connections, a MeterHub, and a Meterbus to Serial Converter (MSC). The programming principles in the code examples should also work with the SunSaver Duo, the TriStar PWM, the TriStar MPPT, the ProStar PWM, the ProStar MPPT, and the Relay Driver.

Example Hardware Required:

Morningstar SunSaver MPPT with Switch 4 ON (MODBUS Protocol)
Morningstar Meterbus to Serial Converter (MSC)
Computer running Linux or Mac OS X
Serial port (either built-in or via USB-Serial adaptor)

Software Required:

Software development environment: Linux – sudo install build-essential (sudo apt install build-essential), Mac OS X – install X Code
Modbus library for Linux and OSX – libmodbus (http://www.libmodbus.org/) - Linux - sudo apt install libmodbus-dev pkg-config

I have an off-grid solar power system with two Morningstar SunSaver MPPTs connected seperately to a Kyocera KD205GX-LP 205 watt photovoltaic (PV) panel and a Grape Solar GS-S-250-Fab5 250 watt PV panel. My system also has a Morningstar SureSine-300 Inverter for AC loads. When I installed the system originally, I built a serial port-attached digital voltmeter to track the charge status of the batteries. This device was attached to a Linksys NSLU2 (powered by the batteries) running Debian Lenny. While this setup worked fine, the only information it could provide was the voltage of the batteries. I wanted to be able to also track the array voltage, charging current, load voltage, and load current. The SunSaver MPPT provides this information via its MODBUS connection, so I bought a Morningstar Meterbus to Serial Converter (MSC) and started looking for examples of Linux software to communicate with the SunSaver MPPT. There was not much out there. The only information I could find was someone successfully using the MODBUS connection with Linux. At least it could be done. I also found libmodbus – a MODBUS library for Linux and Mac OS X. It compiles and works well on both my Mac and my Linux computers and has some example software that helped point me in the right direction. Later, I replaced my Linksys NSLU2 with a much faster Sheevaplug running Debian Squeeze. Currently, my system is using a Raspberry Pi 4 running the latest Raspberry Pi OS Lite (a variant of Debian).
