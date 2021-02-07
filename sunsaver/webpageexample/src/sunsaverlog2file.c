/*
 *  sunsaverlog2file.c - This program reads all the log registers on a Moringstar SunSaver MPPT and writes the results to a logfile.
 *
 *	Use this program when you are setting up the data logging and web page creators to populate the daily log file with the data
 *	currently stored in the SunSaver MPPT.  It should put the past 32 days worth of data into the daily log file.
 *  
 *	Note: The Morningstar SunSaver MPPT creates a new log entry when the charge state switches to night.  This program reads the
 *		  charge state from RAM to create the proper date stamps.
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


/* Compile with: cc `pkg-config --cflags --libs libmodbus` sunsaverlog2file.c -o sunsaverlog2file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <modbus.h>

#include "powersystem.h"

#define DONTINCLUDETODAY	1		/* If you are planning on beginning to run dailylog tonight using the cron file, it will add today's daily log record  */
									/* to the log file, so you shouldn't add it with this utility or you will have a duplicate record after dailylog runs. */

int main(void)
{
	FILE *outfile;
	modbus_t *ctx;
	int i, j, k, n, low, rc, hour, today, indx[32];
	unsigned int hm, hourmeter[32], alarm_daily[32];
	float sunsaver_Ic, Vb_min_daily[32], Vb_max_daily[32], Ahc_daily[32], Ahl_daily[32], Va_max_daily[32];
	unsigned short array_fault_daily[32], load_fault_daily[32], time_ab_daily[32], time_eq_daily[32], time_fl_daily[32];
	unsigned short charge_state;
	unsigned short data[50];
	time_t lclTime, logTime;
	struct tm *now,*logthen;
	char tsdate[32], tstime[3], filepath[64], logfilename[64];
	
	/* Get current local time */
	lclTime = time(NULL);
	now = localtime(&lclTime);
	
	// Create time stamps for the file name
	strcpy(filepath,"");
	sprintf(filepath,"%s/%%Y/%%Ydailylog.txt",LOGFILEPATH);
	strftime(logfilename, 64, filepath, now);
		
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
	
	/* Read the RAM Registers */
	rc = modbus_read_registers(ctx, 0x0008, 45, data);
	if (rc == -1) {
		fprintf(stderr, "%s\n", modbus_strerror(errno));
		return -1;
	}
	
	/* Determine if the last daily log record is from today or yesterday */
	strftime(tstime, 3, "%H", now);
	hour=atoi(tstime);
	sunsaver_Ic=data[3]*79.16/32768.0;
	charge_state=data[9];
	if ((charge_state == 3) && (hour > 12) && (sunsaver_Ic < 0.001)) {		/* If the charge state is night, the time is past noon, 
																			 and the array current is < 0.001, then the newest log record is from today */
		today=1;
	}
	else {											/* Else the newest record is from yesterday */
		today=0;
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
    modbus_free(ctx);
	
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
	if ((outfile = fopen(logfilename, "w")) == NULL) { 
		printf("Can't create log file.\n");
		exit(1);
	}
	
#if (DONTINCLUDETODAY)
	for (i=0; i<n-today; i++) {
#else
	for (i=0; i<n; i++) {
#endif
		logTime=lclTime-(n-i-today)*(24*60*60);
		logthen = localtime(&logTime);
		strftime(tsdate, 32, "%m/%d/%Y", logthen);
		fprintf(outfile,"%s\t%u\t%u\t%.2f\t%.2f", tsdate, hourmeter[indx[i]],alarm_daily[indx[i]],Vb_min_daily[indx[i]],Vb_max_daily[indx[i]]);
		fprintf(outfile,"\t%.2f\t%.2f\t%d\t%d", Ahc_daily[indx[i]],Ahl_daily[indx[i]],array_fault_daily[indx[i]],load_fault_daily[indx[i]]);
		fprintf(outfile,"\t%.2f\t%d\t%d\t%d\n", Va_max_daily[indx[i]],time_ab_daily[indx[i]],time_eq_daily[indx[i]],time_fl_daily[indx[i]]);
	}
	
	fclose(outfile);

	return(0);
}

