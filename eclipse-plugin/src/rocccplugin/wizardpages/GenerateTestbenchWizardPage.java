package rocccplugin.wizardpages;

import java.io.File;
import java.util.Map;
import java.util.TreeMap;
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
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
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
import rocccplugin.database.DatabaseInterface;
import rocccplugin.helpers.FileSelector;
import rocccplugin.utilities.CompositeUtilities;
import rocccplugin.utilities.FileUtils;
import rocccplugin.utilities.StringUtils;



public class GenerateTestbenchWizardPage extends WizardPage 
{
	Table scalarsTable;
	Table expectedValuesTable;
	public String[] scalarNames;
	public Vector<Vector<String>> scalarValues;
	Vector<TableColumn> scalarColumns = new Vector<TableColumn>();
	Vector<TableColumn> outputColumns = new Vector<TableColumn>();
	public Map<String, Text> clks = new TreeMap<String, Text>();
	public Map<String, String> loadedClks = new TreeMap<String, String>();
	public Text memoryBox;
	public FileSelector[] inputTestFiles;
	public FileSelector[] outputTestFiles;
	
	Map<String, String> streamFiles = new TreeMap<String, String>();
	
	String maxStreamMemory = "";
	String componentName;
	TableEditor editor;
	boolean editorOnInputs = true;
	int selectionIndex = 0;
	int selectedRow = 0;
	int startingSets = 1;
	boolean isSystem;
	
	File sourceFile;
	public String[] outputNames;
	public Vector<Vector<String>> outputValues;
	
	int horizontalOffset = 0;
	
	private void onScrollHorizontal(Table t)
	{
		int newSelection = t.getHorizontalBar ().getSelection ();
		Rectangle clientArea = t.getClientArea ();
		t.update ();
		GC gc = new GC (t);
		gc.copyArea (
			0, 0,
			clientArea.width, clientArea.height,
			horizontalOffset - newSelection, 0);
		gc.dispose ();
		if (t.getHeaderVisible()) 
		{
			t.update();
			//header.update ();
			t.update();
			clientArea = t.getClientArea();
			//gc = new GC (header);
			//gc.copyArea (
			//	0, 0,
			//	clientArea.width, clientArea.height,
			//	horizontalOffset - newSelection, 0);
			//gc.dispose ();
		}
		horizontalOffset = newSelection;
	}
	
	public GenerateTestbenchWizardPage(String pageName, String componentName, File sourceFile, File testbenchInfoFile, boolean isSystem) 
	{
		super(pageName);
	
		this.componentName = componentName;
		this.isSystem = isSystem;
		this.sourceFile = sourceFile;
		
		setTitle(pageName);
		setDescription("Fill out the values to use for this testbench.");
		getInputScalars();
		getOutputScalars();
		loadSavedValues(testbenchInfoFile);
	}
	
	private void getInputScalars()
	{
		try
		{
			scalarNames = DatabaseInterface.getInputPorts(componentName);
			scalarValues = new Vector<Vector<String>>(scalarNames.length);
			for(int i = 0; i < scalarNames.length; ++i)
				scalarValues.add(new Vector<String>());
			for(int i = 0; i < scalarValues.size(); ++i)
				scalarValues.get(i).add("0");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void getOutputScalars()
	{
		try
		{
			outputNames = DatabaseInterface.getOutputPorts(componentName);
			outputValues = new Vector<Vector<String>>(outputNames.length);
			for(int i = 0; i < outputNames.length; ++i)
				outputValues.add(new Vector<String>());
			for(int i = 0; i < outputValues.size(); ++i)
				outputValues.get(i).add("0");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
 	}

	public void createControl(Composite parent) 
	{
		try
		{
			Composite top = CompositeUtilities.createDefaultComposite(parent,1, false);
			setControl(top);
	
			//Columns Control
			Composite columnControllerComp = CompositeUtilities.createDefaultComposite(top, 4, false, SWT.LEFT);
			
			
			if(!isSystem)
			{
				new Label(columnControllerComp, SWT.NONE).setText("Number of test sets: ");
				
				
				Button less = new Button(columnControllerComp, SWT.PUSH);
				less.setText("-");
				
				final Label dataSets = new Label(columnControllerComp, SWT.NONE);//Activator.createTextField(columnControllerComp, 75);
				dataSets.setText(Integer.toString(startingSets));
				
				Button more = new Button(columnControllerComp, SWT.PUSH);
				more.setText("+");
				GridData gd = CompositeUtilities.createNewGD(SWT.BEGINNING, false, false, SWT.CENTER);
				gd.horizontalAlignment = SWT.BEGINNING;
				gd.grabExcessHorizontalSpace = false;
				more.setLayoutData(gd);
				
				less.addSelectionListener(new SelectionListener()
				{
					public void widgetDefaultSelected(SelectionEvent e) 
					{
							
					}
		
					public void widgetSelected(SelectionEvent e) 
					{
						handleEditor();
						
						if(Integer.parseInt(dataSets.getText()) > 1)
						{
							dataSets.setText(new String(Integer.toString(Integer.parseInt(dataSets.getText()) - 1)));
							scalarColumns.get(scalarColumns.size() - 1).dispose();
							scalarColumns.remove(scalarColumns.size() - 1);
							
							outputColumns.get(outputColumns.size() - 1).dispose();
							outputColumns.remove(outputColumns.size() - 1);
							
							for(int i = 0; i < scalarValues.size(); ++i)
								scalarValues.get(i).remove(scalarValues.get(i).size() - 1);
							
							for(int i = 0; i < outputValues.size(); ++i)
								outputValues.get(i).remove(outputValues.get(i).size() - 1);
						}
					}
				});
				
				more.addSelectionListener(new SelectionListener()
				{
					public void widgetDefaultSelected(SelectionEvent e) 
					{
							
					}
		
					public void widgetSelected(SelectionEvent e) 
					{
						handleEditor();
						
						if(Integer.parseInt(dataSets.getText()) < 128)
						{
							dataSets.setText(new String(Integer.toString(Integer.parseInt(dataSets.getText()) + 1)));
							TableColumn tc = new TableColumn(scalarsTable, SWT.NONE);
							tc.setText("Input Set " + (scalarColumns.size() + 1));
							tc.setWidth(150);
							scalarColumns.add(tc);
							
							
							TableColumn tc2 = new TableColumn(expectedValuesTable, SWT.NONE);
							tc2.setText("Output Set " + (outputColumns.size() + 1));
							tc2.setWidth(150);
							outputColumns.add(tc2);
							
							for(int i = 0; i < scalarValues.size(); ++i)
							{
								scalarValues.get(i).add(scalarValues.get(i).get(scalarValues.get(i).size() - 1));
								for(int j = 0; j < scalarsTable.getItemCount(); ++j)
									scalarsTable.getItem(j).setText(scalarsTable.getColumnCount() - 1, scalarsTable.getItem(j).getText(scalarsTable.getColumnCount() - 2));
							}
							for(int i = 0; i < outputValues.size(); ++i)
							{
								outputValues.get(i).add(outputValues.get(i).get(outputValues.get(i).size() - 1));
								for(int j = 0; j < expectedValuesTable.getItemCount(); ++j)
									expectedValuesTable.getItem(j).setText(expectedValuesTable.getColumnCount() - 1, expectedValuesTable.getItem(j).getText(expectedValuesTable.getColumnCount() - 2));
							}
						}
					}
				});
			}
			
			if(scalarValues.size() > 0)
			{
				Group group1 = new Group(top, SWT.SHADOW_ETCHED_IN);
				group1.setText("Input Scalars Test Values");
				group1.setLayout(new GridLayout());
				group1.setLayoutData(CompositeUtilities.createNewGD(0));
				
				Composite groupComp = CompositeUtilities.createDefaultComposite(group1, 1, false);
				
				createTable(groupComp);
				
				
			}

			if(isSystem)
			{
				createMemoryValues(top);
			}
			if(!isSystem || DatabaseInterface.getOutputPorts(componentName).length > 0)
			{
				Group group2 = new Group(top, SWT.SHADOW_ETCHED_IN | SWT.V_SCROLL);
				group2.setLayout(new GridLayout());
				group2.setLayoutData(CompositeUtilities.createNewGD(0));
				
				group2.setText("Expected Output Values");
				Composite group2Comp = CompositeUtilities.createDefaultComposite(group2, 1, false);
				createExpectedValuesTable(group2Comp); 
			}
			
			
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void loadSavedValues(File testbenchInfoFile)
	{
		loadedClks.clear();
		try
		{	
			if(!testbenchInfoFile.exists() || !testbenchInfoFile.canRead())
				return;
				
			StringBuffer buffer = new StringBuffer();
			FileUtils.addFileContentsToBuffer(buffer, testbenchInfoFile.getAbsolutePath());
			
			Map<String, String[]> savedValues = new TreeMap<String, String[]>();
			
			startingSets = 1;
			
			loadedClks.clear();
			
			while(buffer.length() > 0)
			{
				String valueType;
				
				while(buffer.length() > 0)
				{				
					valueType = StringUtils.getNextStringValue(buffer);
					
					if(valueType.equals("STREAM_DATA"))
					{
						String streamName = StringUtils.getNextStringValue(buffer);
						String fileName = StringUtils.getNextLine(buffer);
						streamFiles.put(streamName, fileName);
					}
					else if(valueType.equals("SCALAR_DATA"))
					{
						String scalarName = StringUtils.getNextStringValue(buffer);
						String numValues =  StringUtils.getNextStringValue(buffer);
						
						if(!StringUtils.isPositiveInt(numValues))
							return;
			
						startingSets = Math.max(startingSets, Integer.parseInt(numValues));
						
						String[] values = new String[Integer.parseInt(numValues)];
						for(int i = 0; i < Integer.parseInt(numValues); ++i)
						{
							values[i] = StringUtils.getNextStringValue(buffer);
						}
						
						savedValues.put(scalarName, values);
					}
					else if(valueType.equals("CLOCK"))
					{
						String clockName = StringUtils.getNextStringValue(buffer);
						String clkValue = StringUtils.getNextStringValue(buffer);
						
						if(!StringUtils.isPositiveInt(clkValue))
							return;
						
						loadedClks.put(clockName, clkValue);
					}
				}
				
				
				
				/*int numInputStreams, numOutputStreams = 0;
				String num = StringUtils.getNextStringValue(buffer);
				if(!StringUtils.isAnInt(num))
					return;
				numInputStreams = Integer.parseInt(num);
				
				for(int i = 0; i < numInputStreams; ++i)
				{
					String stream = StringUtils.getNextStringValue(buffer);
					//String clkValue = StringUtils.getNextStringValue(buffer);
					String testFile = StringUtils.getNextLine(buffer);
					
					streamFiles.put(stream, testFile);
					//loadedClks.put(stream, clkValue);
				}
				
				num = StringUtils.getNextStringValue(buffer);
				if(!StringUtils.isAnInt(num))
					return;
				numOutputStreams = Integer.parseInt(num);
				for(int i = 0; i < numOutputStreams; ++i)
				{
					String stream = StringUtils.getNextStringValue(buffer);
					//String clkValue = StringUtils.getNextStringValue(buffer);
					String testFile = StringUtils.getNextLine(buffer);
					
					streamFiles.put(stream, testFile);
					//loadedClks.put(stream, clkValue);
				}
			//}	
			
			String sets = StringUtils.getNextStringValue(buffer);
			startingSets = 1;
			if(StringUtils.isPositiveInt(sets))
				startingSets = Integer.parseInt(sets);
			else
				buffer.insert(0, sets + " ");
			
			while(buffer.length() > 0)
			{
				String[] values = new String[startingSets + 1];
				values[0] = StringUtils.getNextStringValue(buffer);
				for(int i = 0; i < startingSets; ++i)
				{
					values[i + 1] = StringUtils.getNextStringValue(buffer);
				}
				savedValues.add(values);
			}*/
			}
			
			//Set the input scalar values to the values loaded.
			for(int i = 0; i < scalarNames.length; ++i)
			{
				if(savedValues.containsKey(scalarNames[i]))
				{
					String[] values = savedValues.get(scalarNames[i]);
					scalarValues.get(i).clear();
					for(int k = 0; k < values.length; ++k)
					{
						scalarValues.get(i).add(values[k]);
					}
				}
			}
			
			//Set the output scalar values to the values loaded.
			for(int i = 0; i < outputNames.length; ++i)
			{
				if(savedValues.containsKey(outputNames[i]))
				{
					String[] values = savedValues.get(outputNames[i]);
					outputValues.get(i).clear();
					for(int k = 0; k < values.length; ++k)
					{
						outputValues.get(i).add(values[k]);
					}
				}
			}
			
			for(int i = 0; i < outputNames.length; ++i)
			{
				while(outputValues.get(i).size() < startingSets)
					outputValues.get(i).add("0");
			}
			
			for(int i = 0; i < scalarNames.length; ++i)
			{
				while(scalarValues.get(i).size() < startingSets)
					scalarValues.get(i).add("0");
			}
			
			
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
	}
	
	private void createTable(Composite parent)
	{
		try
		{
			final Composite tableComp = CompositeUtilities.createDefaultComposite(parent, 1, false);
			
			scalarsTable = new Table(tableComp, SWT.FULL_SELECTION | SWT.BORDER | SWT.H_SCROLL);
			scalarsTable.setHeaderVisible(true);
			scalarsTable.setLinesVisible(true);
			
			TableColumn scalarsColumn = new TableColumn(scalarsTable, SWT.NONE);
			scalarsColumn.setWidth(150);
			scalarsColumn.setText("Input Scalars");
			
			if(!isSystem)
			{
				for(int i = 0; i < startingSets; ++i)
				{
					TableColumn valuesColumn = new TableColumn(scalarsTable, SWT.LEFT);
					valuesColumn.setWidth(150);
					valuesColumn.setText("Input Set " + (i + 1));
					
					scalarColumns.add(valuesColumn);	
				}
			}
			else
			{
				TableColumn valuesColumn = new TableColumn(scalarsTable, SWT.LEFT);
				valuesColumn.setWidth(200);
				valuesColumn.setText("Initial Input Scalar Values");
				
				scalarColumns.add(valuesColumn);	
			}
			
			for(int j = 0; j < scalarValues.size(); ++j)
			{
				TableItem item = new TableItem(scalarsTable, SWT.LEFT);
			
				String[] tableValues = new String[scalarValues.get(j).size() + 1];
				tableValues[0] = scalarNames[j];
				for(int k = 0; k < scalarValues.get(j).size(); ++k)
					tableValues[k + 1] = scalarValues.get(j).get(k);
				
				item.setText(tableValues);
			}	
			
			editor = new TableEditor(scalarsTable);
			editor.horizontalAlignment = SWT.LEFT;
			editor.grabHorizontal = true;
			editor.minimumWidth = 50;
			
			scalarsTable.addMouseListener(new MouseListener()
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
							scalarValues.get(selectionIndex).set(editor.getColumn() - 1, new String(text.getText()));
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
							final TableItem item = scalarsTable.getItem(new Point(event.x, event.y));
							
							selectionIndex = scalarsTable.getSelectionIndex();
							if (item == null)
							{
								return;
							}					
							// The control that will be the editor must be a child of the Table
							final Text newEditor = new Text(scalarsTable, SWT.NONE);
							
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
							
							if(editColumn == 0)
								return;
							
							editorOnInputs = true;
							selectedRow = scalarsTable.getSelectionIndex();
							
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
				
				public void mouseUp(MouseEvent e) {
					// TODO Auto-generated method stub
					
				}
			});
			
			/*scalarsTable.addKeyListener(new KeyListener()
			{
				public void keyPressed(KeyEvent e) 
				{
					if(e.character == 'a')
					{
						TableColumn c = new TableColumn(scalarsTable, SWT.NONE);
						c.setWidth(150);
						c.setText("Scalar Values");
					}
				}
	
				public void keyReleased(KeyEvent e) 
				{
				}
			});*/
			
			CompositeUtilities.setCompositeSize(tableComp, scalarsTable, 550, scalarsTable.getItemHeight() * 5, 600, scalarsTable.getItemHeight() * 10);
	
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void createMemoryValues(Composite parent)
	{	
		String[] inputStreams = DatabaseInterface.getInputStreams(componentName);
		
		if(inputStreams.length > 0)
		{
			Group group2 = new Group(parent, SWT.SHADOW_ETCHED_IN);
			group2.setLayout(new GridLayout());
			group2.setLayoutData(CompositeUtilities.createNewGD(0));
			group2.setText("Input stream test info");	
			
			final Composite sc1 = new Composite(group2, SWT.LEFT | SWT.V_SCROLL);
			GridLayout layout = new GridLayout();
			layout.numColumns = 1;
			layout.marginHeight = 2;
			layout.makeColumnsEqualWidth = false;
			sc1.setLayout(layout);
			
			final Composite memoryComp = CompositeUtilities.createDefaultComposite(sc1, 3, false);
			inputTestFiles = new FileSelector[inputStreams.length];
			
			for(int i = 0; i < inputStreams.length; ++i)
			{
				String file = "/*Stream " + inputStreams[i] + " Test File*/";
				if(streamFiles.containsKey(inputStreams[i]))
					file = streamFiles.get(inputStreams[i]);
				inputTestFiles[i] = new FileSelector("Stream " + inputStreams[i] + " Test File:", file, FileUtils.getFolderOfFile(sourceFile) , memoryComp);
			}
			
			sc1.getVerticalBar().setMaximum(100);
			sc1.getVerticalBar().setMinimum(0);
			
			sc1.getVerticalBar().addSelectionListener(new SelectionListener()
			{
				public void widgetDefaultSelected(SelectionEvent e) 
				{
					
				}
				
				public void widgetSelected(SelectionEvent e) 
				{
					memoryComp.setLocation(memoryComp.getLocation().x, (int) ((sc1.getVerticalBar().getMaximum() - sc1.getVerticalBar().getSelection()) / (float)(sc1.getVerticalBar().getMaximum()) * memoryComp.getSize().y) - memoryComp.getSize().y);
				}
			});
			sc1.getVerticalBar().setVisible(false);
			if(inputStreams.length > 5)
			{
				CompositeUtilities.setCompositeSize(group2, sc1, 175, 200 );
				sc1.getVerticalBar().setVisible(true);
			}
		}
		
		String[] outputStreams = DatabaseInterface.getOutputStreams(componentName);
		if(outputStreams.length > 0)
		{
			Group group3 = new Group(parent, SWT.SHADOW_ETCHED_IN);
			group3.setLayout(new GridLayout());
			group3.setLayoutData(CompositeUtilities.createNewGD(0));
			group3.setText("Output stream test Info");	
			
			final Composite sc2 = new Composite(group3, SWT.LEFT | SWT.V_SCROLL);
			GridLayout layout = new GridLayout();
			layout.numColumns = 1;
			layout.marginHeight = 2;
			layout.makeColumnsEqualWidth = false;
			sc2.setLayout(layout);
			
			final Composite memoryComp2 = CompositeUtilities.createDefaultComposite(sc2, 3, false);
			
			outputTestFiles = new FileSelector[outputStreams.length];
			
			for(int i = 0; i < outputStreams.length; ++i)
			{
				String file = "/*Stream " + outputStreams[i] + " Test File*/";
				if(streamFiles.containsKey(outputStreams[i]))
					file = streamFiles.get(outputStreams[i]);
				outputTestFiles[i] = new FileSelector("Stream " + outputStreams[i] + " Test File:", file, FileUtils.getFolderOfFile(sourceFile), memoryComp2);
			}
			
			sc2.getVerticalBar().setMaximum(100);
			sc2.getVerticalBar().setMinimum(0);
			
			sc2.getVerticalBar().addSelectionListener(new SelectionListener()
			{
				public void widgetDefaultSelected(SelectionEvent e) 
				{
					
				}
				
				public void widgetSelected(SelectionEvent e) 
				{
					memoryComp2.setLocation(memoryComp2.getLocation().x, (int) ((sc2.getVerticalBar().getMaximum() - sc2.getVerticalBar().getSelection()) / (float)(sc2.getVerticalBar().getMaximum()) * memoryComp2.getSize().y) - memoryComp2.getSize().y);
				}
			});
			sc2.getVerticalBar().setVisible(false);
			if(outputStreams.length > 5)
			{
				CompositeUtilities.setCompositeSize(group3, sc2, 175, 200 );
				sc2.getVerticalBar().setVisible(true);
			}
		}
		
		clks.clear();
		
		Group group4 = new Group(parent, SWT.SHADOW_ETCHED_IN);
		group4.setLayout(new GridLayout());
		group4.setLayoutData(CompositeUtilities.createNewGD(0));
		group4.setText("Clocks");
		
		final Composite clkCompositeParent = new Composite(group4, SWT.LEFT | SWT.V_SCROLL);
		GridLayout layout = new GridLayout();
		layout.numColumns = 1;
		layout.marginHeight = 2;
		layout.makeColumnsEqualWidth = false;
		clkCompositeParent.setLayout(layout);
		
		final Composite clkComposite = CompositeUtilities.createDefaultComposite(clkCompositeParent, 8, false);
		
		new Label(clkComposite, SWT.NONE).setText("Clk");
		Text clkText = new Text(clkComposite, SWT.BORDER);
		clkText.setText("1");
		clkText.setSize(50, (int) (clkText.getLineHeight() * 0.85));
        Rectangle trim = clkText.computeTrim(0, 0, 50, (int) (clkText.getLineHeight() * 0.85));
		GridData d = CompositeUtilities.createNewGD(0, true, false, SWT.CENTER);
		d.heightHint = trim.height;
		d.widthHint = trim.width;
		clkText.setLayoutData(d);		
		new Label(clkComposite, SWT.NONE).setText("ns");
		new Label(clkComposite, SWT.NONE).setText("");
		
		clks.put("clk", clkText);
		if(loadedClks.containsKey("clk"))
			clks.get("clk").setText(loadedClks.get("clk"));
		
		String[] clockNames = DatabaseInterface.getSystemClocks(componentName);
		
		for(int i = 0; i < clockNames.length; ++i)
		{
			new Label(clkComposite, SWT.NONE).setText(clockNames[i] + " Clk");
			Text clk = new Text(clkComposite, SWT.BORDER);
			clk.setText("1");
			clk.setSize(50, (int) (clk.getLineHeight() * 0.85));
	        Rectangle clkTrim = clk.computeTrim(0, 0, 50, (int) (clk.getLineHeight() * 0.85));
			GridData d2 = CompositeUtilities.createNewGD(0, true, false, SWT.CENTER);
			d2.heightHint = trim.height;
			d2.widthHint = trim.width;
			clk.setLayoutData(d2);
					
			new Label(clkComposite, SWT.NONE).setText("ns");
			new Label(clkComposite, SWT.NONE).setText("");
			
			clks.put(clockNames[i], clk);
			if(loadedClks.containsKey(clockNames[i]))
				clks.get(clockNames[i]).setText(loadedClks.get(clockNames[i]));
		}
		
		clkCompositeParent.getVerticalBar().setMaximum(100);
		clkCompositeParent.getVerticalBar().setMinimum(0);
		
		clkCompositeParent.getVerticalBar().addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
			}
			
			public void widgetSelected(SelectionEvent e) 
			{
				clkComposite.setLocation(clkComposite.getLocation().x, (int) ((clkCompositeParent.getVerticalBar().getMaximum() - clkCompositeParent.getVerticalBar().getSelection()) / (float)(clkCompositeParent.getVerticalBar().getMaximum()) * clkComposite.getSize().y) - clkComposite.getSize().y);
			}
		});
		
		clkCompositeParent.getVerticalBar().setVisible(false);
		if(clockNames.length > 8)
		{
			CompositeUtilities.setCompositeSize(group4, clkCompositeParent, 150, 175 );
			clkCompositeParent.getVerticalBar().setVisible(true);
			clkCompositeParent.getVerticalBar().setSelection(0);
		}
	}
	

	private void createExpectedValuesTable(Composite parent)
	{
		try
		{
			Composite expectedValuesComp = CompositeUtilities.createDefaultComposite(parent, 1, false);
			
			expectedValuesTable = new Table(expectedValuesComp, SWT.FULL_SELECTION | SWT.BORDER  | SWT.H_SCROLL);
			expectedValuesTable.setHeaderVisible(true);
			expectedValuesTable.setLinesVisible(true);
				
			TableColumn name = new TableColumn(expectedValuesTable, SWT.NONE);
			name.setWidth(150);
			name.setText("Output Scalars");
			name.setMoveable(false);
			
			if(!isSystem)
			{
				for(int i = 0; i < startingSets; ++i)
				{
					TableColumn valuesColumn = new TableColumn(expectedValuesTable, SWT.NONE);
					valuesColumn.setWidth(150);
					valuesColumn.setText("Output Set " + (i + 1));
					
					outputColumns.add(valuesColumn);	
				}
			}
			else
			{
				TableColumn valuesColumn = new TableColumn(expectedValuesTable, SWT.NONE);
				valuesColumn.setWidth(200);
				valuesColumn.setText("Output Scalar Values");
				
				outputColumns.add(valuesColumn);	
			}
			
			for(int i = 0; i < outputNames.length; ++i)
			{
				String[] tableValues = new String[outputValues.get(i).size() + 1];
				tableValues[0] = outputNames[i];
				for(int j = 0; j < outputValues.get(i).size(); ++j)
					tableValues[j + 1] = outputValues.get(i).get(j);
				new TableItem(expectedValuesTable, SWT.NONE).setText(tableValues);
			}
			
			if(editor == null)
			{
				editor = new TableEditor(expectedValuesTable);
				editor.horizontalAlignment = SWT.LEFT;
				editor.grabHorizontal = true;
				editor.minimumWidth = 50;
			}
			
			expectedValuesTable.addMouseListener(new MouseListener()
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
							outputValues.get(selectionIndex).set(editor.getColumn() - 1, new String(text.getText()));
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
							final TableItem item = expectedValuesTable.getItem(new Point(event.x, event.y));
							
							selectionIndex = expectedValuesTable.getSelectionIndex();
							if (item == null)
							{
								return;
							}					
							// The control that will be the editor must be a child of the Table
							final Text newEditor = new Text(expectedValuesTable, SWT.NONE);
							
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
							
							if(editColumn == 0)
								return;
							
							editorOnInputs = false;
							selectedRow = expectedValuesTable.getSelectionIndex();
							
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
				
				public void mouseUp(MouseEvent e) 
				{
				}
			});
			
			CompositeUtilities.setCompositeSize(expectedValuesComp, expectedValuesTable, 550, expectedValuesTable.getItemHeight() * 5, 600, expectedValuesTable.getItemHeight() * 10);
		}
		catch(Exception e)
		{
			e.printStackTrace();
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
			if(editor.getEditor().isDisposed())
				return;
			
			if(editorOnInputs)
			{
				if(scalarValues == null)
					return;
				if(scalarValues.size() == 0)
				{
					editor.getEditor().dispose() ;
					return ;
				}
				if(scalarValues.size() <= selectionIndex)
					return;
				if(scalarValues.get(selectionIndex).get(editor.getColumn() - 1) == null)
					return;
			}
			else
			{
				if(outputValues == null || outputValues.size() == 0 || 
				   outputValues.size() <= selectionIndex || outputValues.get(selectionIndex).get(editor.getColumn() - 1) == null)
					return;
			}
			
			String newString;
			newString = editorOnInputs? scalarValues.get(selectionIndex).get(editor.getColumn() - 1) : outputValues.get(selectionIndex).get(editor.getColumn() - 1);
			if(newString == null)
				return;
			if(editor.getItem() == null)
				return;
			newString = (editor.getItem().getText(editor.getColumn()));
		
			if(editorOnInputs)
				scalarValues.get(selectionIndex).set(editor.getColumn() - 1, newString);
			else
				outputValues.get(selectionIndex).set(editor.getColumn() - 1, newString);
			editor.getEditor().dispose();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}

}
