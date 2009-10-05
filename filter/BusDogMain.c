#include "BusDogCommon.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, BusDogDeviceAdd)
#pragma alloc_text (PAGE, BusDogDeviceContextCleanup)

#endif

//
// ControlDevice provides a sideband communication to the filter from
// usermode. This is required if the filter driver is sitting underneath
// another driver that fails custom ioctls defined by the Filter driver.
// Since there is one control-device for all instances of the device the
// filter is attached to, we will store the device handle in a global variable.
//
WDFDEVICE       ControlDevice = NULL;

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, BusDogIoDeviceControl)
#pragma alloc_text (PAGE, BusDogCreateControlDevice)
#pragma alloc_text (PAGE, BusDogDeleteControlDevice)
#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    Installable driver initialization entry point.
    This entry point is called directly by the I/O system.

Arguments:

    DriverObject - pointer to the driver object

    RegistryPath - pointer to a unicode string representing the path,
                   to driver-specific key in the registry.

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.

--*/
{
    WDF_DRIVER_CONFIG   config;
    NTSTATUS            status;
    WDF_OBJECT_ATTRIBUTES colAttributes;
    WDFDRIVER   hDriver;

    BusDogPrint(BUSDOG_DEBUG_INFO, "DriverEntry\n");
    BusDogPrint(BUSDOG_DEBUG_INFO, "Built %s %s\n", __DATE__, __TIME__);

    //
    // Initiialize driver config to control the attributes that
    // are global to the driver. Note that framework by default
    // provides a driver unload routine. If you create any resources
    // in the DriverEntry and want to be cleaned in driver unload,
    // you can override that by manually setting the EvtDriverUnload in the
    // config structure. In general xxx_CONFIG_INIT macros are provided to
    // initialize most commonly used members.
    //

    WDF_DRIVER_CONFIG_INIT(
        &config,
        BusDogDeviceAdd
    );

    config.EvtDriverUnload = BusDogDriverUnload;

    //
    // Create a framework driver object to represent our driver.
    //
    status = WdfDriverCreate(DriverObject,
                            RegistryPath,
                            WDF_NO_OBJECT_ATTRIBUTES,
                            &config,
                            &hDriver);
    if (!NT_SUCCESS(status)) {
        BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfDriverCreate failed with status 0x%x\n", status);
    }

    //
    // Since there is only one control-device for all the instances
    // of the physical device, we need an ability to get to particular instance
    // of the device in our BusDogIoDeviceControl. For that we
    // will create a collection object and store filter device objects.
    //
    WDF_OBJECT_ATTRIBUTES_INIT(&colAttributes);
    colAttributes.ParentObject = hDriver;

    status = WdfCollectionCreate(&colAttributes,
                                &BusDogDeviceCollection);
    if (!NT_SUCCESS(status))
    {
        BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfCollectionCreate failed with status 0x%x\n", status);
        return status;
    }

    WDF_OBJECT_ATTRIBUTES_INIT(&colAttributes);
    colAttributes.ParentObject = hDriver;

    status = WdfWaitLockCreate(&colAttributes,
                                &BusDogDeviceCollectionLock);
    if (!NT_SUCCESS(status))
    {
        BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfWaitLockCreate failed with status 0x%x\n", status);
        return status;
    }

    //
    // Init trace fifo
    //

    status = BusDogTraceFifoInit(hDriver);


    return status;
}

VOID
BusDogDriverUnload (
    IN WDFDRIVER  Driver
    )
{
    BusDogPrint(BUSDOG_DEBUG_INFO, "DriverUnload.\n");

    //
    // Clean up the trace fifo
    //

    BusDogTraceFifoCleanUp();

}

NTSTATUS
BusDogDeviceAdd(
    IN WDFDRIVER        Driver,
    IN PWDFDEVICE_INIT  DeviceInit
    )
/*++
Routine Description:

    EvtDeviceAdd is called by the framework in response to AddDevice
    call from the PnP manager. Here you can query the device properties
    using WdfFdoInitWdmGetPhysicalDevice/IoGetDeviceProperty and based
    on that, decide to create a filter device object and attach to the
    function stack. If you are not interested in filtering this particular
    instance of the device, you can just return STATUS_SUCCESS without creating
    a framework device.

Arguments:

    Driver - Handle to a framework driver object created in DriverEntry

    DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

Return Value:

    NTSTATUS

--*/
{
    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
    PBUSDOG_CONTEXT         context;
    NTSTATUS                status;
    WDFDEVICE               device;
    ULONG                   serialNo;
    ULONG                   returnSize;
    WDF_IO_QUEUE_CONFIG     ioQueueConfig;

    PAGED_CODE ();

    UNREFERENCED_PARAMETER(Driver);

    //
    // Get some property of the device you are about to attach and check
    // to see if that's the one you are interested. For demonstration
    // we will get the UINumber of the device. The bus driver reports the
    // serial number as the UINumber.
    //
    status = WdfFdoInitQueryProperty(DeviceInit,
                                  DevicePropertyUINumber,
                                  sizeof(serialNo),
                                  &serialNo,
                                  &returnSize);
    if(!NT_SUCCESS(status)){
        BusDogPrint(BUSDOG_DEBUG_ERROR, "Failed to get the property of PDO: 0x%p\n", DeviceInit);

    }

    //
    // Tell the framework that you are filter driver. Framework
    // takes care of inherting all the device flags & characterstics
    // from the lower device you are attaching to.
    //
    WdfFdoInitSetFilter(DeviceInit);

#ifdef WDM_PREPROCESS

    //
    // Lets see if we can hook into some IRPs
    //
    status = WdfDeviceInitAssignWdmIrpPreprocessCallback(
                                            DeviceInit,
                                            BusDogWdmDeviceReadWrite,
                                            IRP_MJ_READ,
                                            NULL, // pointer minor function table
                                            0); // number of entries in the table
    if (!NT_SUCCESS(status)) {
        BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfDeviceInitAssignWdmIrpPreprocessCallback failed with status 0x%x\n",
                                status);

        //
        // Let us not fail AddDevice just because we weren't able to hook up
        // the callback
        //
        status = STATUS_SUCCESS;
    }

    status = WdfDeviceInitAssignWdmIrpPreprocessCallback(
                                            DeviceInit,
                                            BusDogWdmDeviceReadWrite,
                                            IRP_MJ_WRITE,
                                            NULL, // pointer minor function table
                                            0); // number of entries in the table
    if (!NT_SUCCESS(status)) {
        BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfDeviceInitAssignWdmIrpPreprocessCallback failed with status 0x%x\n",
                                status);

        //
        // Let us not fail AddDevice just because we weren't able to hook up
        // the callback
        //
        status = STATUS_SUCCESS;
    }

#endif

    //
    // Specify the size of device extension where we track per device
    // context.
    //

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, BUSDOG_CONTEXT);

    //
    // We will just register for cleanup notification because we have to
    // delete the control-device when the last instance of the device goes
    // away. If we don't delete, the driver wouldn't get unloaded automatcially
    // by the PNP subsystem.
    //
    deviceAttributes.EvtCleanupCallback = BusDogDeviceContextCleanup;

    //
    // Create a framework device object.This call will inturn create
    // a WDM deviceobject, attach to the lower stack and set the
    // appropriate flags and attributes.
    //
    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (!NT_SUCCESS(status)) {
        BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfDeviceCreate failed with status code 0x%x\n", status);
        return status;
    }

    //
    // Get our filter context
    //

    context = BusDogGetDeviceContext(device);

    //
    // Initialize our context
    //
    context->MagicNumber = DEVICE_CONTEXT_MAGIC;
    context->SerialNo = serialNo;
    context->HasDeviceId = FALSE;
    context->DeviceId = -1;
    context->FilterEnabled = FALSE;

    // Figure out where we'll be sending all our requests
    //  once we're done with them
    context->TargetToSendRequestsTo = WdfDeviceGetIoTarget(device);
    

#ifndef WDM_PREPROCESS

    //
    // Now that this step is completed, we can create our default queue.
    //  This queue will allow us to pick off any I/O requests
    //  that we may be interested in before they are forwarded
    //  to the target device.
    //

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig,
                                           WdfIoQueueDispatchParallel);

    //
    // We just want reads and writes for now...
    //

    ioQueueConfig.EvtIoRead = BusDogIoRead;
    ioQueueConfig.EvtIoWrite = BusDogIoWrite;
    ioQueueConfig.EvtIoInternalDeviceControl = BusDogIoInternalDeviceControl;

    //
    // Create the queue...
    //
    status = WdfIoQueueCreate(device,
                              &ioQueueConfig,
                              WDF_NO_OBJECT_ATTRIBUTES,
                              NULL);

    if (!NT_SUCCESS(status)) 
    {
        //
        // No need to delete the WDFDEVICE we created, the framework
        //  will clean that up for us.
        //
        BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfIoQueueCreate failed with status code 0x%x\n", status);
        
        //
        // Let us not fail AddDevice just because we weren't able to hook up
        // the callback
        //
        status = STATUS_SUCCESS;
    }    

#endif

    //
    // Add this device to the FilterDevice collection.
    //
    WdfWaitLockAcquire(BusDogDeviceCollectionLock, NULL);
    //
    // WdfCollectionAdd takes a reference on the item object and removes
    // it when you call WdfCollectionRemove.
    //
    status = WdfCollectionAdd(BusDogDeviceCollection, device);
    if (!NT_SUCCESS(status)) {
        BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfCollectionAdd failed with status code 0x%x\n", status);
    }
    WdfWaitLockRelease(BusDogDeviceCollectionLock);

    //
    // Create a control device
    //

    status = BusDogCreateControlDevice(device);
    if (!NT_SUCCESS(status)) {
        BusDogPrint(BUSDOG_DEBUG_ERROR, "BusDogCreateControlDevice failed with status 0x%x\n",
                                status);
        //
        // Let us not fail AddDevice just because we weren't able to create the
        // control device.
        //
        status = STATUS_SUCCESS;
    }

    //
    // Update device ids
    //

    BusDogUpdateDeviceIds();

    return status;
}

VOID
BusDogDeviceContextCleanup(
    IN WDFDEVICE Device
    )
/*++

Routine Description:

   EvtDeviceRemove event callback must perform any operations that are
   necessary before the specified device is removed. The framework calls
   the driver's EvtDeviceRemove callback when the PnP manager sends
   an IRP_MN_REMOVE_DEVICE request to the driver stack.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    WDF status code

--*/
{
    ULONG   count;

    PAGED_CODE();

    BusDogPrint(BUSDOG_DEBUG_INFO, "Entered BusDogDeviceContextCleanup\n");

    WdfWaitLockAcquire(BusDogDeviceCollectionLock, NULL);

    count = WdfCollectionGetCount(BusDogDeviceCollection);

    if (count == 1)
    {
        //
        // We are the last instance. So let us delete the control-device
        // so that driver can unload when the FilterDevice is deleted.
        // We absolutely have to do the deletion of control device with
        // the collection lock acquired because we implicitly use this
        // lock to protect ControlDevice global variable. We need to make
        // sure another thread doesn't attempt to create while we are
        // deleting the device.
        //
        BusDogDeleteControlDevice(Device);
    }

    WdfCollectionRemove(BusDogDeviceCollection, Device);

    WdfWaitLockRelease(BusDogDeviceCollectionLock);

}

NTSTATUS
BusDogCreateControlDevice(
    WDFDEVICE Device
    )
/*++

Routine Description:

    This routine is called to create a control deviceobject so that application
    can talk to the filter driver directly instead of going through the entire
    device stack. This kind of control device object is useful if the filter
    driver is underneath another driver which prevents ioctls not known to it
    or if the driver's dispatch routine is owned by some other (port/class)
    driver and it doesn't allow any custom ioctls.

    NOTE: Since the control device is global to the driver and accessible to
    all instances of the device this filter is attached to, we create only once
    when the first instance of the device is started and delete it when the
    last instance gets removed.

Arguments:

    Device - Handle to a filter device object.

Return Value:

    WDF status code

--*/
{
    PWDFDEVICE_INIT             pInit = NULL;
    WDFDEVICE                   controlDevice = NULL;
    WDF_OBJECT_ATTRIBUTES       controlAttributes;
    WDF_IO_QUEUE_CONFIG         ioQueueConfig;
    BOOLEAN                     bCreate = FALSE;
    NTSTATUS                    status;
    WDFQUEUE                    queue;
    DECLARE_CONST_UNICODE_STRING(ntDeviceName, NTDEVICE_NAME_STRING) ;
    DECLARE_CONST_UNICODE_STRING(symbolicLinkName, SYMBOLIC_NAME_STRING) ;

    PAGED_CODE();

    //
    // First find out whether any ControlDevice has been created. If the
    // collection has more than one device then we know somebody has already
    // created or in the process of creating the device.
    //
    WdfWaitLockAcquire(BusDogDeviceCollectionLock, NULL);

    if(WdfCollectionGetCount(BusDogDeviceCollection) == 1) {
        bCreate = TRUE;
    }

    WdfWaitLockRelease(BusDogDeviceCollectionLock);

    if(!bCreate) {
        //
        // Control device is already created. So return success.
        //
        return STATUS_SUCCESS;
    }

    BusDogPrint(BUSDOG_DEBUG_INFO, "Creating Control Device\n");

    //
    //
    // In order to create a control device, we first need to allocate a
    // WDFDEVICE_INIT structure and set all properties.
    //
    pInit = WdfControlDeviceInitAllocate(
                            WdfDeviceGetDriver(Device),
                            &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R
                            );

    if (pInit == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

    //
    // Set exclusive to false so that more than one app can talk to the
    // control device simultaneously.
    //
    WdfDeviceInitSetExclusive(pInit, FALSE);

    status = WdfDeviceInitAssignName(pInit, &ntDeviceName);

    if (!NT_SUCCESS(status)) {
        goto Error;
    }

    //
    // Specify the size of device context
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&controlAttributes,
                                    CONTROL_DEVICE_EXTENSION);
    status = WdfDeviceCreate(&pInit,
                             &controlAttributes,
                             &controlDevice);
    if (!NT_SUCCESS(status)) {
        goto Error;
    }

    //
    // Create a symbolic link for the control object so that usermode can open
    // the device.
    //

    status = WdfDeviceCreateSymbolicLink(controlDevice,
                                &symbolicLinkName);

    if (!NT_SUCCESS(status)) {
        goto Error;
    }

    //
    // Configure the default queue associated with the control device object
    // to be Serial so that request passed to EvtIoDeviceControl are serialized.
    //

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig,
                             WdfIoQueueDispatchSequential);

    ioQueueConfig.EvtIoDeviceControl = BusDogIoDeviceControl;

    //
    // Framework by default creates non-power managed queues for
    // filter drivers.
    //
    status = WdfIoQueueCreate(controlDevice,
                                        &ioQueueConfig,
                                        WDF_NO_OBJECT_ATTRIBUTES,
                                        &queue // pointer to default queue
                                        );
    if (!NT_SUCCESS(status)) {
        goto Error;
    }

    //
    // Control devices must notify WDF when they are done initializing.   I/O is
    // rejected until this call is made.
    //
    WdfControlFinishInitializing(controlDevice);

    ControlDevice = controlDevice;

    return STATUS_SUCCESS;

Error:

    if (pInit != NULL) {
        WdfDeviceInitFree(pInit);
    }

    if (controlDevice != NULL) {
        //
        // Release the reference on the newly created object, since
        // we couldn't initialize it.
        //
        WdfObjectDelete(controlDevice);
        controlDevice = NULL;
    }

    return status;
}

VOID
BusDogDeleteControlDevice(
    WDFDEVICE Device
    )
/*++

Routine Description:

    This routine deletes the control by doing a simple dereference.

Arguments:

    Device - Handle to a framework filter device object.

Return Value:

    WDF status code

--*/
{
    UNREFERENCED_PARAMETER(Device);

    PAGED_CODE();

    BusDogPrint(BUSDOG_DEBUG_INFO, "Deleting Control Device\n");

    if (ControlDevice) {
        WdfObjectDelete(ControlDevice);
        ControlDevice = NULL;
    }
}

VOID
BusDogIoDeviceControl(
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN size_t           OutputBufferLength,
    IN size_t           InputBufferLength,
    IN ULONG            IoControlCode
    )
/*++
Routine Description:

    This event is called when the framework receives IRP_MJ_DEVICE_CONTROL
    requests from the system.

Arguments:

    Queue - Handle to the framework queue object that is associated
            with the I/O request.
    Request - Handle to a framework request object.

    OutputBufferLength - length of the request's output buffer,
                        if an output buffer is available.
    InputBufferLength - length of the request's input buffer,
                        if an input buffer is available.

    IoControlCode - the driver-defined or system-defined I/O control code
                    (IOCTL) that is associated with the request.

Return Value:

   VOID

--*/
{
    NTSTATUS               status = STATUS_SUCCESS;
    ULONG                  i;
    ULONG                  noItems;
    WDFDEVICE              hFilterDevice;
    PBUSDOG_CONTEXT        context;
    PVOID                  outputBuffer = NULL;
    PBUSDOG_FILTER_ENABLED filterEnabledBuffer;
    PBUSDOG_DEBUG_LEVEL    debugLevelBuffer;
    size_t                 realLength;
    size_t                 bytesWritten;
    size_t                 bytesNeeded;

    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    PAGED_CODE();

    BusDogPrint(BUSDOG_DEBUG_INFO, "Ioctl recieved into filter control object.\n");

    //
    // What IOCTL have we got?
    //
    switch (IoControlCode)
    {
        case IOCTL_BUSDOG_GET_BUFFER:

            if (InputBufferLength) 
            {

                BusDogPrint(BUSDOG_DEBUG_WARN, "Sorry buddy...No input buffers allowed\n");

                WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);

                return;

            }

            BusDogPrint(BUSDOG_DEBUG_INFO, "Get buffer\n");

            //
            // Get the output buffer...
            //
            status = WdfRequestRetrieveOutputBuffer(Request,
                    sizeof(BUSDOG_FILTER_TRACE),
                    &outputBuffer,
                    &realLength);

            if (!NT_SUCCESS(status)) 
            {
                BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfRequestRetrieveOutputBuffer failed - 0x%x\n",
                            status);

                WdfRequestComplete(Request, status);

                return;
            }

            //
            // Fill buffer with traces
            //
            
            bytesWritten = BusDogFillBufferWithTraces(outputBuffer, realLength);

            //
            // Yes! Return to the user, telling them how many bytes
            //  we copied....
            //
            WdfRequestCompleteWithInformation(Request, 
                                              STATUS_SUCCESS,
                                              bytesWritten);

            return;

        case IOCTL_BUSDOG_START_FILTERING:

            //
            // Set global filtering var
 
            BusDogPrint(BUSDOG_DEBUG_INFO, "Filtering enabled\n");

            BusDogFiltering = TRUE;

            BusDogTraceFifoCleanUp();

            WdfRequestComplete(Request, STATUS_SUCCESS);

            return;

        case IOCTL_BUSDOG_STOP_FILTERING:

            //
            // Set global filtering var
            //
            
            BusDogPrint(BUSDOG_DEBUG_INFO, "Filtering disabled\n");

            BusDogFiltering = FALSE;

            BusDogTraceFifoCleanUp();

            WdfRequestComplete(Request, STATUS_SUCCESS);
            
            return;

        case IOCTL_BUSDOG_PRINT_DEVICES: 

            BusDogPrint(BUSDOG_DEBUG_WARN, "IOCTL_BUSDOG_PRINT_DEVICES is depreciated\n");

            WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);

            return;

        case IOCTL_BUSDOG_SET_DEVICE_FILTER_ENABLED:
        {
            BOOLEAN foundDevice = FALSE;

            if (!InputBufferLength)
            {
                WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);

                return;
            }

            //
            // Get the input buffer...
            //
            status = WdfRequestRetrieveInputBuffer(Request,
                    sizeof(BUSDOG_FILTER_ENABLED),
                    (PVOID *)&filterEnabledBuffer,
                    &realLength);

            if (!NT_SUCCESS(status)) 
            {
                BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfRequestRetrieveOutputBuffer failed - 0x%x\n",
                            status);

                WdfRequestComplete(Request, status);

                return;
            }

            //
            // Find device and enable/disable it
            //

            WdfWaitLockAcquire(BusDogDeviceCollectionLock, NULL);

            noItems = WdfCollectionGetCount(BusDogDeviceCollection);

            for (i = 0; i < noItems; i++)
            {
                //
                // Get our device and context
                //

                hFilterDevice = WdfCollectionGetItem(BusDogDeviceCollection, i);

                context = BusDogGetDeviceContext(hFilterDevice);


                if (filterEnabledBuffer->DeviceId == context->DeviceId)
                {
                    //
                    // Set filter enabled
                    //

                    context->FilterEnabled = filterEnabledBuffer->FilterEnabled;

                    BusDogPrint(BUSDOG_DEBUG_INFO, "%d - FilterEnabled: %d\n", filterEnabledBuffer->DeviceId, context->FilterEnabled);

                    foundDevice = TRUE;

                    break;

                }
            }

            if (!foundDevice)
            {
                BusDogPrint(BUSDOG_DEBUG_WARN, "Error DeviceId (%d) is not valid\n", filterEnabledBuffer->DeviceId);

                status = STATUS_NO_SUCH_DEVICE;
            }

            WdfWaitLockRelease(BusDogDeviceCollectionLock);

            WdfRequestCompleteWithInformation(Request, status, realLength);

            return;
        }

        case IOCTL_BUSDOG_GET_DEVICE_LIST:

            if (InputBufferLength) 
            {

                BusDogPrint(BUSDOG_DEBUG_WARN, "Sorry buddy...No input buffers allowed\n");

                WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);

                return;

            }

            BusDogPrint(BUSDOG_DEBUG_INFO, "get device list\n");

            //
            // Get the output buffer...
            //
            status = WdfRequestRetrieveOutputBuffer(Request,
                    sizeof(BUSDOG_FILTER_TRACE),
                    &outputBuffer,
                    &realLength);

            if (!NT_SUCCESS(status)) 
            {
                BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfRequestRetrieveOutputBuffer failed - 0x%x\n",
                            status);

                WdfRequestComplete(Request, status);

                return;
            }

            //
            // Fill buffer with device ids
            //
            
            if (BusDogFillBufferWithDeviceIds(outputBuffer, 
                                              realLength, 
                                              &bytesWritten,
                                              &bytesNeeded)
               )
            {

                //
                // Yes! Return to the user, telling them how many bytes
                //  we copied....
                //
                WdfRequestCompleteWithInformation(Request, 
                        STATUS_SUCCESS,
                        bytesWritten);

            }
            else
            {

                //
                // No! Return to the user, telling them how many bytes
                //  we need
                //
                WdfRequestCompleteWithInformation(Request, 
                        STATUS_BUFFER_TOO_SMALL,
                        bytesNeeded);
            }

            return;

        case IOCTL_BUSDOG_GET_DEBUG_LEVEL:

            if (InputBufferLength) 
            {

                BusDogPrint(BUSDOG_DEBUG_WARN, "Sorry buddy...No input buffers allowed\n");

                WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);

                return;

            }

            BusDogPrint(BUSDOG_DEBUG_INFO, "get debug level\n");

            //
            // Get the output buffer...
            //
            status = WdfRequestRetrieveOutputBuffer(Request,
                    sizeof(BUSDOG_DEBUG_LEVEL),
                    &outputBuffer,
                    &realLength);

            if (!NT_SUCCESS(status)) 
            {
                BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfRequestRetrieveOutputBuffer failed - 0x%x\n",
                            status);

                WdfRequestComplete(Request, status);

                return;
            }

            //
            // Fill buffer debug level info
            //
            
            ((PBUSDOG_DEBUG_LEVEL)outputBuffer)->DebugLevel = BusDogDebugLevel;
            
            //
            // Yes! Return to the user, telling them how many bytes
            //  we copied....
            //
            WdfRequestCompleteWithInformation(Request, 
                    STATUS_SUCCESS,
                    realLength);

            return;

        case IOCTL_BUSDOG_SET_DEBUG_LEVEL:

            if (!InputBufferLength)
            {
                WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);

                return;
            }

            //
            // Get the input buffer...
            //
            status = WdfRequestRetrieveInputBuffer(Request,
                    sizeof(BUSDOG_DEBUG_LEVEL),
                    (PVOID *)&debugLevelBuffer,
                    &realLength);

            if (!NT_SUCCESS(status)) 
            {
                BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfRequestRetrieveOutputBuffer failed - 0x%x\n",
                            status);

                WdfRequestComplete(Request, status);

                return;
            }

            //
            // Set debug level
            //

            BusDogDebugLevel = debugLevelBuffer->DebugLevel;

            WdfRequestCompleteWithInformation(Request, status, realLength);

            return;

        default:

            break;
    }

    WdfRequestComplete(Request, STATUS_INVALID_DEVICE_REQUEST);
}


#ifdef WDM_PREPROCESS

NTSTATUS
BusDogWdmDeviceReadWrite (
    IN WDFDEVICE Device,
    IN PIRP Irp
    )
{
    PBUSDOG_CONTEXT         context = BusDogGetDeviceContext(Device);

    if (BusDogFiltering && context->FilterEnabled)
    {
        PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

        if (stack->MajorFunction == IRP_MJ_READ)
        {
            BusDogPrint(BUSDOG_DEBUG_INFO, "%2d - IRP_MJ_READ, Length: %d\n", context->DeviceId, stack->Parameters.Read.Length);
        }
        else if (stack->MajorFunction == IRP_MJ_WRITE)
        {
            BusDogPrint(BUSDOG_DEBUG_INFO, "%2d - IRP_MJ_WRITE, Length: %d\n", context->DeviceId, stack->Parameters.Write.Length);
        }

        if (FlagOn(stack->DeviceObject->Flags, DO_BUFFERED_IO)) 
        {
            //buffer at Irp->AssociatedIrp.SystemBuffer
            BusDogPrint(BUSDOG_DEBUG_INFO, "         DO_BUFFERED_IO - Irp->AssociatedIrp.SystemBuffer: %d\n", Irp->AssociatedIrp.SystemBuffer);
        }
        else if (FlagOn(stack->DeviceObject->Flags, DO_DIRECT_IO)) 
        {
            PVOID buffer;

            // buffer at Irp->MdlAddress
            BusDogPrint(BUSDOG_DEBUG_INFO, "         DO_DIRECT_IO - Irp->MdlAddress: %d\n", Irp->MdlAddress);

            //         
            // Map the physical pages described by the MDL into system space. 
            // Note: double mapping the buffer this way causes lot of 
            // system overhead for large size buffers. 
            // 

            buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);

            if (!buffer) 
            {
                //ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                //MmUnlockPages(mdl);
                //IoFreeMdl(mdl);
            }
            else
            {
                // 
                // Now you can safely read the data from the buffer.
                //
                BusDogPrint(BUSDOG_DEBUG_INFO, "         Data: ");
                PrintChars(buffer, MmGetMdlByteCount(Irp->MdlAddress));
            }

        }
        else 
        {
            // buffer at Irp->UserBuffer 
            BusDogPrint(BUSDOG_DEBUG_INFO, "         Neither - Irp->UserBuffer: %d\n", Irp->UserBuffer);
        }
    }

    IoSkipCurrentIrpStackLocation(Irp); 
    return WdfDeviceWdmDispatchPreprocessedIrp(Device, Irp);
}

#else

VOID
BusDogIoRead(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t Length
    ) 
{

    NTSTATUS              status;
    PCHAR                 dataBuffer = NULL;
    WDFDEVICE             device = WdfIoQueueGetDevice(Queue);
    PBUSDOG_CONTEXT       context = BusDogGetDeviceContext(device);

    if (BusDogFiltering && context->FilterEnabled)
    {
        //
        // Retrieve some parameters of the request to print 
        //  to the debugger
        //
        if (Length != 0) 
        {

            //
            // This is a non-zero length transfer, retrieve
            //  the data buffer.
            //
            status = WdfRequestRetrieveOutputBuffer(Request,
                    Length,
                    (PVOID *)&dataBuffer,
                    NULL);

            if (NT_SUCCESS(status)) 
            {
                //
                // Print the info to the debugger
                //
                BusDogPrint(BUSDOG_DEBUG_INFO, "BusDogIoRead       %2d: Length-0x%x Data-", context->DeviceId, Length);
                PrintChars(dataBuffer, Length);

                BusDogAddTraceToFifo(device, context->DeviceId, BusDogReadRequest, dataBuffer, Length);
            }
            else
            {

                //
                // Not good. We'll still pass the request down
                //  and let the next device decide what to do with 
                //  this.
                //
                BusDogPrint(BUSDOG_DEBUG_INFO, "RetrieveOutputBuffer failed - 0x%x\n", 
                            status);
            }

        }
    }

    //
    // For reads, we want completion info. Call the helper
    //  routine to forward the request with a completion routine.
    //
    BusDogForwardRequestWithCompletion(WdfIoQueueGetDevice(Queue), 
                                        Request,
                                        BusDogReadComplete,
                                        NULL);
    return;

}

VOID
BusDogReadComplete(
    IN WDFREQUEST Request,
    IN WDFIOTARGET Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS Params,
    IN WDFCONTEXT Context
    ) 
{
    PBUSDOG_CONTEXT       context = BusDogGetDeviceContext(WdfIoQueueGetDevice(WdfRequestGetIoQueue(Request)));

    if (BusDogFiltering && context->FilterEnabled)
    {
        //
        // And print the information to the debugger
        //
        BusDogPrint(BUSDOG_DEBUG_INFO, "BusDogReadComplete %2d: Status-0x%x; Information-0x%x\n",
                    context->DeviceId,
                    Params->IoStatus.Status, 
                    Params->IoStatus.Information);
    }

    //
    // Restart completion processing.
    //
    WdfRequestComplete(Request, Params->IoStatus.Status);
    
}

VOID
BusDogIoWrite(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t Length
    ) 
{

    NTSTATUS                 status;
    PUCHAR                   dataBuffer = NULL;
    WDFDEVICE                device = WdfIoQueueGetDevice(Queue);
    PBUSDOG_CONTEXT          context = BusDogGetDeviceContext(device);

    if (BusDogFiltering && context->FilterEnabled)
    {
        //
        // Retrieve some parameters of the request to print
        //  to the debugger
        //
        if (Length != 0) 
        {

            //
            // This is a non-zero length transfer, retrieve
            //  the data buffer.
            //

            status = WdfRequestRetrieveInputBuffer(Request,
                    Length,
                    (PVOID *)&dataBuffer,
                    NULL);

            if (NT_SUCCESS(status)) 
            {
                //
                // Print the info to the debugger
                //
                BusDogPrint(BUSDOG_DEBUG_INFO, "BusDogIoWrite      %2d: Length-0x%x Data-", context->DeviceId, Length);
                PrintChars(dataBuffer, Length);

                BusDogAddTraceToFifo(device, context->DeviceId, BusDogWriteRequest, dataBuffer, Length);
            }
            else
            {
                //
                // Not good. We'll still pass the request down
                //  and let the next device decide what to do with
                //  this.
                //
                BusDogPrint(BUSDOG_DEBUG_ERROR, "RetrieveInputBuffer failed - 0x%x\n",
                            status);
            }

        }
    }

    //
    // For simplicity sake, we'll just send and forget writes
    //
    BusDogForwardRequest(WdfIoQueueGetDevice(Queue), 
                          Request);

    return;
}

VOID
BusDogForwardRequest(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request
    ) 
{

    WDF_REQUEST_SEND_OPTIONS options;
    PBUSDOG_CONTEXT          context;
    NTSTATUS                 status;

    //
    // Get the context that we setup during DeviceAdd processing
    //
    context = BusDogGetDeviceContext(Device);

    ASSERT(IS_DEVICE_CONTEXT(context));

    //
    // We're just going to be passing this request on with 
    //  zero regard for what happens to it. Therefore, we'll
    //  use the WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET option
    //
    WDF_REQUEST_SEND_OPTIONS_INIT(
                        &options,
                        WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);


    //
    // And send it!
    // 
    if (!WdfRequestSend(Request, 
                        context->TargetToSendRequestsTo, 
                        &options)) {

        //
        // Oops! Something bad happened, complete the request
        //
        status = WdfRequestGetStatus(Request);
        BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfRequestSend failed - 0x%x\n", status);
        WdfRequestComplete(Request, status);
    }
    return;
}

VOID
BusDogForwardRequestWithCompletion(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request,
    IN PFN_WDF_REQUEST_COMPLETION_ROUTINE CompletionRoutine,
    IN WDFCONTEXT CompletionContext
    ) {

    PBUSDOG_CONTEXT context;
    NTSTATUS        status;

    //
    // Get the context that we setup during DeviceAdd processing
    //
    context = BusDogGetDeviceContext(Device);

    ASSERT(IS_DEVICE_CONTEXT(context));

    //
    // Setup the request for the next driver
    //
    WdfRequestFormatRequestUsingCurrentType(Request);

    //
    // Set the completion routine...
    //
    WdfRequestSetCompletionRoutine(Request,
                                   CompletionRoutine,
                                   CompletionContext);

    //
    // And send it!
    // 
    if (!WdfRequestSend(Request, 
                        context->TargetToSendRequestsTo, 
                        NULL)) {
        //
        // Oops! Something bad happened, complete the request
        //
        status = WdfRequestGetStatus(Request);
        BusDogPrint(BUSDOG_DEBUG_ERROR, "WdfRequestSend failed - 0x%x\n", status);
        WdfRequestComplete(Request, status);
    }
    return;
}

VOID
BusDogIoInternalDeviceControl(
    IN WDFQUEUE  Queue,
    IN WDFREQUEST  Request,
    IN size_t  OutputBufferLength,
    IN size_t  InputBufferLength,
    IN ULONG  IoControlCode
    )
{
    NTSTATUS              status = STATUS_SUCCESS;
    WDFDEVICE             device = WdfIoQueueGetDevice(Queue);
    PBUSDOG_CONTEXT       context = BusDogGetDeviceContext(device);
    BOOLEAN               bRead = FALSE;

    BusDogProcessInternalDeviceControl(device,
        context,
        Request,
        IoControlCode,
        FALSE,
        &bRead);

    if (bRead)
    {
        //
        // For reads, we want completion info. Call the helper
        //  routine to forward the request with a completion routine.
        //

        BusDogForwardRequestWithCompletion(WdfIoQueueGetDevice(Queue), 
                Request,
                BusDogIoInternalDeviceControlComplete,
                (WDFCONTEXT)IoControlCode);
    }
    else
    {        
        //
        // pass the request on to the filtered device
        //

        BusDogForwardRequest(WdfIoQueueGetDevice(Queue), 
                Request);
    }
}

VOID
BusDogIoInternalDeviceControlComplete(
    IN WDFREQUEST Request,
    IN WDFIOTARGET Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS Params,
    IN WDFCONTEXT Context
    ) 
{
    WDFDEVICE             device = WdfIoQueueGetDevice(WdfRequestGetIoQueue(Request));
    PBUSDOG_CONTEXT       context = BusDogGetDeviceContext(device);
    BOOLEAN               bRead = TRUE;
    
    BusDogProcessInternalDeviceControl(device,
        context,
        Request,
        (ULONG)Context,
        TRUE,
        &bRead);

    WdfRequestComplete(Request, Params->IoStatus.Status);
}

VOID
BusDogProcessInternalDeviceControl(
    IN WDFDEVICE Device,
    IN PBUSDOG_CONTEXT Context,
    IN WDFREQUEST  Request,
    IN ULONG  IoControlCode,
    IN BOOLEAN bCompletion,
    OUT BOOLEAN* bRead)
{
    if (BusDogFiltering && Context->FilterEnabled)
    {
        if (bCompletion)
        {
            BusDogPrint(BUSDOG_DEBUG_INFO, "BusDogIoInternalDeviceControlComplete - Id: %d, IOCTL: %d\n",  Context->DeviceId, IoControlCode);
        }
        else
        {
            BusDogPrint(BUSDOG_DEBUG_INFO, "BusDogIoInternalDeviceControl - Id: %d, IOCTL: %d\n",  Context->DeviceId, IoControlCode);
        }

        switch (IoControlCode)
        {
            case IOCTL_INTERNAL_USB_SUBMIT_URB:
            {
                PURB pUrb;
                PVOID pTransferBuffer = NULL;
                PMDL pTransferBufferMDL = NULL;
                ULONG TransferBufferLength = 0;
                BOOLEAN urbHandled = TRUE;

                BusDogPrint(BUSDOG_DEBUG_INFO, "    IOCTL_INTERNAL_USB_SUBMIT_URB\n");
                
                pUrb = (PURB) IoGetCurrentIrpStackLocation(WdfRequestWdmGetIrp(Request))->Parameters.Others.Argument1;

                //
                // Figure out what kinda urb we have and set up transfer vars
                //

                switch (pUrb->UrbHeader.Function)
                {
                    case URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER:
                    {
                        struct _URB_BULK_OR_INTERRUPT_TRANSFER* pTransfer = (struct _URB_BULK_OR_INTERRUPT_TRANSFER*)pUrb;
                        *bRead = (BOOLEAN)(pTransfer->TransferFlags & USBD_TRANSFER_DIRECTION_IN);

                        BusDogPrint(BUSDOG_DEBUG_INFO, "        URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER\n");

                        pTransferBuffer = pTransfer->TransferBuffer;
                        pTransferBufferMDL = pTransfer->TransferBufferMDL;
                        TransferBufferLength = pTransfer->TransferBufferLength;

                        break;
                    }
                    case URB_FUNCTION_CONTROL_TRANSFER:
                    {
                        struct _URB_CONTROL_TRANSFER* pTransfer =  (struct _URB_CONTROL_TRANSFER*)pUrb;
                        *bRead = (BOOLEAN)(pTransfer->TransferFlags & USBD_TRANSFER_DIRECTION_IN);

                        BusDogPrint(BUSDOG_DEBUG_INFO, "        URB_FUNCTION_CONTROL_TRANSFER\n");

                        pTransferBuffer = pTransfer->TransferBuffer;
                        pTransferBufferMDL = pTransfer->TransferBufferMDL;
                        TransferBufferLength = pTransfer->TransferBufferLength;

                        break;
                    }
                    case URB_FUNCTION_VENDOR_DEVICE:
                    case URB_FUNCTION_VENDOR_INTERFACE:
                    case URB_FUNCTION_VENDOR_ENDPOINT:
                    case URB_FUNCTION_VENDOR_OTHER:
                    case URB_FUNCTION_CLASS_DEVICE:
                    case URB_FUNCTION_CLASS_INTERFACE:
                    case URB_FUNCTION_CLASS_ENDPOINT:
                    case URB_FUNCTION_CLASS_OTHER:
                    {
                        struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST* pTransfer =  (struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST*)pUrb;
                        *bRead = (BOOLEAN)(pTransfer->TransferFlags & USBD_TRANSFER_DIRECTION_IN);

                        switch (pUrb->UrbHeader.Function)
                        {
                            case URB_FUNCTION_VENDOR_DEVICE:
                                BusDogPrint(BUSDOG_DEBUG_INFO, "        URB_FUNCTION_VENDOR_DEVICE\n");
                                break;
                            case URB_FUNCTION_VENDOR_INTERFACE:
                                BusDogPrint(BUSDOG_DEBUG_INFO, "        URB_FUNCTION_VENDOR_INTERFACE\n");
                                break;
                            case URB_FUNCTION_VENDOR_ENDPOINT:
                                BusDogPrint(BUSDOG_DEBUG_INFO, "        URB_FUNCTION_VENDOR_ENDPOINT\n");
                                break;
                            case URB_FUNCTION_VENDOR_OTHER:
                                BusDogPrint(BUSDOG_DEBUG_INFO, "        URB_FUNCTION_VENDOR_OTHER\n");
                                break;
                            case URB_FUNCTION_CLASS_DEVICE:
                                BusDogPrint(BUSDOG_DEBUG_INFO, "        URB_FUNCTION_CLASS_DEVICE\n");
                                break;
                            case URB_FUNCTION_CLASS_INTERFACE:
                                BusDogPrint(BUSDOG_DEBUG_INFO, "        URB_FUNCTION_CLASS_INTERFACE\n");
                                break;
                            case URB_FUNCTION_CLASS_ENDPOINT:
                                BusDogPrint(BUSDOG_DEBUG_INFO, "        URB_FUNCTION_CLASS_ENDPOINT\n");
                                break;
                            case URB_FUNCTION_CLASS_OTHER:
                                BusDogPrint(BUSDOG_DEBUG_INFO, "        URB_FUNCTION_CLASS_OTHER\n");
                                break;
                        }

                        pTransferBuffer = pTransfer->TransferBuffer;
                        pTransferBufferMDL = pTransfer->TransferBufferMDL;
                        TransferBufferLength = pTransfer->TransferBufferLength;

                        break;
                    }
                    default:
                        BusDogPrint(BUSDOG_DEBUG_INFO, "        URB_FUNCTION: %d\n", pUrb->UrbHeader.Function);
                        urbHandled = FALSE;
                        break;
                }

                //
                // If we have an urb we want (bulk, interupt and control) then lets process it
                //
                
                if (urbHandled)
                {
                    BusDogPrint(BUSDOG_DEBUG_INFO, "        TransferBufferLength: %d\n", TransferBufferLength);
                    BusDogPrint(BUSDOG_DEBUG_INFO, "        R/W: %s, MDL: %d\n", *bRead ? "read" : "write", pTransferBufferMDL != NULL);

                    if (bCompletion && *bRead || !bCompletion && !*bRead)
                    {
                        BusDogPrint(BUSDOG_DEBUG_INFO, "        Data: ");

                        if (pTransferBuffer != NULL)
                        {
                            PrintChars((PCHAR)pTransferBuffer, TransferBufferLength);

                            BusDogAddTraceToFifo(Device,
                                    Context->DeviceId, 
                                    *bRead ? BusDogReadRequest : BusDogWriteRequest, 
                                    pTransferBuffer, 
                                    TransferBufferLength);
                        }
                        else if (pTransferBufferMDL != NULL)
                        {
                            PCHAR pMDLBuf = (PCHAR)MmGetSystemAddressForMdlSafe(pTransferBufferMDL, NormalPagePriority);

                            PrintChars(pMDLBuf, TransferBufferLength);

                            BusDogAddTraceToFifo(Device,
                                    Context->DeviceId, 
                                    *bRead ? BusDogReadRequest : BusDogWriteRequest, 
                                    pMDLBuf, 
                                    TransferBufferLength);
                        }
                        else
                        {
                            BusDogPrint(BUSDOG_DEBUG_ERROR, "Buffer error!\n");
                        }
                    }
                }
                break;
            }
        }

    }

}

#endif
