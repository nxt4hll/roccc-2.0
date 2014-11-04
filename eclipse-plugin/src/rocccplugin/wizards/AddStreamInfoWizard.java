package rocccplugin.wizards;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

import rocccplugin.utilities.StringUtils;
import rocccplugin.wizardpages.AddStreamInfoWizardPage;

public class AddStreamInfoWizard extends Wizard implements INewWizard 
{
	private AddStreamInfoWizardPage wizardPage;
	public String streamName;
	public String elementsRead;
	public String memoryRequests;
	public boolean maximizeThroughput;
	boolean inputStream;
	
	public AddStreamInfoWizard(boolean inputStream)
	{
		this.inputStream = inputStream;
		this.setDefaultPageImageDescriptor(ImageDescriptor.createFromImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/buildToHardware.png"))));
	}
	
	public void addPages() 
	{
		//Add the component page to the page list.
		wizardPage = new AddStreamInfoWizardPage("Add Stream Info Page", inputStream);
		addPage(wizardPage);
	}
	
	public boolean performFinish() 
	{
		streamName = wizardPage.streamName.getText();
		elementsRead = wizardPage.elementsRead.getText();
		//if(inputStream)
		{
			memoryRequests = wizardPage.memoryRequests.getText();
			//maximizeThroughput = wizardPage.maximizeThroughput.getSelection();
		}
		if(!StringUtils.isValidVariableName(streamName))
		{
			MessageDialog.openError(new Shell(), "Stream Error", "Stream name " + streamName + " is invalid. Check your stream name before continuing.");
			return false;
		}
	
		//Check for valid numeric values.
		if(!StringUtils.isPositiveInt(elementsRead))
		{
			MessageDialog.openError(new Shell(), "Elements Read Error", "Number of data channels is invalid. Value must be a positive integer.");
			return false;
		}
		
		//Check for valid numeric values.
		//if(inputStream)
		{
			if(!StringUtils.isPositiveInt(memoryRequests))
			{
				MessageDialog.openError(new Shell(), "Memory Requests Error", "Number of address channels is invalid. Value must be a positive integer.");
				return false;
			}
			
			if(Integer.parseInt(elementsRead) != Integer.parseInt(memoryRequests) && Integer.parseInt(memoryRequests) != 1)
			{
				MessageDialog.openError(new Shell(), "Stream Error", "Number of address channels must be \"1\" or equal to the number of data channels.");
				return false;
			}
		}

		return true;
	}

	
	
	public void init(IWorkbench workbench, IStructuredSelection selection) 
	{

	}

}
