using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace MemLeakAnalyse
{
	/// <summary>
	/// Summary description for OptionsDialog.
	/// </summary>
	public class OptionsDialog : System.Windows.Forms.Form
	{
    private System.Windows.Forms.Button cmdCancel;
    private System.Windows.Forms.Button cmdOk;
    private System.Windows.Forms.Label label1;
    private System.Windows.Forms.ListBox lstSearchPath;
    private System.Windows.Forms.Button cmdSearchPathAdd;
    private System.Windows.Forms.Button cmdSearchPathEdit;
    private System.Windows.Forms.Button cmdSearchPathDelete;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
    private System.Windows.Forms.Button cmdSearchPathUp;
    private System.Windows.Forms.Button cmdSearchPathDown;

    private Options m_org;
    private Options m_o;  // working version...


		public OptionsDialog(Options o)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

      m_org = o;
      m_o = new Options(m_org);

      FillOptionsDialog();

      lstSearchPath_SelectedIndexChanged(null, null);
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
      this.cmdCancel = new System.Windows.Forms.Button();
      this.cmdOk = new System.Windows.Forms.Button();
      this.label1 = new System.Windows.Forms.Label();
      this.lstSearchPath = new System.Windows.Forms.ListBox();
      this.cmdSearchPathAdd = new System.Windows.Forms.Button();
      this.cmdSearchPathEdit = new System.Windows.Forms.Button();
      this.cmdSearchPathDelete = new System.Windows.Forms.Button();
      this.cmdSearchPathUp = new System.Windows.Forms.Button();
      this.cmdSearchPathDown = new System.Windows.Forms.Button();
      this.SuspendLayout();
      // 
      // cmdCancel
      // 
      this.cmdCancel.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
      this.cmdCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
      this.cmdCancel.Location = new System.Drawing.Point(420, 156);
      this.cmdCancel.Name = "cmdCancel";
      this.cmdCancel.TabIndex = 6;
      this.cmdCancel.Text = "&Cancel";
      this.cmdCancel.Click += new System.EventHandler(this.cmdCancel_Click);
      // 
      // cmdOk
      // 
      this.cmdOk.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
      this.cmdOk.Location = new System.Drawing.Point(332, 156);
      this.cmdOk.Name = "cmdOk";
      this.cmdOk.TabIndex = 5;
      this.cmdOk.Text = "&Ok";
      this.cmdOk.Click += new System.EventHandler(this.cmdOk_Click);
      // 
      // label1
      // 
      this.label1.Location = new System.Drawing.Point(8, 8);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(100, 16);
      this.label1.TabIndex = 0;
      this.label1.Text = "File search path:";
      // 
      // lstSearchPath
      // 
      this.lstSearchPath.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
        | System.Windows.Forms.AnchorStyles.Left) 
        | System.Windows.Forms.AnchorStyles.Right);
      this.lstSearchPath.Location = new System.Drawing.Point(8, 32);
      this.lstSearchPath.Name = "lstSearchPath";
      this.lstSearchPath.Size = new System.Drawing.Size(376, 108);
      this.lstSearchPath.TabIndex = 1;
      this.lstSearchPath.SelectedIndexChanged += new System.EventHandler(this.lstSearchPath_SelectedIndexChanged);
      // 
      // cmdSearchPathAdd
      // 
      this.cmdSearchPathAdd.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
      this.cmdSearchPathAdd.Location = new System.Drawing.Point(424, 32);
      this.cmdSearchPathAdd.Name = "cmdSearchPathAdd";
      this.cmdSearchPathAdd.TabIndex = 2;
      this.cmdSearchPathAdd.Text = "&Add";
      this.cmdSearchPathAdd.Click += new System.EventHandler(this.cmdSearchPathAdd_Click);
      // 
      // cmdSearchPathEdit
      // 
      this.cmdSearchPathEdit.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
      this.cmdSearchPathEdit.Location = new System.Drawing.Point(424, 72);
      this.cmdSearchPathEdit.Name = "cmdSearchPathEdit";
      this.cmdSearchPathEdit.TabIndex = 3;
      this.cmdSearchPathEdit.Text = "&Edit";
      this.cmdSearchPathEdit.Click += new System.EventHandler(this.cmdSearchPathEdit_Click);
      // 
      // cmdSearchPathDelete
      // 
      this.cmdSearchPathDelete.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
      this.cmdSearchPathDelete.Location = new System.Drawing.Point(424, 112);
      this.cmdSearchPathDelete.Name = "cmdSearchPathDelete";
      this.cmdSearchPathDelete.TabIndex = 4;
      this.cmdSearchPathDelete.Text = "&Delete";
      this.cmdSearchPathDelete.Click += new System.EventHandler(this.cmdSearchPathDelete_Click);
      // 
      // cmdSearchPathUp
      // 
      this.cmdSearchPathUp.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
      this.cmdSearchPathUp.Location = new System.Drawing.Point(392, 56);
      this.cmdSearchPathUp.Name = "cmdSearchPathUp";
      this.cmdSearchPathUp.Size = new System.Drawing.Size(24, 23);
      this.cmdSearchPathUp.TabIndex = 7;
      this.cmdSearchPathUp.Text = "Up";
      this.cmdSearchPathUp.Click += new System.EventHandler(this.cmdSearchPathUp_Click);
      // 
      // cmdSearchPathDown
      // 
      this.cmdSearchPathDown.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
      this.cmdSearchPathDown.Location = new System.Drawing.Point(392, 80);
      this.cmdSearchPathDown.Name = "cmdSearchPathDown";
      this.cmdSearchPathDown.Size = new System.Drawing.Size(24, 23);
      this.cmdSearchPathDown.TabIndex = 8;
      this.cmdSearchPathDown.Text = "Down";
      this.cmdSearchPathDown.Click += new System.EventHandler(this.cmdSearchPathDown_Click);
      // 
      // OptionsDialog
      // 
      this.AcceptButton = this.cmdOk;
      this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
      this.CancelButton = this.cmdCancel;
      this.ClientSize = new System.Drawing.Size(504, 189);
      this.ControlBox = false;
      this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                  this.cmdSearchPathDown,
                                                                  this.cmdSearchPathUp,
                                                                  this.cmdSearchPathDelete,
                                                                  this.cmdSearchPathEdit,
                                                                  this.cmdSearchPathAdd,
                                                                  this.lstSearchPath,
                                                                  this.label1,
                                                                  this.cmdOk,
                                                                  this.cmdCancel});
      this.Name = "OptionsDialog";
      this.ShowInTaskbar = false;
      this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
      this.Text = "OptionsDialog";
      this.ResumeLayout(false);

    }
		#endregion

    private void cmdOk_Click(object sender, System.EventArgs e)
    {
      this.DialogResult = DialogResult.OK;

      // übernehme die Daten aus der Liste in meine Optionen
      this.m_o.LookupDirectories = new string[this.lstSearchPath.Items.Count];
      for(int i=0; i<this.lstSearchPath.Items.Count; i++)
      {
        this.m_o.LookupDirectories[i] = this.lstSearchPath.Items[i].ToString();
      }
      this.m_org.Set(this.m_o);
      this.Close();
    }

    private void cmdCancel_Click(object sender, System.EventArgs e)
    {
      this.DialogResult = DialogResult.Cancel;
      this.Close();
    }

    private void FillOptionsDialog()
    {
      this.lstSearchPath.Items.Clear();
      foreach(string s in this.m_o.LookupDirectories)
      {
        this.lstSearchPath.Items.Add(s);
      }
    }

    private void cmdSearchPathAdd_Click(object sender, System.EventArgs e)
    {
      EditSearchPath esp = new EditSearchPath(null);
      if (esp.ShowDialog(this) == DialogResult.OK)
      {
        this.lstSearchPath.Items.Add(esp.SelectedPath);
      }
      esp.Dispose();
      lstSearchPath_SelectedIndexChanged(null, null);
    }

    private void cmdSearchPathEdit_Click(object sender, System.EventArgs e)
    {
      if (this.lstSearchPath.SelectedIndex < 0)
        return;

      EditSearchPath esp = new EditSearchPath(this.lstSearchPath.SelectedItem.ToString());
      if (esp.ShowDialog(this) == DialogResult.OK)
      {
        this.lstSearchPath.SelectedItem = esp.SelectedPath;
      }
      esp.Dispose();
    }

    private void lstSearchPath_SelectedIndexChanged(object sender, System.EventArgs e)
    {
      if (this.lstSearchPath.SelectedIndex < 0)
      {
        this.cmdSearchPathAdd.Enabled = true;
        this.cmdSearchPathEdit.Enabled = false;
        this.cmdSearchPathDelete.Enabled = false;
        this.cmdSearchPathUp.Enabled = false;
        this.cmdSearchPathDown.Enabled = false;
      }
      else
      {
        if (this.lstSearchPath.Items.Count <= 1)
        {
          this.cmdSearchPathUp.Enabled = false;
          this.cmdSearchPathDown.Enabled = false;
        }
        else
        {
          if (this.lstSearchPath.SelectedIndex == 0)
          {
            this.cmdSearchPathUp.Enabled = false;
            this.cmdSearchPathDown.Enabled = true;
          }
          else if(this.lstSearchPath.SelectedIndex == this.lstSearchPath.Items.Count-1)
          {
            this.cmdSearchPathUp.Enabled = true;
            this.cmdSearchPathDown.Enabled = false;
          }
          else
          {
            this.cmdSearchPathUp.Enabled = true;
            this.cmdSearchPathDown.Enabled = true;
          }
        }
        this.cmdSearchPathAdd.Enabled = true;
        this.cmdSearchPathEdit.Enabled = true;
        this.cmdSearchPathDelete.Enabled = true;
      }
    }

    private void cmdSearchPathDelete_Click(object sender, System.EventArgs e)
    {
      if (this.lstSearchPath.SelectedIndex < 0)
        return;

      this.lstSearchPath.Items.RemoveAt(this.lstSearchPath.SelectedIndex);
      lstSearchPath_SelectedIndexChanged(null, null);
    }

    private void cmdSearchPathUp_Click(object sender, System.EventArgs e)
    {
      if (this.lstSearchPath.SelectedIndex <= 0)
        return;

      string tmp = this.lstSearchPath.Items[this.lstSearchPath.SelectedIndex].ToString();
      this.lstSearchPath.Items[this.lstSearchPath.SelectedIndex] = this.lstSearchPath.Items[this.lstSearchPath.SelectedIndex-1];
      this.lstSearchPath.Items[this.lstSearchPath.SelectedIndex-1] = tmp;
      this.lstSearchPath.SelectedIndex = this.lstSearchPath.SelectedIndex-1;
      lstSearchPath_SelectedIndexChanged(null, null);
    }

    private void cmdSearchPathDown_Click(object sender, System.EventArgs e)
    {
      if ( (this.lstSearchPath.SelectedIndex < 0) || (this.lstSearchPath.SelectedIndex >= this.lstSearchPath.Items.Count-1) )
        return;

      string tmp = this.lstSearchPath.Items[this.lstSearchPath.SelectedIndex].ToString();
      this.lstSearchPath.Items[this.lstSearchPath.SelectedIndex] = this.lstSearchPath.Items[this.lstSearchPath.SelectedIndex+1];
      this.lstSearchPath.Items[this.lstSearchPath.SelectedIndex+1] = tmp;
      this.lstSearchPath.SelectedIndex = this.lstSearchPath.SelectedIndex+1;
      lstSearchPath_SelectedIndexChanged(null, null);
    }
	}
}
