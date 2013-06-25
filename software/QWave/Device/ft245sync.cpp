//
//   ft245sync.cpp
//   QWave
//   Copyright (c) 2012-2013, Bruno Kremel
//   All rights reserved.
//
//    Redistribution and use in source and binary forms, with or without
//    modification, are permitted provided that the following conditions are met:
//    1. Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//    2. Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    3. All advertising materials mentioning features or use of this software
//       must display the following acknowledgement:
//       This product includes software developed by Bruno Kremel.
//    4. Neither the name of Bruno Kremel nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
//    THIS SOFTWARE IS PROVIDED BY Bruno Kremel ''AS IS'' AND ANY
//    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//    DISCLAIMED. IN NO EVENT SHALL Bruno Kremel BE LIABLE FOR ANY
//    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


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

/**
 * @brief Ft245sync::Ft245sync constructor initializes FTDI comunication first in MPSSE mode to
 * make FPGA load design from FLASH and reset design loaded in FPGA. Then switches mode to SYNC FT245
 * and sends cleanup command to initialize communication with device.
 * @param chunkSizeRead
 * @param chunkSizeWrite
 * @param gpio
 * @param vftdic
 * @param vftdic2
 */
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
        throw DeviceException("ftdi_init failure\n", FTDI_ERROR);
    }
    ftdi_set_interface(ftdic, INTERFACE_A);
    f = ftdi_usb_open(ftdic, 0x0403, PID);
    if (f < 0 && f != -5)
    {
        qDebug() << "Error code: " << f;
        throw DeviceException("Unable to open FTDI device, channel A\n", FTDI_ERROR);
    }
    
    // Init 2. channel
    if (ftdi_init(ftdic2) < 0)
    {
        throw DeviceException("ftdi_init failure\n", FTDI_ERROR);
    }
    ftdi_usb_reset(ftdic);
    ftdi_usb_reset(ftdic2);
    ftdi_set_interface(ftdic2, INTERFACE_B);
    f = ftdi_usb_open(ftdic2, 0x0403, PID);
    if (f < 0 && f != -5)
    {
        qDebug() << "Error code: " << f;
        throw DeviceException("Unable to open FTDI device, channel B\n", FTDI_ERROR);
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
    usleep(300);
    buf[0] = SET_BITS_LOW;
    buf[1] = 0xFF; //reseting design in FPGA
    buf[2] = BIT_DIR;
    ftdi_write_data(ftdic2, buf, 3);
    usleep(300);
    buf[0] = SET_BITS_LOW;
    buf[1] = 0xDD; //releasing reset
    buf[2] = BIT_DIR;
    ftdi_write_data(ftdic2, buf, 3);
    usleep(300);
    buf[0] = SET_BITS_HIGH;
    buf[1] = 0xFF; //lighting leds
    buf[2] = BIT_DIR;
    ftdi_write_data(ftdic2, buf, 3);
    
    if (ftdi_usb_purge_buffers(ftdic2))
    {
        throw DeviceException("Purging buffers failed\n", FTDI_ERROR);
    }
    ftdi_usb_close(ftdic2); // close channel 2
    ftdi_deinit(ftdic2); // close channel 2
    ftdic->usb_read_timeout = READ_TIMEOUT;
    ftdic->usb_write_timeout = WRITE_TIMEOUT;
    ftdi_read_data_set_chunksize(ftdic, chunkSizeRead);
    ftdi_write_data_set_chunksize(ftdic, chunkSizeWrite);
    
    if (ftdi_usb_reset(ftdic))
    {
        throw DeviceException("Reset failed\n", FTDI_ERROR);
    }
    usleep(1000);
    
    if(ftdi_usb_purge_buffers(ftdic) < 0)
    {
       throw DeviceException("Setting FT2232 synchronous bitmode failed\n", FTDI_ERROR);
    }
    if(ftdi_set_bitmode(ftdic, 0xFF, 0x00) < 0)
    {
       throw DeviceException("Setting FT2232 synchronous bitmode failed\n", FTDI_ERROR);
    }
    if(ftdi_set_bitmode(ftdic, 0xFF, 0x40) < 0) 
    {
        throw DeviceException("Setting FT2232 synchronous bitmode failed\n", FTDI_ERROR);
    }
    if (ftdi_set_latency_timer(ftdic, 2)) /* AN_130 */
    {
        throw DeviceException("Set latency failed failed\n", FTDI_ERROR);
    }
    //SetUSBParameters(ftHandle,0x10000, 0x10000);
    if (ftdi_setflowctrl(ftdic, SIO_RTS_CTS_HS)) // AN_130 
    {
        throw DeviceException("Set RTS_CTS failed\n", FTDI_ERROR);
    }
    if(ftdi_usb_purge_buffers(ftdic) < 0)
    {
        throw DeviceException("Setting FT2232 synchronous bitmode failed\n", FTDI_ERROR);
    }
    //fixes unalignment of first read (should be fixed in cleaner manner)
    usleep(800);
    unsigned char cleanup[10] = { 0xBB, 0xBB, 0xBB, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
    ftdi_write_data(ftdic, cleanup, 10);
    unsigned char recvbuf[4000];
    read(recvbuf);
}

/**
 * @brief Ft245sync::getReadChunkSize
 * @return returns read chunk size
 */
unsigned int Ft245sync::getReadChunkSize()
{
    unsigned int value;
    if(ftdi_read_data_get_chunksize(ftdic, &value) != 0)
    {
        throw  DeviceException("Wrong ftdi context. FTDI not initilized?", FTDI_ERROR);
    }
    return value;
}

/**
 * @brief Ft245sync::getWriteChunkSize
 * @return returns write chunk size
 */
unsigned int Ft245sync::getWriteChunkSize()
{
    unsigned int value;
    if(ftdi_write_data_get_chunksize(ftdic, &value) != 0)
    {
        throw  DeviceException("Wrong ftdi context. FTDI not initilized?", FTDI_ERROR);
    }
    return value;
}

/**
 * @brief Ft245sync::read reads data into buffer buf.
 * @param buf
 * @return returns <0 if error ocurred, else returns number of bytes read.
 */
int Ft245sync::read(unsigned char * buf)
{
    int res = ftdi_read_data(ftdic, buf, getReadChunkSize());
    //qDebug() << "res rd: " << res << std::endl;
    //std::cerr << "rd data[0]: " << (int)buf[0] << std::endl;
    return res; 
}

/**
 * @brief Ft245sync::write writes data stored in buffer buf.
 * @param buf
 * @return eturns <0 if error ocurred, else returns number of bytes written.
 */
int Ft245sync::write(unsigned char * buf)
{
    int res = ftdi_write_data(ftdic, buf, getWriteChunkSize());
    //qDebug() << "res wr: " << res << std::endl;
    return res;   
}
