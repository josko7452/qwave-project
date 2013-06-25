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

from buff import buff

PERIOD = 6

FPGA_BITS_PER_BRAM = 2048*8 #SPARTAN 6
FPGA_TOTAL_NUMBER_OF_BRAM = 32 #SPARTAN 6
BRAM_RESERVE = 2 # How much BRAM blocks to spare
BIT_WIDTH = 16 #Bit width of buffer
FPGA_BUFFER_LENGHT = FPGA_TOTAL_NUMBER_OF_BRAM*FPGA_BITS_PER_BRAM//(BIT_WIDTH) - BRAM_RESERVE*FPGA_BITS_PER_BRAM//(BIT_WIDTH)
MAX_DATA_VALUE = 2**16-1

def bench():
    """ Unit test for buffer. """
    
    clock = Signal(bool(False))
    reset = Signal(bool(False))
    dataIn = Signal(intbv(0)[16:])
    dataOut = Signal(intbv(0)[16:])
    bufferReady = Signal(bool(False))
    outputInputReady = Signal(bool(False))
    allowWriteOverlap = Signal(bool(False))
    useSdram = Signal(bool(False))
    dataDirection = Signal(bool(False))

    dut_buffer = buff(clock, reset, dataIn, dataOut, bufferReady, outputInputReady, 
                        allowWriteOverlap, useSdram, dataDirection)

    @always(delay(PERIOD//2))
    def clkgen():
        clock.next = not clock

    @instance
    def stimulus():
        reset.next = True
        useSdram.next = False #BRAM or RAM buffer test
        #useSdram.next = True #BRAM or RAM buffer test
        dataDirection.next = False
        allowWriteOverlap.next = False
        outputInputReady.next = False
        dataIn.next = 0
        yield delay(12)
        reset.next = False
        dataDirection.next = True
        allowWriteOverlap.next = True
        yield delay(18)
        outputInputReady.next = True
        yield bufferReady.posedge
        for i in range(2*FPGA_BUFFER_LENGHT + 345):
            dataIn.next = i % MAX_DATA_VALUE
            if(i == FPGA_BUFFER_LENGHT + 113):
                allowWriteOverlap.next = False
            if(bufferReady == 0):
                break
            yield clock.posedge
        outputInputReady.next = False
        dataDirection.next = False
        yield delay(18)
        outputInputReady.next = True
        yield bufferReady
        while(bufferReady):
            outputInputReady.next = True
            yield delay(6)
            outputInputReady.next = False
            yield delay(30)
            
        raise StopSimulation

    return dut_buffer, clkgen, stimulus
    
#tb_cntrl = traceSignals(bench)
#simulation = Simulation(tb_cntrl)
#simulation.run()

toVHDL(bench)