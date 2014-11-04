package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.handlers.HandlerUtil;
import org.osgi.framework.Version;

import rocccplugin.ROCCCPlugin;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.wizards.AddComponentWizard;

public class AddComponentHandler implements IHandler 
{
	
	public void addHandlerListener(IHandlerListener handlerListener) 
	{
	}

	
	public void dispose() 
	{ 
	}
	
	public Object execute(ExecutionEvent event) throws ExecutionException 
	{	
		try
		{ 
			//See if we can execute this command.
			if(!GuiLockingUtils.canRunCommand())
				return null;
			
			String dbVersion = DatabaseInterface.getDatabaseVersion();
			if(dbVersion.equals("NA") || ROCCCPlugin.getVersionNumber().equals(new Version(dbVersion)) == false)
			{
				MessageUtils.openErrorWindow("Database Error", "The ROCCC database is out of date with the compiler. The database must be reset before running this command");
				GuiLockingUtils.unlockGui();
				DatabaseInterface.closeConnection();
				return null;
			}
			
			GuiLockingUtils.setLockMessage("Adding IPCore to the database, please wait.");
			
			IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
			ISelection selection = HandlerUtil.getCurrentSelection(event);
			 
			AddComponentWizard wizard = new AddComponentWizard();
			
			WizardDialog dialog = new WizardDialog(window.getShell(), wizard);
			dialog.setHelpAvailable(false);
			if(dialog.open() == Window.CANCEL)
			{
				GuiLockingUtils.unlockGui();
				DatabaseInterface.closeConnection();
				return null;
			}
			
			GuiLockingUtils.unlockGui();
			DatabaseInterface.openConnection();
			DatabaseInterface.updateAllListeners();
			DatabaseInterface.closeConnection();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
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
