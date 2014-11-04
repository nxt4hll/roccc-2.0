package rocccplugin.wizardpages;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.PlatformUI;

import rocccplugin.Activator;
import rocccplugin.utilities.CompositeUtilities;

public class CreateNewIntrinsicWizardPage extends WizardPage 
{
	public Table intrinsicTypes;
	public Text name;
	public Text bitSize;
	public Text bitSize2;
	
	Label bitSizeLabel;
	Label bitSizeLabel2;
	
	public Combo project;
	
	public CreateNewIntrinsicWizardPage(String pageName) 
	{
		super(pageName);
		setTitle("Create New Intrinsic");
		setDescription("Pick an intrinsic type and set its properties.");
	}

	public void createControl(Composite parent)
	{
		Composite top = CompositeUtilities.createDefaultComposite(parent, 1, false);
		setControl(top);
		Group group1 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group1.setText("Select Intrinsic Properties");
		group1.setLayout(new GridLayout());
		group1.setLayoutData(CompositeUtilities.createNewGD(0));
		
		Composite insideGroup = CompositeUtilities.createDefaultComposite(group1, 3, false);	
		
		createIntrinsicTable(insideGroup);
		new Label(insideGroup, SWT.NONE).setText("   ");
		createProperties(insideGroup);
		
		intrinsicTypes.select(0);
	}
	
	private void createIntrinsicTable(Composite parent)
	{	
		Composite tableComp = CompositeUtilities.createDefaultComposite(parent, 1, false);
		
		intrinsicTypes = new Table(tableComp, SWT.FULL_SELECTION | SWT.BORDER);
		intrinsicTypes.setHeaderVisible(true);
		intrinsicTypes.setLinesVisible(true);
		intrinsicTypes.setLayoutData(CompositeUtilities.createNewGD(GridData.GRAB_VERTICAL));
		
		TableColumn column = new TableColumn(intrinsicTypes, SWT.NONE);
		column.setWidth(150);
		column.setText("Intrinsic Type");
		
		new TableItem(intrinsicTypes, SWT.NONE).setText("int_div");
		new TableItem(intrinsicTypes, SWT.NONE).setText("int_mod");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_mul");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_add");
		new TableItem(intrinsicTypes, SWT.NONE).setText("fp_sub");
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
		
		intrinsicTypes.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
			}
			
			public void widgetSelected(SelectionEvent e) 
			{
				String type = intrinsicTypes.getSelection()[0].getText();
				
				if(type.equals("int_to_fp") ||
					type.equals("fp_to_int") ||
					type.equals("fp_to_fp"))
				{
					bitSizeLabel2.setText(type.equals("fp_to_int")? "Bit Size of int:" : type.equals("fp_to_fp")? "Bit Size of fp2:" : "Bit Size of fp:");
					bitSizeLabel.setText(type.equals("int_to_fp")? "Bit Size of int:" : type.equals("fp_to_fp")? "Bit Size of fp1:" : "Bit Size of fp:");
					
					bitSizeLabel2.setVisible(true);
					bitSize2.setEnabled(true);
					bitSize2.setVisible(true);
				}
				else
				{
					bitSizeLabel.setText(type.startsWith("int")? "Bit Size for int:" : "Bit Size for fp:");
					
					bitSizeLabel2.setVisible(false);
					bitSize2.setEnabled(false);
					bitSize2.setVisible(false);
				}
			}
		});
	
		CompositeUtilities.setCompositeSize(tableComp, intrinsicTypes, intrinsicTypes.getItemHeight() * 7, intrinsicTypes.getItemHeight() * 10);
		intrinsicTypes.getHorizontalBar().setVisible(false);
	}
		
	private void createProperties(Composite parent)
	{
		Composite propertiesComp = CompositeUtilities.createDefaultComposite(parent, 1, false);
		Composite variablesComp = CompositeUtilities.createDefaultComposite(propertiesComp, 2, false);
		
		
		new Label(variablesComp,SWT.NONE).setText("Project to add to:");
		project = new Combo (variablesComp, SWT.READ_ONLY);
		project.setLayoutData(CompositeUtilities.createNewGD(0, true, false, SWT.LEFT));
		IProject[] project_names = ResourcesPlugin.getWorkspace().getRoot().getProjects();
		for(int i = 0; i < project_names.length; ++i)
		{
			project.add(project_names[i].getName());
		}
		IWorkbenchPart workbenchPart = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().findView("org.eclipse.jdt.ui.PackageExplorer");
		ISelection structuredSelection = null;
		if( workbenchPart != null )
			structuredSelection = workbenchPart.getSite().getSelectionProvider().getSelection();
		else
			project.select(0);
		
		if (structuredSelection instanceof org.eclipse.jface.viewers.TreeSelection) 
		{
			org.eclipse.jface.viewers.TreeSelection treeSelection = (org.eclipse.jface.viewers.TreeSelection) structuredSelection;
			IAdaptable firstElement = (IAdaptable) treeSelection.getFirstElement();
			if( firstElement instanceof IProject)
			{
				IProject proj = (IProject)firstElement;
				for(int i = 0; i < project_names.length; ++i)
				{
					if( project_names[i].getName() == proj.getName() )
					{
						project.select(i);
					}
				}
			}
		}
		
		Label n = new Label(variablesComp, SWT.NONE);
		n.setText("Name: ");
		
		name = CompositeUtilities.createTextField(variablesComp, 300);
		
		bitSizeLabel = new Label(variablesComp, SWT.NONE);
		bitSizeLabel.setText("Bit Size for int: ");
	
		bitSize = CompositeUtilities.createTextField(variablesComp, 75);
		
		bitSizeLabel2 = new Label(variablesComp, SWT.NONE);
		bitSizeLabel2.setText("Bit Size for int: ");
		
		bitSize2 = CompositeUtilities.createTextField(variablesComp, 75);
		
		bitSizeLabel2.setVisible(false);
		bitSize2.setEnabled(false);
		bitSize2.setVisible(false);
	}
		
}
