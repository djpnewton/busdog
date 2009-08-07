
#include <ntddk.h>
#include <wdf.h>

#if !defined(_BUSDOG_FILTER_H_)
#define _BUSDOG_FILTER_H_

//
// String definitions
//

#define DRIVERNAME "busdog.sys: "

#define NTDEVICE_NAME_STRING      L"\\Device\\BusDogFilter"
#define SYMBOLIC_NAME_STRING      L"\\DosDevices\\BusDogFilter"

//
// Struct definitions
//

typedef struct _BUSDOG_CONTEXT {
    ULONG     SerialNo;
} BUSDOG_CONTEXT, *PBUSDOG_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(BUSDOG_CONTEXT,
                                        GetBusDogContext)

typedef struct _CONTROL_DEVICE_EXTENSION {

    PVOID   ControlData; // Store your control data here

} CONTROL_DEVICE_EXTENSION, *PCONTROL_DEVICE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(CONTROL_DEVICE_EXTENSION,
                                        ControlGetData)

//
// Function definitions
//

DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD BusDogDeviceAdd;
EVT_WDF_DRIVER_UNLOAD BusDogDriverUnload;
EVT_WDF_DEVICE_CONTEXT_CLEANUP BusDogDeviceContextCleanup;

#ifdef IOCTL_INTERFACE

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL BusDogIoDeviceControl;

NTSTATUS
BusDogCreateControlDevice(
    WDFDEVICE Device
    );

VOID
BusDogDeleteControlDevice(
    WDFDEVICE Device
    );

#endif

NTSTATUS
BusDogWdmDeviceRead (
    IN WDFDEVICE Device,
    IN PIRP Irp
    );

VOID
PrintChars(
    __in_ecount(CountChars) PCHAR BufferAddress,
    __in ULONG CountChars
    );

//
// Helper macros
//

#define FlagOn(F,SF) ( \
    (((F) & (SF)))     \
)

#endif
