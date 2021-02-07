/*
 *  relaydrivertest.c
 *  
 
 Copyright 2013 Tom Rinehart.
 
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
 
 */

/* Compile with: cc `pkg-config --cflags --libs libmodbus` relaydrivertest.c -o relaydrivertest */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#define RELAYDRIVER    0x09	/* Default address of the Relay Driver */

int main(void)
{
	modbus_t *ctx;
	int rc;
	float adc_vb,adc_vch1,adc_vch2,adc_vch3,adc_vch4;
	uint16_t data[10];
	
	/* Set up a new MODBUS context */
	ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 2);
	if (ctx == NULL) {
		fprintf(stderr, "Unable to create the libmodbus context\n");
		return -1;
	}
	
	/* Set the slave id to the Relay Driver MODBUS id */
	modbus_set_slave(ctx, RELAYDRIVER);
	
	/* Open the MODBUS connection to the Relay Driver */
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
		
	/* Read the first five RAM Registers */
	rc = modbus_read_registers(ctx, 0x0000, 5, data);
	if (rc == -1) {
		fprintf(stderr, "%s\n", modbus_strerror(errno));
		return -1;
	}
	
	/* Convert the results to their proper floating point values */
	adc_vb=data[0]*78.421/32768.0;
	printf("adc_vb=%.2f\n",adc_vb);
	
	adc_vch1=data[1]*78.421/32768.0;
	printf("adc_vch1=%.2f\n",adc_vch1);
	
	adc_vch2=data[2]*78.421/32768.0;
	printf("adc_vch2=%.2f\n",adc_vch2);
	
	adc_vch3=data[3]*78.421/32768.0;
	printf("adc_vch3=%.2f\n",adc_vch3);
	
	adc_vch4=data[4]*78.421/32768.0;
	printf("adc_vch4=%.2f\n",adc_vch4);
	
    /* Close the MODBUS connection */
	modbus_close(ctx);
	modbus_free(ctx);
	
	return(0);
}
