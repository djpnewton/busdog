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
        DeviceManagement devManage = new DeviceManagement();
        IntPtr devNotificationsHandle;

        public MainForm()
        {
            InitializeComponent();

            devManage.RegisterForDeviceNotifications(Handle, ref devNotificationsHandle);

            EnumFilterDevices();
        }

        private void EnumFilterDevices()
        {
            tvDevices.Nodes.Clear();

            List<DeviceId> deviceIds;
            native.GetDeviceList(out deviceIds);

            for (int i = 0; i < deviceIds.Count; i++)
            {
                DeviceId devId = deviceIds[i];
                devManage.FindDeviceProps(devId.PhysicalDeviceObjectName, out devId.HardwareId, out devId.Description, out devId.InstanceId);

                TreeNode child = new TreeNode(devId.ToString());
                child.Checked = devId.Enabled;
                child.Tag = devId;
                for (int j = 0; j < tvDevices.Nodes.Count; j++)
                {
                    DeviceId devId2 = (DeviceId)tvDevices.Nodes[j].Tag;
                    if (devManage.IsDeviceChild(devId2.InstanceId, devId.InstanceId))
                    {
                        tvDevices.Nodes[j].Nodes.Add(child);
                        break;
                    }
                }
                if (child.Parent == null)
                    tvDevices.Nodes.Add(child);
            }
            tvDevices.ExpandAll();
        }

        protected override void WndProc(ref Message m)
        {
            if (m.Msg == DeviceManagement.WM_DEVICECHANGE)
            {
                tmrDeviceChange.Enabled = false;
                tmrDeviceChange.Enabled = true;
            }

            //  Let the base form process the message.
            base.WndProc(ref m);
        }        

        private void tmrTrace_Tick(object sender, EventArgs e)
        {
            if (tabControl.SelectedTab == tabTrace)
            {
                List<FilterTrace> filterTraces;
                if (native.GetTraceList(out filterTraces))
                {
                    foreach (FilterTrace filterTrace in filterTraces)
                    {
                        AddFilterTrace(filterTrace);
                    }
                }
            }
        }

        private void AddFilterTrace(FilterTrace filterTrace)
        {
            // Create a new row.
            ListViewItem item = new ListViewItem(filterTrace.DeviceId.ToString());

            for (int i = 1; i < lvTraces.Columns.Count; i++)
            {
                switch (i)
                {
                    case 1:
                        item.SubItems.Add(filterTrace.TypeToStr());
                        break;
                    case 2:
                        item.SubItems.Add(filterTrace.TimestampToStr());
                        break;
                    case 3:
                        item.SubItems.Add(filterTrace.Buffer.Length.ToString());
                        break;
                    case 4:
                        item.SubItems.Add(filterTrace.BufToHex());
                        break;
                    case 5:
                        item.SubItems.Add(filterTrace.BufToChars());
                        break;
                }
            }

            lvTraces.TopItem = lvTraces.Items.Add(item);
        }

        private void tmrDeviceChange_Tick(object sender, EventArgs e)
        {
            EnumFilterDevices();
            tmrDeviceChange.Enabled = false;
        }

        private void tvDevices_AfterCheck(object sender, TreeViewEventArgs e)
        {            
            native.SetDeviceEnabled(
                ((DeviceId)e.Node.Tag).DevId,
                e.Node.Checked);
        }
    }
}
