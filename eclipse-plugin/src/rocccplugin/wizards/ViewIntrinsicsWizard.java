package rocccplugin.wizards;

import java.util.Map;
import java.util.TreeMap;
import java.util.Vector;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.StringUtils;
import rocccplugin.wizardpages.ViewIntrinsicsWizardPage;

public class ViewIntrinsicsWizard extends Wizard implements INewWizard 
{	
	ViewIntrinsicsWizardPage page;
	
	final int NAME_COLUMN = 0;
	final int BITSIZE_COLUMN = 1;
	final int BITSIZE_COLUMN2 = 2;
	final int DELAY_COLUMN = 3;
	final int DESCRIPTION_COLUMN = 4;
	final int ACTIVE_COLUMN = 5;
	
	public ViewIntrinsicsWizard() 
	{
		this.setDefaultPageImageDescriptor(ImageDescriptor.createFromImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/intrinsics.png"))));
	}

	public void addPages() 
	{
		page = new ViewIntrinsicsWizardPage("Intrinsics List");
		addPage(page);
	} 
	
	public boolean performFinish() 
	{
		try
		{
			Vector<Vector<String[]>> intrinsicValues = page.intrinsicValues;
			
			Vector<String> deletedComponents = page.deletedComponents;
			
			for(int i = 0; i < deletedComponents.size(); ++ i)
			{
				DatabaseInterface.removeComponent(deletedComponents.get(i));
			}
			
			for(int i = 0; i < intrinsicValues.size(); ++i)
			{
				Map<String, Vector<String>> map = new TreeMap<String, Vector<String>>();
				
				for(int j = 0; j < intrinsicValues.get(i).size(); ++j)
				{ 
					String name = intrinsicValues.get(i).get(j)[NAME_COLUMN];
					String delay = intrinsicValues.get(i).get(j)[DELAY_COLUMN];
					
					if(!StringUtils.isPositiveInt(delay))
					{
						MessageDialog.openError(new Shell(), "Delay Error", "The delay on intrinsic \"" + name + "\" is invalid. The delay must be a positive integer.");
						return false;
					}	
				}
			}
			
			updateTableValuesToDatabase();
			GuiLockingUtils.unlockGui();
			DatabaseInterface.updateAllListeners();
			
			return true;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
	}
	
	private void updateTableValuesToDatabase()
	{
		Vector<Vector<String[]>> intrinsicValues = page.intrinsicValues;	
		DatabaseInterface.removeIntrinsics();
		
		for(int i = 0; i < intrinsicValues.size(); ++i)
		{
			for(int j = 0; j < intrinsicValues.get(i).size(); ++j)
			{
				String name = intrinsicValues.get(i).get(j)[NAME_COLUMN];
				int bitSize = Integer.parseInt(intrinsicValues.get(i).get(j)[BITSIZE_COLUMN]);
				int bitSize2 = 0;
				if(!intrinsicValues.get(i).get(j)[BITSIZE_COLUMN2].equals("NA"))
					bitSize2 = Integer.parseInt(intrinsicValues.get(i).get(j)[BITSIZE_COLUMN2]);
				int delay = Integer.parseInt(intrinsicValues.get(i).get(j)[DELAY_COLUMN]);
				String description = intrinsicValues.get(i).get(j)[DESCRIPTION_COLUMN];
				if(description == null)
					description = "";
				boolean active = intrinsicValues.get(i).get(j)[ACTIVE_COLUMN].equals("*");
				
				String type = i == 0 ? "INT_DIV" : i == 1 ? "INT_MOD" : i == 2 ? "FP_ADD" : i == 3 ? "FP_SUB" : i == 4 ? "FP_MUL" : 
							  i == 5 ? "FP_DIV"  : i == 6 ? "FP_GREATER_THAN" : i == 7 ? "FP_LESS_THAN" : i == 8 ? "FP_EQUAL" :
							  i == 9 ? "FP_GREATER_THAN_EQUAL" : i == 10 ? "FP_LESS_THAN_EQUAL" : i == 11? "FP_NOT_EQUAL" : 
							  i == 12 ? "FP_TO_INT" : i == 13? "INT_TO_FP" : i == 14? "FP_TO_FP" : i == 15? "DOUBLE_VOTE" : "TRIPLE_VOTE";
				
				if(type.contains("VOTE"))
					DatabaseInterface.addVoterIntrinsic(name, type, bitSize, delay, active, description);
				else if(type.equals("INT_TO_FP") || type.equals("FP_TO_INT") || type.equals("FP_TO_FP"))
					DatabaseInterface.addIntrinsic(name, type, bitSize, bitSize2, delay, active, description);
				else
					DatabaseInterface.addIntrinsic(name, type, bitSize, delay, active, description);
			}
		}
	}
	
	

	public void init(IWorkbench workbench, IStructuredSelection selection) 
	{

	}

}
