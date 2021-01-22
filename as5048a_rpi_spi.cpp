#include "as5048a_rpi_spi.h"

As5048a::As5048a(int chip_select_spi, int frequency_divider) : csn(chip_select_spi),freq_divider(frequency_divider) {

    // Initialise SPI using the BCM2835 library
    if (!bcm2835_init())
    {
        std::cout<<"bcm2835_init failed. Are you running as root??";
        std::exit(EXIT_FAILURE);
    }

    if (!bcm2835_spi_begin())
    {
        std::cout<<"bcm2835_spi_begin failed. Are you running as root??\n";
        std::exit(EXIT_FAILURE);
    }

    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);

    //Set the Chip Select pin, using the value passed in the constructor

    if(csn)
    {
        bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, LOW);
        bcm2835_spi_chipSelect(BCM2835_SPI_CS1);
    }
    else
    {
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
    }

    // Create a check to make sure the divider isn't an invalid number
    bcm2835_spi_setClockDivider(freq_divider);
    std::cout<<"Initialization done"<<'\n';
    }

As5048a::~As5048a(){}

int send_over_spi(int cmd, int nbytes)
{
    char  send_buff[nbytes];
    char  recv_buff[nbytes];
    char mask = 0xFF;
    int value=0;
    for(int i=0; i < nbytes; i++)
    {
        send_buff[nbytes-i-1]=((cmd>>(i*8)) & mask);
    }
    bcm2835_spi_transfernb(&send_buff[0],&recv_buff[0],nbytes);
    for(int i=0; i < nbytes; i++)
    {
        value<<=(i*8);
        value |= recv_buff[i];
    }
    return value;
}

// The 15th bit(MSB) is always the parity bit with Even parity
// Hence to insert or validate the parity bit
int As5048a::EvenParityCalc(int value_16_bit)
{
    int parity=0;
    for(int i = 0; i <= 14; ++i)
    {
        parity = (((value_16_bit >> i) & 0x0001) ^ parity);
    }
    return parity;
}

// Converts Hex value to decimal
float As5048a::Degrees(int hex_value)
{
    return hex_value * 360 / 0x3FFF;;
}

// Reads encoder position and returns Hex value
const int As5048a::Read(int reg_address)
{
    int result;
    int cmd = reg_address | AS5048_READ;
    cmd = (cmd | ((EvenParityCalc(cmd)<<15) & 0x8000));
    result = send_over_spi(cmd,2);
    usleep(1000);
    result = send_over_spi(AS5048_CMD_NOP,2);
    usleep(100);
    int parity_chk=EvenParityCalc(result);

    return (result & 0x3FFF);
}
