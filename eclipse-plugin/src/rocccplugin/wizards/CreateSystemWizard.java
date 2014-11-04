package rocccplugin.wizards;

import java.io.ByteArrayInputStream;

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
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.StringUtils;
import rocccplugin.wizardpages.CreateSystemWizardPage;

public class CreateSystemWizard extends Wizard implements INewWizard 
{
	private CreateSystemWizardPage createSystemWizardPage;

	public CreateSystemWizard() 
	{
		setWindowTitle("New ROCCC System");
	} 
	
	@Override
	public void addPages() 
	{
		createSystemWizardPage = new CreateSystemWizardPage("System Information Page");
		addPage(createSystemWizardPage);
		this.setDefaultPageImageDescriptor(ImageDescriptor.createFromImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/newSystem.png"))));
	}
	
	private void createTextForFile(StringBuffer data, String project_name, String component_name, int dimensionSize, int arraySize)
	{
		createHeader(data, component_name, dimensionSize);
		createVariables(data, dimensionSize);
		createSkeletonLoops(data, arraySize, dimensionSize);
	}
	
	private void createHeader(StringBuffer data, String component_name, int dimensionSize)
	{
		data.append("\nvoid " + component_name + "(");
		data.append("int");
		
		for(int i = 0; i < dimensionSize; ++i)
			data.append("*");
		data.append(" A, int");
		
		for(int i = 0; i < dimensionSize; ++i)
			data.append("*");
		
		data.append(" B");	
		data.append(")\n{\n");
	}
	
	private void createVariables(StringBuffer data, int dimensionSize)
	{
		data.append("\tint i");
		for(int i = 1; i < dimensionSize; ++i)
		{
			data.append(", " + (char)('i' + i)); 
		}
		data.append(";\n\n");
	}
	
	private void createSkeletonLoops(StringBuffer data, int arraySize, int dimensionSize)
	{	
		for(int i = 0; i < dimensionSize; ++i)
		{
			for(int t = 0; t < i; ++t)
				data.append("\t");
			data.append("\tfor(" + (char)('i' + i) + " = 0; " + (char)('i' + i) + " < ");
			data.append(arraySize);
			data.append("; ++" + (char)('i' + i) + ")\n");
			for(int t = 0; t < i; ++t)
				data.append("\t");
			data.append("\t{\n");
		}
		
		for(int t = 0; t <= dimensionSize; ++t)
			data.append("\t");
		data.append("// Example code to pass stream A into stream B\n");
		
		for(int t = 0; t <= dimensionSize; ++t)
			data.append("\t");
		data.append("B");
		for(int i = 0; i < dimensionSize; ++i)
		{
			data.append("[" + (char)('i' + i) + "]");
		}
		data.append(" = A");
		for(int i = 0; i < dimensionSize; ++i)
		{
			data.append("[" + (char)('i' + i) + "]");
		}
		data.append(";");
		data.append("\t\n");
		
		for(int i = 0; i < dimensionSize; ++i)
		{
			for(int t = i + 1; t < dimensionSize; ++t)
				data.append("\t");
			data.append("\t}\n");
		}
		data.append("}\n");
	}
	
	private boolean createFile(String project_name, String component_name, IFile file, StringBuffer stringBuffer)
	{
		//Actually create the source file.
		try
		{
			ByteArrayInputStream Bis1 = new ByteArrayInputStream(stringBuffer.toString().getBytes("UTF-8"));
			file.create(Bis1, false, null);
		}
		catch(java.lang.Exception e)
		{
			MessageDialog.openInformation(new Shell(), "Error", "There was an error creating your file.");
			return false;
		}
		
		return true;
	}
	
	private void openFileInEditor(IFile file)
	{
		//Open the newly created file in the default editor.
		try
		{
			IEditorDescriptor desc = PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor("file.java");
			IWorkbenchPage page = PlatformUI.getWorkbench().getWorkbenchWindows()[0].getActivePage();

			page.openEditor(new FileEditorInput(file), desc.getId());
		}
		catch (Exception err) 
		{
			err.printStackTrace();
		}
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
	
	@Override
	public boolean performFinish() 
	{
		try
		{
			if(createSystemWizardPage.project.getSelectionIndex() == -1)
			{
				MessageDialog.openError(new Shell(), "Project Selection Error", "There is no project selected.");
				return false;
			}
			String project_name = createSystemWizardPage.project.getItem(createSystemWizardPage.project.getSelectionIndex());
			String component_name = createSystemWizardPage.component_name.getText();
			
			if(StringUtils.isCPlusPlusReservedWord(component_name))
			{
				//It did not start with an alphabetic character.
				MessageDialog.openInformation(new Shell(), "System Name Error", "System name \"" + component_name + "\" is reserved by C++.\n\n" +
		                                                   "Make sure the system name is not reserved by C++.");
				return false;
			}
			
			if(StringUtils.isROCCCReservedWord(component_name))
			{
				//It did not start with an alphabetic character.
				MessageDialog.openInformation(new Shell(), "System Name Error", "System name \"" + component_name + "\" is reserved by ROCCC.\n\n" +
														   "Make sure the system name does not start with \"ROCCC\" or \"suifTmp\".");
				return false;
			}
			
			if(!StringUtils.isValidVariableName(component_name))
			{
				MessageUtils.openErrorWindow("System Name Error", "The system name is an invalid name. The system name must be one that would be valid as a C function name.");
				return false;
			}
			
			if(component_name.equalsIgnoreCase("hi_cirrf"))
			{
				//The name cannot be the keyword hi_cirrf.
				MessageDialog.openInformation(new Shell(), "Component Name Error", "The name hi_cirrf is a ROCCC reserved file name and cannot be used.");	
				return false;
			}
			
			int numComponentsInDatabase;
			if(DatabaseInterface.openConnection())
				numComponentsInDatabase = DatabaseInterface.getNumComponents();
			else
				numComponentsInDatabase = 0;
			
			
			String[] componentsInDatabase = DatabaseInterface.getAllComponents();
			
			//Check to see if there is a component in the database with the same already.
			for(int i = 0; i < componentsInDatabase.length; ++i)
			{
				if(componentsInDatabase[i].compareToIgnoreCase(component_name) == 0)
				{
					//There was a component in the database with the same name, display error.
					boolean continueCreation = MessageDialog.openQuestion(null, "Matching Name", "The module name matches a name in the component database." +
																		  						 "\n\nDo you wish to continue?");
					if(!continueCreation)
						return false;
				}
				
				String structName = DatabaseInterface.getStructName(componentsInDatabase[i]);
				if(structName != null && structName.equals(component_name))
				{
					//There was a component in the database with the same name, display error.
					MessageDialog.openError(null, "Matching Name", "The system name matches a struct name in the component database." +
																		  			     "\n\nPlease choose a different system name.");
					return false;
				}
			}
			
			StringBuffer data = new StringBuffer("");
			int arraySize = 100;
			
			IFile file = ResourcesPlugin.getWorkspace().getRoot().getProject(project_name).getFile("src/systems/" + component_name + "/" + component_name + ".c");
			
			if(file.exists())
			{
				MessageDialog.openError(new Shell(), "System Name Error", "There is already a file in the project named " +
														   component_name + ".c.\n\nTry renaming the system name.");
				return false;
			}
			
			int dimensionSize = Integer.parseInt(createSystemWizardPage.dimensions.getText());
			
			//Build a skeleton text data with the function name, variables, and loop set up for the new file.
			createTextForFile(data, project_name, component_name, dimensionSize, arraySize);
			
			//Create the src folder if it does not exist.
			IFolder folder = ResourcesPlugin.getWorkspace().getRoot().getProject(project_name).getFolder("/src/");
			try 
			{
				if(!folder.exists())
					folder.create(true, true, null);
			} 
			catch (CoreException e1) 
			{
				e1.printStackTrace();
			}
			
			//Create the systems folder if it does not exist.
			folder = ResourcesPlugin.getWorkspace().getRoot().getProject(project_name).getFolder("/src/systems/");
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
			folder = ResourcesPlugin.getWorkspace().getRoot().getProject(project_name).getFolder("/src/systems/" + component_name + "/");
			try 
			{
				if(!folder.exists())
					folder.create(true, true, null);
			}
			catch (CoreException e1) 
			{
				e1.printStackTrace();
			}
			
			//Create the actual file with the text created.
			if(createFile(project_name, component_name, file, data) == false)
				return false;
			
			String filename = file.getProject() + "/" + file.getProjectRelativePath().toOSString();
			filename = filename.substring(2, filename.length());
			EclipseResourceUtils.openLocalFileInEditor(filename);
			refreshProjects();
	
			return true;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
	}

	public void init(IWorkbench workbench, IStructuredSelection selection) 
	{
	}
}