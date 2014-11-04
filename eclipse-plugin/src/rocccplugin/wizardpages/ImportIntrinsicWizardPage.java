package rocccplugin.wizardpages;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;

import rocccplugin.Activator;
import rocccplugin.utilities.CompositeUtilities;

public class ImportIntrinsicWizardPage extends WizardPage 
{
	public Table intrinsicTypes;
	public Text name;
	private Label bitSize1Label;
	private Label bitSize2Label;
	public Text bitSize1;
	public Text bitSize2;
	public Text delay;
	public Text description;
	public Button active;
	int typeIndex;
	
	
	public ImportIntrinsicWizardPage(String pageName, int startingIndex) 
	{
		super(pageName);
		typeIndex = startingIndex;
		setTitle(pageName);
		setDescription("Enter the info for the intrinsic you are adding.");
	}

	public void createControl(Composite parent)
	{
		Composite top = CompositeUtilities.createDefaultComposite(parent, 1, false);
		setControl(top);
		Group group1 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group1.setText("Select Intrinsic Properties");
		group1.setLayout(new GridLayout());
		group1.setLayoutData(CompositeUtilities.createNewGD(0));
		
		Composite insideGroup = CompositeUtilities.createDefaultComposite(group1, 2, false);
		
		createIntrinsicTable(insideGroup);
		createProperties(insideGroup);

		intrinsicTypes.select(typeIndex);
		intrinsicTypes.notifyListeners(SWT.Selection, null);
		intrinsicTypes.showSelection();
	}
	
	private void createIntrinsicTable(Composite parent)
	{	
		Composite tableComp = CompositeUtilities.createDefaultComposite(parent, 1, false);
		
		intrinsicTypes = new Table(tableComp, SWT.FULL_SELECTION | SWT.BORDER);
		intrinsicTypes.setHeaderVisible(true);
		intrinsicTypes.setLinesVisible(true);
		TableColumn column = new TableColumn(intrinsicTypes, SWT.NONE);
		column.setWidth(150);
		column.setText("Intrinsic Type");
		
		new TableItem(intrinsicTypes, SWT.NONE).setText("int_div");
		new TableItem(intrinsicTypes, SWT.NONE).setText("int_mod");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_add");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_sub");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_mul");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_div");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_greater_than");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_less_than");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_equal");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_greater_than_equal");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_less_than_equal");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_not_equal");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_to_int");
		new TableItem(intrinsicTypes, SWT.NONE).setText("int_to_fp");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_to_fp");
		new TableItem(intrinsicTypes, SWT.NONE).setText("double_vote");
		new TableItem(intrinsicTypes, SWT.NONE).setText("triple_vote");
		
		intrinsicTypes.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				widgetSelected(e);
			}
			
			public void widgetSelected(SelectionEvent e) 
			{
				String type = intrinsicTypes.getSelection()[0].getText();
				
				if(type.equals("int_to_fp") || 
				   type.equals("fp_to_int") ||
				   type.equals("fp_to_fp"))
				{
					bitSize2.setEnabled(true);
					bitSize2.setText("");
					bitSize2.setVisible(true);
					
					bitSize2Label.setVisible(true);
					
					bitSize2Label.setText(type.equals("fp_to_int")? "Bit Size of int:   " : type.equals("fp_to_fp")? "Bit Size of fp2:   " : "Bit Size of fp:   ");
					bitSize1Label.setText(type.equals("int_to_fp")? "Bit Size of int:   " : type.equals("fp_to_fp")? "Bit Size of fp1:   " : "Bit Size of fp:   ");
					
				}
				else if(type.contains("vote"))
				{
					bitSize2.setEnabled(false);
					bitSize2.setText("");
					bitSize2.setVisible(false);
					bitSize2Label.setText("");
					
					bitSize1Label.setText("Bit Size of value:");
				}
				else
				{
					bitSize2.setEnabled(false);
					bitSize2.setText("");
					bitSize2.setVisible(false);
					bitSize2Label.setText("");
					
					bitSize1Label.setText(type.startsWith("fp")? "Bit Size of fp:" : "Bit Size of int:");
						
				}
			}
			
		});
		
		CompositeUtilities.setCompositeSize(tableComp, intrinsicTypes, intrinsicTypes.getItemHeight() * 7, intrinsicTypes.getItemHeight() * 10);
		
	}

	private void createProperties(Composite parent)
	{
		Composite propertiesComp = CompositeUtilities.createDefaultComposite(parent, 1, false);
		Composite nameComp = CompositeUtilities.createDefaultComposite(propertiesComp, 1, false);
		Composite variablesComp = CompositeUtilities.createDefaultComposite(propertiesComp, 2, false);
		Composite descriptionComp = CompositeUtilities.createDefaultComposite(propertiesComp, 1, false);
		
		Label n = new Label(nameComp, SWT.NONE);
		n.setText("Name: ");
		name = CompositeUtilities.createTextField(nameComp, 300);
		
		
		bitSize1Label = new Label(variablesComp, SWT.NONE);
		bitSize1Label.setText("Bit Size of int:   ");
		
		bitSize2Label = new Label(variablesComp, SWT.NONE);
		bitSize2Label.setText("Bit Size of fp2:   ");
		bitSize2Label.setVisible(false);
		
		bitSize1 = CompositeUtilities.createTextField(variablesComp, 100);
		bitSize2 = CompositeUtilities.createTextField(variablesComp, 100);
		bitSize2.setEnabled(false);
		bitSize2.setVisible(false);
		
		Label d = new Label(variablesComp, SWT.NONE);
		d.setText("Latency: ");
		Label a = new  Label(variablesComp, SWT.NONE);
		a.setText("Active: ");
	
		
		delay = CompositeUtilities.createTextField(variablesComp, 100);
		active = new Button(variablesComp, SWT.CHECK);
		active.setLayoutData(CompositeUtilities.createNewGD(GridData.CENTER));
		
		Label t = new Label(descriptionComp, SWT.NONE);
		t.setText("Description: ");
		
		description = CompositeUtilities.createTextField(descriptionComp, 300);
		
	}

}
