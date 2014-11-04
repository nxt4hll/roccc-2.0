package rocccplugin.handlers;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.handlers.HandlerUtil;

import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.wizards.PlatformConnectionsWizard;

public class OpenPlatformConnectionsHandler implements IHandler {

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
		
		//Test if a project is open
		if(ResourcesPlugin.getWorkspace().getRoot().getProjects() == null ||
		   ResourcesPlugin.getWorkspace().getRoot().getProjects().length == 0)
		{
			MessageDialog.openError(new Shell(), "No Projects Error", "A valid project must be opened to perform this action.");
			GuiLockingUtils.unlockGui();
			return null;
		}
		
		//Test if a file is even opened
		try
		{
			PlatformUI.getWorkbench().getWorkbenchWindows()[0].getActivePage().getActiveEditor().isDirty();
		}
		catch(Exception e)
		{
			MessageDialog.openError(new Shell(), "File Load Error", "This action cannot be done without a valid file opened.");
			GuiLockingUtils.unlockGui();
			return null;
		}
		
		//Get the actual file.
		IWorkbench wb = PlatformUI.getWorkbench();
		//Could be volatile!
		IWorkbenchWindow win = wb.getWorkbenchWindows()[0];
		IWorkbenchPage page = win.getActivePage();
	
		IEditorPart part = page.getActiveEditor();
		
		IEditorInput input = part.getEditorInput();
		if( !(input instanceof IFileEditorInput) )
		{
			GuiLockingUtils.unlockGui();
			return null;
		}
		IFileEditorInput fei = (IFileEditorInput)input;
		final String fullFilename = fei.getFile().getLocation().toOSString();
	
		//Check that the file you are working with is a c file.
		if(!fullFilename.endsWith(".c"))
		{
			MessageDialog.openError(new Shell(), "Error", "Error: A \".c\" ROCCC system file must be selected. Compilation Aborted.");
			GuiLockingUtils.unlockGui();
			return null;
		}
	
		final Display dis = Display.getCurrent();
			
		IWorkbenchWindow window = HandlerUtil.getActiveWorkbenchWindow(event);
		ISelection selection = HandlerUtil.getCurrentSelection(event);
			 
		
		try
		{
			if(fei.getFile().getLocation().toString().contains("/src/modules/"))
			{
				MessageDialog.openError(new Shell(), "Module Error", "You can only setup connections on ROCCC systems.\n\nYou have selected a module, please select a system to perform this task.");
				GuiLockingUtils.unlockGui();
				return null;
			}
			else if(fei.getFile().getLocation().toString().contains("/src/systems/") == false)
			{
				MessageDialog.openError(new Shell(), "File Error", "You can only setup connections on ROCCC systems. If you are selecting a system, make sure it is in a proper directory structure.");
				GuiLockingUtils.unlockGui();
				return null;
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		PlatformConnectionsWizard wizard = new PlatformConnectionsWizard(fei.getFile().getName().toString());
		wizard.init(window.getWorkbench(), selection instanceof IStructuredSelection
				? (IStructuredSelection) selection : StructuredSelection.EMPTY);
		
		WizardDialog dialog = new WizardDialog(window.getShell(), wizard);
		dialog.setHelpAvailable(false);
		
		if(dialog.open() == Window.CANCEL)
		{
			GuiLockingUtils.unlockGui();
			return null;
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
