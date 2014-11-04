package rocccplugin.wizards;

import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.graphics.Image;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

import rocccplugin.actions.ProjectCreator;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.wizardpages.CreateProjectWizardPage;

public class CreateProjectWizard extends Wizard implements INewWizard 
{
	CreateProjectWizardPage createProjectWizardPage;

	public CreateProjectWizard() 
	{
		setWindowTitle("New ROCCC Project");
	} 
	
	public boolean performFinish()
	{
		String projectName = createProjectWizardPage.projectName.getText();
		
		if(ProjectCreator.createProject(projectName))
		{
			MessageUtils.printlnConsoleSuccess("Project \"" + projectName + "\" created.\n");
			return true;
		}
		else
		{
			MessageUtils.openErrorWindow("Project Error", "The project name " + projectName + " either exists or is invalid. Please choose a different name.");
			return false;
		}
	}
	
	public void addPages() 
	{
		createProjectWizardPage = new CreateProjectWizardPage("New Project Page");
		addPage(createProjectWizardPage);
		this.setDefaultPageImageDescriptor(ImageDescriptor.createFromImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/newProject.png"))));
	}

	public void init(IWorkbench workbench, IStructuredSelection selection)
	{	
	}

}
