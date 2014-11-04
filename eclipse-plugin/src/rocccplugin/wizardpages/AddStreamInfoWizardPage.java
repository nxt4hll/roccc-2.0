package rocccplugin.wizardpages;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import rocccplugin.Activator;
import rocccplugin.utilities.CompositeUtilities;

public class AddStreamInfoWizardPage extends WizardPage
{
	public Text streamName;
	public Text elementsRead;
	public Text memoryRequests;
	public Button maximizeThroughput;
	boolean inputStream;
	
	
	public AddStreamInfoWizardPage(String pageName, boolean inputStream) 
	{
		super(pageName);
		this.inputStream = inputStream;
		setTitle("Add System Stream Info");
		setDescription("Enter the " + (inputStream? "input" : "output") + " stream info.");
	}

	public void createControl(Composite parent) 
	{
		Composite top = CompositeUtilities.createDefaultComposite(parent, 1, false);
		setControl(top);
		
		Group group1 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group1.setText("Stream Info");
		group1.setLayout(new GridLayout());
		group1.setLayoutData(CompositeUtilities.createNewGD(0));
		Composite composite1 = CompositeUtilities.createDefaultComposite(group1,1, false);

		createMainGroup(group1);
	}
	
	private void createMainGroup(Composite parent)
	{
		Composite infoSection = CompositeUtilities.createDefaultComposite(parent, 2, false);
		
		new Label(infoSection, SWT.NONE).setText((inputStream? "Input" : "Output") + " Stream:");
		streamName = new Text(infoSection,SWT.BORDER);
		streamName.setLayoutData(CompositeUtilities.createNewGD(0));
		streamName.setText("/* " + (inputStream? "Input" : "Output") + " Stream Name */");
		
		if(inputStream)
			new Label(infoSection, SWT.NONE).setText("Number of Parallel Data Channels:");
		else
			new Label(infoSection, SWT.NONE).setText("Number of Parallel Data Channels:");
		elementsRead = new Text(infoSection,SWT.BORDER);
		elementsRead.setLayoutData(CompositeUtilities.createNewGD(0));
		if(inputStream)
			elementsRead.setText("/* Number of Data Channels */");
		else
			elementsRead.setText("/* Number of Data Channels */");
		//if(inputStream)
		{
			new Label(infoSection, SWT.NONE).setText("Number of Parallel Address Channels:");
			memoryRequests = new Text(infoSection,SWT.BORDER);
			memoryRequests.setLayoutData(CompositeUtilities.createNewGD(0));
			memoryRequests.setText("/* Number of Address Channels */");
			
			/*new Label(infoSection, SWT.NONE).setText("Maximize Throughput");
			maximizeThroughput = new Button(infoSection, SWT.CHECK);
			maximizeThroughput.setSelection(false);*/
		}
	}

}
