#include "BusDogCommon.h"

VOID
PrintChars(
    __in_ecount(CountChars) PCHAR BufferAddress,
    __in ULONG CountChars
    )
{
    PAGED_CODE();

    if (CountChars) {

        while (CountChars--) {

            if (*BufferAddress > 31
                 && *BufferAddress != 127) {

                KdPrint (( "%c", *BufferAddress) );

            } else {

                KdPrint(( ".") );

            }
            BufferAddress++;
        }
        KdPrint (("\n"));
    }
    return;
}

VOID
BusDogUpdateDeviceIds(
    VOID
    )
{
    ULONG         count;
    ULONG         i;

    PAGED_CODE();

    //
    // Aquire busdog device collection lock
    //

    WdfWaitLockAcquire(BusDogDeviceCollectionLock, NULL);

    count = WdfCollectionGetCount(BusDogDeviceCollection);

    //
    // insert new device ids
    //

    for (i = 0; i < count; i++)
    {
        WDFDEVICE device = WdfCollectionGetItem(BusDogDeviceCollection, i);

        PBUSDOG_CONTEXT context = BusDogGetDeviceContext(device);
        
        if (!context->HasDeviceId)
        {
            ULONG j;
            ULONG newDevId = 0;
            BOOLEAN idColision;

            do
            {
                idColision = FALSE;

                for (j = 0; j < count; j++)
                {
                    PBUSDOG_CONTEXT contextCompare = 
                        BusDogGetDeviceContext(
                                WdfCollectionGetItem(BusDogDeviceCollection, j));


                    if (contextCompare->HasDeviceId && contextCompare->DeviceId == newDevId)
                    {
                        newDevId++;

                        idColision = TRUE;

                        break;
                    }
                }
            }
            while (idColision);

            context->HasDeviceId = TRUE;

            context->DeviceId = newDevId;

            KdPrint(("BusDog - New DeviceId: %2d\n", newDevId));
        }
    }

    //
    // Release busdog device collection lock
    //

    WdfWaitLockRelease(BusDogDeviceCollectionLock);
}


BOOLEAN
BusDogFillBufferWithDeviceIds(
    PVOID Buffer,
    size_t BufferSize,
    size_t* BytesWritten,
    size_t* BytesNeeded
    )
{

    ULONG         count;
    ULONG         i;
    BOOLEAN       Result = TRUE;

    PAGED_CODE();

    *BytesWritten = 0;
    *BytesNeeded = 0;

    //
    // Aquire busdog device collection lock
    //

    WdfWaitLockAcquire(BusDogDeviceCollectionLock, NULL);

    count = WdfCollectionGetCount(BusDogDeviceCollection);

    for (i = 0; i < count ; i++) 
    {

        NTSTATUS status;
        WDFDEVICE device;
        PBUSDOG_CONTEXT context;
        WDF_OBJECT_ATTRIBUTES attributes;
        WDFMEMORY memory;
        size_t hwidMemorySize;
        PVOID hwidBuffer;
        UNICODE_STRING hardwareId;
        size_t deviceIdSize;
        PBUSDOG_DEVICE_ID pDeviceId;

        //
        // Get our device and context
        //

        device = WdfCollectionGetItem(BusDogDeviceCollection, i);

        context = BusDogGetDeviceContext(device);

        //
        // Put the hardware id of a device into a unicode string
        //

        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        attributes.ParentObject = device;

        status = WdfDeviceAllocAndQueryProperty(device,
                DevicePropertyHardwareID,
                NonPagedPool,
                &attributes,
                &memory
                );

        if (!NT_SUCCESS(status)) 
        {
            KdPrint(("WdfDeviceAllocAndQueryProperty failed - 0x%x\n",
                        status));

            continue;
        }

        hwidBuffer = WdfMemoryGetBuffer(memory, &hwidMemorySize);

        if (hwidBuffer == NULL) 
        {   
            KdPrint(("WdfMemoryGetBuffer failed\n"));  

            continue;   
        }   

        // assuming here that the string is null-terminated (hope this doesnt bite me later)
        RtlInitUnicodeString(&hardwareId, hwidBuffer);

        //
        // Print item
        //

        KdPrint(("%2d - Enabled: %d, HardwareId: %wZ\n", context->DeviceId, context->FilterEnabled, &hardwareId));

        //
        // Copy Item to user buffer
        //

        deviceIdSize = sizeof(BUSDOG_DEVICE_ID) + hardwareId.Length;

        if (deviceIdSize > BufferSize - *BytesWritten)
        {
            Result = FALSE;

            KdPrint(("BusDog - No room for device id\n"));
        }
        else
        {
            pDeviceId = (PBUSDOG_DEVICE_ID)((PCHAR)Buffer + *BytesWritten);

            pDeviceId->DeviceId = context->DeviceId;

            pDeviceId->Enabled = context->FilterEnabled;

            pDeviceId->HardwareIdSize = hardwareId.Length;

            RtlCopyMemory((PCHAR)Buffer + *BytesWritten + sizeof(BUSDOG_DEVICE_ID),
                          hardwareId.Buffer,
                          hardwareId.Length);

            *BytesWritten += deviceIdSize;
        }

        *BytesNeeded += deviceIdSize;

    }
    
    //
    // Release busdog device collection lock
    //

    WdfWaitLockRelease(BusDogDeviceCollectionLock);

    return Result;
}
