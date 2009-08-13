/*
 * Util -- This is a test application to connect to the GFilters sideband communication channel
 */


// Includes
#include <windows.h>
#include <stdio.h>
#include "Setupapi.h"
#include "util.h"


// Prototypes
HANDLE EnumGFilterDevice(int DevIndex);
DWORD WINAPI readerThread(LPVOID param);

// Types
typedef  void (*ONREAD)(char *, DWORD);
typedef struct  // Device that we IO with
{
	HANDLE	h;
	HANDLE	readerThread;
	HANDLE	killReaderThread;
	ONREAD	onRead;
} DEVICE;




////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////

// Inits
void initDEVICE(DEVICE *d)
{
	d->h = NULL;
	d->readerThread = NULL;
	d->killReaderThread = CreateEvent(	NULL,
										TRUE,
										FALSE,
										NULL);
	d->onRead = NULL;
}




////////////////////////////////////////////
// Enumerate
////////////////////////////////////////////
HANDLE openGFilterControlDevice(int devIndex)
{
	DWORD			lastError;		
	HANDLE			h;

	h = CreateFile(	L"\\\\.\\GFilterDevice06",//L"\\DosDevices\\GFilterDevice06",							// Path to device on system
					GENERIC_READ | GENERIC_WRITE,									// Open for RW
					FILE_SHARE_READ | FILE_SHARE_WRITE /*| FILE_SHARE_DELETE*/,		// Allow other process to RW (and del?)
					NULL,															// No security
					OPEN_EXISTING,													// Device file already exists so dont create a new file
#ifdef DO_OVERLAPPED_IO
					FILE_FLAG_OVERLAPPED,											// Open overlapped -- not supported yet?
#else
					FILE_ATTRIBUTE_NORMAL,							
#endif
					NULL	);
	if (h == INVALID_HANDLE_VALUE) // Check file opened OK 
	{		
		// Error, log the error and move to the next device
		lastError = GetLastError();
		// If access is denied just move on to the next device, otherwise raise an error
		ERR(0, "Failed to get details for device %d. 0x%0.4x\n", devIndex, lastError);
		h = NULL;
	}

	return(h);	
}

HANDLE EnumGFilterDevice(int devIndex)
{
	DWORD								lastError;
	GUID								gfGUID = {0x9b30185d, 0xa7f0, 0x4a73, 0xa1, 0x5a, 0xe7, 0x88, 0x5e, 0x69, 0x4b, 0xb3}; //GUID_DEVCLASS_UNKNOWN
	HDEVINFO 							hardwareDeviceInfo;
	SP_DEVICE_INTERFACE_DATA            deviceInfoData;
	DWORD								RequiredFunctionClassDeviceDataSize;
	DWORD								AllocatedFunctionClassDeviceDataSize;
	PSP_DEVICE_INTERFACE_DETAIL_DATA    functionClassDeviceData = NULL;
	HANDLE								h=NULL;
	

	// Get a list of all the devices with this GUID type
	hardwareDeviceInfo	=	SetupDiGetClassDevs(	&gfGUID,						// Get GFilter devices only
													NULL,							// Enumerator (none just get all)
													NULL,							// HwndParent (none just get all)
													(DIGCF_DEVICEINTERFACE |		// Only return devices that match gfGUID
													DIGCF_PRESENT )	);				// Only get devices that are currently connected to this machine
	if ( hardwareDeviceInfo == INVALID_HANDLE_VALUE ) // Error checking
	{
		// Failed to get a list of all the devices on this machine 
		// all we can do is cleanup and return
		lastError = GetLastError();
		ERR(0, "Failed to get a list of the devices connected to this machine. 0x%0.4x\n", lastError);
		return NULL;
	}

	// Find DevIndex
	deviceInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
	if ( !SetupDiEnumDeviceInterfaces(	hardwareDeviceInfo,		// The list of devices 
										NULL,					// Go through all the devices
										&gfGUID,				// These are our gfilter devices
										devIndex,				// Index of the device to check in the hardwareDeviceInfo list
										&deviceInfoData) )		// Fill this in with data about the device requested
									
	{
		// Error handling here
		lastError = GetLastError();
		// Did we reach the end of all the devices in the hardwareDeviceInfo list
		if (lastError == ERROR_NO_MORE_ITEMS)
		{
			DBG(0, "Device Index not found.\n");
			goto findGFilterDeviceCleanUp; // We have come to the end of the list so clean up and return.
		}
		// If here a real problem was encountered so log the error and move on
		ERR(0, "Failed to get details for device %d. 0x%0.4x\n", devIndex, lastError);
		goto findGFilterDeviceCleanUp;
	}

	// Get the device path	
	if ( !SetupDiGetDeviceInterfaceDetail(	hardwareDeviceInfo,							// The device to find out about 
											&deviceInfoData,							// Details about the device 
											NULL,										// Just checking how long path string is 
											0,											// "
											&RequiredFunctionClassDeviceDataSize,		// "
											NULL)	)									// "
	{
	}
	AllocatedFunctionClassDeviceDataSize = RequiredFunctionClassDeviceDataSize;
	functionClassDeviceData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(AllocatedFunctionClassDeviceDataSize);
	functionClassDeviceData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
	if ( !SetupDiGetDeviceInterfaceDetail(	hardwareDeviceInfo,							// The device to find out about 
											&deviceInfoData,							// Details about the device
											functionClassDeviceData,					// Place the path in here
											AllocatedFunctionClassDeviceDataSize,		// How big is functionClassDeviceData (alloced above)
											&RequiredFunctionClassDeviceDataSize,		// How many bytes did it write
											NULL)	)									// Dont care about this
	{
		// Error, log the error and move to the next device
		lastError = GetLastError();		
		ERR(0, "Failed to get details for device %d. 0x%0.4x\n", devIndex, lastError);
		goto findGFilterDeviceCleanUp;
	}
	
	// Open the device
	h = CreateFile(	functionClassDeviceData->DevicePath,							// Path to device on system
					GENERIC_READ | GENERIC_WRITE,									// Open for RW
					FILE_SHARE_READ | FILE_SHARE_WRITE /*| FILE_SHARE_DELETE*/,		// Allow other process to RW (and del?)
					NULL,															// No security
					OPEN_EXISTING,													// Device file already exists so dont create a new file
#ifdef DO_OVERLAPPED_IO
					FILE_FLAG_OVERLAPPED,											// Open overlapped -- not supported yet?
#else
					FILE_ATTRIBUTE_NORMAL,							
#endif
					NULL	);		 	
	if (h == INVALID_HANDLE_VALUE) // Check file opened OK 
	{		
		// Error, log the error and move to the next device
		lastError = GetLastError();
		// If access is denied just move on to the next device, otherwise raise an error
		ERR(0, "Failed to get details for device %d. 0x%0.4x\n", devIndex, lastError);
		h = NULL;
	}

findGFilterDeviceCleanUp:
	if (functionClassDeviceData != NULL) 
		free(functionClassDeviceData);
	SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
	return(h);
}




////////////////////////////////////////////
// Reader Thread
////////////////////////////////////////////

// Start Reader Thread
DWORD startReaderThread(DEVICE *d)
{
	DWORD lastError;
	d->readerThread = CreateThread(	NULL,
									0,
									readerThread,
									d,
									0,
									NULL);
	if (d->readerThread == NULL)
	{
		lastError = GetLastError();
		ERR(0, "Failed to start the reader thread. 0x%0.4\n", lastError);
		return(lastError);
	}
	
	return(ERROR_SUCCESS);
}

// Reader Thread 
#define IN_BUFFER_LENGTH 65
DWORD WINAPI readerThread(LPVOID param)
{
	DWORD			lastError;
	char 			ibuf[IN_BUFFER_LENGTH];
	OVERLAPPED		overlapped;
	WORD			running=TRUE;
	DWORD			bytesRead;
	// Retrieve the device
	DEVICE 			*d = (DEVICE *)param;

#ifdef DO_OVERLAPPED_IO
	// Setup the overlapped struct
	memset(&overlapped, 0x00, sizeof(OVERLAPPED));
	overlapped.hEvent = CreateEvent(	NULL,
										TRUE,
										FALSE,
										NULL);
	if (overlapped.hEvent == NULL)
	{
			lastError = GetLastError();
			ERR(0, "Unable to create overlapped object. 0x%0.4x\n", lastError);
			return(lastError);
	}
#else
#endif

	while (running)
	{
		memset(ibuf, 0x00, IN_BUFFER_LENGTH);
		if (!ReadFile(	d->h,
						ibuf,
						IN_BUFFER_LENGTH,
						&bytesRead,
#ifdef DO_OVERLAPPED_IO
						&overlapped))
#else
						NULL))
#endif
		{
				lastError = GetLastError();
				switch(lastError)
				{
#ifdef DO_OVERLAPPED_IO						
					case ERROR_IO_PENDING:
						// wait for read to complete (we are serialized atm)
						if (!GetOverlappedResult(	d->h,
													&overlapped,
													&bytesRead,
													TRUE))
						{
							// This is a problem
							lastError = GetLastError();
							ERR(0, "Failed waiting for overlapped result. 0x%0.2x", lastError);
							break;
						}
						// Read finished 
						if (d->onRead != NULL)
							d->onRead(ibuf, bytesRead);
						break;
#endif
					default:
						// This is a problem
						ERR(0, "Read failed. 0x%0.4x", lastError);
						running=FALSE;
						break;
				}
		}
		else
		{
				// Read finished already
				if (d->onRead != NULL)
					d->onRead(ibuf, bytesRead);
		}

		// DEBUG
		Sleep(1000);

	}

	// Cleanup
#ifdef DO_OVERLAPPED_IO
	if (overlapped.hEvent != NULL)
		CloseHandle(overlapped.hEvent);
#endif

	return(ERROR_SUCCESS);
}




////////////////////////////////////////////
// Writing
////////////////////////////////////////////
DWORD write(DEVICE	*d,
			void 	*buf,
			DWORD	length)
{

	DWORD			lastError;
	OVERLAPPED		overlapped;
	DWORD			bytesWritten;

#ifdef DO_OVERLAPPED_IO
	// Setup the overlapped struct
	memset(&overlapped, 0x00, sizeof(OVERLAPPED));
	overlapped.hEvent = CreateEvent(	NULL,
										TRUE,
										FALSE,
										NULL);
	if (overlapped.hEvent == NULL)
	{
			lastError = GetLastError();
			ERR(0, "Unable to create overlapped object. 0x%0.4x\n", lastError);
			return(lastError);
	}
#else
#endif
	
	// Write to the device
	if (!WriteFile(	d->h,
					buf,
					length,
					&bytesWritten,
#ifdef DO_OVERLAPPED_IO
					&overlapped))
#else					
					NULL))
#endif
	{
		lastError = GetLastError();
		switch (lastError)
		{
#ifdef DO_OVERLAPPED_IO				
			case ERROR_IO_PENDING:
				if (!GetOverlappedResult(	d->h,
											&overlapped,
											&bytesWritten,
											TRUE))
				{
					lastError = GetLastError();
					ERR(0, "Failed waiting for overlapped result. 0x%0.4x\n", lastError);
					break;
				}
				if (bytesWritten != length)
				{
					ERR(0, "Wrote %d bytes of %d\n", bytesWritten, length);
					lastError = ERROR_BAD_LENGTH;
				}				
				else
					lastError = ERROR_SUCCESS;
				break;
#endif				
			default:
				ERR(0, "Failed to write. 0x%0.4x\n", lastError);
				break;
		}
	}
	else
	{
		if (bytesWritten != length)
		{
			ERR(0, "Wrote %d bytes of %d\n", bytesWritten, length);
			lastError = ERROR_BAD_LENGTH;
		}
		else
			lastError = ERROR_SUCCESS;
	}

	// Cleanup
#ifdef DO_OVERLAPPED_IO
	if (overlapped.hEvent != NULL)
		CloseHandle(overlapped.hEvent);
#endif	
	return(lastError);
}



////////////////////////////////////////////
// Main
////////////////////////////////////////////

void OnRead(char *buf, DWORD bufLength)
{
	WORD k;
	// Display some data
	LOG(0, "Received: Len=%d:", bufLength);
	for (k=0; k<bufLength; k++)
	{
		printf(" %#0.2x", (unsigned char)buf[k]);
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	int 	devIndex;
	DWORD	lastError;	
	DEVICE 	dev;

	char 	buf[128];

	if (argc !=2)
	{
		ERR(0, "syntax: util.exe <DEVINDEX>\n");
		return(1);
	}
	devIndex = atoi(argv[1]);

		
	DBG(0, "Util Started\n");	
	initDEVICE(&dev);

	// Connect to the specified device
//	dev.h = EnumGFilterDevice(devIndex);
	dev.h = openGFilterControlDevice(devIndex);
	if (dev.h == NULL)
		return(1);

	// Write something as a test
	buf[0] = 'a';
	lastError = write(&dev, buf, 128);

	// Start the reader
	dev.onRead 	= &OnRead; 
	lastError 	= startReaderThread(&dev);

	// Loop until done
	while(1) {}

	// Close up
	CloseHandle(dev.h);

	DBG(0, "Util Ended\n");
	return(0);
}
