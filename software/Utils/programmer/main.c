#include <stdio.h>
#include "ftdispi.h"
#include <stdint.h>
#include <stdlib.h>
#include <ftdi.h>

#define PROGRAM_THROUGH_BUFFER 0x82
#define WORD_COUNT 33

#define TRUE 1
#define FALSE 0

#define GPO_STATE 0x00
#define SPEED 2000000

int main(int argc, char **argv)
{   
    FILE * pFile;
    if(argc < 2)
    {
        fprintf(stderr, "Error: you have to provide filename\n");
        exit(1);
    }
    else
    {
        char * file = argv[1];
        pFile = fopen (file, "rb");
        if(pFile == NULL) 
        {
            fprintf(stderr, "Can't open file specified.\n");
            exit(1);
        }
    }
    
    uint8_t buffer[WORD_COUNT*8];
    fprintf(stderr, "FTDI AT45 programmer\n");
    struct ftdi_context fc;
    struct ftdispi_context fsc;
    int i;
    
    if (ftdi_init(&fc) < 0) 
    {
        fprintf(stderr, "ftdi_init failed\n");
        return 1;
    }
    ftdi_set_interface(&fc, INTERFACE_B);
    i = ftdi_usb_open_desc_index(&fc, 0x0403, 0x6010, NULL, NULL, 0);
    if (i < 0 && i != -5) 
    {
        fprintf(stderr, "OPEN: %s\n", ftdi_get_error_string(&fc));
        exit(-1);
    }
    fprintf(stderr, "FTDI Initialized\n");
    ftdispi_open(&fsc, &fc, INTERFACE_B);
    ftdispi_setmode(&fsc, 1, 0, 0, 0, 0, GPO_STATE);
    ftdispi_setclock(&fsc, SPEED);
    ftdispi_setloopback(&fsc, 0);
    fprintf(stderr, "SPI mode initialized.\n");
    
    fseek(pFile , 0 , SEEK_END);
    unsigned long lSize = ftell (pFile);
    rewind(pFile);
    unsigned long bytesRead = 0;
    int end = FALSE;
    uint32_t address = 0;
    
    if(FTDISPI_ERROR_NONE != ftdispi_setgpo(&fsc, GPO_STATE))
    {
        fprintf(stderr, "Can't set GPO status\n");
    }
    fprintf(stderr, "GPO set for PFGA's Hi-Z. Writing...\n");
    while(!end)
    {
        unsigned long currentCount;
        uint8_t command[4];
        command[0] = PROGRAM_THROUGH_BUFFER;
        command[1] = (int8_t) (address >> 16);
        command[2] = (int8_t) (address >> 8);
        command[3] = (int8_t) (address);
        address++;
        fprintf(stderr, "\rWriting: %d %%", (int)(bytesRead*100/(lSize)));
        fflush(stderr);
        ftdispi_write(&fsc, command, 4, GPO_STATE);
        //read page from input
        if(bytesRead + WORD_COUNT*8 <= lSize)
        {
            currentCount = WORD_COUNT*8;
            unsigned long o = fread((void*)buffer, 1, WORD_COUNT*8, pFile);
            if(o != currentCount)
            {
                fprintf(stderr, "Read failure\n");
            }
            bytesRead += WORD_COUNT*8;
        }
        else
        {
            currentCount = lSize - bytesRead;
            unsigned long o = fread((void*)buffer, 1, lSize - bytesRead, pFile);
            if(o != currentCount)
            {
                fprintf(stderr, "Read failure\n");
            }
            end = TRUE;
        }
        //write page
        ftdispi_write(&fsc, buffer, currentCount, GPO_STATE);
    }
    fprintf(stderr, "\nWritten\n");
    sleep(10);
    if(FTDISPI_ERROR_NONE != ftdispi_setgpo(&fsc, 0xFF))
    {
        fprintf(stderr, "Can't set GPO status\n");
    }
    else
        fprintf(stderr, "GPO released\n");
    ftdispi_close(&fsc, 1);
    return 0;
}
