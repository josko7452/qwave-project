#Makefile for FPGA Programmer

LIB=-L/opt/local/lib
INC=-I/opt/local/include

all: flash-fpga

flash-fpga: ftdispi.o main.o
	gcc $(LIB) $(INC) -lftdi main.o ftdispi.o -o flash-fpga 

main.o: ftdispi.h main.c 
	gcc -lftdi $(LIB) $(INC) -c main.c

ftdispi.o: ftdispi.c ftdispi.h
	gcc -lftdi $(LIB) $(INC) -c ftdispi.c

clean:
	rm flash-fpga main.o ftdispi.o


# that's all
