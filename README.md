QWave-project
=============

Working FPGA development board with USB 2.0 interface and DSO module (125MHz analog dual channel with 16bit logic analyzer)

## Project parts
* **hardware** design sources of FPGA developement board and DSO module in eagle
* **firmware** sources of FPGA firmware in MyHDL and translated VHDL Xinix ISE project directory
* **software** sources of Qt based DSO sampling software

## What works:
* FPGA programming and 3.3V logic interface and FT245 Sync interface (USB 2.0 speed)
* DSO functionality

## What was not tested:
* Logic analyzer on anything faster than few kHz
* Compensation of analog input

## TODO:
* exact voltage meter
* direct USB programming of FPGA flash - need to use JTAG and Xilinx USB Platform Cable for programming
* eliminating noise on analog circuits