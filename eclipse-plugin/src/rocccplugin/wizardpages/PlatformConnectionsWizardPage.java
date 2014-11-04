package rocccplugin.wizardpages;

import java.io.File;
import java.io.FileInputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Vector;

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
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;

import rocccplugin.Activator;
import rocccplugin.helpers.CategoryData;
import rocccplugin.helpers.ConnectionData;
import rocccplugin.helpers.PlatformData;
import rocccplugin.helpers.ResourceData;
import rocccplugin.preferences.PreferenceConstants;

public class PlatformConnectionsWizardPage extends WizardPage 
{
	private int editColumn;
	private int platformIndex;
	private String resourceName;
	private int selectedRow;
	private int selectedCategory;
	TableEditor editor;
	Table platforms;
	Table resources;
	Table connections;
	Image boardImage;
	Button imageLoc;
	public Vector<PlatformData> platformData;
	public Vector<Vector<ConnectionData>> savedConnections;

	public PlatformConnectionsWizardPage(String pageName) 
	{
		super(pageName);
		String title = "Platform and Connections for filename.c";
		setTitle(pageName);
		setDescription("Select a platform and resources to connect the steams to.");
	}

	private void loadPlatforms()
	{
		String ROCCC_dir = Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION);
		File dir = new File(ROCCC_dir + "/Platforms/");
		
		FilenameFilter filter = new FilenameFilter()
		{
			public boolean accept(File dir, String name)
			{
				return name.endsWith(".plt");
			}
		};
		
		platformData = new Vector<PlatformData>();
		platformData.add(new PlatformData("None"));
		
		savedConnections = new Vector< Vector<ConnectionData>>();
		savedConnections.add(new Vector<ConnectionData>());
		
		TableItem none = new TableItem(platforms, SWT.NONE);
		none.setText("None");
		
		String[] files = dir.list(filter);
		
		for(int i = 0; i < files.length; ++i)
		{
			StringBuffer buf = new StringBuffer();
			try
			{
				FileInputStream fis = new FileInputStream(ROCCC_dir + "/Platforms/" + files[i]);
				InputStreamReader in = new InputStreamReader(fis, "UTF-8");
			
				while(in.ready())
				{
					buf.append((char) in.read());
				}
				in.close();
			} 
			catch (IOException e) 
			{
				e.printStackTrace();
				return;
			}
			finally 
			{
			}
			
			String platform = getLine(buf);
			
			TableItem name = new TableItem(platforms, SWT.NONE);
			name.setText(platform);
			
			platformData.add(new PlatformData(platform));
			savedConnections.add(new Vector<ConnectionData>());
			
			while(buf.length() > 0)
			{	
				platformData.lastElement().addResource(getNextStringValue(buf), getNextStringValue(buf), Integer.parseInt(getNextStringValue(buf)));
			}
		}
		
		platforms.select(0);
	}
	
	private String getLine(StringBuffer buf)
	{
		String val = "";
		
		if(buf.length() == 0)
			return val;
		int min = 0;
	
		min = buf.indexOf("\n");
		if(min == -1)
			min = buf.length() - 1;
		val = buf.substring(0, min);
		buf.delete(0, min + 1);
		buf.trimToSize();
		
		return val;
	}
	
	private String getNextStringValue(StringBuffer buf)
	{
		String val = "";
		while(val == null || val.compareTo("") == 0 || val.compareTo("\n") == 0)
		{
			if(buf.length() == 0)
				return val;
			int min = 0;
			
			int tab = buf.indexOf("\t");
			int space = buf.indexOf(" ");
			int newline = buf.indexOf("\n");
			
			if(space == -1)
			{
				if(tab == -1)
					min = newline;
				else if(newline == -1)
					min = tab;
				else
					min = Math.min(newline, tab);
			}	
			else if(tab == -1)
			{
				if(space == -1)
					min = newline;
				else if(newline == -1)
					min = space;
				else
					min = Math.min(newline, space);
				
			}
			else if(newline == -1)
			{
				if(space == -1)
					min = tab;
				else if(tab == -1)
					min = space;
				else
					min = Math.min(tab, space);
			}	
			else
			{
				min = Math.min(space, Math.min(tab, newline)); 
			}
			
			if(min == -1)
				min = buf.length();
			val = buf.substring(0, min);
			buf.delete(0, min + 1);
			buf.trimToSize();
			val = val.trim();
		}
	
		return val;
	}
	
	public void createControl(Composite parent) 
	{
		Composite top = createDefaultComposite(parent, 1, false);
		setControl(top);

		Group group1 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group1.setText("Select Platform");
		group1.setLayout(new GridLayout());
		group1.setLayoutData(createNewGD(0));
		Composite platformSection = createDefaultComposite(group1,2, true);
		
		createPlatformsSection(platformSection);
		
		Group group2 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group2.setText("Resources and Connections");
		group2.setLayout(new GridLayout());
		group2.setLayoutData(createNewGD(0));
		Composite resourceConnectionSection = createDefaultComposite(group2, 3, false);
		
		createConnectionSection(resourceConnectionSection);
		
		loadPlatforms();
	}
	
	private void createPlatformsSection(Composite parent)
	{	
		platforms = new Table(parent, SWT.BORDER | SWT.FILL);
		platforms.setHeaderVisible(true);
		platforms.setLinesVisible(true);
		platforms.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
		
		TableColumn platformNames = new TableColumn(platforms, SWT.FILL);
		
		platformNames.setWidth(400);
		platformNames.setMoveable(false);
		platformNames.setText("Platforms");

		imageLoc = new Button(parent, SWT.PUSH | SWT.FLAT);
		imageLoc.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
		
		String imageFolder = Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/Platforms/";
		String imagePath = new String(imageFolder + "None" + ".png");
		boardImage = new Image(null, imagePath);
		imageLoc.setImage(boardImage);
		
		imageLoc.addSelectionListener(new SelectionListener()
		{

			public void widgetDefaultSelected(SelectionEvent e) 
			{
				widgetSelected(e);	
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleEditor();
			}
			
		});
		
		
		platforms.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
					widgetSelected(e);
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleEditor();
				int selection = platforms.getSelectionIndex();
				resources.removeAll();
				PlatformData data = platformData.get(selection);
				for(int i = 0; i < data.numberOfCategorys(); ++i)
				{
					TableItem item = new TableItem(resources, SWT.NONE);
					
					int total = data.getCategory(i).getAmountInCategory();;
					String name = data.getCategory(i).name;
					
					item.setText(new String[]{name, Integer.toString(total)});
				}
				
				try
				{
					if(boardImage != null)
						boardImage.dispose();
					String imageFolder = Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/Platforms/";
					String imagePath = new String(imageFolder + platforms.getItem(platforms.getSelectionIndex()).getText().toString() + ".png");
					boardImage = new Image(null, imagePath);
					imageLoc.setImage(boardImage);
				}
				catch(Exception ex)
				{
					ex.printStackTrace();
				}
				
				connections.removeAll();
				for(int i = 0; i < savedConnections.get(selection).size(); ++i)
				{
					TableItem item = new TableItem(connections, SWT.NONE);
					item.setText(new String[]{savedConnections.get(selection).get(i).data[0], savedConnections.get(selection).get(i).data[1]});
				}
				
			}
		});
		
	}
	
	private void createConnectionSection(Composite parent)
	{
		resources = new Table(parent, SWT.BORDER | SWT.FILL);
		resources.setHeaderVisible(true);
		resources.setLinesVisible(true);
		resources.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
		
		TableColumn resourceNames = new TableColumn(resources, SWT.FILL);
		
		resourceNames.setWidth(175);
		resourceNames.setMoveable(false);
		resourceNames.setText("Resources");

		TableColumn amount = new TableColumn(resources, SWT.FILL);
		
		amount.setWidth(175);
		amount.setMoveable(false);
		amount.setText("Amount Availble");
		
		resources.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				handleEditor();
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleEditor();
			}
		});
		
		createTableButtons(parent);
		
		connections = new Table(parent, SWT.BORDER | SWT.FILL);
		connections.setHeaderVisible(true);
		connections.setLinesVisible(true);
		connections.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
		
		TableColumn resourcesSelected = new TableColumn(connections, SWT.FILL);
		
		resourcesSelected.setWidth(175);
		resourcesSelected.setMoveable(false);
		resourcesSelected.setText("Resource Selected");
		
		TableColumn stream = new TableColumn(connections, SWT.FILL);
		
		stream.setWidth(175);
		stream.setMoveable(false);
		stream.setText("Stream Connected");
		
		editor = new TableEditor(connections);
		editor.horizontalAlignment = SWT.LEFT;
		editor.grabHorizontal = true;
		editor.minimumWidth = 50;
		
		connections.addMouseListener(new MouseListener()
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
					//v.get(selectedIndex).getArgs()[selectedRow * 2 + 1] = new String(text.getText());
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
					editColumn = 0;
					// Clean up any previous editor control
					Control oldEditor = editor.getEditor();
					if (oldEditor != null) 
						oldEditor.dispose();
					
					//Get the item we clicked.
					final TableItem item = connections.getItem(new Point(event.x, event.y));
					
					if (item == null)
					{
						return;
					}	
					
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
					
					//Get the selection of which platform and resource we are editing.
					resourceName = connections.getItem(connections.getSelectionIndex()).getText(editColumn).toString();
					platformIndex = platforms.getSelectionIndex();
									
					// The control that will be the editor must be a child of the Table
					final Text newEditor = new Text(connections, SWT.NONE);
					
					selectedRow = connections.getSelectionIndex();
					
					if(editColumn == 0)
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
						
						Combo combo = new Combo (connections, SWT.READ_ONLY | SWT.FILL);
				        
						//We need to find which category we are removing from.
						
						PlatformData data = platformData.get(platformIndex);
						String category = data.getCategory(resourceName);
						CategoryData cData = data.getCategoryData(category);
						Vector<ResourceData> rData = cData.rData;
						
						combo.add(connections.getItem(selectedRow).getText(0));
						for(int i = 0; i < rData.size(); ++i) 
						{
							if(rData.get(i).num > 0 && connections.getItem(selectedRow).getText(0).compareTo(rData.get(i).name) != 0)
							{
								combo.add(rData.get(i).name);
							}
						}
				        
						combo.select(0);
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
							{
								handleEditor();
							}
						}
					});
					editor.setEditor(newEditor, item, editColumn);
					
				}
			}
			public void mouseUp(MouseEvent event)
			{
			}			
		});
		
	}
	
	private void handleEditor()
	{
		try
		{
			if(savedConnections == null)
				return;
			if(editor == null)
				return;
			if(editor.getEditor() == null)
				return;
			if(selectedCategory == -1 || platformIndex == -1)
				return;
			if(savedConnections.size() == 0)
			{
				editor.getEditor().dispose() ;
				return ;
			}
			if(editor.getEditor().isDisposed())
				return;
			if(savedConnections.get(platformIndex) == null)
				return;
			String[] newStrings;
			newStrings = savedConnections.get(platformIndex).get(selectedRow).data;
			if(newStrings == null)
				return;
			if(editor.getItem() == null || editor.getItem().getText(1) == null)
				return;
			
			if(editColumn == 0)
			{
				if(newStrings[0] != editor.getItem().getText(0))
				{	
					ResourceData r1 = platformData.get(platformIndex).getResource(editor.getItem().getText(0));
					r1.setAmount(r1.getAmount() - 1);
					
					ResourceData r2 = platformData.get(platformIndex).getResource(newStrings[0]);
					r2.setAmount(r2.getAmount() + 1);	
				}
			}
			
			newStrings[1] = new String(editor.getItem().getText(1));
			newStrings[0] = new String(editor.getItem().getText(0));
			savedConnections.get(platformIndex).removeElementAt(selectedRow);
			String platform = platformData.get(platformIndex).name;
			String category = platformData.get(platformIndex).getCategory(newStrings[0]);
			savedConnections.get(platformIndex).add(selectedRow, new ConnectionData(platform, category, newStrings));
			editor.getEditor().dispose();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void createTableButtons(Composite parent)
	{
		Composite buttons = createDefaultComposite(parent, 1, false);
		
		Label l = new Label(buttons, 0);
		l.setText("");
		Label l2 = new Label(buttons, 0);
		l2.setText("");
		Button add = new Button(buttons, SWT.PUSH);
		add.setText("Add >");
		add.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
		
		add.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				widgetSelected(e);
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleEditor();
				try
				{
					int platSelection = platforms.getSelectionIndex();
					if(platSelection == -1 || platSelection == 0)
						return;
					int resourceSelection = resources.getSelectionIndex();
					if(resourceSelection != -1)
					{	
						ResourceData resource = platformData.get(platSelection).getCategory(resourceSelection).getFirstAvailableResource();
						
						if(resource == null)
							return;
						
						--resource.num;
						TableItem item = new TableItem(connections, SWT.NONE);
						item.setText(new String[]{resource.name, "/*Stream*/"});
						int count = Integer.parseInt(resources.getItem(resources.getSelectionIndex()).getText(1));
						resources.getItem(resources.getSelectionIndex()).setText(1, Integer.toString(count - 1));
						
						String platform = platformData.get(platSelection).name;
						String category = platformData.get(platSelection).getCategory(resource.name);
						
						savedConnections.get(platSelection).add(new ConnectionData(platform, category, new String[]{ resource.name, "/*Stream*/"}));
					}
				}
				catch(Exception ex)
				{
					ex.printStackTrace();
				}
			}
			
		});
		
		Label l3 = new Label(buttons, 0);
		l3.setText("");
		Button remove = new Button(buttons, SWT.PUSH);
		remove.setText("< Remove");
		remove.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
		
		remove.addSelectionListener(new SelectionListener()
		{
			
			public void widgetDefaultSelected(SelectionEvent e) 
			{
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleEditor();
				try
				{
					int platSelection = platforms.getSelectionIndex();
					if(platSelection == -1 || platSelection == 0)
						return;
					int connectionSelection = connections.getSelectionIndex();
					if(connectionSelection != -1)
					{
						String connectedResource = connections.getItem(connectionSelection).getText(0);
						CategoryData cData = platformData.get(platSelection).getCategoryData(platformData.get(platSelection).getCategory(connectedResource));
						
						ResourceData resource = cData.getResource(connectedResource);
						
						resource.setAmount(resource.getAmount() + 1);
						
						int categoryIndex = platformData.get(platSelection).getCategoryIndex(cData.name);
						
						resources.getItem(categoryIndex).setText(1, Integer.toString(cData.getAmountInCategory()));
						
						savedConnections.get(platSelection).remove(connections.getSelectionIndex());
						
						int[] items = connections.getSelectionIndices();
						connections.remove(items);
						if( items != null )
							connections.select(items[0]);
						else if(connections.getItemCount() > 0)
						{
							connections.select(connections.getItemCount() - 1);
						}
					}
				}
				catch(Exception ex)
				{
					ex.printStackTrace();
				}
			}
			
		});
		
	}
	
	private GridData createNewGD(int a)
	{
		GridData gd = new GridData(a);
		gd.verticalAlignment = SWT.FILL;
		gd.horizontalAlignment = SWT.FILL;
		gd.grabExcessHorizontalSpace = true;
		return gd;
	}
	
	private Composite createDefaultComposite(Composite parent, int numCols, boolean equal) 
	{
		Composite composite = new Composite(parent, SWT.NONE);
		GridLayout layout = new GridLayout();
		layout.numColumns = numCols;
		layout.marginHeight = 2;
		layout.makeColumnsEqualWidth = equal;
		composite.setLayout(layout);
		composite.setLayoutData(createNewGD(0));
		return composite;
	}

}
