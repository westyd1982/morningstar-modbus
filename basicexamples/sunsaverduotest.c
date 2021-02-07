/*
 *  sunsaverduotest.c
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

/* Compile with: cc `pkg-config --cflags --libs libmodbus` sunsaverduotest.c -o sunsaverduotest */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#define SUNSAVERDUO    0x01	/* Default address of the SunSaver Duo */

int main(void)
{
	modbus_t *ctx;
	int rc;
	float vb1, vb2, va, ia1, ia2;
	uint16_t data[10];
	
	/* Set up a new MODBUS context */
	ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 2);
	if (ctx == NULL) {
		fprintf(stderr, "Unable to create the libmodbus context\n");
		return -1;
	}
	
	/* Set the slave id to the SunSaver Duo MODBUS id */
	modbus_set_slave(ctx, SUNSAVERDUO);
	
	/* Open the MODBUS connection to the SunSaver Duo */
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
	vb1=data[0]/1800.0;
	printf("vb1=%.2f\n",vb1);
	
	vb2=data[1]/1800.0;
	printf("vb2=%.2f\n",vb2);
	
	va=data[2]/1032.0;
	printf("va=%.2f\n",va);
	
	ia1=data[3]/673.0;
	printf("ia1=%.2f\n",ia1);
	
	ia2=data[4]/673.0;
	printf("ia2=%.2f\n",ia2);
	
    /* Close the MODBUS connection */
	modbus_close(ctx);
	modbus_free(ctx);
	
	return(0);
}
