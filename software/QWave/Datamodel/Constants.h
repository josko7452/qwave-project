//
//   Constants.h
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


#ifndef QWave_Constants_h
#define QWave_Constants_h

/**
 * This file defines constants used in various places in application.
 */

#define DIV_S "s/div"
#define DIV_MS "ms/div"
#define DIV_US  "us/div"
#define DIV_NS  "ns/div"
#define DIV_PS  "ps/div"

#define S  "s"
#define MS "ms"
#define US  "us"
#define NS "ns"
#define PS "ps"

#define E3_HZ  "mHz"
#define HERTZ  "Hz"
#define KHZ  "kHz"
#define MHZ  "MHz"

static const unsigned char HIGH = 255;
static const unsigned char LOW = 0;
static const unsigned char HIGH_IMPEDANCE = 100;
static const unsigned char NO_INFORMATION = 150;

//static const unsigned long long E20 = 100000000000000000000ULL;
static const unsigned long long E19 = 10000000000000000000ULL;
static const unsigned long long E18 = 1000000000000000000ULL;
static const unsigned long long E17 = 100000000000000000ULL;
static const unsigned long long E16 = 10000000000000000ULL;
static const unsigned long long E15 = 1000000000000000ULL;
static const unsigned long long E14 = 100000000000000ULL;
static const unsigned long long E13 = 10000000000000ULL;
static const unsigned long long E12 = 1000000000000ULL;
static const unsigned long long E11 = 100000000000ULL;
static const unsigned long long E10 = 10000000000ULL;
static const unsigned long long E9  = 1000000000ULL;
static const unsigned long long E8  = 100000000ULL;
static const unsigned long long E7  = 10000000ULL;
static const unsigned long long E6  = 1000000ULL;
static const unsigned long long E5  = 100000ULL;
static const unsigned long long E4  = 10000ULL;
static const unsigned long long E3  = 1000ULL;
static const unsigned long long E2  = 100ULL;

#endif
