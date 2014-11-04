package rocccplugin.handlers;

import java.awt.Desktop;
import java.io.File;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.core.filesystem.EFS;
import org.eclipse.core.filesystem.IFileStore;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.swt.program.Program;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;

import rocccplugin.Activator;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.MessageUtils;

public class ViewUserManualHandler implements IHandler 
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
		
		if(!GuiLockingUtils.canRunCommand())
			return null;
		
		//String editorId = PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor("file.txt").getId().toString();
		//PlatformUI.getWorkbench().getEditorRegistry().setDefaultEditor("*.pdf", editorId);
		
		IWorkbenchPage page = PlatformUI.getWorkbench().getWorkbenchWindows()[0].getActivePage();

		String manualPath = Activator.getDistributionFolder() + "/Documentation/UserManual.pdf";
		
		if(new File(manualPath).exists() == false)
		{
			MessageUtils.printlnConsoleError("Error: File " + Activator.getDistributionFolder() + "/Documentation/UserManual.pdf does not exist. Cannot Open.");
			DatabaseInterface.closeConnection();
			GuiLockingUtils.unlockGui();
			return null;
		}
		
		try
		{
			boolean opened = true;
			
			//Generic version of opening a program.
			Program p = Program.findProgram(".pdf");
			if(p == null)
				opened = false;
			else
				p.execute(manualPath);
			
			//If the original version didn't work, let's try the newly added Desktop method.
			if(!opened && Desktop.isDesktopSupported())
			{
				Desktop desktop = Desktop.getDesktop();
				desktop.open(new File(manualPath));
				opened = true;
			}
			
			//If for some reason it failed the first two which is most likely to happen on Linux,
			//try this version which just uses the Linux "open" command.
			if(!opened && isLinux())
			{
				String[] cmds = new String[2];
				cmds[0] = "xdg-open";
				cmds[1] = manualPath;
				opened = true;
				try
				{
					Runtime.getRuntime().exec(cmds);
				}
				catch(Exception e1)
				{
					e1.printStackTrace();
					opened = false;
				}
			}
			
			//If all else has failed, try to open it in the default editor even if it looks bad.
			//Should not get to this point.
			if(!opened)
			{
				IPath path = new Path(manualPath);
				IFileStore fileStore = EFS.getLocalFileSystem().getStore(path);
				IDE.openEditorOnFileStore(page, fileStore);
			}
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
		}
		
		GuiLockingUtils.unlockGui();
		DatabaseInterface.closeConnection();
		
		return null;
	}
	
	private boolean isLinux() 
	{
        String os = System.getProperty("os.name").toLowerCase();
        return os.indexOf("linux") != -1;
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
