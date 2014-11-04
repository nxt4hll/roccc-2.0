package rocccplugin.wizardpages;

import java.io.File;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.widgets.Composite;

import rocccplugin.Activator;
import rocccplugin.composites.OptimizationSelector;
import rocccplugin.composites.OptimizationSelector.OptimizationValueType;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.CompositeUtilities;

public class LoCompilerFlagsWizardPage extends WizardPage 
{
	public OptimizationSelector optimizationSelector;
	private File fileToCompile;
	private File saveLocation;
	
	public LoCompilerFlagsWizardPage(String pageName, File fileToCompile, File saveLocation) 
	{
		super(pageName);
		
		try
		{
			setTitle("Select Low-Level Compiler Flags for " + fileToCompile.getName());
			setDescription("Please select the low level compiler flags for " + fileToCompile.getName());
		
			this.fileToCompile = fileToCompile;
			this.saveLocation = saveLocation;
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}	

	public void createControl(Composite parent) 
	{
		try
		{
			Composite top = CompositeUtilities.createDefaultComposite(parent, 1, false);
			setControl(top);
			
			//Create the optimization selector
			optimizationSelector = new OptimizationSelector(top, fileToCompile, saveLocation, "Low Level", null);
		
			//Add which flags are available and their values and descriptions.
			optimizationSelector.addFlags("ArithmeticBalancing", null, null, new String[]{"Parallelizing optimization that converts chains of arithmetic operations into parallel arithmetic operations.", ""}, null, null, false, false);
			optimizationSelector.addFlags("CopyReduction", null, null, new String[]{"Reschedules pipelined operations in an attempt to minimize registers created.", ""}, null, null, false, false);
			//optimizationSelector.addFlags("CreateDataflowGraph", null, null, new String[]{"Generates a dataflow graph image of the component for analyzation.", ""}, null, null, true, false);
			optimizationSelector.addFlags("FanoutTreeGeneration", new String[]{"Max Fanout"}, new String[]{"/* The maximum fanout of any value in the tree */"}, new String[]{"Guarantees that no variable will have a higher fanout than the specified max fanout.", ""}, new OptimizationValueType[]{OptimizationValueType.AMOUNT}, null, false, false);
			optimizationSelector.addFlags("MaximizePrecision", null, null, new String[]{"Temporary arithmetic results use maximum precision when enabled and possibly truncate at every step when not.", ""}, null, null, false, false);

			//Give the preference that houses the default flags for this page.
			optimizationSelector.setDefaultsPreference(PreferenceConstants.DEFAULT_LOW_OPTIMIZATIONS);
			
			//Load any previous flags that were saved on a previous compile.
			optimizationSelector.loadExistingFlags();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
	}	
	
	public void saveFlags()
	{
		optimizationSelector.saveFlags();
	}
	
	public String getSavedFile()
	{
		return saveLocation.getAbsolutePath();
	}
}


