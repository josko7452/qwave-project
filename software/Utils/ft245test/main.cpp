//
//  main.cpp
//  ft245test
//
//  Created by Bruno Kremel on 25.3.2013.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "ft245sync.h"

#define SET_VLOG 0x11
#define SET_VREF 0x10
#define SET_DECIMATION 0x30
#define SET_TRIGGER 0x31
#define SET_ATTENUATORS 0x20
#define SET_COUPLINGS 0x21
#define START_CAPTURE 0x40
#define DATA_OK 0x80

static const int BUFFSIZE = 1<<15;

int main(int argc, char **argv)
{
    Ft245sync ft245(BUFFSIZE, 10, 0xFF);
    std::cout << "Interface configured\n";
    //send SET VREF
    unsigned char recvbuf[BUFFSIZE];
    recvbuf[0] = 0;
    while(recvbuf[0] != DATA_OK)
    {
        unsigned char buf[10] = { SET_VREF, 255, 0, 255, 0, 0, 0, 0, 0, 0 };
        if(ft245.write(buf) <  0)
        {
            std::cerr << "Error writing data\n";
        }
        if(ft245.read(recvbuf) < 0)
        {
            std::cerr << "Error receiving data\n";
        }
    }
    /*//send SET ATTENUATORS
    recvbuf[0] = 0;
    while(recvbuf[0] != DATA_OK)
    {
        unsigned char buf2[10] = { SET_ATTENUATORS, 0, 0, 0, 0, 0, 0, 0xCC, 0, 0xFF };
        if(ft245.write(buf2) <  0)
        {
            std::cerr << "Error writing data2\n";
        }
        if(ft245.read(recvbuf) < 0)
        {
            std::cerr << "Error receiving data\n";
        }
    }
    //send SET COUPLINGS 
    recvbuf[0] = 0;
    while(recvbuf[0] != DATA_OK)
    {
        unsigned char buf4[10] = { SET_COUPLINGS, 0, 0, 0, 0, 0, 0, 0xAA, 0, 0 };
        if(ft245.write(buf4) <  0)
        {
            std::cerr << "Error writing data4\n";
        }
        if(ft245.read(recvbuf) < 0)
        {
            std::cerr << "Error receiving data\n";
        }
    }*/
    //send SET ATTENUATORS
    recvbuf[0] = 0;
    while(recvbuf[0] != DATA_OK)
    {
        unsigned char buf8[10] = { SET_ATTENUATORS, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0 };
        if(ft245.write(buf8) <  0)
        {
            std::cerr << "Error writing data2\n";
        }
        if(ft245.read(recvbuf) < 0)
        {
            std::cerr << "Error receiving data\n";
        }
    }
    //send SET COUPLINGS
    recvbuf[0] = 0;
    while(recvbuf[0] != DATA_OK)
    {
        unsigned char buf9[10] = { SET_COUPLINGS, 0xFF, 0xFF, 0, 0, 0, 0, 0, 0xBB, 0 };
        if(ft245.write(buf9) <  0)
        {
            std::cerr << "Error writing data4\n";
        }
        if(ft245.read(recvbuf) < 0)
        {
            std::cerr << "Error receiving data\n";
        }
    }
    //send SET VLOG
    recvbuf[0] = 0;
    while(recvbuf[0] != DATA_OK)
    {
        unsigned char buf0[10] = { SET_VLOG, 10, 0, 0, 0, 0, 0, 0, 0, 0 };
        if(ft245.write(buf0) <  0)
        {
            std::cerr << "Error writing data0\n";
        }
        if(ft245.read(recvbuf) < 0)
        {
            std::cerr << "Error receiving data\n";
        }
    }
    //send SET DECIMATION
    recvbuf[0] = 0;
    while(recvbuf[0] != DATA_OK)
    {
        unsigned char buf1[10] = { SET_DECIMATION, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        if(ft245.write(buf1) <  0)
        {
            std::cerr << "Error writing data1\n";
        }
        if(ft245.read(recvbuf) < 0)
        {
            std::cerr << "Error receiving data\n";
        }
    }
    //send SET TRIGGER
    //while(1)
    //{
        recvbuf[0] = 0;
        while(recvbuf[0] != DATA_OK)
        {
            unsigned char buf3[10] = { SET_TRIGGER, 120, 0, 0, 0, 0, 0, 0, 0, 0 };
            if(ft245.write(buf3) <  0)
            {
                std::cerr << "Error writing data3\n";
            }
            if(ft245.read(recvbuf) < 0)
            {
                std::cerr << "Error receiving data\n";
            }
            
        }
    //}
    //send START CAPTURE 
    for(int i = 0; i < 100; ++i)
    {
        unsigned char buf5[10] = { START_CAPTURE, 0x05, 0, 0, 0, 0, 0, 0, 0, 0 };
        if(ft245.write(buf5) <  0)
        {
            std::cerr << "Error writing data5\n";
        }
        std::cout << "Capture started\n";
        //for (int i = 0; i < 30; ++i) 
        //{
        while(ft245.read(recvbuf) < 1){}
        while(ft245.read(recvbuf) > 0){}
        std::cerr << "Data captured: " << i << "\n";
        usleep(1000);
    }
    //}
}