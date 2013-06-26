QWave-project
=============

Working FPGA development board with USB 2.0 interface and DSO module (125MHz analog dual channel with 16bit logic analyzer)
Although hadware was implemented it is not thoroughly tested, especially analog part is not intented as ready to build kit, it is inteded for hackers and people who are wiling to play around to get it working.

# Project parts
* **hardware** design sources of FPGA developement board and DSO module in eagle
* **firmware** sources of FPGA firmware in MyHDL and translated VHDL Xinix ISE project directory
* **software** sources of Qt based DSO sampling software

# Preview
## FPGA dev board and DSO module (dimensions 10x4cm):
-![alt text](http://i.imgur.com/6oWPM6L.jpg "FPGA board and DSO module")
## Software
[Screenshot gallery](http://imgur.com/a/RICwH#0)

#Details:
## FPGA board:
### What works:
* FPGA programming and 3.3V logic interface and FT245 Sync interface (USB 2.0 speed)

### TODO:
* Direct USB programming of FPGA flash - need to use JTAG and Xilinx USB Platform Cable for programming

## Analog (DSO module) board:
### What works:
* DSO functionality

### What was not tested:
* Logic analyzer on anything faster than few kHz
* Compensation of analog input

### TODO:
* Eliminating noise on analog circuits

## Firmware
### TODO:
* Fix trigger
* Add selectable trigger holdoff
* Do not insert trigger in single-shot mode

## Software
### TODO:
* Better performance
* Make more user-friendly selection of trigger
* Add exact voltage meter

## For hackers/ee guys and developers:
See HACKING.md


