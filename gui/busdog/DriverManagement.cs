using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices; 
using System.IO;
using System.Reflection;
using System.Diagnostics;
using System.Resources;

using System.Globalization;

namespace busdog
{
    static class DriverManagement
    {
        [DllImport("kernel32")]
        public extern static IntPtr LoadLibrary(string libraryName);

        [DllImport("kernel32")]
        public extern static bool FreeLibrary(IntPtr hModule);

        [DllImport("kernel32", CharSet = CharSet.Ansi)]
        public extern static IntPtr GetProcAddress(IntPtr hMod, string procedureName);

        [DllImport("advapi32.dll", EntryPoint = "OpenSCManagerW", ExactSpelling = true, CharSet = CharSet.Unicode, SetLastError = true)]
        static extern IntPtr OpenSCManager(string machineName, string databaseName, uint dwAccess);

        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        static extern IntPtr OpenService(IntPtr hSCManager, string lpServiceName, uint dwDesiredAccess);

        [DllImport("advapi32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool CloseServiceHandle(IntPtr hSCObject);

        public static bool IsDriverInstalled(out bool drvInstalled)
        {
            drvInstalled = false;
            const uint GENERIC_READ = 0x80000000;
            IntPtr scmgr = OpenSCManager(null, null, GENERIC_READ);
            if (!scmgr.Equals(IntPtr.Zero))
            {
                const uint SERVICE_QUERY_STATUS       = 0x00004;
                IntPtr service = OpenService(scmgr, "busdog", SERVICE_QUERY_STATUS);
                if (!service.Equals(IntPtr.Zero))
                {
                    drvInstalled = true;
                    CloseServiceHandle(service);
                }
                CloseServiceHandle(scmgr);
                return true;
            }
            return false;
        }

        delegate uint WdfCoinstallInvoker(
            [MarshalAs(UnmanagedType.LPWStr)]
            string infPath,
            [MarshalAs(UnmanagedType.LPWStr)]
            string infSectionName);

        const string infFile = "busdog.inf";
        const string sysFile = "busdog.sys";
        const string dpinstFile = "dpinst.exe";
        const string coinstFile = "WdfCoInstaller01009.dll";
        const string wdfInfSection = "busdog.NT.Wdf";
        const string wdfPreDeviceInstall = "WdfPreDeviceInstall";
        const string wdfPostDeviceInstall = "WdfPostDeviceInstall";
        const string wdfPreDeviceRemove = "WdfPreDeviceRemove";
        const string wdfPostDeviceRemove = "WdfPostDeviceRemove";

        private static IntPtr GetCoinstallerFuncs(string mydir, out IntPtr wdfPreDeviceInstallPtr, out IntPtr wdfPostDeviceInstallPtr, out IntPtr wdfPreDeviceRemovePtr, out IntPtr wdfPostDeviceRemovePtr, out bool procAddressFailure)
        {
            procAddressFailure = false;
            // Get wdf coinstaller functions
            IntPtr hModule = LoadLibrary(Path.Combine(mydir, coinstFile));
            wdfPreDeviceInstallPtr = GetProcAddress(hModule, wdfPreDeviceInstall);
            wdfPostDeviceInstallPtr = GetProcAddress(hModule, wdfPostDeviceInstall);
            wdfPreDeviceRemovePtr = GetProcAddress(hModule, wdfPreDeviceRemove);
            wdfPostDeviceRemovePtr = GetProcAddress(hModule, wdfPostDeviceRemove);

            if (wdfPreDeviceInstallPtr.ToInt64() == 0 ||
                wdfPostDeviceInstallPtr.ToInt64() == 0 ||
                wdfPreDeviceRemovePtr.ToInt64() == 0 ||
                wdfPostDeviceRemovePtr.ToInt64() == 0)
            {
                procAddressFailure = true;
            }

            return hModule;
        }

        public static bool InstallDriver(out bool needRestart, out string failureReason)
        {
            failureReason = null;
            bool result = true;
            needRestart = false;
            string mydir;
            if (ExtractDriverFiles(out mydir))
            {
                ulong wdfCallResult = 0;
                IntPtr wdfPreDeviceInstallPtr;
                IntPtr wdfPostDeviceInstallPtr;
                IntPtr wdfPreDeviceRemovePtr;
                IntPtr wdfPostDeviceRemovePtr;
                bool procAddressFailure;
                IntPtr hModule = GetCoinstallerFuncs(mydir, out wdfPreDeviceInstallPtr, out wdfPostDeviceInstallPtr, out wdfPreDeviceRemovePtr, out wdfPostDeviceRemovePtr, out procAddressFailure);
                if (!procAddressFailure)
                {
                    // call WdfPreDeviceInstall
                    WdfCoinstallInvoker preDevInst = (WdfCoinstallInvoker)Marshal.GetDelegateForFunctionPointer(wdfPreDeviceInstallPtr, typeof(WdfCoinstallInvoker));
                    wdfCallResult = preDevInst(Path.Combine(mydir, infFile), wdfInfSection);
                    if (wdfCallResult != 0)
                    {
                        result = false;
                        failureReason = string.Format("{0} result = 0x{1:X}", wdfPreDeviceInstall, wdfCallResult);
                    }
                    else
                    {
                        // run dpinst
                        Process p = Process.Start(Path.Combine(mydir, dpinstFile), "/lm /q");
                        p.WaitForExit();
                        if (((p.ExitCode >> 24) & 0x40) == 0x40)
                            needRestart = true;
                        if (((p.ExitCode >> 24) & 0x80) == 0x80)
                        {
                            result = false;
                            failureReason = string.Format("DPInst result = 0x{0:X}", p.ExitCode);
                        }
                        else
                        {
                            // call WdfPostDeviceInstall
                            WdfCoinstallInvoker postDevInst = (WdfCoinstallInvoker)Marshal.GetDelegateForFunctionPointer(wdfPostDeviceInstallPtr, typeof(WdfCoinstallInvoker));
                            wdfCallResult = postDevInst(Path.Combine(mydir, infFile), wdfInfSection);
                            if (wdfCallResult != 0)
                            {
                                result = false;
                                failureReason = string.Format("{0} result = 0x{1:X}", wdfPostDeviceInstall, wdfCallResult);
                            }
                        }
                    }
                }
                else
                {
                    result = false;
                    failureReason = "Error getting WDF coinstaller function addresses";
                }
                // free coinstaller library
                FreeLibrary(hModule);
                // delete extracted files
                if (Directory.Exists(mydir))
                   Directory.Delete(mydir, true);
            }
            else
            {
                result = false;
                failureReason = "ExtractDriverFiles failed";
            }
            return result;
        }

        public static bool UninstallDriver(out bool needRestart, out string failureReason)
        {
            failureReason = null;
            ulong wdfCallResult = 0;
            bool result = true;
            needRestart = false;
            string mydir;
            if (ExtractDriverFiles(out mydir))
            {
                IntPtr wdfPreDeviceInstallPtr;
                IntPtr wdfPostDeviceInstallPtr;
                IntPtr wdfPreDeviceRemovePtr;
                IntPtr wdfPostDeviceRemovePtr;
                bool procAddressFailure;
                IntPtr hModule = GetCoinstallerFuncs(mydir, out wdfPreDeviceInstallPtr, out wdfPostDeviceInstallPtr, out wdfPreDeviceRemovePtr, out wdfPostDeviceRemovePtr, out procAddressFailure);
                if (!procAddressFailure)
                {
                    // call WdfPreDeviceRemove
                    WdfCoinstallInvoker preDevRemove = (WdfCoinstallInvoker)Marshal.GetDelegateForFunctionPointer(wdfPreDeviceRemovePtr, typeof(WdfCoinstallInvoker));
                    wdfCallResult = preDevRemove(Path.Combine(mydir, infFile), wdfInfSection);
                    if (wdfCallResult != 0)
                    {
                        result = false;
                        failureReason = string.Format("{0} result = 0x{1:X}", wdfPreDeviceRemove, wdfCallResult);
                    }
                    else
                    {
                        // run dpinst
                        string inffile = Path.Combine(mydir, infFile);
                        Process p = Process.Start(Path.Combine(mydir, dpinstFile), string.Format("/u \"{0}\" /d /q", inffile));
                        p.WaitForExit();
                        if (((p.ExitCode >> 24) & 0x40) == 0x40)
                            needRestart = true;
                        if (((p.ExitCode >> 24) & 0x80) == 0x80)
                        {
                            result = false;
                            failureReason = string.Format("DPInst result = 0x{0:X}", p.ExitCode);
                        }
                        else
                        {
                            // call WdfPostDeviceRemove
                            WdfCoinstallInvoker postDevRemove = (WdfCoinstallInvoker)Marshal.GetDelegateForFunctionPointer(wdfPostDeviceRemovePtr, typeof(WdfCoinstallInvoker));
                            wdfCallResult = postDevRemove(Path.Combine(mydir, infFile), wdfInfSection);
                            if (wdfCallResult != 0)
                            {
                                result = false;
                                failureReason = string.Format("{0} result = 0x{1:X}", wdfPostDeviceRemove, wdfCallResult);
                            }
                        }
                    }
                }
                else
                {
                    result = false;
                    failureReason = "Error getting WDF coinstaller function addresses";
                }
                // free coinstaller library
                FreeLibrary(hModule);
                // delete extracted files
                if (Directory.Exists(mydir))
                    Directory.Delete(mydir, true);
            }
            else
            {
                result = false;
                failureReason = "ExtractDriverFiles failed";
            }
            return result;
        }

        private static bool ExtractDriverFiles(out string mydir)
        {
            string tempdir = Path.GetTempPath();
            // This needs to be the same dir for uninstall (dpinst not cool here)
            mydir = Path.Combine(tempdir, "busdog_temp_busdog_temp");
            try
            {
                Directory.CreateDirectory(mydir);
                WriteDrverFile(sysFile, mydir);
                WriteDrverFile(infFile, mydir);
                WriteDrverFile(coinstFile, mydir);
                WriteDrverFile(dpinstFile, mydir);
            }
            catch
            {
                return false;
            }
            return true;
        }
        private static void WriteDrverFile(string resname, string dir)
        {
            Assembly ass = Assembly.GetExecutingAssembly();
            ResourceManager rm = new ResourceManager("busdog.driver", ass);

            ResourceSet set = rm.GetResourceSet(CultureInfo.CurrentCulture, true, true);
            byte[] buf = (byte[])set.GetObject(resname);

            Stream w = File.OpenWrite(Path.Combine(dir, resname));
            w.Write(buf, 0, buf.Length);
            w.Flush();
            w.Close();
        }
    }
}
