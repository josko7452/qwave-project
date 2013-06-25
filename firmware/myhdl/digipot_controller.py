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

tPotStates = enum('INIT', 'IDLE', 'WRITE1', 'WRITE2', 'WRITE3', 'WRITE4')

DEFAULT_POT_ADDRESS = int('0101111', 2)
DATA_WORD_MAX = 2**8

def digipot_controller(clk, reset, value, strobe, busy, sda, scl):
    """
    ckl - clock input
    reset - asynchornous reset
    value - input of desired pot value
    strobe - strobe of value
    busy - busy indicator
    sda - i2c sda wire
    scl - i2c scl wire
    """
    DIVISION = 640
    counter = Signal(intbv(0, min = 0, max = DIVISION/2))
    oneHundredKhzClock = Signal(bool(False))
    delayedClock = Signal(bool(False))
    delayedClock_int = Signal(bool(False))
    clkEn = Signal(bool(False))
    repeatTransfer = Signal(bool(False))
    generateStart = Signal(bool(False))
    generateStop = Signal(bool(False))
    sda_i = Signal(bool(False))
    sda_neg = Signal(bool(False))
    sclIdleValue = Signal(bool(True))
    dataWord = Signal(intbv(0)[8:])
    txCounter = Signal(intbv(0, min = 0, max = 15))
    state = Signal(tPotStates.INIT)
    valueInt  = Signal(intbv(0)[7:])
    secondByte = Signal(bool(False)) 
    
    sda_d = sda.driver()
    scl_d = scl.driver()
    
    @always_comb
    def sda_nega():
        """This process selects if output of SDA should be 
           in Hi-Z (sda_neg HIGH) or LOW(sda_neg LOW)"""
        if(generateStart):
            sda_neg.next = oneHundredKhzClock
        elif(generateStop):
            sda_neg.next = not oneHundredKhzClock
        else:
            sda_neg.next = sda_i
    
    @always_comb
    def sda_con():
        """This process outputs SDA to High-Z or
           LOW according to sda_neg"""
        if(not sda_neg):
            sda_d.next = False
        else:
            sda_d.next = None
    
    @always(delayedClock.posedge, reset.posedge)
    def connectClock():
        """This process drives scl with delayedClock_int negation or sclIdleValue according to clkEn
           high signal state is translated to High-Z"""
        if(reset == 1):
            delayedClock_int.next = True
        else:
            delayedClock_int.next = not delayedClock_int
            if(clkEn):
                if(delayedClock_int):
                    scl_d.next = False
                else:
                    scl_d.next = None
            else:
                if(sclIdleValue):
                    scl_d.next = None
                else:
                    scl_d.next = False
    
    @always(clk.posedge, reset.posedge)
    def clockDivisor():
        """This process divides clock to 100kHz used by I2C, it also generted half cycle delayed clock (delayedClock) on 200kHz
           which is then divided to correct 100kHz frequency in connectClock process"""
        if(reset == 1):
            counter.next = 0
            oneHundredKhzClock.next = False
            delayedClock.next = False
        else:
            if((counter == (DIVISION/4)-1) or
               (counter == (DIVISION/2)-1) or
               (counter == ((DIVISION/2)+(DIVISION/4))-1)):
                delayedClock.next = not delayedClock
            if(counter == (DIVISION/2)-1):
                counter.next = 0
                oneHundredKhzClock.next = not oneHundredKhzClock
            else:
                counter.next = counter + 1
                    
    @always(oneHundredKhzClock.posedge, reset.posedge)
    def fsm():
        """This is process of FSM. In INIT state it generates reset sequence for digital pot
           in IDLE state it waits for data strobe, then it transmits data on I2C bus to digital pot.
           In WRITE1 it generates start condition, then in WRITE2 it first transmits
           address of digipot device and then in WRITE3 waits for acknowledge and generate stop condition, 
           in WRITE4 it transmits actual pot setting data then goes back to WRITE3 waiting for ack and generate stop
           and then goes back to IDLE state."""
        if(reset == 1):
            secondByte.next = False
            clkEn.next = False
            busy.next = True
            state.next = tPotStates.INIT
            sda_i.next = True
            sclIdleValue.next = 1
            txCounter.next = 0
            valueInt.next = 0
            dataWord.next = 0
            generateStart.next = False
            generateStop.next = False
            repeatTransfer.next = False
        else:
            if(state == tPotStates.INIT):
                if(txCounter == 0):
                    clkEn.next = True
                    txCounter.next = txCounter + 1
                    generateStart.next = True
                elif(txCounter < 10):
                    generateStart.next = False
                    txCounter.next = txCounter + 1
                    sda_i.next = True
                elif(txCounter == 10):
                    txCounter.next = txCounter + 1
                    clkEn.next = False
                    sclIdleValue.next = 1
                elif(txCounter == 11):
                    txCounter.next = txCounter + 1
                    sda_i.next = False
                    sclIdleValue.next = 0
                elif(txCounter == 12):
                    sclIdleValue.next = 1
                    #generateStop.next = True
                    state.next = tPotStates.IDLE
                    txCounter.next = 0
            elif(state == tPotStates.IDLE):
                generateStart.next = False
                generateStop.next = False
                sda_i.next = True
                sclIdleValue.next = 1
                clkEn.next = False
                txCounter.next = 0
                if(strobe == 1):
                    state.next = tPotStates.WRITE1
                    busy.next = True
                    valueInt.next = value
                else:
                    if(repeatTransfer):
                        repeatTransfer.next = False
                        state.next = tPotStates.WRITE1
                        busy.next = True
                    else:
                        state.next = tPotStates.IDLE
                        busy.next = False
            elif(state == tPotStates.WRITE1):
                dataWord.next[8:1] = DEFAULT_POT_ADDRESS
                dataWord.next[0] = 0
                clkEn.next = True
                generateStart.next = True
                state.next = tPotStates.WRITE2
            elif(state == tPotStates.WRITE2):
                generateStart.next = False
                if(txCounter < 8):
                    sda_i.next = dataWord[7]
                    txCounter.next = txCounter + 1
                    dataWord.next = (dataWord << 1) % DATA_WORD_MAX
                    state.next = tPotStates.WRITE2
                else:
                    secondByte.next = False
                    state.next = tPotStates.WRITE3
                    sda_i.next = True
            elif(state == tPotStates.WRITE3):
                #if(sda == 0):
                    if(secondByte):
                        state.next = tPotStates.IDLE
                        sclIdleValue.next = True
                        generateStop.next = True
                        clkEn.next = False
                    else:
                        state.next = tPotStates.WRITE4
                        sda_i.next = False
                        dataWord.next[8:1] = valueInt
                        dataWord.next[0] = 0
                        txCounter.next = 0
                #else:
                #    state.next = tPotStates.INIT
                #    clkEn.next = False
                #    busy.next = True
                #    sda_i.next = True
                #    sclIdleValue.next = 1
                #    txCounter.next = 0
                #    repeatTransfer.next = True
                    #valueInt.next = 0
            elif(state == tPotStates.WRITE4):
                if(txCounter < 7):
                    sda_i.next = dataWord[7]
                    txCounter.next = txCounter + 1
                    dataWord.next = (dataWord << 1) % DATA_WORD_MAX
                    state.next = tPotStates.WRITE4
                else:
                    secondByte.next = True
                    state.next = tPotStates.WRITE3
                    sda_i.next = True
            else:
                clkEn.next = False
                busy.next = True
                state.next = tPotStates.INIT
                sda_i.next = True
                sclIdleValue.next = 1
                txCounter.next = 0
    
    return connectClock, clockDivisor, fsm, sda_con, sda_nega
                
                    
                