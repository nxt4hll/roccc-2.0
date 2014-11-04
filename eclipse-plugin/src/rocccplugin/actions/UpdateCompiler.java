package rocccplugin.actions;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.Vector;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.osgi.framework.Version;

import rocccplugin.Activator;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.StringUtils;

public class UpdateCompiler 
{
	public static final int NO_UPDATE_AVAILABLE = 0;
	public static final int UPDATE_AVAILABLE = 1;
	public static final int CONNECTION_ERROR = -1;
	public static final int LOCAL_VERSION_ERROR = -2;
	
	static Version localVersion;
	static Version latestVersion;
	
	static Version nextVersion;
	static String nextVersionPatchLink;
	
	static String localPatchDestination;
	
	static Vector<String[]> onlineVersionInfo;
	static boolean patchAll = true;
	
	static Display dis = Display.getDefault();
	
	//final static String compilerInfoURL = "http://www.jacquardcomputing.com/internalTest/current_compiler_version.txt";
	//final static String compilerInfoURL = "http://www.jacquardcomputing.com/downloads/current_compiler_version.txt";
	
	static boolean showCheckingErrors = true;
	static boolean showCheckingMessages = true;
	
	// Added by Jason to handle the patching of binaries.  These have to be constructed on the fly based upon the OS we are on.
	static public String compilerInfoURL()
	{
		String preface = "http://www.jacquardcomputing.com/downloads/" ;
		String os = Activator.getOSString() ;
		
		return preface + os + "_compiler_version.txt" ;
	}
	
	//Set whether or not certain messages are shown.
	static public void showMessages(boolean checkingErrors, boolean checkingMessages)
	{
		showCheckingErrors = checkingErrors;
		showCheckingMessages = checkingMessages;
	}
	
	static public boolean run(Display d)
	{
		dis = d;
			
		//Download the next patch and run it. Keep doing this if we want to fully patch.
		do
		{
			//Find the next version and get where we want to download it to.
			findNextVersion();
			
			Version versionPriorToUpdate = localVersion;
			
			localPatchDestination = Activator.getDistributionFolder() + "/" + nextVersion + "_patch.tar.gz";
		
			//Download, run the patch, and figure out what our new version is.
			if(downloadPatch() == false)
				return false;
			if(runPatch() == false)
				return false;
			if(getCurrentVersion() == false)
				return false;
			
			//If the version did not change after patching, something went wrong and we should quit.
			if(localVersion.equals(versionPriorToUpdate))
			{	
				MessageUtils.printlnConsoleMessage("There appeared to be a problem during update, your distribution folder may be corrupt.");
				MessageUtils.printlnConsoleMessage("Canceling update of compiler.");
				return false;
			}
		}
		while(newVersionAvailable() && patchAll);		
		
		//compileTools();
		
		MessageUtils.printlnConsoleMessage("Update of ROCCC Compiler Complete.");
		
		return true;
	}
	
	static private boolean compileTools()
	{
		try
		{
			
			String executable = Activator.getDistributionFolder() + "/compile.sh";
			
			if(new File(executable).exists() == false)
			{
				MessageUtils.printlnConsoleError("Error: Script " + executable + " does not exist. Cannot run.");
				return false;
			}
			
			String[] cmdArray = new String[3];
			cmdArray[0] = "chmod";
			cmdArray[1] = "700";
			cmdArray[2] = Activator.getDistributionFolder() + "/compile.sh";
			
			Process p = Runtime.getRuntime().exec(cmdArray);
			while(!Activator.isProcessDone(p));
			
			cmdArray = new String[2];
			cmdArray[0] = Activator.getDistributionFolder() + "/compile.sh";
			cmdArray[1] = Activator.getDistributionFolder() + "/";
			
			p = Runtime.getRuntime().exec(cmdArray);
			BufferedReader inputStream = new BufferedReader(new InputStreamReader(p.getInputStream()));
			BufferedReader outputStream = new BufferedReader(new InputStreamReader(p.getErrorStream()));
			
			//Output all the stream data from the script.
			while(inputStream.ready() || !Activator.isProcessDone(p)) 
			{
				if(inputStream.ready())
				{
					String line;
					if((line = inputStream.readLine()) != null)
					{
						MessageUtils.printlnConsoleMessage(line);
					}
				}
			}
			while(outputStream.ready())
			{
				String line;
				if((line = outputStream.readLine()) != null)
				{
					MessageUtils.printlnConsoleError(line);
				}
			}
			
			cmdArray = new String[3];
			cmdArray[0] = "rm";
			cmdArray[1] = "-rf";
			cmdArray[2] = Activator.getDistributionFolder() + "/compile.sh";
			
			p = Runtime.getRuntime().exec(cmdArray);
			while(!Activator.isProcessDone(p));
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
		
		return true;
	}
	
	static private boolean runPatch()
	{
		MessageUtils.printlnConsoleMessage("Patching ROCCC to " + nextVersion.toString() + "...");
		try 
		{
			//Untar the patch file.
			String[] cmdArray = new String[5];
			cmdArray[0] = "tar";
			cmdArray[1] = "-xzvf";
			cmdArray[2] = localPatchDestination;
			cmdArray[3] = "-C";
			cmdArray[4] = Activator.getDistributionFolder() + "/";
			Process p = Runtime.getRuntime().exec(cmdArray);
			while(!Activator.isProcessDone(p));
			
			//Run the script that was untar'ed.
			String executable = Activator.getDistributionFolder() + "/latestPatch.sh";
			
			if(new File(executable).exists() == false)
			{
				MessageUtils.printlnConsoleError("Error: Script " + executable + " does not exist. Cannot run.");
				return false;
			}
			
			cmdArray = new String[3];
			cmdArray[0] = "chmod";
			cmdArray[1] = "700";
			cmdArray[2] = Activator.getDistributionFolder() + "/latestPatch.sh";
			//Change permissions of the script
			p = Runtime.getRuntime().exec(cmdArray);
			while(!Activator.isProcessDone(p));
				
			cmdArray = new String[2];
			
			cmdArray[0] = Activator.getDistributionFolder() + "/latestPatch.sh";
			cmdArray[1] = Activator.getDistributionFolder() + "/";
 			
			p = Runtime.getRuntime().exec(cmdArray);
			BufferedReader inputStream = new BufferedReader(new InputStreamReader(p.getInputStream()));
			BufferedReader outputStream = new BufferedReader(new InputStreamReader(p.getErrorStream()));
			
			//Output all the stream data from the script.
			while(inputStream.ready() || !Activator.isProcessDone(p)) 
			{
				if(inputStream.ready())
				{
					String line;
					if((line = inputStream.readLine()) != null)
					{
						MessageUtils.printlnConsoleMessage(line);
					}
				}
			}
			while(outputStream.ready())
			{
				String line;
				if((line = outputStream.readLine()) != null)
				{
					MessageUtils.printlnConsoleMessage(line);
				}
			}
			
			
			inputStream.close();
			
			// Clean up the files we downloaded 
			cmdArray = new String[3] ;
			cmdArray[0] = "rm" ;
			cmdArray[1] = "-rf" ;
			cmdArray[2] = Activator.getDistributionFolder() + "/latestPatch.sh" ;
			p = Runtime.getRuntime().exec(cmdArray) ;
			while(!Activator.isProcessDone(p)) ;
			
			cmdArray = new String[3];
			cmdArray[0] = "rm";
			cmdArray[1] = "-rf";
			cmdArray[2] = localPatchDestination;
			p = Runtime.getRuntime().exec(cmdArray);
			while(!Activator.isProcessDone(p));
			
			// Should this be here?
			if(p.exitValue() == 1)
			{
				dis.asyncExec(new Runnable()
				{
					public void run() 
					{
						MessageDialog.openError(new Shell(), "Patch Error", "It appears that the ROCCC Compiler is not in a state that is valid for patching. Update will cancel without running this patch.");
					}
				});
				return false;
			}
			
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
			dis.syncExec(new Runnable()
			{
				public void run()
				{
					MessageDialog.openError(new Shell(), "Patch Error", "There was an error patching the ROCCC Compiler. Your distribution folder may be corrupt.");
				}
			});
					
			return false;
		}
			
		return true;
	}
	
	static private boolean findNextVersion()
	{
		try
		{
			//Go through the list of available patches and find the one that matches our version.
			//If we find our version in the list, the next available version is one higher.
			for(int i = 0; i < onlineVersionInfo.size(); ++i)
			{
				if(new Version(onlineVersionInfo.get(i)[0]).compareTo(localVersion) == 0)
				{
					if(i == 0)
						nextVersion = latestVersion;
					else
						nextVersion = new Version(onlineVersionInfo.get(i - 1)[0]);
					
					nextVersionPatchLink = onlineVersionInfo.get(i)[1];			
					break;
				}
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
		
		return true;
	}
	
	public static boolean getCurrentVersion()
	{
		String versionString = Activator.getCompilerVersion();
		if(versionString == null)
			return false;
		localVersion = new Version(versionString);

		//If the version has no quantifier, add 0 as the quantifier.
		if(localVersion.getQualifier() == null || localVersion.getQualifier().equals(""))
			localVersion = new Version(localVersion.toString() + ".0");
		
		if(localVersion.toString().equals(""))
		{
			dis.syncExec(new Runnable()
			{
				public void run()
				{
					//MessageDialog.openError(new Shell(), "Distribution Error", "There was an error reading the version of the local ROCCC Compiler. Your distribution folder may be corrupt.");
				}
			});
					
			return false;
		}	
		return true;
	}
	
	static public boolean getOnlineVersionInfo()
	{
		try
		{
			//Go to the version info page on ROCCC and get the latest version.
			URL url = new URL(compilerInfoURL());
			InputStream is = url.openStream();

			BufferedReader dis = new BufferedReader(new InputStreamReader(is));
			latestVersion = new Version(dis.readLine());
		
			//Get all the info about all the available versions.
			onlineVersionInfo = new Vector<String[]>();
			for(String versionInfo = dis.readLine(); versionInfo != null; versionInfo = dis.readLine())
			{
				StringBuffer buffer = new StringBuffer(versionInfo);
				onlineVersionInfo.add(new String[]{StringUtils.getNextStringValue(buffer), StringUtils.getNextStringValue(buffer)});
			}
			dis.close();
			is.close();
		}
		catch(java.io.FileNotFoundException e)
		{
			return false ;
		}
		catch(Exception e)
		{
			if(showCheckingErrors)
				e.printStackTrace();
			dis.syncExec(new Runnable()
			{
				public void run()
				{
					if(showCheckingErrors)
						MessageDialog.openError(new Shell(), "Connection Error", "There was an error receiving update info. Make sure you have a working internet connection.");
				}
			});
					
			return false;
		}
		
		return true;
	}
	
	public static boolean newVersionAvailable()
	{
		return localVersion.compareTo(latestVersion) < 0;
	}
	
	static private boolean downloadPatch()
	{	
		MessageUtils.printlnConsoleMessage("Downloading patch " + nextVersion + "...");
		
		//Get the patch link and download all the bytes onto the computer at the destination we chose.
		BufferedInputStream in;
		try 
		{
			in = new java.io.BufferedInputStream(new URL(nextVersionPatchLink).openStream());
		
			FileOutputStream fos = new FileOutputStream(localPatchDestination);
			BufferedOutputStream bout = new BufferedOutputStream(fos, 1024);
		
			byte[] data = new byte[1024];
			for(int x = in.read(data, 0, 1024); x >= 0; x = in.read(data, 0, 1024))
			{
				bout.write(data,0,x);
			}
			
			bout.close();
			in.close();	
		} 
		catch (Exception e)
		{
			e.printStackTrace();
			dis.syncExec(new Runnable()
			{
				public void run()
				{
					MessageDialog.openError(new Shell(), "Connection Error", "There was an error downloading the ROCCC Compiler patch.  Make sure you have a working connection.");
				}
			});
					
			return false;
		}
		
		MessageUtils.printlnConsoleMessage("Patch Downloaded.");
		return true;
	}
	
	static public int readyToUpdate()
	{
		//Check current version.
		if(getCurrentVersion() == false)
		{
			return LOCAL_VERSION_ERROR;
		}
		
		//Get the latest version available online.
		if(getOnlineVersionInfo() == false)
		{
			return NO_UPDATE_AVAILABLE ;
//			return CONNECTION_ERROR;
		}
		
		//See if the latest version is newer 
		if(newVersionAvailable() == false)
		{
			if(showCheckingMessages)
				MessageUtils.printlnConsoleMessage("ROCCC Compiler is currently up to date.");
			return NO_UPDATE_AVAILABLE;
		}
		
		if(showCheckingMessages)
			MessageUtils.printlnConsoleMessage("ROCCC Compiler has a newer version available.");
		
 		return UPDATE_AVAILABLE;
	}
	
}
