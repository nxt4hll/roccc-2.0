package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.handlers.HandlerUtil;

import rocccplugin.Activator;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.views.DebugVariables;

public class ViewDebugVariables implements IHandler 
{
	public void addHandlerListener(IHandlerListener handlerListener) 
	{
	}

	public void dispose() 
	{
	}

	public Object execute(ExecutionEvent event) throws ExecutionException 
	{
		//Get the current window.
		IWorkbenchWindow window = HandlerUtil.getActiveWorkbenchWindow(event);
		if(window == null)
		{
			return null;
		}
		//Get the active page on the window.
		IWorkbenchPage page = window.getActivePage();
		if(page == null)
		{
			return null;
		}
		//Open the view.
		try
		{
			EclipseResourceUtils.openView(DebugVariables.ID);
				
			DatabaseInterface.updateAllListeners();
			DatabaseInterface.closeConnection();
		}
		catch(Exception e)
		{
			e.printStackTrace();
			//System.out.println("Failed to open ROCCC IPCores view: " + e);
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
