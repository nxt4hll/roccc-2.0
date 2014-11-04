package rocccplugin.views;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.text.FindReplaceDocumentAdapter;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.ITextSelection;
import org.eclipse.jface.text.TextSelection;
import org.eclipse.jface.util.IPropertyChangeListener;
import org.eclipse.jface.util.PropertyChangeEvent;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.KeyListener;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.events.MouseMoveListener;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.dialogs.PreferencesUtil;
import org.eclipse.ui.part.ViewPart;
import org.eclipse.ui.texteditor.AbstractTextEditor;
import org.eclipse.ui.texteditor.IDocumentProvider;
import org.eclipse.ui.texteditor.ITextEditor;
import org.osgi.framework.Version;

import rocccplugin.Activator;
import rocccplugin.ROCCCPlugin;
import rocccplugin.Activator.VersionStatus;
import rocccplugin.actions.RemoveComponentPass;
import rocccplugin.actions.ResetDatabase;
import rocccplugin.composites.ComponentTable;
import rocccplugin.composites.PortTable;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.CompositeUtilities;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.PreferenceUtils;

public class ROCCC_IPCores extends ViewPart implements IPropertyChangeListener
{
	ComponentTable compTable = null;
	PortTable port = null;
	public static boolean showingLockMessage = false;
	static public Composite p;
	
	public static final String ID = "rocccplugin.views.ROCCC_IPCores";
	
	public ROCCC_IPCores() 
	{   
		super();
	}
		
	
	public void createPartControl(Composite parent) 
	{
		try
		{
			if(parent == null)
				return;
			
			if(!GuiLockingUtils.lockGui())
			{
				compTable = new ComponentTable(parent, true);
				new Label(compTable, SWT.NONE).setText("");
				new Label(parent, SWT.NONE).setText(GuiLockingUtils.getLockMessage());
				showingLockMessage = true;
				return;
			}
			
			showingLockMessage = false;
			
			if(!DatabaseInterface.openConnectionWithoutWarnings())
			{
				
					Composite comp = CompositeUtilities.createDefaultComposite(parent, 1, false, SWT.CENTER);
					
					GridData gd1 = new GridData(SWT.CENTER);
					gd1.verticalAlignment = SWT.NONE;
					gd1.horizontalAlignment = SWT.CENTER;
					gd1.grabExcessHorizontalSpace = true;
					
					comp.setLayoutData(gd1);
					
					if(Activator.getDistributionFolder().equals(""))
					{
						new Label(comp, SWT.NONE).setText(""); 
						new Label(comp, SWT.NONE).setText("The distribution folder preference has not been set. This must be set in the ROCCC Preferences to begin.");
						new Label(comp, SWT.NONE).setText("");
					}
					else
					{
						new Label(comp, SWT.NONE).setText("");
						new Label(comp, SWT.NONE).setText("The distribution folder selected is not a valid distribution folder.");
						new Label(comp, SWT.NONE).setText("");
					}
					
					Button b = new Button(comp, SWT.PUSH);
					
					b.setText("Open ROCCC Preferences");
					b.setImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/icons/preferences.png")));
					
					GridData gd = new GridData(SWT.CENTER);
					gd.verticalAlignment = SWT.NONE;
					gd.horizontalAlignment = SWT.CENTER;
					gd.grabExcessHorizontalSpace = false;
					
					b.setLayoutData(gd);
					
					b.addSelectionListener(new SelectionListener()
					{
						public void widgetDefaultSelected(SelectionEvent e) 
						{
								
						}

						public void widgetSelected(SelectionEvent e) 
						{
							Display.getCurrent().asyncExec(new Runnable()
							{
								public void run()
								{
									try
									{
										PreferencesUtil.createPreferenceDialogOn(new Shell(), "rocccplugin.preferences.ROCCCPreferencePage", null, null).open();
									}
									catch(Exception e)
									{
										e.printStackTrace();
									}	
								}
							});
						}
						
					});
					
				GuiLockingUtils.unlockGui();
				return;
			}
			
			if(Activator.checkCompilerAndPluginStatus() == VersionStatus.COMPILER_UPDATE_NEEDED ||
			   Activator.checkCompilerAndPluginStatus() == VersionStatus.PLUGIN_UPDATE_NEEDED)
			{
				Composite comp = CompositeUtilities.createDefaultComposite(parent, 1, false, SWT.CENTER);
				
				GridData gd1 = new GridData(SWT.CENTER);
				gd1.verticalAlignment = SWT.NONE;
				gd1.horizontalAlignment = SWT.CENTER;
				gd1.grabExcessHorizontalSpace = true;
				
				comp.setLayoutData(gd1);
				
				new Label(comp, SWT.NONE).setText("");
				new Label(comp, SWT.NONE).setText("The ROCCC Compiler and ROCCC GUI plugin are out of sync. An update must take place before using this view.");
				
				Button b = new Button(comp, SWT.PUSH);
				
				b.setText("Open ROCCC Preferences");
				b.setImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/icons/preferences.png")));
				
				GridData gd = new GridData(SWT.CENTER);
				gd.verticalAlignment = SWT.NONE;
				gd.horizontalAlignment = SWT.CENTER;
				gd.grabExcessHorizontalSpace = false;
				
				b.setLayoutData(gd);
				
				b.addSelectionListener(new SelectionListener()
				{
					public void widgetDefaultSelected(SelectionEvent e) 
					{
							
					}

					public void widgetSelected(SelectionEvent e) 
					{
						Display.getCurrent().asyncExec(new Runnable()
						{
							public void run()
							{
								try
								{
									PreferencesUtil.createPreferenceDialogOn(new Shell(), "rocccplugin.preferences.ROCCCPreferencePage", null, null).open();
								}
								catch(Exception e)
								{
									e.printStackTrace();
								}	
							}
						});
					}
					
				});
				
				DatabaseInterface.closeConnection();
				GuiLockingUtils.unlockGui();
				return;
			}
			
			//Check to see if the database version is current.
			String dbVersion = DatabaseInterface.getDatabaseVersion();
			if(dbVersion.equals("NA") || ROCCCPlugin.getVersionNumber().equals(new Version(dbVersion)) == false)
			{
				Composite comp = CompositeUtilities.createDefaultComposite(parent, 1, false, SWT.CENTER);
				
				GridData gd1 = new GridData(SWT.CENTER);
				gd1.verticalAlignment = SWT.NONE;
				gd1.horizontalAlignment = SWT.CENTER;
				gd1.grabExcessHorizontalSpace = true;
				
				comp.setLayoutData(gd1);
				
				new Label(comp, SWT.NONE).setText("");
				new Label(comp, SWT.NONE).setText("The ROCCC database is out of date with the compiler. The database must be reset.");
				
				Button b = new Button(comp, SWT.PUSH);
				
				b.setText("Reset ROCCC Database");
				b.setImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/icons/resetcompiler.png")));
				
				GridData gd = new GridData(SWT.CENTER);
				gd.verticalAlignment = SWT.NONE;
				gd.horizontalAlignment = SWT.CENTER;
				gd.grabExcessHorizontalSpace = false;
				
				b.setLayoutData(gd);
				
				b.addSelectionListener(new SelectionListener()
				{
					public void widgetDefaultSelected(SelectionEvent e) 
					{
							
					}

					public void widgetSelected(SelectionEvent e) 
					{
						Display.getCurrent().asyncExec(new Runnable()
						{
							public void run()
							{
								try
								{
									ResetDatabase.run();
									
									//Reset the IPCores view to show the updated database.	
									if(DatabaseInterface.openConnection())
									{	
										DatabaseInterface.updateAllListeners();
										
										boolean IPCoresViewOpen = EclipseResourceUtils.isViewOpen(ROCCC_IPCores.ID);
										EclipseResourceUtils.closeView(ROCCC_IPCores.ID);
										
										GuiLockingUtils.unlockGui();
										
										if(IPCoresViewOpen)
											EclipseResourceUtils.openView(ROCCC_IPCores.ID);
										
										DatabaseInterface.updateAllListeners();
										DatabaseInterface.closeConnection();
									
										DatabaseInterface.closeConnection();
									}
								}
								catch(Exception e)
								{
									e.printStackTrace();
								}	
							}
						});
					}
					
				});
				
				DatabaseInterface.closeConnection();
				GuiLockingUtils.unlockGui();
				return;
				
			}
			
			p = parent;
			
			compTable = new ComponentTable(parent, true);
			port = new PortTable(parent, false);
			DatabaseInterface.addPropertyChangeListener(port);
			DatabaseInterface.addPropertyChangeListener(this);
			compTable.components.addSelectionListener(new SelectionListener()
			{
				public void widgetSelected(SelectionEvent event) 
				{	
					if(!GuiLockingUtils.canRunCommand())
						return;
					if(compTable.components.getSelectionIndex() == -1)
					{
						DatabaseInterface.closeConnection();
						GuiLockingUtils.unlockGui();
						return;
					}
					TableItem t = compTable.components.getSelection()[0];
					if(port.component == null || !t.getText(0).equals(port.component))
						port.setComponent( t.getText(0) );
					DatabaseInterface.closeConnection();
					GuiLockingUtils.unlockGui();
				}
				public void widgetDefaultSelected(SelectionEvent event) 
				{
					widgetSelected(event);
				}
			});
			compTable.components.addMouseMoveListener(new MouseMoveListener() 
			{
			      public void mouseMove(MouseEvent arg0) 
			      {
			    	  int item = arg0.y / compTable.components.getItemHeight();
			    	  
			    	  if(item < compTable.components.getItemCount())
			    	  {
			    		  TableItem t = compTable.components.getItem(item);
			    		  
			    		  if(GuiLockingUtils.isGuiLocked())
			    			  return;
			    		  DatabaseInterface.openConnection();
				    	  compTable.components.setToolTipText("Component: " + t.getText(0) + "\nAuthor: " + PreferenceUtils.getPreferenceString(PreferenceConstants.USER_NAME) + "\nDate Compiled: " + DatabaseInterface.getDateCompiled(t.getText(0)));
				    	  DatabaseInterface.closeConnection();
			    	  }
			    	  else
			    	  {
			    		  compTable.components.setToolTipText(null);
			    	  }
			      }
			});
			compTable.components.addMouseListener(new MouseListener()
			{
				
				public void mouseDoubleClick(MouseEvent e)
				{
					if(compTable.components.getItemCount() == 0)
						return;
					if(compTable.components.getSelection().length == 0)
						return;
					if(!GuiLockingUtils.canRunCommand())
						return;
					if(e.button != 1)
					{
						DatabaseInterface.closeConnection();
						GuiLockingUtils.unlockGui();
						return;
					}
						
					String versionCompiledOn = DatabaseInterface.versionCompiledOn(compTable.components.getItem(compTable.components.getSelectionIndex()).getText(0));
					
					if(versionCompiledOn != null && versionCompiledOn.equals("NA"))
					{
					}
					else if(versionCompiledOn == null || versionCompiledOn.equals("") || new Version(versionCompiledOn).compareTo(Activator.getMinimumCompilerVersionNeeded()) < 0)
					{
						MessageDialog.openError(new Shell(), "Version Compiled On Error", "This component appears to have been compiled in a previous version of ROCCC.  Please recompile this component prior to using it.");
						GuiLockingUtils.unlockGui();
						DatabaseInterface.closeConnection();
						return;
					}
					
					IWorkbenchPage page = Activator.getDefault().getWorkbench().getActiveWorkbenchWindow().getActivePage();
					IEditorPart part = page.getActiveEditor();
					if (!(part instanceof AbstractTextEditor))
					{
						DatabaseInterface.closeConnection();
						GuiLockingUtils.unlockGui();
						return;
					}
					ITextEditor editor = (ITextEditor)part;
					IDocumentProvider dp = editor.getDocumentProvider();
					IDocument doc = dp.getDocument(editor.getEditorInput());
					try
					{
						DatabaseInterface.openConnection();
						String component_name = compTable.components.getSelection()[0].getText(0);
						String insert = component_name + "(";
						
						insert += DatabaseInterface.getPortOrder(component_name) + ");";
						
						ITextSelection textSelection = (ITextSelection)editor.getSelectionProvider().getSelection();
						doc.replace(textSelection.getOffset(), textSelection.getLength(), insert);
						TextSelection newSelection = new TextSelection(doc, textSelection.getOffset(), insert.length());
						editor.getSelectionProvider().setSelection(newSelection);
						String include = "#include \"roccc-library.h\"";
						FindReplaceDocumentAdapter finder = new FindReplaceDocumentAdapter(doc);
						if(finder.find(0, include, true, true, false, false) == null )
						{
							doc.replace(0, 0, include + "\n");
						}
					}
					catch(Exception e2)
					{
						e2.printStackTrace();
					}
					DatabaseInterface.closeConnection();
					GuiLockingUtils.unlockGui();
				}
				public void mouseDown(MouseEvent e){}
				public void mouseUp(MouseEvent e){}
			});
			compTable.components.addKeyListener(new KeyListener()
			{
				public void keyPressed(KeyEvent e)
				{
					try
					{
						if( e.character == SWT.BS || e.character == SWT.DEL )
						{	
							if(!GuiLockingUtils.canRunCommand())
								return;
							
							GuiLockingUtils.setLockMessage("Removing IPCore from the database. Please wait.");
												
							if(compTable.components.getSelectionIndex() != -1)
							{
								
								if(!MessageDialog.openQuestion(new Shell(), "IPCore Delete", "Do you really want to delete \"" + compTable.components.getItem(compTable.components.getSelectionIndex()).getText(0) + "\" from the database?"))
								{
									DatabaseInterface.closeConnection();
									GuiLockingUtils.unlockGui();
									return;
								}
							}
							else
							{
								DatabaseInterface.closeConnection();
								GuiLockingUtils.unlockGui();
								return;
							}
								
							//Call remove module script.	
							String compName = compTable.components.getItem(compTable.components.getSelectionIndex()).getText().toString();
							
							RemoveComponentPass.run(compName);
							
							GuiLockingUtils.unlockGui();
							
							Display.getDefault().asyncExec(new Runnable()
							{
								public void run() 
								{
									try
									{
										if(DatabaseInterface.openConnectionWithoutWarnings())
											DatabaseInterface.updateAllListeners();
										
									}
									catch(Exception e)
									{
										e.printStackTrace();
									}
								}
							});
							
							DatabaseInterface.closeConnection();
						}
					}
					catch(Exception er)
					{
						er.printStackTrace();
						DatabaseInterface.closeConnection();
						GuiLockingUtils.unlockGui();
					}
					
				}
				public void keyReleased(KeyEvent e)
				{
					
				}
			});
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		if(DatabaseInterface.openConnectionWithoutWarnings())
			DatabaseInterface.updateAllListeners();
		
		DatabaseInterface.closeConnection(); 
		GuiLockingUtils.unlockGui();
	}

	@Override
	public void setFocus() 
	{
		//comp.propertyChange(null);
		//port.propertyChange(null);
	}
	
	@Override
	public void dispose()
	{
		try
		{
			DatabaseInterface.removePropertyChangeListener(port);
			if(port != null)
			{
				port.dispose();
				port = null;
			}
			if(compTable != null)
			{
				compTable.dispose();
				port = null;
			}	
				
			super.dispose();
		
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}


	public void propertyChange(PropertyChangeEvent event) 
	{
		//createPartControl(p);
	}

}
