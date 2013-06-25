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

def trigger_digital(clk, reset, dataIn, pattern, samples, trigger, inputEnable, alwaysEn):
    """
    simple logic analyzer trigger.
    clk - clock
    reset - asynchronous reset
    dataIn - input data
    pattern - data pattern
    samples - number of samples to compare (1 .. 4)
    inputEnable - clock of data coming from decimator, on which frequency data is checked in trigger
    alwaysEn - use clk instead of inputEnable (used when decmiation is set to 1:1)
    """
    
    previousValues = [Signal(intbv(0)[16:]) for i in range(4)]
    clkEn = Signal(bool(False))
    
    @always(clk.posedge, reset.posedge)
    def triggering():
        """This process defines triggering according to selected sample number and pattern"""
        if(reset == 1):
            trigger.next = False
            previousValues[0].next = 0
            previousValues[1].next = 0
            previousValues[2].next = 0
        else:
            if(not inputEnable):
                clkEn.next = True
            if(inputEnable and (clkEn or alwaysEn)):
                clkEn.next = False
                previousValues[0].next = dataIn
                previousValues[1].next = previousValues[0]
                previousValues[2].next = previousValues[1]
                if(samples == 0): #compare one sample
                    if(pattern[16:] == dataIn):
                        trigger.next = True
                    else:
                        trigger.next = False
                elif(samples == 1): #compare two samples
                    if((pattern[16:] == dataIn) and (pattern[32:16] == previousValues[0])):
                        trigger.next = True
                    else:
                        trigger.next = False
                elif(samples == 2):
                    if((pattern[16:] == dataIn) and 
                       (pattern[32:16] == previousValues[0]) and
                       (pattern[48:32] == previousValues[1])):
                        trigger.next = True
                    else:
                        trigger.next = False
                elif(samples == 3):
                    if((pattern[16:] == dataIn) and 
                       (pattern[32:16] == previousValues[0]) and
                       (pattern[48:32] == previousValues[1]) and
                       (pattern[64:48] == previousValues[2])):
                        trigger.next = True
                    else:
                        trigger.next = False
    
    return triggering