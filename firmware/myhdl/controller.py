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

#command codes
RESET              = 0xAE                
SET_VREF           = 0x10                 
SET_VLOG           = 0x11               
SET_ATTENUATORS    = 0x20                
SET_COUPLINGS      = 0x21             
SET_DECIMATION     = 0x30            
SET_ANALOGTRIGGER  = 0x31         
SET_DIGITALTRIGGER = 0x32                      
START_CAPTURE      = 0x40

CLEANUP = 0xAA
#responses
DATA_DUMP          = 0x50
DATA_OK            = 0x80
DATA_BAD           = 0x90

tCtrlStates = enum('INIT0', 'INIT1', 'IDLE', 'SET_VREF0', 'SET_VREF1', 'SET_VLOG0', 
                   'SET_VLOG1', 'SET_ATTENUATORS', 'SET_COUPLINGS', 'SET_DECIMATION', 
                   'SET_ANALOGTRIGGER', 'SET_DIGITALTRIGGER', 'DUMP_DATA_INT1', 'DUMP_DATA_INT2',
                   'DUMP_DATA0', 'DUMP_DATA1', 'DUMP_DATA2', 'DUMP_DATA3', 'DUMP_DATA4', 'DUMP_DATA_PREPFT', 
                   'START_CAPTURE', 'CAPTURE', 'READ_COMMAND', 'READ_CMD_DATA', 'DECODE_CMD', 
                   'REQUEST_RESEND0', 'REQUEST_RESEND1', 'REQUEST_RESEND2', 'SEND_ACKNOWLEDGE0',
                   'SEND_ACKNOWLEDGE1', 'SEND_ACKNOWLEDGE2', 'PREPARE_ADC')

def controller(clk, reset, ft245din, ft245dout, ft245rw, ft245busy, ft245oe, ft245wr,
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
               adc1relco, adc2relco, led0, led1, logicAnalyzer, la1dir, la2dir, la1oe, la2oe, fastClk):
    """
    Main DSO controller.
    clk - clock 60MHz from FTDI
    reset - asynchronous reset
    ft245* respective FT245 interfaces
    dac* respective dac interfaces
    digipot* respective digipot interfaces
    buffer* respective buffer interfaces
    decim* respective decimators interfaces
    trigger* triggers interfaces
    adc1pd - power down of ADC1
    adc2pd - power down of ACC2
    adc1relatt - relay of ADC1 attenuator
    adc2relatt - relay of ADC2 attenuator
    adc1relco - relay of ADC1 coupling
    adc2relco - relay of ADC2 coupling
    led0 - info LED 0
    led1 - info LED 1
    logicAnalyzer - data from logic analyzer decimator
    la* interface of logic analyzer transciever
    fastClk - 125MHz clock from ADC oscillator
    
    Commnnication protocol:
     Commands:
     Bit lengths        | 8bit                 | 72bit     |   
     Field description  | Message number       | Data      | 

     RESET              | 0x00                 | DON'T CARE
     SET_VREF           | 0x10                 | 8bit - TopA, 8bit - BotA, 8bit - TopB, 8bit - BotB
     SET_VLOG           | 0x11                 | 8bit - 7bit - value
     SET_ATTENUATORS    | 0x20                 | 8bit - 1bit - att. relay ADC1, 8bit - 1bit att. relay ADC2
     SET_COUPLINGS      | 0x21                 | 1bit - ac/dc relay ADC1, 1bit ac/dc relay ADC2
     SET_DECIMATION     | 0x30                 | 8bit - 4bit - decimationRatioBase, 8bit -2bit - decimationStyle
     SET_ANALOGTRIGGER  | 0x31                 | Trigger adc1: 8bit - value, 8bit -  1bit - triggerType, 1bit - slope | 
                                                 Trigger adc2: 8bit - value, 8bit - 1bit - triggerType, 1bit - slope
     SET_DIGITALTRIGGER | 0x32                 | 64bit - pattern, 2bit - samples
     START_CAPTURE      | 0x40                 | 8bit - channel selection
                                                 0x00 - analog1 only, 0x01 analog2 only, 0x02 analog1 + digi,
                                                 0x03 - analog2 + digi, 0x04 analog1+analog2, 0x05 an1+an2+dig
                                                 0x06 - dig only
     Respones:
     DATA_DUMP          | 0x50                 | burst of data (longer than 72bit) where first two bytes are respective ADC data and
                                                 second two bytes are data of logic analyzer
     DATA_OK            | 0x80                 | DON'T CARE

    """
    BLINK_COUNTER_MAX = 2**24
    BLINK_MODULO = 2**21
    HOLDOFF_MAX = 2**26
    
    holdoffCounter = Signal(intbv(0, min = 0, max = HOLDOFF_MAX))
    responseCounter = Signal(intbv(0, min = 0, max = 6))
    state = Signal(tCtrlStates.INIT0)
    backState = Signal(tCtrlStates.INIT0)
    command = Signal(intbv(0)[8:])
    commandData = Signal(intbv(0)[72:])
    commandDataCounter = Signal(intbv(0, min = 0, max = 9))
    triggerConfiguration = Signal(intbv(0)[8:])
    oldBufferData = Signal(intbv(0)[32:])
    nTrigger = Signal(bool(False))
    answerNumber = Signal(intbv(0, min = 0, max = 4))
    wait245 = Signal(intbv(0, min = 0, max = 4))
    preparationCycles = Signal(intbv(0, min = 0, max = 256))
    
    @always(fastClk.posedge)
    def connectCapture():
        """This process generates trigger approximately after 1second if no trigger happend"""
        if(state == tCtrlStates.CAPTURE):
            holdoffCounter.next = holdoffCounter + 1
            if(nTrigger or (holdoffCounter == HOLDOFF_MAX-1)):
                bufferAllowWriteOverlap.next = False
            else:
                bufferAllowWriteOverlap.next = bufferAllowWriteOverlap
        else:
            holdoffCounter.next = 0
            bufferAllowWriteOverlap.next = True
    
    @always(fastClk.posedge)
    def triggering():
        """This process connects data from ADCs and logic analyzer
           to single 4byte word connected to input of buffer, 
           and selects appropriate trigger inputs for triggering"""
        bufferDataIn.next[32:24] = decimator1out
        bufferDataIn.next[24:16] = decimator2out
        bufferDataIn.next[16:0] = logicAnalyzer
        if(triggerConfiguration == 0x00):
            nTrigger.next = analog1Trigger
        elif(triggerConfiguration == 0x01):
            nTrigger.next = analog2Trigger
        elif(triggerConfiguration == 0x02):
            nTrigger.next = analog1Trigger or digiTrigger
        elif(triggerConfiguration == 0x03):
            nTrigger.next = analog2Trigger or digiTrigger
        elif(triggerConfiguration == 0x04):
            nTrigger.next = analog1Trigger or analog2Trigger
        elif(triggerConfiguration == 0x05):
            nTrigger.next = analog1Trigger or analog2Trigger or digiTrigger
        else:
            nTrigger.next = digiTrigger
    
    @always_comb
    def newFlag():
        """This process creates flag for new value set into decimator"""
        if(state == tCtrlStates.SET_DECIMATION):
            decimatorNewValueFlag.next = True
        else:
            decimatorNewValueFlag.next = False
    
    @always(clk.posedge, reset.posedge)
    def fsm():
        """This is state machine of main controller. 
           After initialization in INIT0 and INIT1 controller
           waits for indication of incoming data from ft245 controller (IDLE), 
           when this happend data are read into command 
           and commandData registers (READ_DATA and READ_CMD_DATA)
           then command is decoded (DECODE_CMD) if successful 
           acknowledge is sent back to PC(SEND_ACKNOWLEDGE0, 1 and 2),
           if unexpected data is received then request for 
           resend is sent to PC(REQUEST_RESEND0, 1 and 2).
           Then appropriate command is executed, decimation, triggers, attenuators and couplings are
           set directly. Voltage references and logic voltage are set to respective
           controllers and controller waits until operation is complete.
           When START_CAPTURE command is received then ADC power dows are released and ADCs are
           activated this state (PREPARE_ADC) is held for 255 clock cycles to warm up ADCs 
           (first 100-150 of ADC samples after power down is released are garbage).
           then in START_CAPTURE transicever of logic analyzer is activated
           and directed to input data from outer circuit to FPGA (by la* signals) and
           buffer direction is set to WRITE and outputInputReady of buffer is set to HIGH.
           (which makes buffer continualy input data from decimators).
           Controller stays in CAPTURE state until buffer indicates that it is FULL by 
           dropping bufferReady to LOW.
           Then in state DUMP_DATA_PREPFT, controller is waiting until FT245 controller goes to IDLE state.
           In state DUMP_DATA0 buffer is switched to READ mode and FT245 controller is switchet to WRITE mode.
           In DUMP_DATA1 four bytes indicating dump of data are sent.
           DUMP_DATA2 then outputs first byte of 32bit data packet, and puts bufferOutputInputReady to LOW.
           DUMP_DATA_INT1 outputs second byte of 32bit data packet.
           DUMP_DATA_INT2 outputs third byte of 32bit data packet
           DUMP_DATA3 outputs fourth byte of 32bit data packet and puts bufferOutputInputReady
           to HIGH getting next 32bit dat packet out of buffer.
           DUMP_DATA4 is used to stop transmission when in any of DUMP_DATA2, DUMP_DATA_INT1, DUMP_DATA_INT2 or DUMP_DATA3
           TXE goes high indicating full incoming buffer in FTDI chip. Then transmission must continue where it stopped
           to make data incoming to PC consistent. 
           """
        if(reset == 1):
            state.next = tCtrlStates.INIT0
            ft245reset.next = True
            dacReset.next = True
            bufferReset.next = True
            digipotReset.next = True
            analogTrigger1Reset.next = True
            analogTrigger2Reset.next = True
            digitalTriggerReset.next = True
            adc1pd.next = True
            adc2pd.next = True
            adc1relatt.next = True
            adc2relatt.next = True
            adc1relco.next = True
            adc2relco.next = True
            led0.next = False
            led1.next = True
            commandDataCounter.next = 0
            triggerConfiguration.next = 0
            la1dir.next = True
            la2dir.next = True
            la1oe.next = True 
            la2oe.next = True
            answerNumber.next = 0
            responseCounter.next = 0
            preparationCycles.next = 0
        else:
            if(state == tCtrlStates.INIT0):
                responseCounter.next = 0
                ft245reset.next = True
                dacReset.next = True
                bufferReset.next = True
                digipotReset.next = True
                analogTrigger1Reset.next = True
                analogTrigger2Reset.next = True
                digitalTriggerReset.next = True
                adc1pd.next = True
                adc2pd.next = True
                adc1relatt.next = True
                adc2relatt.next = True
                adc1relco.next = True
                adc2relco.next = True
                led0.next = True
                led1.next = False
                la1dir.next = True
                la2dir.next = True
                la1oe.next = True 
                la2oe.next = True
                commandDataCounter.next = 0
                triggerConfiguration.next = 0
                ft245strobe.next = False
                ft245dataWaitIn.next = False
                ft245rw.next = False
                dacStrobe.next = False
                digipotStrobe.next = False
                bufferDataDirection.next = False
                bufferOutputInputReady.next = False
                decimationStyle.next = 0
                decimationRatioBase.next = 0
                trigger1Value.next = 100
                trigger1Type.next = 0 #threshold trigger
                trigger1Slope.next = 0 #falling
                trigger2Value.next = 100
                trigger2Type.next = 0 #threshold trigger
                trigger2Slope.next = 0 #falling
                triggerPattern.next = 0x00000000
                triggerSamples.next = 0
                state.next = tCtrlStates.INIT1
            elif(state == tCtrlStates.INIT1):
                led0.next = True
                led1.next = True
                ft245reset.next = False
                dacReset.next = False
                digipotReset.next = False
                analogTrigger1Reset.next = False
                analogTrigger2Reset.next = False
                digitalTriggerReset.next = False
                if(dacBusy or digipotBusy):
                    state.next = tCtrlStates.INIT1
                    led0.next = True
                    led1.next = True
                else:
                    state.next = tCtrlStates.IDLE
                    led0.next = False
                    led1.next = False
            elif(state == tCtrlStates.IDLE):
                bufferReset.next = True
                preparationCycles.next = 0
                ft245rw.next = False
                ft245strobe.next = True
                ft245dataWaitIn.next = False
                if(ft245busy and (ft245oe == 0)):
                    wait245.next = 0
                    state.next = tCtrlStates.READ_COMMAND
                    ft245strobe.next = False
                else:
                    state.next = tCtrlStates.IDLE
            elif(state == tCtrlStates.READ_COMMAND):
                if(ft245oe == 0):
                    if(wait245 < 1):
                        wait245.next = wait245 + 1
                        command.next = ft245dout
                    else:
                        wait245.next = 0
                        state.next = tCtrlStates.READ_CMD_DATA
                        commandDataCounter.next = 0
                else:
                    state.next = tCtrlStates.IDLE
            elif(state == tCtrlStates.READ_CMD_DATA):
                if(ft245oe == 0):
                    if(commandDataCounter < 8):
                        commandDataCounter.next = commandDataCounter + 1
                    if(commandDataCounter == 0):
                        commandData.next[72:64] = ft245dout
                    elif(commandDataCounter == 1):
                        commandData.next[64:56] = ft245dout
                    elif(commandDataCounter == 2):
                        commandData.next[56:48] = ft245dout
                    elif(commandDataCounter == 3):
                        commandData.next[48:40] = ft245dout
                    elif(commandDataCounter == 4):
                        commandData.next[40:32] = ft245dout
                    elif(commandDataCounter == 5):
                        commandData.next[32:24] = ft245dout
                    elif(commandDataCounter == 6):
                        commandData.next[24:16] = ft245dout
                    elif(commandDataCounter == 7):
                        commandData.next[16:8] = ft245dout
                    elif(commandDataCounter == 8):
                        commandData.next[8:] = ft245dout
                        state.next = tCtrlStates.DECODE_CMD
                else:
                    led0.next = not led0
                    if(commandDataCounter > 2):
                        state.next = tCtrlStates.DECODE_CMD
                    else:
                        state.next = tCtrlStates.REQUEST_RESEND0
            elif(state == tCtrlStates.DECODE_CMD):
                if(command == RESET):
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE0
                elif(command == SET_VREF):
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE0
                elif(command == SET_VLOG):
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE0
                elif(command == SET_ATTENUATORS):
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE0
                elif(command == SET_COUPLINGS):
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE0
                elif(command == SET_DECIMATION):
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE0
                elif(command == SET_ANALOGTRIGGER):
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE0
                elif(command == SET_DIGITALTRIGGER):
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE0
                elif(command == START_CAPTURE):
                    state.next = tCtrlStates.PREPARE_ADC
                elif(command == CLEANUP):
                    state.next = tCtrlStates.IDLE
                else:
                    state.next = tCtrlStates.REQUEST_RESEND0
                    led0.next = not led0
                    led1.next = not led1
            elif(state == tCtrlStates.SEND_ACKNOWLEDGE0):
                if(ft245busy):
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE0
                else:
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE1
            elif(state == tCtrlStates.SEND_ACKNOWLEDGE1):
                ft245rw.next = True
                responseCounter.next = 0
                ft245din.next = DATA_OK
                if(ft245wr == 0):
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE2
                    ft245strobe.next = False
                else:
                    ft245strobe.next = True
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE1
            elif(state == tCtrlStates.SEND_ACKNOWLEDGE2):
                if(responseCounter < 1):
                    responseCounter.next = responseCounter + 1
                    ft245din.next = DATA_OK
                    state.next = tCtrlStates.SEND_ACKNOWLEDGE2
                else:
                    responseCounter.next = 0
                    if(command == RESET):
                        state.next = tCtrlStates.INIT0
                    elif(command == SET_VREF):
                        state.next = tCtrlStates.SET_VREF0
                    elif(command == SET_VLOG):
                        state.next = tCtrlStates.SET_VLOG0
                    elif(command == SET_ATTENUATORS):
                        state.next = tCtrlStates.SET_ATTENUATORS
                    elif(command == SET_COUPLINGS):
                        state.next = tCtrlStates.SET_COUPLINGS
                    elif(command == SET_DECIMATION):
                        state.next = tCtrlStates.SET_DECIMATION
                    elif(command == SET_ANALOGTRIGGER):
                        state.next = tCtrlStates.SET_ANALOGTRIGGER
                    elif(command == SET_DIGITALTRIGGER):
                        state.next = tCtrlStates.SET_DIGITALTRIGGER
                    elif(command == START_CAPTURE):
                        state.next = tCtrlStates.START_CAPTURE
                    else:
                        state.next = tCtrlStates.IDLE
            elif(state == tCtrlStates.REQUEST_RESEND0):
                state.next = tCtrlStates.IDLE
                if(ft245busy):
                    state.next = tCtrlStates.REQUEST_RESEND0
                else:
                    state.next = tCtrlStates.REQUEST_RESEND1
            elif(state == tCtrlStates.REQUEST_RESEND1):
                ft245rw.next = True
                responseCounter.next = 0
                ft245din.next = DATA_BAD
                if(ft245wr == 0):
                    state.next = tCtrlStates.REQUEST_RESEND2
                    ft245strobe.next = False
                else:
                    ft245strobe.next = True
                    state.next = tCtrlStates.REQUEST_RESEND1
            elif(state == tCtrlStates.REQUEST_RESEND2):
                if(responseCounter < 1):
                    responseCounter.next = responseCounter + 1
                    ft245din.next = DATA_BAD
                    state.next = tCtrlStates.REQUEST_RESEND2
                else:
                    responseCounter.next = 0
                    ft245dataWaitIn.next = True
                    ft245rw.next = False
                    state.next = tCtrlStates.IDLE
            elif(state == tCtrlStates.SET_VREF0):
                dacVrefTopA.next = commandData[72:64]
                dacVrefBotA.next = commandData[64:56]
                dacVrefTopB.next = commandData[56:48]
                dacVrefBotB.next = commandData[48:40]
                if(dacBusy):
                    dacStrobe.next = False
                    state.next = tCtrlStates.SET_VREF1
                else:
                    state.next = tCtrlStates.SET_VREF0
                    dacStrobe.next = True
            elif(state == tCtrlStates.SET_VREF1):
                if(dacBusy):
                    state.next = tCtrlStates.SET_VREF1
                else:
                    led1.next = not led1
                    state.next = tCtrlStates.IDLE
            elif(state == tCtrlStates.SET_VLOG0):
                digipotValue.next = commandData[72:65]
                if(digipotBusy):
                    digipotStrobe.next = False
                    state.next = tCtrlStates.SET_VLOG1
                else:
                    digipotStrobe.next = True
                    state.next = tCtrlStates.SET_VLOG0
            elif(state == tCtrlStates.SET_VLOG1):
                if(digipotBusy):
                    state.next = tCtrlStates.SET_VLOG1
                else:
                    state.next = tCtrlStates.IDLE
            elif(state == tCtrlStates.SET_ATTENUATORS):
                led1.next = not led1
                adc1relatt.next = commandData[71]
                adc2relatt.next = commandData[63]
                state.next = tCtrlStates.IDLE
            elif(state == tCtrlStates.SET_COUPLINGS):
                led1.next = not led1
                adc1relco.next = commandData[71]
                adc2relco.next = commandData[63]
                state.next = tCtrlStates.IDLE
            elif(state == tCtrlStates.SET_DECIMATION):
                decimationRatioBase.next = commandData[68:64]
                decimationStyle.next = commandData[58:56]
                state.next = tCtrlStates.IDLE
            elif(state == tCtrlStates.SET_ANALOGTRIGGER):
                trigger1Value.next = commandData[72:64]
                trigger1Type.next = commandData[63]
                trigger1Slope.next = commandData[62]
                trigger2Value.next = commandData[56:48]
                trigger2Type.next = commandData[47]
                trigger2Slope.next = commandData[46]
                state.next = tCtrlStates.IDLE
            elif(state == tCtrlStates.SET_DIGITALTRIGGER):
                triggerPattern.next = commandData[72:8]
                triggerSamples.next = commandData[8:6]
                state.next = tCtrlStates.IDLE
            elif(state == tCtrlStates.PREPARE_ADC):
                bufferReset.next = True
                triggerConfiguration.next = commandData[72:64]
                if(triggerConfiguration == 0x00 or 
                   triggerConfiguration == 0x02 or 
                   triggerConfiguration == 0x04 or
                   triggerConfiguration == 0x05):
                    adc1pd.next = False
                if(triggerConfiguration == 0x01 or
                   triggerConfiguration == 0x03 or
                   triggerConfiguration == 0x04 or
                   triggerConfiguration == 0x05):
                    adc2pd.next = False
                if(preparationCycles == 255):
                    bufferReset.next = False
                    state.next = tCtrlStates.START_CAPTURE
                    preparationCycles.next = 0
                else:
                    state.next = tCtrlStates.PREPARE_ADC
                    preparationCycles.next = preparationCycles + 1
            elif(state == tCtrlStates.START_CAPTURE):
                la1dir.next = False
                la2dir.next = False
                la1oe.next = False
                la2oe.next = False
                bufferOutputInputReady.next = True
                bufferDataDirection.next = True
                if(bufferReady):
                    state.next = tCtrlStates.CAPTURE
                else:
                    state.next = tCtrlStates.START_CAPTURE
            elif(state == tCtrlStates.CAPTURE):
                if(bufferReady):
                    state.next = tCtrlStates.CAPTURE
                else:
                    state.next = tCtrlStates.DUMP_DATA_PREPFT
                    bufferOutputInputReady.next = False
            elif(state == tCtrlStates.DUMP_DATA_PREPFT):
                if(ft245busy):
                    state.next = tCtrlStates.DUMP_DATA_PREPFT
                else:
                    state.next = tCtrlStates.DUMP_DATA0
            elif(state == tCtrlStates.DUMP_DATA0):
                adc1pd.next = True
                adc2pd.next = True
                ft245rw.next = True
                bufferDataDirection.next = False
                answerNumber.next = 0
                ft245din.next = DATA_DUMP
                if(ft245wr == 0 and bufferReady):
                    #bufferOutputInputReady.next = True
                    state.next = tCtrlStates.DUMP_DATA1
                    ft245strobe.next = False
                else:
                    ft245strobe.next = True
                    state.next = tCtrlStates.DUMP_DATA0
            elif(state == tCtrlStates.DUMP_DATA1):
                ft245din.next = DATA_DUMP
                ft245dataWaitIn.next = False
                answerNumber.next = answerNumber + 1
                if(answerNumber == 3):
                    state.next = tCtrlStates.DUMP_DATA2
                    bufferOutputInputReady.next = True
                else:
                    state.next = tCtrlStates.DUMP_DATA1
            elif(state == tCtrlStates.DUMP_DATA2):
                if(ft245dataWaitOut):
                    state.next = tCtrlStates.DUMP_DATA4
                    bufferOutputInputReady.next = False
                    backState.next = tCtrlStates.DUMP_DATA2
                else:
                    bufferOutputInputReady.next = False
                    ft245din.next = bufferDataOut[32:24]
                    state.next = tCtrlStates.DUMP_DATA_INT1
            elif(state == tCtrlStates.DUMP_DATA_INT1):
                if(ft245dataWaitOut):
                    state.next = tCtrlStates.DUMP_DATA4
                    backState.next = tCtrlStates.DUMP_DATA_INT1
                else:
                    ft245din.next = bufferDataOut[24:16]
                    state.next = tCtrlStates.DUMP_DATA_INT2
            elif(state == tCtrlStates.DUMP_DATA_INT2):
                if(ft245dataWaitOut):
                    state.next = tCtrlStates.DUMP_DATA4
                    backState.next = tCtrlStates.DUMP_DATA_INT2
                else:
                    ft245din.next = bufferDataOut[16:8]
                    state.next = tCtrlStates.DUMP_DATA3
            elif(state == tCtrlStates.DUMP_DATA3):
                if(bufferReady):
                    if(ft245dataWaitOut):
                        state.next = tCtrlStates.DUMP_DATA4
                        bufferOutputInputReady.next = False
                        backState.next = tCtrlStates.DUMP_DATA3
                    else:
                        ft245din.next = bufferDataOut[8:0]
                        state.next = tCtrlStates.DUMP_DATA2
                        bufferOutputInputReady.next = True
                        oldBufferData.next = bufferDataOut
                else:
                    ft245din.next = bufferDataOut[8:0]
                    ft245dataWaitIn.next = True
                    state.next = tCtrlStates.IDLE
            elif(state == tCtrlStates.DUMP_DATA4):
                ft245rw.next = True
                if(backState == tCtrlStates.DUMP_DATA3):
                    ft245din.next = oldBufferData[24:16]
                elif(backState == tCtrlStates.DUMP_DATA_INT2):
                    ft245din.next = oldBufferData[32:24]
                elif(backState == tCtrlStates.DUMP_DATA_INT1):
                    ft245din.next = oldBufferData[8:0]
                else:
                    ft245din.next = oldBufferData[16:8]
                if(ft245busy and not ft245dataWaitOut):
                    if(backState == tCtrlStates.DUMP_DATA3):
                        state.next = tCtrlStates.DUMP_DATA3
                        ft245din.next = bufferDataOut[16:8]
                    elif(backState == tCtrlStates.DUMP_DATA_INT2):
                        state.next = tCtrlStates.DUMP_DATA_INT2
                        ft245din.next = bufferDataOut[24:16]
                    elif(backState == tCtrlStates.DUMP_DATA_INT1):
                        state.next = tCtrlStates.DUMP_DATA_INT1
                        ft245din.next = bufferDataOut[32:24]
                    else:
                        state.next = tCtrlStates.DUMP_DATA2
                        ft245din.next = bufferDataOut[8:0]
                    ft245strobe.next = False
                else:
                    ft245strobe.next = True
                    state.next = tCtrlStates.DUMP_DATA4
    
    return fsm, triggering, connectCapture, newFlag