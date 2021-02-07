/*
 *  sunsaverlog.c - This program reads all the log registers from a Moringstar SunSaver MPPT, sorts the records by hourmeter, and prints the results.
 *  
 *	Note: the Morningstar SunSaver MPPT creates a new log entry when the charge state switches to night and the charging current drops to zero.
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


/* Compile with: cc `pkg-config --cflags --libs libmodbus` sunsaverlog.c -o sunsaverlog */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <modbus.h>

#include "powersystem.h"

int main(void)
{
	modbus_t *ctx;
	int i, j, k, n, low, rc, hour, today, indx[32];
	unsigned int hm, hourmeter[32], alarm_daily[32];
	float sunsaver_Ic, Vb_min_daily[32], Vb_max_daily[32], Ahc_daily[32], Ahl_daily[32], Va_max_daily[32];
	unsigned short array_fault_daily[32], load_fault_daily[32], time_ab_daily[32], time_eq_daily[32], time_fl_daily[32];
	unsigned short charge_state;
	unsigned short data[50];
	time_t lclTime, logTime;
	struct tm *now,*logthen;
	char tsdate[32], tshour[3];
	
	/* Get current local time */
	lclTime = time(NULL);
	now = localtime(&lclTime);
	
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
	strftime(tshour, 3, "%H", now);
	hour=atoi(tshour);
	sunsaver_Ic=data[3]*79.16/32768.0;
	charge_state=data[9];
	if ((charge_state == 3) && (hour > 12) && (sunsaver_Ic < 0.001)) {		/* If the charge state is night, the time is past noon, 
																			 and the array current is < 0.001, then the newest log record is from today */
		today=1;
	}
	else {											/* Otherwise, the newest record is from yesterday */
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
	
	/* Print out each of the daily log records */
	for (i=0; i<n; i++) {
		if (i != 0) {
			printf("-------------------------------------------------------------\n\n");
		}
		logTime=lclTime-(n-i-today)*(24*60*60);
		logthen = localtime(&logTime);
		strftime(tsdate, 32, "%m/%d/%Y", logthen);
		printf("Date = %s\n",tsdate);
		printf("hourmeter = %d h\n",hourmeter[indx[i]]);
		printf("alarm_daily = Daily controller self-diagnostic alarms:\n");
		if (alarm_daily[indx[i]] == 0) {
			printf("\tNo alarms\n");
		} else {
			if (alarm_daily[indx[i]] & 1) printf("\tRTS open\n");
			if ((alarm_daily[indx[i]] & (1 << 1)) >> 1) printf("\tRTS shorted\n");
			if ((alarm_daily[indx[i]] & (1 << 2)) >> 2) printf("\tRTS disconnected\n");
			if ((alarm_daily[indx[i]] & (1 << 3)) >> 3) printf("\tThs open\n");
			if ((alarm_daily[indx[i]] & (1 << 4)) >> 4) printf("\tThs shorted\n");
			if ((alarm_daily[indx[i]] & (1 << 5)) >> 5) printf("\tSSMPPT hot\n");
			if ((alarm_daily[indx[i]] & (1 << 6)) >> 6) printf("\tCurrent limit\n");
			if ((alarm_daily[indx[i]] & (1 << 7)) >> 7) printf("\tCurrent offset\n");
			if ((alarm_daily[indx[i]] & (1 << 8)) >> 8) printf("\tUndefined\n");
			if ((alarm_daily[indx[i]] & (1 << 9)) >> 9) printf("\tUndefined\n");
			if ((alarm_daily[indx[i]] & (1 << 10)) >> 10) printf("\tUncalibrated\n");
			if ((alarm_daily[indx[i]] & (1 << 11)) >> 11) printf("\tRTS miswire\n");
			if ((alarm_daily[indx[i]] & (1 << 12)) >> 12) printf("\tUndefined\n");
			if ((alarm_daily[indx[i]] & (1 << 13)) >> 13) printf("\tUndefined\n");
			if ((alarm_daily[indx[i]] & (1 << 14)) >> 14) printf("\tMiswire\n");
			if ((alarm_daily[indx[i]] & (1 << 15)) >> 15) printf("\tFET open\n");
			if ((alarm_daily[indx[i]] & (1 << 16)) >> 16) printf("\tP12\n");
			if ((alarm_daily[indx[i]] & (1 << 17)) >> 17) printf("\tHigh Va current limit\n");
			if ((alarm_daily[indx[i]] & (1 << 18)) >> 18) printf("\tPower On Reset\n");
			if ((alarm_daily[indx[i]] & (1 << 19)) >> 19) printf("\tLVD Condition\n");
			if ((alarm_daily[indx[i]] & (1 << 20)) >> 20) printf("\tLog Timeout Alarm\n");
			if ((alarm_daily[indx[i]] & (1 << 21)) >> 21) printf("\tAlarm 22\n");
			if ((alarm_daily[indx[i]] & (1 << 22)) >> 22) printf("\tAlarm 23\n");
			if ((alarm_daily[indx[i]] & (1 << 23)) >> 23) printf("\tAlarm 24\n");
		}
		printf("Vb_min_daily = %.2f V\n",Vb_min_daily[indx[i]]);
		printf("Vb_max_daily = %.2f V\n",Vb_max_daily[indx[i]]);
		printf("Ahc_daily = %.2f Ah\n",Ahc_daily[indx[i]]);
		printf("Ahl_daily = %.2f Ah\n",Ahl_daily[indx[i]]);
		printf("array_fault_daily = Daily solar input self-diagnostic faults:\n");
		if (array_fault_daily[indx[i]] == 0) {
			printf("\tNo faults\n");
		} else {
			if (array_fault_daily[indx[i]] & 1) printf("\tOvercurrent\n");
			if ((array_fault_daily[indx[i]] & (1 << 1)) >> 1) printf("\tFETs shorted\n");
			if ((array_fault_daily[indx[i]] & (1 << 2)) >> 2) printf("\tSoftware bug\n");
			if ((array_fault_daily[indx[i]] & (1 << 3)) >> 3) printf("\tBattery HVD\n");
			if ((array_fault_daily[indx[i]] & (1 << 4)) >> 4) printf("\tArray HVD\n");
			if ((array_fault_daily[indx[i]] & (1 << 5)) >> 5) printf("\tEEPROM setting edit (reset required)\n");
			if ((array_fault_daily[indx[i]] & (1 << 6)) >> 6) printf("\tRTS shorted\n");
			if ((array_fault_daily[indx[i]] & (1 << 7)) >> 7) printf("\tRTS was valid, now disconnected\n");
			if ((array_fault_daily[indx[i]] & (1 << 8)) >> 8) printf("\tLocal temperature sensor failed\n");
			if ((array_fault_daily[indx[i]] & (1 << 9)) >> 9) printf("\tFault 10\n");
			if ((array_fault_daily[indx[i]] & (1 << 10)) >> 10) printf("\tFault 11\n");
			if ((array_fault_daily[indx[i]] & (1 << 11)) >> 11) printf("\tFault 12\n");
			if ((array_fault_daily[indx[i]] & (1 << 12)) >> 12) printf("\tFault 13\n");
			if ((array_fault_daily[indx[i]] & (1 << 13)) >> 13) printf("\tFault 14\n");
			if ((array_fault_daily[indx[i]] & (1 << 14)) >> 14) printf("\tFault 15\n");
			if ((array_fault_daily[indx[i]] & (1 << 15)) >> 15) printf("\tFault 16\n");
		}
		printf("load_fault_daily = Daily load output self-diagnostic faults:\n");
		if (load_fault_daily[indx[i]] == 0) {
			printf("\tNo faults\n");
		} else {
			if (load_fault_daily[indx[i]] & 1) printf("\tExternal short circuit\n");
			if ((load_fault_daily[indx[i]] & (1 << 1)) >> 1) printf("\tOvercurrent\n");
			if ((load_fault_daily[indx[i]] & (1 << 2)) >> 2) printf("\tFETs shorted\n");
			if ((load_fault_daily[indx[i]] & (1 << 3)) >> 3) printf("\tSoftware bug\n");
			if ((load_fault_daily[indx[i]] & (1 << 4)) >> 4) printf("\tHVD\n");
			if ((load_fault_daily[indx[i]] & (1 << 5)) >> 5) printf("\tHeatsink over-temperature\n");
			if ((load_fault_daily[indx[i]] & (1 << 6)) >> 6) printf("\tEEPROM setting edit (reset required)\n");
			if ((load_fault_daily[indx[i]] & (1 << 7)) >> 7) printf("\tFault 8\n");
		}
		printf("Va_max_daily = %.2f V\n",Va_max_daily[indx[i]]);
		printf("time_ab_daily = %d min\n",time_ab_daily[indx[i]]);
		printf("time_eq_daily = %d min\n",time_eq_daily[indx[i]]);
		printf("time_fl_daily = %d min\n\n",time_fl_daily[indx[i]]);
	}
	
	return(0);
}

