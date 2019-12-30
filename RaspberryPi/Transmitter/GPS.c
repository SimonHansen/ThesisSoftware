#include "GPS.h"
#include <time.h>



void getGPSData()
{
	int fd;

	int i = 0;
	int j = 0;

// Open serial port 
	if ((fd = serialOpen (SERIAL_PORT, BAUDRATE)) < 0)
	{	
		fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
	}

// Startup WiringPi
	if (wiringPiSetup () == -1)
	{
		fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
	}

// Check for correct string
	char c[6] = {'$', 'G', 'P', 'G', 'G', 'A'};
	char d;
	delay(1000);
	while(serialDataAvail(fd))
	{
		d = serialGetchar(fd);
		if(d == c[i] || i>=6)
		{
			i++;
			if(i>=6)
			{
				if(d == '\r')
				{
					break;
				}
				gps[i-6]=d;
			}
		}
		else
		{
			i=0;
		}
	}


// Write GPS position to file
	FILE * fPtr;
	fPtr = fopen("GPSLocation.txt", "a");

	struct tm *timeinfo;
	time_t rawtime;
	rawtime = time (NULL);

/* Write data to file */
	fputs(ctime(&rawtime) ,fPtr);
	fputs(gps, fPtr);
	fputs("\n",fPtr);
	fclose(fPtr);
}

