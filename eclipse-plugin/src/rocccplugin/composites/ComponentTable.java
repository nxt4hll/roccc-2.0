package rocccplugin.composites;

import java.util.Vector;

import org.eclipse.jface.util.IPropertyChangeListener;
import org.eclipse.jface.util.PropertyChangeEvent;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.KeyListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.osgi.framework.Version;

import rocccplugin.Activator;
import rocccplugin.database.DatabaseInterface;

public class ComponentTable extends Composite implements IPropertyChangeListener 
{
	public Table components;
	boolean editable;

	private boolean componentIsUsedByCompilerOnly(String name)
	{
		return !DatabaseInterface.getComponentType(name).equals("SYSTEM") &&
			   !DatabaseInterface.getComponentType(name).equals("MODULE");
	}
	
	public void propertyChange(PropertyChangeEvent e)
	{	
		try
		{
			//if(!Activator.getDefault().openLibraryWithoutWarning())
			//{
			//	components.dispose();
			//	components = null;
			//}
			
			Color red = Display.getDefault().getSystemColor(SWT.COLOR_RED);
				
			
			String selectedComponent = null;
			if(components.getSelectionIndex() != -1)
				selectedComponent = components.getSelection()[0].getText(0);
			
			components.removeAll();
			try
			{
				Vector<String[]> componentValues = new Vector<String[]>();
				
				String[] componentNames = DatabaseInterface.getAllComponents();
				
				for( int i = 0; i < componentNames.length; ++i)
				{
					String componentName = componentNames[i];
				
					if(DatabaseInterface.getComponentType(componentName).equals("SYSTEM"))
						continue;
					if(componentIsUsedByCompilerOnly(componentName))
						continue;
					
					int delay = DatabaseInterface.getDelay(componentName);
					int num_ports = DatabaseInterface.getNumPorts(componentName);
				
					boolean inserted = false;
					
					for(int j = 0; j < componentValues.size(); ++j)
					{
						if(componentValues.get(j)[0].compareToIgnoreCase(componentName) > 0)
						{
							componentValues.insertElementAt(new String[]{componentName, Integer.toString(delay), Integer.toString(num_ports)}, j);
							inserted = true;
							break;
						}
					}
					
					if(!inserted)
						componentValues.add(new String[]{componentName, Integer.toString(delay), Integer.toString(num_ports)});
				}
				
				for(int i = 0; i < componentValues.size(); ++i)
				{
					TableItem item = new TableItem(components, SWT.NONE);
					item.setText(componentValues.get(i));
					
					String versionCompiledOn = DatabaseInterface.versionCompiledOn(componentValues.get(i)[0]);
					
					if(versionCompiledOn != null && versionCompiledOn.equals("NA"))
					{
						item.setText(3, "User Imported");
					
					}
					else if(versionCompiledOn == null || versionCompiledOn.equals("") || 
					   new Version(versionCompiledOn).compareTo(Activator.getMinimumCompilerVersionNeeded()) < 0)
					{
						item.setText(3, "Out of Date");
						item.setForeground(0, red);
						item.setForeground(1, red);
						item.setForeground(2, red);
						item.setForeground(3, red);
					}
				}
			
			}
			catch(Exception ex)
			{
				ex.printStackTrace();
			}
			
			if(selectedComponent != null)
			{
				for(int i = 0; i < components.getItemCount(); ++i)
				{
					if(components.getItem(i).getText(0).equals(selectedComponent))
					{
						components.setSelection(i);
					}
				}
			}
			
			if(editable)
			{
				//final TableEditor editor = new TableEditor(components);
				//editor.horizontalAlignment = SWT.LEFT;
				//editor.grabHorizontal = true;
				//editor.minimumWidth = 50;
				components.addKeyListener(new KeyListener()
				{
					public void keyPressed(KeyEvent e)
					{
						if( e.character == SWT.BS || e.character == SWT.DEL )
						{
							//components.remove(components.getSelectionIndices());
							//DatabaseInterface.updateAllListeners();
						}
					}
					public void keyReleased(KeyEvent e)
					{
						
					}
				});
				/*components.addMouseListener(new MouseListener() 
				{
					class myModListener implements ModifyListener 
					{
						private int col;
						myModListener(int c){col = c;}
						public void modifyText(ModifyEvent e) 
						{
							Text text = (Text)editor.getEditor();
							editor.getItem().setText(col, text.getText());
						}
					};
					public void mouseUp(MouseEvent e) 
					{
						int EDITABLECOLUMN = 1;
						// Clean up any previous editor control
						Control oldEditor = editor.getEditor();
						if (oldEditor != null) oldEditor.dispose();
	
						if(e.button != 3) return;
						// Identify the selected row
						TableItem item = (TableItem)components.getItem(new Point(e.x, e.y));
						if (item == null)
						{
							//right click where there is no component, add new
							TableItem i = new TableItem(components, SWT.NONE);
							i.setText(new String[] {"NEW", Integer.toString(0), Integer.toString(0)});
							return;
						}
						int total = 0;
						for(int i = 0; i < item.getParent().getColumnCount(); ++i)
						{
							total += item.getParent().getColumn(i).getWidth();
							if( e.x < total)
							{
								EDITABLECOLUMN = i;
								break;
							}
						}
						// The control that will be the editor must be a child of the Table
						final Text newEditor = new Text(components, SWT.NONE);
						newEditor.setText(item.getText(EDITABLECOLUMN));
	
						final myModListener L = new myModListener(EDITABLECOLUMN);
						newEditor.addModifyListener(L);
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
						editor.setEditor(newEditor, item, EDITABLECOLUMN);
					}
					public void mouseDoubleClick(MouseEvent e){}
					public void mouseDown(MouseEvent e){}
					
				});*/
			}
		}
		catch(Exception ex)
		{
			ex.printStackTrace();
		}
		
	}
	
	public ComponentTable(Composite p, boolean editable)
	{
		super(p, SWT.NONE);
		this.editable = editable;
		this.setLayout(new GridLayout());
		components = new Table(this, SWT.HIDE_SELECTION | SWT.BORDER);
		components.setHeaderVisible(true);
		components.setLinesVisible(true);
		components.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));

		TableColumn tableColumn = new TableColumn(components, SWT.NONE);
		tableColumn.setWidth(200);
		tableColumn.setMoveable(true);
		tableColumn.setText("Component Name");


		TableColumn tableColumn1 = new TableColumn(components, SWT.NONE);
		tableColumn1.setWidth(100);
		tableColumn1.setMoveable(true);
		tableColumn1.setText("Latency");

		TableColumn tableColumn2 = new TableColumn(components, SWT.NONE);
		tableColumn2.setWidth(100);
		tableColumn2.setMoveable(true);
		tableColumn2.setText("Num Ports");
		
		TableColumn tableColumn3 = new TableColumn(components, SWT.NONE);
		tableColumn3.setWidth(100);
		tableColumn3.setMoveable(true);
		tableColumn3.setText("");
		
		DatabaseInterface.addPropertyChangeListener(this);
		//Activator.getDefault().openLibrary();
	}

	@Override
	public void dispose()
	{
		DatabaseInterface.removePropertyChangeListener(this);
		super.dispose();
	}
}
