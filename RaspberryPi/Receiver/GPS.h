 #ifndef GPS_H_
 #define GPS_H_
 
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringSerial.h>

#define SERIAL_PORT "/dev/ttyS0"
#define BAUDRATE	9600

char gps[255];

void getGPSData();


#endif
