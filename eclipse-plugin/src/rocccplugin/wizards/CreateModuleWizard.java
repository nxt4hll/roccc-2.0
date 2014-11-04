package rocccplugin.wizards;

import java.io.ByteArrayInputStream;
import java.util.HashMap;
import java.util.Vector;

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
import org.eclipse.swt.widgets.Table;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

import rocccplugin.Activator;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.StringUtils;
import rocccplugin.wizardpages.CreateModuleWizardPage;

public class CreateModuleWizard extends Wizard implements INewWizard 
{
	//The only page for the wizard.
	private CreateModuleWizardPage createModulePage;
	
	public CreateModuleWizard() 
	{
	}
	
	//Adds the addCreateModulePage to the page list of the wizard.
	@Override
	public void addPages() 
	{
		//Add the create module page to the page list.
		createModulePage = new CreateModuleWizardPage("Module Information Page");
		addPage(createModulePage);
		this.setDefaultPageImageDescriptor(ImageDescriptor.createFromImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/newModule.png"))));
	}
	
	//Checks for valid input on the component wizard.
	private boolean validInputs()
	{
		try
		{
			Table ports = createModulePage.ports;
			String compName = createModulePage.component_name.getText();
			int numComponentsInDatabase;
			//Check to see the name of the component starts with a alphabetic character.
			
			if(compName.length() == 0)
			{
				//It did not start with an alphabetic character.
				MessageDialog.openInformation(new Shell(), "Component Name Error", "No name was typed for the component name.");	
				return false;
			}
			
			if(!Character.isLetter(compName.charAt(0)))
			{
				//It did not start with an alphabetic character.
				MessageDialog.openInformation(new Shell(), "Component Name Error", "The name \"" + compName + "\" is an invalid component name.\n\n" +
		                                                   "Make sure the component name starts with an alphabetic character.\n\n" +
		                                                   "Note: Names are not case sensitive.");	
				return false;
			}
			
			if(StringUtils.isCPlusPlusReservedWord(compName))
			{
				MessageDialog.openInformation(new Shell(), "Component Name Error", "Component name \"" + compName + "\" is reserved by C++.\n\n" +
		                                                   "Make sure the component name is not reserved by C++.");
				return false;
			}
			
			if(StringUtils.isROCCCReservedWord(compName))
			{
				MessageDialog.openInformation(new Shell(), "Component Name Error", "Component name \"" + compName + "\" is reserved by ROCCC.\n\n" +
		                                                   "Make sure the component name does not start with \"ROCCC\" or \"suifTmp\".");
				return false;
			}
			
			if(compName.equalsIgnoreCase("hi_cirrf"))
			{
				//The name cannot be the keyword hi_cirrf.
				MessageDialog.openInformation(new Shell(), "Component Name Error", "The name hi_cirrf is a ROCCC reserved file name and cannot be used.");	
				return false;
			}
			
			if(DatabaseInterface.openConnection())
				numComponentsInDatabase = DatabaseInterface.getNumComponents();
			else
				return true;
			
			String[] componentsInDatabase = DatabaseInterface.getAllComponents();
			
			//Check to see if there is a component in the database with the same already.
			for(int i = 0; i < componentsInDatabase.length; ++i)
			{
				if(componentsInDatabase[i].compareToIgnoreCase(compName) == 0)
				{
					//There was a component in the database with the same name, display error.
					boolean continueCreation = MessageDialog.openQuestion(null, "Matching Name", "The module name matches a name in the component database." +
																		  						 "\n\nDo you wish to continue?");
					if(!continueCreation)
						return false;
				}
				
				String structName = DatabaseInterface.getStructName(componentsInDatabase[i]);
				if(structName != null && structName.equals(compName))
				{
					//There was a component in the database with the same name, display error.
					MessageDialog.openError(null, "Matching Name", "The module name matches a struct name in the component database." +
																		  						 "\n\nPlease choose a different module name.");
					return false;
				}
			}
			
			Vector<String> v = new Vector<String>();
			
			for(int i = 0; i < ports.getItemCount(); ++i)
			{			
				boolean inputPort = ports.getItem(i).getText(1).equals("in");
				
				if((inputPort && ports.getItem(i).getText(0).concat("_in").equals(compName)) || (!inputPort && ports.getItem(i).getText(0).concat("_out").equals(compName)))
				{
					//Component name matches a port name
					MessageDialog.openError(new Shell(), "Matching Error", "The port \"" + ports.getItem(i).getText(0) + "\" will match the component name when its suffix is added. No ports can end up having the same name as the component.");
					return false;
				}
				
				if(v.contains(ports.getItem(i).getText(0)))
				{
					//We have matching port names.
					MessageDialog.openError(new Shell(), "Port Name Error", "There are more than one port with the same name: \"" + ports.getItem(i).getText(0) + "\"\n\n" +
	                									 "Make sure each port has a unique name.");
					return false;
				}
				else
					v.add(ports.getItem(i).getText(0));
			
				if(StringUtils.isCPlusPlusReservedWord(ports.getItem(i).getText(0)))
				{
					//It did not start with an alphabetic character.
					MessageDialog.openInformation(new Shell(), "Port Name Error", "Port name \"" + ports.getItem(i).getText(0) + "\" is reserved by C++.\n\n" +
			                                                   "Make sure the port names are not reserved by C++.");
					return false;
				}
				
				if(StringUtils.isROCCCReservedWord(ports.getItem(i).getText(0)))
				{
					MessageDialog.openInformation(new Shell(), "Port Name Error", "Port name \"" + ports.getItem(i).getText(0) + "\" is reserved by ROCCC.\n\n" +
                    										   "Make sure the port names are not reserved by ROCCC.");
					return false;
				}
				
				if(ports.getItem(i).getText(0).length() == 0)
				{
					//the port name was blank.
					MessageDialog.openError(new Shell(), "Port Name Error", "One of the ports has no name." +
	                									 "Make sure each port has a unique name with at least one character.");
					return false;
				}
				
				if(!Character.isLetter(ports.getItem(i).getText(0).charAt(0)))
				{
					//The first character was not a letter.
					MessageDialog.openError(new Shell(), "Port Name Error", "Port Name \"" + ports.getItem(i).getText(0) + "\" is invalid.\n\n" +
	                									 "Make sure the port name starts with an alphabetic character.");
					return false;
				}
				
				for(int j = 0; j < ports.getItem(i).getText(0).length(); ++j)
				{
					if(!Character.isLetter(ports.getItem(i).getText(0).charAt(j)) &&
					   !Character.isDigit(ports.getItem(i).getText(0).charAt(j)) &&
					   ports.getItem(i).getText(0).charAt(j) != '_')
					{
						//It did not start with an alphabetic character.
						MessageDialog.openError(new Shell(), "Port Name Error", "Port name \"" + ports.getItem(i).getText(0) + "\" is invalid.\n\n" +
				                                             "Make sure the port name has no special characters.");
						return false;
					}
				}
				
				if(ports.getItem(i).getText(2).length() == 0)
				{
					//Port size was blank.
					MessageDialog.openError(new Shell(), "Port Size Error", "One of the port sizes has no value." +
	                									 "Make sure each port size is a positive integer.");
					return false;
				}	
				
				for(int j = 0; j < ports.getItem(i).getText(2).length(); ++j)
				{
					if(!Character.isDigit(ports.getItem(i).getText(2).charAt(j)))
					{
						MessageDialog.openError(new Shell(), "Port Size Error", "Port size \"" + ports.getItem(i).getText(2) + "\" is invalid.\n\n" +
                        									 "Make sure the port size is a positive integer.");
						return false;
					}
				}
				
				if(Integer.parseInt(ports.getItem(i).getText(2)) <= 0)
				{
					//Port size is not a positive integer.
					MessageDialog.openError(new Shell(), "Port Size Error", "One of the port sizes has a non positive value.\n\n" +
	                									 "Make sure each port size is a positive integer.");
					return false;
				}
				
				if(ports.getItem(i).getText(3).equals("float"))
				{
					int bitSize = Integer.parseInt(ports.getItem(i).getText(2));
					if(bitSize != 16 && bitSize != 32 && bitSize != 64)
					{
						MessageDialog.openError(new Shell(), "Float Port Size Error", "One of the ports is a float and does not have a port size of 16, 32 or 64.\n\nAll floats must be either 16, 32, or 64 bits.");
						
						return false;
					}
				}
				
			}
		}
		catch(java.lang.Exception e)
		{
			System.out.println(e.getMessage());
		}
		
		return true;
	}

	public boolean performFinish() 
	{
		if(!validInputs())
			return false;
		
		String project_name = createModulePage.project.getItem(createModulePage.project.getSelectionIndex());
		String component_name = createModulePage.component_name.getText();
		String data = "";
		
		HashMap<String, String> size_typedefs = new HashMap<String, String>();
		for(int i = 0; i < createModulePage.ports.getItems().length; ++i)
		{
			String size = createModulePage.ports.getItem(i).getText(2);
			String type = createModulePage.ports.getItem(i).getText(3);
			if(type.equals("int"))
				size_typedefs.put(type + size, "ROCCC_int" + size);
			else
				size_typedefs.put(type + size, "ROCCC_float" + size);
		}
		for(int i = 0; i < size_typedefs.size(); ++i)
		{
			String[] a = new String[size_typedefs.size()];
			String typeDef = size_typedefs.values().toArray(a)[i];
			data += "typedef " + (typeDef.contains("int")? "int" : typeDef.contains("64")? "double" : "float") + " " + typeDef + " ;\n";
		}
		
		data += "\n";/* +
		        "typedef struct\n" +
		        "{\n" +
		        "\t//inputs\n";
		for(int i = 0; i < createModulePage.ports.getItems().length; ++i)
		{
			String size_type = size_typedefs.get(createModulePage.ports.getItem(i).getText(3) + createModulePage.ports.getItem(i).getText(2));
			if(createModulePage.ports.getItem(i).getText(1).compareTo("in") == 0)
				data += "\t" + size_type + " " + createModulePage.ports.getItem(i).getText(0) + "_in ;\n";
		}
		data += "\t//outputs\n";
		for(int i = 0; i < createModulePage.ports.getItems().length; ++i)
		{
			String size_type = size_typedefs.get(createModulePage.ports.getItem(i).getText(3) + createModulePage.ports.getItem(i).getText(2));
			if(createModulePage.ports.getItem(i).getText(1).compareTo("out") == 0)
				data += "\t" + size_type + " " + createModulePage.ports.getItem(i).getText(0) + "_out ;\n";
		}
		data += "} " + component_name + "_t ;\n";*/
		
		data += "void " + component_name + "(";
		
		int numInputs = 0;
		int numOutputs = 0;
		for(int i = 0; i < createModulePage.ports.getItems().length; ++i)
		{
			String size_type = size_typedefs.get(createModulePage.ports.getItem(i).getText(3) + createModulePage.ports.getItem(i).getText(2));
			if(createModulePage.ports.getItem(i).getText(1).compareTo("in") == 0)
			{
				if(numInputs > 0)
					data += ", ";
				data += size_type + " " + createModulePage.ports.getItem(i).getText(0);
				++numInputs;
			}
		}

		for(int i = 0; i < createModulePage.ports.getItems().length; ++i)
		{
			String size_type = size_typedefs.get(createModulePage.ports.getItem(i).getText(3) + createModulePage.ports.getItem(i).getText(2));
			if(createModulePage.ports.getItem(i).getText(1).compareTo("out") == 0)
			{
				if(numOutputs > 0 || numInputs > 0)
					data += ", ";
				data += size_type + "& " + createModulePage.ports.getItem(i).getText(0);
				++numOutputs;
			}
		}
		
		data += ")\n" +
				"{\n" +
				"\t\n" +
				"}\n";
		StringBuffer StringBuffer1 = new StringBuffer(data);
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
		
		//Create the modules folder if it does not exist.
		folder = ResourcesPlugin.getWorkspace().getRoot().getProject(project_name).getFolder("/src/modules/");
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
		folder = ResourcesPlugin.getWorkspace().getRoot().getProject(project_name).getFolder("/src/modules/" + component_name + "/");
		try 
		{
			if(!folder.exists())
				folder.create(true, true, null);
		} 
		catch (CoreException e1) 
		{
			e1.printStackTrace();
		}
		IFile file = ResourcesPlugin.getWorkspace().getRoot().getProject(project_name).getFile("/src/modules/" + component_name + "/" + component_name + ".c");
		
		try
		{
			ByteArrayInputStream Bis1 = new ByteArrayInputStream(StringBuffer1.toString().getBytes("UTF-8"));
			file.create(Bis1, true, null);
		}
		catch(java.lang.Exception e)
		{
			MessageDialog.openInformation(new Shell(), "Error", "There was an error creating your file.\n" +
																"Make sure the file does not already exist in the project.");
		}
		
		try
		{
			//Update the project list to show the new VHDL generated.
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
		
		try
		{
			String filename = file.getProject() + "/" + file.getProjectRelativePath().toOSString();
			filename = filename.substring(2, filename.length());
			EclipseResourceUtils.openLocalFileInEditor(filename);
			//IEditorDescriptor desc = PlatformUI.getWorkbench().getEditorRegistry().getDefaultEditor("file.java");
			//IWorkbenchPage page = PlatformUI.getWorkbench().getWorkbenchWindows()[0].getActivePage();

			//page.openEditor(new FileEditorInput(file), desc.getId());
		}
		catch (Exception err) 
		{
			err.printStackTrace();
		}
		
		return true;
	}

	public void init(IWorkbench workbench, IStructuredSelection selection) 
	{
	}
}
