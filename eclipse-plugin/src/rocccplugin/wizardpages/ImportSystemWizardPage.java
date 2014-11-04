package rocccplugin.wizardpages;

import java.io.File;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.PlatformUI;

public class ImportSystemWizardPage extends WizardPage 
{
	public Text sourceFileField;
	public Text systemName;
	public Combo project;
	
	public ImportSystemWizardPage(String pageName) 
	{
		super(pageName);
		setTitle("Import ROCCC System");
		setDescription("Choose a ROCCC System file and a project to import it into.\n");
	}

	@Override
	public boolean isPageComplete()
	{
		if(project.getSelectionIndex() == -1 || sourceFileField.getText().length() == 0 ||
		   systemName.getText().length() == 0)
				return false; 
		return true;
				 
	}
	
	private IPath browse(IPath path, boolean mustExist)
	{
		FileDialog dialog = new FileDialog(getShell(), mustExist? SWT.OPEN : SWT.SAVE);
		File f = new File(path != null? path.toOSString() : "");
		if(f.exists())
			dialog.setFilterPath(f.getAbsolutePath().replace(f.getName(), ""));
	//	else if(Activator.getSelectedFileInEditor() != null)
		//	dialog.setFilterPath(Activator.getSelectedFileLocationInEditor().replace(Activator.getSelectedFileInEditor().getName(), ""));
		else
			dialog.setFilterPath("");
		String result = dialog.open();
		if(result == null)
			return null;
		
		return new Path(result);
	}
	
	private IPath getSourceLocation()
	{
		String text = sourceFileField.getText().trim();
		if(text.length() == 0)
			return null;
		IPath path = new Path(text);
		if(!path.isAbsolute())
			path = ResourcesPlugin.getWorkspace().getRoot().getLocation().append(path);
		return path;
	}
	
	private void browseForSourceFile()
	{
		IPath path = browse(getSourceLocation(), true);
		if(path == null)
			return;
		IPath rootLoc = ResourcesPlugin.getWorkspace().getRoot().getLocation();
		//if(rootLoc.isPrefixOf(path))
			//path = path.setDevice(null).removeFirstSegments(rootLoc.segmentCount());
		sourceFileField.setText(path.toString());
	}
	
	public void createControl(Composite parent) 
	{
		Composite top = createDefaultComposite(parent,1);
		setControl(top);
		this.setPageComplete(false);
		
		Group group1 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group1.setText("Import ROCCC System");
		group1.setLayout(new GridLayout());
		group1.setLayoutData(createNewGD(0));
		
		Composite sourceSection = createDefaultComposite(group1,3);
		
		final Label label_1 = new Label(sourceSection, SWT.NONE);
		final GridData gridData_1 = new GridData(GridData.HORIZONTAL_ALIGN_END);
		label_1.setLayoutData(gridData_1);
		label_1.setText("Import File:");
		
		sourceFileField = new Text(sourceSection, SWT.BORDER);
		sourceFileField.addModifyListener(new ModifyListener()
		{
			public void modifyText(ModifyEvent e)
			{
				setPageComplete(isPageComplete());
			}
		});
		sourceFileField.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
		
		final Button button = new Button(sourceSection, SWT.NONE);
		button.addSelectionListener(new SelectionAdapter()
		{
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				try
				{
					String oldFile = sourceFileField.getText();
					browseForSourceFile();
					if(sourceFileField.getText().equals(oldFile) == false)
					{
						File f = new File(sourceFileField.getText());
						try
						{
							if(systemName != null)
								systemName.setText(f.getName().split("\\.")[0]);
						}
						catch(Exception ex){}
					}
				}
				catch(Exception ex2)
				{
					
				}
			}
		});
		button.setText("Browse...");
		
		Label name = new Label(sourceSection,SWT.NONE);
		name.setText("System Name:");
		name.setLayoutData(gridData_1);
		systemName = new Text(sourceSection, SWT.BORDER);
		systemName.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
		systemName.addModifyListener(new ModifyListener()
		{
			public void modifyText(ModifyEvent e)
			{   
				setPageComplete(isPageComplete());
			}
		});
		new Label(sourceSection, SWT.NONE).setText("");
		
		new Label(sourceSection,SWT.NONE).setText("To Project:");
		project = new Combo (sourceSection, SWT.READ_ONLY);
		project.setLayoutData(createNewGD(0));
		project.addModifyListener(new ModifyListener()
		{
			public void modifyText(ModifyEvent e)
			{
				setPageComplete(isPageComplete());
			}
		});
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
		
		
	}
	
	private GridData createNewGD(int a)
	{
		GridData gd = new GridData(a);
		gd.verticalAlignment = SWT.FILL;
		gd.horizontalAlignment = SWT.FILL;
		gd.grabExcessHorizontalSpace = true;
		return gd;
	}
	
	private Composite createDefaultComposite(Composite parent, int numCols) 
	{
		Composite composite = new Composite(parent, SWT.NULL);
		GridLayout layout = new GridLayout();
		layout.numColumns = numCols;
		composite.setLayout(layout);
		composite.setLayoutData(createNewGD(0));
		return composite;
	}

}
