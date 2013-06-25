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
from digital_decimator import digital_decimator
from buff import buff
from digipot_controller import digipot_controller
from dac_controller import dac_controller
from ft245sync import ft245sync
from trigger_analog import trigger_analog
from trigger_digital import trigger_digital
from decimator_clock_divisor import decimator_clock_divisor

import sys
sys.setrecursionlimit(10000)

PERIOD = 6
PERIOD_FTDI = 16

HALF_PERIOD = PERIOD//2
HALF_PERIOD_FTDI = PERIOD_FTDI//2

LFSR_SEED = 468
LFSR_WIDTH = 10

MAXIMAL_RATIO = 2**8

#include or exclude testbench
INCLUDE_TESTBENCH = True

def convert_modules():
    clock = Signal(bool(False))
    clockFtdi = Signal(bool(False))
    reset = Signal(bool(False))
    
    decimatorNewValueFlag = Signal(bool(False))
    ft245din = Signal(intbv(0)[8:])
    ft245dout = Signal(intbv(0)[8:])
    ft245rw = Signal(bool(False))
    ft245clock = Signal(bool(False))
    ft245reset = Signal(bool(False))
    ft245busy = Signal(bool(False))
    ft245oe = Signal(bool(False))
    ft245wr = Signal(bool(False))
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
    bufferDataIn = Signal(intbv(0)[32:])
    bufferDataOut = Signal(intbv(0)[32:])
    bufferReady = Signal(bool(False))
    bufferOutputInputReady = Signal(bool(False))
    bufferAllowWriteOverlap = Signal(bool(False))
    bufferUseSdram = Signal(bool(False))
    bufferDataDirection = Signal(bool(False))
    decimatorclockOut = Signal(bool(False))
    decimationRatioBase = Signal(intbv(0, min = 0, max = 9))
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
    logicAnalyzer_dec = Signal(intbv(0)[16:])
    la1dir = Signal(bool(False))
    la2dir = Signal(bool(False))
    la1oe  = Signal(bool(False))
    la2oe  = Signal(bool(False))
    decimationRatio = Signal(intbv(0, min = 0, max = MAXIMAL_RATIO+1))
    alwaysEn  = Signal(bool(False))
    
    decimator1clockOut = Signal(bool(False))
    decimator2clockOut = Signal(bool(False))
    decimatorDigClockOut = Signal(bool(False))
    
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
    
    print("Converting decimator_clock_divisor")
    toVHDL(decimator_clock_divisor, clock, reset, decimationRatioBase, decimationRatio, decimator2clockOut, decimatorNewValueFlag, alwaysEn)
    print("Converting: buffer")
    toVHDL(buff, clock, bufferReset, bufferDataIn, bufferDataOut, bufferReady, bufferOutputInputReady, 
                        bufferAllowWriteOverlap, bufferDataDirection, decimator2clockOut, clockFtdi, alwaysEn)
    print("Converting: ft245sync")
    toVHDL(ft245sync, clockFtdi, ft245reset, rxf, txe, rd, wr, ft245oe, siwu, adbus, ft245din, ft245dout, 
                               ft245rw, ft245busy, ft245dataWaitIn, ft245dataWaitOut, ft245strobe)
    print("Converting: digipot_controller")
    toVHDL(digipot_controller, clock, digipotReset, digipotValue, digipotStrobe, digipotBusy, sda, scl)
    print("Converting: dac_controller")
    toVHDL(dac_controller, clock, dacReset, dacVrefTopA, dacVrefTopB, dacVrefBotA, dacVrefBotB, dacStrobe, 
                                       serialOut, load, ldac, clkDacOut, dacBusy)
    print("Converting: trigger_digital")
    toVHDL(trigger_digital, clock, digitalTriggerReset, logicAnalyzer, triggerPattern, triggerSamples, digiTrigger, decimator2clockOut, alwaysEn)
    print("Converting: trigger_analog")
    toVHDL(trigger_analog, clock, analogTrigger2Reset, decimator2out, 
                                        trigger2Type, trigger2Slope, trigger2Value, analog2Trigger, decimator2clockOut, alwaysEn)
    print("Converting: decimator")
    toVHDL(decimator, clock, reset, decimator1in, decimator2out, decimationRatio,  decimationRatioBase, decimationStyle, decimator2clockOut, decimatorNewValueFlag)
    print("Converting: digital_decimator")
    toVHDL(digital_decimator, clock, reset, logicAnalyzer, logicAnalyzer_dec, decimationRatio, decimatorDigClockOut)
    print("Converting: controller")
    toVHDL(controller, clockFtdi, reset, ft245din, ft245dout,  ft245rw, ft245busy, ft245oe, ft245wr,
                   ft245dataWaitIn, ft245dataWaitOut, ft245strobe, ft245reset, dacReset, 
                   dacVrefTopA, dacVrefTopB, dacVrefBotA, dacVrefBotB, dacStrobe, dacBusy,
                   digipotReset, digipotValue, digipotStrobe, digipotBusy, bufferReset, 
                   bufferDataOut, bufferDataIn, bufferReady, bufferOutputInputReady,
                   bufferAllowWriteOverlap, bufferDataDirection, decimatorNewValueFlag,
                   decimationRatioBase, decimationStyle, decimator1out, decimator2out,
                   trigger1Type, trigger1Slope, analogTrigger1Reset, trigger1Value,
                   analog1Trigger, trigger2Type, trigger2Slope, analogTrigger2Reset,
                   trigger2Value, analog2Trigger, digitalTriggerReset, triggerPattern, 
                   triggerSamples, digiTrigger, adc1pd, adc2pd, adc1relatt, adc2relatt, 
                   adc1relco, adc2relco, led0, led1, logicAnalyzer_dec, la1dir, la2dir, la1oe, la2oe, clock)
    
    
def top(reset, clock, clockFtdi, adc1data, adc2data, adc1pd, adc2pd, adc1relatt, adc2relatt, 
        adc1relco, adc2relco, led0, led1, logicAnalyzer, la1dir, la2dir, la1oe, la2oe,
        serialOut, load, ldac, clkDacOut, sda, scl, rxf, oe, txe, rd, wr, siwu, adbus):
    
    decimatorNewValueFlag = Signal(bool(False))
    ft245din = Signal(intbv(0)[8:])
    ft245din.driven = 'reg'
    ft245dout = Signal(intbv(0)[8:])
    ft245dout.driven = 'reg'
    ft245rw = Signal(bool(False))
    ft245rw.driven = 'reg'
    ft245reset = Signal(bool(False))
    ft245reset.driven = 'wire'
    ft245busy = Signal(bool(False))
    ft245busy.driven = 'wire'
    ft245dataWaitIn = Signal(bool(False))
    ft245dataWaitIn.driven = 'wire'
    ft245dataWaitOut = Signal(bool(False))
    ft245dataWaitOut.driven = 'wire'
    ft245strobe = Signal(bool(False))
    ft245strobe.driven = 'wire'
    dacReset = Signal(bool(False))
    dacReset.driven = 'wire'
    dacBusy = Signal(bool(False))
    dacBusy.driven = 'wire'
    dacStrobe = Signal(bool(False))
    dacStrobe.driven = 'wire'
    dacVrefTopA = Signal(intbv(0)[8:])
    dacVrefTopA.driven = 'reg'
    dacVrefTopB = Signal(intbv(0)[8:])
    dacVrefTopB.driven = 'reg'
    dacVrefBotA = Signal(intbv(0)[8:])
    dacVrefBotA.driven = 'reg'
    dacVrefBotB = Signal(intbv(0)[8:])
    dacVrefBotB.driven = 'reg'
    digipotValue = Signal(intbv(0)[7:])
    digipotValue.driven = 'reg'
    digipotReset = Signal(bool(False))
    digipotReset.driven = 'wire'
    digipotBusy = Signal(bool(False))
    digipotBusy.driven = 'wire'
    digipotStrobe = Signal(bool(False))
    digipotStrobe.driven = 'wire'
    bufferclock = Signal(bool(False))
    bufferclock.driven = 'wire'
    bufferReset = Signal(bool(False))
    bufferReset.driven = 'wire'
    bufferDataIn = Signal(intbv(0)[32:])
    bufferDataIn.driven = 'reg'
    bufferDataOut = Signal(intbv(0)[32:])
    bufferDataOut.driven = 'reg'
    bufferReady = Signal(bool(False))
    bufferReady.driven = 'wire'
    bufferOutputInputReady = Signal(bool(False))
    bufferOutputInputReady.driven = 'wire'
    bufferAllowWriteOverlap = Signal(bool(False))
    bufferAllowWriteOverlap.driven = 'wire'
    bufferUseSdram = Signal(bool(False))
    bufferUseSdram.driven = 'wire'
    bufferDataDirection = Signal(bool(False))
    bufferDataDirection.driven = 'wire'
    decimationRatioBase = Signal(intbv(0, min = 0, max = 9))
    decimationRatioBase.driven = 'reg'
    decimationRatio = Signal(intbv(0, min = 0, max = MAXIMAL_RATIO+1))
    
    decimationStyle = Signal(intbv(0)[2:])
    decimationStyle.driven = 'reg'
    analogTrigger1Reset = Signal(bool(False))
    analogTrigger1Reset.driven = 'wire'
    trigger1Type = Signal(bool(False))
    trigger1Type.driven = 'wire'
    trigger1Slope = Signal(bool(False))
    trigger1Slope.driven = 'wire'
    analog1Trigger = Signal(bool(False))
    analog1Trigger.driven = 'wire'
    trigger1Value = Signal(intbv(0)[8:])
    trigger1Value.driven = 'reg'
    analogTrigger2Reset = Signal(bool(False))
    analogTrigger2Reset.driven = 'wire'
    trigger2Type = Signal(bool(False))
    trigger2Type.driven = 'wire'
    trigger2Slope = Signal(bool(False))
    trigger2Slope.driven = 'wire'
    analog2Trigger = Signal(bool(False))
    analog2Trigger.driven = 'wire'
    trigger2Value = Signal(intbv(0)[8:])
    trigger2Value.driven = 'reg'
    digitalTriggerReset = Signal(bool(False))
    digitalTriggerReset.driven = 'wire'
    digiTrigger = Signal(bool(False))
    digiTrigger.driven = 'wire'
    triggerPattern = Signal(intbv(0)[64:])
    triggerPattern.driven = 'reg'
    triggerSamples = Signal(intbv(0, min = 0, max = 4))
    triggerSamples.driven = 'reg'
    decimator1out = Signal(intbv(0)[8:])
    decimator1out.driven = 'reg'
    decimator2out = Signal(intbv(0)[8:])
    decimator2out.driven = 'reg'
    decimatorclock = Signal(bool(False))
    #decimatorclock.driven = 'wire'
    decimator1in = Signal(intbv(0)[8:])
    decimator2in = Signal(intbv(0)[8:])
    oe_i = Signal(bool(False))
    rxf_i = Signal(bool(False))
    txe_i = Signal(bool(False))
    rd_i = Signal(bool(False))
    wr_i = Signal(bool(False))
    siwu_i = Signal(bool(False))
    serialOut_i = Signal(bool(False))
    load_i = Signal(bool(False))
    ldac_i = Signal(bool(False))
    clkDacOut_i = Signal(bool(False))
    adc1pd_i = Signal(bool(False))
    adc2pd_i = Signal(bool(False))
    adc1relatt_i = Signal(bool(False))
    adc2relatt_i = Signal(bool(False))
    adc1relco_i = Signal(bool(False))
    adc2relco_i = Signal(bool(False))
    led0_i = Signal(bool(False))
    led1_i = Signal(bool(False))
    logicAnalyzer_i = Signal(intbv(0)[16:])
    logicAnalyzer_dec = Signal(intbv(0)[16:])
    la1dir_i = Signal(bool(False))
    la2dir_i = Signal(bool(False))
    la1oe_i  = Signal(bool(False))
    la2oe_i  = Signal(bool(False))
    clock_i = Signal(bool(False))
    clockFtdi_i = Signal(bool(False))
    reset_i = Signal(bool(False))
    alwaysEn  = Signal(bool(False))
    
    @always_comb
    def connectSignals():
        decimator1in.next = adc1data
        decimator2in.next = adc2data
        oe.next = oe_i
        rxf_i.next = rxf
        txe_i.next = txe
        rd.next = rd_i
        wr.next = wr_i
        siwu.next = siwu_i
        serialOut.next = serialOut_i
        load.next = load_i
        ldac.next = ldac_i
        clkDacOut.next = clkDacOut_i
        adc1pd.next = adc1pd_i
        adc2pd.next = adc2pd_i
        adc1relatt.next = adc1relatt_i
        adc2relatt.next = adc2relatt_i
        adc1relco.next = adc1relco_i
        adc2relco.next = adc2relco_i
        led0.next = led0_i
        led1.next = led1_i
        logicAnalyzer_i.next = logicAnalyzer
        la1dir.next = la1dir_i
        la2dir.next = la2dir_i
        la1oe.next  = la1oe_i
        la2oe.next  = la2oe_i
        clock_i.next = clock
        clockFtdi_i.next = clockFtdi
        reset_i.next = reset
    
    
    controller.vhdl_instance = "controller"
    inst_controller = controller(clockFtdi_i, reset_i, ft245din, ft245dout, ft245rw, ft245busy, oe_i, wr_i,
                   ft245dataWaitIn, ft245dataWaitOut, ft245strobe, ft245reset, dacReset, 
                   dacVrefTopA, dacVrefTopB, dacVrefBotA, dacVrefBotB, dacStrobe, dacBusy,
                   digipotReset, digipotValue, digipotStrobe, digipotBusy, bufferReset, 
                   bufferDataOut, bufferDataIn, bufferReady, bufferOutputInputReady,
                   bufferAllowWriteOverlap, bufferDataDirection, decimatorNewValueFlag,
                   decimationRatioBase, decimationStyle, decimator1out, decimator2out,
                   trigger1Type, trigger1Slope, analogTrigger1Reset, trigger1Value,
                   analog1Trigger, trigger2Type, trigger2Slope, analogTrigger2Reset,
                   trigger2Value, analog2Trigger, digitalTriggerReset, triggerPattern, 
                   triggerSamples, digiTrigger, adc1pd_i, adc2pd_i, adc1relatt_i, adc2relatt_i, 
                   adc1relco_i, adc2relco_i, led0_i, led1_i, logicAnalyzer_dec, la1dir_i, la2dir_i, la1oe_i, la2oe_i, clock_i)
    
    decimator_clock_divisor.vhdl_instance = "decimator_clock_divisor"
    inst_decimator_clock_divisor = decimator_clock_divisor(clock_i, reset_i, decimationRatioBase, decimationRatio, decimatorclock, decimatorNewValueFlag, alwaysEn)
    
    digital_decimator.vhdl_instance = "digital_decimator"
    inst_digital_decimator = digital_decimator(clock_i, reset_i, logicAnalyzer_i, logicAnalyzer_dec, decimationRatio, decimatorclock)
    
    decimator.vhdl_instance = "decimator"
    inst_decimator1 = decimator(clock_i, reset_i, decimator1in, decimator1out, decimationRatio, decimationRatioBase, decimationStyle, decimatorclock, decimatorNewValueFlag)
    inst_decimator2 = decimator(clock_i, reset_i, decimator2in, decimator2out, decimationRatio, decimationRatioBase, decimationStyle, decimatorclock, decimatorNewValueFlag)
    trigger_analog.vhdl_instance = "trigger_analog"
    inst_trigger_analog1 = trigger_analog(clock_i, analogTrigger1Reset, decimator1out, 
                                        trigger1Type, trigger1Slope, trigger1Value, analog1Trigger, decimatorclock, alwaysEn)
    inst_trigger_analog2 = trigger_analog(clock_i, analogTrigger2Reset, decimator2out, 
                                        trigger2Type, trigger2Slope, trigger2Value, analog2Trigger, decimatorclock, alwaysEn)
    trigger_digital.vhdl_instance = "trigger_digital"
    inst_trigger_digital = trigger_digital(decimatorclock, digitalTriggerReset, logicAnalyzer_dec, triggerPattern, triggerSamples, digiTrigger, decimatorclock, alwaysEn)
    dac_controller.vhdl_instance = "dac_controller"
    inst_dac_controller = dac_controller(clockFtdi_i, dacReset, dacVrefTopA, dacVrefTopB, dacVrefBotA, dacVrefBotB, dacStrobe, 
                                       serialOut_i, load_i, ldac_i, clkDacOut_i, dacBusy)
    digipot_controller.vhdl_instance = "digipot_controller"
    inst_digipot_controller = digipot_controller(clockFtdi_i, digipotReset, digipotValue, digipotStrobe, digipotBusy, sda, scl)
    
    buff.vhdl_instance = "buff"
    inst_buffer = buff(clock_i, bufferReset, bufferDataIn, bufferDataOut, bufferReady, bufferOutputInputReady, 
                        bufferAllowWriteOverlap, bufferDataDirection, decimatorclock, clockFtdi, alwaysEn)
    ft245sync.vhdl_instance = "ft245sync"
    inst_ft245sync = ft245sync(clockFtdi_i, ft245reset, rxf_i, txe_i, rd_i, wr_i, oe_i, siwu_i, adbus, ft245din, ft245dout, 
                                                   ft245rw, ft245busy, ft245dataWaitIn, ft245dataWaitOut, ft245strobe)
    return connectSignals, inst_decimator1, inst_decimator2, inst_controller, inst_trigger_analog1, inst_trigger_analog2, \
           inst_buffer, inst_ft245sync, inst_digipot_controller, inst_dac_controller, inst_trigger_digital, \
           inst_digital_decimator, inst_decimator_clock_divisor

def convert_top():
    print("Converting: top module")
    adc1data = Signal(intbv(0)[8:])
    adc2data = Signal(intbv(0)[8:])
    oe = Signal(bool(False))
    rxf = Signal(bool(False))
    txe = Signal(bool(False))
    rd = Signal(bool(False))
    wr = Signal(bool(False))
    siwu = Signal(bool(False))
    adbus = TristateSignal(intbv(0)[8:])
    serialOut = Signal(bool(False))
    load = Signal(bool(False))
    ldac = Signal(bool(False))
    clkDacOut = Signal(bool(False))
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
    sda = TristateSignal(bool(False))
    scl = TristateSignal(bool(False))
    clock = Signal(bool(False))
    clockFtdi = Signal(bool(False))
    reset = Signal(bool(False))
    toVHDL(top, reset, clock, clockFtdi, adc1data, adc2data, adc1pd, adc2pd, adc1relatt, adc2relatt, 
            adc1relco, adc2relco, led0, led1, logicAnalyzer, la1dir, la2dir, la1oe, la2oe,
            serialOut, load, ldac, clkDacOut, sda, scl, rxf, oe, txe, rd, wr, siwu, adbus)

def tb_top():
    adc1data = Signal(intbv(0)[8:])
    adc2data = Signal(intbv(0)[8:])
    oe = Signal(bool(False))
    rxf = Signal(bool(False))
    txe = Signal(bool(False))
    rd = Signal(bool(False))
    rd.driven = 'wire'
    wr = Signal(bool(False))
    wr.driven = 'wire'
    siwu = Signal(bool(False))
    siwu.driven = 'wire'
    adbus = TristateSignal(intbv(0)[8:])
    adbus.driven = 'reg'
    serialOut = Signal(bool(False))
    serialOut.driven = 'reg'
    load = Signal(bool(False))
    load.driven = 'reg'
    ldac = Signal(bool(False))
    ldac.driven = 'reg'
    clkDacOut = Signal(bool(False))
    clkDacOut.driven = 'reg'
    adc1pd = Signal(bool(False))
    adc2pd = Signal(bool(False))
    adc1relatt = Signal(bool(False))
    adc1relatt.driven = 'reg'
    adc2relatt = Signal(bool(False))
    adc2relatt.driven = 'reg'
    adc1relco = Signal(bool(False))
    adc1relco.driven = 'reg'
    adc2relco = Signal(bool(False))
    adc2relco.driven = 'reg'
    led0 = Signal(bool(False))
    led0.driven = 'reg'
    led1 = Signal(bool(False))
    led1.driven = 'reg'
    logicAnalyzer = Signal(intbv(0)[16:])
    #logicAnalyzer.driven = 'reg'
    la1dir = Signal(bool(False))
    la1dir.driven = 'reg'
    la2dir = Signal(bool(False))
    la2dir.driven = 'reg'
    la1oe  = Signal(bool(False))
    la1oe.driven = 'reg'
    la2oe  = Signal(bool(False))
    la2oe.driven = 'reg'
    sda = TristateSignal(bool(False))
    sda.driven = 'reg'
    scl = TristateSignal(bool(False))
    scl.driven = 'reg'
    clock = Signal(bool(False))
    clockFtdi = Signal(bool(False))
    reset = Signal(bool(False))
    
    sda_d = sda.driver()
    scl_d = scl.driver()
    
    sda_dpull = sda.driver()
    scl_dpull = scl.driver()
    
    adbus_drv = adbus.driver()
    
    data = [Signal(intbv(0)[8:]) for i in range(9)]
    lfsr = Signal(intbv(0)[LFSR_WIDTH:])
    top.vhdl_instance = "top"
    dut_top = top(reset, clock, clockFtdi, adc1data, adc2data, adc1pd, adc2pd, adc1relatt, adc2relatt, 
            adc1relco, adc2relco, led0, led1, logicAnalyzer, la1dir, la2dir, la1oe, la2oe,
            serialOut, load, ldac, clkDacOut, sda, scl, rxf, oe, txe, rd, wr, siwu, adbus)
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

    @always(clock.posedge)
    def logicAnalyzerGen():
        if(reset == 1):
            logicAnalyzer.next = 0
        else:
            logicAnalyzer.next = logicAnalyzer + 1
    
    @always_comb
    def adcGen():
        if(reset == 1):
            adc1data.next = 0
            adc2data.next = 0
        else:
            if(adc1pd):
                adc1data.next = 0
            else:
                adc1data.next = lfsr[10:2]
            if(adc2pd):
                adc2data.next = 0
            else:
                adc2data.next = lfsr[8:]

    @instance
    def stimulus():
        i = 0
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
        yield delay(196)
        print ("FT245 ready")
        yield clockFtdi.posedge
        rxf.next = False
        print ("FT245 RXF low")
        yield (oe.negedge)
        yield clockFtdi.posedge
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
        yield rd.negedge
        yield clockFtdi.posedge
        while(rxf == 0):
            yield clockFtdi.posedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.negedge
        yield clockFtdi.posedge
        adbus_drv.next = None
        rxf.next = True
        yield delay(200)
        txe.next = False
        yield delay(200)
        txe.next = True
        #SET decimation
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield (oe.negedge)
        yield clockFtdi.posedge
        print ("OE came low, pushing data from FTDI")
        adbus_drv.next = 0x30
        i = 0
        data[0].next = 0x04
        data[1].next = 0x03
        data[2].next = 0
        data[3].next = 0
        data[4].next = 0
        data[5].next = 0
        data[6].next = 0
        data[7].next = 0
        data[8].next = 0
        yield rd.negedge
        yield clockFtdi.posedge
        while(rxf == 0):
            yield clockFtdi.posedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.negedge
        yield clockFtdi.posedge
        adbus_drv.next = None
        rxf.next = True
        yield delay(200)
        txe.next = False
        yield delay(200)
        txe.next = True
        #SET attenuators
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield (oe.negedge)
        yield clockFtdi.posedge
        print ("OE came low, pushing data from FTDI")
        adbus_drv.next = 0x20
        i = 0
        data[0].next = 0xFF
        data[1].next = 0x00
        data[2].next = 0
        data[3].next = 0
        data[4].next = 0
        data[5].next = 0
        data[6].next = 0
        data[7].next = 0
        data[8].next = 0
        yield rd.negedge
        yield clockFtdi.posedge
        while(rxf == 0):
            yield clockFtdi.posedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.negedge
        yield clockFtdi.posedge
        adbus_drv.next = None
        rxf.next = True
        yield delay(200)
        txe.next = False
        yield delay(200)
        txe.next = True
        #SET trigger
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield (oe.negedge)
        yield clockFtdi.posedge
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
        yield rd.negedge
        yield clockFtdi.posedge
        while(rxf == 0):
            yield clockFtdi.posedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.negedge
        yield clockFtdi.posedge
        rxf.next = True
        adbus_drv.next = None
        yield delay(200)
        txe.next = False
        yield delay(200)
        txe.next = True
        #SET couplings
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield (oe.negedge)
        yield clockFtdi.posedge
        print ("OE came low, pushing data from FTDI")
        adbus_drv.next = 0x21
        i = 0
        data[0].next = 0x00
        data[1].next = 0xFF
        data[2].next = 0
        data[3].next = 0
        data[4].next = 0
        data[5].next = 0
        data[6].next = 0
        data[7].next = 0
        data[8].next = 0
        yield rd.negedge
        yield clockFtdi.posedge
        while(rxf == 0):
            yield clockFtdi.posedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.negedge
        yield clockFtdi.posedge
        adbus_drv.next = None
        rxf.next = True
        yield delay(200)
        txe.next = False
        yield delay(200)
        txe.next = True
        #Start capture
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield (oe.negedge)
        yield clockFtdi.posedge
        print ("OE came low, pushing data from FTDI")
        adbus_drv.next = 0x40
        i = 0
        data[0].next = 0x05
        data[1].next = 0
        data[2].next = 0
        data[3].next = 0
        data[4].next = 0
        data[5].next = 0
        data[6].next = 0
        data[7].next = 0
        data[8].next = 0
        yield rd.negedge
        yield clockFtdi.posedge
        while(rxf == 0):
            yield clockFtdi.posedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.negedge
        yield clockFtdi.posedge
        rxf.next = True
        adbus_drv.next = None
        yield delay(400)
        txe.next = False
        yield clockFtdi.negedge
        yield wr.posedge
        print("First data captured")
        #set digital trigger
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield (oe.negedge)
        yield clockFtdi.posedge
        print ("OE came low, pushing data from FTDI")
        adbus_drv.next = 0x32
        i = 0
        data[0].next = 0x11
        data[1].next = 0x22
        data[2].next = 0x33
        data[3].next = 0x44
        data[4].next = 0x55
        data[5].next = 0x66
        data[6].next = 0x77
        data[7].next = 0x88
        data[8].next = 0x00
        yield rd.negedge
        yield clockFtdi.posedge
        while(rxf == 0):
            yield clockFtdi.posedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.negedge
        yield clockFtdi.posedge
        adbus_drv.next = None
        rxf.next = True
        yield delay(200)
        txe.next = False
        yield delay(200)
        txe.next = True
        #set VLOG
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield (oe.negedge)
        yield clockFtdi.posedge
        print ("OE came low, pushing data from FTDI")
        adbus_drv.next = 0x11
        i = 0
        data[0].next = 100
        data[1].next = 0
        data[2].next = 0
        data[3].next = 0
        data[4].next = 0
        data[5].next = 0
        data[6].next = 0
        data[7].next = 0
        data[8].next = 0
        yield rd.negedge
        yield clockFtdi.posedge
        while(rxf == 0):
            yield clockFtdi.posedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.negedge
        yield clockFtdi.posedge
        adbus_drv.next = None
        rxf.next = True
        yield delay(200)
        txe.next = False
        yield delay(200)
        txe.next = True
        yield scl.negedge
        for i in range(0, 9):
            yield scl.posedge
        sda_d.next = False
        yield scl.posedge
        sda_d.next = None
        for i in range(0, 8):
            yield scl.posedge
        sda_d.next = False
        yield scl.posedge
        sda_d.next = None
        yield delay(200)
        #start another capture
        print ("FT245 ready")
        rxf.next = False
        print ("FT245 RXF low")
        yield (oe.negedge)
        yield clockFtdi.posedge
        print ("OE came low, pushing data from FTDI")
        adbus_drv.next = 0x40
        i = 0
        data[0].next = 0x06
        data[1].next = 0
        data[2].next = 0
        data[3].next = 0
        data[4].next = 0
        data[5].next = 0
        data[6].next = 0
        data[7].next = 0
        data[8].next = 0
        yield rd.negedge
        yield clockFtdi.posedge
        while(rxf == 0):
            yield clockFtdi.posedge
            adbus_drv.next = data[i]
            if(i == 8):
                break
            i = i + 1
            yield clockFtdi.negedge
        yield clockFtdi.posedge
        rxf.next = True
        adbus_drv.next = None
        yield delay(400)
        txe.next = False
        yield clockFtdi.negedge
        yield wr.negedge
        yield delay(300)
        txe.next = True
        yield delay(100)
        txe.next = False
        yield wr.posedge
        print("second data captured")
        yield delay(100)
        raise StopSimulation
    
    return stimulus, clkgen, clkgen_f, dut_top, adcGen, lfsr_proc, logicAnalyzerGen

def convert_tb():
    print("Converting: top_tb (testbench)")
    toVHDL(tb_top)
    


convert_modules()
convert_top()
if(INCLUDE_TESTBENCH):
    convert_tb()