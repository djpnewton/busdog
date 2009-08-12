#include <windows.h>
#include <stdio.h>
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

#define USAGE_STRING "Usage: poke {-print|-start|-stop}\n\n"       \
                     "  -print -- Send the \"print devices\" "     \
                     "command\n"                                   \
                     "  -start -- Send the \"start tracking\" "    \
                     "command\n"                                   \
                     "  -stop  -- Send the \"stop tracking\" "     \
                     "command\n"                                   \


void __cdecl main(int argc, CHAR **argv) 
{
    HANDLE hFilter;
    DWORD bytesRet;
    DWORD ioctl;

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
            ioctl = IOCTL_BUSDOG_START_TRACK;
        } 
        else if (strcmp(argv[1], "-stop") == 0) 
        {
            ioctl = IOCTL_BUSDOG_STOP_TRACK;
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
        return;
    }

    //
    // Ping filter with an IOCTL
    //

    if (!DeviceIoControl(hFilter,
                         ioctl,
                         0,
                         0,
                         0,
                         0,
                         &bytesRet,
                         0)) 
    {

        printf("DeviceIoControl failed - %d\n",
                GetLastError());
        return;
    }

    printf("DeviceIoControl succeeded\n");
}
