package rocccplugin.wizardpages;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import rocccplugin.Activator;
import rocccplugin.utilities.CompositeUtilities;

public class CreateProjectWizardPage extends WizardPage 
{
	public Text projectName;

	public CreateProjectWizardPage(String pageName) 
	{
		super(pageName);
		setTitle("New ROCCC Project");
		setDescription("Please enter the new project's information.\n");
	}

	public void createControl(Composite parent) 
	{
		Composite top = CompositeUtilities.createDefaultComposite(parent, 1, false);
		setControl(top);
		
		Group projectGroup = new Group(top, SWT.NONE);
		projectGroup.setText("Project Details");
		projectGroup.setLayout(new GridLayout());
		projectGroup.setLayoutData(CompositeUtilities.createNewGD(0));
		
		Composite projectComposite = CompositeUtilities.createDefaultComposite(projectGroup, 1, false);
		
		new Label(projectComposite, SWT.NONE).setText("Choose a name for the new project:");
		projectName = CompositeUtilities.createTextField(projectComposite, 400);
		
	}

}
