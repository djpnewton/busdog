#include "BusDogCommon.h"

//
// Collection object is used to store all the FilterDevice objects so
// that any event callback routine can easily walk thru the list and pick a
// specific instance of the device for filtering.
//
WDFCOLLECTION   BusDogDeviceCollection;
WDFWAITLOCK     BusDogDeviceCollectionLock;


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, BusDogDeviceAdd)
#pragma alloc_text (PAGE, BusDogDeviceContextCleanup)

#endif

//
// Define IOCTL_INTERFACE in your sources file if  you want your
// app to have private interaction with the filter driver. Read KB Q262305
// for more information.
//

#ifdef IOCTL_INTERFACE

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

    KdPrint(("BusDog Filter Driver - Driver Framework Edition.\n"));
    KdPrint(("Built %s %s\n", __DATE__, __TIME__));

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

    //
    // Create a framework driver object to represent our driver.
    //
    status = WdfDriverCreate(DriverObject,
                            RegistryPath,
                            WDF_NO_OBJECT_ATTRIBUTES,
                            &config,
                            &hDriver);
    if (!NT_SUCCESS(status)) {
        KdPrint( ("WdfDriverCreate failed with status 0x%x\n", status));
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
        KdPrint( ("WdfCollectionCreate failed with status 0x%x\n", status));
        return status;
    }

    WDF_OBJECT_ATTRIBUTES_INIT(&colAttributes);
    colAttributes.ParentObject = hDriver;

    status = WdfWaitLockCreate(&colAttributes,
                                &BusDogDeviceCollectionLock);
    if (!NT_SUCCESS(status))
    {
        KdPrint( ("WdfWaitLockCreate failed with status 0x%x\n", status));
        return status;
    }

    return status;
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
        KdPrint(("Failed to get the property of PDO: 0x%p\n", DeviceInit));

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
        KdPrint( ("WdfDeviceInitAssignWdmIrpPreprocessCallback failed with status 0x%x\n",
                                status));

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
        KdPrint( ("WdfDeviceInitAssignWdmIrpPreprocessCallback failed with status 0x%x\n",
                                status));

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
        KdPrint( ("WdfDeviceCreate failed with status code 0x%x\n", status));
        return status;
    }

    context = GetBusDogContext(device);
    context->SerialNo = serialNo;

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
        KdPrint( ("WdfCollectionAdd failed with status code 0x%x\n", status));
    }
    WdfWaitLockRelease(BusDogDeviceCollectionLock);

    //
    // Create a control device if IOCTL_INTERFACE is defined in the sources file.
    //
#ifdef IOCTL_INTERFACE

    status = BusDogCreateControlDevice(device);
    if (!NT_SUCCESS(status)) {
        KdPrint( ("BusDogCreateControlDevice failed with status 0x%x\n",
                                status));
        //
        // Let us not fail AddDevice just because we weren't able to create the
        // control device.
        //
        status = STATUS_SUCCESS;
    }

#endif

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

    KdPrint(("Entered BusDogDeviceContextCleanup\n"));

    WdfWaitLockAcquire(BusDogDeviceCollectionLock, NULL);

    count = WdfCollectionGetCount(BusDogDeviceCollection);

#ifdef IOCTL_INTERFACE
        if(count == 1)
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

#endif

    WdfCollectionRemove(BusDogDeviceCollection, Device);

    WdfWaitLockRelease(BusDogDeviceCollectionLock);
}

#ifdef IOCTL_INTERFACE

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

    KdPrint(("Creating Control Device\n"));

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

    KdPrint(("Deleting Control Device\n"));

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
    ULONG               i;
    ULONG               noItems;
    WDFDEVICE           hFilterDevice;
    PBUSDOG_CONTEXT     context;

    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(IoControlCode);

    PAGED_CODE();

    KdPrint(("Ioctl recieved into filter control object.\n"));

    WdfWaitLockAcquire(BusDogDeviceCollectionLock, NULL);

    noItems = WdfCollectionGetCount(BusDogDeviceCollection);

    for(i=0; i<noItems ; i++) {

        hFilterDevice = WdfCollectionGetItem(BusDogDeviceCollection, i);

        context = GetBusDogContext(hFilterDevice);

        KdPrint(("Serial No: %d\n", context->SerialNo));
    }

    WdfWaitLockRelease(BusDogDeviceCollectionLock);


    WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, 0);
}


#endif

#ifdef WDM_PREPROCESS

NTSTATUS
BusDogWdmDeviceReadWrite (
    IN WDFDEVICE Device,
    IN PIRP Irp
    )
{
    PBUSDOG_CONTEXT         context = GetBusDogContext(Device);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);



    if (stack->MajorFunction == IRP_MJ_READ)
    {
        KdPrint(("BusDog - IRP_MJ_READ, Length: %d\n", stack->Parameters.Read.Length));
    }
    else if (stack->MajorFunction == IRP_MJ_WRITE)
    {
        KdPrint(("BusDog - IRP_MJ_WRITE, Length: %d\n", stack->Parameters.Write.Length));
    }

    if (FlagOn(stack->DeviceObject->Flags, DO_BUFFERED_IO)) 
    {
        //buffer at Irp->AssociatedIrp.SystemBuffer
        KdPrint(("         DO_BUFFERED_IO - Irp->AssociatedIrp.SystemBuffer: %d\n", Irp->AssociatedIrp.SystemBuffer));
    }
    else if (FlagOn(stack->DeviceObject->Flags, DO_DIRECT_IO)) 
    {
        PVOID buffer;

        // buffer at Irp->MdlAddress
        KdPrint(("         DO_DIRECT_IO - Irp->MdlAddress: %d\n", Irp->MdlAddress));

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
            KdPrint(("         Data: "));
            PrintChars(buffer, MmGetMdlByteCount(Irp->MdlAddress));
        }

    }
    else 
    {
        // buffer at Irp->UserBuffer 
        KdPrint(("         Neither - Irp->UserBuffer: %d\n", Irp->UserBuffer));
    }

    IoSkipCurrentIrpStackLocation(Irp); 
    return WdfDeviceWdmDispatchPreprocessedIrp(Device, Irp);
}

#endif
