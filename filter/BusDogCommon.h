
#include <ntddk.h>
#include <wdf.h>
#include <Usbioctl.h>
#include <usb.h>

#define NT_INCLUDED
#include "WinDef.h"

#include "BusDogUserCommon.h"
#include "BusDogData.h"

#if !defined(_BUSDOG_COMMON_H_)
#define _BUSDOG_COMMON_H_

//
// String definitions
//

#define DRIVERNAME "busdog.sys: "

#define NTDEVICE_NAME_STRING      L"\\Device\\BusDogFilter"
#define SYMBOLIC_NAME_STRING      L"\\DosDevices\\BusDogFilter"

//
// Struct definitions
//

#define DEVICE_CONTEXT_MAGIC    0x98761234

typedef struct _BUSDOG_CONTEXT {

    ULONG       MagicNumber;

    BOOLEAN     HasDeviceId;

    ULONG       DeviceId;

    BOOLEAN     FilterEnabled;

    WDFIOTARGET TargetToSendRequestsTo;

} BUSDOG_CONTEXT, *PBUSDOG_CONTEXT;

#define IS_DEVICE_CONTEXT(_DC_) (((_DC_)->MagicNumber) == DEVICE_CONTEXT_MAGIC)

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(BUSDOG_CONTEXT,
                                        BusDogGetDeviceContext)

typedef struct _CONTROL_DEVICE_EXTENSION {

    PVOID   ControlData; // Store your control data here

} CONTROL_DEVICE_EXTENSION, *PCONTROL_DEVICE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(CONTROL_DEVICE_EXTENSION,
                                        ControlGetData)

typedef struct
{
    ULONG                           ItemSize; // BUSDOG_FILTER_TRACE comes after this member in memory

} BUSDOG_FILTER_TRACE_FIFO_ITEM, *PBUSDOG_FILTER_TRACE_FIFO_ITEM;

#define BUSDOG_FILTER_TRACE_FIFO_LENGTH 100

typedef struct
{
    PBUSDOG_FILTER_TRACE_FIFO_ITEM  TraceItems[BUSDOG_FILTER_TRACE_FIFO_LENGTH];

    ULONG                           WriteIndex;

    ULONG                           ReadIndex;

} BUSDOG_FILTER_TRACE_FIFO;

//
// Function definitions
//

DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_UNLOAD BusDogDriverUnload;
EVT_WDF_DRIVER_DEVICE_ADD BusDogDeviceAdd;
EVT_WDF_DRIVER_UNLOAD BusDogDriverUnload;
EVT_WDF_DEVICE_CONTEXT_CLEANUP BusDogDeviceContextCleanup;

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL BusDogIoDeviceControl;

NTSTATUS
BusDogCreateControlDevice(
    WDFDEVICE Device
    );

VOID
BusDogDeleteControlDevice(
    WDFDEVICE Device
    );

VOID
BusDogIoRead(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t Length
    );

VOID
BusDogReadComplete(
    IN WDFREQUEST Request,
    IN WDFIOTARGET Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS Params,
    IN WDFCONTEXT Context
    );

VOID
BusDogIoWrite(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t Length
    );

VOID
BusDogForwardRequest(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request
    );

VOID
BusDogForwardRequestWithCompletion(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request,
    IN PFN_WDF_REQUEST_COMPLETION_ROUTINE CompletionRoutine,
    IN WDFCONTEXT CompletionContext
    );

VOID
BusDogIoInternalDeviceControl(
    IN WDFQUEUE  Queue,
    IN WDFREQUEST  Request,
    IN size_t  OutputBufferLength,
    IN size_t  InputBufferLength,
    IN ULONG  IoControlCode
    );

VOID
BusDogIoInternalDeviceControlComplete(
    IN WDFREQUEST Request,
    IN WDFIOTARGET Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS Params,
    IN WDFCONTEXT Context
    );

VOID
BusDogProcessInternalDeviceControl(
    IN WDFDEVICE Device,
    IN PBUSDOG_CONTEXT Context,
    IN WDFREQUEST  Request,
    IN ULONG  IoControlCode,
    IN BOOLEAN bCompletion,
    OUT BOOLEAN* bRead);

//
// BusDogGeneric.c
//

VOID
PrintChars(
    __in_ecount(CountChars) PCHAR BufferAddress,
    __in ULONG CountChars
    );

BUSDOG_TIMESTAMP
BusDogGetTimeStamp(
    VOID
    );

VOID
BusDogUpdateDeviceIds(
    VOID
    );

BOOLEAN
BusDogFillBufferWithDeviceIds(
    PVOID Buffer,
    size_t BufferSize,
    size_t* BytesWritten,
    size_t* BytesNeeded
    );

//
// BusDogTraceList.c
//

NTSTATUS
BusDogTraceFifoInit(
    WDFDRIVER Driver
    );

VOID
BusDogTraceFifoCleanUp(
    VOID
    );

VOID
BusDogAddTraceToFifo(
    WDFDEVICE device,
    ULONG DeviceId,
    BUSDOG_REQUEST_TYPE Type,
    BUSDOG_REQUEST_PARAMS Params,
    PVOID TraceBuffer,
    ULONG BufferLength
    );

NTSTATUS 
BusDogFufillRequestWithTraces(
    IN WDFREQUEST Request,
    OUT size_t* bytesWritten
    );

//
// Helper macros
//

#define FlagOn(F,SF) ( \
    (((F) & (SF)))     \
)

#if DBG
#define BusDogPrint(dbglevel, fmt, ...) {  \
    if (BusDogDebugLevel >= dbglevel)      \
    {                                      \
        DbgPrint(DRIVERNAME);              \
        DbgPrint(fmt, __VA_ARGS__);        \
    }                                      \
}
#else
#define BusDogPrint(dbglevel, fmt, ...) {  \
}
#endif

#endif
