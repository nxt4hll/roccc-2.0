package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.dialogs.PreferencesUtil;


public class OpenPreferenceHandler implements IHandler 
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
			PreferencesUtil.createPreferenceDialogOn(new Shell(), "rocccplugin.preferences.ROCCCPreferencePage", null, null).open();
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
