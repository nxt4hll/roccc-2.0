package rocccplugin.handlers;
import java.util.Vector;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.widgets.Shell;

import rocccplugin.Activator;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.helpers.UpdateInfo;
import rocccplugin.utilities.GuiLockingUtils;


public class AboutROCCCHandler implements IHandler 
{
	public void addHandlerListener(IHandlerListener handlerListener) 
	{
	}

	public void dispose() 
	{
	}
	
	public Object execute(ExecutionEvent event) throws ExecutionException 
	{
		//If the database is not accessible, do nothing.
		if(!DatabaseInterface.openConnection())
		{
			GuiLockingUtils.unlockGui();
			return null;
		}
		
		if(!GuiLockingUtils.lockGui())
		{
			MessageDialog.openError(new Shell(), "Gui Locked", GuiLockingUtils.getLockMessage());
			return null;
		}
		
		try
		{
			String addedInfo = new String();
			
			Vector<UpdateInfo> updateInfo = UpdateInfo.updateInfo;
			for(int i = 0; i < updateInfo.size(); ++i)
			{
				addedInfo = addedInfo.concat("ROCCC " + updateInfo.get(i).pluginName + " Version: " + updateInfo.get(i).version.toString() + "\n");
			}
			if(updateInfo.size() > 0)
				addedInfo = addedInfo.concat("\n\n");
			
			String disclaimer = "ROCCC 2.0 is still under development and may contain bugs or be partially unstable. This version is not fully representative of the end product.";
			
			MessageDialog.openInformation(new Shell(), "About ROCCC", "ROCCC Compiler Version: " + Activator.getCompilerVersion() + "\nROCCC GUI Version: " + Activator.bc.getBundle().getVersion().toString() + "\n\n" + addedInfo  + disclaimer);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		GuiLockingUtils.unlockGui();
		DatabaseInterface.closeConnection();

		return null;
	}

	public boolean isEnabled() 
	{
		return true;
	}

	public boolean isHandled() 
	{
		return true;
	}

	public void removeHandlerListener(IHandlerListener handlerListener) 
	{
		
	}

}
