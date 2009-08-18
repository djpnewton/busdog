using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace busdog
{
    public partial class MainForm : Form
    {
        Native native = new Native();

        public MainForm()
        {
            InitializeComponent();

            List<DeviceId> deviceIds;
            native.GetDeviceList(out deviceIds);

            foreach (DeviceId devId in deviceIds)
            {
                lbDevices.Items.Add(devId, devId.Enabled);
            }
        }

        private void lbDevices_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            native.SetDeviceEnabled(
                ((DeviceId)lbDevices.Items[e.Index]).DevId,
                e.NewValue == CheckState.Checked);
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (tabControl.SelectedTab == tabTrace)
            {
                List<FilterTrace> filterTraces;
                if (native.GetTraceList(out filterTraces))
                {
                    foreach (FilterTrace filterTrace in filterTraces)
                    {
                        lbTraces.SelectedIndex = lbTraces.Items.Add(filterTrace);
                    }
                }
            }
        }
    }
}
