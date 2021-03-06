-- File: buff.vhd
-- Generated by MyHDL 0.7
-- Date: Thu May  9 18:08:45 2013



package pck_buff is

    type t_enum_tBufStates_1 is (
    INIT,
    WRITE_BRAM,
    READ_BRAM,
    FULL_EMPTY_BRAM
);

end package pck_buff;

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use std.textio.all;

use work.pck_myhdl_07.all;

use work.pck_buff.all;

entity buff is
    port (
        clk: in std_logic;
        reset: in std_logic;
        dataIn: in unsigned(31 downto 0);
        dataOut: out unsigned(31 downto 0);
        bufferReady: out std_logic;
        outputInputReady: in std_logic;
        allowWriteOverlap: in std_logic;
        dataDirection: in std_logic;
        clkDecimator: in std_logic;
        clkFtdi: in std_logic;
        alwaysEn: in std_logic
    );
end entity buff;
-- clk - input - clock
-- reset - asynchronous reset
-- dataIn - input - data to be written to BUFFER
-- dataOut - output - data read from BUFFER
-- bufferReady - output - indicates that buffer is ready to input data or output data
-- outputInputReady - input - output/input is ready
-- allowWriteOverlap - input - allow memory to overlap, useful when in trigger armed mode of scope
--                             memory will overwrite previous data when full instead lowering bufferReady
-- dataDirection - input - indicates wether to input or output data
-- clkDecimator - input - clock of decimated data which data are sampled into buffer
-- clkFtdi - input - 60MHz clock of FTDI FT245mode and controller
-- alwaysEn - input - used in case of no decimation applied,
--                    data are sampled to buffer at clk clock (instead of clkDecimator)
--                    
-- Two clock domains are used therefore Xilinx tools infers dualport-dualclock FIFO

architecture MyHDL of buff is

signal bufferReady_int: std_logic;
signal bram_address_bottom: unsigned(14 downto 0);
signal bottomSet: std_logic;
signal state: t_enum_tBufStates_1;
signal currentBramOverlapBoundary: unsigned(14 downto 0);
signal boundarySet: std_logic;
signal overlapped: std_logic;
signal clkEn: std_logic;
signal bram_address_top: unsigned(14 downto 0);
signal holdFullEmpty: unsigned(3 downto 0);
signal alreadyRead: std_logic;
type t_array_fpga_mem is array(0 to 18432-1) of unsigned(31 downto 0);
signal fpga_mem: t_array_fpga_mem;

begin



-- This process is control FSM of buffer
-- In INIT state it initializes buffer,
-- In WRITE_BRAM state it writes data in overlapping mode until allowWriteOverlap
-- goes low than it fills memory until overlap boundary is reached
-- In FULL_EMPTY_BRAM state it waits for controller to change dataDirection 
-- and then goes to READ_BRAM if BRAM if FULL or 
-- holds bufferReady in 0 if BRAM is EMPTY and goes to INIT
-- In READ bram mode it waits until BRAM is EMPTY
BUFF_FSM: process (clk) is
begin
    if rising_edge(clk) then
        if (reset = '1') then
            state <= INIT;
            overlapped <= '0';
            bram_address_top <= "000000000000000";
            bufferReady_int <= '0';
            alreadyRead <= '0';
            holdFullEmpty <= "0000";
            clkEn <= '1';
        else
            case state is
                when INIT =>
                    bram_address_top <= "000000000000000";
                    alreadyRead <= '0';
                    overlapped <= '0';
                    if to_boolean(outputInputReady) then
                        if to_boolean(dataDirection) then
                            state <= WRITE_BRAM;
                            bufferReady_int <= '1';
                        else
                            state <= READ_BRAM;
                            bufferReady_int <= '1';
                        end if;
                    else
                        state <= INIT;
                    end if;
                when WRITE_BRAM =>
                    bufferReady_int <= '1';
                    if (clkDecimator = '0') then
                        clkEn <= '1';
                    end if;
                    if ((clkDecimator = '1') and (outputInputReady = '1') and ((clkEn = '1') or to_boolean(alwaysEn))) then
                        clkEn <= '0';
                        if (bram_address_top = 0) then
                            overlapped <= '0';
                        end if;
                        if (bram_address_top = currentBramOverlapBoundary) then
                            overlapped <= '1';
                            if to_boolean(allowWriteOverlap) then
                                fpga_mem(to_integer(bram_address_top)) <= dataIn;
                                state <= WRITE_BRAM;
                                bram_address_top <= ((bram_address_top + 1) mod 18432);
                            else
                                fpga_mem(to_integer(bram_address_top)) <= dataIn;
                                state <= FULL_EMPTY_BRAM;
                            end if;
                        else
                            fpga_mem(to_integer(bram_address_top)) <= dataIn;
                            state <= WRITE_BRAM;
                            bram_address_top <= ((bram_address_top + 1) mod 18432);
                        end if;
                    else
                        state <= WRITE_BRAM;
                    end if;
                when FULL_EMPTY_BRAM =>
                    if to_boolean(dataDirection) then
                        state <= FULL_EMPTY_BRAM;
                        bufferReady_int <= '0';
                    else
                        if to_boolean(alreadyRead) then
                            bufferReady_int <= '0';
                            if (holdFullEmpty = 8) then
                                state <= INIT;
                                holdFullEmpty <= "0000";
                            else
                                state <= FULL_EMPTY_BRAM;
                                holdFullEmpty <= (holdFullEmpty + 1);
                            end if;
                        else
                            bufferReady_int <= '1';
                            if (to_boolean(outputInputReady) and to_boolean(bottomSet)) then
                                state <= READ_BRAM;
                            else
                                state <= FULL_EMPTY_BRAM;
                            end if;
                        end if;
                    end if;
                when others => -- READ_BRAM
                    alreadyRead <= '1';
                    if (outputInputReady = '1') then
                        if (bram_address_bottom = bram_address_top) then
                            state <= FULL_EMPTY_BRAM;
                        else
                            state <= READ_BRAM;
                        end if;
                    else
                        state <= READ_BRAM;
                    end if;
            end case;
        end if;
    end if;
end process BUFF_FSM;

-- This process sets overlap boundary when trigger occurs, pre trigger value is defined by
-- BLOCKS_BEFORE_DISSALOWED_OVERLAP.
BUFF_SETBOUNDARY: process (clk, reset) is
begin
    if (reset = '1') then
        currentBramOverlapBoundary <= to_unsigned(18432 - 1, 15);
        boundarySet <= '0';
    elsif rising_edge(clk) then
        if ((allowWriteOverlap = '0') and (boundarySet = '0')) then
            if (bram_address_top > 4096) then
                currentBramOverlapBoundary <= (bram_address_top - 4096);
                boundarySet <= '1';
            else
                if to_boolean(overlapped) then
                    currentBramOverlapBoundary <= ((bram_address_top + 18431) - 4096);
                    boundarySet <= '1';
                end if;
            end if;
        end if;
    end if;
end process BUFF_SETBOUNDARY;

-- This process connexts internal bufferReady signal

bufferReady <= bufferReady_int;

-- This process outputs data to be transmitted to PC and 60MHz FTDI clock,
-- if outputInputReady is low then buffer stops 
-- outputing data until outputInputReady goes high
BUFF_FSMSLOWPART: process (clkFtdi) is
begin
    if rising_edge(clkFtdi) then
        if (reset = '1') then
            bram_address_bottom <= "000000000000000";
            dataOut <= "00000000000000000000000000000000";
            bottomSet <= '0';
        else
            if (state = INIT) then
                bram_address_bottom <= "000000000000000";
            elsif (state = FULL_EMPTY_BRAM) then
                if ((dataDirection = '0') and (alreadyRead = '0')) then
                    bram_address_bottom <= ((bram_address_top + 1) mod 18432);
                    bottomSet <= '1';
                end if;
            elsif (state = READ_BRAM) then
                bottomSet <= '0';
                if (outputInputReady = '1') then
                    if (bram_address_bottom = bram_address_top) then
                        dataOut <= fpga_mem(to_integer(bram_address_bottom));
                    else
                        dataOut <= fpga_mem(to_integer(bram_address_bottom));
                        bram_address_bottom <= ((bram_address_bottom + 1) mod 18432);
                    end if;
                end if;
            end if;
        end if;
    end if;
end process BUFF_FSMSLOWPART;

end architecture MyHDL;
