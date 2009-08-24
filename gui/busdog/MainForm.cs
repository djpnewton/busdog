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
        FilterTrace prevTrace = new FilterTrace();

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
                child.ToolTipText = devId.HardwareId;
                child.Tag = devId;
                if (!InsertNodeInDeviceTree(devId, tvDevices.Nodes, child))
                    tvDevices.Nodes.Add(child);
            }
            tvDevices.ExpandAll();
        }

        private bool InsertNodeInDeviceTree(DeviceId devId, TreeNodeCollection parentNodes, TreeNode child)
        {
            for (int i = 0; i < parentNodes.Count; i++)
            {
                DeviceId devIdParent = (DeviceId)parentNodes[i].Tag;
                if (devManage.IsDeviceChild(devIdParent.InstanceId, devId.InstanceId))
                {
                    parentNodes[i].Nodes.Add(child);
                    return true;
                }
                if (InsertNodeInDeviceTree(devId, parentNodes[i].Nodes, child))
                    return true;
            }
            return false;
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
                        item.SubItems.Add(filterTrace.GetTimestampDelta(prevTrace).ToString());
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
            prevTrace = filterTrace;
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

        private void btnStartTraces_Click(object sender, EventArgs e)
        {
            if (btnStartTraces.Checked)
                native.StartTracing();
            else
                native.StopTracing();
            tmrTrace.Enabled = btnStartTraces.Checked;
            btnStartTraces.Checked = btnStartTraces.Checked;
        }

        private void btnClearTraces_Click(object sender, EventArgs e)
        {
            lvTraces.Items.Clear();
            prevTrace = new FilterTrace();
        }
    }
}
