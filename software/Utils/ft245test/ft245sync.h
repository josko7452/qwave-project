//
//  ft245sync.h
//  ft245test
//
//  Created by Bruno Kremel on 25.3.2013.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef ft245test_ft245sync_h
#define ft245test_ft245sync_h

#include <ftdi.h>
#define PID 0x6010

#define READ_TIMEOUT 20000
#define WRITE_TIMEOUT 20000


class Exception 
{    
protected:
    std::string message;
public:
    Exception(std::string message):message(message) {};
    inline std::string getError() { return message; };
};

class Ft245sync 
{
    struct ftdi_context * ftdic, * ftdic2;
    unsigned int chunkSize;
public:
    Ft245sync(unsigned int chunkSizeRead, unsigned int chunkSizeWrite, uint8_t gpio, struct ftdi_context * ftdic = NULL, struct ftdi_context * ftdic2 = NULL);
    unsigned int getReadChunkSize();
    unsigned int getWriteChunkSize();
    struct ftdi_context * getFtdiContext() { return ftdic; };
    int write(unsigned char * buf);
    int read(unsigned char * buf);
};

#endif
