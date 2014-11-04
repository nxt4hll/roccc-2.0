package rocccplugin.wizards;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IEditorDescriptor;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.part.FileEditorInput;

import rocccplugin.Activator;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.wizardpages.ImportModuleWizardPage;

public class ImportModuleWizard extends Wizard implements INewWizard 
{
	private ImportModuleWizardPage importModuleWizardPage;
	
	public ImportModuleWizard() 
	{
		setWindowTitle("Import ROCCC Module");
	}
	
	@Override
	public void addPages()
	{
		importModuleWizardPage = new ImportModuleWizardPage("Module Information Page");
		addPage(importModuleWizardPage);
		this.setDefaultPageImageDescriptor(ImageDescriptor.createFromImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/importModule.png"))));
	}

	@Override
	public boolean performFinish() 
	{
		if(importModuleWizardPage.project.getSelectionIndex() == -1)
		{
			MessageDialog.openError(new Shell(), "Project Selection Error", "A project was not selected.");
			return false;
		}
		
		String name = importModuleWizardPage.moduleName.getText();
		for(int j = 0; j < name.length(); ++j)
		{
			if(!Character.isLetter(name.charAt(j)) &&
			   !Character.isDigit(name.charAt(j)) &&
			   name.charAt(j) != '_')
			{
				//It did not start with an alphabetic character.
				MessageDialog.openError(new Shell(), "Module Name Error", "Module name \"" + name + "\" is invalid.\n\n" +
		                                             "Make sure the module name has no special characters.");
				return false;
			}
		}
		
		String filePath = importModuleWizardPage.sourceFileField.getText();
		String projectName = importModuleWizardPage.project.getItem(importModuleWizardPage.project.getSelectionIndex());
		File file = new File(filePath);
		
		if(!file.exists())
		{
			MessageDialog.openError(new Shell(), "File Error", "File \"" + filePath + "\" does not exist.");
			return false;
		}
		
		if(file.getName().contains(" "))
		{
			MessageDialog.openError(new Shell(), "File Error", "The filename of the file being imported cannot contain a space.");
			return false;
		}
		
		if(!filePath.endsWith(".c"))
		{
			MessageDialog.openError(new Shell(), "File Error", "Module file must end in \".c\".");
			return false;
		}
		
		File checker = new File(ResourcesPlugin.getWorkspace().getRoot().getProject(projectName).getLocation().toOSString() + "/src/modules/" + name + "/" + file.getName());
		if(checker.exists())
		{
			MessageDialog.openError(new Shell(), "File Error", "There is a file in this project with this name already.");
			return false;
		}
		
		//Create the src folder if it does not exist.
		IFolder folder = ResourcesPlugin.getWorkspace().getRoot().getProject(projectName).getFolder("/src/");
		try 
		{
			if(!folder.exists())
				folder.create(true, true, null);
		} 
		catch (CoreException e1) 
		{
			e1.printStackTrace();
		}
		
		//Create the modules folder if it does not exist.
		folder = ResourcesPlugin.getWorkspace().getRoot().getProject(projectName).getFolder("/src/modules/");
		try 
		{
			if(!folder.exists())
				folder.create(true, true, null);
		} 
		catch (CoreException e1) 
		{
			e1.printStackTrace();
		}
		
		
		//Create the component folder
		folder = ResourcesPlugin.getWorkspace().getRoot().getProject(projectName).getFolder("/src/modules/" + name + "/");
		IFile ifile = ResourcesPlugin.getWorkspace().getRoot().getProject(projectName).getFile("src/modules/" + name + "/" + file.getName());
		try 
		{
			if(!folder.exists())
				folder.create(true, true, null);
		}
		catch (CoreException e1) 
		{
			e1.printStackTrace();
			return false;
		}
		
		try 
		{
			ifile.create(null, false, null);
		} 
		catch (CoreException e) 
		{
			e.printStackTrace();
			return false;
		}
		//Create the actual file with the text created.
		if(createFile(file.getPath().toString(), ResourcesPlugin.getWorkspace().getRoot().getProject(projectName).getLocation().toOSString() + "/src/modules/" + name + "/" + file.getName()) == false)
			return false;
		
		refreshProjects();
		
		String filename = ifile.getProject() + "/" + ifile.getProjectRelativePath().toOSString();
		filename = filename.substring(2, filename.length());
		EclipseResourceUtils.openLocalFileInEditor(filename);
		
		return true;
	}
	
	private void openFileInEditor(IFile file)
	{
		//Open the newly created file in the default editor.
		try
		{
			IEditorDescriptor desc = PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor(file.getName());
			if(desc == null)
				desc = PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor("file.java");
			if(desc == null)
				desc = PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor("file.txt");
			IWorkbenchPage page = PlatformUI.getWorkbench().getWorkbenchWindows()[0].getActivePage();

			page.openEditor(new FileEditorInput(file), desc.getId());
		}
		catch (Exception err) 
		{
			err.printStackTrace();
		}
	}
	
	private boolean createFile(String file, String destination)
	{
		try
		{
			FileInputStream fis = new FileInputStream(file);
			InputStreamReader in = new InputStreamReader(fis, "UTF-8");

			FileOutputStream fos = new FileOutputStream(destination);
			OutputStreamWriter out = new OutputStreamWriter(fos, "UTF-8"); 
		
			while(in.ready())
			{
				out.append((char) in.read());
			}
			in.close();
			out.close();
		} 
		catch (IOException e) 
		{
			e.printStackTrace();
			return false;
		}
		finally 
		{
		}
		return true;
	}
	
	private void refreshProjects()
	{
		//Update the project list to show the new VHDL generated.
		try
		{
			IProject[] projects = ResourcesPlugin.getWorkspace().getRoot().getProjects();
			for(int i = 0; i < projects.length; ++i)
			{
				projects[i].refreshLocal(IResource.DEPTH_INFINITE, null);
			}
		}
		catch (Exception err) 
		{
			err.printStackTrace();
		}
	}

	public void init(IWorkbench workbench, IStructuredSelection selection) 
	{
	}
}
