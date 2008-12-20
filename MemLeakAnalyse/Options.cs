using System;
using System.Xml.Serialization;

namespace MemLeakAnalyse
{
	/// <summary>
	/// Summary description for Options.
	/// </summary>
	public class Options
	{
    public Options()
    {
    }

    public Options(Options o)
    {
      Set(o);
    }

    public void Set(Options o)
    {
      this.lastFile = o.lastFile;

      this.lookupDirectories = new string[o.lookupDirectories.Length];
      for(int i=0; i<o.lookupDirectories.Length; i++)
      {
        this.lookupDirectories[i] = o.lookupDirectories[i];
      }
    }

    private string[] lookupDirectories = new string[0];
    public string[] LookupDirectories
    {
      get
      {
        return lookupDirectories;
      }
      set
      {
        lookupDirectories = value;
      }
    }

    private string lastFile = null;
    public string LastFile
    {
      get
      {
        return lastFile;
      }
      set
      {
        lastFile = value;
      }
    }

    // static methods:
    public static void Save(Options o, string Filename)
    {
      XmlSerializer s = new XmlSerializer(typeof(Options));
      System.IO.TextWriter tw = new System.IO.StreamWriter(Filename);
      s.Serialize(tw, o);
      tw.Close();
    }
    public static Options Load(string Filename)
    {
      if (System.IO.File.Exists(Filename) == false)
        return new Options();
      try
      {
        using(System.IO.TextReader tr = new System.IO.StreamReader(Filename))
        {
          XmlSerializer s = new XmlSerializer(typeof(Options));
          Options o = (Options) s.Deserialize(tr);
          tr.Close();
          return o;
        }
      }
      catch(System.Exception)
      {
      }
      return new Options();
    }
  }
}
