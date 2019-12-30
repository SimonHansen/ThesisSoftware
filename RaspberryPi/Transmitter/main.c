/*
 * main.c
 * 
 * Copyright 2019  <pi@raspberrypi>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include "LoRa.h"
#include "GPS.h"

int main(int argc, char **argv)
{
	getGPSData();
	char* message = "ThisIsATestPacketWithSequenceNumber";
	char* warmUpMessage = "WarmUpPacket";
	setParameters(868100000, 10, SF7, BW_125, CR_4_5);
	char finalMessage[50];
	char str[5];
	opmode(OPMODE_LORA);
	int i = 0;
	int j = 0;
	while(j<=3000) // Depends on SF
	{
		transmitpacket(warmUpMessage, strlen(warmUpMessage));
		j++;
		delay(100); // Depends on SF
	}	
	while(i<=6000) // Depends on SF
	{
		sprintf(str, "%05d", i);
		strcpy(finalMessage, message);
		strcat(finalMessage, str);
		transmitpacket(finalMessage, strlen(finalMessage));
		i++;
		delay(100); // Depends on SF
	}

	return 0;
}

