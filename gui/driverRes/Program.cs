using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace driverRes
{
    class Program
    {
        static int Main(string[] args)
        {
            string driver_file = "driver.zip";
            if (args.Length > 0)
                driver_file = args[0];
            if (File.Exists(driver_file))
            {
                FileStream fs = File.OpenRead(driver_file);
                byte[] data = new byte[fs.Length];
                fs.Read(data, 0, data.Length);
                fs.Close();

                string output_file = "driver.resx";
                output_file = Path.Combine(Path.GetDirectoryName(driver_file), output_file);
                System.Resources.ResXResourceWriter rsxw = new System.Resources.ResXResourceWriter(output_file);
                rsxw.AddResource("driver", data);
                rsxw.Close();
                return 0;
            }
            System.Console.WriteLine("Error: file \"{0}\" not found.", driver_file);
            return 1;
        }
    }
}
