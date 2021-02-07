/*
 *  tristarpwmtest.c
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

/* Compile with: cc `pkg-config --cflags --libs libmodbus` tristarpwmtest.c -o tristarpwmtest */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#define TRISTARPWM    0x01	/* Default address of the TriStar PWM */

int main(void)
{
	modbus_t *ctx;
	int rc;
	float adc_vb_f,adc_vs_f,adc_vx_f,adc_ipv_f,adc_iload_f;
	uint16_t data[10];
	
	/* Set up a new MODBUS context */
	ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 2);
	if (ctx == NULL) {
		fprintf(stderr, "Unable to create the libmodbus context\n");
		return -1;
	}
	
	/* Set the slave id to the TriStar PWM MODBUS id */
	modbus_set_slave(ctx, TRISTARPWM);
	
	/* Open the MODBUS connection to the TriStar PWM */
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
		
	/* Read the first five RAM Registers */
	rc = modbus_read_registers(ctx, 0x0008, 5, data);
	if (rc == -1) {
		fprintf(stderr, "%s\n", modbus_strerror(errno));
		return -1;
	}
	
	/* Convert the results to their proper floating point values */
	adc_vb_f=data[0]*96.667/32768.0;
	printf("adc_vb_f=%.2f\n",adc_vb_f);
	
	adc_vs_f=data[1]*96.667/32768.0;
	printf("adc_vs_f=%.2f\n",adc_vs_f);
	
	adc_vx_f=data[2]*139.15/32768.0;
	printf("adc_vx_f=%.2f\n",adc_vx_f);
	
	adc_ipv_f=data[3]*66.667/32768.0;
	printf("adc_ipv_f=%.2f\n",adc_ipv_f);
	
	adc_iload_f=data[4]*316.67/32768.0;
	printf("adc_iload_f=%.2f\n",adc_iload_f);
	
    /* Close the MODBUS connection */
	modbus_close(ctx);
	modbus_free(ctx);
	
	return(0);
}
