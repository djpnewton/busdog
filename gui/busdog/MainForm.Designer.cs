namespace busdog
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.tabControl = new System.Windows.Forms.TabControl();
            this.tabDevices = new System.Windows.Forms.TabPage();
            this.tvDevices = new System.Windows.Forms.TreeView();
            this.tabTrace = new System.Windows.Forms.TabPage();
            this.tscTrace = new System.Windows.Forms.ToolStripContainer();
            this.lvTraces = new busdog.BufferedListView();
            this.chId = new System.Windows.Forms.ColumnHeader();
            this.chType = new System.Windows.Forms.ColumnHeader();
            this.chTime = new System.Windows.Forms.ColumnHeader();
            this.chLength = new System.Windows.Forms.ColumnHeader();
            this.chHex = new System.Windows.Forms.ColumnHeader();
            this.chAscii = new System.Windows.Forms.ColumnHeader();
            this.tsTrace = new System.Windows.Forms.ToolStrip();
            this.btnStartTraces = new System.Windows.Forms.ToolStripButton();
            this.btnClearTraces = new System.Windows.Forms.ToolStripButton();
            this.tabSetup = new System.Windows.Forms.TabPage();
            this.filterControl = new busdog.FilterControl();
            this.gbTraceList = new System.Windows.Forms.GroupBox();
            this.cbAscii = new System.Windows.Forms.CheckBox();
            this.cbHex = new System.Windows.Forms.CheckBox();
            this.cbLength = new System.Windows.Forms.CheckBox();
            this.cbTime = new System.Windows.Forms.CheckBox();
            this.cbType = new System.Windows.Forms.CheckBox();
            this.cbId = new System.Windows.Forms.CheckBox();
            this.gbFilterDriver = new System.Windows.Forms.GroupBox();
            this.btnReinstall = new System.Windows.Forms.Button();
            this.btnUninstall = new System.Windows.Forms.Button();
            this.tmrTrace = new System.Windows.Forms.Timer(this.components);
            this.tmrDeviceChange = new System.Windows.Forms.Timer(this.components);
            this.tabControl.SuspendLayout();
            this.tabDevices.SuspendLayout();
            this.tabTrace.SuspendLayout();
            this.tscTrace.ContentPanel.SuspendLayout();
            this.tscTrace.TopToolStripPanel.SuspendLayout();
            this.tscTrace.SuspendLayout();
            this.tsTrace.SuspendLayout();
            this.tabSetup.SuspendLayout();
            this.gbTraceList.SuspendLayout();
            this.gbFilterDriver.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl
            // 
            this.tabControl.Controls.Add(this.tabDevices);
            this.tabControl.Controls.Add(this.tabTrace);
            this.tabControl.Controls.Add(this.tabSetup);
            this.tabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl.Location = new System.Drawing.Point(0, 0);
            this.tabControl.Name = "tabControl";
            this.tabControl.SelectedIndex = 0;
            this.tabControl.Size = new System.Drawing.Size(385, 386);
            this.tabControl.TabIndex = 0;
            this.tabControl.SelectedIndexChanged += new System.EventHandler(this.tabControl_SelectedIndexChanged);
            // 
            // tabDevices
            // 
            this.tabDevices.Controls.Add(this.tvDevices);
            this.tabDevices.Location = new System.Drawing.Point(4, 22);
            this.tabDevices.Name = "tabDevices";
            this.tabDevices.Padding = new System.Windows.Forms.Padding(3);
            this.tabDevices.Size = new System.Drawing.Size(377, 360);
            this.tabDevices.TabIndex = 0;
            this.tabDevices.Text = "Devices";
            this.tabDevices.UseVisualStyleBackColor = true;
            // 
            // tvDevices
            // 
            this.tvDevices.CheckBoxes = true;
            this.tvDevices.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvDevices.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tvDevices.Location = new System.Drawing.Point(3, 3);
            this.tvDevices.Name = "tvDevices";
            this.tvDevices.ShowNodeToolTips = true;
            this.tvDevices.Size = new System.Drawing.Size(371, 354);
            this.tvDevices.TabIndex = 1;
            this.tvDevices.AfterCheck += new System.Windows.Forms.TreeViewEventHandler(this.tvDevices_AfterCheck);
            // 
            // tabTrace
            // 
            this.tabTrace.Controls.Add(this.tscTrace);
            this.tabTrace.Location = new System.Drawing.Point(4, 22);
            this.tabTrace.Name = "tabTrace";
            this.tabTrace.Padding = new System.Windows.Forms.Padding(3);
            this.tabTrace.Size = new System.Drawing.Size(377, 360);
            this.tabTrace.TabIndex = 1;
            this.tabTrace.Text = "Trace";
            this.tabTrace.UseVisualStyleBackColor = true;
            // 
            // tscTrace
            // 
            // 
            // tscTrace.ContentPanel
            // 
            this.tscTrace.ContentPanel.Controls.Add(this.lvTraces);
            this.tscTrace.ContentPanel.Size = new System.Drawing.Size(371, 329);
            this.tscTrace.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tscTrace.Location = new System.Drawing.Point(3, 3);
            this.tscTrace.Name = "tscTrace";
            this.tscTrace.Size = new System.Drawing.Size(371, 354);
            this.tscTrace.TabIndex = 1;
            this.tscTrace.Text = "toolStripContainer1";
            // 
            // tscTrace.TopToolStripPanel
            // 
            this.tscTrace.TopToolStripPanel.Controls.Add(this.tsTrace);
            // 
            // lvTraces
            // 
            this.lvTraces.AllowColumnReorder = true;
            this.lvTraces.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.chId,
            this.chType,
            this.chTime,
            this.chLength,
            this.chHex,
            this.chAscii});
            this.lvTraces.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lvTraces.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lvTraces.FullRowSelect = true;
            this.lvTraces.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.lvTraces.Location = new System.Drawing.Point(0, 0);
            this.lvTraces.Name = "lvTraces";
            this.lvTraces.Size = new System.Drawing.Size(371, 329);
            this.lvTraces.TabIndex = 3;
            this.lvTraces.UseCompatibleStateImageBehavior = false;
            this.lvTraces.View = System.Windows.Forms.View.Details;
            // 
            // chId
            // 
            this.chId.Text = "Id";
            this.chId.Width = 30;
            // 
            // chType
            // 
            this.chType.Text = "Type";
            this.chType.Width = 46;
            // 
            // chTime
            // 
            this.chTime.Text = "Time";
            this.chTime.Width = 79;
            // 
            // chLength
            // 
            this.chLength.Text = "Length";
            this.chLength.Width = 59;
            // 
            // chHex
            // 
            this.chHex.Text = "Hex";
            // 
            // chAscii
            // 
            this.chAscii.Text = "Ascii";
            // 
            // tsTrace
            // 
            this.tsTrace.Dock = System.Windows.Forms.DockStyle.None;
            this.tsTrace.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.btnStartTraces,
            this.btnClearTraces});
            this.tsTrace.Location = new System.Drawing.Point(3, 0);
            this.tsTrace.Name = "tsTrace";
            this.tsTrace.Size = new System.Drawing.Size(83, 25);
            this.tsTrace.TabIndex = 4;
            this.tsTrace.Text = "tsTrace";
            // 
            // btnStartTraces
            // 
            this.btnStartTraces.CheckOnClick = true;
            this.btnStartTraces.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.btnStartTraces.Image = ((System.Drawing.Image)(resources.GetObject("btnStartTraces.Image")));
            this.btnStartTraces.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnStartTraces.Name = "btnStartTraces";
            this.btnStartTraces.Size = new System.Drawing.Size(35, 22);
            this.btnStartTraces.Text = "Start";
            this.btnStartTraces.Click += new System.EventHandler(this.btnStartTraces_Click);
            // 
            // btnClearTraces
            // 
            this.btnClearTraces.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.btnClearTraces.Image = ((System.Drawing.Image)(resources.GetObject("btnClearTraces.Image")));
            this.btnClearTraces.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnClearTraces.Name = "btnClearTraces";
            this.btnClearTraces.Size = new System.Drawing.Size(36, 22);
            this.btnClearTraces.Text = "Clear";
            this.btnClearTraces.Click += new System.EventHandler(this.btnClearTraces_Click);
            // 
            // tabSetup
            // 
            this.tabSetup.Controls.Add(this.filterControl);
            this.tabSetup.Controls.Add(this.gbTraceList);
            this.tabSetup.Controls.Add(this.gbFilterDriver);
            this.tabSetup.Location = new System.Drawing.Point(4, 22);
            this.tabSetup.Name = "tabSetup";
            this.tabSetup.Size = new System.Drawing.Size(377, 360);
            this.tabSetup.TabIndex = 2;
            this.tabSetup.Text = "Setup";
            this.tabSetup.UseVisualStyleBackColor = true;
            // 
            // filterControl
            // 
            this.filterControl.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.filterControl.Filter = "";
            this.filterControl.Include = busdog.FilterInclude.Include;
            this.filterControl.LengthMatch_ = busdog.LengthMatch.GreaterThen;
            this.filterControl.Location = new System.Drawing.Point(8, 156);
            this.filterControl.Name = "filterControl";
            this.filterControl.Size = new System.Drawing.Size(361, 196);
            this.filterControl.TabIndex = 4;
            this.filterControl.Type = busdog.FilterType.Length;
            // 
            // gbTraceList
            // 
            this.gbTraceList.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.gbTraceList.Controls.Add(this.cbAscii);
            this.gbTraceList.Controls.Add(this.cbHex);
            this.gbTraceList.Controls.Add(this.cbLength);
            this.gbTraceList.Controls.Add(this.cbTime);
            this.gbTraceList.Controls.Add(this.cbType);
            this.gbTraceList.Controls.Add(this.cbId);
            this.gbTraceList.Location = new System.Drawing.Point(8, 60);
            this.gbTraceList.Name = "gbTraceList";
            this.gbTraceList.Size = new System.Drawing.Size(361, 90);
            this.gbTraceList.TabIndex = 3;
            this.gbTraceList.TabStop = false;
            this.gbTraceList.Text = "Trace List Columns";
            // 
            // cbAscii
            // 
            this.cbAscii.AutoSize = true;
            this.cbAscii.Checked = true;
            this.cbAscii.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbAscii.Location = new System.Drawing.Point(114, 65);
            this.cbAscii.Name = "cbAscii";
            this.cbAscii.Size = new System.Drawing.Size(48, 17);
            this.cbAscii.TabIndex = 5;
            this.cbAscii.Text = "Ascii";
            this.cbAscii.UseVisualStyleBackColor = true;
            this.cbAscii.CheckedChanged += new System.EventHandler(this.cbTraceListColumn_CheckedChanged);
            // 
            // cbHex
            // 
            this.cbHex.AutoSize = true;
            this.cbHex.Checked = true;
            this.cbHex.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbHex.Location = new System.Drawing.Point(114, 42);
            this.cbHex.Name = "cbHex";
            this.cbHex.Size = new System.Drawing.Size(45, 17);
            this.cbHex.TabIndex = 4;
            this.cbHex.Text = "Hex";
            this.cbHex.UseVisualStyleBackColor = true;
            this.cbHex.CheckedChanged += new System.EventHandler(this.cbTraceListColumn_CheckedChanged);
            // 
            // cbLength
            // 
            this.cbLength.AutoSize = true;
            this.cbLength.Checked = true;
            this.cbLength.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbLength.Location = new System.Drawing.Point(114, 19);
            this.cbLength.Name = "cbLength";
            this.cbLength.Size = new System.Drawing.Size(59, 17);
            this.cbLength.TabIndex = 3;
            this.cbLength.Text = "Length";
            this.cbLength.UseVisualStyleBackColor = true;
            this.cbLength.CheckedChanged += new System.EventHandler(this.cbTraceListColumn_CheckedChanged);
            // 
            // cbTime
            // 
            this.cbTime.AutoSize = true;
            this.cbTime.Checked = true;
            this.cbTime.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbTime.Location = new System.Drawing.Point(6, 65);
            this.cbTime.Name = "cbTime";
            this.cbTime.Size = new System.Drawing.Size(49, 17);
            this.cbTime.TabIndex = 2;
            this.cbTime.Text = "Time";
            this.cbTime.UseVisualStyleBackColor = true;
            this.cbTime.CheckedChanged += new System.EventHandler(this.cbTraceListColumn_CheckedChanged);
            // 
            // cbType
            // 
            this.cbType.AutoSize = true;
            this.cbType.Checked = true;
            this.cbType.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbType.Location = new System.Drawing.Point(6, 42);
            this.cbType.Name = "cbType";
            this.cbType.Size = new System.Drawing.Size(50, 17);
            this.cbType.TabIndex = 1;
            this.cbType.Text = "Type";
            this.cbType.UseVisualStyleBackColor = true;
            this.cbType.CheckedChanged += new System.EventHandler(this.cbTraceListColumn_CheckedChanged);
            // 
            // cbId
            // 
            this.cbId.AutoSize = true;
            this.cbId.Checked = true;
            this.cbId.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbId.Location = new System.Drawing.Point(6, 19);
            this.cbId.Name = "cbId";
            this.cbId.Size = new System.Drawing.Size(35, 17);
            this.cbId.TabIndex = 0;
            this.cbId.Text = "Id";
            this.cbId.UseVisualStyleBackColor = true;
            this.cbId.CheckedChanged += new System.EventHandler(this.cbTraceListColumn_CheckedChanged);
            // 
            // gbFilterDriver
            // 
            this.gbFilterDriver.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.gbFilterDriver.Controls.Add(this.btnReinstall);
            this.gbFilterDriver.Controls.Add(this.btnUninstall);
            this.gbFilterDriver.Location = new System.Drawing.Point(8, 3);
            this.gbFilterDriver.Name = "gbFilterDriver";
            this.gbFilterDriver.Size = new System.Drawing.Size(361, 51);
            this.gbFilterDriver.TabIndex = 2;
            this.gbFilterDriver.TabStop = false;
            this.gbFilterDriver.Text = "Filter Driver";
            // 
            // btnReinstall
            // 
            this.btnReinstall.Location = new System.Drawing.Point(6, 19);
            this.btnReinstall.Name = "btnReinstall";
            this.btnReinstall.Size = new System.Drawing.Size(102, 23);
            this.btnReinstall.TabIndex = 0;
            this.btnReinstall.Text = "Reinstall";
            this.btnReinstall.UseVisualStyleBackColor = true;
            this.btnReinstall.Click += new System.EventHandler(this.btnReinstall_Click);
            // 
            // btnUninstall
            // 
            this.btnUninstall.Location = new System.Drawing.Point(114, 19);
            this.btnUninstall.Name = "btnUninstall";
            this.btnUninstall.Size = new System.Drawing.Size(102, 23);
            this.btnUninstall.TabIndex = 1;
            this.btnUninstall.Text = "Uninstall";
            this.btnUninstall.UseVisualStyleBackColor = true;
            this.btnUninstall.Click += new System.EventHandler(this.btnUninstall_Click);
            // 
            // tmrTrace
            // 
            this.tmrTrace.Tick += new System.EventHandler(this.tmrTrace_Tick);
            // 
            // tmrDeviceChange
            // 
            this.tmrDeviceChange.Interval = 1000;
            this.tmrDeviceChange.Tick += new System.EventHandler(this.tmrDeviceChange_Tick);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(385, 386);
            this.Controls.Add(this.tabControl);
            this.Name = "MainForm";
            this.Text = "BusDog";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.tabControl.ResumeLayout(false);
            this.tabDevices.ResumeLayout(false);
            this.tabTrace.ResumeLayout(false);
            this.tscTrace.ContentPanel.ResumeLayout(false);
            this.tscTrace.TopToolStripPanel.ResumeLayout(false);
            this.tscTrace.TopToolStripPanel.PerformLayout();
            this.tscTrace.ResumeLayout(false);
            this.tscTrace.PerformLayout();
            this.tsTrace.ResumeLayout(false);
            this.tsTrace.PerformLayout();
            this.tabSetup.ResumeLayout(false);
            this.gbTraceList.ResumeLayout(false);
            this.gbTraceList.PerformLayout();
            this.gbFilterDriver.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl;
        private System.Windows.Forms.TabPage tabDevices;
        private System.Windows.Forms.TabPage tabTrace;
        private System.Windows.Forms.Timer tmrTrace;
        private BufferedListView lvTraces;
        private System.Windows.Forms.ColumnHeader chId;
        private System.Windows.Forms.ColumnHeader chType;
        private System.Windows.Forms.ColumnHeader chTime;
        private System.Windows.Forms.ColumnHeader chLength;
        private System.Windows.Forms.ColumnHeader chHex;
        private System.Windows.Forms.ColumnHeader chAscii;
        private System.Windows.Forms.Timer tmrDeviceChange;
        private System.Windows.Forms.TreeView tvDevices;
        private System.Windows.Forms.ToolStripContainer tscTrace;
        private System.Windows.Forms.ToolStrip tsTrace;
        private System.Windows.Forms.ToolStripButton btnStartTraces;
        private System.Windows.Forms.ToolStripButton btnClearTraces;
        private System.Windows.Forms.TabPage tabSetup;
        private System.Windows.Forms.Button btnUninstall;
        private System.Windows.Forms.Button btnReinstall;
        private System.Windows.Forms.GroupBox gbTraceList;
        private System.Windows.Forms.CheckBox cbTime;
        private System.Windows.Forms.CheckBox cbType;
        private System.Windows.Forms.CheckBox cbId;
        private System.Windows.Forms.GroupBox gbFilterDriver;
        private System.Windows.Forms.CheckBox cbAscii;
        private System.Windows.Forms.CheckBox cbHex;
        private System.Windows.Forms.CheckBox cbLength;
        private FilterControl filterControl;
    }
}

