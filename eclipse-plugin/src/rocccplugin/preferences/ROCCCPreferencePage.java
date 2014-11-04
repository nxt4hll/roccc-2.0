package rocccplugin.preferences;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.preference.BooleanFieldEditor;
import org.eclipse.jface.preference.DirectoryFieldEditor;
import org.eclipse.jface.preference.FileFieldEditor;
import org.eclipse.jface.preference.FieldEditorPreferencePage;

import org.eclipse.jface.preference.StringFieldEditor;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.KeyListener;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IMemento;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPreferencePage;

import rocccplugin.Activator;
import rocccplugin.actions.ImportExamples;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.CompositeUtilities;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.PreferenceUtils;
import rocccplugin.views.DebugVariables;
import rocccplugin.views.ROCCC_IPCores;

import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.StringSelection;
import java.awt.Toolkit;

public class ROCCCPreferencePage extends FieldEditorPreferencePage
implements IWorkbenchPreferencePage 
{	
	ROCCCPreferencePage page;
	static String previousDistroDirectory;
	boolean obtainedLock;

	public ROCCCPreferencePage() 
	{
		super(FLAT);
		setDescription("");
		this.setTitle("ROCCC Settings");
		this.noDefaultAndApplyButton();
		this.setValid(true);
		setPreferenceStore(PreferenceUtils.getStore());

		//this.setImageDescriptor(ImageDescriptor.createFromImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/settings.png"))));

		page = this;
	}

	public void saveState(IMemento memento)
	{

	}	

	@Override
	public void dispose()
	{
		if(!obtainedLock)
			return;

		super.dispose();


		Display.getDefault().asyncExec(new Runnable()
		{
			public void run() 
			{
				Activator.compilerAndPluginNeedChecking();

				if(Activator.testDistributionFolder(PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION)))
					Activator.checkCompilerAndPluginStatus();

				boolean IPCoresViewOpen = EclipseResourceUtils.isViewOpen(ROCCC_IPCores.ID);
				EclipseResourceUtils.closeView(ROCCC_IPCores.ID);
				boolean debugViewOpen = EclipseResourceUtils.isViewOpen(DebugVariables.ID);
				EclipseResourceUtils.closeView(DebugVariables.ID);

				GuiLockingUtils.unlockGui();

				if(IPCoresViewOpen)
					EclipseResourceUtils.openView(ROCCC_IPCores.ID);
				if(debugViewOpen)
					EclipseResourceUtils.openView(DebugVariables.ID);
			}
		});



		Display.getDefault().asyncExec(new Runnable()
		{
			public void run()
			{
				if(PreferenceUtils.getPreferenceBoolean(PreferenceConstants.NEVER_HAD_VALID_DISTRO) && 
						Activator.testDistributionFolder(PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION)))
				{
					PreferenceUtils.setValue(PreferenceConstants.NEVER_HAD_VALID_DISTRO, false);

					if(!GuiLockingUtils.canRunCommand())
						return;

					if(MessageDialog.openQuestion(new Shell(), "Installation Success!", "Would you like to have ROCCC load up the distributed examples?"))
					{
						ImportExamples.run();
					}
				}

				DatabaseInterface.closeConnection();
				GuiLockingUtils.unlockGui();
			}
		});
	}

	public void init(IWorkbench workbench) 
	{
		previousDistroDirectory = PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION);
	}

	@Override
	protected void createFieldEditors() 
	{
		try
		{
			obtainedLock = false;
			final Composite fieldEditorComp = getFieldEditorParent();

			Composite top = CompositeUtilities.createDefaultComposite(fieldEditorComp.getParent(), 1, false);
			fieldEditorComp.setLayout(new RowLayout());
			fieldEditorComp.setLayoutData(CompositeUtilities.createNewGD(0));

			//Composite imageComp = Activator.createDefaultComposite(fieldEditorComp, 1, false, SWT.LEFT);
			//imageComp.setLayout(new RowLayout());
			//imageComp.setLayoutData(Activator.createNewGD(0));

			Label image = new Label(fieldEditorComp, SWT.NONE);
			image.setImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/settingsPlain.png")));

			if(!GuiLockingUtils.lockGui())
			{
				new Label(top, SWT.NONE).setText(GuiLockingUtils.getLockMessage() + " The ROCCC preferences are not accessable until then.");
				return;
			}
			//Activator.unlockGui();
			obtainedLock = true;
			
			page.setValid(true);

			//new Label(top, SWT.NONE).setImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/settings.png")));

			Group distroGroup = new Group(top, SWT.SHADOW_ETCHED_IN | SWT.TOP);
			distroGroup.setText("Distribution Directory");
			distroGroup.setLayout(new GridLayout());
			distroGroup.setLayoutData(CompositeUtilities.createNewGD(0));

			Composite comp = CompositeUtilities.createDefaultComposite(distroGroup, 3, false);
			comp.setLayout(new GridLayout());
			comp.setLayoutData(CompositeUtilities.createNewGD(GridData.BEGINNING));

			final DirectoryFieldEditor directory = new DirectoryFieldEditor(PreferenceConstants.ROCCC_DISTRIBUTION, "&ROCCC Distribution Directory:", comp);
			addField(directory);


			/*
			//Tim - I removed this since now there are more Composites that can fail
			comp.addKeyListener(new KeyListener()
			{
				public void keyPressed(KeyEvent e) 
				{
					page.setValid(true);
				}

				public void keyReleased(KeyEvent e)
				{
				}
			});
			*/

			new Label(comp, SWT.NONE).setText("");
			new Label(comp, SWT.NONE).setText("");
			new Label(comp, SWT.NONE).setText("");
			new Label(comp, SWT.NONE).setText("");

			Button button = new Button(comp, SWT.PUSH);
			button.setText("Verify ROCCC Distribution Folder");	

			button.addSelectionListener(new SelectionListener()
			{
				public void widgetDefaultSelected(SelectionEvent e) 
				{
					widgetSelected(e);
				}

				public void widgetSelected(SelectionEvent e) 
				{
					if(Activator.testDistributionFolder(directory.getStringValue()))
					{
						MessageDialog.openInformation(new Shell(), "Distribution Success", "The ROCCC distribution folder is valid. ROCCC database found.");
					}
					else
					{
						MessageDialog.openError(new Shell(), "Distribution Error", "The ROCCC distribution folder selected is incorrect. No ROCCC database found.");
					}
				}
			});




			//Group compilerGroup = new Group(flagsSection, SWT.SHADOW_ETCHED_IN | SWT.TOP);
			//compilerGroup.setText("Compiler Preferences");
			//compilerGroup.setLayout(new GridLayout());
			//compilerGroup.setLayoutData(CompositeUtilities.createNewGD(0));

			//addField(new BooleanFieldEditor(PreferenceConstants.MAXIMIZE_PRECISION, "Maximize Precision", compilerGroup));
			//addField(new BooleanFieldEditor(PreferenceConstants.SEPARATE_TEMPORARY_ARRAYS, "Separate Temporary Arrays", compilerGroup));

			//new Label(flagsSection, SWT.NONE).setText("");
			
			
//			Group licenseGroup = new Group(top, SWT.SHADOW_ETCHED_IN | SWT.TOP);
//			licenseGroup.setText("License File Path");
//			licenseGroup.setLayout(new GridLayout());
//			licenseGroup.setLayoutData(CompositeUtilities.createNewGD(0));
//
//			Composite licenseComp = CompositeUtilities.createDefaultComposite(licenseGroup, 3, false);
//			licenseComp.setLayout(new GridLayout());
//			licenseComp.setLayoutData(CompositeUtilities.createNewGD(GridData.BEGINNING));
//
//			final LicenseFileFieldEditor licFile = new LicenseFileFieldEditor(PreferenceConstants.LICENSE_FILE_PATH, "&ROCCC License File Path:", licenseComp);
//			addField(licFile);
//						
//			new Label(licenseComp, SWT.NONE).setText("");
//			new Label(licenseComp, SWT.NONE).setText("");
//			new Label(licenseComp, SWT.NONE).setText("");
//			new Label(licenseComp, SWT.NONE).setText("");
//
//			Button licButton = new Button(licenseComp, SWT.PUSH);
//			licButton.setText("Verify License File Path");
//			
//
//			licButton.addSelectionListener(new SelectionListener()
//			{
//				public void widgetDefaultSelected(SelectionEvent e) 
//				{
//					widgetSelected(e);
//				}
//
//				public void widgetSelected(SelectionEvent e)
//				{
//
//				}
//			});
			
			// timk: we don't need users to be able to look up their mac
			/*
			Group macGroup = new Group(top, SWT.SHADOW_ETCHED_IN | SWT.TOP);
			macGroup.setText("MAC Address");
			macGroup.setLayout(new GridLayout());
			macGroup.setLayoutData(CompositeUtilities.createNewGD(0));

			Composite macComp = CompositeUtilities.createDefaultComposite(macGroup, 3, false);
			macComp.setLayout(new GridLayout());
			macComp.setLayoutData(CompositeUtilities.createNewGD(GridData.BEGINNING));
			
			final StringFieldEditor mac = new StringFieldEditor(PreferenceConstants.MAC_ADDRESS, "&Your Current MAC Address:", macComp);
			mac.setEnabled(false, macComp);
			addField(mac);
			
			new Label(licenseComp, SWT.NONE).setText("");
			new Label(licenseComp, SWT.NONE).setText("");
			new Label(licenseComp, SWT.NONE).setText("");
			
			Button macButton = new Button(macComp, SWT.PUSH);
			macButton.setText("Copy MAC to Clipboard");
			
			macButton.addSelectionListener(new SelectionListener()
			{
				public void widgetDefaultSelected(SelectionEvent e) 
				{
					widgetSelected(e);
				}

				public void widgetSelected(SelectionEvent e) 
				{
					Toolkit toolkit = Toolkit.getDefaultToolkit();
				    Clipboard clipboard = toolkit.getSystemClipboard();
				    StringSelection stringSelection = new StringSelection(PreferenceUtils.getPreferenceString(PreferenceConstants.MAC_ADDRESS));
				    clipboard.setContents(stringSelection, null);
				}
			});
			*/
			
			Composite flagsSection = CompositeUtilities.createDefaultComposite(top, 1, false);

			new Label(flagsSection, SWT.NONE).setText("");

			Group generalGroup = new Group(flagsSection, SWT.SHADOW_ETCHED_IN | SWT.TOP);
			generalGroup.setText("General Preferences");
			generalGroup.setLayout(new GridLayout());
			generalGroup.setLayoutData(CompositeUtilities.createNewGD(0));


			BooleanFieldEditor openReportCheckBox = new BooleanFieldEditor(PreferenceConstants.OPEN_COMPILATION_REPORT_AFTER_COMPILE, "Automatically open compilation report after compiling.", generalGroup);
			addField(openReportCheckBox);
			//Composite imageComp = CompositeUtilities.createDefaultComposite(flagsSection, 1, false, SWT.CENTER);
			//Label image = new Label(imageComp, SWT.NONE);
			//image.setImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/settings.png")));
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
}