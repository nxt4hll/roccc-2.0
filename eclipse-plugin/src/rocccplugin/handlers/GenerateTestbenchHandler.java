package rocccplugin.handlers;

import java.io.File;
import java.util.Vector;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.core.resources.IFile;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.widgets.Shell;
import org.osgi.framework.Version;

import rocccplugin.Activator;
import rocccplugin.ROCCCPlugin;
import rocccplugin.actions.TestbenchGenerationPass;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.MessageUtils;

public class GenerateTestbenchHandler implements IHandler 
{
	String componentName;
	String tab = "  ";
	String[] inputScalarNames;
	Vector<Vector<String>> inputScalarValues;
	String[] outputScalars;
	String[] outputScalarVHDLNames;
	String[] outputScalarsPortSizes;
	Vector<Vector<String>> outputScalarValues;
	public String[] inputTestFiles;
	public String[] outputTestFiles;
	Vector<String> oneBitStreamChannels;
	
	int maxChannelBitwidth = 0;
	String maxStreamMemory;
	boolean isSystem;
	
	public void addHandlerListener(IHandlerListener handlerListener) 
	{
	}

	public void dispose() 
	{
	}
	
	private boolean checkROCCCFile(File fileToRunTestbenchOn)
	{
		try
		{
			//Check if the file selected is a system, has VHDL, and is in the database.
			if(fileToRunTestbenchOn == null || !fileToRunTestbenchOn.getName().endsWith(".c"))
			{
				MessageDialog.openError(new Shell(), "Testbench Error", "The ROCCC module or system must be a .c file.");
				return false;
			}
			
			if(fileToRunTestbenchOn.getName().equals("hi_cirrf.c"))
			{
				MessageDialog.openError(new Shell(), "Compiler Error", "The file hi_cirrf.c is a ROCCC reserved file and cannot have a testbench generated through ROCCC. Compilation Aborted.");
				GuiLockingUtils.unlockGui();
				return false;
			}
			
			Boolean isModule = Activator.isModule(fileToRunTestbenchOn);
			if(isModule == null && Activator.isIntrinsic(fileToRunTestbenchOn) == false)
			{
				MessageDialog.openError(new Shell(), "Testbench Error", "You can only create testbenches for modules or systems. If you are selecting a module or system, make sure it is in a proper directory structure.");
				return false;
			}
			
			componentName = DatabaseInterface.getComponentFromSourceFile(fileToRunTestbenchOn);
			if(componentName == null)
			{
				MessageDialog.openError(new Shell(), "Testbench Error", "There was no VHDL found for this system. Make sure you have compiled this system prior to creating a testbench.");
				return false;
			}
			
			
			if(!DatabaseInterface.doesComponentExist(componentName))
			{
				MessageDialog.openError(new Shell(), "Testbench Error", "This component was not found in the ROCCC database. Make sure you have compiled this component prior to creating a testbench.");
				DatabaseInterface.closeConnection();
				return false;
			}
			
			String versionCompiledOn = DatabaseInterface.versionCompiledOn(componentName);
			
			if(versionCompiledOn == null || versionCompiledOn.equals("") || 
			   new Version(versionCompiledOn).compareTo(Activator.getMinimumCompilerVersionNeeded()) <= 0)
			{
				MessageDialog.openError(new Shell(), "Version Compiled On Error", "This component appears to have been compiled in a previous version of ROCCC.  Please recompile this component prior to executing this action.");
				DatabaseInterface.closeConnection();
				return false;
			}
			
			return true;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
	}

	
	
	public Object execute(ExecutionEvent event) throws ExecutionException 
	{
		try
		{
			if(!GuiLockingUtils.canRunCommand())
				return null;
			
			String dbVersion = DatabaseInterface.getDatabaseVersion();
			if(dbVersion.equals("NA") || ROCCCPlugin.getVersionNumber().equals(new Version(dbVersion)) == false)
			{
				MessageUtils.openErrorWindow("Database Error", "The ROCCC database is out of date with the compiler. The database must be reset before running this command");
				GuiLockingUtils.unlockGui();
				DatabaseInterface.closeConnection();
				return null;
			}
	
			IFile fileInEditor = EclipseResourceUtils.getSelectedFileInEditor();
			if(fileInEditor == null)
			{
				MessageDialog.openError(new Shell(), "Testbench Error", "The ROCCC module or system must be a .c file.");
				GuiLockingUtils.unlockGui();
				DatabaseInterface.closeConnection();
				return false;
			}
			
			File fileToRunTestbenchOn = new File(fileInEditor.getRawLocation().toOSString());
			
			if(checkROCCCFile(fileToRunTestbenchOn) == false)
			{
				GuiLockingUtils.unlockGui();
				DatabaseInterface.closeConnection();
				return null;
			}
			
			//Open the testbench wizard

			//RUN THE TESTBENCH
			try
			{
				TestbenchGenerationPass.run(fileToRunTestbenchOn);
			}
			catch(Exception e)
			{
				e.printStackTrace();
			}
			
			DatabaseInterface.closeConnection();
			GuiLockingUtils.unlockGui();
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
