package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;
import org.osgi.framework.Version;

import rocccplugin.ROCCCPlugin;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.wizards.ViewIntrinsicsWizard;

public class ViewIntrinsicsHandler implements IHandler 
{
	public void addHandlerListener(IHandlerListener handlerListener) 
	{
	}

	public void dispose() 
	{
	}

	public Object execute(ExecutionEvent event) throws ExecutionException 
	{
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
		
		IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
		
		ViewIntrinsicsWizard wizard = new ViewIntrinsicsWizard();
		
		WizardDialog dialog = new WizardDialog(window.getShell(), wizard);
		dialog.setHelpAvailable(false);
		if(dialog.open() == Window.CANCEL)
		{
			DatabaseInterface.closeConnection();
			GuiLockingUtils.unlockGui();
			return null;
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
