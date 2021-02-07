This set of software includes basic examples of communicating with each Morningstar device using MODBUS RTU (serial connections).  Please see the Libmodbus Documentation (http://www.libmodbus.org/documentation/) for more information on using the libmodbus commands.  There are also examples of setting up a TCP context.  It looks simple, but I don't have a TriStar-MPPT-60 to test it on.

Each example reads the first five RAM registers of each device and prints out the results.  These are the "Hello, World" programs for communicating with these devices.  If you can make these work, you should be able to read and write to the device using MODBUS.

Please note, I only have a SunSaver MPPT and a SureSine-300.  The respective examples have been tested with my devices.  The others all compile, but have not been tested with their respective devices.  They are my best guess as to where to start.  Good luck and enjoy!
