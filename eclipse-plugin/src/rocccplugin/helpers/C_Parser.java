package rocccplugin.helpers;

import java.io.File;
import java.util.Vector;

import rocccplugin.utilities.FileUtils;

public class C_Parser 
{
	public static String[] findAllLabels(String filename)
	{
		Vector<String> labels = new Vector<String>();
		
		//See if the source file exists
		File cFile = new File(filename);
		if(!cFile.exists())
			return null;
	
		/*String processedFile = Activator.getDistributionFolder() + "/processedFile.c";
		try
		{
			Process p = Runtime.getRuntime().exec(Activator.getDistributionFolder() + "/Install/buildOf4.0/gcc/cpp " + filename + " " + processedFile);
			while(!Activator.isProcessDone(p));
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}*/
		
		//Get the content from the file
		StringBuffer buffer = new StringBuffer();
		FileUtils.addFileContentsToBuffer(buffer, filename);
		
		//Get rid of all the useless characters
		String sourceText = buffer.toString();
		String[] values = sourceText.split("[ ,;\n\t]+");
		
		//Find all potential labels
		for(int i = 0; i < values.length; ++i)
		{
			if(values[i].contains(":") && values[i].length() > 1)
			{
				labels.add(values[i].split(":")[0]);
			}
			else if(values[i].endsWith(":"))
			{
				labels.add(values[i - 1]);
			}
		}
		
		//Convert them to a vector
		String[] result = new String[0];
		result = labels.toArray(result);
		return result;
	}
}
