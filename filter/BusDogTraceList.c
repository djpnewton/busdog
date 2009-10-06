#include "BusDogCommon.h"

//
// This fifo stores all the filter traces
//
BUSDOG_FILTER_TRACE_FIFO    BusDogTraceFifo;
WDFSPINLOCK                 BusDogTraceFifoLock;

NTSTATUS
BusDogTraceFifoInit(
    WDFDRIVER Driver
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    WDF_OBJECT_ATTRIBUTES attributes;

    PAGED_CODE ();

    //
    // Init our trace fifo and create a lock for it
    //

    RtlZeroMemory(&BusDogTraceFifo.TraceItems[0],
        BUSDOG_FILTER_TRACE_FIFO_LENGTH * sizeof(PBUSDOG_FILTER_TRACE_FIFO_ITEM));
    BusDogTraceFifo.WriteIndex = 0;
    BusDogTraceFifo.ReadIndex = 0;

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = Driver;

    status = WdfSpinLockCreate(&attributes,
                                &BusDogTraceFifoLock);
    if (!NT_SUCCESS(status))
    {
        BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfWaitLockCreate failed with status 0x%x\n", status);
    }

    return status;
}

VOID
BusDogTraceFifoCleanUp(
    VOID
    )
{
    ULONG i;

    PAGED_CODE ();

    //
    // Clean up anything in the trace fifo
    //

    WdfSpinLockAcquire(BusDogTraceFifoLock);

    for (i = 0; i < BUSDOG_FILTER_TRACE_FIFO_LENGTH; i++)
    {
        if (BusDogTraceFifo.TraceItems[i] != NULL)
        {
            ExFreePool(BusDogTraceFifo.TraceItems[i]);

            BusDogTraceFifo.TraceItems[i] = NULL;
        }
    }

    BusDogTraceFifo.WriteIndex = 0;

    BusDogTraceFifo.ReadIndex = 0;

    WdfSpinLockRelease(BusDogTraceFifoLock);
}

PBUSDOG_FILTER_TRACE_FIFO_ITEM
__BusDogCreateTrace(
    PBUSDOG_FILTER_TRACE_FIFO_ITEM pTraceItem,
    ULONG DeviceId,
    BUSDOG_REQUEST_TYPE Type,
    BUSDOG_REQUEST_PARAMS Params,
    PVOID TraceBuffer,
    ULONG BufferLength
    )
{
    PBUSDOG_FILTER_TRACE pTrace;
    ULONG requiredItemSize = sizeof(BUSDOG_FILTER_TRACE_FIFO_ITEM) + sizeof(BUSDOG_FILTER_TRACE) + BufferLength;

    //
    // Check if we will need to reallocate the trace item to fit the buffer
    //

    if (pTraceItem != NULL && pTraceItem->ItemSize < requiredItemSize)
    {
        ExFreePool(pTraceItem);

        pTraceItem = NULL;
    }

    //
    // Allocate memory if neccesary
    //

    if (pTraceItem == NULL)
    {

        pTraceItem = ExAllocatePoolWithTag(
                NonPagedPool,
                requiredItemSize,
                'GATT');

        if (pTraceItem == NULL)
        {
            BusDogPrint(BUSDOG_DEBUG_ERROR, "ExAllocatePoolWithTag failed\n");

            return NULL;
        }

        pTraceItem->ItemSize = requiredItemSize;
    }

    //
    // Copy in the trace info
    //

    pTrace = (PBUSDOG_FILTER_TRACE)((PCHAR)pTraceItem + sizeof(BUSDOG_FILTER_TRACE_FIFO_ITEM));

    pTrace->DeviceId = DeviceId;

    pTrace->Type = Type;

    pTrace->Params = Params;

    pTrace->Timestamp = BusDogGetTimeStamp();

    pTrace->BufferSize = BufferLength;

    RtlCopyMemory((PCHAR)pTrace + sizeof(BUSDOG_FILTER_TRACE),
            TraceBuffer,
            BufferLength);

    return pTraceItem;
}

VOID
BusDogAddTraceToFifo(
    WDFDEVICE device,
    ULONG DeviceId,
    BUSDOG_REQUEST_TYPE Type,
    BUSDOG_REQUEST_PARAMS Params,
    PVOID TraceBuffer,
    ULONG BufferLength
    )
{
    PBUSDOG_FILTER_TRACE_FIFO_ITEM pTraceItem = NULL; 

    WdfSpinLockAcquire(BusDogTraceFifoLock);

    pTraceItem = BusDogTraceFifo.TraceItems[BusDogTraceFifo.WriteIndex];

    pTraceItem = 
        __BusDogCreateTrace(
            pTraceItem,
            DeviceId,
            Type,
            Params,
            TraceBuffer,
            BufferLength);

    BusDogTraceFifo.TraceItems[BusDogTraceFifo.WriteIndex] = 
        pTraceItem;

    BusDogTraceFifo.WriteIndex++;

    if (BusDogTraceFifo.WriteIndex >= BUSDOG_FILTER_TRACE_FIFO_LENGTH)
        BusDogTraceFifo.WriteIndex = 0;

    if (BusDogTraceFifo.WriteIndex == BusDogTraceFifo.ReadIndex)
    {
        BusDogPrint(BUSDOG_DEBUG_ERROR, "On noes! We have overflow\n");
    }

    WdfSpinLockRelease(BusDogTraceFifoLock);
}

//
// Assumes trace fifo already locked
//
PBUSDOG_FILTER_TRACE
__BusDogRetrieveTrace(
    VOID
    )
{
    PBUSDOG_FILTER_TRACE pTrace = NULL;

    if (BusDogTraceFifo.ReadIndex != BusDogTraceFifo.WriteIndex)
    {
        PBUSDOG_FILTER_TRACE_FIFO_ITEM pTraceItem = 
            BusDogTraceFifo.TraceItems[BusDogTraceFifo.ReadIndex];

        if (pTraceItem == NULL)
        {
            BusDogPrint(BUSDOG_DEBUG_ERROR, "On noes! invalid trace\n");

            return NULL;
        }

        pTrace = (PBUSDOG_FILTER_TRACE)((PCHAR)pTraceItem + sizeof(BUSDOG_FILTER_TRACE_FIFO_ITEM));

        BusDogTraceFifo.ReadIndex++;

        if (BusDogTraceFifo.ReadIndex >= BUSDOG_FILTER_TRACE_FIFO_LENGTH)
        {
            BusDogTraceFifo.ReadIndex = 0;
        }
    }

    return pTrace;
}

//
// Assumes trace fifo already locked
//
size_t
__BusDogRetrieveTraceSize(
    VOID
    )
{
    PBUSDOG_FILTER_TRACE_FIFO_ITEM pTraceItem = 
        BusDogTraceFifo.TraceItems[BusDogTraceFifo.ReadIndex];

    PBUSDOG_FILTER_TRACE pTrace;

    if (pTraceItem == NULL)
    {
        BusDogPrint(BUSDOG_DEBUG_ERROR, "On noes! invalid trace\n");

        return 0;
    }

    pTrace = (PBUSDOG_FILTER_TRACE)((PCHAR)pTraceItem + sizeof(BUSDOG_FILTER_TRACE_FIFO_ITEM));

    return sizeof(BUSDOG_FILTER_TRACE) + pTrace->BufferSize;
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

    WdfSpinLockAcquire(BusDogTraceFifoLock);

    while (TRUE)
    {
        TraceSize = __BusDogRetrieveTraceSize();
            
        if (TraceSize > BufferSize - BytesWritten)
        {
            BusDogPrint(BUSDOG_DEBUG_WARN, "No room for next trace\n");

            break;
        }

        pTrace = __BusDogRetrieveTrace();

        if (pTrace == NULL)
        {
            BusDogPrint(BUSDOG_DEBUG_INFO, "No more traces\n");

            break;
        }

        BusDogPrint(BUSDOG_DEBUG_INFO, "Got trace %d\n", pTrace);

        RtlCopyMemory((PCHAR)Buffer + BytesWritten,
                pTrace,
                TraceSize);

        BytesWritten += TraceSize;

        BusDogPrint(BUSDOG_DEBUG_INFO, "     Bytes written %d\n", BytesWritten);
    }

    WdfSpinLockRelease(BusDogTraceFifoLock);

    return BytesWritten;
}


