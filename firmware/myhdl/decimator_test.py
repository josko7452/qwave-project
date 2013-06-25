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
import random
import sys
sys.setrecursionlimit(10000)

from decimator import decimator

PERIOD = 6

def bench():
    """ Unit test for decimator. """
    
    clock = Signal(bool(False))
    reset = Signal(bool(False))
    clkDataOut = Signal(bool(False))
    dataIn = Signal(intbv(0)[8:])
    dataOut = Signal(intbv(0)[8:])
    decimationRatioBase = Signal(intbv(0, min = 0, max = 11))
    decimationStyle = Signal(intbv(0)[2:])
    
    dut_decimator = decimator(clock, reset, dataIn, dataOut, decimationRatioBase, decimationStyle, clkDataOut)
    
    @always(delay(PERIOD//2))
    def clkgen():
        clock.next = not clock

    @instance
    def stimulus():
        reset.next = True
        dataIn.next = 0
        decimationStyle.next = 0
        decimationRatioBase.next = 0
        yield delay(100)
        reset.next = False
        for i in range(0, 10000):
            dataIn.next = random.randint(0, 255)
            #dataIn.next = 10
            yield clock.negedge
        dataIn.next = 0
        yield delay(1000)
        reset.next = True
        yield delay(100)
        reset.next = False
        decimationStyle.next = 1
        yield delay(100)
        reset.next = False
        for i in range(0, 10000):
            dataIn.next = random.randint(0, 255)
            #dataIn.next = 10
            yield clock.negedge
        dataIn.next = 0
        yield delay(1000)
        reset.next = True
        yield delay(100)
        reset.next = False
        decimationStyle.next = 2
        yield delay(100)
        reset.next = False
        for i in range(0, 10000):
            dataIn.next = random.randint(0, 255)
            #dataIn.next = 10
            yield clock.negedge
        dataIn.next = 0
        yield delay(1000)
        reset.next = True
        yield delay(100)
        reset.next = False
        decimationStyle.next = 3
        yield delay(100)
        reset.next = False
        for i in range(0, 10000):
            dataIn.next = random.randint(0, 255)
            #dataIn.next = 10
            yield clock.negedge
        dataIn.next = 0
        yield delay(1000)
        reset.next = True
        decimationStyle.next = 0
        decimationRatioBase.next = 4
        yield delay(100)
        reset.next = False
        for i in range(0, 10000):
            dataIn.next = random.randint(0, 255)
            #dataIn.next = 10
            yield clock.negedge
        dataIn.next = 0
        yield delay(1000)
        reset.next = True
        yield delay(100)
        reset.next = False
        decimationStyle.next = 1
        yield delay(100)
        reset.next = False
        for i in range(0, 10000):
            dataIn.next = random.randint(0, 255)
            #dataIn.next = 10
            yield clock.negedge
        dataIn.next = 0
        yield delay(1000)
        reset.next = True
        yield delay(100)
        reset.next = False
        decimationStyle.next = 2
        yield delay(100)
        reset.next = False
        for i in range(0, 10000):
            dataIn.next = random.randint(0, 255)
            #dataIn.next = 10
            yield clock.negedge
        dataIn.next = 0
        yield delay(1000)
        reset.next = True
        yield delay(100)
        reset.next = False
        decimationStyle.next = 3
        yield delay(100)
        reset.next = False
        for i in range(0, 10000):
            dataIn.next = random.randint(0, 255)
            #dataIn.next = 10
            yield clock.negedge
        dataIn.next = 0
        yield delay(1000)
        reset.next = True
        raise StopSimulation

    return dut_decimator, clkgen, stimulus
    
tb_cntrl = traceSignals(bench)
simulation = Simulation(tb_cntrl)
simulation.run()

toVHDL(bench)