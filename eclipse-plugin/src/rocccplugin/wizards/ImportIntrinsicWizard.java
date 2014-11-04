package rocccplugin.wizards;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.StringUtils;
import rocccplugin.wizardpages.ImportIntrinsicWizardPage;

public class ImportIntrinsicWizard extends Wizard implements INewWizard 
{

	ImportIntrinsicWizardPage wizardPage;
	String type;
	public boolean temporaryAdd = false;
	public boolean overwriteIntrinsic = false;
	public String name = "";
	public int bitSize = 0;
	public int bitSize2 = 0;
	public boolean bitSize2Visible = false;
	public int delay = 0;
	public boolean active = false;
	public String description = "";
	public int row = 0;
	public String deletedComponent;
	int typeIndex = 0;
	
	
	public ImportIntrinsicWizard(boolean tempAdd, int startingType) 
	{
		temporaryAdd = tempAdd;
		typeIndex = startingType;
		this.setDefaultPageImageDescriptor(ImageDescriptor.createFromImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/intrinsics.png"))));
	}
	
	public void addPages() 
	{
		wizardPage = new ImportIntrinsicWizardPage("Add Intrinsic To Database", typeIndex);
		addPage(wizardPage);
	}

	private boolean validCheck()
	{
		String name = wizardPage.name.getText();
		
		String bitSize = wizardPage.bitSize1.getText();
		String bitSize2 = wizardPage.bitSize2.getText();
		
		int sel = wizardPage.intrinsicTypes.getSelectionIndex();
		type = sel == 0 ? "INT_DIV" : sel == 1 ? "INT_MOD" :  sel == 2 ? "FP_ADD" : sel == 3 ? "FP_SUB" : sel == 4 ? "FP_MUL" : 
			  		  sel == 5 ? "FP_DIV"  : sel == 6 ? "FP_GREATER_THAN" : sel == 7 ? "FP_LESS_THAN" : sel == 8 ? "FP_EQUAL" :
				      sel == 9 ? "FP_GREATER_THAN_EQUAL" : sel == 10 ? "FP_LESS_THAN_EQUAL" : sel == 11? "FP_NOT_EQUAL" : 
				      sel == 12 ? "FP_TO_INT" : sel == 13? "INT_TO_FP" : sel == 14? "FP_TO_FP" : sel == 15? "DOUBLE_VOTE" : "TRIPLE_VOTE";
		
		row = sel;
		
		if(!StringUtils.isPositiveInt(bitSize))
		{
			MessageDialog.openError(new Shell(), "Intrinsic Error", "The bit size " + bitSize + " is invalid. Positive integers only.");
			return false;
		}
		
		if(wizardPage.bitSize2.isEnabled() && !StringUtils.isPositiveInt(bitSize2))
		{
			MessageDialog.openError(new Shell(), "Intrinsic Error", "The bit size " + bitSize2 + " is invalid. Positive integers only.");
			return false;
		}
		
		if(type.startsWith("FP") && !bitSize.equals("16") && !bitSize.equals("32") && !bitSize.equals("64"))
		{
			MessageDialog.openError(new Shell(), "Floating Point Error", "The bit size " + bitSize + " is invalid for floating points. Floating points can only have a bitsize of 16, 32, or 64.");
			return false;
		}
		
		if(type.endsWith("FP") && wizardPage.bitSize2.isEnabled() && !bitSize2.equals("16") && !bitSize2.equals("32") && !bitSize2.equals("64"))
		{
			MessageDialog.openError(new Shell(), "Floating Point Error", "The bit size " + bitSize2 + " is invalid for floating points. Floating points can only have a bitsize of 16, 32, or 64.");
			return false;
		}
		
		String delay = wizardPage.delay.getText();
		if(!StringUtils.isPositiveInt(delay))
		{
			MessageDialog.openError(new Shell(), "Intrinsic Error", "The latency is invalid. Positive integers only.");
			return false;
		}
		
		if(!StringUtils.isValidVariableName(name))
		{
			MessageDialog.openError(new Shell(), "Name Error", "The name \"" + name + "\" is invalid. No special characters or a leading number or underscore is allowed.");
			return false;
		}
		
		if(StringUtils.isCPlusPlusReservedWord(name))
		{
			MessageDialog.openInformation(new Shell(), "Intrinsic Name Error", "Intrinsic name \"" + name + "\" is reserved by C++.\n\n" +
													   "Make sure the intrinsic name that does not start with \"ROCCC\" or \"suifTmp\".");
			return false;
		}
		
		if(StringUtils.isROCCCReservedWord(name))
		{
			MessageDialog.openInformation(new Shell(), "Intrinsic Name Error", "Intrinsic name \"" + name + "\" is reserved by ROCCC.\n\n" +
	                                                   "Make sure the intrinsic name is not reserved by ROCCC.");
			return false;
		}
		
		return true;
	}
	
	public boolean performFinish() 
	{
		try
		{
			if(validCheck() == false)
				return false; 
			
			name = wizardPage.name.getText();
			bitSize = Integer.parseInt(wizardPage.bitSize1.getText());
			bitSize2 = 0;
			
			bitSize2Visible = wizardPage.bitSize2.isVisible();
			
			if(wizardPage.bitSize2.isVisible())
				bitSize2 = Integer.parseInt(wizardPage.bitSize2.getText());
			delay = Integer.parseInt(wizardPage.delay.getText());		
			active = wizardPage.active.getSelection();
			description = wizardPage.description.getText();
			
			//Check for exact copy, if so, delete it
			if(DatabaseInterface.doesComponentExist(name))
			{
				if(!MessageDialog.openQuestion(new Shell(), "Matching Component Name", "There is a component in the database with this same name. Do you want to overwrite it?"))
					return false;
				if(!temporaryAdd)
				{
					DatabaseInterface.removeComponent(name, bitSize, delay);	
				}
				else
					deletedComponent = name;
			}
			
			int numComponentsInDatabase = DatabaseInterface.getNumComponents();
			
			String[] componentsInDatabase = DatabaseInterface.getAllComponents();
			
			//Check to see if there is a component in the database with the same already.
			for(int i = 0; i < componentsInDatabase.length; ++i)
			{	
				String structName = DatabaseInterface.getStructName(componentsInDatabase[i]);
				if(structName != null && structName.equals(name))
				{
					//There was a component in the database with the same name, display error.
					MessageDialog.openError(new Shell(), "Matching Name Error", "There is a struct name in the " +
							                             "database with the name \"" + name + "\" already.\n\n" + 
							                             "Choose a different component name.");
					return false;
				}
			}
			
			if(!temporaryAdd)
			{
				//Is it active? Inactivate others with same bitSizes.
				if(active)
				{
					if(wizardPage.bitSize2.isVisible())
						DatabaseInterface.deactivateComponents(type, bitSize, bitSize2);
					else
						DatabaseInterface.deactivateComponents(type, bitSize);
				}
				
				if(type.contains("VOTE"))
					DatabaseInterface.addVoterIntrinsic(name, type, bitSize, delay, active, description);
				else if(wizardPage.bitSize2.isVisible())
					DatabaseInterface.addIntrinsic(name, type, bitSize, bitSize2, delay, active, description);
				else
					DatabaseInterface.addIntrinsic(name, type, bitSize, delay, active, description);
			}
			return true;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
	}

	public void init(IWorkbench workbench, IStructuredSelection selection) 
	{

	}

}
