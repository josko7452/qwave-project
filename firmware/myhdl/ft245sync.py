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

tFtState = enum('IDLE', 'WRITE_PREP', 'READ_PREP', 'WRITE', 'READ')

def ft245sync(clk, reset, rxf, txe, rd_int, wr_int, oe_int, siwu, adbus, dataIn, dataOut, rw, busy, dataWaitIn, dataWaitOut, strobe):
    """
    FTDI interface:
    clk - 60MHz clock from FTDI
    reset - asynchronous reset
    rxf - IN - When low, there is data available in the FIFO which can be read by driving RD# low. 
          When in synchronous mode, data is transferred on every clock that RXF# and RD# are both low. 
          Note that the OE# pin must be driven low at least 1 clock period before asserting RD# low.
    txe - IN - When low, data can be written into the FIFO by driving WR# low. 
          When in synchronous mode, data is transferred on every clock that TXE# and WR# are both low.
    rd - OUT -  Enables the current FIFO data byte to be driven onto D0...D7 when RD# goes low. 
          The next FIFO data byte (if available) is fetched from the receive FIFO buffer each CLKOUT cycle until RD# goes high.
    wr - OUT - Enables the data byte on the D0...D7 pins to be written into the transmit FIFO buffer when WR# is low. 
         The next FIFO data byte is written to the transmit FIFO buffer each CLKOUT cycle until WR# goes high.
    oe - OUT - Output enable when low to drive data onto D0-7. T
         his should be driven low at least 1 clock period before driving RD# low to allow for data buffer turn-around.
    adbus - I/O - D7 to D0 bidirectional FIFO data. This bus is normally output unless OE# is low.
    Controller interface:
    dataIn - IN - Data writen from controller
    dataOut - OUT - Data read from controller
    rw - IN - data direction 0 read, 1 write
    busy - OUT - busy flag
    strobe - IN - strobe of RW flag
    """
    
    adbus_drv = adbus.driver()
    adbus_out = Signal(intbv(0)[8:])
    state = Signal(tFtState.IDLE)
    #oe_int = Signal(bool(False))
    #wr_int = Signal(bool(False))
    #rd_int = Signal(bool(False))
    
    @always_comb
    def bus_direction():
        """Controls bus direction, by setting adbus output to high-z if not RXF is LOW"""
        if(rxf):
            adbus_drv.next = adbus_out
        else:
            adbus_drv.next = None #do not hold bus
    
    @always(clk.posedge)
    def tx():
        """Outpus data to ADBUS when writing"""
        if(state == tFtState.WRITE or state == tFtState.WRITE_PREP):
            adbus_out.next = dataIn
    
    @always(clk.posedge)
    def rx():
        """Receives data and outpus to dataOut output"""
        if(state == tFtState.READ or state == tFtState.READ_PREP):
            dataOut.next = adbus
                
    #@always(clk.negedge)
    #def connectDelayed():
    #    oe.next = oe_int
    #    wr.next = wr_int
    #    rd.next = rd_int
        
    @always_comb
    def dataWait():
        """Signaling that transmitter should wait before putting data to dataIn"""
        if(reset == 1):
            dataWaitOut.next = True
        else:
            if(state == tFtState.IDLE):
                dataWaitOut.next = True
            elif(state == tFtState.WRITE):
                if(txe == 0):
                    dataWaitOut.next = False
                else:
                    dataWaitOut.next = True
            else:
                dataWaitOut.next = True
    
    @always(clk.posedge, reset.posedge)
    def fsm():
        """State machine controlling writing and reading data,
           by reading and setting appropriate FT245 control signals"""
        if(reset == 1):
            state.next = tFtState.IDLE
            busy.next = True
            rd_int.next = True
            wr_int.next = True
            oe_int.next = True
            siwu.next = True
        else:
            if(state == tFtState.IDLE):
                if(strobe == 1):
                    busy.next = True
                    if(rw == 1):
                        state.next = tFtState.WRITE_PREP
                    else:
                        state.next = tFtState.READ_PREP
                else:
                    busy.next = False
                    state.next = tFtState.IDLE
            elif(state == tFtState.WRITE_PREP):
                if(txe == 0):
                    state.next = tFtState.WRITE
                else:
                    state.next = tFtState.WRITE_PREP
            elif(state == tFtState.READ_PREP):
                if(rxf == 0):
                    state.next = tFtState.READ
                    oe_int.next = False
                else:
                    state.next = tFtState.READ_PREP
            elif(state == tFtState.WRITE):
                if(txe == 0 and not dataWaitIn):
                    wr_int.next = False
                else:
                    wr_int.next = True
                    state.next = tFtState.IDLE
                    busy.next = False
            elif(state == tFtState.READ):
                if(rxf == 0):
                    if(dataWaitIn):
                        rd_int.next = True
                    else:
                        rd_int.next = False
                else:
                    rd_int.next = True
                    oe_int.next = True
                    state.next = tFtState.IDLE
                    busy.next = False
                    
    
    return fsm, tx, rx, bus_direction, dataWait