package rocccplugin.actions;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.osgi.framework.Version;

import rocccplugin.Activator;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.StringUtils;

public class UpdateGUI 
{
	public static final int CONNECTION_ERROR = -1;
	public static final int NO_UPDATE_AVAILABLE = 0;
	public static final int UPDATE_AVAILABLE = 1;
	
	private static String newPluginPath;
	private static String[] onlineGUIInfo = new String[2];
	private static String localPluginDestination;
	
	static boolean showCheckingErrors = true;
	static boolean showCheckingMessages = true;
	
	//Set whether or not certain messages are shown.
	static public void showMessages(boolean checkingErrors, boolean checkingMessages)
	{
		showCheckingErrors = checkingErrors;
		showCheckingMessages = checkingMessages;
	}
	
	private static String getGUILocation()
	{
		String preface = "http://www.jacquardcomputing.com/downloads/" ;
		String os = Activator.getOSString();
		return preface + os + "_GUI_version.txt" ;		
	}
	
	private static boolean getOnlineVersionInfo()
	{
		try
		{
			//Go to the version info page on ROCCC and get the latest version.
			//URL url = new URL("http://www.jacquardcomputing.com/internalTest/current_GUI_version.txt");
			//URL url = new URL("http://www.jacquardcomputing.com/downloads/current_GUI_version.txt");
			URL url = new URL(getGUILocation()) ;
			
			InputStream is = url.openStream();
			BufferedReader dis = new BufferedReader(new InputStreamReader(is));
		
			//Get all the info about all the available versions.
			StringBuffer buffer = new StringBuffer(dis.readLine());
			
			onlineGUIInfo[0] = StringUtils.getNextStringValue(buffer);
			onlineGUIInfo[1] = StringUtils.getNextStringValue(buffer);
			
			dis.close();
			is.close();
		}
		catch(Exception e)
		{	
			if(showCheckingErrors)
			{
				e.printStackTrace();
				MessageDialog.openError(new Shell(), "Connection Error", "There was an error checking the latest available versions. Make sure you have a working internet connection.");
			}
			return false;
		}
		
		return true;
	}
	
	public static boolean newVersionAvailable()
	{
		//Extract the numbers between the '.'s of the version number.
		return Activator.bc.getBundle().getVersion().compareTo(new Version(onlineGUIInfo[0])) < 0;
	}
	
	public static String getPatchDownloadDestination()
	{
		File file = new File(onlineGUIInfo[1]);
		String name = file.getName();
		return Activator.getDistributionFolder() + "/GUI/" + name;
	}

	public static boolean downloadPlugin(Display dis)
	{	
		BufferedInputStream in;
		try 
		{
			in = new java.io.BufferedInputStream(new URL(onlineGUIInfo[1]).openStream());
		
			FileOutputStream fos = new FileOutputStream(getPatchDownloadDestination());
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
			dis.syncExec(new Runnable()
			{
				public void run()
				{
					MessageDialog.openError(new Shell(), "Connection Error", "Could not download the latest plugin. Make sure you have a valid internet connection");
				}
			});
			
			return false;
		}
		
		MessageUtils.printlnConsoleMessage("GUI Plugin Downloaded.");
		
		dis.syncExec(new Runnable()
		{
			public void run()
			{
				//MessageDialog.openInformation(new Shell(), "Update Complete", "To finish this update, please close Eclipse and move the new plugin:\n\n" + getPatchDownloadDestination() + "\n\nto your Eclipse plugins directory. You will have to flush the plugin cache for the new plugin to be loaded.\n\nTo flush Eclipse using the command line, use the command \"eclipse -clean\" after you have removed the old plugin and installed the new one.");
			}
		});
		return true;
	}
	
	static public int readyToUpdate()
	{
		//Get the latest version available online.
		if(getOnlineVersionInfo() == false)
		{
			return CONNECTION_ERROR;
		}
		//See if the latest version is newer 
		if(newVersionAvailable() == false)
		{
			if(showCheckingMessages)
				MessageUtils.printlnConsoleMessage("ROCCC GUI plugin is currently up to date.");
			return NO_UPDATE_AVAILABLE;
		}
		
		if(showCheckingMessages)
			MessageUtils.printlnConsoleMessage("ROCCC GUI plugin has a newer version available.");
		
		return UPDATE_AVAILABLE;
	}
}
