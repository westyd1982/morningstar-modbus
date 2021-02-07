/*
 *  powersystem.h - Common header file for MODBUS addresses, file paths, and file names.
 *  
 
 Copyright 2014 Tom Rinehart.
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see http://www.gnu.org/licenses/.
 *
 */

/*	Set the MODBUS address for the device(s) on your system.  If you have more than one of the same device, cut and paste the device
	line and add a numeral after the defined variable (e.g. I have two SunSaver MPPTs on my system, so I have SUNSAVERMPPT1 and
	SUNSAVERMPPT2 in my header file with different MODBUS addresses).  For multiple devices, you will also need to modify the code files
	to do something useful with multiple devices. */

#define SUNSAVERMPPT	0x01									/* MODBUS Address of the SunSaver MPPT */
#define SUNSAVERDUO		0x01									/* MODBUS Address of the SunSaver Duo */
#define TRISTARPWM		0x01									/* MODBUS Address of the TriStar PWM */
#define SURESINE		0x01									/* MODBUS Address of the SureSine-300 */


/*	Battery voltage settings for the daily graph - This changes the scale for graphing the voltage. */

#define	VOLTAGE12		1.0
#define	VOLTAGE24		2.0
#define	VOLTAGE48		4.0
#define	VOLTAGESCALE	VOLTAGE12								/* This sets the y-axis scaling for the voltage graph. Change this to match your battery voltage */


/*	Power settings for the daily graph - This changes the scale for graphing power.  The power on the daily graph is scaled using
	this power scaling factor and the voltage scaling factor to show the maximum power of the device.  As an example, on a SunSaver
	MPPT with a 12 volt battery, the maximum current is 15 amps and the maximum voltage is 15 volts, so the daily graph needs to
	be able to show at least 225 Watts.  With the settings of VOLTAGE12 and POWER15, the daily graph will have a maximum of 230 Watts
	for the top of the graph.  To see the full power range of a SureSine-300, you should use the settings of VOLTAGE12 and POWER45.
	With these settings, the top of the graph will be 690 Watts.  */

#define POWER15			2.0										/* y-axis scaling factor for the SunSaver MPPT */
#define POWER30			1.0										/* y-axis scaling factor for the TriStar 30 or SunSaver Duo */
#define POWER45			(2.0/3.0)								/* y-axis scaling factor for the TriStar 45 */
#define POWER60			0.5										/* y-axis scaling factor for the TriStar 60 */
#define	POWERSCALE		POWER15									/* This sets the y-axis scaling for the power graph. Change this to match your charge controller */


/*	Device and file path settings */

#define SERIALPORTPATH	"/dev/ttyUSB0"							/* Path to appropriate serial port - typically /dev/ttyS0 for physical port or /dev/ttyUSB0 for USB-serial cable */

#define LOGFILEPATH		"/home/tom/test/powersystem/log"		/* Path to directory to store log files - you need to create this directory
																	You also need to create subdirectories with the year number (e.g, 2014, 2015, 2016, ...),
																	since the log files are stored here. */

#define WEBPAGEFILEPATH	"/home/tom/test/powersystem/www"		/* Path to directory to store web page files - you need to create this 
																	directory and configure the web server to serve this directory.
																	You also need to create subdirectories with the year number (e.g, 2014, 2015, 2016, ...),
																	since the daily graphs and daily log files are stored here. */

#define MAINWEBPAGENAME	"index.html"							/* File name of the main power system status web page */
