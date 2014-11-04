package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.core.filesystem.EFS;
import org.eclipse.core.filesystem.IFileStore;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;

import rocccplugin.Activator;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.GuiLockingUtils;

public class OpenROCCCLibraryHandler implements IHandler 
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
		
		String editorId = PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor("file.txt").getId().toString();
		PlatformUI.getWorkbench().getEditorRegistry().setDefaultEditor("*.h", editorId);
		
		IWorkbenchPage page = PlatformUI.getWorkbench().getWorkbenchWindows()[0].getActivePage();

		String library = Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/LocalFiles/roccc-library.h";
		
		try
		{
			IPath path = new Path(library);
			IFileStore fileStore = EFS.getLocalFileSystem().getStore(path);
			IDE.openEditorOnFileStore(page, fileStore);
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
		}
		
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
