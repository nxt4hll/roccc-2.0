package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.jface.dialogs.MessageDialog;

import rocccplugin.Activator;
import rocccplugin.actions.CancelCompile;
import rocccplugin.actions.CompilationPass;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.views.DebugVariables;
import rocccplugin.views.ROCCC_IPCores;

public class CancelCompileHandler implements IHandler 
{

	//static boolean enabled = true;
	
	//static void setEnabled(boolean e)
	//{
	//	enabled = e;
//	}
	
	public void addHandlerListener(IHandlerListener handlerListener) 
	{
		// TODO Auto-generated method stub
	}

	public void dispose() 
	{
		// TODO Auto-generated method stub
		
	}

	public Object execute(ExecutionEvent event) throws ExecutionException 
	{	
		if(CompilationPass.canCompile())
		{
			MessageDialog.openError(null, "Cancel Build Error", "There is no ROCCC build being done to cancel.");
			return null;
		}
			
		//See if they really want to cancel the build.
		if(!MessageDialog.openQuestion(null, "Cancel Current Build", "Are you sure you want to cancel the current build?"))
			return null;
		
		CancelCompile.run(true);
		
		if(ROCCC_IPCores.showingLockMessage)
		{
		
			boolean IPCoresViewOpen = EclipseResourceUtils.isViewOpen(ROCCC_IPCores.ID);
			EclipseResourceUtils.closeView(ROCCC_IPCores.ID);
			boolean debugViewOpen = EclipseResourceUtils.isViewOpen(DebugVariables.ID);
			EclipseResourceUtils.closeView(DebugVariables.ID);
			
			GuiLockingUtils.unlockGui();
			
			if(IPCoresViewOpen)
				EclipseResourceUtils.openView(ROCCC_IPCores.ID);
			if(debugViewOpen)
				EclipseResourceUtils.openView(DebugVariables.ID);
		}
		else
		{
			GuiLockingUtils.unlockGui();
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
