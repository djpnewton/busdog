using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace busdog
{
    enum StartupActions
    {
        InstallDriver = 1,
        UninstallDriver = 2
    }

    public partial class MainForm : Form
    {
        public delegate void FilterTraceArrived(object sender, FilterTraceArrivedEventArgs e);

        Native native = new Native();
        DeviceManagement devManage = new DeviceManagement();
        IntPtr devNotificationsHandle;
        FilterTrace prevTrace = new FilterTrace();

        public MainForm()
        {
            InitializeComponent();

            if (!VistaSecurity.IsAdmin())
            {
                VistaSecurity.AddShieldToButton(btnReinstall);
                VistaSecurity.AddShieldToButton(btnUninstall);
            }
            else
                this.Text += " (Elevated)";
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            if (!ProcessCommandLine())
                CheckDriverInstallation();

            devManage.RegisterForDeviceNotifications(Handle, ref devNotificationsHandle);

            EnumFilterDevices();

            native.FilterTraceArrived += new EventHandler<FilterTraceArrivedEventArgs>(RecievedFilterTraces);
        }

        private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            native.StopTraceReader();
        }

        private bool ProcessCommandLine()
        {
            foreach (string arg in Environment.GetCommandLineArgs())
            {
                if (arg == StartupActions.InstallDriver.ToString())
                {
                    if (VistaSecurity.IsAdmin())
                        InstallDriver();
                    else
                        MessageBox.Show("Could not install driver as user is not an Admin",
                            "Startup Action Install Driver failed");
                    return true;
                }
                else if (arg == StartupActions.UninstallDriver.ToString())
                {
                    if (VistaSecurity.IsAdmin())
                        UninstallDriver();
                    else
                        MessageBox.Show("Could not uninstall driver as user is not an Admin",
                            "Startup Action Uninstall Driver failed");
                    return true;
                }
            }
            return false;
        }

        private void EnumFilterDevices()
        {
            SuspendLayout();

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

            ResumeLayout(true);
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

        private void RecievedFilterTraces(object sender, FilterTraceArrivedEventArgs e)
        {
            if (InvokeRequired)
            {
                Invoke(new FilterTraceArrived(RecievedFilterTraces), new Object[] { sender, e });
            }
            else
            {
                if (tabControl.SelectedTab == tabTrace)
                {
                    SuspendLayout();

                    foreach (FilterTrace filterTrace in e.Traces)
                    {
                        AddFilterTrace(filterTrace);
                    }

                    ResumeLayout(true);
                }
            }
        }

        private void AddFilterTrace(FilterTrace filterTrace)
        {
            // Check filters
            if (DoesTracePassFilters(filterTrace, FilterInclude.Include) &&
                DoesTracePassFilters(filterTrace, FilterInclude.Exclude))
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
                            if (filterTrace.Buffer != null)
                                item.SubItems.Add(filterTrace.Buffer.Length.ToString());
                            else
                                item.SubItems.Add(Convert.ToString(0));
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
        }

        private bool DoesTracePassFilters(FilterTrace filterTrace, FilterInclude include)
        {
            List<FilterMatch> filters;
            if (include == FilterInclude.Include)
                filters = filterControl.IncludeFilters;
            else
                filters = filterControl.ExcludeFilters;

            if (filters.Count == 0)
                return true;

            bool check = true;

            foreach (FilterMatch filter in filters)
            {
                switch (filter.FilterType)
                {
                    case FilterType.Length:
                        switch (filter.LengthMatch)
                        {
                            case LengthMatch.GreaterThen:
                                check = filterTrace.Buffer.Length > filter.Length;
                                break;
                            case LengthMatch.LessThen:
                                check = filterTrace.Buffer.Length < filter.Length;
                                break;
                            case LengthMatch.EqualTo:
                                check = filterTrace.Buffer.Length == filter.Length;
                                break;
                        }
                        break;
                    case FilterType.Hex:
                        check = filterTrace.BufToHex().Contains(filter.Filter);
                        break;
                    case FilterType.Ascii:
                        check = filterTrace.BufToChars().Contains(filter.Filter);
                        break;
                }
                if (include == FilterInclude.Include)
                {
                    if (check)
                        return true;
                    else
                        continue;
                }
                else
                {
                    if (check)
                        return false;
                    else
                        continue;
                }
            }
            if (include == FilterInclude.Include)
                return false;
            else
                return true;
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
            UpdateTracingStatus();
        }

        private void UpdateTracingStatus()
        {
            if (btnStartTraces.Checked)
            {
                if (tabControl.SelectedTab == tabTrace)
                    native.StartTracing();
                else
                    native.StopTracing();
                native.StartTraceReader();
            }
            else
                native.StopTraceReader();
            btnStartTraces.Checked = btnStartTraces.Checked;
        }

        private void btnClearTraces_Click(object sender, EventArgs e)
        {
            lvTraces.Items.Clear();
            prevTrace = new FilterTrace();
        }

        private void CheckDriverInstallation()
        {
            bool drvInstalled;
            if (DriverManagement.IsDriverInstalled(out drvInstalled))
            {
                if (!drvInstalled)
                {
                    if (MessageBox.Show(
                        "BusDog Filter Driver is not installed. Do you want to install it now?",
                        "Driver Not Installed",
                        MessageBoxButtons.YesNo) == DialogResult.Yes)
                    {
                        if (VistaSecurity.IsAdmin())
                            InstallDriver();
                        else
                            VistaSecurity.RestartElevated(StartupActions.InstallDriver.ToString());
                    }
                }
            }
        }

        private void InstallDriver()
        {
            bool needRestart;
            string failureReason;
            if (DriverManagement.InstallDriver(out needRestart, out failureReason))
            {
                if (needRestart)
                    MessageBox.Show("BusDog Filter Driver installed! Restart required to complete.",
                        "Driver Installed");
                else
                    MessageBox.Show("BusDog Filter Driver installed!",
                        "Driver Installed");
            }
            else
                MessageBox.Show(string.Format("BusDog Filter Driver installation failed ({0})", failureReason), 
                    "Driver Installation Failed");
        }

        private void UninstallDriver()
        {
            bool needRestart;
            string failureReason;
            if (DriverManagement.UninstallDriver(out needRestart, out failureReason))
            {
                if (needRestart)
                    MessageBox.Show("BusDog Filter Driver uninstalled! Restart required to complete.",
                        "Driver Installed");
                else
                    MessageBox.Show("BusDog Filter Driver uninstalled!",
                        "Driver Uninstalled");
            }
            else
                MessageBox.Show(string.Format("BusDog Filter Driver uninstallation failed ({0})", failureReason), 
                    "Driver Uninstallation Failed");
        }

        private void btnReinstall_Click(object sender, EventArgs e)
        {
            if (VistaSecurity.IsAdmin())
            {
                InstallDriver();
            }
            else
            {
                VistaSecurity.RestartElevated(StartupActions.InstallDriver.ToString());
            } 
        }

        private void btnUninstall_Click(object sender, EventArgs e)
        {
            if (VistaSecurity.IsAdmin())
            {
                UninstallDriver();
            }
            else
            {
                VistaSecurity.RestartElevated(StartupActions.UninstallDriver.ToString());
            } 
        }

        private void cbTraceListColumn_CheckedChanged(object sender, EventArgs e)
        {
            UpdateColumn(chId, cbId);
            UpdateColumn(chType, cbType);
            UpdateColumn(chTime, cbTime);
            UpdateColumn(chLength, cbLength);
            UpdateColumn(chHex, cbHex);
            UpdateColumn(chAscii, cbAscii);
        }

        private void UpdateColumn(ColumnHeader ch, CheckBox cb)
        {
            if (cb.Checked && ch.Width == 0)
                ch.Width = (int)ch.Tag;
            else if (!cb.Checked && ch.Width != 0)
            {
                ch.Tag = ch.Width;
                ch.Width = 0;
            }
        }

        private void tabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdateTracingStatus();
        }
    }

    public class BufferedListView : ListView
    {
        public BufferedListView()
            : base()
        {
            SetStyle(ControlStyles.OptimizedDoubleBuffer, true);
        }
    }
}
