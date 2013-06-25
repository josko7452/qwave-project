//
//  ft245sync.cpp
//  ft245test
//
//  Created by Bruno Kremel on 25.3.2013.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "ft245sync.h"

#define BBMODE_SPI    2
#define FTDISPI_GPO0 0x10 /**< @brief General Purpose Output bits 0 (D4) */
#define FTDISPI_GPO1 0x20 /**< @brief General Purpose Output bits 1 (D5) */
#define FTDISPI_GPO2 0x40 /**< @brief General Purpose Output bits 2 (D6) */
#define FTDISPI_GPO3 0x80 /**< @brief General Purpose Output bits 3 (D7) */
#define BIT_P_CS 0x08
#define BIT_P_DI 0x04
#define BIT_P_DO 0x02
#define BIT_P_SK 0x01
#define BIT_P_G0 FTDISPI_GPO0
#define BIT_P_G1 FTDISPI_GPO1
#define BIT_P_G2 FTDISPI_GPO2
#define BIT_P_G3 FTDISPI_GPO3
#define BIT_DIR (BIT_P_SK|BIT_P_DO|BIT_P_CS|BIT_P_G0|BIT_P_G1|BIT_P_G2|BIT_P_G3)

Ft245sync::Ft245sync(unsigned int chunkSizeRead,
                     unsigned int chunkSizeWrite,
                     uint8_t gpio,
                     struct ftdi_context * vftdic, 
                     struct ftdi_context * vftdic2)
{
    if(vftdic == NULL)
    {
        this->ftdic = static_cast<struct ftdi_context*>(malloc(sizeof(struct ftdi_context)));
    }
    else
    {
        this->ftdic = vftdic;
    }
    if(vftdic2 == NULL)
    {
        this->ftdic2 = static_cast<struct ftdi_context*>(malloc(sizeof(struct ftdi_context)));
    }
    else
    {
        this->ftdic2 = vftdic2;
    }
    int f;
    // Init 1. channel
    if (ftdi_init(ftdic) < 0)
    {
        throw Exception("ftdi_init failure\n");
    }
    ftdi_set_interface(ftdic, INTERFACE_A);
    f = ftdi_usb_open(ftdic, 0x0403, PID);
    if (f < 0 && f != -5)
    {
        throw Exception("Unable to open FTDI device, channel A\n");
    }
    
    // Init 2. channel
    if (ftdi_init(ftdic2) < 0)
    {
        throw Exception("ftdi_init failure\n");
    }
    ftdi_usb_reset(ftdic);
    ftdi_usb_reset(ftdic2);
    ftdi_set_interface(ftdic2, INTERFACE_B);
    f = ftdi_usb_open(ftdic2, 0x0403, PID);
    if (f < 0 && f != -5)
    {
        throw Exception("Unable to open FTDI device, channel B\n");
    }
    
    ftdi_write_data_set_chunksize(ftdic2, 512);
    ftdi_set_interface(ftdic2, INTERFACE_B);
    ftdi_usb_reset(ftdic2);
    ftdi_set_latency_timer(ftdic2, 2);
    ftdi_setflowctrl(ftdic2, SIO_RTS_CTS_HS);
    ftdi_set_bitmode(ftdic2, 0, BBMODE_SPI);
    
    uint8_t buf[3];
    buf[0] = SET_BITS_LOW;
    buf[1] = 8;
    buf[2] = BIT_DIR; //holding programming of FPGA*/
    ftdi_write_data(ftdic2, buf, 3);
    buf[0] = SET_BITS_HIGH;
    buf[1] = 0xFF; //lighting leds
    buf[2] = BIT_DIR;
    ftdi_write_data(ftdic2, buf, 3);
    buf[0] = SET_BITS_HIGH;
    buf[1] = 0x00; //lighting leds
    buf[2] = BIT_DIR;
    ftdi_write_data(ftdic2, buf, 3);
    buf[0] = SET_BITS_LOW;
    buf[1] = gpio;
    buf[2] = BIT_DIR; //releasing programming of FPGA
    ftdi_write_data(ftdic2, buf, 3);
    sleep(1);
    buf[0] = SET_BITS_LOW;
    buf[1] = 0xFF; //reseting design in FPGA
    buf[2] = BIT_DIR;
    ftdi_write_data(ftdic2, buf, 3);
    sleep(1);
    buf[0] = SET_BITS_LOW;
    buf[1] = 0xDD; //releasing reset
    buf[2] = BIT_DIR;
    ftdi_write_data(ftdic2, buf, 3);
    sleep(1);
    buf[0] = SET_BITS_HIGH;
    buf[1] = 0xFF; //lighting leds
    buf[2] = BIT_DIR;
    ftdi_write_data(ftdic2, buf, 3);
    
    if (ftdi_usb_purge_buffers(ftdic2))
    {
        throw Exception("Purging buffers failed\n");
    }
    ftdi_usb_close(ftdic2); // close channel 2
    ftdi_deinit(ftdic2); // close channel 2
    ftdic->usb_read_timeout = READ_TIMEOUT;
    ftdic->usb_write_timeout = WRITE_TIMEOUT;
    ftdi_read_data_set_chunksize(ftdic, chunkSizeRead);
    ftdi_write_data_set_chunksize(ftdic, chunkSizeWrite);
    
    if (ftdi_usb_reset(ftdic))
    {
        throw Exception("Reset failed\n");
    }
    usleep(1000);
    
    if(ftdi_usb_purge_buffers(ftdic) < 0)
    {
       throw Exception("Setting FT2232 synchronous bitmode failed\n");
    }
    if(ftdi_set_bitmode(ftdic, 0xFF, 0x00) < 0)
    {
       throw Exception("Setting FT2232 synchronous bitmode failed\n");
    }
    if(ftdi_set_bitmode(ftdic, 0xFF, 0x40) < 0) 
    {
        throw Exception("Setting FT2232 synchronous bitmode failed\n");
    }
    if (ftdi_set_latency_timer(ftdic, 2)) /* AN_130 */
    {
        throw Exception("Set latency failed failed\n");
    }
    //SetUSBParameters(ftHandle,0x10000, 0x10000);
    if (ftdi_setflowctrl(ftdic, SIO_RTS_CTS_HS)) // AN_130 
    {
        throw Exception("Set RTS_CTS failed\n");
    }
    /*if(ftdi_usb_purge_buffers(ftdic) < 0)
    {
        throw Exception("Setting FT2232 synchronous bitmode failed\n");
    }*/
    //fixes unalignment of first read (should be fixed in cleaner manner)
    usleep(400);
    unsigned char cleanup[10] = { 0xBB, 0xBB, 0xBB, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
    ftdi_write_data(ftdic, cleanup, 10);
    unsigned char recvbuf[4000];
    read(recvbuf);
}

unsigned int Ft245sync::getReadChunkSize()
{
    unsigned int value;
    if(ftdi_read_data_get_chunksize(ftdic, &value) != 0)
    {
        throw Exception("Wrong ftdi context. FTDI not initilized?");
    }
    return value;
}

unsigned int Ft245sync::getWriteChunkSize()
{
    unsigned int value;
    if(ftdi_write_data_get_chunksize(ftdic, &value) != 0)
    {
        throw Exception("Wrong ftdi context. FTDI not initilized?");
    }
    return value;
}

int Ft245sync::read(unsigned char * buf)
{
    int res = ftdi_read_data(ftdic, buf, getReadChunkSize());
    std::cerr << "res rd: " << res << std::endl;
    std::cerr << "rd data[0]: " << (int)buf[0] << std::endl;
    return res; 
}

int Ft245sync::write(unsigned char * buf)
{
    int res = ftdi_write_data(ftdic, buf, getWriteChunkSize());
    std::cerr << "res wr: " << res << std::endl;
    return res;   
}