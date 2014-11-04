package rocccplugin.handlers;

import java.io.File;
import java.util.Vector;
import java.util.concurrent.locks.ReentrantLock;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.core.resources.IFile;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.handlers.HandlerUtil;
import org.osgi.framework.Version;

import rocccplugin.Activator;
import rocccplugin.ROCCCPlugin;
import rocccplugin.actions.CompilationPass;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.MessageUtils;

public class CompileIntoHardwareHandler implements IHandler 
{
	static Thread t;
	static boolean enabled = true;
	static ReentrantLock lock = new ReentrantLock(true);
	static Process compileProcess;
	static String distributionDirectory;
	
	static boolean preProcessSucceeded = false;
	static Vector<String> modulesCalled = new Vector<String>();
	static File suifFile = null;
	
	static final boolean CANCELED = false;

	public void addHandlerListener(IHandlerListener handlerListener) 
	{	
	}
 
	public void dispose() 
	{
	}
	
	private void checkIfFileNeedsSaving()
	{
		//If the selected file is modified ask the user if they would like to save prior to compiling.
		if(PlatformUI.getWorkbench().getWorkbenchWindows()[0].getActivePage().getActiveEditor().isDirty())
		{
			String fileName = ((IFileEditorInput)PlatformUI.getWorkbench().getWorkbenchWindows()[0].getActivePage().getActiveEditor().getEditorInput()).getFile().getName();
			boolean doSave = MessageDialog.openQuestion(null, "Save Resource", fileName + " has been modified. Save changes?");
			if(doSave)
			{
				PlatformUI.getWorkbench().getWorkbenchWindows()[0].getActivePage().getActiveEditor().doSave(null);
			}
		}
	}
	
	private boolean openFileInEditor(final ExecutionEvent event) throws ExecutionException
	{
		//See if the file selected was the one in the context menu.
		if(HandlerUtil.getActiveMenuSelection(event) != null)
		{
			String filename = PlatformUI.getWorkbench().getWorkbenchWindows()[0].getSelectionService().getSelection().toString();
			filename = filename.substring(3, filename.length() -1);
			if(EclipseResourceUtils.openLocalFileInEditor(filename) == false)
			{
				MessageUtils.openErrorWindow("File Load Error", "Please specify a valid system or module file.");
				GuiLockingUtils.unlockGui();
				return false;
			}
		}
		
		try
		{
			PlatformUI.getWorkbench().getWorkbenchWindows()[0].getActivePage().getActiveEditor().isDirty();
		}
		catch(Exception e)
		{
			MessageDialog.openError(new Shell(), "File Load Error", "This action cannot be done without a valid file opened.");
			GuiLockingUtils.unlockGui();
			return false;
		}
		
		return true;
	}
	
	public Object execute(final ExecutionEvent event) throws ExecutionException 
	{
		try
		{
			if(!CompilationPass.canCompile())
			{
				MessageDialog.openError(new Shell(), "Compile Error", "There is a compilation in process. Please wait for the compilation to finish.");
				return null;
			}
			
			if(!GuiLockingUtils.canRunCommand())
				return null;
			
			String dbVersion = DatabaseInterface.getDatabaseVersion();
			if(dbVersion.equals("NA") || ROCCCPlugin.getVersionNumber().equals(new Version(dbVersion)) == false)
			{
				MessageUtils.openMessageWindow("got an error", "db Version: " + dbVersion + " Plugin Version: " + ROCCCPlugin.getVersionNumber().toString());
				MessageUtils.openErrorWindow("Database Error", "The ROCCC database is out of date with the compiler. The database must be reset before running this command");
				GuiLockingUtils.unlockGui();
				DatabaseInterface.closeConnection();
				return null;
			}
			
			//If the compile command was ran through the context menu, open the file it was selected on.
			if(openFileInEditor(event) == false)
				return null;
			
			checkIfFileNeedsSaving();
		
			final String fullFilename = EclipseResourceUtils.getSelectedFileLocationInEditor();
			
			//Check that the file you are compiling is a c file.
			if(fullFilename == null || !fullFilename.endsWith(".c"))
			{
				MessageDialog.openError(new Shell(), "Compiler Error", "A \".c\" file must be selected. Compilation Aborted.");
				GuiLockingUtils.unlockGui();
				return null;
			}
		
			IFile file = EclipseResourceUtils.getSelectedFileInEditor();
			
			GuiLockingUtils.setLockMessage("ROCCC is currently compiling the file " + file.getName() + ". Please wait until that is finished.");
			
			if(file.getName().equals("hi_cirrf.c"))
			{
				MessageDialog.openError(new Shell(), "Compiler Error", "The file hi_cirrf.c is a ROCCC reserved file and cannot be compiled through ROCCC. Compilation Aborted.");
				GuiLockingUtils.unlockGui();
				return null;
			}
			
			CompilationPass.compileFile(new File(file.getRawLocation().toOSString()), false);	
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
