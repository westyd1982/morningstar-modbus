/*
 *  suresine.c - This program reads all the RAM and EEPROM registers on a Moringstar SureSine-300 and prints the results.
 *  

Copyright 2012 Tom Rinehart.

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

/* Compile with: cc `pkg-config --cflags --libs libmodbus` suresine.c -o suresine */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#define SURESINE    0x02	/* MODBUS Address of the SureSine-300 */

int main(void)
{
	modbus_t *ctx;
	int rc;
	float adc_vb, adc_iac, adc_ths, adc_remon, Vb, Iac, mod_index;
	short Ths;
	unsigned short load_state, volts, hertz, m_disconnect, modbus_reset;
	unsigned short fault, alarm, dip_switch;
	float EVb_min, EVb_max, EV_lvd2, EV_lvr2, EV_hvd2, EV_hvr2, Et_lvd_warn2, EV_lvdwarn_beep2, EV_lvrwarn_beep2, EV_startlvd2;
	unsigned short Emodbus_id, Emeter_id, Ehourmeter;
	char Eserial_no[9];
	uint16_t data[20];
	
	/* Set up a new MODBUS context */
	ctx = modbus_new_rtu("/dev/tty.solar", 9600, 'N', 8, 2);
	if (ctx == NULL) {
		fprintf(stderr, "Unable to create the libmodbus context\n");
		return -1;
	}
	
	/* Set the slave id to the SureSine-300 MODBUS id */
	modbus_set_slave(ctx, SURESINE);
	
	/* Open the MODBUS connection to the SureSine-300 */
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
	
	/* Read the RAM Registers and convert the results to their proper values */
	rc = modbus_read_registers(ctx, 0x0000, 17, data);
	
	printf("RAM Registers\n\n");
	
	adc_vb=data[0]*16.92/65536.0;
	printf("adc_vb = %.2f V\n",adc_vb);
	
	adc_iac=data[1]*16.92/32768.0;
	printf("adc_iac = %.4f A\n",adc_iac);
	
	adc_ths=data[2]*16.92/65536.0;
	printf("adc_ths = %.2f V\n",adc_ths);
	
	adc_remon=data[3]*16.92/65536.0;
	printf("adc_remon = %.2f V\n",adc_remon);
	
	Vb=data[4]*16.92/65536.0;
	printf("Vb = %.2f V\n",Vb);
	
	Iac=data[5]*16.92/32768.0;
	printf("Iac = %.4f A\n",Iac);
	
	Ths=data[6];
	printf("Ths = %d °C\n",Ths);
	
	fault=data[7];
	printf("fault = SureSine Faults = %d\n", fault);
	if (fault == 0) {
		printf("\tNo faults\n");
	} else {
		if (fault & 1) printf("\tReset\n");
		if ((fault & (1 << 1)) >> 1) printf("\tOver-current\n");
		if ((fault & (1 << 2)) >> 2) printf("\tnot used\n");
		if ((fault & (1 << 3)) >> 3) printf("\tSoftware\n");
		if ((fault & (1 << 4)) >> 4) printf("\tHVD\n");
		if ((fault & (1 << 5)) >> 5) printf("\tHot (heatsink temp. over 95 C)\n");
		if ((fault & (1 << 6)) >> 6) printf("\tDIP switch\n");
		if ((fault & (1 << 7)) >> 7) printf("\tSettings Edit\n");
	}
	
	alarm=data[8];
	printf("alarm = SureSine Alarms = %d\n", alarm);
	if (alarm == 0) {
		printf("\tNo alarms\n");
	} else {
		if (alarm & 1) printf("\tHeatsink temp. sensor open\n");
		if ((alarm & (1 << 1)) >> 1) printf("\tHeatsink temp. sensor shorted\n");
		if ((alarm & (1 << 2)) >> 2) printf("\tnot used\n");
		if ((alarm & (1 << 3)) >> 3) printf("\tHeatsink hot (above 80 C)\n");
		if ((alarm & (1 << 5)) >> 5) printf("\tHeatsink hot (above 80 C)\n");  /* On my system the heatsink hot alarm is the 5th bit ??? */
	}
	
	dip_switch=data[10];
	printf("dip_switch = DIP switch settings\n");
	if (dip_switch & 1) {
		printf("\tSwitch 1 ON - Power Mode = Standby Mode\n");
	} else {
		printf("\tSwitch 1 OFF - Power Mode = Always On\n");
	}
	if ((dip_switch & (1 << 1)) >> 1) {
		printf("\tSwitch 2 ON - LVD = 10.5 V, LVR = 11.6 V or custom settings\n");
	} else {
		printf("\tSwitch 2 OFF - LVD = 11.5 V, LVR = 12.6 V\n");
	}
	if ((dip_switch & (1 << 2)) >> 2) {
		printf("\tSwitch 3 ON - Beeper Warning Off\n");
	} else {
		printf("\tSwitch 3 OFF - Beeper Warning On\n");
	}
	if ((dip_switch & (1 << 3)) >> 3) {
		printf("\tSwitch 4 ON - MODBUS Protocol\n");
	} else {
		printf("\tSwitch 4 OFF - Meterbus Protocol\n");
	}
	
	load_state=data[11];
	switch (load_state) {
		case 0:
			printf("load_state = %d Start-up\n",load_state);
			break;
		case 1:
			printf("load_state = %d Load On\n",load_state);
			break;
		case 2:
			printf("load_state = %d LVD Warning\n",load_state);
			break;
		case 3:
			printf("load_state = %d LVD (Low Voltage Disconnect)\n",load_state);
			break;
		case 4:
			printf("load_state = %d Fault State\n",load_state);
			break;
		case 5:
			printf("load_state = %d Load Disconnected\n",load_state);
			break;
		case 6:
			printf("load_state = %d Load Off\n",load_state);
			break;
		case 7:
			printf("load_state = %d not used\n",load_state);
			break;
		case 8:
			printf("load_state = %d Standby\n",load_state);
			break;
	}
	
	mod_index=data[12]*100.0/256.0;
	printf("mod_index = %.2f %%\n",mod_index);
	
	volts=data[13];
	printf("volts = %d V\n",volts);
	
	hertz=data[14];
	printf("hertz = %d Hz\n",hertz);
	
	m_disconnect=data[15];
	printf("m_disconnect = %d\n",m_disconnect);
	
	modbus_reset=data[16];
	printf("modbus_reset = %d\n",modbus_reset);
	
	/* Read the EEPROM Registers and convert the results to their proper values */
	rc = modbus_read_registers(ctx, 0xE000, 12, data);
	
	printf("\nEEPROM Registers\n\n");
	
	EVb_min=data[0]*16.92/65536.0;
	printf("EVb_min = %.2f V\n",EVb_min);
	
	EVb_max=data[1]*16.92/65536.0;
	printf("EVb_max = %.2f V\n",EVb_max);
	
	Emodbus_id=data[2];
	printf("Emodbus_id = %d\n",Emodbus_id);
	
	Emeter_id=data[3];
	printf("Emeter_id = %d\n",Emeter_id);
	
	EV_lvd2=data[4]*16.92/65536.0;
	printf("EV_lvd2 = %.2f V\n",EV_lvd2);
	
	EV_lvr2=data[5]*16.92/65536.0;
	printf("EV_lvr2 = %.2f V\n",EV_lvr2);
	
	EV_hvd2=data[6]*16.92/65536.0;
	printf("EV_hvd2 = %.2f V\n",EV_hvd2);
	
	EV_hvr2=data[7]*16.92/65536.0;
	printf("EV_hvr2 = %.2f V\n",EV_hvr2);
	
	Et_lvd_warn2=data[8]*0.1;
	printf("Et_lvd_warn2 = %.1f s\n",Et_lvd_warn2);
	
	EV_lvdwarn_beep2=data[9]*16.92/65536.0;
	printf("EV_lvdwarn_beep2 = %.2f V\n",EV_lvdwarn_beep2);
	
	EV_lvrwarn_beep2=data[10]*16.92/65536.0;
	printf("EV_lvrwarn_beep2 = %.2f V\n",EV_lvrwarn_beep2);
	
	EV_startlvd2=data[11]*16.92/65536.0;
	printf("EV_startlvd2 = %.2f V\n",EV_startlvd2);
	
	/* Read the EEPROM Registers and convert the results to their proper values */
	rc = modbus_read_registers(ctx, 0xE040, 8, data);
	
	Ehourmeter=data[0];
	printf("Ehourmeter = %d\n",Ehourmeter);
	
	Eserial_no[0]=data[4] & 0x00FF;
	Eserial_no[1]=data[4] >> 8;
	Eserial_no[2]=data[5] & 0x00FF;
	Eserial_no[3]=data[5] >> 8;
	Eserial_no[4]=data[6] & 0x00FF;
	Eserial_no[5]=data[6] >> 8;
	Eserial_no[6]=data[7] & 0x00FF;
	Eserial_no[7]=data[7] >> 8;
	Eserial_no[8]='\0';
	printf("Eserial_no = %s\n",Eserial_no);
	
	/* Close the MODBUS connection */
	modbus_close(ctx);
	modbus_free(ctx);
	
	return(0);
}

