package rocccplugin.wizardpages;

import java.util.Vector;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import rocccplugin.Activator;
import rocccplugin.composites.OptimizationSelector.OptimizationInfo;
import rocccplugin.composites.OptimizationSelector.OptimizationValueType;
import rocccplugin.helpers.ResourceData;
import rocccplugin.utilities.CompositeUtilities;

public class OptimizationArgumentWizardPage extends WizardPage 
{
	String optimization;
	OptimizationInfo optimizationInfo;
	Vector<ResourceData> functionsCalled;
	
	public Text argValue1;
	public Text argValue2;
	
	public Combo selection1;
	public Combo selection2;
	
	String[] initialValues;
	
	public OptimizationArgumentWizardPage(String optimization, OptimizationInfo optimizationInfo) 
	{
		super("OptimizationArgumentWizardPage");
		
		setTitle("Optimization Arguments");
		setDescription("Select the values for the optimization " + optimization);

		this.optimization = optimization;
		this.optimizationInfo = optimizationInfo;
	}
	
	public OptimizationArgumentWizardPage(String optimization, OptimizationInfo optimizationInfo, String[] initialValues) 
	{
		super("OptimizationArgumentWizardPage");
		
		setTitle("Optimization Arguments");
		setDescription("Select the values for the optimization " + optimization);

		this.optimization = optimization;
		this.optimizationInfo = optimizationInfo;
		
		this.initialValues = initialValues;
	}

	public void createControl(Composite parent) 
	{
		try
		{
			Composite top = CompositeUtilities.createDefaultComposite(parent,1, false);
			setControl(top);
			
			Composite argumentsComposite = CompositeUtilities.createDefaultComposite(top, 2, false);
			argumentsComposite.setLayoutData(CompositeUtilities.createNewGD(SWT.NONE));
		
			Label argName1 = new Label(argumentsComposite, SWT.NONE);
			argName1.setText(optimizationInfo.arguments[0] + ": ");
			
			//Create a text box if the argument value is not a function name or label.
			if(optimizationInfo.optimizationValueTypes != null && optimizationInfo.optimizationValueTypes.length > 0 && optimizationInfo.optimizationValueTypes[0].equals(OptimizationValueType.SELECTION))
			{
				selection1 = new Combo (argumentsComposite, SWT.READ_ONLY);
				selection1.setLayoutData(CompositeUtilities.createNewGD(0));
				
				for(int i = 0; i < optimizationInfo.selectionValues[0].length; ++i)
				{
					selection1.add(optimizationInfo.selectionValues[0][i]);
				}
				selection1.select(0);
				
				if(initialValues != null && initialValues.length > 0)
				{
					for(int i = 0; i < optimizationInfo.selectionValues[0].length; ++i)
					{
						if(optimizationInfo.selectionValues[0][i].equals(initialValues[0]))
						{
							selection1.select(i);
							break;
						}
					}
				}
				
			}
			else if(optimizationInfo.optimizationValueTypes != null && optimizationInfo.optimizationValueTypes.length > 0)
			{
				argValue1 = CompositeUtilities.createTextBox(argumentsComposite, optimizationInfo.argumentDefaultValues[0]);
				
				if(initialValues != null && initialValues.length > 0)
					argValue1.setText(initialValues[0]);
			}
			//Create a text box if the argument value is not a function name or label.
			if(optimizationInfo.optimizationValueTypes != null && optimizationInfo.optimizationValueTypes.length > 1 && optimizationInfo.optimizationValueTypes[1].equals(OptimizationValueType.SELECTION))
			{
				Label argName2 = new Label(argumentsComposite, SWT.NONE);
				argName2.setText(optimizationInfo.arguments[1] + ": ");
				
				selection2 = new Combo (argumentsComposite, SWT.READ_ONLY);
				selection2.setLayoutData(CompositeUtilities.createNewGD(0));
				
				for(int i = 0; i < optimizationInfo.selectionValues[1].length; ++i)
				{
					selection2.add(optimizationInfo.selectionValues[1][i]);
				}
				selection2.select(0);	
				
				if(initialValues != null && initialValues.length > 1)
				{
					for(int i = 0; i < optimizationInfo.selectionValues[1].length; ++i)
					{
						if(optimizationInfo.selectionValues[1][i].equals(initialValues[1]))
						{
							selection2.select(i);
							break;
						}
					}
				}
				
			}
			else if(optimizationInfo.optimizationValueTypes != null && optimizationInfo.optimizationValueTypes.length > 1)
			{
				Label argName2 = new Label(argumentsComposite, SWT.NONE);
				argName2.setText(optimizationInfo.arguments[1] + ": ");
				
				argValue2 = CompositeUtilities.createTextBox(argumentsComposite, optimizationInfo.argumentDefaultValues[1]);
				
				if(initialValues != null && initialValues.length > 1)
					argValue2.setText(initialValues[1]);
			}
		}

		catch(Exception e)
		{
			e.printStackTrace();
		}
	}

}
