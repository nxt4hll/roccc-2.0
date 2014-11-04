package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;

import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.wizards.CreateProjectWizard;

public class NewProjectHandler implements IHandler 
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
		
		EclipseResourceUtils.openWizard(new CreateProjectWizard());
		
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
