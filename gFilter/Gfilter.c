/* 
 * Module: Generic Filter Driver
 * Description: This is a generic filter template. This will insert into any driver stack and sniff the packets
 * Author: Oliver Thane (othane@gmail.com), ...
 *
 * License: GPL ?
 */



#include "Gfilter.h"
#include "log.h"
#include <wdmsec.h> // for SDDLs
#include <Ntstrsafe.h>

#ifndef USE_OWN_GUID
#include <devguid.h>
#endif


#define xUSE_SEPERATE_QUEUE_FOR_READ
#define xUSE_CONTROL_DEVICE

////////////////////////////////////////////////////////////////////////////
// Macros Globals etc...
////////////////////////////////////////////////////////////////////////////

//
// Device Collection Stuff
//
typedef struct _FILTER_EXTENSION 
{
	ULONG 	Instance; 
} FILTER_EXTENSION, *PFILTER_EXTENSION;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FILTER_EXTENSION,
                                   FilterGetData);
typedef struct _SIDEBAND_DEVICE_DATA
{
	ULONG	Instance;
} SIDEBAND_DEVICE_DATA, *PSIDEBAND_DEVICE_DATA;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(SIDEBAND_DEVICE_DATA,
                                   SideBandGetData);


////////////////////////////////////////////////////////////////////////////
// Macros Globals etc...
////////////////////////////////////////////////////////////////////////////

//
// Error handling macros
//
#define CHECKSTATUS(s, r) {\
       						if (!NT_SUCCESS(s))\
							{\
								r;\
							}\
						} // Error handler macro for NTSTATUS results 

ULONG DeviceInstances = 0;	// Record how many device registered with this driver.
							//!todo Make static


////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////
NTSTATUS GFilterCreateControlDevice(	WDFDEVICE	Device,
										ULONG		Instance);
NTSTATUS GFilterCreateRawPDO(	WDFDEVICE	Device,
								ULONG		Instance);

// Not required I think
/*NTSTATUS GFilterRead(	IN 	WDFQUEUE		Queue,
						IN	WDFREQUEST		Request,
						IN	size_t			Length);*/ 





////////////////////////////////////////////////////////////////////////////
// Entry/Add Device/Cleanup etc..
////////////////////////////////////////////////////////////////////////////

// Driver Entry
NTSTATUS DriverEntry(	IN 	PDRIVER_OBJECT 	DriverObject,
						IN	PUNICODE_STRING	RegistryPath)
{
	// Init
	NTSTATUS 				status;	
	WDF_DRIVER_CONFIG 		config;
	WDF_OBJECT_ATTRIBUTES 	colAttributes;
	WDFDRIVER 				hDriver;


	DEBUG("Entry");

	// Register this driver and its add dev method with the WDF
	WDF_DRIVER_CONFIG_INIT(	&config, 	
							GFilterAddDevice);
	status = WdfDriverCreate(	DriverObject, 	
								RegistryPath, 
								WDF_NO_OBJECT_ATTRIBUTES,
								&config,
								&hDriver);
	CHECKSTATUS(status, ERROR("Unable to create driver object"));	
	return(status);
}

// Add device
NTSTATUS GFilterAddDevice( 	IN	WDFDRIVER			Driver,
							IN	PWDFDEVICE_INIT		DeviceInit)
{
	NTSTATUS				status;
	WDF_OBJECT_ATTRIBUTES 	deviceAttributes;
	WDFDEVICE 				device;
	PFILTER_EXTENSION 		filtExt;
	WDF_IO_QUEUE_CONFIG		ioQueueConfig;
	WDFQUEUE				ioDefaultQueue;

	DEBUG("Adding Device");

	// Indicate we are a filter driver, This forwards all IRPs on to the lower filters
	WdfFdoInitSetFilter(DeviceInit);

	// Setup the actual filter for the IRPs
#ifdef USE_SETIOINCALLERCONTEXTCALLBACK
	// Use built in KMDF filter
	WdfDeviceInitSetIoInCallerContextCallback(	DeviceInit, 
												GFilterIOCallback);
#endif

	// Create a new device
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(	&deviceAttributes, 
												FILTER_EXTENSION);
	// no cleanup callback needed -- deviceAttributes.EvtCleanupCallback = GFilterDeviceCleanUp; 	// Set device unload callback
	status = WdfDeviceCreate(	&DeviceInit, 				// Create the device
				 				&deviceAttributes, 
				 				&device);
	CHECKSTATUS(status, return(status));

	// Create the filter extension -- We don't really care about this really
	filtExt = FilterGetData(device);
	filtExt->Instance = 0x00;

#ifndef USE_SETIOINCALLERCONTEXTCALLBACK
	// Create IO queue and set callbacks to be filtered.
	//!todo, we created a parrallel que here but some drivers may need multiple parallel queues which is not done
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(	&ioQueueConfig,					// Create parallel IO queue in case the device we filter does not like sequential
											WdfIoQueueDispatchParallel );
	ioQueueConfig.EvtIoRead 	= GFilterRead;	// Read
	ioQueueConfig.EvtIoWrite 	= GFilterWrite;	// Write
	status = WdfIoQueueCreate(	device,
								&ioQueueConfig,
								WDF_NO_OBJECT_ATTRIBUTES,
								&ioDefaultQueue);
	CHECKSTATUS(status, ERROR("Failed to create queues. status=0x%0.4x",status); goto GFilterAddDeviceCleanup);		
#endif

	// Create sideband communications to userspace
#ifdef USE_CONTROL_DEVICE
	GFilterCreateControlDevice(	device,
								++DeviceInstances); //!todo, should we lock instance ??
#else
	GFilterCreateRawPDO(	device,
							++DeviceInstances); //!todo, should we lock instance ??
#endif
GFilterAddDeviceCleanup:
	return(status);
}




////////////////////////////////////////////////////////////////////////////
// Filtering of IRPs 
////////////////////////////////////////////////////////////////////////////
#ifdef USE_SETIOINCALLERCONTEXTCALLBACK
VOID GFilterIOCallback(	IN	WDFDEVICE	Device,
						IN	WDFREQUEST	Request)
{
	NTSTATUS 				status;
	WDF_REQUEST_PARAMETERS 	params;
	char 					*buf;
	size_t					bufLength;	

	// Filter the desired information
	WDF_REQUEST_PARAMETERS_INIT(&params);
	WdfRequestGetParameters(Request,
							&params);
	switch (params.Type)
	{
		//!todo - Record the IRP contents
		case WdfRequestTypeRead:
			DEBUG("Intercepted Read IRP");
			// We must register a callback for when the read completes (in case it is asynchronous) so the buffers will contain something from
			// the device (or what ever driver is below us).
			WdfRequestSetCompletionRoutine(	Request,
											GFilterReadCompleteionRoutine,
											WDF_NO_CONTEXT); // This should be the filter|device context so we can pass the results to the PDO
			break;
		case WdfRequestTypeWrite:
			DEBUG("Intercepted Write IRP");
			status = WdfRequestRetrieveInputBuffer(	Request,	//!todo This should work for BUFFERED and DIRECT, but I think it will break for Neither !!
													1,
													(PVOID)&buf,
													&bufLength);
			CHECKSTATUS(status, ERROR("Failed to retrieve intercepted write buffer. 0x%0.4", status); break;);
			DEBUG("Intercepted Write of %d bytes: %#0.2x ...", bufLength, (unsigned char)buf[0]);			
			//!todo place the buf into the RAW PDO context
			break;
		// All else can just be passed on...
	}

	// Try to pass this down the stack
	status = WdfDeviceEnqueueRequest(	Device, 
					 					Request);
	if (!NT_SUCCESS(status))		
	{
		// If we failed then complete it with an error
		ERROR("Error passing Intercepted IO request on, completing it ourself.");
		WdfRequestComplete(Request,
						   status);
	}
}
#else
// Filter read
VOID GFilterRead(	IN 	WDFQUEUE		Queue,
					IN	WDFREQUEST		Request,
					IN	size_t			Length)
{
	NTSTATUS					status = STATUS_ABANDONED;
	WDFDEVICE					device;
	WDF_REQUEST_SEND_OPTIONS 	options;
	WDF_REQUEST_PARAMETERS 		params;

	DEBUG("Read Intercepted");
	device = WdfIoQueueGetDevice(Queue); // Get a reference back to the device
	
	// Forward the request
	WdfRequestFormatRequestUsingCurrentType(Request);
//    WDF_REQUEST_SEND_OPTIONS_INIT(&options,
//                                  WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);	
	WdfRequestSetCompletionRoutine(	Request,
									GFilterReadCompleteionRoutine,
									NULL); // This should be the filter|device context so we can pass the results to the PDO		
	if (WdfRequestSend(	Request,
						WdfDeviceGetIoTarget(device),
						WDF_NO_SEND_OPTIONS) == FALSE)
	{
		status = WdfRequestGetStatus (Request);
        CHECKSTATUS(status, ERROR("Failed to forward read request. Completing myself. 0x%0.4x", status););
		goto GFilterReadFail;
	}
	return;

GFilterReadFail:
	WdfRequestComplete(	Request,
					   	status);
}

// Filter write
VOID GFilterWrite(	IN 	WDFQUEUE		Queue,
					IN	WDFREQUEST		Request,
					IN	size_t			Length)
{
	NTSTATUS					status = STATUS_ABANDONED;
	WDFDEVICE					device;
	char 						*buf;
	size_t						bufLength;	
	WDF_REQUEST_SEND_OPTIONS 	options;
	WDF_REQUEST_PARAMETERS 		params;
	
	// Init
	DEBUG("Write Intercepted");
	device = WdfIoQueueGetDevice(Queue); // Get a reference back to the device

	// Query the data
	status = WdfRequestRetrieveInputBuffer(	Request,
											1,
											(PVOID)&buf,
											&bufLength);
	CHECKSTATUS(status, ERROR("Failed to retrieve write buffer. 0x%0.4", status); goto GFilterWriteFail);
	DEBUG("Driver wrote %d bytes: %c ...", bufLength, buf[0]);	

	// Forward the request unmodified
	WdfRequestFormatRequestUsingCurrentType(Request);
    WDF_REQUEST_SEND_OPTIONS_INIT(&options,
                                  WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);	
	if (WdfRequestSend(	Request,
						WdfDeviceGetIoTarget(device),
						&options) == FALSE)
	{
		status = WdfRequestGetStatus (Request);
        CHECKSTATUS(status, ERROR("Failed to forward write request. Completing myself. 0x%0.4x", status););			
		goto GFilterWriteFail;
	}	
	return;

GFilterWriteFail:
	WdfRequestComplete(	Request,
						status);
}
#endif 

VOID GFilterReadCompleteionRoutine(	IN	WDFREQUEST							Request,
									IN	WDFIOTARGET							Target,
									IN	PWDF_REQUEST_COMPLETION_PARAMS		CompletionParams,
									IN	WDFCONTEXT							Context)
{
	NTSTATUS 		status;
	char 			*buf;
	size_t			bufLength;		

	DEBUG("Intercepted Read Completion");

	//DEBUG("Intercepted Read Completion Routine. %#0.4x", CompletionParams->IoStatus.Status);
	status = WdfRequestRetrieveOutputBuffer(	Request,	//!todo This should work for BUFFERED and DIRECT, but I think it will break for Neither !!
											1,
											(PVOID)&buf,
											&bufLength);
	CHECKSTATUS(status, ERROR("Failed to retrieve intercepted read buffer. 0x%0.4", status); goto GFilterReadCompleteionRoutineCleanup;);
	DEBUG("Intercepted Read of %d bytes: %#0.2x ...", bufLength, (unsigned char)buf[0]);
		
GFilterReadCompleteionRoutineCleanup:	
	//!todo Test CompletionParams->IoStatus.Status has not been set before completing this request
	//because this may have been done already if the read op was synchronous
	WdfRequestComplete(Request, CompletionParams->IoStatus.Status);
}


////////////////////////////////////////////////////////////////////////////
// Side Band Communications (new raw PDO & device interface that user space 
// 		can read/write ioctl to etc. to ...) 
////////////////////////////////////////////////////////////////////////////
#define MAX_ID_LEN 256

// Create control device
//!todo WdfObjectDelete on EvtCleanupCallback in device add routine...
NTSTATUS GFilterCreateControlDevice(	WDFDEVICE	Device,
										ULONG		Instance)
{
	NTSTATUS				status;
	PWDFDEVICE_INIT			pDeviceInit		= NULL;
	WDF_OBJECT_ATTRIBUTES 	controlAttributes;
	WDFDEVICE				controlDevice	= NULL;
	PSIDEBAND_DEVICE_DATA	sideBandData 	= NULL;
	WDF_IO_QUEUE_CONFIG		ioQueueConfig;
#ifdef USE_SEPERATE_QUEUE_FOR_READ
	WDFQUEUE				ioReadQueue, ioWriteQueue;
#else
	WDFQUEUE				ioQueue;
#endif

	DECLARE_UNICODE_STRING_SIZE(ntName, MAX_ID_LEN);
	DECLARE_UNICODE_STRING_SIZE(symbolicLinkName, MAX_ID_LEN);	


	DEBUG("Creating a new control device node");

	// 	
	// Create a control device
	//
	pDeviceInit = WdfControlDeviceInitAllocate(	WdfDeviceGetDriver(Device),
												&SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R);

	if (pDeviceInit == NULL)
	{
		ERROR("Unable to allocate resources to create a control device.");
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto GFilterCreateControlDeviceFail;
	}
	// Let many apps open this interface
	WdfDeviceInitSetExclusive(pDeviceInit, FALSE); 
	// Name this device
	status = RtlUnicodeStringPrintf(&ntName, DEVICE_PATH L"%02d", Instance);
	CHECKSTATUS(status, ERROR("Failed to set control device path. status=0x%0.4x", status); goto GFilterCreateControlDeviceFail);
	status = WdfDeviceInitAssignName(	pDeviceInit, 
									 	&ntName);

	CHECKSTATUS(status, ERROR("Failed to set control device path. status=0x%0.4x", status); goto GFilterCreateControlDeviceFail);
	// Create device
	DEBUG("Creating new control device node at %wZ", &ntName);
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&controlAttributes,
											SIDEBAND_DEVICE_DATA);
	status = WdfDeviceCreate(	&pDeviceInit,
								&controlAttributes,
								&controlDevice);
	CHECKSTATUS(status, ERROR("Failed to create control device. status=%#0.4x", status); goto GFilterCreateControlDeviceFail);
	// Set device symbolic name
	status = RtlUnicodeStringPrintf(&symbolicLinkName, SYMBOLIC_PATH L"%02d", Instance);
	CHECKSTATUS(status, ERROR("Failed to set control device symbolic link. status=0x%0.4x", status); goto GFilterCreateControlDeviceFail);
	status = WdfDeviceCreateSymbolicLink(	controlDevice,
                                			&symbolicLinkName);
	// Set device context
	sideBandData = SideBandGetData(controlDevice);
	sideBandData->Instance = Instance;	

	//
	// Configure and attach to default queue
	//
#ifdef USE_SEPERATE_QUEUE_FOR_READ
	// It maybe useful to have the reader on one sequential queue, and the write/ioctl on another so that the
	// infrequent write/ioctl will not interfere with the readers operation...
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(	&ioQueueConfig,					// Create a read sequential queue
											WdfIoQueueDispatchSequential);
	ioQueueConfig.EvtIoRead 	= GFilterSideBandRead;		// Read
	status = WdfIoQueueCreate(	controlDevice,
								&ioQueueConfig,
								WDF_NO_OBJECT_ATTRIBUTES,
								&ioReadQueue);
	CHECKSTATUS(status, ERROR("Failed to create read queue. status=0x%0.4x",status); goto GFilterCreateControlDeviceFail);
#if 0 // Do we need this ?
	status = WdfDeviceConfigureRequestDispatching(	Device,
													ioReadQueue,
													WdfRequestTypeRead);		
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);		
#endif

	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(	&ioQueueConfig,					// Create a write sequential queue
											WdfIoQueueDispatchSequential);		
	ioQueueConfig.EvtIoWrite 	= GFilterSideBandWrite;	// Write
	status = WdfIoQueueCreate(	controlDevice,
								&ioQueueConfig,
								WDF_NO_OBJECT_ATTRIBUTES,
								&ioWriteQueue);
	CHECKSTATUS(status, ERROR("Failed to create write queue. status=0x%0.4x",status); goto GFilterCreateControlDeviceFail);
#if 0 // Do we need this ?	
	status = WdfDeviceConfigureRequestDispatching(	Device,
													ioWriteQueue,
													WdfRequestTypeWrite);
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);		
#endif
#else
	// For getting stated this method works...
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(	&ioQueueConfig,					// Create a read sequential queue
											WdfIoQueueDispatchSequential);
	ioQueueConfig.EvtIoRead 	= GFilterSideBandRead;	// Read
	ioQueueConfig.EvtIoWrite 	= GFilterSideBandWrite;	// Write
	status = WdfIoQueueCreate(	controlDevice,
								&ioQueueConfig,
								WDF_NO_OBJECT_ATTRIBUTES,
								&ioQueue);
	CHECKSTATUS(status, ERROR("Failed to create queues. status=0x%0.4x",status); goto GFilterCreateControlDeviceFail);		
#endif			

	WdfControlFinishInitializing(controlDevice);

	DEBUG("Control Device Node Created");

	return (STATUS_SUCCESS);

GFilterCreateControlDeviceFail:


	if (!pDeviceInit)
			WdfDeviceInitFree(pDeviceInit);
	if (controlDevice !=NULL)
	{
		WdfObjectDelete(controlDevice);
		controlDevice = NULL;
	}

	return(status);
}

// Create RAW PDO and attach child device for IO
NTSTATUS GFilterCreateRawPDO(	WDFDEVICE	Device,
								ULONG		Instance)
{
	// Note this could have been done by making a control device but Microsoft 
	// advise against this see http://msdn.microsoft.com/en-us/library/aa490288.aspx
	// "...Note that a better way for the filter driver to avoid the problem is to 
	//  act as a bus driver and  enumerate child devices that operate in  raw mode..."
	

	NTSTATUS						status;
	PWDFDEVICE_INIT					pDeviceInit 	= NULL;
	WDF_OBJECT_ATTRIBUTES 			pdoAttributes;
	WDFDEVICE 						hChild 			= NULL;
	PSIDEBAND_DEVICE_DATA			sideBandData 	= NULL;
	WDF_IO_QUEUE_CONFIG				ioQueueConfig;
#ifdef USE_SEPERATE_QUEUE_FOR_READ
	WDFQUEUE						ioReadQueue, ioWriteQueue;
#else
	WDFQUEUE						ioQueue;
#endif
	WDF_DEVICE_PNP_CAPABILITIES 	pnpCaps;
	DECLARE_CONST_UNICODE_STRING(deviceId, GFILTER_DEVICE_ID);
	DECLARE_CONST_UNICODE_STRING(deviceLocation,L"GFilter\0" );
	DECLARE_UNICODE_STRING_SIZE(buffer, MAX_ID_LEN);


	// 
	// Create a new raw PDO (bus driver)
	//
	DEBUG("Creating a new device node");
	pDeviceInit = WdfPdoInitAllocate(Device);
	if (pDeviceInit == NULL)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto GFilterCreateRawPDOCleanUp;			
	}
#ifdef USE_OWN_GUID
	status = WdfPdoInitAssignRawDevice(pDeviceInit, &GUID_DEVCLASS_GFILTER); //!todo We really need our own class type but I dont know how to do this correctly
#else
	status = WdfPdoInitAssignRawDevice(pDeviceInit, &GUID_DEVCLASS_UNKNOWN); // NOTE: The GUID specifies details to look for in registry 
#endif
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);		
	status = WdfPdoInitAssignDeviceID(	pDeviceInit, 
										&deviceId);
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);
	// Set the instance
	status = RtlUnicodeStringPrintf(&buffer, L"%02d", Instance);
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);
	status = WdfPdoInitAssignInstanceID(pDeviceInit, 
										&buffer);
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);
	// Set the device text to show in device manager
	status = RtlUnicodeStringPrintf(&buffer, L"GFilter_%02d", Instance);
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);
	status = WdfPdoInitAddDeviceText(	pDeviceInit, 
										&buffer,
										&deviceLocation,
										0x409);
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);
	WdfPdoInitSetDefaultLocale(pDeviceInit, 0x409);
	// Set the IO type used for this device (This is not needed for WdfDeviceIoBuffered but it lets us change it later...)
	WdfDeviceInitSetIoType(	pDeviceInit,
							WdfDeviceIoBuffered);		
	// Create the PDO, this provides us a handle for the next section
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&pdoAttributes, 
											SIDEBAND_DEVICE_DATA);
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);
	status = WdfDeviceCreate(	&pDeviceInit, 
								&pdoAttributes, 
								&hChild);
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);


	//
	// Create device node  
	//

	// Set device context
	sideBandData = SideBandGetData(hChild);
	sideBandData->Instance = Instance;
	// Configure Read/Write/ioctl using sequential queue (for now)

#ifdef USE_SEPERATE_QUEUE_FOR_READ
	// It maybe useful to have the reader on one sequential queue, and the write/ioctl on another so that the
	// infrequent write/ioctl will not interfere with the readers operation...
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(	&ioQueueConfig,					// Create a read sequential queue
											WdfIoQueueDispatchSequential);
	ioQueueConfig.EvtIoRead 	= GFilterSideBandRead;		// Read
	status = WdfIoQueueCreate(	hChild,
								&ioQueueConfig,
								WDF_NO_OBJECT_ATTRIBUTES,
								&ioReadQueue);
	CHECKSTATUS(status, ERROR("Failed to create read queue. status=0x%0.4x",status); goto GFilterCreateRawPDOCleanUp);
#if 0 // Do we need this ?
	status = WdfDeviceConfigureRequestDispatching(	Device,
													ioReadQueue,
													WdfRequestTypeRead);		
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);		
#endif

	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(	&ioQueueConfig,					// Create a write sequential queue
											WdfIoQueueDispatchSequential);		
	ioQueueConfig.EvtIoWrite 	= GFilterSideBandWrite;	// Write
	status = WdfIoQueueCreate(	hChild,
								&ioQueueConfig,
								WDF_NO_OBJECT_ATTRIBUTES,
								&ioWriteQueue);
	CHECKSTATUS(status, ERROR("Failed to create write queue. status=0x%0.4x",status); goto GFilterCreateRawPDOCleanUp);
#if 0 // Do we need this ?	
	status = WdfDeviceConfigureRequestDispatching(	Device,
													ioWriteQueue,
													WdfRequestTypeWrite);
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);		
#endif
#else
	// For getting stated this method works...
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(	&ioQueueConfig,					// Create a read sequential queue
											WdfIoQueueDispatchSequential);
	ioQueueConfig.EvtIoRead 	= GFilterSideBandRead;	// Read
	ioQueueConfig.EvtIoWrite 	= GFilterSideBandWrite;	// Write
	status = WdfIoQueueCreate(	hChild,
								&ioQueueConfig,
								WDF_NO_OBJECT_ATTRIBUTES,
								&ioQueue);
	CHECKSTATUS(status, ERROR("Failed to create queues. status=0x%0.4x",status); goto GFilterCreateRawPDOCleanUp);		
#endif		
	// Configure deviceCaps
	WDF_DEVICE_PNP_CAPABILITIES_INIT(&pnpCaps);
	pnpCaps.Removable			= WdfTrue;
	pnpCaps.SurpriseRemovalOK 	= WdfTrue;
	pnpCaps.Address 			= Instance;
	pnpCaps.UINumber			= Instance;		
		//pnpCaps.NoDisplayInUI		= WdfTrue; // Device manager should not display this device
		//pnpCaps.SilentInstall		= WdfTrue; // Stop the device manager from pestering the user 
	WdfDeviceSetPnpCapabilities(hChild, &pnpCaps);		
	// Create an interface to talk to the user mode apps
	status = WdfDeviceCreateDeviceInterface(	hChild,
												&GUID_DEVICEINTERFACE_GFILTER,
												NULL);
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);
	status = WdfFdoAddStaticChild(	Device, 
									hChild);
	CHECKSTATUS(status, goto GFilterCreateRawPDOCleanUp);

	// Done
	DEBUG("Device Node Created");
	return STATUS_SUCCESS;
	
GFilterCreateRawPDOCleanUp:	
	// Error state, we did not complete the process so clean up and return in error.
	ERROR("Unable to create device node");
	if (pDeviceInit != NULL)
		WdfDeviceInitFree(pDeviceInit);
	if (hChild)
		WdfObjectDelete(hChild);

	return(status);	
}

// Sideband Devices Read Callback
VOID GFilterSideBandRead(	IN 	WDFQUEUE		Queue,
							IN	WDFREQUEST		Request,
							IN	size_t			Length)
{
	NTSTATUS		status;
	char			*buf;
	size_t			bufLength=0;
	size_t			bytesRead=0;
	DECLARE_UNICODE_STRING_SIZE(str, 9);

	DEBUG("Read Requested");

	status = WdfRequestRetrieveOutputBuffer(	Request,
												8,
												(PVOID)&buf,
												&bufLength);
	CHECKSTATUS(status, ERROR("Failed to retrieve read buffer. 0x%0.4", status); goto GFilterSideBandReadCleaUp);

	DEBUG("Writing to read buffer");
	status =  RtlUnicodeStringPrintf(&str, L"Device%02d", 1);
	CHECKSTATUS(status, ERROR("Failed to print unicode string. 0x%0.4", status); goto GFilterSideBandReadCleaUp);
	//memcpy(buf, &str, sizeof(str));
	buf[0] = 0xEE;
	bytesRead = 1;

GFilterSideBandReadCleaUp:
	WdfRequestCompleteWithInformation(	Request,
					   					status,
										bytesRead);
}

// SideBand Devices Write Callback
VOID GFilterSideBandWrite(	IN 	WDFQUEUE		Queue,
							IN	WDFREQUEST		Request,
							IN	size_t			Length)
{
	NTSTATUS	status;
	char 		*buf;
	size_t		bufLength;
	
	DEBUG("Write Requested");

	status = WdfRequestRetrieveInputBuffer(	Request,
											1,
											(PVOID)&buf,
											&bufLength);
	CHECKSTATUS(status, ERROR("Failed to retrieve write buffer. 0x%0.4", status); goto GFilterSideBandWriteCleanUp);

	DEBUG("Driver wrote %d bytes: %c ...", bufLength, buf[0]);

GFilterSideBandWriteCleanUp:
	WdfRequestCompleteWithInformation(	Request,
										status,
										Length); // Bytes written..
}


