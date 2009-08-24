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
            this.lvTraces = new System.Windows.Forms.ListView();
            this.chId = new System.Windows.Forms.ColumnHeader();
            this.chType = new System.Windows.Forms.ColumnHeader();
            this.chTime = new System.Windows.Forms.ColumnHeader();
            this.chLength = new System.Windows.Forms.ColumnHeader();
            this.chHex = new System.Windows.Forms.ColumnHeader();
            this.chAscii = new System.Windows.Forms.ColumnHeader();
            this.tsTrace = new System.Windows.Forms.ToolStrip();
            this.btnStartTraces = new System.Windows.Forms.ToolStripButton();
            this.btnClearTraces = new System.Windows.Forms.ToolStripButton();
            this.tmrTrace = new System.Windows.Forms.Timer(this.components);
            this.tmrDeviceChange = new System.Windows.Forms.Timer(this.components);
            this.tabControl.SuspendLayout();
            this.tabDevices.SuspendLayout();
            this.tabTrace.SuspendLayout();
            this.tscTrace.ContentPanel.SuspendLayout();
            this.tscTrace.TopToolStripPanel.SuspendLayout();
            this.tscTrace.SuspendLayout();
            this.tsTrace.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl
            // 
            this.tabControl.Controls.Add(this.tabDevices);
            this.tabControl.Controls.Add(this.tabTrace);
            this.tabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl.Location = new System.Drawing.Point(0, 0);
            this.tabControl.Name = "tabControl";
            this.tabControl.SelectedIndex = 0;
            this.tabControl.Size = new System.Drawing.Size(292, 266);
            this.tabControl.TabIndex = 0;
            // 
            // tabDevices
            // 
            this.tabDevices.Controls.Add(this.tvDevices);
            this.tabDevices.Location = new System.Drawing.Point(4, 22);
            this.tabDevices.Name = "tabDevices";
            this.tabDevices.Padding = new System.Windows.Forms.Padding(3);
            this.tabDevices.Size = new System.Drawing.Size(284, 240);
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
            this.tvDevices.Size = new System.Drawing.Size(278, 234);
            this.tvDevices.TabIndex = 1;
            this.tvDevices.AfterCheck += new System.Windows.Forms.TreeViewEventHandler(this.tvDevices_AfterCheck);
            // 
            // tabTrace
            // 
            this.tabTrace.Controls.Add(this.tscTrace);
            this.tabTrace.Location = new System.Drawing.Point(4, 22);
            this.tabTrace.Name = "tabTrace";
            this.tabTrace.Padding = new System.Windows.Forms.Padding(3);
            this.tabTrace.Size = new System.Drawing.Size(284, 240);
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
            this.tscTrace.ContentPanel.Size = new System.Drawing.Size(278, 209);
            this.tscTrace.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tscTrace.Location = new System.Drawing.Point(3, 3);
            this.tscTrace.Name = "tscTrace";
            this.tscTrace.Size = new System.Drawing.Size(278, 234);
            this.tscTrace.TabIndex = 1;
            this.tscTrace.Text = "toolStripContainer1";
            // 
            // tscTrace.TopToolStripPanel
            // 
            this.tscTrace.TopToolStripPanel.Controls.Add(this.tsTrace);
            // 
            // lvTraces
            // 
            this.lvTraces.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.chId,
            this.chType,
            this.chTime,
            this.chLength,
            this.chHex,
            this.chAscii});
            this.lvTraces.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lvTraces.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lvTraces.Location = new System.Drawing.Point(0, 0);
            this.lvTraces.Name = "lvTraces";
            this.lvTraces.Size = new System.Drawing.Size(278, 209);
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
            this.ClientSize = new System.Drawing.Size(292, 266);
            this.Controls.Add(this.tabControl);
            this.Name = "MainForm";
            this.Text = "BusDog";
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
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl;
        private System.Windows.Forms.TabPage tabDevices;
        private System.Windows.Forms.TabPage tabTrace;
        private System.Windows.Forms.Timer tmrTrace;
        private System.Windows.Forms.ListView lvTraces;
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
    }
}

