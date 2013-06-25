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

from controller import controller


PERIOD = 6

def bench():
    """ Unit test for controller. """
    
    clock = Signal(bool(False))
    reset = Signal(bool(False))
    
    ft245din = Signal(intbv(0)[8:])
    ft245dout = Signal(intbv(0)[8:])
    ft245rw = Signal(bool(False))
    ft245clock = Signal(bool(False))
    ft245reset = Signal(bool(False))
    ft245busy = Signal(bool(False))
    ft245oe = Signal(bool(False))
    ft245dataWaitIn = Signal(bool(False))
    ft245dataWaitOut = Signal(bool(False))
    ft245strobe = Signal(bool(False))
    dacReset = Signal(bool(False))
    dacBusy = Signal(bool(False))
    dacStrobe = Signal(bool(False))
    dacVrefTopA = Signal(intbv(0)[8:])
    dacVrefTopB = Signal(intbv(0)[8:])
    dacVrefBotA = Signal(intbv(0)[8:])
    dacVrefBotB = Signal(intbv(0)[8:])
    digipotValue = Signal(intbv(0)[7:])
    digipotReset = Signal(bool(False))
    digipotBusy = Signal(bool(False))
    digipotStrobe = Signal(bool(False))
    bufferclock = Signal(bool(False))
    bufferReset = Signal(bool(False))
    bufferDataIn = Signal(intbv(0)[16:])
    bufferDataOut = Signal(intbv(0)[16:])
    bufferReady = Signal(bool(False))
    bufferOutputInputReady = Signal(bool(False))
    bufferAllowWriteOverlap = Signal(bool(False))
    bufferUseSdram = Signal(bool(False))
    bufferDataDirection = Signal(bool(False))
    decimatorclockOut = Signal(bool(False))
    decimationRatioBase = Signal(intbv(0, min = 0, max = 11))
    decimationStyle = Signal(intbv(0)[2:])
    analogTrigger1Reset = Signal(bool(False))
    trigger1Type = Signal(bool(False))
    trigger1Slope = Signal(bool(False))
    analog1Trigger = Signal(bool(False))
    trigger1Value = Signal(intbv(0)[8:])
    analogTrigger2Reset = Signal(bool(False))
    trigger2Type = Signal(bool(False))
    trigger2Slope = Signal(bool(False))
    analog2Trigger = Signal(bool(False))
    trigger2Value = Signal(intbv(0)[8:])
    digitalTriggerReset = Signal(bool(False))
    digiTrigger = Signal(bool(False))
    triggerPattern = Signal(intbv(0)[64:])
    triggerSamples = Signal(intbv(0, min = 0, max = 4))
    decimator1out = Signal(intbv(0)[8:])
    decimator2out = Signal(intbv(0)[8:])
    adc1pd = Signal(bool(False))
    adc2pd = Signal(bool(False))
    adc1relatt = Signal(bool(False))
    adc2relatt = Signal(bool(False))
    adc1relco = Signal(bool(False))
    adc2relco = Signal(bool(False))
    led0 = Signal(bool(False))
    led1 = Signal(bool(False))
    logicAnalyzer = Signal(intbv(0)[16:])
    la1dir = Signal(bool(False))
    la2dir = Signal(bool(False))
    la1oe  = Signal(bool(False))
    la2oe  = Signal(bool(False))
    
    dut_controller = controller(clock, reset, ft245din, ft245dout, ft245clock, ft245rw, ft245busy, ft245oe,
                   ft245dataWaitIn, ft245dataWaitOut, ft245strobe, ft245reset, dacReset, 
                   dacVrefTopA, dacVrefTopB, dacVrefBotA, dacVrefBotB, dacStrobe, dacBusy,
                   digipotReset, digipotValue, digipotStrobe, digipotBusy, bufferReset, 
                   bufferDataOut, bufferDataIn, bufferReady, bufferOutputInputReady, bufferclock,
                   bufferAllowWriteOverlap, bufferUseSdram, bufferDataDirection, decimatorclockOut,
                   decimationRatioBase, decimationStyle, decimator1out, decimator2out,
                   trigger1Type, trigger1Slope, analogTrigger1Reset, trigger1Value,
                   analog1Trigger, trigger2Type, trigger2Slope, analogTrigger2Reset,
                   trigger2Value, analog2Trigger, digitalTriggerReset, triggerPattern, 
                   triggerSamples, digiTrigger, adc1pd, adc2pd, adc1relatt, adc2relatt, 
                   adc1relco, adc2relco, led0, led1, logicAnalyzer, la1dir, la2dir, la1oe, la2oe)
        
    @always(delay(PERIOD//2))
    def clockgen():
        clock.next = not clock

    @instance
    def stimulus():
        reset.next = True
        digipotBusy.next = True
        dacBusy.next = True
        ft245oe.next = True
        yield delay(100)
        reset.next = False
        yield delay(100)
        digipotBusy.next = False
        dacBusy.next = False
        yield delay(300)
        ft245oe.next = False
        ft245busy.next = True
        yield clock.negedge
        ft245dout.next = 0x31
        yield clock.negedge
        ft245dout.next = 131
        yield clock.negedge
        nextValue = 0x32
        ft245dout.next = (nextValue >> 2) | 3
        yield clock.negedge
        ft245dout.next = ((nextValue << 6) | 2) % 255
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245oe.next = True
        ft245busy.next = False
        yield delay(300)
        ft245oe.next = False
        ft245busy.next = True
        yield clock.negedge
        ft245dout.next = 0x40
        yield clock.negedge
        ft245dout.next = 3
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245oe.next = True
        ft245busy.next = False
        yield delay(100)
        bufferReady.next = True
        yield delay(400)
        analog1Trigger.next = True
        yield delay(300)
        bufferReady.next = False
        yield delay(100)
        bufferReady.next = True
        ft245busy.next = True
        yield delay(100)
        ft245busy.next = False
        ft245dataWaitOut.next = True
        yield delay(50)
        ft245busy.next = True
        ft245dataWaitOut.next = False
        yield delay(300)
        bufferReady.next = False
        yield delay(100)
        #reset
        ft245oe.next = False
        ft245busy.next = True
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245oe.next = True
        ft245busy.next = False
        yield delay(100)
        #VREF
        ft245oe.next = False
        ft245busy.next = True
        yield clock.negedge
        ft245dout.next = 0x10
        yield clock.negedge
        ft245dout.next = 100
        yield clock.negedge
        ft245dout.next = 80
        yield clock.negedge
        ft245dout.next = 150
        yield clock.negedge
        ft245dout.next = 40
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245oe.next = True
        ft245busy.next = False
        yield delay(100)
        dacBusy.next = True
        yield delay(30)
        dacBusy.next = False
        #VLOG
        ft245oe.next = False
        ft245busy.next = True
        yield clock.negedge
        ft245dout.next = 0x11
        yield clock.negedge
        ft245dout.next = 0xEF
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245oe.next = True
        ft245busy.next = False
        yield delay(100)
        digipotBusy.next = True
        yield delay(30)
        digipotBusy.next = False
        #ATTENUATORS
        ft245oe.next = False
        ft245busy.next = True
        yield clock.negedge
        ft245dout.next = 0x20
        yield clock.negedge
        ft245dout.next = 0xFF
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245oe.next = True
        ft245busy.next = False
        yield delay(100)
        #COUPLINGS
        ft245oe.next = False
        ft245busy.next = True
        yield clock.negedge
        ft245dout.next = 0x21
        yield clock.negedge
        ft245dout.next = 0xFF
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245oe.next = True
        ft245busy.next = False
        yield delay(100)
        #DECIMATION
        ft245oe.next = False
        ft245busy.next = True
        yield clock.negedge
        ft245dout.next = 0x30
        yield clock.negedge
        ft245dout.next = 0x1E
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245oe.next = True
        ft245busy.next = False
        yield delay(100)
        #DIGITAL TRIGGER
        ft245oe.next = False
        ft245busy.next = True
        yield clock.negedge
        ft245dout.next = 0x32
        yield clock.negedge
        ft245dout.next = 0xEF
        yield clock.negedge
        ft245dout.next = 0xEE
        yield clock.negedge
        ft245dout.next = 0xAB
        yield clock.negedge
        ft245dout.next = 0x5A
        yield clock.negedge
        ft245dout.next = 0xCB
        yield clock.negedge
        ft245dout.next = 0xA3
        yield clock.negedge
        ft245dout.next = 0x5B
        yield clock.negedge
        ft245dout.next = 0xAA
        yield clock.negedge
        ft245dout.next = 0xFF
        yield clock.negedge
        ft245oe.next = True
        ft245busy.next = False
        yield delay(100)
        #Ram usage
        ft245oe.next = False
        ft245busy.next = True
        yield clock.negedge
        ft245dout.next = 0x33
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245dout.next = 0x00
        yield clock.negedge
        ft245oe.next = True
        ft245busy.next = False
        yield delay(100)
        
        yield delay(2000)
        raise StopSimulation

    return dut_controller, clockgen, stimulus
    
tb_cntrl = traceSignals(bench)    
simulation = Simulation(tb_cntrl)
simulation.run()

toVerilog(bench)
toVHDL(bench)