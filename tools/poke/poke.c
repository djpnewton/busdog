#include <windows.h>
#include <stdio.h>
#include <winioctl.h>

void __cdecl main(void) 
{
    HANDLE hFilter;
    DWORD bytesRet;

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
                         100, /* IOCTL_XXX in future */
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
