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

def digital_decimator(clk, reset, dataIn, dataOut, decimationRatio, dataClk):
    """
    clk - input - clock
    reset - asynchronous reset
    dataIn - input - data to be decimated
    dataOut - output - decimated data
    decimationRatioBase - base of oversamplig ratio i.e. 2^0, 2^1, 2^2 etc..
    dataClk - decimated data clock
    """
    outputReady = Signal(bool(False))
    clkDataOut_int = Signal(bool(False))
    dataOut_decimated = Signal(intbv(0)[16:])
     
    @always(clk.posedge, reset.posedge)
    def outputConnect():
        """Connects otput straight if decimation is set to 1:1"""
        if(reset == 1):
            dataOut.next = 0
        else:
            if(decimationRatio == 1):
                dataOut.next = dataIn
            else:
                dataOut.next = dataOut_decimated
               
    @always(dataClk.posedge, reset.posedge)
    def output():
        """Decimated digital data by always taking first sample of decimated interval"""
        if(reset == 1):
            dataOut_decimated.next = 0
        else:
            if(dataClk):
                dataOut_decimated.next = dataIn
            
    return output, outputConnect