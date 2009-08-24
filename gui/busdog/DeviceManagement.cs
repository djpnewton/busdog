///  <summary>
///  Routines for detecting devices and receiving device notifications.
///  Sourced from http://www.lvr.com/hidpage.htm (http://www.lvr.com/files/generic_hid_cs_451.zip).. thanks Jan Axelson!
///  </summary>
  
using Microsoft.VisualBasic;
using System;
using System.Collections;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Runtime.InteropServices; 
using System.Windows.Forms;

namespace busdog
{
	sealed internal partial class DeviceManagement
	{
        internal Boolean IsDeviceChild(string parentInstanceId, string childInstanceId)
        {
            IntPtr ptrParentDevNode, ptrChildDevNode;
            if (CM_Locate_DevNode(out ptrParentDevNode, parentInstanceId, 0) == 0)
            {
                if (CM_Locate_DevNode(out ptrChildDevNode, childInstanceId, 0) == 0)
                {
                    IntPtr ptrChildCompare;
                    if (CM_Get_Child(out ptrChildCompare, ptrParentDevNode.ToInt32(), 0) == 0)
                    {
                        while (ptrChildCompare.ToInt32() != ptrChildDevNode.ToInt32())
                        {
                            if (!(CM_Get_Sibling(out ptrChildCompare, ptrChildCompare.ToInt32(), 0) == 0))
                                break;
                        }
                        return ptrChildCompare.ToInt32() == ptrChildDevNode.ToInt32();
                    }
                }
            }
            return false;
        }

        Boolean GetRegProp(IntPtr deviceInfoSet, ref SP_DEVINFO_DATA devInfo, uint property, out string propertyVal)
        {
            uint RequiredSize = 0;
            uint RegType;
            propertyVal = null;
            SetupDiGetDeviceRegistryProperty(deviceInfoSet, ref devInfo, property, out RegType, IntPtr.Zero, 0, out RequiredSize);
            if (RequiredSize > 0)
            {
                IntPtr ptrBuf = Marshal.AllocHGlobal((int)RequiredSize);
                if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, ref devInfo, property, out RegType, ptrBuf, RequiredSize, out RequiredSize))
                    propertyVal = Marshal.PtrToStringAuto(ptrBuf);
                Marshal.FreeHGlobal(ptrBuf);
            }
            return propertyVal != null;
        }

        Boolean GetInstanceId(IntPtr deviceInfoSet, ref SP_DEVINFO_DATA devInfo, out string instanceId)
        {
            uint RequiredSize = 256;
            instanceId = null;
            IntPtr ptrBuf = Marshal.AllocHGlobal((int)RequiredSize);
            if (SetupDiGetDeviceInstanceId(deviceInfoSet, ref devInfo, ptrBuf, RequiredSize, out RequiredSize))
                instanceId = Marshal.PtrToStringAuto(ptrBuf);
            else if (RequiredSize > 0)
            {
                Marshal.ReAllocHGlobal(ptrBuf, new IntPtr(RequiredSize));
                if (SetupDiGetDeviceInstanceId(deviceInfoSet, ref devInfo, ptrBuf, RequiredSize, out RequiredSize))
                    instanceId = Marshal.PtrToStringAuto(ptrBuf);
            }
            Marshal.FreeHGlobal(ptrBuf);
            return instanceId != null;
        }

        internal Boolean FindDeviceProps(string physicalDeviceObjectName, out string hardwareId, out string description, out string instanceId)
		{
			IntPtr detailDataBuffer = IntPtr.Zero;
			Boolean deviceFound;
			IntPtr deviceInfoSet = new System.IntPtr();
			Boolean lastDevice = false;
			UInt32 memberIndex = 0;
			Boolean success;

            hardwareId = null;
            description = null;
            instanceId = null;

			try
			{
                deviceInfoSet = SetupDiGetClassDevs(IntPtr.Zero, IntPtr.Zero, IntPtr.Zero, DIGCF_PRESENT | DIGCF_ALLCLASSES);

				deviceFound = false;
				memberIndex = 0;

                // The cbSize element of the devInfo structure must be set to
				// the structure's size in bytes. 

                SP_DEVINFO_DATA devInfo = new SP_DEVINFO_DATA();
                devInfo.cbSize = Marshal.SizeOf(devInfo);

                do
                {
                    // Begin with 0 and increment through the device information set until
                    // no more devices are available.

                    success = SetupDiEnumDeviceInfo
                        (deviceInfoSet,
                        memberIndex,
                        ref devInfo);

                    // Find out if a device information set was retrieved.

                    if (!success)
                    {
                        lastDevice = true;

                    }
                    else
                    {
                        // A device is present.

                        // now we can get some more detailed information
                        string pdoName;
                        if (GetRegProp(deviceInfoSet, ref devInfo, (uint)SPDRP.SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, out pdoName))
                        {
                            if (physicalDeviceObjectName == pdoName)
                            {
                                GetRegProp(deviceInfoSet, ref devInfo, (uint)SPDRP.SPDRP_HARDWAREID, out hardwareId);
                                GetRegProp(deviceInfoSet, ref devInfo, (uint)SPDRP.SPDRP_DEVICEDESC, out description);
                                GetInstanceId(deviceInfoSet, ref devInfo, out instanceId);
                                deviceFound = true;
                                break;
                            }
                        }
                    }
                    memberIndex++;
                }
                while (!((lastDevice == true)));				

				return deviceFound;
			}
			catch (Exception ex)
			{
				throw;
			}
				finally
			{
				if (detailDataBuffer != IntPtr.Zero) 
				{
					// Free the memory allocated previously by AllocHGlobal.

					Marshal.FreeHGlobal(detailDataBuffer);
				}
				// ***
				//  API function

				//  summary
				//  Frees the memory reserved for the DeviceInfoSet returned by SetupDiGetClassDevs.

				//  parameters
				//  DeviceInfoSet returned by SetupDiGetClassDevs.

				//  returns
				//  True on success.
				// ***

				if (deviceInfoSet != IntPtr.Zero)
				{
					SetupDiDestroyDeviceInfoList(deviceInfoSet);
				}
			}
		}	

		///  <summary>
		///  Requests to receive a notification when a device is attached or removed.
		///  </summary>
		///  
		///  <param name="formHandle"> handle to the window that will receive device events. </param>
		///  <param name="deviceNotificationHandle"> returned device notification handle. </param>
		///  
		///  <returns>
		///  True on success.
		///  </returns>
		///  
		internal Boolean RegisterForDeviceNotifications(IntPtr formHandle, ref IntPtr deviceNotificationHandle)
		{
			// A DEV_BROADCAST_DEVICEINTERFACE header holds information about the request.

			DEV_BROADCAST_DEVICEINTERFACE devBroadcastDeviceInterface = new DEV_BROADCAST_DEVICEINTERFACE();
			IntPtr devBroadcastDeviceInterfaceBuffer = IntPtr.Zero;
			Int32 size = 0;

			try
			{
				// Set the parameters in the DEV_BROADCAST_DEVICEINTERFACE structure.

				// Set the size.

				size = Marshal.SizeOf(devBroadcastDeviceInterface);
				devBroadcastDeviceInterface.dbcc_size = size;

				// Request to receive notifications about a class of devices.

				devBroadcastDeviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

				devBroadcastDeviceInterface.dbcc_reserved = 0;

				// Specify the interface class to receive notifications about.

				//devBroadcastDeviceInterface.dbcc_classguid = classGuid;
                devBroadcastDeviceInterface.dbcc_classguid = Guid.Empty;

				// Allocate memory for the buffer that holds the DEV_BROADCAST_DEVICEINTERFACE structure.

				devBroadcastDeviceInterfaceBuffer = Marshal.AllocHGlobal(size);

				// Copy the DEV_BROADCAST_DEVICEINTERFACE structure to the buffer.
				// Set fDeleteOld True to prevent memory leaks.

				Marshal.StructureToPtr(devBroadcastDeviceInterface, devBroadcastDeviceInterfaceBuffer, true);

				// ***
				//  API function

				//  summary
				//  Request to receive notification messages when a device in an interface class
				//  is attached or removed.

				//  parameters 
				//  Handle to the window that will receive device events.
				//  Pointer to a DEV_BROADCAST_DEVICEINTERFACE to specify the type of 
				//  device to send notifications for.
				//  DEVICE_NOTIFY_WINDOW_HANDLE indicates the handle is a window handle.

				//  Returns
				//  Device notification handle or NULL on failure.
				// ***

				deviceNotificationHandle = RegisterDeviceNotification(formHandle, devBroadcastDeviceInterfaceBuffer, DEVICE_NOTIFY_WINDOW_HANDLE);

				// Marshal data from the unmanaged block devBroadcastDeviceInterfaceBuffer to
				// the managed object devBroadcastDeviceInterface

				Marshal.PtrToStructure(devBroadcastDeviceInterfaceBuffer, devBroadcastDeviceInterface);
				
				if ((deviceNotificationHandle.ToInt32() == IntPtr.Zero.ToInt32()))
				{
					return false;
				}
				else
				{
					return true;
				}
			}
			catch (Exception ex)
			{
				throw;
			}
			finally
			{
				if (devBroadcastDeviceInterfaceBuffer != IntPtr.Zero)
				{
					// Free the memory allocated previously by AllocHGlobal.

					Marshal.FreeHGlobal(devBroadcastDeviceInterfaceBuffer);
				}
			}
		}

		///  <summary>
		///  Requests to stop receiving notification messages when a device in an
		///  interface class is attached or removed.
		///  </summary>
		///  
		///  <param name="deviceNotificationHandle"> handle returned previously by
		///  RegisterDeviceNotification. </param>

		internal void StopReceivingDeviceNotifications(IntPtr deviceNotificationHandle)
		{
			try
			{
				// ***
				//  API function

				//  summary
				//  Stop receiving notification messages.

				//  parameters
				//  Handle returned previously by RegisterDeviceNotification.  

				//  returns
				//  True on success.
				// ***

				//  Ignore failures.

				DeviceManagement.UnregisterDeviceNotification(deviceNotificationHandle);
			}
			catch (Exception ex)
			{
				throw;
			}
		}
	}
}





