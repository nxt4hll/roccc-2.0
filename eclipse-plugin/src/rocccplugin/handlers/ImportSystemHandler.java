package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.widgets.Shell;

import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.wizards.ImportSystemWizard;

public class ImportSystemHandler implements IHandler 
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
		
		EclipseResourceUtils.openWizard(new ImportSystemWizard());
		
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
