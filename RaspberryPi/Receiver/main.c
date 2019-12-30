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
	getGPSData();  // Uncomment for outdoor scenario
	setParameters(868100000, 10, SF12, BW_125, CR_4_5);
	opmode(OPMODE_RX);
	createLogFile(argv[1]);
	while(1)
	{
		receivepacket();
	}
	
	return 0;
}

