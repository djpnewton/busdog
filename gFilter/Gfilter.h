
#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>


#define DRIVERNAME "GFILTER"


#if !defined(_FILTER_H_)
#define _FILTER_H_

//
// Names and locations
//
#define DEVICE_PATH L"\\Device\\GFilterDevice"
#define SYMBOLIC_PATH L"\\DosDevices\\GFilterDevice"

//
// GUIDs
//
#ifdef USE_OWN_GUID
DEFINE_GUID(GUID_DEVCLASS_GFILTER, 0x1fb40e23, 0x98bd, 0x40da, 0x84, 0x34, 0x2b, 0xa0, 0x6f, 0xb8, 0xb1, 0x11); // {1FB40E23-98BD-40da-8434-2BA06FB8B111} -- UNTESTED
#endif
// Bus GUID etc for PDO
DEFINE_GUID(GUID_BUS_GFILTER, 0x6e566f26, 0x3df2, 0x412b, 0x87, 0x43, 0xfb, 0x8b, 0x5d, 0x46, 0x24, 0xc1); // {6E566F26-3DF2-412b-8743-FB8B5D4624C1}
#define GFILTER_DEVICE_ID L"{6E566F26-3DF2-412b-8743-FB8B5D4624C1}\\gfilter\0"
// Device GUID for interface
DEFINE_GUID(GUID_DEVICEINTERFACE_GFILTER, 0x9b30185d, 0xa7f0, 0x4a73, 0xa1, 0x5a, 0xe7, 0x88, 0x5e, 0x69, 0x4b, 0xb3); // {9B30185D-A7F0-4a73-A15A-E7885E694BB3}




//
// Driver Prototypes for Entry Points
//

// Driver
DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD GFilterAddDevice;

// Filter
EVT_WDF_REQUEST_COMPLETION_ROUTINE GFilterReadCompleteionRoutine;
#ifdef USE_SETIOINCALLERCONTEXTCALLBACK
EVT_WDF_IO_IN_CALLER_CONTEXT GFilterIOCallback; 
#else
EVT_WDF_IO_QUEUE_IO_READ GFilterRead;
EVT_WDF_IO_QUEUE_IO_WRITE GFilterWrite;
#endif

// Raw PDO (sideband)
EVT_WDF_IO_QUEUE_IO_READ GFilterSideBandRead;
EVT_WDF_IO_QUEUE_IO_WRITE GFilterSideBandWrite;

#endif 
