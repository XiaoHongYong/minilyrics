/*
 * Project: 
 *   MemLeakAnalyse
 * 
 * File:
 *   Form1.cs
 * 
 * Remarks:
 *   Displays the callstack of memory leaks as reported with the "Memory_and_Exception_Trace"
 *
 * Known bugs:
 *   -
 *
 * Author:
 *   Jochen Kalmbach, Germany
 *   (c) 2002-2003 (Freeware)
 *   http://www.codeproject.com/tools/leakfinder.asp
 *
 */
using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Runtime.InteropServices;

namespace MemLeakAnalyse
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
		private static int EM_SETSEL =  0x00B1;
		private System.Windows.Forms.Panel topPanel;
		private System.Windows.Forms.Panel fileNamePanel;
    private System.Windows.Forms.Button cmdOptions;
		private static int EM_LINEINDEX = 0x00BB;

		[DllImportAttribute ("user32.dll")] 
		private static extern int SendMessage(IntPtr hWnd, int Msg, int wParam, int lParam);
		private System.Windows.Forms.RichTextBox richTextBox1;
		private System.Windows.Forms.Label lblFileAndLineInfo;
		private System.Windows.Forms.TreeView treeView1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox txtXMLLeakFile;
		private System.Windows.Forms.Button btnXMLFileSelect;
		private System.Windows.Forms.OpenFileDialog openFileDialog1;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.Splitter splitter1;
		private System.Windows.Forms.Panel panel2;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		private string act_src_file = "";
    private Options opt;

    private string options_file_name;

		public Form1(string fileToLoad)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

      AppDomain cd = AppDomain.CurrentDomain;
      options_file_name = cd.BaseDirectory + System.IO.Path.GetFileNameWithoutExtension(cd.FriendlyName) + ".ini";
      opt = Options.Load(options_file_name);

      if ( (fileToLoad != null) && (fileToLoad.Length > 0) )
      {
        this.txtXMLLeakFile.Text = fileToLoad;
        if (System.IO.File.Exists(this.txtXMLLeakFile.Text) == true)
          button1_Click(null, null);
      }
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
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
      System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(Form1));
      this.treeView1 = new System.Windows.Forms.TreeView();
      this.topPanel = new System.Windows.Forms.Panel();
      this.button1 = new System.Windows.Forms.Button();
      this.btnXMLFileSelect = new System.Windows.Forms.Button();
      this.label1 = new System.Windows.Forms.Label();
      this.txtXMLLeakFile = new System.Windows.Forms.TextBox();
      this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
      this.splitter1 = new System.Windows.Forms.Splitter();
      this.panel2 = new System.Windows.Forms.Panel();
      this.richTextBox1 = new System.Windows.Forms.RichTextBox();
      this.fileNamePanel = new System.Windows.Forms.Panel();
      this.lblFileAndLineInfo = new System.Windows.Forms.Label();
      this.cmdOptions = new System.Windows.Forms.Button();
      this.topPanel.SuspendLayout();
      this.panel2.SuspendLayout();
      this.fileNamePanel.SuspendLayout();
      this.SuspendLayout();
      // 
      // treeView1
      // 
      this.treeView1.Dock = System.Windows.Forms.DockStyle.Left;
      this.treeView1.HideSelection = false;
      this.treeView1.ImageIndex = -1;
      this.treeView1.Location = new System.Drawing.Point(0, 72);
      this.treeView1.Name = "treeView1";
      this.treeView1.SelectedImageIndex = -1;
      this.treeView1.Size = new System.Drawing.Size(224, 453);
      this.treeView1.TabIndex = 1;
      this.treeView1.DoubleClick += new System.EventHandler(this.treeView1_DoubleClick);
      this.treeView1.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeView1_AfterSelect);
      // 
      // topPanel
      // 
      this.topPanel.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                           this.cmdOptions,
                                                                           this.button1,
                                                                           this.btnXMLFileSelect,
                                                                           this.label1,
                                                                           this.txtXMLLeakFile});
      this.topPanel.Dock = System.Windows.Forms.DockStyle.Top;
      this.topPanel.Name = "topPanel";
      this.topPanel.Size = new System.Drawing.Size(720, 72);
      this.topPanel.TabIndex = 0;
      // 
      // button1
      // 
      this.button1.Location = new System.Drawing.Point(336, 24);
      this.button1.Name = "button1";
      this.button1.TabIndex = 3;
      this.button1.Text = "&Read";
      this.button1.Click += new System.EventHandler(this.button1_Click);
      // 
      // btnXMLFileSelect
      // 
      this.btnXMLFileSelect.Location = new System.Drawing.Point(296, 24);
      this.btnXMLFileSelect.Name = "btnXMLFileSelect";
      this.btnXMLFileSelect.Size = new System.Drawing.Size(24, 23);
      this.btnXMLFileSelect.TabIndex = 2;
      this.btnXMLFileSelect.Text = "...";
      this.btnXMLFileSelect.Click += new System.EventHandler(this.btnXMLFileSelect_Click);
      // 
      // label1
      // 
      this.label1.Location = new System.Drawing.Point(8, 8);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(100, 16);
      this.label1.TabIndex = 0;
      this.label1.Text = "&XML-Leak-File";
      // 
      // txtXMLLeakFile
      // 
      this.txtXMLLeakFile.Location = new System.Drawing.Point(8, 24);
      this.txtXMLLeakFile.Name = "txtXMLLeakFile";
      this.txtXMLLeakFile.Size = new System.Drawing.Size(280, 20);
      this.txtXMLLeakFile.TabIndex = 1;
      this.txtXMLLeakFile.Text = "";
      // 
      // openFileDialog1
      // 
      this.openFileDialog1.DefaultExt = "xml";
      this.openFileDialog1.Filter = "XML-Leak file (*.xml-leaks)|*.xml-leaks|All files (*.*)|*.*";
      this.openFileDialog1.RestoreDirectory = true;
      this.openFileDialog1.Title = "Open leak file";
      // 
      // splitter1
      // 
      this.splitter1.Location = new System.Drawing.Point(224, 72);
      this.splitter1.Name = "splitter1";
      this.splitter1.Size = new System.Drawing.Size(3, 453);
      this.splitter1.TabIndex = 2;
      this.splitter1.TabStop = false;
      // 
      // panel2
      // 
      this.panel2.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                         this.richTextBox1,
                                                                         this.fileNamePanel});
      this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
      this.panel2.Location = new System.Drawing.Point(227, 72);
      this.panel2.Name = "panel2";
      this.panel2.Size = new System.Drawing.Size(493, 453);
      this.panel2.TabIndex = 3;
      // 
      // richTextBox1
      // 
      this.richTextBox1.Dock = System.Windows.Forms.DockStyle.Fill;
      this.richTextBox1.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
      this.richTextBox1.HideSelection = false;
      this.richTextBox1.Location = new System.Drawing.Point(0, 32);
      this.richTextBox1.Name = "richTextBox1";
      this.richTextBox1.ReadOnly = true;
      this.richTextBox1.Size = new System.Drawing.Size(493, 421);
      this.richTextBox1.TabIndex = 1;
      this.richTextBox1.Text = "";
      // 
      // fileNamePanel
      // 
      this.fileNamePanel.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                                this.lblFileAndLineInfo});
      this.fileNamePanel.Dock = System.Windows.Forms.DockStyle.Top;
      this.fileNamePanel.Name = "fileNamePanel";
      this.fileNamePanel.Size = new System.Drawing.Size(493, 32);
      this.fileNamePanel.TabIndex = 0;
      // 
      // lblFileAndLineInfo
      // 
      this.lblFileAndLineInfo.Dock = System.Windows.Forms.DockStyle.Fill;
      this.lblFileAndLineInfo.Name = "lblFileAndLineInfo";
      this.lblFileAndLineInfo.Size = new System.Drawing.Size(493, 32);
      this.lblFileAndLineInfo.TabIndex = 0;
      // 
      // cmdOptions
      // 
      this.cmdOptions.Location = new System.Drawing.Point(632, 24);
      this.cmdOptions.Name = "cmdOptions";
      this.cmdOptions.TabIndex = 4;
      this.cmdOptions.Text = "&Options";
      this.cmdOptions.Click += new System.EventHandler(this.cmdOptions_Click);
      // 
      // Form1
      // 
      this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
      this.ClientSize = new System.Drawing.Size(720, 525);
      this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                  this.panel2,
                                                                  this.splitter1,
                                                                  this.treeView1,
                                                                  this.topPanel});
      this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
      this.Name = "Form1";
      this.Text = "Mem-Leak-Analyse";
      this.topPanel.ResumeLayout(false);
      this.panel2.ResumeLayout(false);
      this.fileNamePanel.ResumeLayout(false);
      this.ResumeLayout(false);

    }
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static int Main(string[] args) 
		{
      CommandLineArgs myArgs = new CommandLineArgs();
      if (!Utilities.Utility.ParseCommandLineArguments(args, myArgs))
      {
        // error encountered in arguments. Display usage message
        MessageBox.Show(Utilities.Utility.CommandLineArgumentsUsage(typeof(CommandLineArgs)), "Warning", MessageBoxButtons.OK, MessageBoxIcon.Information);
        return 1;
      }

			Application.Run(new Form1(myArgs.file));
      return 0;
		}

		private void btnXMLFileSelect_Click(object sender, System.EventArgs e)
		{
			if (this.openFileDialog1.ShowDialog(this) == DialogResult.OK)
			{
				this.txtXMLLeakFile.Text = this.openFileDialog1.FileName;
				button1_Click(sender, e);
			}
		}

		private void button1_Click(object sender, System.EventArgs e)
		{
			LeakFile lf = new LeakFile();
			lf.ReadXMLFile(this.txtXMLLeakFile.Text);

			TreeNode tn_root = this.treeView1.Nodes.Add(this.txtXMLLeakFile.Text);
			foreach(Leak l in lf.leaks)
			{
				TreeNode tn = tn_root;
				foreach(StackEntry se in l.Stack)
				{
					if (FindSameNode(se, ref tn) == false)
					{
						tn = tn.Nodes.Add(string.Format("{0} Line: {1}", se.Decl, se.Line.ToString()));
						se.LeakSize = l.Size;
						tn.Tag = se;
					}
					else
					{
						StackEntry se2 = (StackEntry) tn.Tag;
						se2.LeakSize += l.Size;
						se2.Count++;
					}
				}
			}
			// Update Text:
			UpdateNodes(tn_root);

			//SortNodes(tn_root);
		}
		private void UpdateNodes(TreeNode tn)
		{
			foreach(TreeNode t in tn.Nodes)
			{
				UpdateNodes(t);
				t.Text = ((StackEntry)t.Tag).Count.ToString() + " Size: " + ((StackEntry)t.Tag).LeakSize.ToString() + " " + t.Text;
			}
		}
		/*private void SortNodes(TreeNode tn)
		{
			foreach(TreeNode t in tn.Nodes)
			{
				SortNodes(t);
				t.Text = ((StackEntry)t.Tag).Count.ToString() + " Size: " + ((StackEntry)t.Tag).LeakSize.ToString() + " " + t.Text;
			}
		}*/

		private bool FindSameNode(StackEntry se, ref TreeNode tn)
		{
			foreach(TreeNode t in tn.Nodes)
			{
				if (t.Tag == null)
					return false; // nicht gefunden
				if (!(t.Tag is StackEntry))
					return false; // Typ stimmt nicht, also nicht gefunden

				StackEntry se2 = (StackEntry) t.Tag;
				if ( (se.Line == se2.Line) && (se.SrcFile == se2.SrcFile) && (se.Decl == se2.Decl) )
				{
					tn = t;
					return true;
				}
			}
			return false;
		}

    private void treeView1_DoubleClick(object sender, System.EventArgs e)
    {
      TreeNode tn = this.treeView1.SelectedNode;
      if (tn == null)
        return;
      if (tn.Tag == null)
        return;

      StackEntry se = (StackEntry) tn.Tag;
      if (se == null)
        return;

      lblFileAndLineInfo.Text = se.SrcFile + " Line: " + se.Line;

      RichTextBox tb = this.richTextBox1;

      string fn = FindSourceFile(se.SrcFile);
      if (fn != null)
      {
        try
        {
          if (act_src_file != fn)
          {
            lblFileAndLineInfo.Text = fn + " Line: " + se.Line;
            tb.Clear();
            tb.WordWrap = false;
            System.IO.StreamReader sr = new System.IO.StreamReader(fn);
            ArrayList al = new ArrayList();
            uint uiLine = 1;
            while(sr.Peek() > -1)
            {
              string s = uiLine.ToString() + ": " + sr.ReadLine();
              uiLine++;
              al.Add(s);
            }
            sr.Close();
            tb.Lines = (string[]) al.ToArray(typeof(string));

          }
          SelectLine(tb, (int) se.Line);
          act_src_file = se.SrcFile;
        }
        catch(Exception ex) 
        {
          tb.Text = ex.Message;
          act_src_file = "";
        }
      }
      else
      {
        tb.Text = "<file not found>";
        act_src_file = "";
      }
    }

		private void SelectLine(TextBoxBase tb, int line)
		{
			int iCharToStart = Form1.SendMessage(tb.Handle, Form1.EM_LINEINDEX, line-1, 0);
			if (iCharToStart < 0)
				return; // Line number was to high...

			//number of chars up to the next line
		    int iCharToEnd = Form1.SendMessage(tb.Handle, Form1.EM_LINEINDEX, line, 0);
			if (iCharToEnd < 0)
				return; // TODO: It is the last line... do something special...

			Form1.SendMessage(tb.Handle, Form1.EM_SETSEL, iCharToEnd - 1, iCharToStart);
		}

		private void button2_Click(object sender, System.EventArgs e)
		{
			this.richTextBox1.SelectAll();
		}

		private void treeView1_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e)
		{
			treeView1_DoubleClick(null, null);
		
		}

    private void cmdOptions_Click(object sender, System.EventArgs e)
    {
      OptionsDialog od = new OptionsDialog(this.opt);
      if (od.ShowDialog(this) == DialogResult.OK)
      {
        Options.Save(this.opt, options_file_name);
      }
    }

    private string FindSourceFile(string srcName)
    {
      if (System.IO.File.Exists(srcName) == true)
        return srcName;

      // Datei wurde nicht gefunden, also such einfach die Liste der Include-Pfade ab:
      string f = System.IO.Path.GetFileName(srcName);
      foreach(string s in this.opt.LookupDirectories)
      {
        if (s == null)
          continue;
        if (System.IO.File.Exists(s + "\\" + f) == true)
          return s + "\\" + f;
      }

      return null;
    }
	}
}
