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

BIT_WIDTH = 32
FPGA_BUFFER_LENGTH = 2**14 + 2**11
#FPGA_BUFFER_LENGTH = 2**14
BRAM_TOP_ADDRESS = FPGA_BUFFER_LENGTH-1
BLOCKS_BEFORE_DISSALOWED_OVERLAP = 2**12


tBufStates = enum('INIT', 'WRITE_BRAM', 'READ_BRAM', 'FULL_EMPTY_BRAM')

def buff(clk, reset, dataIn, dataOut, bufferReady, outputInputReady, 
           allowWriteOverlap, dataDirection, clkDecimator, clkFtdi, alwaysEn):
    """
        clk - input - clock
        reset - asynchronous reset
        dataIn - input - data to be written to BUFFER
        dataOut - output - data read from BUFFER
        bufferReady - output - indicates that buffer is ready to input data or output data
        outputInputReady - input - output/input is ready
        allowWriteOverlap - input - allow memory to overlap, useful when in trigger armed mode of scope
                                    memory will overwrite previous data when full instead lowering bufferReady
        dataDirection - input - indicates wether to input or output data
        clkDecimator - input - clock of decimated data which data are sampled into buffer
        clkFtdi - input - 60MHz clock of FTDI FT245mode and controller
        alwaysEn - input - used in case of no decimation applied,
                           data are sampled to buffer at clk clock (instead of clkDecimator)
                           
        Two clock domains are used therefore Xilinx tools infers dualport-dualclock FIFO
    """
    overlapped = Signal(bool(False))
    bufferReady_int = Signal(bool(False))
    clkEn = Signal(bool(False))
    boundarySet = Signal(bool(False))
    bottomSet = Signal(bool(False))
    
    fpga_mem = [Signal(intbv(0)[BIT_WIDTH:]) for i in range(FPGA_BUFFER_LENGTH)]
    state = Signal(tBufStates.INIT)
    bram_address_top = Signal(intbv(0, min = 0, max = FPGA_BUFFER_LENGTH))
    bram_address_bottom = Signal(intbv(0, min = 0, max = FPGA_BUFFER_LENGTH))
    alreadyRead = Signal(bool(False))
    currentBramOverlapBoundary = Signal(intbv(0, min = 0, max = FPGA_BUFFER_LENGTH))
    holdFullEmpty = Signal(intbv(0, min = 0, max = 9))
    
    @always_comb
    def connectBufferReady():
        """This process connexts internal bufferReady signal"""
        bufferReady.next = bufferReady_int
    
    @always(clk.posedge, reset.posedge)
    def setBoundary():
        """This process sets overlap boundary when trigger occurs, pre trigger value is defined by
           BLOCKS_BEFORE_DISSALOWED_OVERLAP."""
        if(reset == 1):
            currentBramOverlapBoundary.next = FPGA_BUFFER_LENGTH - 1
            boundarySet.next = False
        else:
            if(allowWriteOverlap == 0 and boundarySet == 0):
                if(bram_address_top > BLOCKS_BEFORE_DISSALOWED_OVERLAP):
                    currentBramOverlapBoundary.next = bram_address_top - BLOCKS_BEFORE_DISSALOWED_OVERLAP
                    boundarySet.next = True
                else:
                    if(overlapped):
                        currentBramOverlapBoundary.next = bram_address_top + BRAM_TOP_ADDRESS - BLOCKS_BEFORE_DISSALOWED_OVERLAP
                        boundarySet.next = True
                #    else:
                #        currentBramOverlapBoundary.next = FPGA_BUFFER_LENGTH - 1
    
    @always(clk.posedge)
    def fsm():
        """This process is control FSM of buffer
           In INIT state it initializes buffer,
           In WRITE_BRAM state it writes data in overlapping mode until allowWriteOverlap
           goes low than it fills memory until overlap boundary is reached
           In FULL_EMPTY_BRAM state it waits for controller to change dataDirection 
           and then goes to READ_BRAM if BRAM if FULL or 
           holds bufferReady in 0 if BRAM is EMPTY and goes to INIT
           In READ bram mode it waits until BRAM is EMPTY
        """
        if(reset == 1):
            state.next = tBufStates.INIT
            overlapped.next = False
            bram_address_top.next = 0
            bufferReady_int.next = False
            alreadyRead.next = False
            holdFullEmpty.next = 0
            clkEn.next = True
        else:
            if(state == tBufStates.INIT):
                bram_address_top.next = 0
                alreadyRead.next = False
                overlapped.next = False
                if(outputInputReady):
                    if(dataDirection):
                        state.next = tBufStates.WRITE_BRAM
                        bufferReady_int.next = True
                    else:
                        state.next = tBufStates.READ_BRAM
                        bufferReady_int.next = True
                else:
                    state.next = tBufStates.INIT
            elif(state == tBufStates.WRITE_BRAM):
                bufferReady_int.next = True
                if(clkDecimator == 0):
                    clkEn.next = True
                if(clkDecimator == 1 and outputInputReady == 1 and (clkEn == 1 or alwaysEn)):
                    clkEn.next = False
                    if(bram_address_top == 0):
                        overlapped.next = False
                    if(bram_address_top == currentBramOverlapBoundary):
                        overlapped.next = True
                        if(allowWriteOverlap):
                            fpga_mem[bram_address_top].next = dataIn
                            state.next = tBufStates.WRITE_BRAM
                            bram_address_top.next = (bram_address_top + 1) % FPGA_BUFFER_LENGTH
                        else:
                            fpga_mem[bram_address_top].next = dataIn
                            state.next = tBufStates.FULL_EMPTY_BRAM
                    else:
                        fpga_mem[bram_address_top].next = dataIn
                        state.next = tBufStates.WRITE_BRAM
                        bram_address_top.next = (bram_address_top + 1) % FPGA_BUFFER_LENGTH
                else:
                    state.next = tBufStates.WRITE_BRAM
            elif(state == tBufStates.FULL_EMPTY_BRAM):
                if(dataDirection):
                    state.next = tBufStates.FULL_EMPTY_BRAM
                    bufferReady_int.next = False
                else:
                    if(alreadyRead):
                        bufferReady_int.next = False
                        if(holdFullEmpty == 8):
                            state.next = tBufStates.INIT
                            holdFullEmpty.next = 0
                        else:
                            state.next = tBufStates.FULL_EMPTY_BRAM
                            holdFullEmpty.next = holdFullEmpty + 1
                    else:
                        bufferReady_int.next = True
                        if(outputInputReady and bottomSet):
                            state.next = tBufStates.READ_BRAM
                        else:
                            state.next = tBufStates.FULL_EMPTY_BRAM
            elif(state == tBufStates.READ_BRAM):
                alreadyRead.next = True
                if(outputInputReady == 1):
                    if(bram_address_bottom == bram_address_top):
                        state.next = tBufStates.FULL_EMPTY_BRAM
                    else:
                        state.next = tBufStates.READ_BRAM
                else:
                    state.next = tBufStates.READ_BRAM
            
    @always(clkFtdi.posedge)
    def fsmSlowPart():
        """This process outputs data to be transmitted to PC at 60MHz FTDI clock,
            if outputInputReady is low then buffer stops 
            outputing data until outputInputReady goes high"""
        if(reset == 1):
            bram_address_bottom.next = 0
            dataOut.next = 0
            bottomSet.next = False
        else:
            if(state == tBufStates.INIT):
                bram_address_bottom.next = 0
            elif(state == tBufStates.FULL_EMPTY_BRAM):
                if(dataDirection == 0 and alreadyRead == 0):
                    bram_address_bottom.next = (bram_address_top + 1) % FPGA_BUFFER_LENGTH
                    bottomSet.next = True
            elif(state == tBufStates.READ_BRAM):
                bottomSet.next = False
                if(outputInputReady == 1):
                    if(bram_address_bottom == bram_address_top):
                        dataOut.next = fpga_mem[bram_address_bottom]
                    else:
                        dataOut.next = fpga_mem[bram_address_bottom]
                        bram_address_bottom.next = (bram_address_bottom + 1) % FPGA_BUFFER_LENGTH
        
    
    return fsm, setBoundary, connectBufferReady, fsmSlowPart