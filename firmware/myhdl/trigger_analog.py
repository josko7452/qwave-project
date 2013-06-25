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

def trigger_analog(clk, reset, dataIn, triggerType, slope, value, trigger, inputEnable, alwaysEn):
    """
    Simple analog trigger.
    clk - clock
    reset - asynchronous reset
    dataIn - data to trigger
    triggerType - 0 threshold, 1 edge
    slope - 0 falling, 1 rising
    trigger - 1 triggered / 0 not triggered
    value - value to be triggered on
    inputEnable - clock of data coming from decimator, on which frequency data is checked in trigger
    alwaysEn - use clk instead of inputEnable (used when decmiation is set to 1:1)
    """
    #previousValues = [Signal(intbv(0)[8:]) for i in range(4)]
    previousValue = Signal(intbv(0)[8:])
    valuesCompared = Signal(intbv(0, min = 0, max = 4))
    clkEn = Signal(bool(False))
    
    @always(clk.posedge, reset.posedge)
    def triggering():
        """This process defines triggering. Trigger triggers according to selected trigger mode 
           /threshold or edge/ and trigger slope /rising or falling/"""
        if(reset == 1):
            trigger.next = False
            previousValue.next = 0
            valuesCompared.next = 0
        else:
            if(not inputEnable):
                clkEn.next = True
            if(inputEnable and (clkEn or alwaysEn)):
                clkEn.next = False
                previousValue.next = dataIn
                if(triggerType == 1): #edge trigger
                    if(slope == 1): #rising edge
                        if(previousValue <= dataIn):
                            if(valuesCompared == 3):
                                valuesCompared.next = 3
                            else:
                                valuesCompared.next = valuesCompared + 1
                        if(value <= dataIn and valuesCompared == 3):
                            trigger.next = True
                        else:
                            trigger.next = False
                    else: #falling edge
                        if(previousValue >= dataIn):
                            if(valuesCompared == 3):
                                valuesCompared.next = 3
                            else:
                                valuesCompared.next = valuesCompared + 1
                        if(value >= dataIn and valuesCompared == 3):
                            trigger.next = True
                        else:
                            trigger.next = False
                else: #threshold trigger
                    if(slope == 1): #rising
                        if(value <= dataIn):
                            trigger.next = True
                        else:
                            trigger.next = False
                    else: #falling
                        if(value >= dataIn):
                            trigger.next = True
                        else:
                            trigger.next = False
    
    return triggering
                        
                    
                           