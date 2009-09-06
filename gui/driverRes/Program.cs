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
            if (args.Length == 0)
            {
                System.Console.WriteLine("Error: no arguments.");
                return 2;
            }

            
            string output_file = "driver.resx";
            output_file = Path.Combine(Path.GetDirectoryName(Application.ExecutablePath), output_file);

            System.Resources.ResXResourceWriter rsxw = new System.Resources.ResXResourceWriter(output_file);

            for (int i = 0; i < args.Length; i++)
            {
                string driver_file = args[i];
                if (File.Exists(driver_file))
                {
                    FileStream fs = File.OpenRead(driver_file);
                    byte[] data = new byte[fs.Length];
                    fs.Read(data, 0, data.Length);
                    fs.Close();

                    rsxw.AddResource(Path.GetFileName(driver_file), data);
                }
                else
                {
                    System.Console.WriteLine("Error: file \"{0}\" not found.", driver_file);
                    return 1;
                }
            }

            rsxw.Generate();
            rsxw.Close();

            return 0;
        }
    }
}
