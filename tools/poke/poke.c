#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winioctl.h>

//
// Some stuff used in the shared header that isn't defined
//  for user mode...
//
typedef LONG NTSTATUS;

typedef struct _IO_STATUS_BLOCK 
{
    union 
    {
        NTSTATUS Status;
        PVOID Pointer;
    };

    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

#include "BusDogUserCommon.h"

#define USAGE_STRING "Usage: poke {-print|-start|-stop}\n"               \
                     "       poke -setenabled ${DEVICEID} TRUE|FALSE"    \
                     "       poke -getbuffer\n\n"                        \
                     "  -print -- Send the \"print devices\" "           \
                     "command\n"                                         \
                     "  -start -- Send the \"start filtering\" "         \
                     "command\n"                                         \
                     "  -stop  -- Send the \"stop filtering\" "          \
                     "command\n"                                         \
                     "  -setenabled  --  Set the device filter "         \
                     "specified by the Id to enabled or disabled\n"      \
                     "  -getbuffer   --  Get the current filter buffer"

void printChars(char* traceBuf, DWORD bufSize)
{
    if (bufSize) {

        while (bufSize--) {

            if (*traceBuf > 31
                 && *traceBuf != 127) {

                printf( "%c", *traceBuf);

            } else {

                printf( ".");

            }
            traceBuf++;
        }
        printf("\n");
    }
    return;
}

void __cdecl main(int argc, CHAR **argv) 
{
    HANDLE hFilter;
    DWORD bytesRet;
    DWORD ioctl;
    
    char* inBuf = NULL;
    DWORD inBufSize = 0;
    char* outBuf = NULL;
    DWORD outBufSize = 0;

    //
    // Determine ioctl
    //

    if (argc == 2)
    {
        if (strcmp(argv[1], "-print") == 0) 
        {
            ioctl = IOCTL_BUSDOG_PRINT_DEVICES;
        } 
        else if (strcmp(argv[1], "-start") == 0) 
        {
            ioctl = IOCTL_BUSDOG_START_FILTERING;
        } 
        else if (strcmp(argv[1], "-stop") == 0) 
        {
            ioctl = IOCTL_BUSDOG_STOP_FILTERING;
        } 
        else if (strcmp(argv[1], "-getbuffer") == 0) 
        {
            ioctl = IOCTL_BUSDOG_GET_BUFFER;
            outBufSize = 1024;
            outBuf = malloc(outBufSize);
        } 
        else 
        {
            printf(USAGE_STRING);
            return;
        }
    }
    else if (argc == 4)
    {
        if (strcmp(argv[1], "-setenabled") == 0) 
        {
            BUSDOG_FILTER_ENABLED filterEnabled;

            ioctl = IOCTL_BUSDOG_SET_DEVICE_FILTER_ENABLED;
            filterEnabled.DeviceId = atoi(argv[2]);
            filterEnabled.FilterEnabled = (BOOLEAN)atoi(argv[3]);
            inBufSize = sizeof(BUSDOG_FILTER_ENABLED);
            inBuf = malloc(inBufSize);
            memcpy(inBuf, &filterEnabled, sizeof(BUSDOG_FILTER_ENABLED));
        } 
        else
        {
            printf(USAGE_STRING);
            return;
        }
    }
    else
    {
        printf(USAGE_STRING);
        return;
    }


    //
    // Open the filter device
    //

    hFilter  = CreateFile("\\\\.\\BusDogFilter",
                               GENERIC_READ|GENERIC_WRITE,
                               0,
                               0,
                               OPEN_EXISTING,
                               FILE_FLAG_OVERLAPPED,
                               0);

    if (hFilter == INVALID_HANDLE_VALUE) 
    {
        printf("Couldn't open device - %d\n",
                GetLastError());
        goto finish;
    }

    //
    // Ping filter with an IOCTL
    //

    if (!DeviceIoControl(hFilter,
                         ioctl,
                         inBuf,
                         inBufSize,
                         outBuf,
                         outBufSize,
                         &bytesRet,
                         0)) 
    {

        printf("DeviceIoControl failed - %d\n",
                GetLastError());
    }
    else
    {
        printf("DeviceIoControl succeeded\n");

        switch (ioctl)
        {
            case IOCTL_BUSDOG_GET_BUFFER:
            {
                DWORD index = 0;

                while (index <= outBufSize - sizeof(BUSDOG_FILTER_TRACE))
                {
                    BUSDOG_FILTER_TRACE* pTrace = (BUSDOG_FILTER_TRACE*)(outBuf + index);

                    index += sizeof(BUSDOG_FILTER_TRACE);

                    if (index <= outBufSize - pTrace->BufferSize)
                    {
                        char* traceBuf = outBuf + index;

                        printf("Got a trace (type: %d, size: %d): ", pTrace->Type, pTrace->BufferSize);

                        printChars(traceBuf, min(outBufSize - index, pTrace->BufferSize));
                    }
                    else
                        break;

                    index += pTrace->BufferSize;
                }

                break;
            }
        }
    }

finish:

    if (inBuf != NULL)
        free(inBuf);

    if (outBuf != NULL)
        free(outBuf);
}
