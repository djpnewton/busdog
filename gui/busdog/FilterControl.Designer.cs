namespace busdog
{
    partial class FilterControl
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.gbTraceListFilters = new System.Windows.Forms.GroupBox();
            this.lbExcludeFilters = new System.Windows.Forms.ListBox();
            this.btnAdd = new System.Windows.Forms.Button();
            this.cbLengthMatch = new System.Windows.Forms.ComboBox();
            this.tbFilter = new System.Windows.Forms.TextBox();
            this.lbIncludeFilters = new System.Windows.Forms.ListBox();
            this.cbType = new System.Windows.Forms.ComboBox();
            this.cbInclude = new System.Windows.Forms.ComboBox();
            this.gbTraceListFilters.SuspendLayout();
            this.SuspendLayout();
            // 
            // gbTraceListFilters
            // 
            this.gbTraceListFilters.Controls.Add(this.lbExcludeFilters);
            this.gbTraceListFilters.Controls.Add(this.btnAdd);
            this.gbTraceListFilters.Controls.Add(this.cbLengthMatch);
            this.gbTraceListFilters.Controls.Add(this.tbFilter);
            this.gbTraceListFilters.Controls.Add(this.lbIncludeFilters);
            this.gbTraceListFilters.Controls.Add(this.cbType);
            this.gbTraceListFilters.Controls.Add(this.cbInclude);
            this.gbTraceListFilters.Dock = System.Windows.Forms.DockStyle.Fill;
            this.gbTraceListFilters.Location = new System.Drawing.Point(0, 0);
            this.gbTraceListFilters.Name = "gbTraceListFilters";
            this.gbTraceListFilters.Size = new System.Drawing.Size(374, 225);
            this.gbTraceListFilters.TabIndex = 6;
            this.gbTraceListFilters.TabStop = false;
            this.gbTraceListFilters.Text = "Trace List Filters";
            this.gbTraceListFilters.SizeChanged += new System.EventHandler(this.gbTraceListFilters_SizeChanged);
            // 
            // lbExcludeFilters
            // 
            this.lbExcludeFilters.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.lbExcludeFilters.FormattingEnabled = true;
            this.lbExcludeFilters.Location = new System.Drawing.Point(7, 150);
            this.lbExcludeFilters.Name = "lbExcludeFilters";
            this.lbExcludeFilters.Size = new System.Drawing.Size(361, 69);
            this.lbExcludeFilters.TabIndex = 6;
            this.lbExcludeFilters.KeyDown += new System.Windows.Forms.KeyEventHandler(this.lbExcludeFilters_KeyDown);
            // 
            // btnAdd
            // 
            this.btnAdd.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnAdd.Location = new System.Drawing.Point(321, 44);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(47, 23);
            this.btnAdd.TabIndex = 5;
            this.btnAdd.Text = "Add";
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // cbLengthMatch
            // 
            this.cbLengthMatch.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbLengthMatch.FormattingEnabled = true;
            this.cbLengthMatch.Items.AddRange(new object[] {
            "Greater then:",
            "Less then:",
            "Equal to:"});
            this.cbLengthMatch.Location = new System.Drawing.Point(6, 45);
            this.cbLengthMatch.Name = "cbLengthMatch";
            this.cbLengthMatch.Size = new System.Drawing.Size(91, 21);
            this.cbLengthMatch.TabIndex = 4;
            this.cbLengthMatch.SelectedIndexChanged += new System.EventHandler(this.cbLengthMatch_SelectedIndexChanged);
            // 
            // tbFilter
            // 
            this.tbFilter.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbFilter.Location = new System.Drawing.Point(6, 46);
            this.tbFilter.Name = "tbFilter";
            this.tbFilter.Size = new System.Drawing.Size(309, 20);
            this.tbFilter.TabIndex = 3;
            this.tbFilter.TextChanged += new System.EventHandler(this.tbFilter_TextChanged);
            // 
            // lbIncludeFilters
            // 
            this.lbIncludeFilters.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.lbIncludeFilters.FormattingEnabled = true;
            this.lbIncludeFilters.Location = new System.Drawing.Point(7, 75);
            this.lbIncludeFilters.Name = "lbIncludeFilters";
            this.lbIncludeFilters.Size = new System.Drawing.Size(361, 69);
            this.lbIncludeFilters.TabIndex = 2;
            this.lbIncludeFilters.KeyDown += new System.Windows.Forms.KeyEventHandler(this.lbIncludeFilters_KeyDown);
            // 
            // cbType
            // 
            this.cbType.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.cbType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbType.FormattingEnabled = true;
            this.cbType.Items.AddRange(new object[] {
            "Length",
            "Hex",
            "Ascii"});
            this.cbType.Location = new System.Drawing.Point(103, 19);
            this.cbType.Name = "cbType";
            this.cbType.Size = new System.Drawing.Size(265, 21);
            this.cbType.TabIndex = 1;
            this.cbType.SelectedIndexChanged += new System.EventHandler(this.cbType_SelectedIndexChanged);
            // 
            // cbInclude
            // 
            this.cbInclude.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbInclude.FormattingEnabled = true;
            this.cbInclude.Items.AddRange(new object[] {
            "Include",
            "Exclude"});
            this.cbInclude.Location = new System.Drawing.Point(6, 19);
            this.cbInclude.Name = "cbInclude";
            this.cbInclude.Size = new System.Drawing.Size(91, 21);
            this.cbInclude.TabIndex = 0;
            this.cbInclude.SelectedIndexChanged += new System.EventHandler(this.cbInclude_SelectedIndexChanged);
            // 
            // FilterControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.gbTraceListFilters);
            this.Name = "FilterControl";
            this.Size = new System.Drawing.Size(374, 225);
            this.gbTraceListFilters.ResumeLayout(false);
            this.gbTraceListFilters.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox gbTraceListFilters;
        private System.Windows.Forms.ComboBox cbType;
        private System.Windows.Forms.ComboBox cbInclude;
        private System.Windows.Forms.ListBox lbIncludeFilters;
        private System.Windows.Forms.TextBox tbFilter;
        private System.Windows.Forms.ComboBox cbLengthMatch;
        private System.Windows.Forms.Button btnAdd;
        private System.Windows.Forms.ListBox lbExcludeFilters;

    }
}
