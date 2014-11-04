package rocccplugin.composites;

import org.eclipse.jface.util.IPropertyChangeListener;
import org.eclipse.jface.util.PropertyChangeEvent;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.TableEditor;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.KeyListener;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;

import rocccplugin.database.DatabaseInterface;

public class PortTable extends Composite implements IPropertyChangeListener
{
	public Table ports;
	public String component;

	public void propertyChange(PropertyChangeEvent e)
	{	
		try
		{
			ports.removeAll();
			
			if(component != null)
				setComponent(component);
		}
		catch(Exception e1)
		{
			e1.printStackTrace();
		}
		
	}
	
	public PortTable(Composite p, boolean editable)
	{	
		super(p, SWT.NONE);
		
		this.setLayout(new GridLayout());
		ports = new Table(this, SWT.FULL_SELECTION | SWT.HIDE_SELECTION | SWT.BORDER);
		ports.setHeaderVisible(true);
		ports.setLinesVisible(true);
		ports.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));

		TableColumn tableColumn = new TableColumn(ports, SWT.NONE);
		tableColumn.setWidth(200);
		tableColumn.setMoveable(true);
		tableColumn.setText("Port Name");

		TableColumn tableColumn1 = new TableColumn(ports, SWT.NONE);
		tableColumn1.setWidth(100);
		tableColumn1.setMoveable(true);
		tableColumn1.setText("Direction");

		TableColumn tableColumn2 = new TableColumn(ports, SWT.NONE);
		tableColumn2.setWidth(100);
		tableColumn2.setMoveable(true);
		tableColumn2.setText("Size");
		
		TableColumn tableColumn3 = new TableColumn(ports, SWT.NONE);
		tableColumn3.setWidth(100);
		tableColumn3.setMoveable(true);
		tableColumn3.setText("Data Type");
		
		if(editable)
		{
			final TableEditor editor = new TableEditor(ports);
			editor.horizontalAlignment = SWT.LEFT;
			editor.grabHorizontal = true;
			editor.minimumWidth = 50;
			ports.addKeyListener(new KeyListener()
			{
				public void keyPressed(KeyEvent e)
				{
				}
				public void keyReleased(KeyEvent e)
				{
					if( e.character == SWT.BS || e.character == SWT.DEL )
						ports.remove(ports.getSelectionIndices());
				}
			});
			
			ports.addMouseListener(new MouseListener() 
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
					TableItem item = ports.getItem(new Point(e.x, e.y));
					if (item == null)
					{
						//right click where there is no component, add new
						TableItem i = new TableItem(ports, SWT.NONE);
						i.setText(new String[] {"NEW", "in", Integer.toString(0)});
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
					final Text newEditor = new Text(ports, SWT.NONE);
					newEditor.setText(item.getText(EDITABLECOLUMN));

					newEditor.addModifyListener(new myModListener(EDITABLECOLUMN));
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
			});
		}
	}
	
	
	
	public void setComponent(String component_name)
	{
		try
		{
			component = new String(component_name);
			ports.removeAll();
			
			if(!DatabaseInterface.doesComponentExist(component))
			{
				return;
			}
			
			String[] portNames = DatabaseInterface.getPorts(component_name);
			String[] directions = DatabaseInterface.getAllPortInformation(component_name, "direction");
			String[] portSizes = DatabaseInterface.getAllPortInformation(component_name, "bitwidth");	
			String[] dataTypes = DatabaseInterface.getAllPortInformation(component_name, "dataType");
			
			for( int i = 0; i < portNames.length; ++i)
			{
				String port_name = portNames[i];
				String direction = directions[i];
				String port_size = portSizes[i];
				String dataType = dataTypes[i];
				TableItem item = new TableItem(ports, SWT.NONE);
				item.setText(new String[] {port_name, direction, port_size, dataType});
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
	}
	
	@Override
	public void dispose()
	{
		super.dispose();
	}
}
