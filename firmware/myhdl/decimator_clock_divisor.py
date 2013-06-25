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

MAXIMAL_RATIO = 2**8

def decimator_clock_divisor(clk, reset, decimationRatioBase, decimationRatio, clkDataOut, newValueFlag, alwaysEn):
    """
    clk - input - clock
    reset - asynchronous reset
    decimationRatioBase - base of oversamplig ratio i.e. 2^0, 2^1, 2^2 etc..
    
    """
    clkDataOut_int = Signal(bool(False))
    counter = Signal(intbv(0, min = 0, max = MAXIMAL_RATIO/2))
    metaCounter = Signal(intbv(0, min = 0, max = 9))
    flagRegistered = Signal(bool(False))
    
    @always(clk.posedge, reset.posedge)
    def decimationRatioConv():
        """This process registers flag indicating new data from
           lower clock domain and then waits 8 clock cycles to prevent
           data coccuprion caused by metastability of lower frequency registers,
           then it updates decimationRatio out of exponent value stored in decimationRatioBase
           where decimationRatio = 2^deimationRatioBase"""
        if(reset == 1):
            metaCounter.next = 0
            flagRegistered.next = False
            decimationRatio.next = 1
        else:
            if(newValueFlag):
                metaCounter.next = 0
                flagRegistered.next = True
            else:
                if(flagRegistered):
                    if(metaCounter == 8):
                        if(decimationRatioBase == 0):
                            decimationRatio.next = 1
                        elif(decimationRatioBase == 1):
                            decimationRatio.next = 2
                        elif(decimationRatioBase == 2):
                            decimationRatio.next = 4
                        elif(decimationRatioBase == 3):
                            decimationRatio.next = 8
                        elif(decimationRatioBase == 4):
                            decimationRatio.next = 16
                        elif(decimationRatioBase == 5):
                            decimationRatio.next = 32
                        elif(decimationRatioBase == 6):
                            decimationRatio.next = 64
                        elif(decimationRatioBase == 7):
                            decimationRatio.next = 128
                        else:
                            decimationRatio.next = 256
                        metaCounter.next = 0
                        flagRegistered.next = False
                    else:
                        metaCounter.next = metaCounter + 1
                else:
                    decimationRatio.next = decimationRatio
    
    @always(clk.posedge, reset.posedge)
    def clockDivisor():
        """Divides ADC clock according to selected decimation ratio"""
        if(reset == 1):
            counter.next = 0
            clkDataOut_int.next = False
        else:
            if(decimationRatio == 1):
                counter.next = 0
                clkDataOut_int.next = True
            else:
                if(counter == ((decimationRatio>>1)-1)):
                    counter.next = 0
                    clkDataOut_int.next = not clkDataOut_int
                else:
                    counter.next = counter + 1
    
    @always_comb
    def enableOutClk():
        """Output clock is enabled only when decimation ratio is higher than 1:1"""
        if(decimationRatio == 1):
            clkDataOut.next = True
            alwaysEn.next = True
        else:
            alwaysEn.next = False
            clkDataOut.next = clkDataOut_int
    
    return clockDivisor, enableOutClk, decimationRatioConv