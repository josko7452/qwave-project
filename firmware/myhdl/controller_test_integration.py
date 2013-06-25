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
from decimator import decimator
from buff import buff
from digipot_controller import digipot_controller
from dac_controller import dac_controller
from ft245sync import ft245sync
from trigger_analog import trigger_analog
from trigger_digital import trigger_digital

import sys
sys.setrecursionlimit(10000)

PERIOD = 6
PERIOD_FTDI = 16

HALF_PERIOD = PERIOD//2
HALF_PERIOD_FTDI = PERIOD_FTDI//2

LFSR_SEED = 468
LFSR_WIDTH = 10

def bench():
    """ Integration test for controller. """

    clock = Signal(bool(False))
    clockFtdi = Signal(bool(False))
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
    decimator1in = Signal(intbv(0)[8:])
    decimator2in = Signal(intbv(0)[8:])
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
    
    decimator1clockOut = Signal(bool(False))
    decimator2clockOut = Signal(bool(False))
    
    serialOut = Signal(bool(False))
    load = Signal(bool(False))
    ldac = Signal(bool(False))
    clkDacOut = Signal(bool(False))
    
    sda = TristateSignal(bool(False))
    scl = TristateSignal(bool(False))
    sda_d = sda.driver()
    scl_d = scl.driver()
    sda_dpull = sda.driver()
    scl_dpull = scl.driver()
    
    rxf = Signal(bool(False))
    txe = Signal(bool(False))
    rd = Signal(bool(False))
    wr = Signal(bool(False))
    siwu = Signal(bool(False))
    adbus = TristateSignal(intbv(0)[8:])
    
    lfsr = Signal(intbv(0)[LFSR_WIDTH:])
    adbus_drv = adbus.driver()
    data = [Signal(intbv(0)[8:]) for i in range(9)]
    
    dut_controller = controller(clockFtdi, reset, ft245din, ft245dout, ft245clock, ft245rw, ft245busy, ft245oe,
                   ft245dataWaitIn, ft245dataWaitOut, ft245strobe, ft245reset, dacReset, 
                   dacVrefTopA, dacVrefTopB, dacVrefBotA, dacVrefBotB, dacStrobe, dacBusy,
                   digipotReset, digipotValue, digipotStrobe, digipotBusy, bufferReset, 
                   bufferDataOut, bufferDataIn, bufferReady, bufferOutputInputReady, bufferclock,
                   bufferAllowWriteOverlap, bufferUseSdram, bufferDataDirection, decimator1clockOut,
                   decimationRatioBase, decimationStyle, decimator1out, decimator2out,
                   trigger1Type, trigger1Slope, analogTrigger1Reset, trigger1Value,
                   analog1Trigger, trigger2Type, trigger2Slope, analogTrigger2Reset,
                   trigger2Value, analog2Trigger, digitalTriggerReset, triggerPattern, 
                   triggerSamples, digiTrigger, adc1pd, adc2pd, adc1relatt, adc2relatt, 
                   adc1relco, adc2relco, led0, led1, logicAnalyzer, la1dir, la2dir, la1oe, la2oe)
               
    inst_decimator1 = decimator(clock, reset, decimator1in, decimator1out, decimationRatioBase, decimationStyle, decimator1clockOut)
    inst_decimator2 = decimator(clock, reset, decimator2in, decimator2out, decimationRatioBase, decimationStyle, decimator2clockOut)
    inst_trigger_analog1 = trigger_analog(decimator1clockOut, analogTrigger1Reset, decimator1out, 
                                        trigger1Type, trigger1Slope, trigger1Value, analog1Trigger)
    inst_trigger_analog2 = trigger_analog(decimator2clockOut, analogTrigger2Reset, decimator2out, 
                                        trigger2Type, trigger2Slope, trigger2Value, analog2Trigger)
    inst_trigger_digital = trigger_digital(clock, digitalTriggerReset, logicAnalyzer, triggerPattern, triggerSamples, digiTrigger)
    inst_dac_controller = dac_controller(clock, dacReset, dacVrefTopA, dacVrefTopB, dacVrefBotA, dacVrefBotB, dacStrobe, 
                                       serialOut, load, ldac, clkDacOut, dacBusy)
    inst_digipot_controller = digipot_controller(clock, digipotReset, digipotValue, digipotStrobe, digipotBusy, sda, scl)
    inst_ft245sync = ft245sync(clockFtdi, ft245reset, rxf, txe, rd, wr, ft245oe, siwu, adbus, ft245din, ft245dout, 
                               ft245rw, ft245busy, ft245dataWaitIn, ft245dataWaitOut, ft245strobe)
    inst_buffer = buff(bufferclock, bufferReset, bufferDataIn, bufferDataOut, bufferReady, bufferOutputInputReady, 
                        bufferAllowWriteOverlap, bufferUseSdram, bufferDataDirection)

    @instance
    def clkgen():
        clock.next = not clock
        yield delay(HALF_PERIOD)
    
    @instance
    def clkgen_f():
        clockFtdi.next = not clockFtdi
        yield delay(HALF_PERIOD_FTDI)
    
    @always(clock.posedge, reset.posedge)
    def lfsr_proc():
        if(reset == 1):
            lfsr.next = LFSR_SEED
        else:
            lfsr.next = concat(lfsr[LFSR_WIDTH-1:0], lfsr[9] ^ lfsr[6])
    
    @always_comb
    def adcGen():
        if(reset == 1):
            decimator1in.next = 0
            decimator2in.next = 0
        else:
            if(adc1pd):
                decimator1in.next = 0
            else:
                decimator1in.next = lfsr[10:2]
            if(adc2pd):
                decimator2in.next = 0
            else:
                decimator2in.next = lfsr[8:]
    
    @instance
    def stimulus():
        i = int(0)
        reset.next = True
        adbus_drv.next = None
        rxf.next = True
        txe.next = True
        sda_d.next = None
        scl_d.next = None
        sda_dpull.next = None
        scl_dpull.next = None
        yield delay(32)
        reset.next = False
        #set VREF
        yield ft245busy.negedge
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield clockFtdi.posedge
        yield (ft245oe.negedge)
        print ("OE came low, pushing data from FTDI")
        adbus_drv.next = 0x10
        i = 0
        data[0].next = 100
        data[1].next = 80
        data[2].next = 150
        data[3].next = 40
        data[4].next = 0
        data[5].next = 0
        data[6].next = 0
        data[7].next = 0
        data[8].next = 0
        while(rxf == 0):
            yield clockFtdi.negedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.posedge
        yield clockFtdi.negedge
        adbus_drv.next = None
        rxf.next = True
        if(ft245busy != 0):
            print ("Error: Busy should be low")
            yield ft245busy.negedge
        #SET decimation
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield clockFtdi.posedge
        yield (ft245oe.negedge)
        print ("OE came low, pushing data from FTDI")
        adbus_drv.next = 0x30
        i = 0
        data[0].next = 0x00
        data[1].next = 0x00
        data[2].next = 0
        data[3].next = 0
        data[4].next = 0
        data[5].next = 0
        data[6].next = 0
        data[7].next = 0
        data[8].next = 0
        while(rxf == 0):
            yield clockFtdi.negedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.posedge
        yield clockFtdi.negedge
        adbus_drv.next = None
        rxf.next = True
        if(ft245busy != 0):
            print ("Error: Busy should be low")
            yield ft245busy.negedge
        #SET attenuators
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield clockFtdi.posedge
        yield (ft245oe.negedge)
        print ("OE came low, pushing data from FTDI")
        adbus_drv.next = 0x20
        i = 0
        data[0].next = 0xFF
        data[1].next = 0xFF
        data[2].next = 0
        data[3].next = 0
        data[4].next = 0
        data[5].next = 0
        data[6].next = 0
        data[7].next = 0
        data[8].next = 0
        while(rxf == 0):
            yield clockFtdi.negedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.posedge
        yield clockFtdi.negedge
        adbus_drv.next = None
        rxf.next = True
        if(ft245busy != 0):
            print ("Error: Busy should be low")
            yield ft245busy.negedge
        
        #SET trigger
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield clockFtdi.posedge
        yield (ft245oe.negedge)
        print ("OE came low, pushing data from FTDI")
        adbus_drv.next = 0x31
        i = 0
        data[0].next = 120
        data[1].next = 0
        data[2].next = 145
        data[3].next = 0
        data[4].next = 0
        data[5].next = 0
        data[6].next = 0
        data[7].next = 0
        data[8].next = 0
        while(rxf == 0):
            yield clockFtdi.negedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.posedge
        yield clockFtdi.negedge
        rxf.next = True
        adbus_drv.next = None
        if(ft245busy != 0):
            print ("Error: Busy should be low")
            yield ft245busy.negedge
        #Start capture
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield clockFtdi.posedge
        yield (ft245oe.negedge)
        print ("OE came low, pushing data from FTDI")
        adbus_drv.next = 0x40
        i = 0
        data[0].next = 0xF0
        data[1].next = 0
        data[2].next = 0
        data[3].next = 0
        data[4].next = 0
        data[5].next = 0
        data[6].next = 0
        data[7].next = 0
        data[8].next = 0
        while(rxf == 0):
            yield clockFtdi.negedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.posedge
        yield clockFtdi.negedge
        rxf.next = True
        adbus_drv.next = None
        if(ft245busy != 0):
            print ("Error: Busy should be low")
            yield ft245busy.negedge
        yield bufferDataDirection.negedge
        yield ft245busy.posedge
        txe.next = False
        yield clockFtdi.posedge
        for i in range(0, 100000):
            if(ft245dataWaitOut):
                print("Data wait out")
            print("Data:")
            print(adbus)
            yield clockFtdi.posedge
            if(bufferReady == 0):
                break
        
        raise StopSimulation

    return inst_buffer, inst_ft245sync, inst_digipot_controller, inst_dac_controller, inst_trigger_digital, lfsr_proc, adcGen, \
           inst_trigger_analog2, inst_trigger_analog1, inst_decimator2, inst_decimator1, dut_controller, clkgen, clkgen_f, stimulus
           
    
#tb_cntrl = traceSignals(bench)    
simulation = Simulation(bench())
simulation.run()

toVHDL(bench)