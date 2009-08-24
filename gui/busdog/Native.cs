using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;
using System.IO;

namespace busdog
{
    public struct DeviceId
    {
        public uint DevId;
        public bool Enabled;
        public string PhysicalDeviceObjectName;
        public string HardwareId;
        public string Description;
        public string InstanceId;
        public DeviceId(uint devId, bool enabled, string pdoName)
        {
            DevId = devId;
            Enabled = enabled;
            PhysicalDeviceObjectName = pdoName;
            HardwareId = null;
            Description = null;
            InstanceId = null;
        }

        public override string ToString()
        {
            if (Description != null)
                return string.Format("{0:D2}: {1}", DevId, Description);
            if (HardwareId != null)
                return string.Format("{0:D2}: {1}", DevId, HardwareId);
            return string.Format("{0:D2}: {1}", DevId, PhysicalDeviceObjectName);
        }
    }

    public enum BUSDOG_REQUEST_TYPE
    {
        BusDogReadRequest = 0x2000,
        BusDogWriteRequest,
        BusDogDeviceControlRequest,
        BusDogInternalDeviceControlRequest,
        BusDogPnPRequest,
        BusDogMaxRequestType
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct BUSDOG_TIMESTAMP
    {
        public int Seconds;
        public int USec;

        public override string ToString()
        {
            return string.Format("{0}.{1:D6}", Seconds, USec);
        }
    }
        
    public struct FilterTrace
    {
        public uint  DeviceId;
        public BUSDOG_REQUEST_TYPE Type;
        public BUSDOG_TIMESTAMP Timestamp;
        public byte[] Buffer;
        public FilterTrace(uint devId, BUSDOG_REQUEST_TYPE type, BUSDOG_TIMESTAMP timestamp, byte[] buffer)
        {
            DeviceId = devId;
            Type = type;
            Timestamp = timestamp;
            Buffer = buffer;
        }

        public string TypeToStr()
        {
            switch (Type)
            {
                case BUSDOG_REQUEST_TYPE.BusDogReadRequest:
                    return "R";
                case BUSDOG_REQUEST_TYPE.BusDogWriteRequest:
                    return "W";
                default:
                    return "?";
            }
        }

        public string BufToChars()
        {
            StringBuilder sb = new StringBuilder(Buffer.Length);
            sb.Length = Buffer.Length;
            for (int i = 0; i < Buffer.Length; i++)
            {
                byte b = Buffer[i];
                if (b > 31 && b < 128)
                    sb[i] = (char)b;
                else
                    sb[i] = '.';
            }
            return sb.ToString();
        }

        public string BufToHex()
        {
            StringBuilder sb = new StringBuilder(Buffer.Length * 3 - 1);
            sb.Length = Buffer.Length * 3 - 1;
            for (int i = 0; i < Buffer.Length; i++)
            {
                string hex = String.Format("{0:x2}", Buffer[i]);
                sb[i * 3] = hex[0];
                sb[i * 3 + 1] = hex[1];
                if (i < Buffer.Length - 1)
                    sb[i * 3 + 2] = ' ';
            }
            return sb.ToString();
        }

        public override string ToString()
        {
            return string.Format("{0:D2}: {1}: {2}: {3}", DeviceId, TypeToStr(), Timestamp.ToString(), BufToChars());
        }

        public BUSDOG_TIMESTAMP GetTimestampDelta(FilterTrace prevTrace)
        {
            BUSDOG_TIMESTAMP delta = new BUSDOG_TIMESTAMP();
            if (prevTrace.Timestamp.Seconds > 0 || prevTrace.Timestamp.USec > 0)
            {
                delta.Seconds = Timestamp.Seconds - prevTrace.Timestamp.Seconds;
                delta.USec = Timestamp.USec - prevTrace.Timestamp.USec;
                if (delta.USec < 0)
                    delta.USec = 1000000 + delta.USec;
            }
            return delta;
        }
    }

    public class Native
    {
        static uint CTL_CODE(uint deviceType, uint function, uint method, uint access)
        {
            return ((deviceType) << 16) | ((access) << 14) | ((function) << 2) | (method);
        }

        const uint FILE_DEVICE_BUSDOG = 0x0F59;

        const uint METHOD_BUFFERED = 0;
        const uint METHOD_IN_DIRECT = 1;
        const uint METHOD_OUT_DIRECT = 2;
        const uint METHOD_NEITHER = 3;
        const uint FILE_ANY_ACCESS = 0;
        const uint FILE_READ_ACCESS = 0x0001;
        const uint FILE_WRITE_ACCESS = 0x0002;

        uint IOCTL_BUSDOG_GET_BUFFER = CTL_CODE(FILE_DEVICE_BUSDOG, 
                                          2049, 
                                          METHOD_OUT_DIRECT, 
                                          FILE_READ_ACCESS);
        uint IOCTL_BUSDOG_START_FILTERING = CTL_CODE(FILE_DEVICE_BUSDOG,
                                          2050,              
                                          METHOD_BUFFERED,   
                                          FILE_READ_ACCESS);
        uint IOCTL_BUSDOG_STOP_FILTERING = CTL_CODE(FILE_DEVICE_BUSDOG,
                                         2051,               
                                         METHOD_BUFFERED,    
                                         FILE_READ_ACCESS);
        uint IOCTL_BUSDOG_SET_DEVICE_FILTER_ENABLED = CTL_CODE(FILE_DEVICE_BUSDOG,
                                         2053,               
                                         METHOD_BUFFERED,    
                                         FILE_WRITE_ACCESS);
        uint IOCTL_BUSDOG_GET_DEVICE_LIST = CTL_CODE(FILE_DEVICE_BUSDOG,
                                         2054,               
                                         METHOD_BUFFERED,    
                                         FILE_READ_ACCESS);

        [StructLayout(LayoutKind.Sequential)]
        struct BUSDOG_DEVICE_ID
        {
            public uint DeviceId;
            public byte Enabled;
            public uint PhysicalDeviceObjectNameSize;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct BUSDOG_FILTER_ENABLED
        {
            public uint DeviceId;
            public byte FilterEnabled;
            public BUSDOG_FILTER_ENABLED(uint devId, bool enabled)
            {
                DeviceId = devId;
                FilterEnabled = Convert.ToByte(enabled);
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        struct BUSDOG_FILTER_TRACE
        {
            public uint  DeviceId;
            public BUSDOG_REQUEST_TYPE Type;
            public BUSDOG_TIMESTAMP Timestamp;
            public uint  BufferSize;
        }

        [DllImport("Kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        static extern IntPtr CreateFile(
            string fileName,
            [MarshalAs(UnmanagedType.U4)] FileAccess fileAccess,
            [MarshalAs(UnmanagedType.U4)] FileShare fileShare,
            int securityAttributes,
            [MarshalAs(UnmanagedType.U4)] FileMode creationDisposition,
            int flags,
            IntPtr overlapped);

        [DllImport("Kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern bool DeviceIoControl(
            SafeFileHandle hDevice,
            [MarshalAs(UnmanagedType.U4)]uint dwIoControlCode, 
            IntPtr lpInBuffer, 
            [MarshalAs(UnmanagedType.U4)]uint nInBufferSize, 
            IntPtr lpOutBuffer, 
            [MarshalAs(UnmanagedType.U4)]uint nOutBufferSize, 
            [MarshalAs(UnmanagedType.U4)]out uint lpBytesReturned, 
            IntPtr lpOverlapped
        );

        SafeFileHandle hDevice = null;

        void Init()
        {
            if (hDevice == null)
            {
                hDevice = new SafeFileHandle(
                    CreateFile(
                        "\\\\.\\BusDogFilter",
                        FileAccess.ReadWrite,
                        FileShare.ReadWrite,
                        0,
                        FileMode.Open,
                        0,
                        IntPtr.Zero),
                    true);
            }
        }

        public bool DeviceIoControl(
            uint code,
            IntPtr lpInBuffer, 
            uint nInBufferSize, 
            IntPtr lpOutBuffer, 
            uint nOutBufferSize, 
            out uint lpBytesReturned)
        {
            Init();
            return DeviceIoControl(
                hDevice,
                code,
                lpInBuffer,
                nInBufferSize,
                lpOutBuffer,
                nOutBufferSize,
                out lpBytesReturned,
                IntPtr.Zero);
        }

        public bool GetDeviceList(out List<DeviceId> deviceIds)
        {
            deviceIds = new List<DeviceId>();
            uint outBufSize = 1024;
            IntPtr outBuf = Marshal.AllocHGlobal((int)outBufSize);
            uint bytesReturned;
            bool result =
                DeviceIoControl(IOCTL_BUSDOG_GET_DEVICE_LIST,
                    IntPtr.Zero,
                    0,
                    outBuf,
                    outBufSize,
                    out bytesReturned);
            if (result)
            {
                int index = 0;
                while (bytesReturned >= index + Marshal.SizeOf(typeof(BUSDOG_DEVICE_ID)))
                {
                    BUSDOG_DEVICE_ID devId =
                        (BUSDOG_DEVICE_ID)
                        Marshal.PtrToStructure(new IntPtr(outBuf.ToInt64() + index),
                            typeof(BUSDOG_DEVICE_ID));
                    index += Marshal.SizeOf(typeof(BUSDOG_DEVICE_ID));
                    string hardwareId =
                        Marshal.PtrToStringUni(new IntPtr(outBuf.ToInt32() + index),
                            (int)devId.PhysicalDeviceObjectNameSize / 2);
                    index += (int)devId.PhysicalDeviceObjectNameSize;
                    deviceIds.Add(new DeviceId(devId.DeviceId, Convert.ToBoolean(devId.Enabled), hardwareId));
                }        
            }
            else
            {
                System.Diagnostics.Debug.WriteLine(Marshal.GetLastWin32Error());
            }
            Marshal.FreeHGlobal(outBuf);
            return result;
        }

        public bool SetDeviceEnabled(uint deviceId, bool enabled)
        {
            GCHandle h = GCHandle.Alloc(new BUSDOG_FILTER_ENABLED(deviceId, enabled), GCHandleType.Pinned);
            IntPtr p = h.AddrOfPinnedObject();
            uint bytesReturned;
            bool result = 
                DeviceIoControl(IOCTL_BUSDOG_SET_DEVICE_FILTER_ENABLED,
                    p,
                    (uint)Marshal.SizeOf(typeof(BUSDOG_FILTER_ENABLED)),
                    IntPtr.Zero,
                    0,
                    out bytesReturned);
            if (!result)
            {
                System.Diagnostics.Debug.WriteLine(Marshal.GetLastWin32Error());
            }
            h.Free();
            return result;
        }

        public bool GetTraceList(out List<FilterTrace> filterTraces)
        {
            filterTraces = new List<FilterTrace>();
            uint outBufSize = 1024;
            IntPtr outBuf = Marshal.AllocHGlobal((int)outBufSize);
            uint bytesReturned;
            bool result =
                DeviceIoControl(IOCTL_BUSDOG_GET_BUFFER,
                    IntPtr.Zero,
                    0,
                    outBuf,
                    outBufSize,
                    out bytesReturned);
            if (result)
            {
                int index = 0;
                while (bytesReturned >= index + Marshal.SizeOf(typeof(BUSDOG_FILTER_TRACE)))
                {
                    BUSDOG_FILTER_TRACE filterTrace =
                        (BUSDOG_FILTER_TRACE)
                        Marshal.PtrToStructure(new IntPtr(outBuf.ToInt64() + index),
                            typeof(BUSDOG_FILTER_TRACE));
                    index += Marshal.SizeOf(typeof(BUSDOG_FILTER_TRACE));
                    byte[] trace = new byte[filterTrace.BufferSize];
                    Marshal.Copy(new IntPtr(outBuf.ToInt32() + index), trace, 0, (int)filterTrace.BufferSize);
                    index += (int)filterTrace.BufferSize;
                    filterTraces.Add(new FilterTrace(filterTrace.DeviceId, filterTrace.Type, filterTrace.Timestamp, trace));
                }        
            }
            else
            {
                System.Diagnostics.Debug.WriteLine(Marshal.GetLastWin32Error());
            }
            Marshal.FreeHGlobal(outBuf);
            return result;
        }

        public bool StartTracing()
        {
            uint bytesReturned;
            return DeviceIoControl(IOCTL_BUSDOG_START_FILTERING,
                    IntPtr.Zero,
                    0,
                    IntPtr.Zero,
                    0,
                    out bytesReturned);
        }

        public bool StopTracing()
        {
            uint bytesReturned;
            return DeviceIoControl(IOCTL_BUSDOG_STOP_FILTERING,
                    IntPtr.Zero,
                    0,
                    IntPtr.Zero,
                    0,
                    out bytesReturned);
        }
    }
}
