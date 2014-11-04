package rocccplugin.utilities;

import rocccplugin.Activator;
import rocccplugin.database.DatabaseInterface;

public class GuiLockingUtils 
{
	static boolean guiLock = false;
	static String lockMessage = "";
	
	public static boolean isGuiLocked()
	{
		return guiLock;
	}
	
	public static boolean lockGui(String message)
	{
		if(!guiLock)
		{
			guiLock = true;
			lockMessage = new String(message);
			return true;
		}
		
		return false;
	}
	
	public static boolean lockGui()
	{
		return lockGui("There is another ROCCC process being run, please wait.");
	}
	
	public static void unlockGui()
	{
		guiLock = false;
	}
	
	public static void setLockMessage(String message)
	{
		lockMessage = new String(message);
	}
	
	public static String getLockMessage()
	{
		return lockMessage;
	}
	
	public static boolean canRunCommand(String message)
	{
		try
		{
			//Lock the Gui
			if(!lockGui(message))
			{
				MessageUtils.openErrorWindow("Gui locked", lockMessage);
				
				//Activator.showStackTrace();
				
				return false;
			}
		
			//Check for the correct distribution folder.
			if(DatabaseInterface.openConnection() == false)
			{
				unlockGui();
				return false;
			}
			
			//Make sure the compiler and plugin are on compatible versions.
			if(Activator.areCompilerAndPluginSynced() == false)
			{
				DatabaseInterface.closeConnection();
				unlockGui();
				return false;
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return true;
	}
	
	public static boolean canRunCommand()
	{
		return canRunCommand(new String("There is another ROCCC process being run, please wait."));
	}
	
	
}
