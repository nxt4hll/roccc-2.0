package rocccplugin.wizardpages;

import java.io.File;
import java.util.Vector;

import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.TableEditor;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.KeyListener;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;

import rocccplugin.Activator;
import rocccplugin.utilities.CompositeUtilities;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.FileUtils;
import rocccplugin.utilities.StringUtils;
import rocccplugin.wizards.AddStreamInfoWizard;

public class SystemInfoWizardPage extends WizardPage 
{
	Table streamInfoTable;	
	Table outputStreamInfoTable;
	public Vector<String[]> streamInfo;
	public Vector<Button> checkButtons;
	public Vector<String[]> outputStreamInfo;
	TableEditor editor;
	TableEditor outputEditor;
	int selectionIndex = 0;
	int selectedRow = 0;
	File fileToCompile;

	public SystemInfoWizardPage(String pageName, String systemName, File fileToCompile) 
	{
		super(pageName);
		setTitle("Stream Accessing Management");
		setDescription("Tune any of the input or output stream accessing for " + systemName);
		streamInfo = new Vector<String[]>();
		checkButtons = new Vector<Button>();
		outputStreamInfo = new Vector<String[]>();
		this.fileToCompile = fileToCompile;
	}

	public void createControl(Composite parent) 
	{
		Composite top = CompositeUtilities.createDefaultComposite(parent,1, false);
		setControl(top);
		
		Group group1 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group1.setText("Input Stream Information");
		group1.setLayout(new GridLayout());
		group1.setLayoutData(CompositeUtilities.createNewGD(0));
		
		createStreamInfoTable(group1);
		
		Group group2 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group2.setText("Output Stream Information");
		group2.setLayout(new GridLayout());
		group2.setLayoutData(CompositeUtilities.createNewGD(0));
		
		createOutputInfoTable(group2);
		
		loadPreviousSystemInfoFile(fileToCompile);
		
		Label l = new Label(top, SWT.NONE);
		l.setImage(new Image(null, this.getClass().getResourceAsStream("/rocccplugin/images/optimizationDescriptions/Channels.png")));
	}
	
	private void createThroughputCheckBox(boolean checked)
	{
		try
		{
			/*TableEditor ed = new TableEditor(streamInfoTable);
			final Button button = new Button(streamInfoTable, SWT.CHECK);
			button.setSelection(checked);
			button.pack();
			ed.minimumWidth = button.getSize().x;
			ed.horizontalAlignment = SWT.LEFT;
			ed.setEditor(button, streamInfoTable.getItems()[streamInfoTable.getItemCount() - 1], 3);
			checkButtons.add(button); 
			button.addSelectionListener(new SelectionListener()
			{
				public void widgetDefaultSelected(SelectionEvent e) 
				{
					
				}
	
				public void widgetSelected(SelectionEvent e) 
				{
					try
					{			
						int row = -1;
						//Find out which row the button is in.
						for(int i = 0, total = 0; i < streamInfoTable.getItemCount(); ++i)
						{
							total += streamInfoTable.getItemHeight();

							if(button.getLocation().y < total)
							{
								row = i;
								break;
							}
						}
						
						if(row == -1)
							return;
						
						if(streamInfo.get(row)[3].equals("1"))
							streamInfo.get(row)[3] = "0";
						else
							streamInfo.get(row)[3] = "1";
					}
					catch(Exception e1)
					{
						e1.printStackTrace();
					}
				}					
			});*/
		}
		catch(Exception e)
		{ 
			System.out.println("We're Screwed");
			e.printStackTrace();
		}
	}
	
	private void loadPreviousSystemInfoFile(File fileToCompile)
	{
		StringBuffer buffer = new StringBuffer();
		//IFile file = Activator.getSelectedFileInEditor();
		//String folderString = file.getProjectRelativePath().toString().replace(file.getName(), "");
		//IFile systemInfoFile = ResourcesPlugin.getWorkspace().getRoot().getProject(file.getProject().getName()).getFile(folderString + "/.ROCCC/.streamInfo");
		File streamInfoFile = new File(FileUtils.getFolderOfFile(fileToCompile) + ".ROCCC/.streamInfo");
		
		if(streamInfoFile == null || !streamInfoFile.exists() || !streamInfoFile.canRead())
			return;
		
		FileUtils.addFileContentsToBuffer(buffer, streamInfoFile.getAbsolutePath());
		
		String temp;
		
		boolean fileError = false;
		
		while(buffer.length() > 0)
		{
			temp = StringUtils.getNextStringValue(buffer);
			if(temp.equals("INPUT"))
			{
				String name = StringUtils.getNextStringValue(buffer);
				String channels = StringUtils.getNextStringValue(buffer);
				String requests = StringUtils.getNextStringValue(buffer);
				
				if(StringUtils.isValidVariableName(name) == false)
					fileError = true;
				else if(StringUtils.isPositiveInt(channels) == false)
					fileError = true;
				else if(StringUtils.isPositiveInt(requests) == false)
					fileError = true;
				
				streamInfo.add(new String[]{name, channels, requests});
				
			}
			else if(temp.equals("OUTPUT"))
			{	
				String name = StringUtils.getNextStringValue(buffer);
				String channels = StringUtils.getNextStringValue(buffer);
				String requests = StringUtils.getNextStringValue(buffer);
				
				if(StringUtils.isValidVariableName(name) == false)
					fileError = true;
				else if(StringUtils.isPositiveInt(channels) == false)
					fileError = true;
				else if(StringUtils.isPositiveInt(requests) == false)
					fileError = true;
				
				outputStreamInfo.add(new String[]{name, channels, requests});
			}
			else
			{
				
				fileError = true;
			}
			
			if(fileError)
			{
				
				streamInfo.clear();
				outputStreamInfo.clear();
				return;
			}
		}
		
		for(int i = 0; i < streamInfo.size(); ++i)
		{
			new TableItem(streamInfoTable, SWT.NONE).setText(new String[]{ streamInfo.get(i)[0], streamInfo.get(i)[1], streamInfo.get(i)[2]});
		}
		for(int i = 0; i < outputStreamInfo.size(); ++i)
		{
			new TableItem(outputStreamInfoTable, SWT.NONE).setText(new String[]{ outputStreamInfo.get(i)[0], outputStreamInfo.get(i)[1], outputStreamInfo.get(i)[2]});
		}
	}
	
	private void createStreamInfoTable(Composite parent)
	{
		Composite seperator = CompositeUtilities.createDefaultComposite(parent, 2, false);
		Composite tableComp = CompositeUtilities.createDefaultComposite(seperator, 1, false);
		Composite buttonsComp = CompositeUtilities.createDefaultComposite(seperator, 1, false);
		
		streamInfoTable = new Table(tableComp, SWT.FULL_SELECTION | SWT.BORDER);
		streamInfoTable.setHeaderVisible(true);
		streamInfoTable.setLinesVisible(true);
		streamInfoTable.setLayoutData(CompositeUtilities.createNewGD(GridData.GRAB_VERTICAL));
		
		streamInfoTable.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				//widgetDefaultSelected(e);
			}

			public void widgetSelected(SelectionEvent e)
			{
				handleEditor();
				selectionIndex = streamInfoTable.getSelectionIndex();
			}
			
		});
		
		
		
		TableColumn streamName = new TableColumn(streamInfoTable, SWT.NONE);
		streamName.setText("Input Stream Name");
		streamName.setWidth(150);
		
		TableColumn amountColumn = new TableColumn(streamInfoTable, SWT.NONE);
		amountColumn.setText("Number of Parallel Data Channels");
		amountColumn.setWidth(225);
		
		TableColumn requestsColumn = new TableColumn(streamInfoTable, SWT.NONE);
		requestsColumn.setText("Number of Parallel Address Channels");
		requestsColumn.setWidth(225);
		
		//TableColumn maximizeThroughputColumn = new TableColumn(streamInfoTable, SWT.NONE);
		//maximizeThroughputColumn.setText("Maximize Throughput");
		//maximizeThroughputColumn.setWidth(150);
		
		Button add = new Button(buttonsComp, SWT.PUSH);
		add.setText("Add");
		add.setLayoutData(CompositeUtilities.createNewGD(GridData.GRAB_HORIZONTAL));
		
		add.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				widgetSelected(e);
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleEditor();
				
				//Open up the AddStreamInfoWizard
				AddStreamInfoWizard wizard = new AddStreamInfoWizard(true);
				
				if(EclipseResourceUtils.openWizard(wizard) == Window.CANCEL)
					return;
				
				streamInfo.add(new String[]{wizard.streamName, wizard.elementsRead, wizard.memoryRequests, "1"});
				new TableItem(streamInfoTable, SWT.NONE).setText(new String[]{wizard.streamName, wizard.elementsRead, wizard.memoryRequests});
				createThroughputCheckBox(wizard.maximizeThroughput);
			}			
		});
		
		Button remove = new Button(buttonsComp, SWT.PUSH);
		remove.setText("Remove");
		remove.setLayoutData(CompositeUtilities.createNewGD(GridData.GRAB_HORIZONTAL));
		
		remove.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				widgetSelected(e);
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleEditor();
				
				selectionIndex = streamInfoTable.getSelectionIndex();
				
				//checkButtons.get(selectionIndex).dispose();
				//checkButtons.remove(selectionIndex);
				
				streamInfo.remove(selectionIndex);
				streamInfoTable.remove(selectionIndex);
				
				streamInfoTable.notifyListeners(SWT.SELECTED, null);
			}
			
		});
		
		editor = new TableEditor(streamInfoTable);
		editor.horizontalAlignment = SWT.LEFT;
		editor.grabHorizontal = true;
		editor.minimumWidth = 50;
		
		streamInfoTable.addMouseListener(new MouseListener()
		{
			class myModListener implements ModifyListener 
			{
				private int col;
				myModListener(int c)
				{
					col = c;
				}
				public void modifyText(ModifyEvent e)
				{
					try
					{
						Text text = (Text)editor.getEditor();
						editor.getItem().setText(col, text.getText());
						streamInfo.get(selectionIndex)[editor.getColumn()] = new String(text.getText());
					}
					catch(Exception e2)
					{
						e2.printStackTrace();
					}
				}
			};
			public void mouseDown(MouseEvent event)
			{
				handleEditor();
			}
			public void mouseDoubleClick(MouseEvent event)
			{
				try
				{
					if(event.button == 1)
					{
						int editColumn = 0;
						// Clean up any previous editor control
						Control oldEditor = editor.getEditor();
						if (oldEditor != null) 
							oldEditor.dispose();
						
						//Get the item we clicked on
						final TableItem item = streamInfoTable.getItem(new Point(event.x, event.y));
						
						selectionIndex = streamInfoTable.getSelectionIndex();
						if (item == null || selectionIndex == -1)
						{
							return;
						}
						
						// The control that will be the editor must be a child of the Table
						final Text newEditor = new Text(streamInfoTable, SWT.NONE);
						
						//Find out which column the area to edit is.
						for(int i = 0, total = 0; i < item.getParent().getColumnCount(); ++i)
						{
							total += item.getParent().getColumn(i).getWidth();
							if(event.x < total)
							{
								editColumn = i;
								break;
							}
						}
						
						if(editColumn == 3)
							return;
						
						selectedRow = streamInfoTable.getSelectionIndex();
						
						newEditor.setText(item.getText(editColumn));
						
						newEditor.addModifyListener(new myModListener(editColumn));
						newEditor.selectAll();
						newEditor.setFocus();
						newEditor.addKeyListener(new KeyListener()
						{
							public void keyPressed(KeyEvent e){}
							public void keyReleased(KeyEvent e)
							{
								if( e.character == SWT.CR )
								{
									handleEditor();
								}
							}
						});
						
						editor.setEditor(newEditor, item, editColumn);
					}
				}
				catch(Exception e)
				{
					e.printStackTrace();
				}
				
			}
			public void mouseUp(MouseEvent event)
			{
			}			
		});
		
		CompositeUtilities.setCompositeSize(tableComp, streamInfoTable, streamInfoTable.getItemHeight() * 4, streamInfoTable.getItemHeight() * 7);
		
		
	}
	
	private void createOutputInfoTable(Composite parent)
	{
		Composite seperator = CompositeUtilities.createDefaultComposite(parent, 2, false);
		Composite tableComp = CompositeUtilities.createDefaultComposite(seperator, 1, false);
		Composite buttonsComp = CompositeUtilities.createDefaultComposite(seperator, 1, false);
		
		outputStreamInfoTable = new Table(tableComp, SWT.FULL_SELECTION | SWT.BORDER);
		outputStreamInfoTable.setHeaderVisible(true);
		outputStreamInfoTable.setLinesVisible(true);
		outputStreamInfoTable.setLayoutData(CompositeUtilities.createNewGD(GridData.GRAB_VERTICAL));
		
		outputStreamInfoTable.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				//widgetDefaultSelected(e);
			}

			public void widgetSelected(SelectionEvent e)
			{
				handleOutputEditor();
				selectionIndex = outputStreamInfoTable.getSelectionIndex();
			}
		});
		
		TableColumn streamName = new TableColumn(outputStreamInfoTable, SWT.NONE);
		streamName.setText("Output Stream Name");
		streamName.setWidth(150);
		
		TableColumn amountColumn = new TableColumn(outputStreamInfoTable, SWT.NONE);
		amountColumn.setText("Number of Parallel Data Channels");
		amountColumn.setWidth(225);		
		
		TableColumn requestsColumn = new TableColumn(outputStreamInfoTable, SWT.NONE);
		requestsColumn.setText("Number of Parallel Address Channels");
		requestsColumn.setWidth(225);
		
		Button add = new Button(buttonsComp, SWT.PUSH);
		add.setText("Add");
		add.setLayoutData(CompositeUtilities.createNewGD(GridData.GRAB_HORIZONTAL));
		
		add.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				widgetSelected(e);
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleOutputEditor();
				
				//Open up the AddStreamInfoWizard
				AddStreamInfoWizard wizard = new AddStreamInfoWizard(false);
				
				if(EclipseResourceUtils.openWizard(wizard) == Window.CANCEL)
					return;
				
				outputStreamInfo.add(new String[]{wizard.streamName, wizard.elementsRead, wizard.memoryRequests});
				new TableItem(outputStreamInfoTable, SWT.NONE).setText(new String[]{wizard.streamName, wizard.elementsRead, wizard.memoryRequests});
			}			
		});
		
		Button remove = new Button(buttonsComp, SWT.PUSH);
		remove.setText("Remove");
		remove.setLayoutData(CompositeUtilities.createNewGD(GridData.GRAB_HORIZONTAL));
		
		remove.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				widgetSelected(e);
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleOutputEditor();
				outputStreamInfo.remove(outputStreamInfoTable.getSelectionIndex());
				outputStreamInfoTable.remove(outputStreamInfoTable.getSelectionIndex());
			}
			
		});
		
		outputEditor = new TableEditor(outputStreamInfoTable);
		outputEditor.horizontalAlignment = SWT.LEFT;
		outputEditor.grabHorizontal = true;
		outputEditor.minimumWidth = 50;
		
		outputStreamInfoTable.addMouseListener(new MouseListener()
		{
			class myModListener implements ModifyListener 
			{
				private int col;
				myModListener(int c)
				{
					col = c;
				}
				public void modifyText(ModifyEvent e)
				{
					try
					{
						Text text = (Text)outputEditor.getEditor();
						outputEditor.getItem().setText(col, text.getText());
						outputStreamInfo.get(selectionIndex)[outputEditor.getColumn()] = new String(text.getText());
					}
					catch(Exception e2)
					{
						e2.printStackTrace();
					}
				}
			};
			public void mouseDown(MouseEvent event)
			{
				handleOutputEditor();
			}
			public void mouseDoubleClick(MouseEvent event)
			{
				try
				{
					if(event.button == 1)
					{
						int editColumn = 0;
						// Clean up any previous editor control
						Control oldEditor = outputEditor.getEditor();
						if (oldEditor != null) 
							oldEditor.dispose();
						
						//Get the item we clicked on
						final TableItem item = outputStreamInfoTable.getItem(new Point(event.x, event.y));
						
						selectionIndex = outputStreamInfoTable.getSelectionIndex();
						if (item == null || selectionIndex == -1)
						{
							return;
						}					
						// The control that will be the editor must be a child of the Table
						final Text newEditor = new Text(outputStreamInfoTable, SWT.NONE);
						
						//Find out which column the area to edit is.
						for(int i = 0, total = 0; i < item.getParent().getColumnCount(); ++i)
						{
							total += item.getParent().getColumn(i).getWidth();
							if(event.x < total)
							{
								editColumn = i;
								break;
							}
						}
						
						selectedRow = outputStreamInfoTable.getSelectionIndex();
						
						newEditor.setText(item.getText(editColumn));
						
						newEditor.addModifyListener(new myModListener(editColumn));
						newEditor.selectAll();
						newEditor.setFocus();
						newEditor.addKeyListener(new KeyListener()
						{
							public void keyPressed(KeyEvent e){}
							public void keyReleased(KeyEvent e)
							{
								if( e.character == SWT.CR )
								{
									handleOutputEditor();
								}
							}
						});
						
						outputEditor.setEditor(newEditor, item, editColumn);
					}
				}
				catch(Exception e)
				{
					e.printStackTrace();
				}
				
			}
			public void mouseUp(MouseEvent event)
			{
			}			
		});
		
		CompositeUtilities.setCompositeSize(tableComp, outputStreamInfoTable, outputStreamInfoTable.getItemHeight() * 4, outputStreamInfoTable.getItemHeight() * 7);
	}
	
	private void handleEditor()
	{
		try
		{
			if(streamInfo == null)
				return;
			if(editor == null)
				return;
			if(editor.getEditor() == null)
				return;
			if(selectionIndex == -1)
				return;
			if(streamInfo.size() == 0)
			{
				editor.getEditor().dispose() ;
				return ;
			}
			if(streamInfo.size() <= selectionIndex)
				return;
			if(streamInfo.get(selectionIndex) == null)
				return;
			if(editor.getEditor().isDisposed())
				return;
			String[] newStrings;
			newStrings = streamInfo.get(selectionIndex);
			if(newStrings == null)
				return;
			if(editor.getItem() == null)
				return;
			newStrings[editor.getColumn()] = new String(editor.getItem().getText(editor.getColumn()));
			
			streamInfo.removeElementAt(selectionIndex);
			streamInfo.add(selectionIndex, newStrings);
			editor.getEditor().dispose();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}

	private void handleOutputEditor()
	{
		try
		{
			if(outputStreamInfo == null)
				return;
			if(outputEditor == null)
				return;
			if(outputEditor.getEditor() == null)
				return;
			if(selectionIndex == -1)
				return;
			if(outputStreamInfo.size() == 0)
			{
				outputEditor.getEditor().dispose() ;
				return ;
			}
			if(outputStreamInfo.size() <= selectionIndex)
				return;
			if(outputStreamInfo.get(selectionIndex) == null)
				return;
			if(outputEditor.getEditor().isDisposed())
				return;
			String[] newStrings;
			newStrings = outputStreamInfo.get(selectionIndex);
			if(newStrings == null)
				return;
			if(outputEditor.getItem() == null)
				return;
			newStrings[outputEditor.getColumn()] = new String(outputEditor.getItem().getText(outputEditor.getColumn()));
			
			outputStreamInfo.removeElementAt(selectionIndex);
			outputStreamInfo.add(selectionIndex, newStrings);
			outputEditor.getEditor().dispose();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
}
