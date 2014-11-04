package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.handlers.HandlerUtil;

import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.wizards.CreateModuleWizard;

public class CreateModuleHandler implements IHandler 
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
		
		if(ResourcesPlugin.getWorkspace().getRoot().getProjects() == null ||
		   ResourcesPlugin.getWorkspace().getRoot().getProjects().length == 0)
		{
			MessageDialog.openError(new Shell(), "No Projects Error", "A valid project must be opened to perform this action.");
			GuiLockingUtils.unlockGui();
			return null;
		}
		
		IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
		ISelection selection = HandlerUtil.getCurrentSelection(event);
		 
		CreateModuleWizard wizard = new CreateModuleWizard();
		wizard.init(window.getWorkbench(), selection instanceof IStructuredSelection
				? (IStructuredSelection) selection : StructuredSelection.EMPTY);
		
		WizardDialog dialog = new WizardDialog(window.getShell(), wizard);
		dialog.setHelpAvailable(false);
		if(dialog.open() == Window.CANCEL)
		{
			DatabaseInterface.closeConnection();
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
