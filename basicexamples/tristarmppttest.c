/*
 *  tristarmppttest.c
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

/* Compile with: cc `pkg-config --cflags --libs libmodbus` tristarmppttest.c -o tristarmppttest */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#define TRISTARMPPT    0x01	/* Default address of the TriStar MPPT */

int main(void)
{
	modbus_t *ctx;
	int rc;
	int V_PU_hi,V_PU_lo,I_PU_hi,I_PU_lo;
	float V_PU,I_PU;
	uint16_t ver_sw;
	uint16_t data[10];
	
	/* Set up a new MODBUS context */
	ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 2);
	if (ctx == NULL) {
		fprintf(stderr, "Unable to create the libmodbus context\n");
		return -1;
	}
	
	/* Set the slave id to the TriStar MPPT MODBUS id */
	modbus_set_slave(ctx, TRISTARMPPT);
	
	/* Open the MODBUS connection to the TriStar MPPT */
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
	V_PU_hi=data[0];
	printf("V_PU_hi=%d\n",V_PU_hi);
	
	V_PU_lo=data[1];
	printf("V_PU_lo=%d\n",V_PU_lo);
	
	I_PU_hi=data[2];
	printf("I_PU_hi=%d\n",I_PU_hi);
	
	I_PU_lo=data[3];
	printf("I_PU_lo=%d\n",I_PU_lo);
	
	ver_sw=data[4];
	printf("ver_sw=0x%0X\n",ver_sw);
	
	/* Calculate V_PU and I_PU following the TriStar MPPT MODBUS documentation examples */
	
	V_PU = V_PU_hi + V_PU_lo/65536.0;
	printf("V_PU=%.5f\n",V_PU);
	
	I_PU = I_PU_hi + I_PU_lo/65536.0;
	printf("I_PU=%.5f\n",I_PU);
	
    /* Close the MODBUS connection */
	modbus_close(ctx);
	modbus_free(ctx);
	
	return(0);
}
