package rocccplugin.utilities;

import java.io.File;

import org.eclipse.core.filesystem.EFS;
import org.eclipse.core.filesystem.IFileStore;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.IEditorDescriptor;
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.IPerspectiveDescriptor;
import org.eclipse.ui.IViewPart;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;
import org.eclipse.ui.part.FileEditorInput;

public class EclipseResourceUtils 
{
	public static void closeFile(IFile file)
	{
		IEditorPart e = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().findEditor(new FileEditorInput(file));
	
		PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().closeEditor(e, false);
	}
	
	public static void deleteProjectFile(IFile file)
	{
		try
		{
			File fullFilePath = new File(file.getRawLocation().toOSString());
	
			if(file.exists())
				file.delete(true, true, null);
			if(fullFilePath.exists())
				fullFilePath.delete();
		}
		catch(java.lang.Exception e)
		{
			e.printStackTrace();
			MessageUtils.openErrorWindow("Error", "There was an error deleting your file.");
			return;
		}
	}
	
	public static int openWizard(Wizard wizard)
	{
		IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
		//ISelection selection = HandlerUtil.getCurrentSelection(event);
		 
		//ImportSystemWizard wizard = new ImportSystemWizard();
		//wizard.init(window.getWorkbench(), selection instanceof IStructuredSelection
		//		? (IStructuredSelection) selection : StructuredSelection.EMPTY);
		
		WizardDialog dialog = new WizardDialog(window.getShell(), wizard);
		dialog.setMinimumPageSize(10, 10);
		dialog.setHelpAvailable(false);
		return dialog.open();
	}
	
	public static IFile getSelectedFileInEditor()
	{
		try
		{
			//Get the filename of the file in the editor.
			IWorkbench wb = PlatformUI.getWorkbench();
			IWorkbenchWindow win = wb.getActiveWorkbenchWindow();
			IWorkbenchPage page = win.getActivePage();
			IEditorPart part = page.getActiveEditor();
			IEditorInput input = part.getEditorInput();
			if( !(input instanceof IFileEditorInput) )
				return null;
			IFileEditorInput fei = (IFileEditorInput)input;
			return fei.getFile();
		}
		catch(Exception e)
		{
			//e.printStackTrace();
			return null;
		}
	}
	
	public static String getSelectedFileProjectName()
	{
		IFile file = getSelectedFileInEditor();
		return file.getProject().getName().toString();
	}
	
	public static String getSelectedFileLocationInEditor()
	{
		//Get the filename of the file in the editor.
		IWorkbench wb = PlatformUI.getWorkbench();
		IWorkbenchWindow win = wb.getWorkbenchWindows()[0];
		IWorkbenchPage page = win.getActivePage();
		IEditorPart part = page.getActiveEditor();
		IEditorInput input = part.getEditorInput();
		if( !(input instanceof IFileEditorInput) )
			return null;
		IFileEditorInput fei = (IFileEditorInput)input;
		String fullFilename = fei.getFile().getLocation().toOSString();
		return fullFilename;
	}
	
	public static String getSelectedFileFolder()
	{
		IFile file = getSelectedFileInEditor();
		return file.getProjectRelativePath().toString().replace(file.getName(), "");
	}
	
	public static void updateProjectFiles()
	{
		//Update the project list to show the new VHDL generated.
		IProject[] projects = ResourcesPlugin.getWorkspace().getRoot().getProjects();
		for(int i = 0; i < projects.length; ++i)
		{
			try 
			{
				projects[i].refreshLocal(IResource.DEPTH_INFINITE, null);
			}
			catch (CoreException e) 
			{
				e.printStackTrace();
			}
		}
	}
	
	public static boolean projectExists(String projectName)
	{
		IProject[] project_names = ResourcesPlugin.getWorkspace().getRoot().getProjects();
		for(int i = 0; i < project_names.length; ++i)
		{
			if(project_names[i].getName().compareToIgnoreCase(projectName) == 0)
				return true;
		}
		return false;
	}
	
	public static void openFileInEditor(final File f)
	{
		try
		{
			
			IWorkbenchPage page = PlatformUI.getWorkbench().getWorkbenchWindows()[0].getActivePage();
			IPath path = new Path(f.getAbsolutePath());
			IFileStore fileStore = EFS.getLocalFileSystem().getStore(path);
			IDE.openEditorOnFileStore(page, fileStore);
					
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static boolean isViewShown(String ID)
	{
		try
		{
			IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
			if(window == null)
				return false;
			//Get the active page on the window.
			IWorkbenchPage page = window.getActivePage();
			if(page == null)
				return false;
			
			for(int i = 0; i < page.getViewReferences().length; ++i)
			{
				if(page.getViewReferences()[i].getId().equals(ID))
				{
					return true;
				}
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return false;
	}
	
	public static void reopenView(String ID)
	{
		try
		{
			boolean open = isViewOpen(ID);
			closeView(ID);
			
			if(open)
				openView(ID);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static boolean isViewOpen(String ID)
	{
		try
		{
			IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
			if(window == null)
				return false;
			//Get the active page on the window.
			IWorkbenchPage page = window.getActivePage();
			if(page == null)
				return false;

			for(int i = 0; i < page.getViewReferences().length; ++i)
			{
				if(page.getViewReferences()[i].getId().equals(ID))
					return true;
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return false;
	}
	
	public static IViewPart openView(String ID)
	{
		try
		{
			IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
			if(window == null)
				return null;
			//Get the active page on the window.
			IWorkbenchPage page = window.getActivePage();
			if(page == null)
				return null;
			
			for(int i = 0; i < page.getViewReferences().length; ++i)
			{
				if(page.getViewReferences()[i].getId().equals(ID))
					return null;
			}
			
			try 
			{
				page.showView(ID);
			} 
			catch (PartInitException e) 
			{
				e.printStackTrace();
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return null;
	}
	
	public static void closeView(String ID)
	{	
		IPerspectiveDescriptor currentPerspective = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().getPerspective();
		IPerspectiveDescriptor[] perspectives = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().getOpenPerspectives();
		
		for(int i = 0; i < perspectives.length; ++i)
		{
			PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().setPerspective(perspectives[i]);
			
			IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
			if(window == null)
				return;
			//Get the active page on the window.
			IWorkbenchPage page = window.getActivePage();
			if(page == null)
				return;
			
			for(int j = 0; j < page.getViewReferences().length; ++j)
			{
				if(page.getViewReferences()[j].getId().equals(ID))
					page.hideView(page.getViewReferences()[j]);
			}
		}
		
		PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().setPerspective(currentPerspective);
	}
	
	static public boolean openLocalFileInEditor(String filename)
	{
		try
		{
			File f = new File(filename);
			Path p = new Path(f.getPath());
			IFile file = ResourcesPlugin.getWorkspace().getRoot().getFile(p);
			
			if(!file.exists())
			{
				throw new Exception();
			}
			IEditorDescriptor desc = PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor("file." + file.getFileExtension());
			if(desc == null)
				desc = PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor("file.java");
			if(desc == null)
				desc = PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor("file.txt");
			IWorkbenchPage page = PlatformUI.getWorkbench().getWorkbenchWindows()[0].getActivePage();
			page.openEditor(new FileEditorInput(file), desc.getId());
		}
		catch(Exception e)
		{
			//System.out.println(e.getMessage());
			e.printStackTrace();
			return false;
		}
		
		return true;
	}
}
