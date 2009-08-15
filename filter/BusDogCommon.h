
#include <ntddk.h>
#include <wdf.h>

#include "BusDogData.h"

#define NT_INCLUDED
#include "WinDef.h"
#include "BusDogUserCommon.h"

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

    ULONG       SerialNo;

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

typedef struct Node 
{

    struct Node*            Prev;

    PBUSDOG_FILTER_TRACE    Trace;

    struct Node*            Next;

} BUSDOG_FILTER_TRACE_LLISTITEM, *PBUSDOG_FILTER_TRACE_LLISTITEM;

typedef struct
{
    ULONG                           Count;

    PBUSDOG_FILTER_TRACE_LLISTITEM  Head;
    
    PBUSDOG_FILTER_TRACE_LLISTITEM  Tail;

} BUSDOG_FILTER_TRACE_LLIST;

#define BUSDOG_FILTER_TRACE_LIST_MAX_LENGTH 20

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

#ifdef WDM_PREPROCESS

NTSTATUS
BusDogWdmDeviceReadWrite (
    IN WDFDEVICE Device,
    IN PIRP Irp
    );

#else

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

#endif

//
// BusDogGeneric.c
//

VOID
PrintChars(
    __in_ecount(CountChars) PCHAR BufferAddress,
    __in ULONG CountChars
    );

VOID
BusDogUpdateDeviceIds(
    VOID
    );

//
// BusDogTraceList.c
//

NTSTATUS
BusDogTraceListInit(
    WDFDRIVER Driver
    );

VOID
BusDogTraceListCleanUp(
    VOID
    );

VOID
BusDogAddTraceToList(
    ULONG DeviceId,
    BUSDOG_REQUEST_TYPE Type,
    PVOID TraceBuffer,
    ULONG BufferLength
    );

size_t
BusDogFillBufferWithTraces(
    PVOID Buffer,
    size_t BufferSize
    );

//
// Helper macros
//

#define FlagOn(F,SF) ( \
    (((F) & (SF)))     \
)

#endif
