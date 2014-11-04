package rocccplugin.wizards;

import java.util.Vector;

import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.graphics.Image;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

import rocccplugin.composites.OptimizationSelector.OptimizationInfo;
import rocccplugin.composites.OptimizationSelector.OptimizationValueType;
import rocccplugin.helpers.ResourceData;
import rocccplugin.wizardpages.CompilerFlagsWizardPage;
import rocccplugin.wizardpages.OptimizationArgumentWizardPage;

public class OptimizationArgumentWizard extends Wizard implements INewWizard 
{
	OptimizationArgumentWizardPage wizardPage;
	String optimization;
	OptimizationInfo optimizationInfo;
	Vector<ResourceData> functionsCalled;
	
	public String value1;
	public String value2;
	
	String[] initialValues;
	
	public OptimizationArgumentWizard(String optimization, OptimizationInfo optimizationInfo, Vector<ResourceData> functionsCalled, String[] initialValues) 
	{
		this.optimization = optimization;
		this.optimizationInfo = optimizationInfo;
		this.functionsCalled = functionsCalled;
		this.setDefaultPageImageDescriptor(ImageDescriptor.createFromImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/buildToHardware.png"))));		
		this.initialValues = initialValues;
	}

	@Override
	public void addPages() 
	{
		try
		{
			wizardPage = new OptimizationArgumentWizardPage(optimization, optimizationInfo, initialValues);
			addPage(wizardPage);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
		
	public boolean performFinish() 
	{
		if(optimizationInfo.optimizationValueTypes != null && optimizationInfo.optimizationValueTypes[0] == OptimizationValueType.AMOUNT)
		{
			value1 = wizardPage.argValue1.getText();
		}
		else if(optimizationInfo.optimizationValueTypes != null && optimizationInfo.optimizationValueTypes[0] == OptimizationValueType.SELECTION)
		{
			value1 = wizardPage.selection1.getText();
		}
		
		if(optimizationInfo.optimizationValueTypes != null && optimizationInfo.optimizationValueTypes.length > 1 && optimizationInfo.optimizationValueTypes[1] == OptimizationValueType.AMOUNT)
		{
			value2 = wizardPage.argValue2.getText();
		}
		else if(optimizationInfo.optimizationValueTypes != null && optimizationInfo.optimizationValueTypes.length > 1 && optimizationInfo.optimizationValueTypes[1] == OptimizationValueType.SELECTION)
		{
			value2 = wizardPage.selection2.getText();
		}
		
		return true;
	}

	public void init(IWorkbench workbench, IStructuredSelection selection) 
	{

	}
}
