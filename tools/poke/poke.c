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

#define USAGE_STRING "Usage: poke {-print|-start|-stop}\n\n"          \
                     "       poke -setactive ${DEVICEID} TRUE|FALSE\n"\
                     "  -print -- Send the \"print devices\" "        \
                     "command\n"                                      \
                     "  -start -- Send the \"start filtering\" "      \
                     "command\n"                                      \
                     "  -stop  -- Send the \"stop filtering\" "       \
                     "command\n"                                      \
                     "  -setenabled  --  Set the device filter "      \
                     "specified by the Id to enabled or disabled"


void __cdecl main(int argc, CHAR **argv) 
{
    HANDLE hFilter;
    DWORD bytesRet;
    DWORD ioctl;
    
    char* inBuf = NULL;
    DWORD inBufSize = 0;

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
                         0,
                         0,
                         &bytesRet,
                         0)) 
    {

        printf("DeviceIoControl failed - %d\n",
                GetLastError());
    }
    else
        printf("DeviceIoControl succeeded\n");

finish:

    if (inBuf != NULL)
        free(inBuf);
}
