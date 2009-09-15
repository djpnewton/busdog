using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace busdog
{
    public enum FilterInclude
    {
        Include = 0,
        Exclude
    }

    public enum FilterType
    {
        Length = 0,
        Hex,
        Ascii
    }

    public enum LengthMatch
    {
        GreaterThen = 0,
        LessThen,
        EqualTo
    }

    public struct FilterMatch
    {
        public FilterType FilterType;
        public LengthMatch LengthMatch;
        public int Length;
        public string Filter;

        public FilterMatch(FilterType filterType, string filter, LengthMatch lengthMatch)
        {
            FilterType = filterType;
            Filter = filter;
            LengthMatch = lengthMatch;
            if (FilterType == FilterType.Length)
                Length = Convert.ToInt32(filter);
            else
                Length = 0;
        }

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();

            switch (FilterType)
            {
                case FilterType.Length:
                    sb.Append("Length is ");
                    switch (LengthMatch)
                    {
                        case LengthMatch.GreaterThen:
                            sb.Append("greater then ");
                            break;
                        case LengthMatch.LessThen:
                            sb.Append("less then ");
                            break;
                        case LengthMatch.EqualTo:
                            sb.Append("equal to ");
                            break;
                    }
                    sb.Append(Filter);
                    break;
                case FilterType.Hex:
                case FilterType.Ascii:
                    sb.Append(Filter);
                    break;
            }

            return sb.ToString();
        }
    }

    public partial class FilterControl : UserControl
    {
        FilterInclude include = FilterInclude.Include;
        public FilterInclude Include
        {
            get { return include; }
            set
            {
                include = value;
                UpdateControls();
            }
        }

        FilterType type = FilterType.Length;
        public FilterType Type
        {
            get { return type; }
            set
            {
                type = value;
                UpdateControls();
            }
        }

        string filter = "";
        public string Filter
        {
            get { return filter; }
            set
            {
                filter = value;
                UpdateControls();
            }
        }

        public List<FilterMatch> IncludeFilters
        {
            get
            {
                List<FilterMatch> filters = new List<FilterMatch>();
                foreach (object f in lbIncludeFilters.Items)
                    filters.Add((FilterMatch)f);
                return filters;
            }
        }

        public List<FilterMatch> ExcludeFilters
        {
            get
            {
                List<FilterMatch> filters = new List<FilterMatch>();
                foreach (object f in lbExcludeFilters.Items)
                    filters.Add((FilterMatch)f);
                return filters;
            }
        }

        LengthMatch lengthMatch = LengthMatch.GreaterThen;
        public LengthMatch LengthMatch_
        {
            get { return lengthMatch; }
            set
            {
                lengthMatch = value;
                UpdateControls();
            }
        }

        public FilterControl()
        {
            InitializeComponent();

            UpdateControls();
        }

        private void UpdateControls()
        {
            SuspendLayout();

            cbInclude.SelectedIndex = (int)include;
            cbType.SelectedIndex = (int)type;
            cbLengthMatch.SelectedIndex = (int)lengthMatch;
            tbFilter.Text = filter;

            int originalTbFilterLeft = tbFilter.Left;
            if (type == FilterType.Length)
            {
                tbFilter.Left = cbLengthMatch.Left + cbLengthMatch.Width + 6;
                cbLengthMatch.Visible = true;
            }
            else
            {
                tbFilter.Left = 6;
                cbLengthMatch.Visible = false;
            }
            tbFilter.Width -= tbFilter.Left - originalTbFilterLeft;

            ResumeLayout(true);
        }

        private void cbInclude_SelectedIndexChanged(object sender, EventArgs e)
        {
            include = (FilterInclude)cbInclude.SelectedIndex;
            UpdateControls();
        }

        private void cbType_SelectedIndexChanged(object sender, EventArgs e)
        {
            type = (FilterType)cbType.SelectedIndex;
            UpdateControls();
        }

        private void cbLengthMatch_SelectedIndexChanged(object sender, EventArgs e)
        {
            lengthMatch = (LengthMatch)cbLengthMatch.SelectedIndex; 
            UpdateControls();
        }

        private void tbFilter_TextChanged(object sender, EventArgs e)
        {
            filter = tbFilter.Text;
            UpdateControls();
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            FilterMatch ft = new FilterMatch(type, filter, lengthMatch);
            if (include == FilterInclude.Include)
                lbIncludeFilters.Items.Add(ft);
            else
                lbExcludeFilters.Items.Add(ft);
        }

        private void gbTraceListFilters_SizeChanged(object sender, EventArgs e)
        {
            SuspendLayout();

            int space = Height - lbIncludeFilters.Top - lbExcludeFilters.Margin.Bottom;
            lbIncludeFilters.Height = space / 2 - lbIncludeFilters.Margin.Bottom;
            lbExcludeFilters.Top = lbIncludeFilters.Bottom + lbExcludeFilters.Margin.Top;
            lbExcludeFilters.Height = lbIncludeFilters.Height;

            ResumeLayout(true);
        }

        private void lbIncludeFilters_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Delete)
                DeleteSelectedFilter(lbIncludeFilters);
        }

        private void lbExcludeFilters_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Delete)
                DeleteSelectedFilter(lbExcludeFilters);
        }

        private void DeleteSelectedFilter(ListBox lb)
        {
            if (lb.SelectedIndex >= 0)
            {
                int prevIndex = lb.SelectedIndex;
                lb.Items.RemoveAt(lb.SelectedIndex);
                if (prevIndex > 0)
                    lb.SelectedIndex = prevIndex - 1;
                else if (lb.Items.Count > 0)
                    lb.SelectedIndex = 0;
            }
        }
    }
}
