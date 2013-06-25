#
#   This file is part of QWave firmware. 
#   Copyright (c) 2012-2013, Bruno Kremel
#   All rights reserved.
#
#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions are met:
#    1. Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#    2. Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#    3. All advertising materials mentioning features or use of this software
#       must display the following acknowledgement:
#       This product includes software developed by Bruno Kremel.
#    4. Neither the name of Bruno Kremel nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
#    THIS SOFTWARE IS PROVIDED BY Bruno Kremel ''AS IS'' AND ANY
#    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#    DISCLAIMED. IN NO EVENT SHALL Bruno Kremel BE LIABLE FOR ANY
#    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from myhdl import *

BIT_WIDTH = 8
MAXIMAL_RATIO = 2**8
MAX_VALUE = 2**BIT_WIDTH-1
MIN_VALUE = 0
LFSR_SEED = 468
LFSR_WIDTH = 10

def decimator(clk, reset, dataIn, dataOut, decimationRatio, decimationRatioBase, decimationStyle_ext, dataClk, newValueFlag):
    """
    clk - input - clock
    reset - asynchronous reset
    dataIn - input - data to be decimated
    dataOut - output - decimated data
    decimationStyle - way of decimation: 00 - simple, 01 - dithering, 10 - miniMax, 11 - average
    decimationRatio - ratio of decimation
    decimationRatioBase - exponent with base of 2 (eg 2^decimationRatioBase = decimationRatio)
    dataClk - decimated data clock
    newValueFlag - flag of new value in decimationStyle register
    """
    bufferCounter = Signal(intbv(0, min = 0, max = MAXIMAL_RATIO))
    buff = [Signal(intbv(0)[BIT_WIDTH:]) for i in range(MAXIMAL_RATIO)]
    lfsr = Signal(intbv(0)[LFSR_WIDTH:])
    maxPeriod = Signal(bool(False))
    maxValue = Signal(intbv(0)[BIT_WIDTH:])
    minValue = Signal(intbv(0)[BIT_WIDTH:])
    #valueSum = Signal(intbv(0, min = 0, max = 255*2))
    dataOut_decimated = Signal(intbv(0)[8:])
    decimationStyle = Signal(intbv(0)[2:])
    metaCounter = Signal(intbv(0, min = 0, max = 9))
    flagRegistered = Signal(bool(False))
    decimationSum = Signal(intbv(0, min = 0, max = 256*MAXIMAL_RATIO))
    dataClkEdge = Signal(bool(False))
    
    @always(clk.posedge, reset.posedge)
    def newData():
        """This process registers flag indicating new data from
           lower clock domain and then waits 8 clock cycles to prevent
           data coccuprion caused by metastability of lower frequency registers"""
        if(reset == 1):
            metaCounter.next = 0
            flagRegistered.next = False
            decimationStyle.next = 0
        else:
            if(newValueFlag):
                metaCounter.next = 0
                flagRegistered.next = True
            else:
                if(flagRegistered):
                    if(metaCounter == 8):
                        decimationStyle.next = decimationStyle_ext
                        metaCounter.next = 0
                        flagRegistered.next = False
                    else:
                        metaCounter.next = metaCounter + 1
                else:
                    decimationStyle.next = decimationStyle
    
    
    @always(reset.posedge, clk.posedge)
    def lfsr_proc():
        """This process makes pseudorandom numbers utilizing LFSR 
        (http://en.wikipedia.org/wiki/Linear_feedback_shift_register)"""
        if(reset == 1):
            lfsr.next = LFSR_SEED
        else:
            if(dataClk):
                lfsr.next = concat(lfsr[LFSR_WIDTH-1:0], lfsr[9] ^ lfsr[6])
    
    @always(clk.posedge, reset.posedge)
    def bufferCnt():
        """This process counts up from 0 to decimationRatio creating pointer
           into buffer memory for saving samples in dithering mode of decimator"""
        if(reset == 1):
            bufferCounter.next = 0
        else:
            if(decimationRatio > 0):
                if(bufferCounter == (decimationRatio-1)):
                    bufferCounter.next = 0
                else:
                    bufferCounter.next = bufferCounter + 1
    
    @always(clk.posedge, reset.posedge)
    def outputConnect():
        """This process connects appropriate output 
        according to selected decimation ratio"""
        if(reset == 1):
            dataOut.next = 0
        else:
            if(decimationRatio == 1):
                dataOut.next = dataIn
            else:
                dataOut.next = dataOut_decimated
               
    @always(clk.posedge, reset.posedge)
    def output():
        """This is main process of decimator which on rising edge od data clock
           outputs decimated data according to selected decimation style
           for simple decimation it just pases current input date from adc,
           for dithering it takes random sample from decimated interval,
           for peak detection it takes maximum or minimum sample,
           for smoothing it makes mean out of decimated interval by shifting data right"""
        if(reset == 1):
            dataOut_decimated.next = 0
            maxPeriod.next = False
            maxValue.next = MIN_VALUE
            minValue.next = MAX_VALUE
            decimationSum.next = 0
            dataClkEdge.next = True
        else:
            if(dataClk == 1 and dataClkEdge == 1):
                dataClkEdge.next = False
                decimationSum[16:8].next = 0
                decimationSum[8:].next = dataIn
                maxValue.next = MIN_VALUE
                minValue.next = MAX_VALUE
                if(decimationRatio > 0):
                    if(decimationStyle == 0):
                        dataOut_decimated.next = dataIn
                    elif(decimationStyle == 1):
                        if(decimationRatio == 2):
                            dataOut_decimated.next = buff[lfsr[1:]]
                        elif(decimationRatio == 4):
                            dataOut_decimated.next = buff[lfsr[2:]]
                        elif(decimationRatio == 8):
                            dataOut_decimated.next = buff[lfsr[3:]]
                        elif(decimationRatio == 16):
                            dataOut_decimated.next = buff[lfsr[4:]]
                        elif(decimationRatio == 32):
                            dataOut_decimated.next = buff[lfsr[5:]]
                        elif(decimationRatio == 64):
                            dataOut_decimated.next = buff[lfsr[6:]]
                        elif(decimationRatio == 128):
                            dataOut_decimated.next = buff[lfsr[7:]]
                        elif(decimationRatio == 256):
                            dataOut_decimated.next = buff[lfsr[8:]]
                    elif(decimationStyle == 2):
                        maxPeriod.next = not maxPeriod
                        if(maxPeriod):
                            dataOut_decimated.next = maxValue
                        else:
                            dataOut_decimated.next = minValue
                    elif(decimationStyle == 3):
                        if(decimationRatioBase == 1):
                            dataOut_decimated.next = decimationSum[9:1]
                        elif(decimationRatioBase == 2):
                            dataOut_decimated.next = decimationSum[10:2]
                        elif(decimationRatioBase == 3):
                            dataOut_decimated.next = decimationSum[11:3]
                        elif(decimationRatioBase == 4):
                            dataOut_decimated.next = decimationSum[12:4]
                        elif(decimationRatioBase == 5):
                            dataOut_decimated.next = decimationSum[13:5]
                        elif(decimationRatioBase == 6):
                            dataOut_decimated.next = decimationSum[14:6]
                        elif(decimationRatioBase == 7):
                            dataOut_decimated.next = decimationSum[15:7]
                        elif(decimationRatioBase == 8):
                            dataOut_decimated.next = decimationSum[16:8]
            else:
                if(dataClk == 0):
                    dataClkEdge.next = True
                decimationSum.next = decimationSum + concat("00000000", dataIn)
                if(dataIn > maxValue):
                    maxValue.next = dataIn
                if(dataIn < minValue):
                    minValue.next = dataIn
    
    @always(clk.posedge)
    def fillBuffer():
        """This process fills in buffer for dithering mode of decimation"""
        buff[bufferCounter].next = dataIn

    return fillBuffer, lfsr_proc, output, bufferCnt, outputConnect, newData