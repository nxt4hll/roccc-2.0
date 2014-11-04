package rocccplugin.handlers;

import java.net.URL;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.browser.IWorkbenchBrowserSupport;

import rocccplugin.ROCCCPlugin;

public class ViewROCCCSiteHandler implements IHandler 
{
	
	static private String ID = "ROCCCBrowser";

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
			//Test the internet connection.
			URL internetChecker = new URL("http://www.google.com/");
			internetChecker.openStream().close();
		}
		catch(Exception e1)
		{
			MessageDialog.openError(new Shell(), "Internet Connection Error", "There was a problem opening up the webpage. Make sure there is a working internet connection.");
			return false;
		}
				
		String[] websiteArray = new String[]{"http://jacquardcomputing.com/roccc/welcome-to-roccc-2-0/", "http://jacquardcomputing.com/roccc/", "http://jacquardcomputing.com/", "roccc.cs.ucr.edu"};
		
		//Try to open the roccc webpage.
		boolean opened = false;
		for(int i = 0; i < websiteArray.length; ++i)
		{
			if(opened)
				return true;
			
			try
			{
				URL url;
				String version = ROCCCPlugin.getVersionNumber().toString();
				url = new URL(websiteArray[i]);
				url.openStream().close();
				PlatformUI.getWorkbench().getBrowserSupport().createBrowser(IWorkbenchBrowserSupport.NAVIGATION_BAR | IWorkbenchBrowserSupport.AS_EDITOR, "rocccplugin.editors.ROCCCBrowser", "Welcome to ROCCC - Version " + version, "Welcome to ROCCC - Version " + version).openURL(url);
				opened = true;
			} 
			catch (Exception e1) 
			{
				
			}
		}
		
		MessageDialog.openError(new Shell(), "Internet Connection Error", "It appears the website is down or no longer exists. We are sorry for this inconvenience.");
		return false;
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
