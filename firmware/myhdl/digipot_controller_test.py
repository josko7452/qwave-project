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

from digipot_controller import digipot_controller

PERIOD = 6

def bench():
    """ Unit test for digipot_controller. """
    
    clock = Signal(bool(False))
    reset = Signal(bool(False))
    strobe = Signal(bool(False))
    busy = Signal(bool(False))
    sda = TristateSignal(bool(False))
    scl = TristateSignal(bool(False))
    sda_d = sda.driver()
    scl_d = scl.driver()
    sda_dpull = sda.driver()
    scl_dpull = scl.driver()
    value = Signal(intbv(0)[7:])
    
    dut_digipot_controller = digipot_controller(clock, reset, value, strobe, busy, sda, scl)
    
    @always(delay(PERIOD//2))
    def clkgen():
        clock.next = not clock

    @instance
    def stimulus():
        reset.next = True
        strobe.next = False
        sda_d.next = None
        scl_d.next = None
        sda_dpull.next = None
        scl_dpull.next = None
        value.next = 0
        yield delay(100)
        reset.next = False
        yield busy.negedge
        value.next = 50
        strobe.next = True
        yield busy.posedge
        strobe.next = False
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
        yield busy.negedge
        yield delay(1000)
        raise StopSimulation
    
    @always(sda, scl)
    def monitor():
        print now(), sda, scl
    

    return dut_digipot_controller, clkgen, stimulus
    
#tb_cntrl = traceSignals(bench)
#simulation = Simulation(bench())
#simulation.run()

toVHDL(bench)