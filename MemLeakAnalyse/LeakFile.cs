/*
 * Project: 
 *   MemLeakAnalyse
 * 
 * File:
 *   LeakFile.cs
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
using System.Collections;
using System.Xml;
using System.Xml.Serialization;

namespace MemLeakAnalyse
{
  public class CommandLineArgs
  {
    [Utilities.DefaultCommandLineArgument(Utilities.CommandLineArgumentType.AtMostOnce)]
    public string file;
  }

	/// <summary>
	/// Summary description for LeakFile.
	/// </summary>
	public class LeakFile
	{
		public LeakFile()
		{
			//
			// TODO: Add constructor logic here
			//
		}

		private string date;
		public string Date
		{
			get
			{
				return date;
			}
		}

		private string time;
		public string Time
		{
			get
			{
				return time;
			}
		}

		public ArrayList leaks = new ArrayList();


		public void ReadXMLFile(string xmlFile)
		{
			XmlTextReader rdr = new XmlTextReader(xmlFile);
			while (rdr.Read())
			{
				if (rdr.NodeType == XmlNodeType.Element)
				{
					if (rdr.Name == "MEMREPORT")
					{
						// lese die Attribute 
						try
						{
							time = rdr.GetAttribute("time");
							date = rdr.GetAttribute("date");
						}
						catch(Exception) { }
					}
					if (rdr.IsEmptyElement == false)
					{
						// lese jetzt hier die einzelnen leaks ein
						while(rdr.Read())
						{
							if (rdr.NodeType == XmlNodeType.Element)
							{
								if (rdr.Name == "LEAK")
								{
									// so, jetzt hab ich ein Leak... lese also den Callstack ein...
									try
									{
										uint requestID = Convert.ToUInt32(rdr.GetAttribute("requestID"));
										uint size = Convert.ToUInt32(rdr.GetAttribute("size"));
										Leak l = new Leak(requestID, size);
										this.leaks.Add(l);

										if (rdr.IsEmptyElement == false)
										{
											ReadLeak(rdr, l);
										}
									}
									catch(Exception) { }
								}
							}
						}  // while.Read
					}
				}
			}
			rdr.Close();
		}  // ReadXMLFile

		void ReadLeak(XmlReader rdr, Leak l)
		{
			// lese jetzt den Callstack dieses Leaks ein
			while(rdr.Read())
			{
				if (rdr.NodeType == XmlNodeType.Element)
				{
					if (rdr.Name == "STACKENTRY")
					{
						// jetzt hab ich einen Stack-Entry... lese ihn ein und füge diesen dem Leak hinzu
						try
						{
							StackEntry se = new StackEntry(
								rdr.GetAttribute("decl"),
								rdr.GetAttribute("decl_offset"),
								rdr.GetAttribute("srcfile"),
								Convert.ToUInt32(rdr.GetAttribute("line")),
								rdr.GetAttribute("line_offset"),
								rdr.GetAttribute("module"),
								rdr.GetAttribute("base"));
							l.Stack.Add(se);
						}
						catch(Exception) {}
					}
				}
				if (rdr.NodeType == XmlNodeType.EndElement)
					if (rdr.Name == "LEAK")
						break;
			}
		}  // ReadLeak
	}  // LeakFile

	public class Leak
	{
		public Leak(uint requestID, uint size)
		{
			this.requestID = requestID;
			this.size = size;
		}

		private uint requestID;
		public uint RequestID
		{
			get
			{
				return requestID;
			}
		}

		private uint size;
		public uint Size
		{
			get
			{
				return size;
			}
		}

		public ArrayList Stack = new ArrayList();
	}  // Leak

	public class StackEntry
	{
		public StackEntry(string decl, string decl_offset, string srcfile, uint line, string lineoffset, string module, string base_str)
		{
			this.base_str = base_str;
			this.decl = decl;
			this.decl_offset = decl_offset;
			this.line = line;
			this.line_offset = line_offset;
			this.module = module;
			this.srcfile = srcfile;
		}

		private string decl;
		public string Decl
		{
			get
			{
				return decl;
			}
		}

		private string decl_offset;
		public string DeclOffset
		{
			get
			{
				return decl_offset;
			}
		}

		private string srcfile;
		public string SrcFile
		{
			get
			{
				return srcfile;
			}
		}

		private uint line;
		public uint Line
		{
			get
			{
				return line;
			}
		}

		private string line_offset;
		public string LineOffset
		{
			get
			{
				return line_offset;
			}
		}

		private string module;
		public string Module
		{
			get
			{
				return module;
			}
		}

		private string base_str;
		public string Base
		{
			get
			{
				return base_str;
			}
		}

		// die folgenden Werte sind für die Anzeige

		private ulong leakSize;
		public ulong LeakSize
		{
			get
			{
				return leakSize;
			}
			set
			{
				leakSize = value;
			}
		}

		private ulong count = 1;
		public ulong Count
		{
			get
			{
				return count;
			}
			set
			{
				count = value;
			}
				
		}


	}
}  // namespace MemLeakAnalyse
