/*
 *  dailylog.c - This program reads all the log registers on a Morningstar SunSaver MPPT, stores the latest in dailylog.txt, and produces dailylog.html.
 *  
 *	Note: the Morningstar SunSaver MPPT creates a new log entry when the charge state switches to night.
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
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


/* Compile with: cc `pkg-config --cflags --libs libmodbus` dailylog.c -o dailylog
 
 Run this program once a day after the sun has set but before midnight using a cron file with these lines.  Store the file at /etc/cron.d/dailylog.
 
 # Update the SunSaver MPPT daily log and web page
 
 59 23 * * * root /home/tom/powersystem/bin/dailylog			# Path to executable file - you need to configure this appropriately in the cron file
 
 Revised to use a common header file for file paths on 3/22/2013.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <modbus.h>

#include "powersystem.h"

void writehtmlfile(char *logfilename, char *htmlfilename);

int main(void)
{
	FILE *outfile;
	modbus_t *ctx;
	int i, j, k, n, low, rc, indx[32];
	unsigned int hm, hourmeter[32], alarm_daily[32];
	float Vb_min_daily[32], Vb_max_daily[32], Ahc_daily[32], Ahl_daily[32], Va_max_daily[32];
	unsigned short array_fault_daily[32], load_fault_daily[32], time_ab_daily[32], time_eq_daily[32], time_fl_daily[32];
	unsigned short data[16];
	time_t lclTime;
	struct tm *now;
	char tsdate[32], filepath[64], logfilename[64], htmlfilename[64];
	
	/* Initialize index array with ascending values starting at 0 */
	for (i=0;i<32;i++) indx[i]=i;
	
	/* Get current local time */
	lclTime = time(NULL);
	now = localtime(&lclTime);
	
	// Create time stamps for log results and file names
	strftime(tsdate, 32, "%m/%d/%Y", now);
	strcpy(filepath,"");
	sprintf(filepath,"%s/%%Y/%%Ydailylog.txt",LOGFILEPATH);
	strftime(logfilename, 64, filepath, now);
	strcpy(filepath,"");
	sprintf(filepath,"%s/%%Y/%%Ydailylog.html",WEBPAGEFILEPATH);
	strftime(htmlfilename, 64, filepath, now);
	
	/* Set up a new MODBUS context */
	ctx = modbus_new_rtu(SERIALPORTPATH, 9600, 'N', 8, 2);
	if (ctx == NULL) {
		fprintf(stderr, "Unable to create the libmodbus context\n");
		return -1;
	}
	
	/* Set the slave id to the SunSaver MPPT MODBUS id */
	modbus_set_slave(ctx, SUNSAVERMPPT);
	
	/* Open the MODBUS connection to the SunSaver MPPT */
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed SUNSAVERMPPT: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
	
	usleep(2500);						// Give the charge controller time before requesting next set of log registers
	
	j=0;
	for(i=0x8000; i<0x81FF; i+=0x0010) {
		
		/* Read the log registers and convert the results to their proper values */
		rc = modbus_read_registers(ctx, i, 13, data);
		if (rc == -1) {
			fprintf(stderr, "modbus_read_registers: %s\n", modbus_strerror(errno));
			return -1;
		}
		
		hm=data[0] + ((data[1] & 0x00FF) << 16);
		if (hm != 0x000000 && hm != 0xFFFFFF) {
			hourmeter[j]=hm;
			
			alarm_daily[j]=(data[2] << 8) + (data[1] >> 8);
			
			Vb_min_daily[j]=data[3]*100.0/32768.0;
			
			Vb_max_daily[j]=data[4]*100.0/32768.0;
			
			Ahc_daily[j]=data[5]*0.1;
			
			Ahl_daily[j]=data[6]*0.1;
			
			array_fault_daily[j]=data[7];
			
			load_fault_daily[j]=data[8];
			
			Va_max_daily[j]=data[9]*100.0/32768.0;
			
			time_ab_daily[j]=data[10];
			
			time_eq_daily[j]=data[11];
			
			time_fl_daily[j]=data[12];
			
			j++;
		}
		usleep(2500);					// Give the charge controller time before requesting next set of log registers
	}
	
	/* Close the MODBUS connection */
    modbus_close(ctx);
	
	/* Order the data with lowest hourmeter value first */
	n=j;
	low=0;
	for (i=0; i<n-1; i++) {
		if (hourmeter[i] > hourmeter[i+1]) {		//Search for the lowest hourmeter value
			low=i+1;
			break;
		}
	}
	if (low > 0) {									// Create an index to order the data
		for (i=0, j=n-low; i<low; i++, j++) indx[j]=i;
		for (i=low, j=0; i<n; i++, j++) indx[j]=i;
	}
	
	// Write data to log file
	if ((outfile = fopen(logfilename, "a")) == NULL) { 
		printf("Can't create log file.\n");
		exit(1);
	}
	
	// Write the last log record to log file (today's data)
	i=n-1;
	fprintf(outfile,"%s\t%u\t%u\t%.2f\t%.2f", tsdate, hourmeter[indx[i]],alarm_daily[indx[i]],Vb_min_daily[indx[i]],Vb_max_daily[indx[i]]);
	fprintf(outfile,"\t%.2f\t%.2f\t%d\t%d", Ahc_daily[indx[i]],Ahl_daily[indx[i]],array_fault_daily[indx[i]],load_fault_daily[indx[i]]);
	fprintf(outfile,"\t%.2f\t%d\t%d\t%d\n", Va_max_daily[indx[i]],time_ab_daily[indx[i]],time_eq_daily[indx[i]],time_fl_daily[indx[i]]);
	
	fclose(outfile);
	
	writehtmlfile(logfilename, htmlfilename);
	
    modbus_free(ctx);
	
	return(0);
}

void writehtmlfile(char *logfilename, char *htmlfilename)
{
	FILE *htmlfile, *infile;
	int month, day, year;
	unsigned int hourmeter, alarm_daily;
	float Vb_min_daily, Vb_max_daily, Ahc_daily, Ahl_daily, Va_max_daily;
	int array_fault_daily, load_fault_daily, time_ab_daily, time_eq_daily, time_fl_daily;
	char inputline[1000] = "";
	
	// Write data to html file
	if ((htmlfile = fopen(htmlfilename, "w")) == NULL) { 
		printf("Can't create file: %s.\n", htmlfilename);
		exit(1);
	}
	
	fprintf(htmlfile,"<html>\n<head>\n\t<title>SunSaver MPPT Daily Log</title>\n</head>\n");
	fprintf(htmlfile,"<body bgcolor=\"#6699FF\" text=\"#000000\" link=\"#330099\" vlink=\"#336633\" alink=\"#FFCC00\">\n");
	fprintf(htmlfile,"<h3 style=\"font-family:Comic Sans MS;color:#663300\">SunSaver MPPT Daily Log</h3>\n");
	fprintf(htmlfile,"<table border=\"1\" style=\"font-family:arial;color:black;font-size:12px;background-color:white;text-align:center;\">\n");
	fprintf(htmlfile,"\t<tr>\n\t\t<th>Date</th>\n");
	fprintf(htmlfile,"\t\t<th>Hour Meter</th>\n");
	fprintf(htmlfile,"\t\t<th>Min Vb</th>\n");
	fprintf(htmlfile,"\t\t<th>Max Vb</th>\n");
	fprintf(htmlfile,"\t\t<th>Charge Ah</th>\n");
	fprintf(htmlfile,"\t\t<th>Load Ah</th>\n");
	fprintf(htmlfile,"\t\t<th>Max Va</th>\n");
	fprintf(htmlfile,"\t\t<th>Ab Time</th>\n");
	fprintf(htmlfile,"\t\t<th>Eq Time</th>\n");
	fprintf(htmlfile,"\t\t<th>Fl Time</th>\n");
	fprintf(htmlfile,"\t\t<th>Controller Alarms</th>\n");
	fprintf(htmlfile,"\t\t<th>Solar Input Faults</th>\n");
	fprintf(htmlfile,"\t\t<th>Load Output Faults</th>\n\t</tr>\n");
	infile = fopen(logfilename, "r");
	if(infile != NULL) {
		while (fscanf(infile, "%[^\n]\n", inputline) != EOF)
		{
			sscanf(inputline,"%2d%*c%2d%*c%4d%u%u%f%f%f%f%d%d%f%d%d%d",&month,&day,&year,&hourmeter,&alarm_daily,&Vb_min_daily,
				   &Vb_max_daily,&Ahc_daily,&Ahl_daily,&array_fault_daily,&load_fault_daily,&Va_max_daily,
				   &time_ab_daily,&time_eq_daily,&time_fl_daily);
			fprintf(htmlfile,"\t<tr>\n\t\t<td>%0d/%0d/%d</td>\n",month,day,year);
			fprintf(htmlfile,"\t\t<td>%d</td>\n",hourmeter);
			fprintf(htmlfile,"\t\t<td>%.2f</td>\n",Vb_min_daily);
			fprintf(htmlfile,"\t\t<td>%.2f</td>\n",Vb_max_daily);
			fprintf(htmlfile,"\t\t<td>%.2f</td>\n",Ahc_daily);
			fprintf(htmlfile,"\t\t<td>%.2f</td>\n",Ahl_daily);
			fprintf(htmlfile,"\t\t<td>%.2f</td>\n",Va_max_daily);
			fprintf(htmlfile,"\t\t<td>%d</td>\n",time_ab_daily);
			fprintf(htmlfile,"\t\t<td>%d</td>\n",time_eq_daily);
			fprintf(htmlfile,"\t\t<td>%d</td>\n",time_fl_daily);
			fprintf(htmlfile,"\t\t<td>");
			if (alarm_daily == 0) {
				fprintf(htmlfile,"No alarms</td>\n");
			} else {
				if (alarm_daily & 1) fprintf(htmlfile,"RTS open<br>");
				if ((alarm_daily & (1 << 1)) >> 1) fprintf(htmlfile,"RTS shorted<br>");
				if ((alarm_daily & (1 << 2)) >> 2) fprintf(htmlfile,"RTS disconnected<br>");
				if ((alarm_daily & (1 << 3)) >> 3) fprintf(htmlfile,"Ths open<br>");
				if ((alarm_daily & (1 << 4)) >> 4) fprintf(htmlfile,"Ths shorted<br>");
				if ((alarm_daily & (1 << 5)) >> 5) fprintf(htmlfile,"SSMPPT hot<br>");
				if ((alarm_daily & (1 << 6)) >> 6) fprintf(htmlfile,"Current limit<br>");
				if ((alarm_daily & (1 << 7)) >> 7) fprintf(htmlfile,"Current offset<br>");
				if ((alarm_daily & (1 << 8)) >> 8) fprintf(htmlfile,"Undefined<br>");
				if ((alarm_daily & (1 << 9)) >> 9) fprintf(htmlfile,"Undefined<br>");
				if ((alarm_daily & (1 << 10)) >> 10) fprintf(htmlfile,"Uncalibrated<br>");
				if ((alarm_daily & (1 << 11)) >> 11) fprintf(htmlfile,"RTS miswire<br>");
				if ((alarm_daily & (1 << 12)) >> 12) fprintf(htmlfile,"Undefined<br>");
				if ((alarm_daily & (1 << 13)) >> 13) fprintf(htmlfile,"Undefined<br>");
				if ((alarm_daily & (1 << 14)) >> 14) fprintf(htmlfile,"Miswire<br>");
				if ((alarm_daily & (1 << 15)) >> 15) fprintf(htmlfile,"FET open<br>");
				if ((alarm_daily & (1 << 16)) >> 16) fprintf(htmlfile,"P12<br>");
				if ((alarm_daily & (1 << 17)) >> 17) fprintf(htmlfile,"High Va current limit<br>");
				if ((alarm_daily & (1 << 18)) >> 18) fprintf(htmlfile,"Alarm 19<br>");
				if ((alarm_daily & (1 << 19)) >> 19) fprintf(htmlfile,"Alarm 20<br>");
				if ((alarm_daily & (1 << 20)) >> 20) fprintf(htmlfile,"Alarm 21<br>");
				if ((alarm_daily & (1 << 21)) >> 21) fprintf(htmlfile,"Alarm 22<br>");
				if ((alarm_daily & (1 << 22)) >> 22) fprintf(htmlfile,"Alarm 23<br>");
				if ((alarm_daily & (1 << 23)) >> 23) fprintf(htmlfile,"Alarm 24");
				fprintf(htmlfile,"</td>\n");
			}
			fprintf(htmlfile,"\t\t<td>");
			if (array_fault_daily == 0) {
				fprintf(htmlfile,"No faults</td>\n");
			} else {
				if (array_fault_daily & 1) fprintf(htmlfile,"Overcurrent<br>");
				if ((array_fault_daily & (1 << 1)) >> 1) fprintf(htmlfile,"FETs shorted<br>");
				if ((array_fault_daily & (1 << 2)) >> 2) fprintf(htmlfile,"Software bug<br>");
				if ((array_fault_daily & (1 << 3)) >> 3) fprintf(htmlfile,"Battery HVD<br>");
				if ((array_fault_daily & (1 << 4)) >> 4) fprintf(htmlfile,"Array HVD<br>");
				if ((array_fault_daily & (1 << 5)) >> 5) fprintf(htmlfile,"EEPROM setting edit (reset required)<br>");
				if ((array_fault_daily & (1 << 6)) >> 6) fprintf(htmlfile,"RTS shorted<br>");
				if ((array_fault_daily & (1 << 7)) >> 7) fprintf(htmlfile,"RTS was valid, now disconnected<br>");
				if ((array_fault_daily & (1 << 8)) >> 8) fprintf(htmlfile,"Local temperature sensor failed<br>");
				if ((array_fault_daily & (1 << 9)) >> 9) fprintf(htmlfile,"Fault 10<br>");
				if ((array_fault_daily & (1 << 10)) >> 10) fprintf(htmlfile,"Fault 11<br>");
				if ((array_fault_daily & (1 << 11)) >> 11) fprintf(htmlfile,"Fault 12<br>");
				if ((array_fault_daily & (1 << 12)) >> 12) fprintf(htmlfile,"Fault 13<br>");
				if ((array_fault_daily & (1 << 13)) >> 13) fprintf(htmlfile,"Fault 14<br>");
				if ((array_fault_daily & (1 << 14)) >> 14) fprintf(htmlfile,"Fault 15<br>");
				if ((array_fault_daily & (1 << 15)) >> 15) fprintf(htmlfile,"Fault 16");
				fprintf(htmlfile,"</td>\n");
			}
			fprintf(htmlfile,"\t\t<td>");
			if (load_fault_daily == 0) {
				fprintf(htmlfile,"No faults</td>\n\t</tr>\n");
			} else {
				if (load_fault_daily & 1) fprintf(htmlfile,"External short circuit\n");
				if ((load_fault_daily & (1 << 1)) >> 1) fprintf(htmlfile,"Overcurrent<br>");
				if ((load_fault_daily & (1 << 2)) >> 2) fprintf(htmlfile,"FETs shorted<br>");
				if ((load_fault_daily & (1 << 3)) >> 3) fprintf(htmlfile,"Software bug<br>");
				if ((load_fault_daily & (1 << 4)) >> 4) fprintf(htmlfile,"HVD<br>");
				if ((load_fault_daily & (1 << 5)) >> 5) fprintf(htmlfile,"Heatsink over-temperature<br>");
				if ((load_fault_daily & (1 << 6)) >> 6) fprintf(htmlfile,"EEPROM setting edit (reset required)<br>");
				if ((load_fault_daily & (1 << 7)) >> 7) fprintf(htmlfile,"Fault 8<br>");
				fprintf(htmlfile,"</td>\n\t</tr>\n");
			}
		}
	}
	fclose(infile);
	
	fprintf(htmlfile,"</table>\n");
 	fprintf(htmlfile,"</body>\n</html>\n");
	
	fclose(htmlfile);
}

