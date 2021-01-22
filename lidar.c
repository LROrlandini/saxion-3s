//---------------------------------------------
// Saxion University of Applied Sciences
// Smart Solutions Semester - Fall 2020/2021
// Sky Workers
//
// Credits:
// https://lightware.co.za
//
// Compile with:
// g++ -Wall lidar.cpp -o lidar -lwiringPi
//
// SF11/C LiDAR Rangefinder
// Wiring:
// SDA = white
// SCL = green
// 5v = red
// 0v = black
//---------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringSerial.h>

#define WATER_SPRAYER 3
#define DIR_PIN1 2
#define STEP_PIN1 0

int serial_port;
int lidar;
int dist;

// Sets up Serial port for data transmission
int setup_serial()
{
    if ((serial_port = serialOpen ("/dev/ttyS0", 9600)) < 0)
    {
        fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
        return 1;
    }
    printf("Serial Port Open\n");
    return 0;
}

// Sets up wiringPi to initialise GPIO
int setup_wiringPi()
{
    if (wiringPiSetup() == -1)
    {
        fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
        return 1;
    }
    printf("WiringPi Initialised\n");
    return 0;
}


// Sets up I2C bus for LiDar sensor
int setup_lidar()
{
    int device_address = 0x67;
    lidar = wiringPiI2CSetup(device_address);

    if (lidar == -1)
    {
        printf("I2C Bus file could not be opened\n");
	return 1;
    }
    else
    {
        printf("I2C bus initialised on address: %d\n", lidar);
	return 0;
    }
}

// Gets reading from sensor
int run_lidar(int lidar)
{
    unsigned char byte[2];
    int res = read(lidar, byte, 2);
    if (res == -1)
    {
	printf("LiDar sensor not available\n");
	return 1;
    }
    else
    {
	dist = (byte[0] << 8) | byte[1];
    }
    return dist;
}

int main(int argc, char **argv)
{
    char dist_cm[6];
    int init = 0;
    while (init == 0)
    {
        if ((setup_serial() == 0) && (setup_wiringPi() == 0) && (setup_lidar() == 0))
        {
            init = 1;
	    pinMode(WATER_SPRAYER, OUTPUT);
	    pinMode(DIR_PIN1, OUTPUT);
	    pinMode(STEP_PIN1, OUTPUT);
	    digitalWrite(WATER_SPRAYER, 0);
	    digitalWrite(DIR_PIN1, 0);
	    digitalWrite(STEP_PIN1, 0);
        }
    }
    bool contact = 0;
    int stop = 0; // For debugging
    while (stop != 1)
    {
	delay(5000);
	dist = run_lidar(lidar);
        sprintf(dist_cm, "%05d", dist);
	for (unsigned int i = 0; i < sizeof(dist_cm) -1; i++)
	{
	    serialPutchar(serial_port, dist_cm[i]);
	}
	printf("Distance in Cm = %d\n", dist);
	if ((dist <= 10) && (contact == 0))
	{
	    contact = 1;
	    digitalWrite(DIR_PIN1, 1);
	    for (int j = 0; j < 200; j++)
	    {
		digitalWrite(STEP_PIN1, 1);
		delay(10);
		digitalWrite(STEP_PIN1, 0);
		delay(10);
	    }
	    digitalWrite(WATER_SPRAYER, 1);
	    delay(3000);
	    digitalWrite(WATER_SPRAYER, 0);
	}
	else if ((dist > 10) && (dist <= 200) && (contact == 1))
	{
	    contact = 0;
	    digitalWrite(DIR_PIN1, 0);
	    for (int k = 0; k < 200; k++)
	    {
	        digitalWrite(STEP_PIN1, 1);
		delay(10);
		digitalWrite(STEP_PIN1, 0);
		delay(10);
	    }
	}
	else if (dist > 200)
	{
	    stop = 1;
	}
    }
    return 0;
}
