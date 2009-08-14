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
