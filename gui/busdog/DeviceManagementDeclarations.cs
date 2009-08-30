using System;
using System.Collections;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Runtime.InteropServices; 
using System.Windows.Forms;

namespace busdog
{
	internal sealed partial class DeviceManagement
	{
		///<summary >
		// API declarations relating to device management (SetupDixxx and 
		// RegisterDeviceNotification functions).   
		/// </summary>

		// from dbt.h

		internal const Int32 DBT_DEVICEARRIVAL = 0X8000;
		internal const Int32 DBT_DEVICEREMOVECOMPLETE = 0X8004;
		internal const Int32 DBT_DEVTYP_DEVICEINTERFACE = 5;
		internal const Int32 DBT_DEVTYP_HANDLE = 6;
		internal const Int32 DEVICE_NOTIFY_ALL_INTERFACE_CLASSES = 4;
		internal const Int32 DEVICE_NOTIFY_SERVICE_HANDLE = 1;
		internal const Int32 DEVICE_NOTIFY_WINDOW_HANDLE = 0;
		internal const Int32 WM_DEVICECHANGE = 0X219;

		// from setupapi.h

		internal const Int32 DIGCF_PRESENT = 2;
        internal const Int32 DIGCF_ALLCLASSES = 0x4;
		internal const Int32 DIGCF_DEVICEINTERFACE = 0X10;

        /// <summary>
        /// Device registry property codes
        /// </summary>
        internal enum SPDRP : int
        {
            /// <summary>
            /// DeviceDesc (R/W)
            /// </summary>
            SPDRP_DEVICEDESC = 0x00000000,

            /// <summary>
            /// HardwareID (R/W)
            /// </summary>
            SPDRP_HARDWAREID = 0x00000001,

            /// <summary>
            /// CompatibleIDs (R/W)
            /// </summary>
            SPDRP_COMPATIBLEIDS = 0x00000002,

            /// <summary>
            /// unused
            /// </summary>
            SPDRP_UNUSED0 = 0x00000003,

            /// <summary>
            /// Service (R/W)
            /// </summary>
            SPDRP_SERVICE = 0x00000004,

            /// <summary>
            /// unused
            /// </summary>
            SPDRP_UNUSED1 = 0x00000005,

            /// <summary>
            /// unused
            /// </summary>
            SPDRP_UNUSED2 = 0x00000006,

            /// <summary>
            /// Class (R--tied to ClassGUID)
            /// </summary>
            SPDRP_CLASS = 0x00000007,

            /// <summary>
            /// ClassGUID (R/W)
            /// </summary>
            SPDRP_CLASSGUID = 0x00000008,

            /// <summary>
            /// Driver (R/W)
            /// </summary>
            SPDRP_DRIVER = 0x00000009,

            /// <summary>
            /// ConfigFlags (R/W)
            /// </summary>
            SPDRP_CONFIGFLAGS = 0x0000000A,

            /// <summary>
            /// Mfg (R/W)
            /// </summary>
            SPDRP_MFG = 0x0000000B,

            /// <summary>
            /// FriendlyName (R/W)
            /// </summary>
            SPDRP_FRIENDLYNAME = 0x0000000C,

            /// <summary>
            /// LocationInformation (R/W)
            /// </summary>
            SPDRP_LOCATION_INFORMATION = 0x0000000D,

            /// <summary>
            /// PhysicalDeviceObjectName (R)
            /// </summary>
            SPDRP_PHYSICAL_DEVICE_OBJECT_NAME = 0x0000000E,

            /// <summary>
            /// Capabilities (R)
            /// </summary>
            SPDRP_CAPABILITIES = 0x0000000F,

            /// <summary>
            /// UiNumber (R)
            /// </summary>
            SPDRP_UI_NUMBER = 0x00000010,

            /// <summary>
            /// UpperFilters (R/W)
            /// </summary>
            SPDRP_UPPERFILTERS = 0x00000011,

            /// <summary>
            /// LowerFilters (R/W)
            /// </summary>
            SPDRP_LOWERFILTERS = 0x00000012,

            /// <summary>
            /// BusTypeGUID (R)
            /// </summary>
            SPDRP_BUSTYPEGUID = 0x00000013,

            /// <summary>
            /// LegacyBusType (R)
            /// </summary>
            SPDRP_LEGACYBUSTYPE = 0x00000014,

            /// <summary>
            /// BusNumber (R)
            /// </summary>
            SPDRP_BUSNUMBER = 0x00000015,

            /// <summary>
            /// Enumerator Name (R)
            /// </summary>
            SPDRP_ENUMERATOR_NAME = 0x00000016,

            /// <summary>
            /// Security (R/W, binary form)
            /// </summary>
            SPDRP_SECURITY = 0x00000017,

            /// <summary>
            /// Security (W, SDS form)
            /// </summary>
            SPDRP_SECURITY_SDS = 0x00000018,

            /// <summary>
            /// Device Type (R/W)
            /// </summary>
            SPDRP_DEVTYPE = 0x00000019,

            /// <summary>
            /// Device is exclusive-access (R/W)
            /// </summary>
            SPDRP_EXCLUSIVE = 0x0000001A,

            /// <summary>
            /// Device Characteristics (R/W)
            /// </summary>
            SPDRP_CHARACTERISTICS = 0x0000001B,

            /// <summary>
            /// Device Address (R)
            /// </summary>
            SPDRP_ADDRESS = 0x0000001C,

            /// <summary>
            /// UiNumberDescFormat (R/W)
            /// </summary>
            SPDRP_UI_NUMBER_DESC_FORMAT = 0X0000001D,

            /// <summary>
            /// Device Power Data (R)
            /// </summary>
            SPDRP_DEVICE_POWER_DATA = 0x0000001E,

            /// <summary>
            /// Removal Policy (R)
            /// </summary>
            SPDRP_REMOVAL_POLICY = 0x0000001F,

            /// <summary>
            /// Hardware Removal Policy (R)
            /// </summary>
            SPDRP_REMOVAL_POLICY_HW_DEFAULT = 0x00000020,

            /// <summary>
            /// Removal Policy Override (RW)
            /// </summary>
            SPDRP_REMOVAL_POLICY_OVERRIDE = 0x00000021,

            /// <summary>
            /// Device Install State (R)
            /// </summary>
            SPDRP_INSTALL_STATE = 0x00000022,

            /// <summary>
            /// Device Location Paths (R)
            /// </summary>
            SPDRP_LOCATION_PATHS = 0x00000023,
        }

		// Two declarations for the DEV_BROADCAST_DEVICEINTERFACE structure.

		// Use this one in the call to RegisterDeviceNotification() and
		// in checking dbch_devicetype in a DEV_BROADCAST_HDR structure:

		[StructLayout(LayoutKind.Sequential)]
		internal class DEV_BROADCAST_DEVICEINTERFACE
		{
			internal Int32 dbcc_size;
			internal Int32 dbcc_devicetype;
			internal Int32 dbcc_reserved;
			internal Guid dbcc_classguid;
			internal Int16 dbcc_name;
		}

		// Use this to read the dbcc_name String and classguid:

		[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
		internal class DEV_BROADCAST_DEVICEINTERFACE_1
		{
			internal Int32 dbcc_size;
			internal Int32 dbcc_devicetype;
			internal Int32 dbcc_reserved;
			[MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 16)]
			internal Byte[] dbcc_classguid;
			[MarshalAs(UnmanagedType.ByValArray, SizeConst = 255)]
			internal Char[] dbcc_name;
		}

		[StructLayout(LayoutKind.Sequential)]
		internal class DEV_BROADCAST_HDR
		{
			internal Int32 dbch_size;
			internal Int32 dbch_devicetype;
			internal Int32 dbch_reserved;
		}

#pragma warning disable 0649
		internal struct SP_DEVICE_INTERFACE_DATA
		{
			internal Int32 cbSize;
			internal System.Guid InterfaceClassGuid;
			internal Int32 Flags;
			internal IntPtr Reserved;
		}

		internal struct SP_DEVICE_INTERFACE_DETAIL_DATA
		{
			internal Int32 cbSize;
			internal String DevicePath;
        }
#pragma warning restore 0649

        internal struct SP_DEVINFO_DATA
		{
			internal Int32 cbSize;
			internal System.Guid ClassGuid;
			internal Int32 DevInst;
			internal Int32 Reserved;
		}

		[DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
		internal static extern IntPtr RegisterDeviceNotification(IntPtr hRecipient, IntPtr NotificationFilter, Int32 Flags);

		[DllImport("setupapi.dll", SetLastError = true)]
		internal static extern Int32 SetupDiCreateDeviceInfoList(ref System.Guid ClassGuid, Int32 hwndParent);

		[DllImport("setupapi.dll", SetLastError = true)]
		internal static extern Int32 SetupDiDestroyDeviceInfoList(IntPtr DeviceInfoSet);

		[DllImport("setupapi.dll", SetLastError = true)]
		internal static extern Boolean SetupDiEnumDeviceInterfaces(IntPtr DeviceInfoSet, IntPtr DeviceInfoData, ref System.Guid InterfaceClassGuid, Int32 MemberIndex, ref SP_DEVICE_INTERFACE_DATA DeviceInterfaceData);

		[DllImport("setupapi.dll", SetLastError = true, CharSet = CharSet.Auto)]
		internal static extern IntPtr SetupDiGetClassDevs(IntPtr ClassGuid, IntPtr Enumerator, IntPtr hwndParent, Int32 Flags);

		[DllImport("setupapi.dll", SetLastError = true, CharSet = CharSet.Auto)]
		internal static extern Boolean SetupDiGetDeviceInterfaceDetail(IntPtr DeviceInfoSet, ref SP_DEVICE_INTERFACE_DATA DeviceInterfaceData, IntPtr DeviceInterfaceDetailData, Int32 DeviceInterfaceDetailDataSize, ref Int32 RequiredSize, IntPtr DeviceInfoData);

		[DllImport("setupapi.dll", SetLastError = true, CharSet = CharSet.Auto)]
        internal static extern bool SetupDiEnumDeviceInfo(IntPtr DeviceInfoSet, uint MemberIndex, ref SP_DEVINFO_DATA DeviceInfoData);

        [DllImport("setupapi.dll", SetLastError = true, CharSet = CharSet.Auto)]
        internal static extern bool SetupDiGetDeviceInstanceId(IntPtr DeviceInfoSet, ref SP_DEVINFO_DATA DeviceInfoData, IntPtr DeviceInstanceId, uint DeviceInstanceIdSize, out uint RequiredSize);

        [DllImport("setupapi.dll", SetLastError = true, CharSet = CharSet.Auto)]
        internal static extern bool SetupDiGetDeviceRegistryProperty(IntPtr DeviceInfoSet, ref SP_DEVINFO_DATA DeviceInfoData, uint Property, out UInt32 PropertyRegDataType, IntPtr PropertyBuffer, uint PropertyBufferSize, out UInt32 RequiredSize);

		[DllImport("user32.dll", SetLastError = true)]
		internal static extern Boolean UnregisterDeviceNotification(IntPtr Handle);

        [DllImport("setupapi.dll", SetLastError = true)]
        static extern int CM_Locate_DevNode(out IntPtr pdnDevInst, string pDeviceID, uint ulFlags);

        [DllImport("setupapi.dll", SetLastError = true)]
        static extern int CM_Get_Child(out IntPtr pdnDevInst, int dnDevInst, int ulFlags);

        [DllImport("setupapi.dll", SetLastError = true)]
        static extern int CM_Get_Sibling(out IntPtr pdnDevInst, int DevInst, int ulFlags);
	}
}
