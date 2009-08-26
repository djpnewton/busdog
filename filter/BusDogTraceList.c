#include "BusDogCommon.h"

//
// This linked list stores all the filter traces
//
BUSDOG_FILTER_TRACE_LLIST   BusDogTraceList;
WDFWAITLOCK                 BusDogTraceListLock;

NTSTATUS
BusDogTraceListInit(
    WDFDRIVER Driver
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    WDF_OBJECT_ATTRIBUTES attributes;

    PAGED_CODE ();

    //
    // Init our trace list and create a lock for it
    //

    BusDogTraceList.Count = 0;
    BusDogTraceList.Head = NULL;
    BusDogTraceList.Tail = NULL;

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = Driver;

    status = WdfWaitLockCreate(&attributes,
                                &BusDogTraceListLock);
    if (!NT_SUCCESS(status))
    {
        KdPrint( ("WdfWaitLockCreate failed with status 0x%x\n", status));
    }

    return status;
}

VOID
BusDogTraceListCleanUp(
    VOID
    )
{

    PAGED_CODE ();

    //
    // Clean up anything in the trace list
    //

    WdfWaitLockAcquire(BusDogTraceListLock, NULL);
    
    while (BusDogTraceList.Count > 0)
    {
        PBUSDOG_FILTER_TRACE_LLISTITEM Node = BusDogTraceList.Tail;

        if (Node->Prev != NULL)
        {
            BusDogTraceList.Tail = Node->Prev;
        }

        ExFreePool(Node->Trace);

        ExFreePool(Node);

        BusDogTraceList.Count--;
    }

    BusDogTraceList.Head = NULL;

    BusDogTraceList.Tail = NULL;

    WdfWaitLockRelease(BusDogTraceListLock);
    
}

PBUSDOG_FILTER_TRACE_LLISTITEM
BusDogCreateTraceListItem(
    ULONG DeviceId,
    BUSDOG_REQUEST_TYPE Type,
    PVOID TraceBuffer,
    ULONG BufferLength
    )
{
    PBUSDOG_FILTER_TRACE pTrace;
    PBUSDOG_FILTER_TRACE_LLISTITEM pTraceListItem;

    pTrace = ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(BUSDOG_FILTER_TRACE) + BufferLength,
            'GATT');

    if (pTrace == NULL)
    {
        KdPrint(("ExAllocatePoolWithTag failed\n"));
        
        return NULL;
    }
    else
    {
        pTrace->DeviceId = DeviceId;

        pTrace->Type = Type;

        pTrace->Timestamp = BusDogGetTimeStamp();

        pTrace->BufferSize = BufferLength;

        RtlCopyMemory((PCHAR)pTrace + sizeof(BUSDOG_FILTER_TRACE),
                TraceBuffer,
                BufferLength);
    }

    pTraceListItem = ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(BUSDOG_FILTER_TRACE_LLISTITEM),
            'TILT');

    if (pTraceListItem == NULL)
    {
        KdPrint(("ExAllocatePoolWithTag failed\n"));
        
        ExFreePool(pTrace);

        return NULL;
    }
    else
    {
        pTraceListItem->Prev = NULL;

        pTraceListItem->Trace = pTrace;

        pTraceListItem->Next = NULL;
    }

    return pTraceListItem;
}

VOID
__BusDogAddTraceToList(
    PBUSDOG_FILTER_TRACE_LLISTITEM pTraceListItem
    )
{
    PAGED_CODE ();

    //
    // Add trace to list
    //

    WdfWaitLockAcquire(BusDogTraceListLock, NULL);

    if (BusDogTraceList.Head != NULL)
    {
        BusDogTraceList.Head->Prev = pTraceListItem;
    }

    pTraceListItem->Next = BusDogTraceList.Head;

    BusDogTraceList.Head = pTraceListItem;

    if (BusDogTraceList.Tail == NULL)
    {
        BusDogTraceList.Tail = pTraceListItem;
    }

    BusDogTraceList.Count++;

    //
    // Remove from tail if list too long
    //

    if (BusDogTraceList.Count > BUSDOG_FILTER_TRACE_LIST_MAX_LENGTH)
    {
        KdPrint(("BusDog - On noes! We have overflow\n"));

        pTraceListItem = BusDogTraceList.Tail;

        BusDogTraceList.Tail = pTraceListItem->Prev;

        ExFreePool(pTraceListItem->Trace);

        ExFreePool(pTraceListItem);

        BusDogTraceList.Count--;
    }

    WdfWaitLockRelease(BusDogTraceListLock);
}

VOID 
BusDogAddTraceWorkItem(
    IN WDFWORKITEM WorkItem
    )
{
    PBUSDOG_WORKITEM_CONTEXT pItemContext;
    NTSTATUS status;

    PAGED_CODE ();

    pItemContext = BusDogGetWorkItemContext(WorkItem);

    __BusDogAddTraceToList(pItemContext->pTraceListItem);

    WdfObjectDelete(WorkItem);

    return;
}

VOID
BusDogAddTraceToList(
    ULONG DeviceId,
    BUSDOG_REQUEST_TYPE Type,
    PVOID TraceBuffer,
    ULONG BufferLength
    )
{
    PBUSDOG_FILTER_TRACE_LLISTITEM pTraceListItem = NULL; 

    PAGED_CODE ();

    pTraceListItem = 
        BusDogCreateTraceListItem(
            DeviceId,
            Type,
            TraceBuffer,
            BufferLength);

    if (pTraceListItem != NULL)
    {
        __BusDogAddTraceToList(pTraceListItem);
    }
}

//
// Assumes trace list already locked
//
PBUSDOG_FILTER_TRACE
__BusDogGetTraceFromList(
    VOID
    )
{
    PBUSDOG_FILTER_TRACE pTrace = NULL;

    PAGED_CODE ();

    if (BusDogTraceList.Count > 0)
    {
        PBUSDOG_FILTER_TRACE_LLISTITEM Node = BusDogTraceList.Tail;

        if (Node->Prev != NULL)
        {
            BusDogTraceList.Tail = Node->Prev;
        }
        else if (BusDogTraceList.Count == 1)
        {
            BusDogTraceList.Tail = BusDogTraceList.Head;
        }
        else
        {
            KdPrint(("BusDogError, I dont think I should be here"));
        }

        pTrace = Node->Trace;

        ExFreePool(Node);

        BusDogTraceList.Count--;
    }
    else
    {
        //
        // No traces left
        //

        BusDogTraceList.Head = NULL;

        BusDogTraceList.Tail = NULL;
    }

    return pTrace;
}

//
// Assumes trace list already locked
//
size_t
__BusDogGetLastTraceSize(
    VOID
    )
{
    if (BusDogTraceList.Count > 0)
    {
        return sizeof(BUSDOG_FILTER_TRACE) + BusDogTraceList.Tail->Trace->BufferSize;
    }
        
    return 0;
}

size_t
BusDogFillBufferWithTraces(
    PVOID Buffer,
    size_t BufferSize
    )
{
    PBUSDOG_FILTER_TRACE pTrace = NULL;

    size_t TraceSize;

    size_t BytesWritten = 0;

    PAGED_CODE ();

    WdfWaitLockAcquire(BusDogTraceListLock, NULL);

    while (TRUE)
    {
        TraceSize = __BusDogGetLastTraceSize();
            
        if (TraceSize > BufferSize - BytesWritten)
        {
            KdPrint(("BusDog - No room for next trace\n"));

            break;
        }

        pTrace = __BusDogGetTraceFromList();

        if (pTrace == NULL)
        {
            KdPrint(("BusDog - No more traces\n"));

            break;
        }

        KdPrint(("BusDog - Got trace %d\n", pTrace));

        RtlCopyMemory((PCHAR)Buffer + BytesWritten,
                pTrace,
                TraceSize);

        BytesWritten += TraceSize;

        KdPrint(("     Bytes written %d\n", BytesWritten));

        ExFreePool(pTrace);
    }

    WdfWaitLockRelease(BusDogTraceListLock);

    return BytesWritten;
}


