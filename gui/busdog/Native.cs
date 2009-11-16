using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;
using System.IO;
using System.Threading;

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

    public enum BUSDOG_REQUEST_INTERNAL_DEVICE_CONTROL_TYPE
    {
        BusDogUSBSubmitURB = 0x2000,
        BusDogUSBResetPort
    };

    public enum BUSDOG_REQUEST_USB_DIRECTION
    {
        BusDogUsbIn = 0x0,
        BusDogUsbOut
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct BUSDOG_REQUEST_PARAMS
    {
        public uint p1;
        public uint p2;
        public uint p3;
        public uint p4;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
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
        #region URB request codes
        const int URB_FUNCTION_SELECT_CONFIGURATION           = 0x0000;
        const int URB_FUNCTION_SELECT_INTERFACE               = 0x0001;
        const int URB_FUNCTION_ABORT_PIPE                     = 0x0002;
        const int URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL      = 0x0003;
        const int URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL   = 0x0004;
        const int URB_FUNCTION_GET_FRAME_LENGTH               = 0x0005;
        const int URB_FUNCTION_SET_FRAME_LENGTH               = 0x0006;
        const int URB_FUNCTION_GET_CURRENT_FRAME_NUMBER       = 0x0007;
        const int URB_FUNCTION_CONTROL_TRANSFER               = 0x0008;
        const int URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER     = 0x0009;
        const int URB_FUNCTION_ISOCH_TRANSFER                 = 0x000A;
        const int URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE     = 0x000B;
        const int URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE       = 0x000C;
        const int URB_FUNCTION_SET_FEATURE_TO_DEVICE          = 0x000D;
        const int URB_FUNCTION_SET_FEATURE_TO_INTERFACE       = 0x000E;
        const int URB_FUNCTION_SET_FEATURE_TO_ENDPOINT        = 0x000F;
        const int URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE        = 0x0010;
        const int URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE     = 0x0011;
        const int URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT      = 0x0012;
        const int URB_FUNCTION_GET_STATUS_FROM_DEVICE         = 0x0013;
        const int URB_FUNCTION_GET_STATUS_FROM_INTERFACE      = 0x0014;
        const int URB_FUNCTION_GET_STATUS_FROM_ENDPOINT       = 0x0015;
        const int URB_FUNCTION_RESERVED_0X0016                = 0x0016;
        const int URB_FUNCTION_VENDOR_DEVICE                  = 0x0017;
        const int URB_FUNCTION_VENDOR_INTERFACE               = 0x0018;
        const int URB_FUNCTION_VENDOR_ENDPOINT                = 0x0019;
        const int URB_FUNCTION_CLASS_DEVICE                   = 0x001A;
        const int URB_FUNCTION_CLASS_INTERFACE                = 0x001B;
        const int URB_FUNCTION_CLASS_ENDPOINT                 = 0x001C;
        const int URB_FUNCTION_RESERVE_0X001D                 = 0x001D;
        // previously URB_FUNCTION_RESET_PIPE
        const int URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL= 0x001E;
        const int URB_FUNCTION_CLASS_OTHER                    = 0x001F;
        const int URB_FUNCTION_VENDOR_OTHER                   = 0x0020;
        const int URB_FUNCTION_GET_STATUS_FROM_OTHER          = 0x0021;
        const int URB_FUNCTION_CLEAR_FEATURE_TO_OTHER         = 0x0022;
        const int URB_FUNCTION_SET_FEATURE_TO_OTHER           = 0x0023;
        const int URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT   = 0x0024;
        const int URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT     = 0x0025;
        const int URB_FUNCTION_GET_CONFIGURATION              = 0x0026;
        const int URB_FUNCTION_GET_INTERFACE                  = 0x0027;
        const int URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE  = 0x0028;
        const int URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE    = 0x0029;
        const int URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR      = 0x002A;
        const int URB_FUNCTION_SYNC_RESET_PIPE                = 0x0030;
        const int URB_FUNCTION_SYNC_CLEAR_STALL               = 0x0031;
        #endregion

        public uint  DeviceId;
        public BUSDOG_REQUEST_TYPE Type;
        public BUSDOG_REQUEST_PARAMS Params;
        public BUSDOG_TIMESTAMP Timestamp;
        public byte[] Buffer;
        public FilterTrace(uint devId, BUSDOG_REQUEST_TYPE type, BUSDOG_REQUEST_PARAMS params_, BUSDOG_TIMESTAMP timestamp, byte[] buffer)
        {
            DeviceId = devId;
            Type = type;
            Params = params_;
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
                case BUSDOG_REQUEST_TYPE.BusDogInternalDeviceControlRequest:
                    switch ((BUSDOG_REQUEST_INTERNAL_DEVICE_CONTROL_TYPE)Params.p1)
                    {
                        case BUSDOG_REQUEST_INTERNAL_DEVICE_CONTROL_TYPE.BusDogUSBSubmitURB:
                            switch (Params.p3)
                            {
                                case URB_FUNCTION_ABORT_PIPE:
                                    return "URB_FUNCTION_ABORT_PIPE";
                                case URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL:
                                    return "URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL";
                                case URB_FUNCTION_SYNC_RESET_PIPE:
                                    return "URB_FUNCTION_SYNC_RESET_PIPE";
                                case URB_FUNCTION_SYNC_CLEAR_STALL:
                                    return "URB_FUNCTION_SYNC_CLEAR_STALL";
                                case URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE:
                                    return "URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE";
                                case URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT:
                                    return "URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT";
                                case URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE:
                                    return "URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE";
                                case URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE:
                                    return "URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE";
                                case URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT:
                                    return "URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT";
                                case URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE:
                                    return "URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE";
                                default:
                                    // urb is a bulk/interrupt or control transfer (or isochronous)
                                    return string.Format("{0}  (USB URB Function: {1})",
                                        (BUSDOG_REQUEST_USB_DIRECTION)Params.p2 == BUSDOG_REQUEST_USB_DIRECTION.BusDogUsbIn ? "In" : "Out",
                                        Params.p3);
                            }
                        case BUSDOG_REQUEST_INTERNAL_DEVICE_CONTROL_TYPE.BusDogUSBResetPort:
                            return "Reset Port";
                    }
                    goto default;
                default:
                    return string.Format("? (Type: {0}, p1, {1}, p2: {2}, p3: {3}, p4: {4})", Type, Params.p1, Params.p2, Params.p3, Params.p4);
            }
        }

        public string BufToChars()
        {
            if (Buffer != null)
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
            else
                return "";
        }

        public string BufToHex()
        {
            if (Buffer != null)
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
            else
                return "";
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

    public class FilterTraceArrivedEventArgs : EventArgs
    {
        public List<FilterTrace> Traces;

        public FilterTraceArrivedEventArgs(List<FilterTrace> traces)
        {
            Traces = traces;
        }
    }

    internal class DeviceIoOverlapped
    {
        private IntPtr mPtrOverlapped = IntPtr.Zero;

        private int mFieldOffset_InternalLow = 0;
        private int mFieldOffset_InternalHigh = 0;
        private int mFieldOffset_OffsetLow = 0;
        private int mFieldOffset_OffsetHigh = 0;
        private int mFieldOffset_EventHandle = 0;

        public DeviceIoOverlapped()
        {
            // Globally allocate the memory for the overlapped structure
            mPtrOverlapped = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(NativeOverlapped)));

            // Find the structural starting positions in the NativeOverlapped structure.
            mFieldOffset_InternalLow = Marshal.OffsetOf(typeof(NativeOverlapped), "InternalLow").ToInt32();
            mFieldOffset_InternalHigh = Marshal.OffsetOf(typeof(NativeOverlapped), "InternalHigh").ToInt32();
            mFieldOffset_OffsetLow = Marshal.OffsetOf(typeof(NativeOverlapped), "OffsetLow").ToInt32();
            mFieldOffset_OffsetHigh = Marshal.OffsetOf(typeof(NativeOverlapped), "OffsetHigh").ToInt32();
            mFieldOffset_EventHandle = Marshal.OffsetOf(typeof(NativeOverlapped), "EventHandle").ToInt32();
        }

        public IntPtr InternalLow
        {
            get { return Marshal.ReadIntPtr(mPtrOverlapped, mFieldOffset_InternalLow); }
            set { Marshal.WriteIntPtr(mPtrOverlapped, mFieldOffset_InternalLow, value); }
        }

        public IntPtr InternalHigh
        {
            get { return Marshal.ReadIntPtr(mPtrOverlapped, mFieldOffset_InternalHigh); }
            set { Marshal.WriteIntPtr(mPtrOverlapped, mFieldOffset_InternalHigh, value); }
        }

        public int OffsetLow
        {
            get { return Marshal.ReadInt32(mPtrOverlapped, mFieldOffset_OffsetLow); }
            set { Marshal.WriteInt32(mPtrOverlapped, mFieldOffset_OffsetLow, value); }
        }

        public int OffsetHigh
        {
            get { return Marshal.ReadInt32(mPtrOverlapped, mFieldOffset_OffsetHigh); }
            set { Marshal.WriteInt32(mPtrOverlapped, mFieldOffset_OffsetHigh, value); }
        }

        /// <summary>
        /// The overlapped event wait hande.
        /// </summary>
        public IntPtr EventHandle
        {
            get { return Marshal.ReadIntPtr(mPtrOverlapped, mFieldOffset_EventHandle); }
            set { Marshal.WriteIntPtr(mPtrOverlapped, mFieldOffset_EventHandle, value); }
        }

        /// <summary>
        /// Pass this into the DeviceIoControl and GetOverlappedResult APIs
        /// </summary>
        public IntPtr GlobalOverlapped
        {
            get { return mPtrOverlapped; }
        }

        /// <summary>
        /// Set the overlapped wait handle and clear out the rest of the structure.
        /// </summary>
        /// <param name="hEventOverlapped"></param>
        public void ClearAndSetEvent(IntPtr hEventOverlapped)
        {
            EventHandle = hEventOverlapped;
            InternalLow = IntPtr.Zero;
            InternalHigh = IntPtr.Zero;
            OffsetLow = 0;
            OffsetHigh = 0;
        }

        // Clean up the globally allocated memory.
        ~DeviceIoOverlapped()
        {
            if (mPtrOverlapped != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(mPtrOverlapped);
                mPtrOverlapped = IntPtr.Zero;
            }
        }
    }

    internal class DeviceIOCTL
    {
        [DllImport("Kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        static extern IntPtr CreateFile(
            string fileName,
            [MarshalAs(UnmanagedType.U4)] FileAccess fileAccess,
            [MarshalAs(UnmanagedType.U4)] FileShare fileShare,
            int securityAttributes,
            [MarshalAs(UnmanagedType.U4)] FileMode creationDisposition,
            [MarshalAs(UnmanagedType.U4)] FileOptions flags,
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

        [DllImport("Kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        static extern bool CancelIo(
            SafeFileHandle hFile);

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        static extern UInt32 WaitForSingleObject(IntPtr hHandle, UInt32 dwMilliseconds);

        /// <summary>
        /// The wait object signaled
        /// </summary>
        public const int WAIT_OBJECT_0 = 0;

        SafeFileHandle hDevice = null;
        DeviceIoOverlapped overlapped = null;
        ManualResetEvent hEvent = null;

        bool async;

        public DeviceIOCTL(string devicePath, bool async)
        {
            this.async = async;
            FileOptions fileOps = FileOptions.None;
            if (async)
            {
                overlapped = new DeviceIoOverlapped();
                hEvent = new ManualResetEvent(false);
                fileOps = FileOptions.Asynchronous;
            }
            hDevice = new SafeFileHandle(
                CreateFile(
                    devicePath,
                    FileAccess.ReadWrite,
                    FileShare.ReadWrite,
                    0,
                    FileMode.Open,
                    fileOps,
                    IntPtr.Zero),
                true);
        }

        public bool DeviceIoControl(
            uint code,
            IntPtr lpInBuffer,
            uint nInBufferSize,
            IntPtr lpOutBuffer,
            uint nOutBufferSize,
            out uint lpBytesReturned)
        {
            IntPtr ol = IntPtr.Zero;
            if (async)
            {
                overlapped.ClearAndSetEvent(hEvent.SafeWaitHandle.DangerousGetHandle());
                ol = overlapped.GlobalOverlapped;
            }
            return DeviceIoControl(
                hDevice,
                code,
                lpInBuffer,
                nInBufferSize,
                lpOutBuffer,
                nOutBufferSize,
                out lpBytesReturned,
                ol);
        }

        public bool WaitForOverlappedIo(UInt32 dwMilliseconds, out uint bytesRead)
        {
            bytesRead = 0;
            uint res = WaitForSingleObject(hEvent.SafeWaitHandle.DangerousGetHandle(), dwMilliseconds);
            if (res == WAIT_OBJECT_0)
            {
                bytesRead = (uint)overlapped.InternalHigh.ToInt32();
                return true;
            }
            return false;
        }

        public bool CancelIo()
        {
            return CancelIo(hDevice);
        }
    }

    public class Native
    {
        uint outBufferSize = 0x10000;
        IntPtr outBuffer;
        uint inBufferSize = 0x10000;
        IntPtr inBuffer;

        DeviceIOCTL devIO;
        Thread TraceBufferThread;

        public event EventHandler<FilterTraceArrivedEventArgs> FilterTraceArrived;

        const string busdogPath = "\\\\.\\BusDogFilter";
 
        public Native()
        {
            outBuffer = Marshal.AllocHGlobal((int)outBufferSize);
            inBuffer = Marshal.AllocHGlobal((int)inBufferSize);

            devIO = new DeviceIOCTL(busdogPath, false);

            TraceBufferThread = new Thread(TraceBufRead);
        }

        ~Native()
        {
            Marshal.FreeHGlobal(outBuffer);
            Marshal.FreeHGlobal(inBuffer);
        }

        public void StartTraceReader()
        {
            try // try/catch here because if guard is falling through even if expression is false???
            {
                if (TraceBufferThread.ThreadState != ThreadState.Running)
                    TraceBufferThread.Start();
            }
            catch
            {}
        }

        public void StopTraceReader()
        {
            if (TraceBufferThread.ThreadState != ThreadState.Stopped &&
                TraceBufferThread.ThreadState != ThreadState.Unstarted)
            {
                TraceBufferThread.Abort();
                TraceBufferThread.Join();
                // recreate thread (cannot restart stopped thread later on)
                TraceBufferThread = new Thread(TraceBufRead);
            }
        }

        /// <summary>
        /// Overlapped I/O operation is in progress.
        /// </summary>
        public const int ERROR_IO_PENDING = 997;

        void TraceBufRead()
        {
            try
            {
                DeviceIOCTL devIO = new DeviceIOCTL(busdogPath, true);

                while (true)
                {
                    uint bytesReturned;
                    // send the get trace buffer command to the driver
                    bool result =
                     devIO.DeviceIoControl(
                         IOCTL_BUSDOG_GET_BUFFER,
                         IntPtr.Zero,
                         0,
                         outBuffer,
                         outBufferSize,
                         out bytesReturned);
                    if (!result)
                    {
                        int err = Marshal.GetLastWin32Error();
                        // check if the I/O request is pending
                        if (err == ERROR_IO_PENDING)
                        {
                            while (true)
                            {
                                // keep checking if the I/O request has been fufilled (abort after 500ms so our thread can be killed)
                                if (devIO.WaitForOverlappedIo(500, out bytesReturned))
                                {
                                    result = true;
                                    break;
                                }
                            }
                        }
                        else
                            System.Diagnostics.Debug.WriteLine(err);
                    }
                    if (result)
                    {
                        // we have a result so now we convert the buffer into a trace list and call the event
                        if (FilterTraceArrived != null)
                            FilterTraceArrived(
                                this,
                                new FilterTraceArrivedEventArgs(GetTraceList(outBuffer, outBufferSize, bytesReturned)));
                    }
                }
            }
            catch (ThreadAbortException)
            {
                // We will cancel any pending IO on this thread before exiting
                devIO.CancelIo();
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
        }

        #region IOCTL definitions
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

        uint IOCTL_BUSDOG_GET_AUTOTRACE = CTL_CODE(FILE_DEVICE_BUSDOG,
                                         2057,               
                                         METHOD_BUFFERED,    
                                         FILE_READ_ACCESS);   

        uint IOCTL_BUSDOG_SET_AUTOTRACE = CTL_CODE(FILE_DEVICE_BUSDOG,
                                         2058,               
                                         METHOD_BUFFERED,    
                                         FILE_WRITE_ACCESS);
        #endregion

        #region busdog public structures
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        struct BUSDOG_DEVICE_ID
        {
            public uint DeviceId;
            public byte Enabled;
            /* type is size_t in c */ 
            public IntPtr PhysicalDeviceObjectNameSize;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
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

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        struct BUSDOG_FILTER_TRACE
        {
            public uint  DeviceId;
            public BUSDOG_REQUEST_TYPE Type;
            public BUSDOG_REQUEST_PARAMS Params;
            public BUSDOG_TIMESTAMP Timestamp;
            /* type is size_t in c */
            public IntPtr BufferSize;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        struct BUSDOG_AUTOTRACE
        {
            public byte AutoTrace;
            public BUSDOG_AUTOTRACE(bool autoTrace)
            {
                AutoTrace = Convert.ToByte(autoTrace);
            }
        }
        #endregion

        public bool GetDeviceList(out List<DeviceId> deviceIds)
        {
            deviceIds = new List<DeviceId>();
            uint bytesReturned;
            bool result =
                devIO.DeviceIoControl(IOCTL_BUSDOG_GET_DEVICE_LIST,
                    IntPtr.Zero,
                    0,
                    outBuffer,
                    outBufferSize,
                    out bytesReturned);
            if (result)
            {
                int index = 0;
                while (bytesReturned >= index + Marshal.SizeOf(typeof(BUSDOG_DEVICE_ID)))
                {
                    BUSDOG_DEVICE_ID devId =
                        (BUSDOG_DEVICE_ID)
                        Marshal.PtrToStructure(new IntPtr(outBuffer.ToInt64() + index),
                            typeof(BUSDOG_DEVICE_ID));
                    index += Marshal.SizeOf(typeof(BUSDOG_DEVICE_ID));
                    string hardwareId =
                        Marshal.PtrToStringUni(new IntPtr(outBuffer.ToInt64() + index),
                            devId.PhysicalDeviceObjectNameSize.ToInt32() / 2);
                    index += devId.PhysicalDeviceObjectNameSize.ToInt32();
                    deviceIds.Add(new DeviceId(devId.DeviceId, Convert.ToBoolean(devId.Enabled), hardwareId));
                }        
            }
            else
            {
                System.Diagnostics.Debug.WriteLine(Marshal.GetLastWin32Error());
            }
            return result;
        }

        public bool SetDeviceEnabled(uint deviceId, bool enabled)
        {
            GCHandle h = GCHandle.Alloc(new BUSDOG_FILTER_ENABLED(deviceId, enabled), GCHandleType.Pinned);
            IntPtr p = h.AddrOfPinnedObject();
            uint bytesReturned;
            bool result =
                devIO.DeviceIoControl(IOCTL_BUSDOG_SET_DEVICE_FILTER_ENABLED,
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

        public List<FilterTrace> GetTraceList(IntPtr buffer, uint bufferSize, uint bytesReturned)
        {
            List<FilterTrace> filterTraces = new List<FilterTrace>();
            int index = 0;
            while (bytesReturned >= index + Marshal.SizeOf(typeof(BUSDOG_FILTER_TRACE)))
            {
                BUSDOG_FILTER_TRACE filterTrace =
                    (BUSDOG_FILTER_TRACE)
                    Marshal.PtrToStructure(new IntPtr(outBuffer.ToInt64() + index),
                        typeof(BUSDOG_FILTER_TRACE));
                index += Marshal.SizeOf(typeof(BUSDOG_FILTER_TRACE));
                if (filterTrace.BufferSize.ToInt32() > 0)
                {
                    if (bytesReturned >= index + filterTrace.BufferSize.ToInt32())
                    {
                        byte[] trace = new byte[filterTrace.BufferSize.ToInt32()];
                        Marshal.Copy(new IntPtr(outBuffer.ToInt64() + index), trace, 0, (int)filterTrace.BufferSize);
                        filterTraces.Add(new FilterTrace(filterTrace.DeviceId, filterTrace.Type, filterTrace.Params, filterTrace.Timestamp, trace));
                    }
                }
                else
                    // trace has no buffer
                    filterTraces.Add(new FilterTrace(filterTrace.DeviceId, filterTrace.Type, filterTrace.Params, filterTrace.Timestamp, null));
                index += (int)filterTrace.BufferSize;
            }
            return filterTraces;
        }

        public bool StartTracing()
        {
            uint bytesReturned;
            return devIO.DeviceIoControl(IOCTL_BUSDOG_START_FILTERING,
                    IntPtr.Zero,
                    0,
                    IntPtr.Zero,
                    0,
                    out bytesReturned);
        }

        public bool StopTracing()
        {
            uint bytesReturned;
            return devIO.DeviceIoControl(IOCTL_BUSDOG_STOP_FILTERING,
                    IntPtr.Zero,
                    0,
                    IntPtr.Zero,
                    0,
                    out bytesReturned);
        }

        public bool SetAutoTrace(bool value)
        {
            GCHandle h = GCHandle.Alloc(new BUSDOG_AUTOTRACE(value), GCHandleType.Pinned);
            IntPtr p = h.AddrOfPinnedObject();
            uint bytesReturned;
            bool result =
                devIO.DeviceIoControl(IOCTL_BUSDOG_SET_AUTOTRACE,
                    p,
                    (uint)Marshal.SizeOf(typeof(BUSDOG_AUTOTRACE)),
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

        public bool GetAutoTrace(out bool value)
        {
            value = false;
            IntPtr p = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(BUSDOG_AUTOTRACE)));
            uint bytesReturned;
            bool result =
                devIO.DeviceIoControl(IOCTL_BUSDOG_GET_AUTOTRACE,
                    IntPtr.Zero,
                    0,
                    p,
                    (uint)Marshal.SizeOf(typeof(BUSDOG_AUTOTRACE)),
                    out bytesReturned);
            if (result)
            {
                if (bytesReturned >= Marshal.SizeOf(typeof(BUSDOG_AUTOTRACE)))
                {
                    BUSDOG_AUTOTRACE autoTrace =
                    (BUSDOG_AUTOTRACE)
                    Marshal.PtrToStructure(p,
                        typeof(BUSDOG_AUTOTRACE));
                    value = Convert.ToBoolean(autoTrace.AutoTrace);
                }
            }
            else
            {
                System.Diagnostics.Debug.WriteLine(Marshal.GetLastWin32Error());
            }
            Marshal.FreeHGlobal(p);
            return result;
        }
    }
}
