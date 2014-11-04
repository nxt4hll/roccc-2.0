package rocccplugin.actions;

import java.io.File;
import java.net.URL;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.jdt.ui.IPackagesViewPart;
import org.eclipse.jface.dialogs.InputDialog;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.window.Window;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IViewPart;
import org.eclipse.ui.IViewReference;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.browser.IWorkbenchBrowserSupport;
import org.eclipse.ui.navigator.resources.ProjectExplorer;

import rocccplugin.Activator;
import rocccplugin.ROCCCPlugin;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.FileUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.StringUtils;

public class ImportExamples 
{
	static private String ID = "ROCCCBrowser";
	
	static public boolean run()
	{
		IProject examples = null;
		try
		{
			//Try the default name of ROCCCExamples.
			String name = "ROCCCExamples";
			examples = ResourcesPlugin.getWorkspace().getRoot().getProject(name);
			boolean retry = EclipseResourceUtils.projectExists(examples.getName());
			
			//Keep asking for a name until we get a valid one.
			while(retry)
			{
				InputDialog dialog = new InputDialog(new Shell(), "Examples Project Name", "Choose a name for the project the ROCCC Examples will be placed in.", "", null);

				if(dialog.open() == Window.CANCEL)
				{
					return false;
				}
				
				boolean invalidProjectName = false;
				
				try
				{
					examples = ResourcesPlugin.getWorkspace().getRoot().getProject(dialog.getValue());
				}
				catch(Exception e)
				{
					invalidProjectName = true;
				}
				
				if(invalidProjectName || !StringUtils.isValidVariableName(dialog.getValue()))
				{
					if(MessageDialog.openQuestion(new Shell(), "Project Name Error", "The project name \"" + dialog.getValue() + "\" is invalid. Would you like to try a different name?") == false) 
					{
						return false;
					}
				}
				else if(EclipseResourceUtils.projectExists(examples.getName()))
				{
					if(MessageDialog.openQuestion(new Shell(), "Existing Project", "The project \"" + examples.getName() + "\" already exists. Would you like to try a different name?") == false) 
					{
						return false;
					}
				}
				else
				{
					retry = false;
				}
				
			}
			
			//Create the project and move all the files from the examples to the project.
			examples.create(null);
			examples.open(null);
			FileUtils.copyDirectory(new File(Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/Examples/ROCCCExamples/"), new File(examples.getWorkspace().getRoot().getRawLocation().toOSString() + "/" + examples.getName()));
						
			//Delete the testcode folder from the project.
			examples.refreshLocal(IResource.DEPTH_INFINITE, null);
			if(examples.getFolder("/src/").getFolder("/testcode/").exists())
				examples.getFolder("/src/").getFolder("/testcode/").delete(true, null);
			examples.refreshLocal(IResource.DEPTH_INFINITE, null);
			
			//Display successful import.
			MessageUtils.printlnConsoleSuccess("ROCCC Examples loaded into project \"" + examples.getName() + "\"\n");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
		
		//This section will expand the newly imported examples project so they can see all the modules and systems that
		//are distributed.
		try
		{
			IViewReference[] parts = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().getViewReferences();
		    IProject activeProject = null;
	
		    for(int i = 0; i < parts.length; i++)
		    {
		    	IViewPart v = parts[i].getView(false);
		    	if(v == null)
		    		continue;
		    	
		        if(v.getClass().toString().equals("class org.eclipse.jdt.internal.ui.packageview.PackageExplorerPart"))
		        {	
		        	Object o = examples;
		            ((IPackagesViewPart)v).getTreeViewer().expandToLevel(o, 3);
		            break;
		        }
		        else if(v.getClass().toString().equals("class org.eclipse.ui.navigator.resources.ProjectExplorer"))
		        {
		        	Object o = examples;
		        	((ProjectExplorer)v).getCommonViewer().expandToLevel(o, 3);
		            break;
		        }
		    }
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		//Finally we try to open the ROCCC webpage to the examples page so they can look at the examples 
		//explanations on the webpage while messing with the examples codes.
		try 
		{
			//Test the internet connection seeing if we can find the ROCCC page.
			URL internetChecker = new URL("http://jacquardcomputing.com/roccc/examples/");
			internetChecker.openStream().close();
			
			URL url;
			String version = ROCCCPlugin.getVersionNumber().toString();
			url = new URL("http://jacquardcomputing.com/roccc/examples/");
			IWorkbenchBrowserSupport browser = PlatformUI.getWorkbench().getBrowserSupport();
			PlatformUI.getWorkbench().getBrowserSupport().createBrowser(IWorkbenchBrowserSupport.AS_EDITOR | IWorkbenchBrowserSupport.NAVIGATION_BAR, ID + "examples", "ROCCC 2.0 Examples", "ROCCC 2.0 Examples").openURL(url);

		} 
		catch (Exception e1) 
		{
			e1.printStackTrace();
		}
		
		return true;
	}
}
