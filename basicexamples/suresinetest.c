/*
 *  suresinetest.c
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

/* Compile with: cc `pkg-config --cflags --libs libmodbus` suresinetest.c -o suresinetest */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#define SURESINE300    0x01	/* Default address of the SureSine-300 */

int main(void)
{
	modbus_t *ctx;
	int rc;
	float adc_vb,adc_iac,adc_ths,adc_remon,Vb;
	uint16_t data[10];
	
	/* Set up a new MODBUS context */
	ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 2);
	if (ctx == NULL) {
		fprintf(stderr, "Unable to create the libmodbus context\n");
		return -1;
	}
	
	/* Set the slave id to the SureSine-300 MODBUS id */
	modbus_set_slave(ctx, SURESINE300);
	
	/* Open the MODBUS connection to the SureSine-300 */
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
	adc_vb=data[0]*16.92/65536.0;
	printf("adc_vb=%.2f\n",adc_vb);
	
	adc_iac=data[1]*17.0/32768.0;
	printf("adc_iac=%.2f\n",adc_iac);
	
	adc_ths=data[2]*16.92/65536.0;			/* Scaling not documented - guess */
	printf("adc_ths=%.2f\n",adc_ths);
	
	adc_remon=data[3]*16.92/65536.0;		/* Scaling not documented - guess */
	printf("adc_remon=%.2f\n",adc_remon);
	
	Vb=data[4]*16.92/65536.0;
	printf("Vb=%.2f\n",Vb);
	
    /* Close the MODBUS connection */
	modbus_close(ctx);
	modbus_free(ctx);
	
	return(0);
}
