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
            this.tabControl = new System.Windows.Forms.TabControl();
            this.tabSetup = new System.Windows.Forms.TabPage();
            this.lbDevices = new System.Windows.Forms.CheckedListBox();
            this.tabTrace = new System.Windows.Forms.TabPage();
            this.tmrTrace = new System.Windows.Forms.Timer(this.components);
            this.lvTraces = new System.Windows.Forms.ListView();
            this.chId = new System.Windows.Forms.ColumnHeader();
            this.chType = new System.Windows.Forms.ColumnHeader();
            this.chTimestamp = new System.Windows.Forms.ColumnHeader();
            this.chLength = new System.Windows.Forms.ColumnHeader();
            this.chHex = new System.Windows.Forms.ColumnHeader();
            this.chAscii = new System.Windows.Forms.ColumnHeader();
            this.tmrDeviceChange = new System.Windows.Forms.Timer(this.components);
            this.tabControl.SuspendLayout();
            this.tabSetup.SuspendLayout();
            this.tabTrace.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl
            // 
            this.tabControl.Controls.Add(this.tabSetup);
            this.tabControl.Controls.Add(this.tabTrace);
            this.tabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl.Location = new System.Drawing.Point(0, 0);
            this.tabControl.Name = "tabControl";
            this.tabControl.SelectedIndex = 0;
            this.tabControl.Size = new System.Drawing.Size(292, 266);
            this.tabControl.TabIndex = 0;
            // 
            // tabSetup
            // 
            this.tabSetup.Controls.Add(this.lbDevices);
            this.tabSetup.Location = new System.Drawing.Point(4, 22);
            this.tabSetup.Name = "tabSetup";
            this.tabSetup.Padding = new System.Windows.Forms.Padding(3);
            this.tabSetup.Size = new System.Drawing.Size(284, 240);
            this.tabSetup.TabIndex = 0;
            this.tabSetup.Text = "Setup";
            this.tabSetup.UseVisualStyleBackColor = true;
            // 
            // lbDevices
            // 
            this.lbDevices.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lbDevices.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lbDevices.FormattingEnabled = true;
            this.lbDevices.Location = new System.Drawing.Point(3, 3);
            this.lbDevices.Name = "lbDevices";
            this.lbDevices.Size = new System.Drawing.Size(278, 229);
            this.lbDevices.TabIndex = 0;
            this.lbDevices.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.lbDevices_ItemCheck);
            // 
            // tabTrace
            // 
            this.tabTrace.Controls.Add(this.lvTraces);
            this.tabTrace.Location = new System.Drawing.Point(4, 22);
            this.tabTrace.Name = "tabTrace";
            this.tabTrace.Padding = new System.Windows.Forms.Padding(3);
            this.tabTrace.Size = new System.Drawing.Size(284, 240);
            this.tabTrace.TabIndex = 1;
            this.tabTrace.Text = "Trace";
            this.tabTrace.UseVisualStyleBackColor = true;
            // 
            // tmrTrace
            // 
            this.tmrTrace.Enabled = true;
            this.tmrTrace.Tick += new System.EventHandler(this.tmrTrace_Tick);
            // 
            // lvTraces
            // 
            this.lvTraces.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.chId,
            this.chType,
            this.chTimestamp,
            this.chLength,
            this.chHex,
            this.chAscii});
            this.lvTraces.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lvTraces.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lvTraces.Location = new System.Drawing.Point(3, 3);
            this.lvTraces.Name = "lvTraces";
            this.lvTraces.Size = new System.Drawing.Size(278, 234);
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
            // chTimestamp
            // 
            this.chTimestamp.Text = "Timestamp";
            this.chTimestamp.Width = 79;
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
            this.tabSetup.ResumeLayout(false);
            this.tabTrace.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl;
        private System.Windows.Forms.TabPage tabSetup;
        private System.Windows.Forms.TabPage tabTrace;
        private System.Windows.Forms.CheckedListBox lbDevices;
        private System.Windows.Forms.Timer tmrTrace;
        private System.Windows.Forms.ListView lvTraces;
        private System.Windows.Forms.ColumnHeader chId;
        private System.Windows.Forms.ColumnHeader chType;
        private System.Windows.Forms.ColumnHeader chTimestamp;
        private System.Windows.Forms.ColumnHeader chLength;
        private System.Windows.Forms.ColumnHeader chHex;
        private System.Windows.Forms.ColumnHeader chAscii;
        private System.Windows.Forms.Timer tmrDeviceChange;
    }
}

