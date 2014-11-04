package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;

import rocccplugin.actions.ImportExamples;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.GuiLockingUtils;

public class ImportExamplesHandler implements IHandler 
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
		
		ImportExamples.run();
		
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
