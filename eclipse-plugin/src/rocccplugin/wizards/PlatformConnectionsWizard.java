package rocccplugin.wizards;

import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

import rocccplugin.database.DatabaseInterface;
import rocccplugin.wizardpages.PlatformConnectionsWizardPage;

public class PlatformConnectionsWizard extends Wizard implements INewWizard 
{
	
	private String file;
	private PlatformConnectionsWizardPage platformConnectionsWizardPage;

	public PlatformConnectionsWizard(String file) 
	{
		this.file = file;
	}

	@Override
	public boolean performFinish() 
	{
		
		return false;
	}
	
	@Override
	public void addPages() 
	{
		//If the database is not accessible, do nothing.
		if(!DatabaseInterface.openConnection())
			return;
		String title = "Platforms and Connections for " + file;
		
		//Add the component page to the page list.
		platformConnectionsWizardPage = new PlatformConnectionsWizardPage(title);
		addPage(platformConnectionsWizardPage);
	}

	public void init(IWorkbench workbench, IStructuredSelection selection) 
	{
			
	}

}
