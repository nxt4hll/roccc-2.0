package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;

import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.wizards.ImportIntrinsicWizard;

public class ImportIntrinsicHandler implements IHandler 
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
		
		ImportIntrinsicWizard wizard = new ImportIntrinsicWizard(false, 0);
		IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
		
		WizardDialog dialog = new WizardDialog(window.getShell(), wizard);
		dialog.setHelpAvailable(false);
		if(dialog.open() == Window.CANCEL)
		{
			GuiLockingUtils.unlockGui();
			return null;
		}
		
		DatabaseInterface.closeConnection();
		GuiLockingUtils.unlockGui();
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
