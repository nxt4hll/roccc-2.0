package rocccplugin.wizardpages;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.PlatformUI;

public class CreateSystemWizardPage extends WizardPage 
{	
	public Text component_name;
	public Combo dimensions;
	public Combo project;

	public CreateSystemWizardPage(String pageName) 
	{
		super(pageName);
		setTitle("New ROCCC System");
		setDescription("Please enter the new system's information.\n");
	}
	@Override
	public boolean isPageComplete()
	{
		return (component_name.getText().length() > 0);
	}
	public void createControl(Composite parent) 
	{
		Composite top = createDefaultComposite(parent,1);
		setControl(top);
		this.setPageComplete(false);

		Group group1 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group1.setText("System Details");
		group1.setLayout(new GridLayout());
		group1.setLayoutData(createNewGD(0));
		Composite composite1 = createDefaultComposite(group1,2);
		createControl1(composite1);
	}

	private void createControl1(Composite parent)
	{
		ModifyListener updater = new ModifyListener() 
		{
			public void modifyText(final ModifyEvent event) 
			{
				getContainer().updateButtons();
			}
		};
		new Label(parent,SWT.NONE).setText("System Name:");
		component_name = new Text(parent,SWT.SINGLE | SWT.BORDER);
		component_name.setLayoutData(createNewGD(0));
		component_name.addModifyListener(updater);
		
		new Label(parent,SWT.NONE).setText("Project to add to:");
		project = new Combo (parent, SWT.READ_ONLY);
		project.setLayoutData(createNewGD(0));
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
		new Label(parent,SWT.NONE).setText("Stream Dimensions:");
		dimensions = new Combo(parent, SWT.READ_ONLY);
		dimensions.add("1"); dimensions.add("2"); dimensions.add("3"); dimensions.add("4"); dimensions.add("5");
		
		dimensions.select(0);
		dimensions.setLayoutData(createNewGD(0));
		dimensions.addModifyListener(updater);
	}
	private GridData createNewGD(int a)
	{
		GridData gd = new GridData(a);
		gd.verticalAlignment = SWT.FILL;
		gd.horizontalAlignment = SWT.FILL;
		gd.grabExcessHorizontalSpace = true;
		return gd;
	}
	private Composite createDefaultComposite(Composite parent, int numCols) {
		Composite composite = new Composite(parent, SWT.NULL);
		GridLayout layout = new GridLayout();
		layout.numColumns = numCols;
		composite.setLayout(layout);
		composite.setLayoutData(createNewGD(0));
		return composite;
	}
}