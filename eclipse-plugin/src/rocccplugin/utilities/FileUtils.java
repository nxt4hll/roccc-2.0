package rocccplugin.utilities;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;

public class FileUtils 
{

	public static boolean createFolder(String path)
	{
		try 
		{
			File folder = new File(path);
			if(folder.exists())
				return true;
			 
			return folder.mkdir();
		}
		catch (Exception e) 
		{
			e.printStackTrace();
		}
		
		return false;
	} 
	 
	public static boolean createFolderWithOverwrite(String path)
	{
		//If the folder exists, delete it first.
		File folder = new File(path);
		if(folder.exists())
			folder.delete();
		
		//Create the folder.
		return createFolder(path);
	}
	
	/*public static void copyFileToLocation(File fileToCopy, String pasteLocation)
	{
		if(fileToCopy.exists() == false)
		{
			MessageUtils.printlnConsoleError("Error: File " + fileToCopy.getName() + " does not exist. Cannot copy.");
			return;
		}
		
		StringBuffer buf = new StringBuffer();
		try
		{
			FileInputStream fis = new FileInputStream(fileToCopy.getAbsolutePath());
			InputStreamReader in = new InputStreamReader(fis, "UTF-8");
		
			while(in.ready())
			{
				buf.append((char) in.read());
			}
			in.close();
		} 
		catch (IOException e) 
		{
			e.printStackTrace();
			return;
		}
		finally 
		{
		}
		
		String fileFolder = selectedFile.getProjectRelativePath().toString().replace(selectedFile.getName(), "");
		IFile theFile = ResourcesPlugin.getWorkspace().getRoot().getProject(selectedFile.getProject().getName()).getFile(fileFolder + pasteLocation);
		
		//Actually create the source file.
		try
		{
			ByteArrayInputStream Bis1 = new ByteArrayInputStream(buf.toString().getBytes("UTF-8"));
			File fullFilePath = new File(theFile.getRawLocation().toOSString());
			if(theFile.exists())
				theFile.delete(true, true, null);
			if(fullFilePath.exists())
				fullFilePath.delete();
			theFile.create(Bis1, false, null);
		}
		catch(java.lang.Exception e)
		{
			e.printStackTrace();
			MessageUtils.openErrorWindow("Error", "There was an error creating your file.");
			return;
		}
	}*/
	
	public static String getFolderOfFile(File file)
	{
		if(file == null)
			return null;
		
		return file.getAbsolutePath().replace(file.getName(), "");
	}
	
	public static void copyFile(File srcFile, String dstPath)
	{
		try
		{
			if(srcFile.isFile() && srcFile.exists())
			{ 
		       InputStream in = new FileInputStream(srcFile.getAbsoluteFile());
		       OutputStream out = new FileOutputStream(dstPath); 
		       byte[] buf = new byte[1024];
		       int len;
		 
		       while ((len = in.read(buf)) > 0) 
		          out.write(buf, 0, len);
		 
		       in.close(); 
		       out.close();
			}	
			else
			{
				if(!srcFile.exists())
					MessageUtils.printlnConsoleError("Error: File " + srcFile.getName() + " does not exist. Cannot copy.");
				else if(srcFile.isDirectory())
					MessageUtils.printlnConsoleError("Error: " + srcFile.getName() + " is a directory. Cannot copy as a file.");
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static void copyDirectory(File srcPath, File dstPath)
	{
		try
		{
			//If the source 
			if (srcPath.isDirectory())
			{
				if (!dstPath.exists())
					dstPath.mkdir();
				
				String files[] = srcPath.list();
			  
			    for(int i = 0; i < files.length; i++)
			        copyDirectory(new File(srcPath, files[i]), new File(dstPath, files[i])); 
			}
			else
			{ 
				if(srcPath.exists())
				{
			       InputStream in = new FileInputStream(srcPath);
			       OutputStream out = new FileOutputStream(dstPath); 
			       byte[] buf = new byte[1024];
			       int len;
			 
			       while ((len = in.read(buf)) > 0)
			          out.write(buf, 0, len);
			 
			       in.close();
			       out.close();
			    }
			}	
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static boolean createFileFromBuffer(StringBuffer buffer, File optloFile)
	{
		try
		{	
			FileWriter fstream = new FileWriter(optloFile.getAbsolutePath());
	        BufferedWriter out = new BufferedWriter(fstream);
	        out.write(buffer.toString());
	        out.close();
	        return true;
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return false;
	}
	
	public static void addFileContentsToBuffer(StringBuffer buffer, String file)
	{
		try
		{
			FileInputStream fis = new FileInputStream(file);
			InputStreamReader in = new InputStreamReader(fis, "UTF-8");
		
			while(in.ready())
				buffer.append((char) in.read());
			
			in.close();
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
			return;
		}
	}
	
}
