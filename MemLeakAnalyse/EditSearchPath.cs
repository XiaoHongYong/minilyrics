using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace MemLeakAnalyse
{
	/// <summary>
	/// Summary description for EditSearchPath.
	/// </summary>
	public class EditSearchPath : System.Windows.Forms.Form
	{
    private System.Windows.Forms.Button cmdSelectFolder;
    private System.Windows.Forms.TextBox txtFolder;
    private System.Windows.Forms.Button cmdCancel;
    private System.Windows.Forms.Button cmdOk;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

    public string SelectedPath = null;

    public EditSearchPath(string selectedPath)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

      SelectedPath = selectedPath;
      if (SelectedPath != null)
        this.txtFolder.Text = SelectedPath;
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
      this.txtFolder = new System.Windows.Forms.TextBox();
      this.cmdSelectFolder = new System.Windows.Forms.Button();
      this.cmdCancel = new System.Windows.Forms.Button();
      this.cmdOk = new System.Windows.Forms.Button();
      this.SuspendLayout();
      // 
      // txtFolder
      // 
      this.txtFolder.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
        | System.Windows.Forms.AnchorStyles.Right);
      this.txtFolder.Location = new System.Drawing.Point(8, 8);
      this.txtFolder.Name = "txtFolder";
      this.txtFolder.Size = new System.Drawing.Size(336, 20);
      this.txtFolder.TabIndex = 0;
      this.txtFolder.Text = "";
      // 
      // cmdSelectFolder
      // 
      this.cmdSelectFolder.Location = new System.Drawing.Point(352, 8);
      this.cmdSelectFolder.Name = "cmdSelectFolder";
      this.cmdSelectFolder.Size = new System.Drawing.Size(24, 23);
      this.cmdSelectFolder.TabIndex = 1;
      this.cmdSelectFolder.Text = "...";
      this.cmdSelectFolder.Click += new System.EventHandler(this.cmdSelectFolder_Click);
      // 
      // cmdCancel
      // 
      this.cmdCancel.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
      this.cmdCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
      this.cmdCancel.Location = new System.Drawing.Point(304, 48);
      this.cmdCancel.Name = "cmdCancel";
      this.cmdCancel.TabIndex = 2;
      this.cmdCancel.Text = "&Cancel";
      this.cmdCancel.Click += new System.EventHandler(this.cmdCancel_Click);
      // 
      // cmdOk
      // 
      this.cmdOk.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
      this.cmdOk.Location = new System.Drawing.Point(216, 48);
      this.cmdOk.Name = "cmdOk";
      this.cmdOk.TabIndex = 3;
      this.cmdOk.Text = "&Ok";
      this.cmdOk.Click += new System.EventHandler(this.cmdOk_Click);
      // 
      // EditSearchPath
      // 
      this.AcceptButton = this.cmdOk;
      this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
      this.CancelButton = this.cmdCancel;
      this.ClientSize = new System.Drawing.Size(384, 85);
      this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                  this.cmdOk,
                                                                  this.cmdCancel,
                                                                  this.cmdSelectFolder,
                                                                  this.txtFolder});
      this.Name = "EditSearchPath";
      this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
      this.Text = "EditSearchPath";
      this.ResumeLayout(false);

    }
		#endregion

    private void cmdSelectFolder_Click(object sender, System.EventArgs e)
    {
      System.Windows.Forms.FolderBrowserDialog fbd = new System.Windows.Forms.FolderBrowserDialog();
      //if (this.txtFolder.Text.Length > 0)
      // TODO: ... (how can I select the "startup-folder" !?
      if (fbd.ShowDialog(this) == DialogResult.OK)
        this.txtFolder.Text = fbd.DirectoryPath;
      fbd.Dispose();
    }

    private void cmdOk_Click(object sender, System.EventArgs e)
    {
      this.DialogResult = DialogResult.OK;
      this.SelectedPath = this.txtFolder.Text;
      this.Close();
    }

    private void cmdCancel_Click(object sender, System.EventArgs e)
    {
      this.DialogResult = DialogResult.Cancel;
      this.Close();
    }
	}
}
