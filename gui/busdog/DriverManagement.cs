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

        public static bool InstallDriver(out bool needRestart)
        {
            needRestart = false;
            string mydir;
            if (ExtractDriverFiles(out mydir))
            {
                Process p = Process.Start(Path.Combine(mydir, "dpinst.exe"), "/lm");
                p.WaitForExit();
                if (Directory.Exists(mydir))
                    Directory.Delete(mydir, true);
                if (((p.ExitCode >> 24) & 0x40) == 0x40)
                    needRestart = true;
                if (((p.ExitCode >> 24) & 0x80) == 0x80)
                    return false;
                return true;
            }
            else
                return false;
        }

        public static bool UninstallDriver(out bool needRestart)
        {
            needRestart = false;
            string mydir;
            if (ExtractDriverFiles(out mydir))
            {
                Process p = Process.Start(Path.Combine(mydir, "dpinst.exe"), "/u busdog.inf /d");
                p.WaitForExit();
                if (Directory.Exists(mydir))
                    Directory.Delete(mydir, true);
                if (((p.ExitCode >> 24) & 0x40) == 0x40)
                    needRestart = true;
                if (((p.ExitCode >> 24) & 0x80) == 0x80)
                    return false;
                return true;
            }
            else
                return false;
        }

        private static bool ExtractDriverFiles(out string mydir)
        {
            string tempdir = Path.GetTempPath();
            // This needs to be the same dir for uninstall (dpinst not cool here)
            mydir = Path.Combine(tempdir, "busdog_temp_busdog_temp");
            try
            {
                Directory.CreateDirectory(mydir);
                WriteDrverFile("busdog.sys", mydir);
                WriteDrverFile("busdog.inf", mydir);
                WriteDrverFile("WdfCoInstaller01009.dll", mydir);
                WriteDrverFile("dpinst.exe", mydir);
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
