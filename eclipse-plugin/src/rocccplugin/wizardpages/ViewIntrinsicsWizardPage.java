package rocccplugin.wizardpages;

import java.util.Vector;

import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardDialog;
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
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;

import rocccplugin.Activator;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.CompositeUtilities;
import rocccplugin.wizards.ImportIntrinsicWizard;

public class ViewIntrinsicsWizardPage extends WizardPage 
{ 
	Table types;
	Table intrinsics;
	Button activateButton;
	Button deactivateButton;
	Button deleteButton;
	Button addButton;
	TableEditor editor;
	int selectionIndex;
	int typeIndex;
	TableColumn bitSizeColumn;
	TableColumn bitSizeColumn2;
	
	final int INT_DIV_ROW = 0;
	final int INT_MOD_ROW = 1;
	final int FP_ADD_ROW = 2;
	final int FP_SUB_ROW = 3;
	final int FP_MUL_ROW = 4;
	final int FP_DIV_ROW = 5;
	final int FP_GREATER_THAN_ROW = 6;
	final int FP_LESS_THAN_ROW = 7;
	final int FP_EQUAL_ROW = 8;
	final int FP_GREATER_THAN_EQUAL_ROW = 9;
	final int FP_LESS_THAN_EQUAL_ROW = 10;
	final int FP_NOT_EQUAL_ROW = 11;
	final int FP_TO_INT_ROW = 12;
	final int INT_TO_FP_ROW = 13;
	final int FP_TO_FP_ROW = 14;
	final int DOUBLE_VOTE_ROW = 15;
	final int TRIPLE_VOTE_ROW = 16;
	
	final int NAME_COLUMN = 0;
	final int BITSIZE_COLUMN = 1;
	final int BITSIZE_COLUMN2 = 2;
	final int DELAY_COLUMN = 3;
	final int DESCRIPTION_COLUMN = 4;
	final int ACTIVE_COLUMN = 5;
	
	public Vector<Vector<String[]> > intrinsicValues = new Vector<Vector<String[]>>();
	public Vector<String> deletedComponents = new Vector<String>();
	
	public ViewIntrinsicsWizardPage(String pageName) 
	{
		super(pageName);
		setTitle("Intrinsics List");
		setDescription("Add, delete, or edit intrinsics in the database.");
	}
	
	public void createControl(Composite parent) 
	{
		Composite top = CompositeUtilities.createDefaultComposite(parent,1, false);
		setControl(top);
		
		Group group1 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group1.setText("Intrinsics Info");
		group1.setLayout(new GridLayout());
		group1.setLayoutData(CompositeUtilities.createNewGD(0));
		
		Composite groupComp = CompositeUtilities.createDefaultComposite(group1, 4, false);
		
		createTypeTable(groupComp);
		new Label(groupComp, SWT.NONE).setText("   ");
		createIntrinsicsTable(groupComp);
		createTableButtons(groupComp);
		
		createLogic();
	}
	
	private void createTypeTable(Composite parent)
	{
		types = new Table(parent, SWT.FULL_SELECTION | SWT.BORDER);
		types.setHeaderVisible(true);
		types.setLinesVisible(true);
		types.setLayoutData(CompositeUtilities.createNewGD(GridData.GRAB_VERTICAL));
	
		TableColumn typesColumn = new TableColumn(types, SWT.NONE);
		typesColumn.setText("Intrinsic Type");
		typesColumn.setWidth(150);
		
		new TableItem(types, SWT.NONE).setText(new String("int_div"));
		new TableItem(types, SWT.NONE).setText(new String("int_mod"));
		new TableItem(types, SWT.NONE).setText(new String("fp_add"));
		new TableItem(types, SWT.NONE).setText(new String("fp_sub"));
		new TableItem(types, SWT.NONE).setText(new String("fp_mul"));
		new TableItem(types, SWT.NONE).setText(new String("fp_div"));
		new TableItem(types, SWT.NONE).setText(new String("fp_greater_than"));
		new TableItem(types, SWT.NONE).setText(new String("fp_less_than"));
		new TableItem(types, SWT.NONE).setText(new String("fp_equal"));
		new TableItem(types, SWT.NONE).setText(new String("fp_greater_than_equal"));
		new TableItem(types, SWT.NONE).setText(new String("fp_less_than_equal"));
		new TableItem(types, SWT.NONE).setText(new String("fp_not_equal"));
		new TableItem(types, SWT.NONE).setText(new String("fp_to_int"));
		new TableItem(types, SWT.NONE).setText(new String("int_to_fp"));
		new TableItem(types, SWT.NONE).setText(new String("fp_to_fp"));
		new TableItem(types, SWT.NONE).setText(new String("double_vote"));
		new TableItem(types, SWT.NONE).setText(new String("triple_vote"));
		
		CompositeUtilities.setCompositeSize(parent, types, types.getItemHeight() * 7, types.getItemHeight() * 10);
	}

	private void createIntrinsicsTable(Composite parent)
	{
		intrinsics = new Table(parent, SWT.FULL_SELECTION | SWT.BORDER);
		intrinsics.setHeaderVisible(true);
		intrinsics.setLinesVisible(true);
		intrinsics.setLayoutData(CompositeUtilities.createNewGD(GridData.GRAB_VERTICAL));
		
		TableColumn nameColumn = new TableColumn(intrinsics, SWT.NONE);
		nameColumn.setText("Name");
		nameColumn.setWidth(100);
		
		bitSizeColumn = new TableColumn(intrinsics, SWT.NONE);
		bitSizeColumn.setText("Bit Size");
		bitSizeColumn.setWidth(75);
		
		bitSizeColumn2 = new TableColumn(intrinsics, SWT.NONE);
		bitSizeColumn2.setText("Bit Size 2");
		bitSizeColumn2.setWidth(75);
		
		TableColumn delayColumn = new TableColumn(intrinsics, SWT.NONE);
		delayColumn.setText("Latency");
		delayColumn.setWidth(75);
		
		TableColumn descriptionColumn = new TableColumn(intrinsics, SWT.NONE);
		descriptionColumn.setText("Description");
		descriptionColumn.setWidth(175);
		
		TableColumn activeColumn = new TableColumn(intrinsics, SWT.CENTER);
		activeColumn.setText("Active");
		activeColumn.setWidth(50);		
		
		CompositeUtilities.setCompositeSize(parent, intrinsics, intrinsics.getItemHeight() * 7, intrinsics.getItemHeight() * 10);
	}

	private void createTableButtons(Composite parent)
	{
		Composite buttonsComp = CompositeUtilities.createDefaultComposite(parent, 1, false);
		
		activateButton = new Button(buttonsComp, SWT.PUSH);
		deactivateButton = new Button(buttonsComp, SWT.PUSH);
		addButton = new Button(buttonsComp, SWT.PUSH);
		deleteButton = new Button(buttonsComp, SWT.PUSH);
		
		activateButton.setText("Activate");
		deactivateButton.setText("Deactivate");
		addButton.setText("Add");
		deleteButton.setText("Delete");
		
		activateButton.setLayoutData(CompositeUtilities.createNewGD(GridData.FILL_HORIZONTAL));
		deactivateButton.setLayoutData(CompositeUtilities.createNewGD(GridData.FILL_HORIZONTAL));
		addButton.setLayoutData(CompositeUtilities.createNewGD(GridData.FILL_HORIZONTAL));
		deleteButton.setLayoutData(CompositeUtilities.createNewGD(GridData.FILL_HORIZONTAL));
	}
	
	private void createLogic()
	{
		types.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
			}

			public void widgetSelected(SelectionEvent e) 
			{	
				handleEditor();
				updateIntrinsicTable();
				intrinsics.select(0);
				typeIndex = types.getSelectionIndex();
			}
		});
		
		activateButton.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
			}

			public void widgetSelected(SelectionEvent e) 
			{	
				handleEditor();
				if(intrinsics.getSelectionIndex() == -1)
					return;
				if(types.getSelectionIndex() == -1)
					return;
				
				int typeIndex = types.getSelectionIndex();
				
				for(int i = 0; i < intrinsicValues.get(typeIndex).size(); ++i)
				{
					if(intrinsicValues.get(typeIndex).get(i)[BITSIZE_COLUMN].equals(intrinsics.getSelection()[0].getText(BITSIZE_COLUMN)))
					{
						if(typeIndex == FP_TO_INT_ROW || typeIndex == INT_TO_FP_ROW || typeIndex == FP_TO_FP_ROW)
						{
							if(intrinsicValues.get(typeIndex).get(i)[BITSIZE_COLUMN2].equals(intrinsics.getSelection()[0].getText(BITSIZE_COLUMN2)))
							{
								intrinsicValues.get(typeIndex).get(i)[ACTIVE_COLUMN] = "";
							}
						}
						else
							intrinsicValues.get(typeIndex).get(i)[ACTIVE_COLUMN] = "";
					}
				}
				
				intrinsicValues.get(typeIndex).get(intrinsics.getSelectionIndex())[ACTIVE_COLUMN] = "*";
				
				int selection = intrinsics.getSelectionIndex();
				refreshIntrinsicTable();
				intrinsics.select(selection);
			}
		});
		
		deactivateButton.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
			}

			public void widgetSelected(SelectionEvent e) 
			{	
				try
				{
					handleEditor();
					if(intrinsics.getSelectionIndex() == -1)
						return;
					if(types.getSelectionIndex() == -1)
						return;
					if(intrinsics.getSelection()[0].getText(ACTIVE_COLUMN).equals(""))
						return;
					
					intrinsicValues.get(types.getSelectionIndex()).get(intrinsics.getSelectionIndex())[ACTIVE_COLUMN] = "";
					int selection = intrinsics.getSelectionIndex();
					refreshIntrinsicTable();
					intrinsics.select(selection);
				}
				catch(Exception ex)
				{
					ex.printStackTrace();
				}
			}
		});
		
		deleteButton.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{	
			}

			public void widgetSelected(SelectionEvent e) 
			{	
				handleEditor();
				if(intrinsics.getSelectionIndex() == -1)
					return;
				if(types.getSelectionIndex() == -1)
					return;
				
				intrinsicValues.get(types.getSelectionIndex()).remove(intrinsics.getSelectionIndex());
				intrinsics.remove(intrinsics.getSelectionIndex());
				refreshIntrinsicTable();
			}
		});
		
		addButton.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleEditor();
				ImportIntrinsicWizard wizard = new ImportIntrinsicWizard(true, typeIndex);
				IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
				
				WizardDialog dialog = new WizardDialog(window.getShell(), wizard);
				dialog.setHelpAvailable(false);
				if(dialog.open() == Window.CANCEL)
				{
					return;
				}
				
				if(wizard.deletedComponent != null)
				{
					deletedComponents.add(wizard.deletedComponent);
 				}
				
				for(int i = 0; i < intrinsicValues.size(); ++i)
				{
					for(int j = 0; j < intrinsicValues.get(i).size(); ++j)
					{
						if(intrinsicValues.get(i).get(j)[0].equals(wizard.name))
						{
							intrinsicValues.get(i).remove(j);
							break;
						}
					}
				}
				
				if(wizard.active)
				{
					for(int i = 0; i < intrinsicValues.get(wizard.row).size(); ++i)
					{
						if(intrinsicValues.get(wizard.row).get(i)[BITSIZE_COLUMN].equals(Integer.toString(wizard.bitSize)))
						{
							if(wizard.bitSize2Visible)
							{
								if(intrinsicValues.get(wizard.row).get(i)[BITSIZE_COLUMN2].equals(Integer.toString(wizard.bitSize2)))
								{
									intrinsicValues.get(wizard.row).get(i)[ACTIVE_COLUMN] = "";
								}
							}
							else
								intrinsicValues.get(wizard.row).get(i)[ACTIVE_COLUMN] = "";
						}
					}
				}
				
				intrinsicValues.get(wizard.row).add(new String[]{wizard.name, Integer.toString(wizard.bitSize), wizard.bitSize2Visible? Integer.toString(wizard.bitSize2) : "NA", Integer.toString(wizard.delay), wizard.description, wizard.active? "*" : ""});
				updateIntrinsicTable();
			}
		});
		
		editor = new TableEditor(intrinsics);
		editor.horizontalAlignment = SWT.LEFT;
		editor.grabHorizontal = true;
		editor.minimumWidth = 50;
		
		intrinsics.addMouseListener(new MouseListener()
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
						intrinsics.getItem(selectionIndex).setText(editor.getColumn(), new String(text.getText()));
						intrinsicValues.get(typeIndex).get(selectionIndex)[editor.getColumn()] = new String(text.getText());
					}
					catch(Exception e2)
					{
						e2.printStackTrace();
					}
				}
			};
			
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
						final TableItem item = intrinsics.getItem(new Point(event.x, event.y));
						
						selectionIndex = intrinsics.getSelectionIndex();
						if (item == null)
						{
							return;
						}					
						// The control that will be the editor must be a child of the Table
						final Text newEditor = new Text(intrinsics, SWT.NONE);
						
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
						
						if(editColumn == NAME_COLUMN     ||
						   editColumn == BITSIZE_COLUMN  ||
						   editColumn == BITSIZE_COLUMN2 ||
						   editColumn == ACTIVE_COLUMN)
							return;
						
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

			public void mouseDown(MouseEvent e) 
			{
				handleEditor();
			}

			public void mouseUp(MouseEvent e) 
			{

			}
			
		});
	
		types.select(0);
		fillVectorWithIntrinsics();
		updateIntrinsicTable();
		intrinsics.select(0);
	}
	
	private void fillVectorWithIntrinsics()
	{
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		intrinsicValues.add(new Vector<String[]>());
		
		String[][] components = DatabaseInterface.getComponentsOfType("INT_DIV");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			intrinsicValues.get(INT_DIV_ROW).add(new String[]{components[i][0], bitSize, "NA", components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("INT_MOD");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			intrinsicValues.get(INT_MOD_ROW).add(new String[]{components[i][0], bitSize, "NA", components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("FP_ADD");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			intrinsicValues.get(FP_ADD_ROW).add(new String[]{components[i][0], bitSize, "NA", components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("FP_SUB");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			intrinsicValues.get(FP_SUB_ROW).add(new String[]{components[i][0], bitSize, "NA", components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("FP_MUL");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			intrinsicValues.get(FP_MUL_ROW).add(new String[]{components[i][0], bitSize, "NA", components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("FP_DIV");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			intrinsicValues.get(FP_DIV_ROW).add(new String[]{components[i][0], bitSize, "NA", components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("FP_GREATER_THAN");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			intrinsicValues.get(FP_GREATER_THAN_ROW).add(new String[]{components[i][0], bitSize, "NA", components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("FP_LESS_THAN");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			intrinsicValues.get(FP_LESS_THAN_ROW).add(new String[]{components[i][0], bitSize, "NA", components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("FP_EQUAL");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			intrinsicValues.get(FP_EQUAL_ROW).add(new String[]{components[i][0], bitSize, "NA", components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("FP_GREATER_THAN_EQUAL");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			intrinsicValues.get(FP_GREATER_THAN_EQUAL_ROW).add(new String[]{components[i][0], bitSize, "NA", components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("FP_LESS_THAN_EQUAL");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			intrinsicValues.get(FP_LESS_THAN_EQUAL_ROW).add(new String[]{components[i][0], bitSize, "NA", components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("FP_NOT_EQUAL");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			intrinsicValues.get(FP_NOT_EQUAL_ROW).add(new String[]{components[i][0], bitSize, "NA", components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("FP_TO_INT");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			String bitSize2 = DatabaseInterface.getBitSizeOfIntrinsic2(components[i][0]);
			intrinsicValues.get(FP_TO_INT_ROW).add(new String[]{components[i][0], bitSize, bitSize2, components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("INT_TO_FP");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			String bitSize2 = DatabaseInterface.getBitSizeOfIntrinsic2(components[i][0]);
			intrinsicValues.get(INT_TO_FP_ROW).add(new String[]{components[i][0], bitSize, bitSize2, components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("FP_TO_FP");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			String bitSize2 = DatabaseInterface.getBitSizeOfIntrinsic2(components[i][0]);
			intrinsicValues.get(FP_TO_FP_ROW).add(new String[]{components[i][0], bitSize, bitSize2, components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("DOUBLE_VOTE");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			String bitSize2 = DatabaseInterface.getBitSizeOfIntrinsic2(components[i][0]);
			intrinsicValues.get(DOUBLE_VOTE_ROW).add(new String[]{components[i][0], bitSize2, bitSize2, components[i][1], components[i][2], components[i][3]});
		}
		
		components = DatabaseInterface.getComponentsOfType("TRIPLE_VOTE");
		for(int i = 0; i < components.length; ++i)
		{
			String bitSize = DatabaseInterface.getBitSizeOfIntrinsic(components[i][0]);
			String bitSize2 = DatabaseInterface.getBitSizeOfIntrinsic2(components[i][0]);
			intrinsicValues.get(TRIPLE_VOTE_ROW).add(new String[]{components[i][0], bitSize2, bitSize2, components[i][1], components[i][2], components[i][3]});
		}
	}
	
	private void updateIntrinsicTable()
	{
		String typeName = types.getItem(types.getSelectionIndex()).getText();
		
		Vector<String[]> components = intrinsicValues.get(types.getSelectionIndex());
		
		intrinsics.removeAll();
		for(int i = 0; i < components.size(); ++i)
		{
			TableItem item = new TableItem(intrinsics, SWT.CENTER);
			item.setText(components.get(i));
		}
		
		if(typeName.contains("vote"))
		{
			bitSizeColumn2.setWidth(0);
			bitSizeColumn2.setText("NA");
			
			bitSizeColumn.setText("Bitsize: val");
		}
		else if(!typeName.equals("int_to_fp") && 
		   !typeName.equals("fp_to_int") &&
		   !typeName.equals("fp_to_fp"))
		{
			bitSizeColumn2.setWidth(0);
			bitSizeColumn2.setText("NA");
			
			bitSizeColumn.setText(typeName.startsWith("int")? "Bitsize: int" : "Bitsize: fp");
		}
		else
		{			
			bitSizeColumn.setText(typeName.equals("int_to_fp")? "Bitsize: int" : 
								  typeName.equals("fp_to_int")? "Bitsize: fp"  :
									  							"Bitsize: fp1");
			
			bitSizeColumn2.setText(typeName.equals("fp_to_int")? "Bitsize: int" : 
								  typeName.equals("int_to_fp")? "Bitsize: fp"  :
																"Bitsize: fp2");
			
			bitSizeColumn2.setWidth(75);
		}
		
	}
	
	private void refreshIntrinsicTable()
	{
		String typeName = types.getItem(types.getSelectionIndex()).getText();
		
		Vector<String[]> components = intrinsicValues.get(types.getSelectionIndex());
		
		for(int i = 0; i < components.size(); ++i)
		{
			TableItem item = intrinsics.getItem(i);
			item.setText(components.get(i));
		}
	}
	
	private void handleEditor()
	{
		try
		{
			if(editor == null)
				return;
			if(editor.getEditor() == null)
				return;
			if(selectionIndex == -1)
				return;
			if(intrinsics.getItemCount() == 0)
			{
				editor.getEditor().dispose() ;
				return ;
			}
			if(intrinsics.getItemCount() <= selectionIndex)
				return;
			if(intrinsics.getItem(selectionIndex) == null)
				return;
			if(editor.getEditor().isDisposed())
				return;
			String newStrings;
			newStrings = intrinsics.getItem(selectionIndex).getText(editor.getColumn());
			if(newStrings == null)
				return;
			if(editor.getItem() == null)
				return;
			
			newStrings = new String(editor.getItem().getText(editor.getColumn()));
			
			intrinsicValues.get(typeIndex).get(selectionIndex)[editor.getColumn()] = newStrings;
			refreshIntrinsicTable();
			editor.getEditor().dispose();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
}
