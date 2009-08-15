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

        Node->Trace = NULL;

        ExFreePool(Node);

        Node = NULL;

        BusDogTraceList.Count--;
    }

    WdfWaitLockRelease(BusDogTraceListLock);
    
}

VOID
BusDogAddTraceToList(
    ULONG DeviceId,
    BUSDOG_REQUEST_TYPE Type,
    PVOID TraceBuffer,
    ULONG BufferLength
    )
{
    PBUSDOG_FILTER_TRACE pTrace;
    PBUSDOG_FILTER_TRACE_LLISTITEM pTraceListItem;

    PAGED_CODE ();

    pTrace = ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(BUSDOG_FILTER_TRACE) + BufferLength,
            'GATT');

    if (pTrace == NULL)
    {
        KdPrint(("ExAllocatePoolWithTag failed\n"));
        
        return;
    }
    else
    {
        pTrace->DeviceId = DeviceId;

        pTrace->Type = Type;

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

        pTrace = NULL;

        return;
    }
    else
    {
        pTraceListItem->Prev = NULL;

        pTraceListItem->Trace = pTrace;

        pTraceListItem->Next = NULL;
    }

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
        KdPrint(("On noes! We have overflow\n"));

        pTraceListItem = BusDogTraceList.Tail;

        BusDogTraceList.Tail = pTraceListItem->Prev;

        ExFreePool(pTraceListItem->Trace);

        pTraceListItem->Trace = NULL;

        ExFreePool(pTraceListItem);

        pTraceListItem = NULL;

        BusDogTraceList.Count--;
    }

    WdfWaitLockRelease(BusDogTraceListLock);

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
        else
        {
            BusDogTraceList.Tail = NULL;
        }

        pTrace = Node->Trace;

        ExFreePool(Node);

        Node = NULL;

        BusDogTraceList.Count--;
    }

    return pTrace;
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

    BOOLEAN TraceFitsInBuffer = TRUE;

    PAGED_CODE ();

    WdfWaitLockAcquire(BusDogTraceListLock, NULL);

    do
    {
        pTrace = __BusDogGetTraceFromList();

        if (pTrace != NULL)
        {
            KdPrint(("Got trace %d\n", pTrace));

            TraceSize = sizeof(BUSDOG_FILTER_TRACE) + pTrace->BufferSize;

            if (TraceSize < BufferSize - BytesWritten)
            {
                RtlCopyMemory((PCHAR)Buffer + BytesWritten,
                        pTrace,
                        TraceSize);

                BytesWritten += TraceSize;

                KdPrint(("Bytes written %d\n", BytesWritten));
            }
            else
            {
                // TODO
                //hmm what to do with this trace already taken off the queue but no room for it ???
                //
                //
                KdPrint(("Got trace but no where to put it!\n"));

                TraceFitsInBuffer = FALSE;
            }

            ExFreePool(pTrace);
        }
    }
    while (pTrace != NULL && TraceFitsInBuffer);

    WdfWaitLockRelease(BusDogTraceListLock);

    return BytesWritten;
}


