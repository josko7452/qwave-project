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

from trigger_digital import trigger_digital

PERIOD = 6

def bench():
    """ Unit test for trigger_digital. """
    
    clock = Signal(bool(False))
    reset = Signal(bool(False))
    trigger = Signal(bool(False))
    dataIn = Signal(intbv(0)[16:])
    pattern = Signal(intbv(0)[64:])
    samples = Signal(intbv(0, min = 0, max = 4))

    dut_trigger_digital = trigger_digital(clock, reset, dataIn, pattern, samples, trigger)
    
    @always(delay(PERIOD//2))
    def clkgen():
        clock.next = not clock

    @instance
    def stimulus():
        reset.next = True
        pattern.next[16:] = 0x53D3
        samples.next = 0
        yield delay(12)
        reset.next = False
        for i in range(0, 64000):
            yield clock.negedge
            dataIn.next = i
        pattern.next[32:] = 0x53D253D3
        samples.next = 1
        reset.next = True
        yield delay(12)
        reset.next = False
        for i in range(0, 64000):
            yield clock.negedge
            dataIn.next = i
        pattern.next[48:] = 0x53D153D253D3
        samples.next = 2
        reset.next = True
        yield delay(12)
        reset.next = False
        for i in range(0, 64000):
            yield clock.negedge
            dataIn.next = i
        pattern.next[64:] = 0x53D053D153D253D3
        samples.next = 2
        reset.next = True
        yield delay(12)
        reset.next = False
        for i in range(0, 64000):
            yield clock.negedge
            dataIn.next = i
 
        raise StopSimulation

    return dut_trigger_digital, clkgen, stimulus
    
tb_cntrl = traceSignals(bench)    
simulation = Simulation(tb_cntrl)
simulation.run()

toVHDL(bench)