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

from sdram_base import sdram_base

FPGA_BITS_PER_BRAM = 2048*8 #SPARTAN 6
FPGA_TOTAL_NUMBER_OF_BRAM = 32 #SPARTAN 6
BRAM_RESERVE = 10 # How much BRAM blocks to spare
BIT_WIDTH = 16 #Bit width of buffer
FPGA_BUFFER_LENGTH = FPGA_TOTAL_NUMBER_OF_BRAM*FPGA_BITS_PER_BRAM//(BIT_WIDTH) - BRAM_RESERVE*FPGA_BITS_PER_BRAM//(BIT_WIDTH)
BLOCKS_BEFORE_DISSALOWED_OVERLAP = 3000

SDRAM_BURST = 256
#SDRAM_SIZE = 2**22-1
SDRAM_SIZE = FPGA_BUFFER_LENGTH

tBufStates = enum('INIT', 'WAIT_FOR_SDRAM', 'WRITE_BRAM', 'PREPARE_WRITE_SDRAM', 'PREPARE_WRITE_SDRAM2', 'PREPARE_NEXT_READ',
                  'PREPARE_READ_SDRAM', 'WRITE_SDRAM', 'READ_BRAM', 'READ_SDRAM', 'FULL_EMPTY_SDRAM', 'FULL_EMPTY_BRAM')

def buff(clk, reset, dataIn, dataOut, bufferReady, outputInputReady, 
           allowWriteOverlap, useSdram, dataDirection, a, dq, ba,
           cke, nCs, nRas, nCas,
           nWe, ramData):
    """
        clk - input - clock
        reset - asynchronous reset
        dataIn - input - data to be written to SDRAM
        dataOut - output - data read from SDRAM
        bufferReady - output - indicates that buffer is ready to input data or output data
        outputInputReady - input - output/input is ready
        allowWriteOverlap - input - allow memory to overlap, useful when in trigger armed mode of scope
                               memory will overwrite previous data when full instead lowering bufferReady
        useSdram - input - enables or disables usage of sdram for buffering, increases buffer performace but dramatically reduces
                           capacity of buffer, must be set while reset and hold
        dataDirection - input - indicates wether to input or output data
    """
    
    ram_reset = Signal(bool(False))
    rw = Signal(bool(False))
    en = Signal(bool(False))
    address = Signal(intbv(0)[22:])
    ack = Signal(bool(False))
    fpBurstMode = Signal(bool(False))
    ram_dataIn = Signal(intbv(0)[16:])
    ram_dataIn_delayed = Signal(intbv(0)[16:])
    burstTerminated = Signal(bool(False))
    burstModeChanged = Signal(bool(False))
    overlapped = Signal(bool(False))
    bufferReady_int = Signal(bool(False))

    sdram_controller = sdram_base(clk, ram_reset, rw, ack, burstTerminated, fpBurstMode, burstModeChanged, en, 
                                 address, ram_dataIn_delayed, a, dq, ba, 
                                 cke, nCs, nRas, nCas, nWe, ramData)
    
    ram_ramData = ramData.driver()
    fpga_mem = [Signal(intbv(0)[BIT_WIDTH:]) for i in range(FPGA_BUFFER_LENGTH)]
    state = Signal(tBufStates.INIT)
    bram_address_top = Signal(intbv(0, min = 0, max = FPGA_BUFFER_LENGTH))
    bram_address_bottom = Signal(intbv(0, min = 0, max = FPGA_BUFFER_LENGTH))
    ram_address_top = Signal(intbv(0)[22:])
    waitForBurst = Signal(bool(False))
    alreadyRead = Signal(bool(False))
    currentSdramOverlapBoundary = Signal(intbv(0)[22:])
    currentBramOverlapBoundary = Signal(intbv(0, min = 0, max = FPGA_BUFFER_LENGTH))
    
    @always_comb
    def connectBufferReady():
        bufferReady.next = bufferReady_int
    
    @always(clk.negedge)
    def delay():
        ram_dataIn_delayed.next = ram_dataIn
        
    @always(allowWriteOverlap.negedge)
    def setBoundary():
        if(reset == 1):
            currentSdramOverlapBoundary.next = SDRAM_SIZE - 1
            currentBramOverlapBoundary.next = FPGA_BUFFER_LENGTH - 1
        else:
            if(allowWriteOverlap == 0):
                if(address > BLOCKS_BEFORE_DISSALOWED_OVERLAP):
                    currentSdramOverlapBoundary.next = address - BLOCKS_BEFORE_DISSALOWED_OVERLAP
                else:
                    if(overlapped):
                        currentSdramOverlapBoundary.next = address + SDRAM_SIZE - BLOCKS_BEFORE_DISSALOWED_OVERLAP
                    else:
                        currentSdramOverlapBoundary.next = SDRAM_SIZE - 1
                if(bram_address_top > BLOCKS_BEFORE_DISSALOWED_OVERLAP):
                    currentBramOverlapBoundary.next = bram_address_top - BLOCKS_BEFORE_DISSALOWED_OVERLAP
                else:
                    if(overlapped):
                        currentBramOverlapBoundary.next = bram_address_top + FPGA_BUFFER_LENGTH - BLOCKS_BEFORE_DISSALOWED_OVERLAP
                    else:
                        currentBramOverlapBoundary.next = FPGA_BUFFER_LENGTH - 1
    
    @always(clk.posedge)
    def fsm():
        if(reset == 1):
            state.next = tBufStates.INIT
            overlapped.next = False
            ram_dataIn.next = 0
            address.next = 0 
            ram_reset.next = True
            rw.next = False
            en.next = False
            fpBurstMode.next = False
            burstModeChanged.next = False
            bram_address_top.next = 0
            bram_address_bottom.next = 0
            bufferReady_int.next = False
            waitForBurst.next = False
            alreadyRead.next = False
            dataOut.next = 0
            ram_address_top.next = 0
        else:
            if(state == tBufStates.INIT):
                overlapped.next = False
                if(useSdram):
                    state.next = tBufStates.WAIT_FOR_SDRAM
                    ram_dataIn.next = 0
                    address.next = 0 
                    ram_reset.next = False
                    rw.next = False
                    en.next = True
                    fpBurstMode.next = True 
                    burstModeChanged.next = False
                else:
                    if(outputInputReady):
                        if(dataDirection):
                            state.next = tBufStates.WRITE_BRAM
                            bufferReady_int.next = True
                        else:
                            state.next = tBufStates.READ_BRAM
                            bufferReady_int.next = True
                    else:
                        state.next = tBufStates.INIT
            if(state == tBufStates.WAIT_FOR_SDRAM):
                if(((ack == 0 and not waitForBurst) or (burstTerminated == 0 and waitForBurst)) and outputInputReady):
                    bufferReady_int.next = True
                    if(dataDirection):
                        state.next = tBufStates.PREPARE_WRITE_SDRAM
                    else:
                        state.next = tBufStates.PREPARE_READ_SDRAM
                        fpBurstMode.next = False
                        burstModeChanged.next = True
                    address.next = 0
                    en.next = True
                    rw.next = True
                else:
                    state.next = tBufStates.WAIT_FOR_SDRAM
            if(state == tBufStates.PREPARE_WRITE_SDRAM):
                if(ack == 0):
                    state.next = tBufStates.PREPARE_WRITE_SDRAM2
                    en.next = False
                else:
                    if((bram_address_top + 1) % FPGA_BUFFER_LENGTH == bram_address_bottom):
                        state.next = tBufStates.PREPARE_WRITE_SDRAM2
                    else:
                        bram_address_top.next = (bram_address_top + 1) % FPGA_BUFFER_LENGTH
                        state.next = tBufStates.PREPARE_WRITE_SDRAM
                        fpga_mem[bram_address_top].next = dataIn
            if(state == tBufStates.PREPARE_WRITE_SDRAM2):
                if(ack == 1):
                    state.next = tBufStates.WRITE_SDRAM
                    if(bram_address_bottom != bram_address_top):
                        ram_dataIn.next = fpga_mem[bram_address_bottom]
                        bram_address_bottom.next = (bram_address_bottom + 1) % FPGA_BUFFER_LENGTH
                    else:
                        ram_dataIn.next = dataIn
                else:
                    if((bram_address_top + 1) % FPGA_BUFFER_LENGTH == bram_address_bottom):
                        bufferReady_int.next = False
                        state.next = tBufStates.WRITE_SDRAM
                    else:
                        state.next = tBufStates.PREPARE_WRITE_SDRAM2
                        fpga_mem[bram_address_top].next = dataIn
                        bram_address_top.next = (bram_address_top + 1) % FPGA_BUFFER_LENGTH
            if(state == tBufStates.WRITE_SDRAM):
                if(burstTerminated):
                    address.next = (address + SDRAM_BURST) % SDRAM_SIZE
                    en.next = True
                    if((address <= currentSdramOverlapBoundary) and ((address + SDRAM_BURST) >= currentSdramOverlapBoundary)):
                        overlapped.next = True
                        if(allowWriteOverlap):
                            state.next = tBufStates.PREPARE_WRITE_SDRAM
                        else:
                            state.next = tBufStates.FULL_EMPTY_SDRAM
                            address.next = address - 1
                    else:
                        if(bufferReady_int):
                            state.next = tBufStates.PREPARE_WRITE_SDRAM  
                        else:
                            state.next = tBufStates.FULL_EMPTY_SDRAM
                else:
                    if(bram_address_bottom != bram_address_top):
                        bram_address_bottom.next = (bram_address_bottom + 1) % FPGA_BUFFER_LENGTH
                        ram_dataIn.next = fpga_mem[bram_address_bottom]
                    else:
                        if(bufferReady_int):
                            ram_dataIn.next = dataIn
                        else:
                            ram_dataIn.next = 0
            if(state == tBufStates.FULL_EMPTY_SDRAM):
                bufferReady_int.next = False
                if(dataDirection):
                    state.next = tBufStates.FULL_EMPTY_SDRAM
                    ram_address_top.next = address
                else:
                    if(alreadyRead):
                        state.next = tBufStates.FULL_EMPTY_SDRAM
                    else:
                        alreadyRead.next = True
                        state.next = tBufStates.WAIT_FOR_SDRAM
                        waitForBurst.next = True
                        bram_address_top.next = 0
                        bram_address_bottom.next = 0
            if(state == tBufStates.PREPARE_READ_SDRAM):
                if(ack == 0):
                    burstModeChanged.next = False
                    rw.next = False
                    #ram_address_top.next = address
                    address.next = address + 1
                    en.next = True
                    state.next = tBufStates.READ_SDRAM
                else:
                    state.next = tBufStates.PREPARE_READ_SDRAM
                if(outputInputReady and bram_address_bottom < bram_address_top):
                    dataOut.next = fpga_mem[bram_address_bottom]
                    bram_address_bottom.next = bram_address_bottom + 1
            if(state == tBufStates.READ_SDRAM):
                if(ack == 0):
                    if(bram_address_top + 1 != bram_address_bottom):
                        en.next = False
                        state.next = tBufStates.PREPARE_NEXT_READ
                    else:
                        state.next = tBufStates.READ_SDRAM
                else:
                    state.next = tBufStates.READ_SDRAM
                if(outputInputReady and bram_address_bottom < bram_address_top):
                    dataOut.next = fpga_mem[bram_address_bottom]
                    bram_address_bottom.next = bram_address_bottom + 1
            if(state == tBufStates.PREPARE_NEXT_READ):
                if(ack == 1):
                    bram_address_top.next = (bram_address_top + 1) % FPGA_BUFFER_LENGTH
                    fpga_mem[bram_address_top].next = ramData
                if(burstTerminated):
                    address.next = address + 1
                    if(address < ram_address_top):
                        en.next = True
                        state.next = tBufStates.READ_SDRAM
                    else:
                        state.next = tBufStates.FULL_EMPTY_SDRAM
                else:
                    state.next = tBufStates.PREPARE_NEXT_READ
                if(outputInputReady and bram_address_bottom < bram_address_top):
                    dataOut.next = fpga_mem[bram_address_bottom]
                    bram_address_bottom.next = bram_address_bottom + 1
            if(state == tBufStates.WRITE_BRAM):
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
            if(state == tBufStates.FULL_EMPTY_BRAM):
                bufferReady_int.next = False
                if(dataDirection):
                    state.next = tBufStates.FULL_EMPTY_BRAM
                else:
                    if(alreadyRead):
                        state.next = tBufStates.FULL_EMPTY_BRAM
                    else:
                        if(outputInputReady):
                            state.next = tBufStates.READ_BRAM
                            bram_address_bottom.next = (bram_address_top + 1) % FPGA_BUFFER_LENGTH
                        else:
                            state.next = tBufStates.FULL_EMPTY_BRAM
            if(state == tBufStates.READ_BRAM):
                bufferReady_int.next = True
                if(outputInputReady):
                    if(bram_address_bottom == bram_address_top):
                        dataOut.next = fpga_mem[bram_address_bottom]
                        state.next = tBufStates.FULL_EMPTY_BRAM
                    else:
                        dataOut.next = fpga_mem[bram_address_bottom]
                        bram_address_bottom.next = (bram_address_bottom + 1)  % FPGA_BUFFER_LENGTH
                        state.next = tBufStates.READ_BRAM
                else:
                    state.next = tBufStates.READ_BRAM
    
    return sdram_controller, fsm, delay, setBoundary, connectBufferReady