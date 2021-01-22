//-----------------------------------------------------------------------
// Saxion University of Applied Sciences
// Smart Solutions Semester - Fall 2020/2021
// Sky Workers
//
// Credits:
// https://github.com/aarushsesto/as5048a_rpi_interface
//
// Compile with:
// g++ -Wall as5048a.cpp as5048a_rpi_spi.cpp -o as5048a -lwiringPi -lbcm2835
//
// AS5048a Magnetic Encoder with adapter board
// Wiring:
// SPI
// CS1
//-----------------------------------------------------------------------

#include "as5048a_rpi_spi.h"
//#include <wiringPi.h>
#include <wiringSerial.h>
#include <string.h>

#define WATER_SPRAYER 22

int serial_port;

// Sets up Serial port for data transmission
int setup_serial()
{
    if ((serial_port = serialOpen ("/dev/ttyS0", 9600)) < 0)
    {
        printf("Unable to open serial device\n");
        return 1;
    }
    printf("Serial Port Open\n");
    return 0;
}

int main()
{
    char dist_rolled[4];
    int result = 0;
    float result_deg = 0.0;
    int init = 0;
    while (init == 0)
    {
        if ((setup_serial() == 0) && (bcm2835_init() == 1))
	{
	    init = 1;
            bcm2835_gpio_fsel(WATER_SPRAYER, BCM2835_GPIO_FSEL_OUTP);
	}
    }

    As5048a chalega_kya(1,2048);

    //Clear any Error on initialization
    result = chalega_kya.Read(AS5048_CMD_CLEAR_ERROR);
    sleep(1);

    //start read loop
   // int tester = 0;
    int reference;
    int dist = 0;
    bool first_read = 1;
    while(1)
    {
        //Get rotation value in hex
        result = chalega_kya.Read(AS5048_CMD_ANGLE);
        // Convert it to degrees
        result_deg = chalega_kya.Degrees(result);
        if (first_read == 1)
	{
	    if (result_deg < 5)
	    {
	        result_deg = 5;
	    }
	    reference = result_deg;
	}
	else
	{
            if(result == -1)
            {
                result = chalega_kya.Read(AS5048_CMD_CLEAR_ERROR); // Clears error
            }
            else
            {
               // std::cout << "Result in degress : " << result_deg << "\n" ;
            }
	}
        if (result_deg == reference - 5)
	{
	    dist += 21; // Circumference of water wheel
	    std::cout << "Distance rolled : " << dist << "cm\n";
	    sprintf(dist_rolled, "%03d", dist);
	    for (unsigned int i = 0; i < sizeof(dist_rolled) - 1; i++)
	    {
		serialPutchar(serial_port, dist_rolled[i]);
	    }
	    bcm2835_gpio_write(WATER_SPRAYER, HIGH);
	    sleep(3);
	    bcm2835_gpio_write(WATER_SPRAYER, LOW);
	}
        first_read = 0;
        usleep(5000);
     //   tester += 1;
    }
    bcm2835_close();
    return 1;
}

