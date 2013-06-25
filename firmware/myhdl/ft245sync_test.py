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

from ft245sync import ft245sync

PERIOD = 16

def bench():
    """ Unit test for dac_controller. """
    
    clock = Signal(bool(False))
    reset = Signal(bool(False))
    rxf = Signal(bool(False))
    txe = Signal(bool(False))
    rd = Signal(bool(False))
    wr = Signal(bool(False))
    oe = Signal(bool(False))
    siwu = Signal(bool(False))
    adbus = TristateSignal(intbv(0)[8:])
    dataIn = Signal(intbv(0)[8:])
    dataOut = Signal(intbv(0)[8:])
    rw = Signal(bool(False))
    busy = Signal(bool(False))
    dataWaitIn = Signal(bool(False))
    dataWaitOut = Signal(bool(False))
    strobe = Signal(bool(False))

    adbus_drv = adbus.driver()

    dut_ft245sync = ft245sync(clock, reset, rxf, txe, rd, wr, oe, siwu, adbus, dataIn, dataOut, rw, busy, dataWaitIn, dataWaitOut, strobe)
    
    @always(delay(PERIOD//2))
    def clkgen():
        clock.next = not clock

    @instance
    def stimulus():
        reset.next = True
        strobe.next = False
        dataWaitIn.next = False
        rw.next = False
        adbus_drv.next = None
        rxf.next = True
        txe.next = True
        yield delay(32)
        reset.next = False
        yield busy.negedge
        print ("FT245 ready")
        rw.next = True
        strobe.next = True
        yield busy.posedge
        print ("FT245 in work")
        strobe.next = False
        txe.next = False
        yield clock.posedge
        dataIn.next = 113
        if(wr == 0):
            print ("Wr is low .. OK .. Written data:")
            print (adbus)
        else:
            print ("Error .. Wr was not LOW")
            yield (wr.negedge)
            yield clock.posedge
            print (adbus)
        i = 25
        while(dataWaitOut == 0):
            yield clock.negedge
            dataIn.next = i
            i = i + 1
            yield clock.posedge
            print ("data:")
            print (adbus)
            if(i == 50):
                txe.next = True
        
        yield clock.posedge
        if(busy != 0):
            print ("Error: Busy should be low")
            yield busy.negedge
        print ("FT245 ready again")
        rw.next = False
        strobe.next = True
        yield busy.posedge
        print ("FT245 in work again")
        strobe.next = False
        rxf.next = False
        yield clock.posedge
        if(oe == 0):
            print ("OE is low .. OK .. reading data")
            adbus_drv.next = 115
        else:
            print ("Error .. OE was not LOW")
            yield (oe.negedge)
            adbus_drv.next = 115
        i = 28
        while(rxf == 0):
            yield clock.negedge
            if(not dataWaitIn):
                adbus_drv.next = i
            yield clock.posedge
            print ("data")
            print (dataOut)
            if(i == 50):
                rxf.next = 1
            if(i == 40):
                dataWaitIn.next = True
            if(i == 43):
                dataWaitIn.next = False
            i = i + 1
        yield clock.posedge
        if(busy != 0):
            print ("Error: Busy should be low")
            yield busy.negedge
        
        raise StopSimulation

    return dut_ft245sync, clkgen, stimulus
    
simulation = Simulation(bench())
simulation.run()

toVHDL(bench)