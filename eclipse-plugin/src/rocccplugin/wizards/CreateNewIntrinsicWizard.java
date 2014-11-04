package rocccplugin.wizards;

import java.io.ByteArrayInputStream;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

import rocccplugin.Activator;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.StringUtils;
import rocccplugin.wizardpages.CreateNewIntrinsicWizardPage;

public class CreateNewIntrinsicWizard extends Wizard implements INewWizard 
{
	CreateNewIntrinsicWizardPage page;
	
	public CreateNewIntrinsicWizard() 
	{
		
	}

	public void addPages() 
	{
		page = new CreateNewIntrinsicWizardPage("Create New Intrinsic");
		addPage(page);
	} 
	
	public boolean performFinish()
	{
		String project = page.project.getText();
		String bitSize = page.bitSize.getText();  
		String bitSize2 = page.bitSize2.getText();
		String name = page.name.getText();
		String type = page.intrinsicTypes.getSelection()[0].getText();
		
		if(project.equals(""))
		{
			MessageDialog.openError(new Shell(), "No Project Error", "Please select a project to add this instrinsic to.");
			return false;
		}
		
		if(!StringUtils.isPositiveInt(bitSize))
		{
			MessageDialog.openError(new Shell(), "Bit Size Error", "The bit size " + bitSize + " must be a positive integer.");
			return false;
		}
		
		if(page.bitSize2.isVisible() && !StringUtils.isPositiveInt(bitSize2))
		{
			MessageDialog.openError(new Shell(), "Bit Size Error", "The bit size " + bitSize2 + " must be a positive integer.");
			return false;
		}
		
		if(type.startsWith("fp") && !bitSize.equals("16") && !bitSize.equals("32") && !bitSize.equals("64"))
		{
			MessageDialog.openError(new Shell(), "Floating Point Error", "The bit size " + bitSize + " is invalid for floating points. Floating points can only have a bitsize of 16, 32, or 64.");
			return false;
		}
		
		if(type.endsWith("fp") && page.bitSize2.isVisible() && !bitSize2.equals("16") && !bitSize2.equals("32") && !bitSize2.equals("64"))
		{
			MessageDialog.openError(new Shell(), "Floating Point Error", "The bit size " + bitSize2 + " is invalid for floating points. Floating points can only have a bitsize of 16, 32, or 64.");
			return false;
		}
		
		if(DatabaseInterface.doesComponentExist(name))
		{
			if(!MessageDialog.openQuestion(new Shell(), "Database Error", "A component of this name exists in the database. Continue anyways?"))
				return false;
		}
		if(!StringUtils.isValidVariableName(name))
		{
			MessageDialog.openError(new Shell(), "Name Error", "The name \"" + name + "\" is invalid. No special characters or a leading number or underscore is allowed.");
			return false;
		}
		
		generateFile();
		
		return true;
	}
	
	private void generateFile()
	{
		String project = page.project.getText();
		int bitSize = Integer.parseInt(page.bitSize.getText());
		int bitSize2 = 0;
		
		if(page.bitSize2.isVisible())
			bitSize2 = Integer.parseInt(page.bitSize2.getText());
		
		String name = page.name.getText();
		String type = page.intrinsicTypes.getSelection()[0].getText();
		
		String[] ports = new String[]{"a", "b", "result"};
		
		if(type.equals("int_div"))
			ports = new String[]{"dividend", "divisor", "quotient"};
		
		String tab = "  ";
		StringBuffer buffer = new StringBuffer();
	
		if(type.startsWith("int"))
			buffer.append("typedef int ROCCC_int" + bitSize + " ;\n");	
		else
			buffer.append("typedef " + (bitSize == 64? "double" : "float") + " ROCCC_float" + bitSize + " ;\n");
		
		if(page.bitSize2.isVisible())
		{
			if(type.endsWith("fp"))
				buffer.append("typedef " + (bitSize2 == 64? "double" : "float") + " ROCCC_float" + bitSize2 + " ;\n");
			else
				buffer.append("typedef int ROCCC_int" + bitSize2 + " ;\n");
		}
			
		if(type.equals("fp_to_int"))
		{
			buffer.append("\n");
			buffer.append("typedef struct\n{\n");
			buffer.append(tab + "//inputs\n");
			buffer.append(tab + "ROCCC_float" + bitSize + " " + "fp_in ;\n");
			buffer.append(tab + "//outputs\n");
			buffer.append(tab + "ROCCC_int" + bitSize2 + " " + "int_out ;\n");
		}
		else if(type.equals("int_to_fp"))
		{
			buffer.append("\n");
			buffer.append("typedef struct\n{\n");
			buffer.append(tab + "//inputs\n");
			buffer.append(tab + "ROCCC_int" + bitSize + " int_in ;\n");
			buffer.append(tab + "//outputs\n");
			buffer.append(tab + "ROCCC_float" + bitSize2 + " fp_out ;\n");
		}
		else if(type.equals("fp_to_fp"))
		{
			buffer.append("\n");
			buffer.append("typedef struct\n{\n");
			buffer.append(tab + "//inputs\n");
			buffer.append(tab + "ROCCC_float" + bitSize + " fp_in ;\n");
			buffer.append(tab + "//outputs\n");
			buffer.append(tab + "ROCCC_float" + bitSize2 + " fp_out ;\n");
		}
		else if(type.contains("equal") || type.contains("than"))
		{
			buffer.append("typedef int ROCCC_int1 ;\n\n");
			buffer.append("typedef struct\n{\n");
			buffer.append(tab + "//inputs\n");
			buffer.append(tab + "ROCCC_float" + bitSize + " " + ports[0] + "_in ;\n");
			buffer.append(tab + "ROCCC_float" + bitSize + " " + ports[1] + "_in ;\n");
			buffer.append(tab + "//outputs\n");
			buffer.append(tab + "ROCCC_int" + 1 + " " + ports[2] + "_out ;\n");
		}
		else if(type.startsWith("fp"))
		{
			buffer.append("\n");
			buffer.append("typedef struct\n{\n");
			buffer.append(tab + "//inputs\n");
			buffer.append(tab + "ROCCC_float" + bitSize + " " + ports[0] + "_in ;\n");
			buffer.append(tab + "ROCCC_float" + bitSize + " " + ports[1] + "_in ;\n");
			buffer.append(tab + "//outputs\n");
			buffer.append(tab + "ROCCC_float" + bitSize + " " + ports[2] + "_out ;\n");
		}
		else
		{
			buffer.append("\n");
			buffer.append("typedef struct\n{\n");
			buffer.append(tab + "//inputs\n");
			buffer.append(tab + "ROCCC_int" + bitSize + " " + ports[0] + "_in ;\n");
			buffer.append(tab + "ROCCC_int" + bitSize + " " + ports[1] + "_in ;\n");
			buffer.append(tab + "//outputs\n");
			buffer.append(tab + "ROCCC_int" + bitSize + " " + ports[2] + "_out ;\n");
		}
		
		buffer.append("} " + name + "_t ;\n\n");
		
		buffer.append(name + "_t " + name + "(" + name + "_t t)\n{\n\n");
		
		buffer.append(tab + "return t;\n}\n");
		
		IFolder folder = ResourcesPlugin.getWorkspace().getRoot().getProject(project).getFolder("/src/");
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
		folder = ResourcesPlugin.getWorkspace().getRoot().getProject(project).getFolder("/src/intrinsics/");
		try 
		{
			if(!folder.exists())
				folder.create(true, true, null);
		} 
		catch (CoreException e1) 
		{
			e1.printStackTrace();
		}
		
		folder = ResourcesPlugin.getWorkspace().getRoot().getProject(project).getFolder("/src/intrinsics/" + type + "/");
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
		folder = ResourcesPlugin.getWorkspace().getRoot().getProject(project).getFolder("/src/intrinsics/" + type + "/" + name + "/");
		try 
		{
			if(!folder.exists())
				folder.create(true, true, null);
		} 
		catch (CoreException e1) 
		{
			e1.printStackTrace();
		}
		IFile file = ResourcesPlugin.getWorkspace().getRoot().getProject(project).getFile("/src/intrinsics/" + type + "/" + name + "/" + name + ".c");
		
		try
		{
			ByteArrayInputStream Bis1 = new ByteArrayInputStream(buffer.toString().getBytes("UTF-8"));
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
