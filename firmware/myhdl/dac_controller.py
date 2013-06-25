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

tDacStates = enum('IDLE', 'WRITE_PREPARE', 'WRITE0', 'WRITE1')
OUTPUT_WORD_MAX = 2**11

def dac_controller(clk, reset, vrefTopA, vrefTopB, vrefBotA, vrefBotB, strobe, 
                   serialOut, load, ldac, clkDacOut, busy):
    """
    ckl - clock input
    vrefTopA - parellel vrefTopA value
    vrefTopB - parellel vrefTopB value
    vrefBotA - parellel vrefBotA value
    vrefBotA - parellel vrefBotB value
    strobe - strobe parellel values
    serialOut - respective serialized output do dac
    load - load output for DAC
    ldac - ldac output for DAC
    clkDacOut - clk for DAC
    busy - output if controller is busy
    """
    """
    DACA - ADC2_VRB_DAC
    DACB - ADC2_VRT_DAC
    DACC - ADC1_VRT_DAC
    DACD - ADC1_VRB_DAC
    """
    
    DIVISION = 120
    counter = Signal(intbv(0, min = 0, max = DIVISION/2))
    halfMhzClock = Signal(bool(False))
    halfMhzClock_int = Signal(bool(False))
    state = Signal(tDacStates.IDLE)
    clkEn = Signal(bool(False))
    
    vrefTopAint = Signal(intbv(0)[8:])
    vrefTopBint = Signal(intbv(0)[8:])
    vrefBotAint = Signal(intbv(0)[8:])
    vrefBotBint = Signal(intbv(0)[8:])
    outputWord = Signal(intbv(0)[11:])
    address = Signal(intbv(0)[2:])
    shiftCounter = Signal(intbv(0, min = 0, max = 12))
    
    @always(clk.posedge, reset.posedge)
    def clockDivisor():
        """This process divides input 60MHz clock to internal 
           500Khz clock used to comuniate with DAC"""
        if(reset == 1):
            counter.next = 0
            halfMhzClock.next = False
        else:
            if(counter == (DIVISION/2)-1):
                counter.next = 0
                halfMhzClock.next = not halfMhzClock
            else:
                counter.next = counter + 1
              
    @always_comb
    def connectClock():
        """This process connects internal clock to DAC only when sending commads"""
        if(clkEn):
            clkDacOut.next = halfMhzClock
        else:
            clkDacOut.next = False
    
    
    @always(halfMhzClock.posedge, reset.posedge)
    def fsm():
        """This process is control FSM of DAC controller,
           in IDLE states it senses STROBE signal going HIGH and
           checks reference voltage values if bottom reference voltage is not set
           higher than top reference votage and then transitions to state WRITE_PREPARE
           in which sets output word transmitted to DAC according to current address,
           after all four words were trasmitted, controller sets DAC to update all outputs"""
        if(reset == 1):
            vrefTopAint.next = 100
            vrefBotAint.next = 0
            vrefTopBint.next = 100
            vrefBotBint.next = 0
            load.next = True
            ldac.next = True
            busy.next = True
            address.next = 0
            state.next = tDacStates.IDLE
            serialOut.next = False
            shiftCounter.next = 0
            clkEn.next = False
            outputWord.next = 0
        else:
            if(state == tDacStates.IDLE):
                load.next = True
                ldac.next = True
                if(strobe == 1):
                    state.next = tDacStates.WRITE_PREPARE
                    busy.next = True
                    address.next = 0
                    shiftCounter.next = 0
                    if(vrefTopA >= vrefBotA):
                        vrefTopAint.next = vrefTopA
                        vrefBotAint.next = vrefBotA
                    if(vrefTopB >= vrefBotB):
                        vrefTopBint.next = vrefTopB
                        vrefBotBint.next = vrefBotB
                else:
                    state.next = tDacStates.IDLE
                    busy.next = False
            elif(state == tDacStates.WRITE_PREPARE):
                load.next = True
                ldac.next = True
                outputWord.next[11:9] = address
                outputWord.next[8] = 1
                if(address == 0):
                    outputWord.next[8:] = vrefBotBint
                elif(address == 1):
                    outputWord.next[8:] = vrefTopBint
                elif(address == 2):
                    outputWord.next[8:] = vrefTopAint
                else:
                    outputWord.next[8:] = vrefBotAint
                state.next = tDacStates.WRITE0
            elif(state == tDacStates.WRITE0):
                if(shiftCounter < 11):
                    clkEn.next = True
                    serialOut.next = outputWord[10]
                    outputWord.next = (outputWord << 1) % OUTPUT_WORD_MAX
                    shiftCounter.next = shiftCounter + 1
                    state.next = tDacStates.WRITE0
                else:
                    shiftCounter.next = 0
                    state.next = tDacStates.WRITE1
                    clkEn.next = False
                    load.next = False
                    load.next = False
            elif(state == tDacStates.WRITE1):
                if(address == 3):
                    ldac.next = False
                    load.next = True
                    state.next = tDacStates.IDLE
                else:
                    load.next = True
                    state.next = tDacStates.WRITE_PREPARE
                    address.next = address + 1
    
    return clockDivisor, connectClock, fsm