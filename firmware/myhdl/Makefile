#Makefile for FPGA Programmer

LIB=-L/opt/local/lib
INC=-I/opt/local/include

all: move

move: generate
	mkdir vhdl
	mv *.vhd ./vhdl
	cp top.ucf ./vhdl

generate: 
	python controller_convert_hierarchy.py

clean:
	rm -r ./vhdl
	rm *.pyc

# that's all
