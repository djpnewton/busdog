using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace driverRes
{
    class Program
    {
        static int Main(string[] args)
        {
            const string driver_file = "driver.zip";
            if (File.Exists(driver_file))
            {
                FileStream fs = File.OpenRead(driver_file);
                byte[] data = new byte[fs.Length];
                fs.Read(data, 0, data.Length);
                fs.Close();

                System.Resources.ResXResourceWriter rsxw = new System.Resources.ResXResourceWriter("driver.resx");
                rsxw.AddResource("driver", data);
                rsxw.Close();
                return 0;
            }
            System.Console.WriteLine("Error: file \"{0}\" not found.", driver_file);
            return 1;
        }
    }
}
