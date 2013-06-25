//
//   ft245sync.h
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


#ifndef ft245test_ft245sync_h
#define ft245test_ft245sync_h

#include <ftdi.h>
#include "../Exceptions/DeviceException.h"
#define PID 0x0666

#define READ_TIMEOUT 20000
#define WRITE_TIMEOUT 20000

/**
 * @brief The Ft245sync class represents wrapper class over libftdi
 *  for controlling FT245 bus on FT2232H device and setting up FPGA bitstream loading a design reset.
 */
class Ft245sync 
{
    struct ftdi_context * ftdic, * ftdic2;
    unsigned int chunkSize;
public:
    Ft245sync(unsigned int chunkSizeRead, unsigned int chunkSizeWrite, uint8_t gpio, struct ftdi_context * ftdic = NULL, struct ftdi_context * ftdic2 = NULL);
    unsigned int getReadChunkSize();
    unsigned int getWriteChunkSize();
    struct ftdi_context * getFtdiContext() { return ftdic; }
    int write(unsigned char * buf);
    int read(unsigned char * buf);
};

#endif
