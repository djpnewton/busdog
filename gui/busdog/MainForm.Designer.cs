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
            this.tabTrace = new System.Windows.Forms.TabPage();
            this.lbDevices = new System.Windows.Forms.CheckedListBox();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.lbTraces = new System.Windows.Forms.ListBox();
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
            // tabTrace
            // 
            this.tabTrace.Controls.Add(this.lbTraces);
            this.tabTrace.Location = new System.Drawing.Point(4, 22);
            this.tabTrace.Name = "tabTrace";
            this.tabTrace.Padding = new System.Windows.Forms.Padding(3);
            this.tabTrace.Size = new System.Drawing.Size(284, 240);
            this.tabTrace.TabIndex = 1;
            this.tabTrace.Text = "Trace";
            this.tabTrace.UseVisualStyleBackColor = true;
            // 
            // lbDevices
            // 
            this.lbDevices.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lbDevices.FormattingEnabled = true;
            this.lbDevices.Location = new System.Drawing.Point(3, 3);
            this.lbDevices.Name = "lbDevices";
            this.lbDevices.Size = new System.Drawing.Size(278, 229);
            this.lbDevices.TabIndex = 0;
            this.lbDevices.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.lbDevices_ItemCheck);
            // 
            // timer1
            // 
            this.timer1.Enabled = true;
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // lbTraces
            // 
            this.lbTraces.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lbTraces.FormattingEnabled = true;
            this.lbTraces.Location = new System.Drawing.Point(3, 3);
            this.lbTraces.Name = "lbTraces";
            this.lbTraces.Size = new System.Drawing.Size(278, 225);
            this.lbTraces.TabIndex = 0;
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
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.ListBox lbTraces;
    }
}

