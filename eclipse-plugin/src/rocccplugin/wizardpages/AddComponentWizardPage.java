package rocccplugin.wizardpages;

import org.eclipse.jface.dialogs.MessageDialog;
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
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;

import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.StringUtils;

public class AddComponentWizardPage extends WizardPage
{
	public Text component_name;
	public Text delay;
	public Table ports;
	public TableEditor editor;
	public Combo intOrFloat;
	
	//Constructor
	public AddComponentWizardPage(String pageName) 
	{
		super(pageName);
		setTitle("Add IPCore");
		setDescription("Please enter the IPCore information.\n" +
		"This IPCore cannot already exist in the database.");
	}
	
	@Override
	public boolean isPageComplete()
	{
		//Check to see that is has an input and output port.
		boolean has_input = false;
		boolean has_output = false;
		for(int i = 0; i < ports.getItemCount(); ++i)
		{
			if(ports.getItem(i).getText(1).compareTo("IN") == 0)
				has_input = true;
			if(ports.getItem(i).getText(1).compareTo("OUT") == 0)
				has_output = true;
		}
		//If it has a name, size, and both in/out ports, it is finishable.
		return (component_name.getText().length() > 0 && 
				delay.getText().length() > 0 &&
				has_input && has_output);
	}
	
	public void createControl(Composite parent) 
	{
		Composite top = createDefaultComposite(parent,1);
		setControl(top);
		this.setPageComplete(false);

		Group group1 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group1.setText("Component Details");
		group1.setLayout(new GridLayout());
		group1.setLayoutData(createNewGD(0));
		Composite composite1 = createDefaultComposite(group1,2);
		createControl1(composite1);

		Group group2 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group2.setText("Ports");
		group2.setLayout(new GridLayout());
		group2.setLayoutData(createNewGD(GridData.GRAB_VERTICAL));
		createControl2(group2);
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
		new Label(parent,SWT.NONE).setText("Component Name:");
		component_name = new Text(parent,SWT.SINGLE | SWT.BORDER);
		component_name.setLayoutData(createNewGD(0));
		component_name.addModifyListener(updater);
		component_name.addMouseListener(new MouseListener()
		{
			public void mouseDoubleClick(MouseEvent e) 
			{
				
			}

			public void mouseDown(MouseEvent e)
			{
				handleEditor();
			}

			public void mouseUp(MouseEvent e) 
			{
	
			}	
		});

		new Label(parent,SWT.NONE).setText("Latency:");
		delay = new Text(parent,SWT.SINGLE | SWT.BORDER);
		delay.setLayoutData(createNewGD(0));
		delay.addModifyListener(updater);
		delay.addMouseListener(new MouseListener()
		{
			public void mouseDoubleClick(MouseEvent e) 
			{
				
			}

			public void mouseDown(MouseEvent e)
			{
				handleEditor();
			}

			public void mouseUp(MouseEvent e) 
			{
	
			}	
		});
	}
	
	private boolean validInputs(Text portName, Text portSize)
	{
		//Check to see if the port name is valid.
		if(!Character.isLetter(portName.getText().charAt(0)))
		{
			//It did not start with an alphabetic character.
			MessageDialog.openError(new Shell(), "Port Name Error", "Port name \"" + portName.getText() + "\" is invalid.\n\n" +
	                                             "Make sure the port name starts with an alphabetic character.");
			
			//setErrorMessage("Port name \"" + portName.getText() + "\" is invalid. Port names must start with an alphabetic character.");
			return false;
		}
		
		for(int i = 0; i < portName.getText().length(); ++i)
		{
			if(!Character.isLetter(portName.getText().charAt(i)) &&
			   !Character.isDigit(portName.getText().charAt(i)) &&
			   portName.getText().charAt(i) != '_')
			{
				//It did not start with an alphabetic character.
				MessageDialog.openError(new Shell(), "Port Name Error", "Port name \"" + portName.getText() + "\" is invalid.\n\n" +
		                                             "Make sure the port name has no special characters.");
				return false;
			}
		}
		
		if(portName.getText().length() == 0)
		{
			//One of the characters was not a number.
			MessageDialog.openError(new Shell(), "Port Name Error", "Enter a port name.");
		
			return false;
		}
		
		if(!Character.isLetter(portName.getText().charAt(portName.getText().length() - 1)) && !Character.isDigit(portName.getText().charAt(portName.getText().length() - 1)))
		{
			//The first character was not a letter.
			MessageUtils.openErrorWindow("Port Name Error", "Port Name \"" + portName.getText() + "\" is invalid.\n\n" +
            									 "Make sure the port name ends with an alphabetic character or number.");
			return false;
		}
		
		if(StringUtils.isCPlusPlusReservedWord(portName.getText()))
		{
			//It did not start with an alphabetic character.
			MessageDialog.openInformation(new Shell(), "Port Name Error", "Port name \"" + portName.getText() + "\" is reserved by C++.\n\n" +
	                                                   "Make sure the port name is not reserved by C++.");
			return false;
		}
		
		for(int i = 0; i < ports.getItemCount(); ++i)
		{
			if(StringUtils.isCPlusPlusReservedWord(ports.getItem(i).getText(0)))
			{
				//It did not start with an alphabetic character.
				MessageDialog.openInformation(new Shell(), "Port Name Error", "Port name \"" + ports.getItem(i).getText(0) + "\" is reserved by C++.\n\n" +
		                                                   "Make sure the port names are not reserved by C++.");
				return false;
			}
		}
		
		//Check to see if the port name matches any registered port name already.
		for(int i = 0; i < ports.getItemCount(); ++i)
		{ 	
			String registeredPortName = ports.getItem(i).getText(0);
			
			if(registeredPortName.compareToIgnoreCase(portName.getText()) == 0)
			{
				//It matched an already registered port name.
				MessageDialog.openError(new Shell(), "Port Name Error", "Port name \"" + portName.getText() +"\" matches an already registered port.\n\n" +
		                                                   "Choose a name that does not match an already registered port.\n\n" +
		                                                   "Note: Port names are not case sensitive.");
				return false;
			}
		}
		
		//Check to see if the port size is a valid number.
		for(int j = 0; j < portSize.getText().length(); ++j)
		{
			if(!Character.isDigit(portSize.getText().charAt(j)))
			{
				//One of the characters was not a number.
				MessageDialog.openError(new Shell(), "Port Size Error", "Port size \"" + portSize.getText() + 
											  "\" is invalid.\n\n Make sure the port size is a positive integer number.");
				return false;
			}
		}
		
		
		if(Integer.parseInt(portSize.getText()) <= 0)
		{
			//The Number was less than 0.
			MessageDialog.openError(new Shell(), "Port Size Error", "Port size \"" + portSize.getText() + 
										  "\" is invalid.\n\n Make sure the port size is a positive integer number.");
			return false;
		}
		
		if(portSize.getText().length() == 0)
		{
			//One of the characters was not a number.
			MessageDialog.openError(new Shell(), "Port Size Error", "Enter a value for the port size.");
			return false;
		}
		
		if(intOrFloat.getText().equals("float"))
		{
			int bitSize = Integer.parseInt(portSize.getText());
			if(bitSize != 32 && bitSize != 64 && bitSize != 16)
			{
				MessageDialog.openInformation(new Shell(), "Float Port Size Error", "Floats can only have a bitsize of 16, 32, or 64.");
				return false;
			}
		}
		
		return true;
	}
	
	private void createControl2(Composite parent)
	{
		Composite apComp = createDefaultComposite(parent, 4);
		Label lab = new Label(apComp, SWT.CENTER);
		lab.setText("Port Name");
		lab.setLayoutData(new GridData(GridData.FILL_BOTH));
		lab = new Label(apComp, SWT.CENTER);
		lab.setText("Direction");
		lab.setLayoutData(new GridData(0));
		lab = new Label(apComp, SWT.CENTER);
		lab.setText("Size");
		lab.setLayoutData(new GridData(GridData.FILL_BOTH));
		lab = new Label(apComp, SWT.CENTER);
		lab.setText("Type");
		final Text portName = new Text(apComp,SWT.SINGLE | SWT.BORDER);
		portName.setLayoutData(createNewGD(0));
		final Combo direction1 = new Combo(apComp, SWT.READ_ONLY);
		direction1.add("IN");
		direction1.add("OUT");
		direction1.select(0);
		final Text portSize = new Text(apComp, SWT.SINGLE | SWT.BORDER);
		portSize.setLayoutData(createNewGD(0));
		
		intOrFloat = new Combo(apComp, SWT.READ_ONLY);
		intOrFloat.add("int");
		intOrFloat.add("float");
		intOrFloat.select(0);
		
		//Button update = new Button(apComp, SWT.PUSH);
		//update.setText("Update");
		
		portSize.addMouseListener(new MouseListener()
		{
			public void mouseDoubleClick(MouseEvent e) 
			{
			}

			public void mouseDown(MouseEvent e) 
			{
				handleEditor();
			}

			public void mouseUp(MouseEvent e) 
			{
			}
			
		});
		
		portName.addMouseListener(new MouseListener()
		{
			public void mouseDoubleClick(MouseEvent e) 
			{
			}

			public void mouseDown(MouseEvent e) 
			{
				handleEditor();
			}

			public void mouseUp(MouseEvent e) 
			{
			}
			
		});
		
		direction1.addMouseListener(new MouseListener()
		{
			public void mouseDoubleClick(MouseEvent e) 
			{
			}

			public void mouseDown(MouseEvent e) 
			{
				handleEditor();
			}

			public void mouseUp(MouseEvent e) 
			{
			}
			
		});

		Composite portsComp = createDefaultComposite(parent, 2);
		ports = new Table(portsComp, SWT.FULL_SELECTION | SWT.HIDE_SELECTION | SWT.BORDER);
		ports.setHeaderVisible(true);
		ports.setLinesVisible(true);
		ports.setLayoutData(createNewGD(GridData.GRAB_VERTICAL));

		TableColumn tableColumn = new TableColumn(ports, SWT.NONE);
		tableColumn.setWidth(200);
		tableColumn.setMoveable(true);
		tableColumn.setText("Port Name");

		TableColumn tableColumn1 = new TableColumn(ports, SWT.NONE);
		tableColumn1.setWidth(75);
		tableColumn1.setMoveable(false);
		tableColumn1.setText("Direction");

		TableColumn tableColumn2 = new TableColumn(ports, SWT.NONE);
		tableColumn2.setWidth(75);
		tableColumn2.setMoveable(false);
		tableColumn2.setText("Size");
		
		TableColumn tableColumn3 = new TableColumn(ports, SWT.NONE);
		tableColumn3.setWidth(75);
		tableColumn3.setMoveable(false);
		tableColumn3.setText("Type");
		
		
		
		Composite controlComp = createDefaultComposite(portsComp, 1);
		Button add = new Button(controlComp, SWT.PUSH);
		add.setText("Add");
		add.setLayoutData(createNewGD(0));
		Button remove = new Button(controlComp, SWT.PUSH);
		remove.setText("Delete");
		remove.setLayoutData(createNewGD(0));
		Button up = new Button(controlComp, SWT.ARROW | SWT.UP);
		up.setLayoutData(createNewGD(0));
		Button down = new Button(controlComp, SWT.ARROW | SWT.DOWN);
		down.setLayoutData(createNewGD(0));

		//Add the selection listener for the "Add" button.
		add.addSelectionListener(new SelectionListener() 
		{
			public void widgetSelected(SelectionEvent event) 
			{
				handleEditor();
				if(validInputs(portName, portSize) == false)
					return;
					
				TableItem item = new TableItem(ports, SWT.NONE);
				item.setText(new String[] {portName.getText(), direction1.getText(), portSize.getText(), intOrFloat.getText()});
				ports.select(ports.getItemCount()-1);
				getContainer().updateButtons();
			}
			public void widgetDefaultSelected(SelectionEvent event) 
			{
				widgetSelected(event);
			}
		});
		//Add the selection listener for the "Remove" button.
		remove.addSelectionListener(new SelectionListener() 
		{
			public void widgetSelected(SelectionEvent event) 
			{
				handleEditor();
				int[] items = ports.getSelectionIndices();
				ports.remove(items);
				if( items != null )
					ports.select(items[0]);
				getContainer().updateButtons();
			}
			public void widgetDefaultSelected(SelectionEvent event) 
			{
				widgetSelected(event);
			}
		});
		//Add the selection listener for the "Up" button
		up.addSelectionListener(new SelectionListener()
		{
			public void widgetSelected(SelectionEvent event) 
			{
				handleEditor();
				if(ports.getSelection() != null)
				{
					if(ports.getSelectionIndex() != 0)
					{
						String abovePort = ports.getSelection()[0].getText(0);
						String abovePortDir = ports.getSelection()[0].getText(1);
						String abovePortSize = ports.getSelection()[0].getText(2);
						String aboveType = ports.getSelection()[0].getText(3);
						
						ports.getSelection()[0].setText(0, ports.getItem(ports.getSelectionIndex() - 1).getText(0));
						ports.getSelection()[0].setText(1, ports.getItem(ports.getSelectionIndex() - 1).getText(1));
						ports.getSelection()[0].setText(2, ports.getItem(ports.getSelectionIndex() - 1).getText(2));
						ports.getSelection()[0].setText(3, ports.getItem(ports.getSelectionIndex() - 1).getText(3));
						
						ports.getItem(ports.getSelectionIndex() - 1).setText(0, abovePort);
						ports.getItem(ports.getSelectionIndex() - 1).setText(1, abovePortDir);
						ports.getItem(ports.getSelectionIndex() - 1).setText(2, abovePortSize);
						ports.getItem(ports.getSelectionIndex() - 1).setText(3, aboveType);
						
						ports.setSelection(ports.getSelectionIndex() - 1);
					}
				}
			}
			public void widgetDefaultSelected(SelectionEvent event) 
			{
				widgetSelected(event);
			}
		});
		//Add the selection listener for the "Down" button.
		down.addSelectionListener(new SelectionListener() 
		{
			public void widgetSelected(SelectionEvent event) 
			{
				handleEditor();
				if(ports.getSelection() != null)
				{
					if(ports.getSelectionIndex() != ports.getItemCount() - 1)
					{
						String belowPort = ports.getSelection()[0].getText(0);
						String belowPortDir = ports.getSelection()[0].getText(1);
						String belowPortSize = ports.getSelection()[0].getText(2);
						String belowType = ports.getSelection()[0].getText(3);
					
						ports.getSelection()[0].setText(0, ports.getItem(ports.getSelectionIndex() + 1).getText(0));
						ports.getSelection()[0].setText(1, ports.getItem(ports.getSelectionIndex() + 1).getText(1));
						ports.getSelection()[0].setText(2, ports.getItem(ports.getSelectionIndex() + 1).getText(2));
						ports.getSelection()[0].setText(3, ports.getItem(ports.getSelectionIndex() + 1).getText(3));
						
						ports.getItem(ports.getSelectionIndex() + 1).setText(0, belowPort);
						ports.getItem(ports.getSelectionIndex() + 1).setText(1, belowPortDir);
						ports.getItem(ports.getSelectionIndex() + 1).setText(2, belowPortSize);
						ports.getItem(ports.getSelectionIndex() + 1).setText(3, belowType);
						
						ports.setSelection(ports.getSelectionIndex() + 1);
					}
				}
			}
			public void widgetDefaultSelected(SelectionEvent event) 
			{
				widgetSelected(event);
			}
		});
		//Add the selection listener for the "Update" button.
		/*update.addSelectionListener(new SelectionListener() 
		{
			public void widgetSelected(SelectionEvent event) 
			{
				if(ports.getSelection() != null)
				{
					if(validInputs(portName, portSize) == false)
						return;
					
					TableItem item = ports.getSelection()[0];
					item.setText(0, portName.getText());
					item.setText(1, direction1.getText());
					item.setText(2, portSize.getText());
				}
			}
			public void widgetDefaultSelected(SelectionEvent event) 
			{
				widgetSelected(event);
			}
		});*/
		//Add the selection listener for the "Ports" button.
		ports.addSelectionListener(new SelectionListener() 
		{
			public void widgetSelected(SelectionEvent event) 
			{
				//handleEditor();
				if(ports.getSelection() != null)
				{
					TableItem item = ports.getSelection()[0];
					portName.setText(item.getText(0));
					direction1.setText(item.getText(1));
					portSize.setText(item.getText(2));
					intOrFloat.setText(item.getText(3));
				}
			}
			public void widgetDefaultSelected(SelectionEvent event) 
			{
				widgetSelected(event);
			}
		});

		editor = new TableEditor(ports);
		editor.horizontalAlignment = SWT.LEFT;
		editor.grabHorizontal = true;
		editor.minimumWidth = 50;
		
		ports.addMouseListener(new MouseListener()
		{
			class mySelectionListener implements SelectionListener
			{
				private int col;
				mySelectionListener(int c)
				{
					col = c;
				}
				
				public void widgetDefaultSelected(SelectionEvent e) 
				{
					widgetSelected(e);
				}

				public void widgetSelected(SelectionEvent e)
				{
					Combo c = ((Combo) editor.getEditor());
					String text = c.getItem(c.getSelectionIndex());
					editor.getItem().setText(col, text);
					setPageComplete(isPageComplete());
				}
			};
			class myModListener implements ModifyListener 
			{
				private int col;
				myModListener(int c)
				{
					col = c;
				}
				public void modifyText(ModifyEvent e)
				{
					Text text = (Text)editor.getEditor();
					editor.getItem().setText(col, text.getText());
				}
			};
			public void mouseDown(MouseEvent event)
			{
				handleEditor();
			}
			public void mouseDoubleClick(MouseEvent event)
			{
				if(event.button == 1)
				{
					int editColumn = 0;
					// Clean up any previous editor control
					Control oldEditor = editor.getEditor();
					if (oldEditor != null) oldEditor.dispose();
					
					TableItem item = ports.getItem(new Point(event.x, event.y));
					if (item == null)
					{
						return;
					}					
					// The control that will be the editor must be a child of the Table
					final Text newEditor = new Text(ports, SWT.NONE);
					
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
					
					if(editColumn == 1)
					{
						Combo combo = new Combo (ports, SWT.READ_ONLY);
				        if(ports.getItem(ports.getSelectionIndex()).getText(1).toString().compareTo("in") == 0)
				        {
				        	combo.add("IN");
				        	combo.add("OUT");
				        	combo.select(0);
				        }
				        else
				        {
				        	combo.add("OUT");
				        	combo.add("IN");
				        	combo.select(0);
				        }
				        
				        editor.grabHorizontal = true;
				        combo.addSelectionListener(new mySelectionListener(editColumn));
				        editor.setEditor(combo, item, editColumn);
						return;
					}
					if(editColumn == 3)
					{
						Combo combo = new Combo (ports, SWT.READ_ONLY);
				        if(ports.getItem(ports.getSelectionIndex()).getText(3).toString().compareTo("int") == 0)
				        {
				        	combo.add("int");
				        	combo.add("float");
				        	combo.select(0);
				        }
				        else
				        {
				        	combo.add("float");
				        	combo.add("int");
				        	combo.select(0);
				        }
				        
				        editor.grabHorizontal = true;
				        combo.addSelectionListener(new mySelectionListener(editColumn));
				        editor.setEditor(combo, item, editColumn);
						return;
					}
					
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
								editor.getEditor().dispose();
						}
					});
					editor.setEditor(newEditor, item, editColumn);
					
				}
			}
			public void mouseUp(MouseEvent event)
			{
			}			
		});
		
		
		
		/*final Table table2 = new Table(parent, SWT.FULL_SELECTION | SWT.HIDE_SELECTION);
			TableColumn column1 = new TableColumn(table2, SWT.NONE);
			TableColumn column2 = new TableColumn(table2, SWT.NONE);
			for (int i = 0; i < 10; i++) {
				TableItem item = new TableItem(table2, SWT.NONE);
				item.setText(new String[] {"item " + i, "edit this value"});
			}
			column1.pack();
			column2.pack();

			final TableEditor editor = new TableEditor(table2);
			//The editor must have the same size as the cell and must
			//not be any smaller than 50 pixels.
			editor.horizontalAlignment = SWT.LEFT;
			editor.grabHorizontal = true;
			editor.minimumWidth = 50;
			// editing the second column
			final int EDITABLECOLUMN = 1;

			table.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					// Clean up any previous editor control
					Control oldEditor = editor.getEditor();
					if (oldEditor != null) oldEditor.dispose();

					// Identify the selected row
					TableItem item = (TableItem)e.item;
					if (item == null) return;

					// The control that will be the editor must be a child of the Table
					Text newEditor = new Text(table2, SWT.NONE);
					newEditor.setText(item.getText(EDITABLECOLUMN));
					newEditor.addModifyListener(new ModifyListener() {
						public void modifyText(ModifyEvent e) {
							Text text = (Text)editor.getEditor();
							editor.getItem().setText(EDITABLECOLUMN, text.getText());
						}
					});
					newEditor.selectAll();
					newEditor.setFocus();
					editor.setEditor(newEditor, item, EDITABLECOLUMN);
				}
			});*/
	}
	
	private void handleEditor()
	{
		try
		{
			if(editor == null)
				return;
			if(editor.getEditor() == null)
				return;
			editor.getEditor().dispose();
		}
		catch(Exception e)
		{
			e.printStackTrace();
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
		//layout.makeColumnsEqualWidth = true;
		composite.setLayout(layout);
		composite.setLayoutData(createNewGD(0));
		return composite;
	}
}
