package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.widgets.Display;

import rocccplugin.Activator;
import rocccplugin.actions.ResetDatabase;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.views.DebugVariables;
import rocccplugin.views.ROCCC_IPCores;

public class ResetCompilerHandler implements IHandler 
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
		
		try
		{
			//Warn the user of what they are trying to do.
			if(MessageDialog.openQuestion(null, "Caution!", "Reseting the compiler will remove all compiled " +
														  " and inserted components from the database.\n\n Are you sure " +
														  "you want to continue?") == false)
			{
				MessageUtils.printlnConsoleError("Reset Database Canceled\n");
				DatabaseInterface.closeConnection();
				GuiLockingUtils.unlockGui();
				return null;
			}
			
			MessageUtils.openConsoleView();
			DatabaseInterface.closeConnection();
			ResetDatabase.run();
			GuiLockingUtils.unlockGui();
		
			Display.getDefault().asyncExec(new Runnable()
			{
				public void run()
				{
					//Reset the IPCores view to show the updated database.	
					if(DatabaseInterface.openConnection())
					{	
						DatabaseInterface.updateAllListeners();
						
						boolean IPCoresViewOpen = EclipseResourceUtils.isViewOpen(ROCCC_IPCores.ID);
						EclipseResourceUtils.closeView(ROCCC_IPCores.ID);
						boolean debugViewOpen = EclipseResourceUtils.isViewOpen(DebugVariables.ID);
						EclipseResourceUtils.closeView(DebugVariables.ID);
							
						GuiLockingUtils.unlockGui();
						
						if(IPCoresViewOpen)
							EclipseResourceUtils.openView(ROCCC_IPCores.ID);
						if(debugViewOpen)
							EclipseResourceUtils.openView(DebugVariables.ID);
						
						DatabaseInterface.closeConnection();
					}
				}
			});
		}
		catch(Exception e)
		{
			e.printStackTrace();
			GuiLockingUtils.unlockGui();
			DatabaseInterface.closeConnection();
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
