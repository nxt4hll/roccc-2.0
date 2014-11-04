package rocccplugin.composites;

import java.io.File;
import java.io.InputStream;
import java.util.Map;
import java.util.TreeMap;
import java.util.Vector;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.window.Window;
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
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;

import rocccplugin.Activator;
import rocccplugin.helpers.FlagData;
import rocccplugin.helpers.ResourceData;
import rocccplugin.utilities.CompositeUtilities;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.FileUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.PreferenceUtils;
import rocccplugin.utilities.StringUtils;
import rocccplugin.wizards.OptimizationArgumentWizard;

/*
 * The Optimization Selector is a general purpose optimization selector for
 * compilation wizards. Just create one of these, tell it what the available
 * flags and arguments are and it will handle the rest for the most part. 
 */

public class OptimizationSelector 
{
	/////////////////////////////////////////////////////////////////////////
	// Variables for the optimization selector.
	/////////////////////////////////////////////////////////////////////////
	
	//Editor for letting them change arguments.
	TableEditor editor;
	
	private File fileToBeCompiled; 
	private File savedOptimizationsFile;
	
	//Vector that holds which selected flags we have and their arguments.
	public Vector<FlagData> selectedFlagsData;
	public Vector<String> constantFlagsData;
	
	Vector<ResourceData> functionsCalled;
	
	//Variables for telling us where we are clicking on the tables.
	private int selectedRow;
	private int selectedIndex;
	
	//Which optimization do we have selected on the available list.
	private String optimization;
	
	//The three tables
	public Table flagList;
	public Table selectedFlags;
	public Table arguments;
	
	//Labels for displaying the description of the optimizations.
	public Label description;
	public Label description2;
	
	Button moreInfoButton;
	
	public Label title;
	Label title2;
	
	Label image;
	
	//Composites that will host the tables.
	Composite flagListButtonsComp;
	Composite flagListComp;
	Composite selectedFlagListComp;
	
	Composite exampleComp;
	
	//Our available flags, arguments, and descriptions.
	Map<String, OptimizationInfo> optimizationData;
	
	OptimizationArgumentWizard argumentsWizard;
	
	//Which flags are allowed to be reloaded
	private Vector<String> acceptedFlags;
	
	//The name of the preference that holds the default flags are for this page.
	private String defaultsPreference;
	
	//Struct that will house the values for an available flag
	
	public enum OptimizationValueType
	{
		SELECTION,
		AMOUNT,
		SELECTION_OR_AMOUNT,
		NONE
	}
	
	public class OptimizationInfo
	{
		public String[] arguments;
		public String[] argumentDefaultValues;
		public String[] description;
		public boolean multipleAllowed;
		public boolean noOtherFlagsAllowed;
		public OptimizationValueType[] optimizationValueTypes;
		public String[][] selectionValues;
		
		public OptimizationInfo(String[] args, String[] argDefaults, String[] desc, OptimizationValueType[] optTypes, String[][] selections, boolean multAllowed, boolean noOthersAllowed)
		{
			arguments = args;
			this.argumentDefaultValues = argDefaults;
			description = desc;
			optimizationValueTypes = optTypes; 
			multipleAllowed = multAllowed;
			noOtherFlagsAllowed = noOthersAllowed;
			selectionValues = selections;
		}
	}
	
	/////////////////////////////////////////////////////
	// Functions for OptimizationSelector
	/////////////////////////////////////////////////////
	
	public OptimizationSelector(Composite parent, File sourceFile, File savedFile, String title, Vector<ResourceData> functionsCalled) 
	{
		try
		{
			fileToBeCompiled = sourceFile;
			savedOptimizationsFile = savedFile;
			
			optimizationData = new TreeMap<String, OptimizationInfo>();
			acceptedFlags = new Vector<String>();
			selectedFlagsData = new Vector<FlagData>();
			constantFlagsData = new Vector<String>();
			
			this.functionsCalled = functionsCalled;
			
			//Create the group that will hold the all the optimization selection tables.
			Group flagsGroup = new Group(parent, SWT.SHADOW_ETCHED_IN);
			flagsGroup.setText(title + " Compile Optimizations");
			flagsGroup.setLayout(new GridLayout());
			flagsGroup.setLayoutData(CompositeUtilities.createNewGD(0));
			
			//Create the composite for the flag tables and buttons
			Composite optimizationsComp = CompositeUtilities.createDefaultComposite(flagsGroup,4, false);
			//Create the composite for the descriptions
			Composite descriptionsComp = CompositeUtilities.createDefaultComposite(flagsGroup, 1, false);
			
			//Create all the controls for the top half of the optimizations selector
			createFlagListControl(optimizationsComp);
			createFlagListButtonsControl(optimizationsComp);
			createSelectedFlagListControl(optimizationsComp);
			createSelectedFlagsButtonsControl(optimizationsComp);
			createDescriptionControl(descriptionsComp);
			
			//Create the group that will hold the arguments table.
			/*Group argumentsGroup = new Group(parent, SWT.SHADOW_ETCHED_IN);
			argumentsGroup.setText("Arguments for Selected " + title + " Optimizations");
			argumentsGroup.setLayout(new GridLayout());
			argumentsGroup.setLayoutData(CompositeUtilities.createNewGD(GridData.FILL_VERTICAL));
			Composite argumentsComp = CompositeUtilities.createDefaultComposite(argumentsGroup, 1, false);
			
			//Create the controls and tables for the setting of the optimization arguments.
			createFlagArgumentsControl(argumentsComp);*/
			
			//Create the defaults buttons on the bottom of the optimization selector out of all the groups.
			createDefaultsButtons(parent);
			
			
			
			//Resize the composites that hold the optimization tables.
			flagListComp.notifyListeners(SWT.Resize, null);
			selectedFlagListComp.notifyListeners(SWT.Resize, null);
			descriptionsComp.notifyListeners(SWT.Resize, null);
			optimizationsComp.notifyListeners(SWT.Resize, null);
			
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public void setDefaultsPreference(String preferenceName)
	{
		//Set the name of the preference that holds the default flags.
		defaultsPreference = preferenceName;
	}
	
	public void addFlags(String optimization, String[] args, String[] defaults, String[] description, OptimizationValueType[] optValueTypes, String[][] selValues, boolean multAllowed, boolean noOthersAllowed)
	{
		try
		{
			//Add the optimization passed in to the accepted flags and store all the passed in data to our
			//optimizationData structure so that when we add a flag to the selected flags list, we know
			//what data and arguments are needed for it.
			acceptedFlags.add(optimization);
			optimizationData.put(optimization, new OptimizationInfo(args, defaults, description, optValueTypes, selValues, multAllowed, noOthersAllowed));
		
			//Add this passed in optimization to the available flags table.
			new TableItem(flagList, SWT.NONE).setText(optimization);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
	}
	
	public void addConstantFlag(String optimization)
	{
		constantFlagsData.add(optimization);
	}
	
	public void flagCannotBeCombinedWithOthers(String flag)
	{
		optimizationData.get(flag).noOtherFlagsAllowed = true;
	}
	
	private void createDefaultsButtons(Composite parent)
	{
		Composite defaultsComp = CompositeUtilities.createDefaultComposite(parent, 3, false);
		
		Button setAsDefaults = new Button(defaultsComp, SWT.PUSH);
		setAsDefaults.setText("Save Values as New Defaults");
		setAsDefaults.addSelectionListener(new SelectionListener()
		{			
			public void widgetDefaultSelected(SelectionEvent e) 
			{
	
			}
			
			public void widgetSelected(SelectionEvent e) 
			{
				if(MessageDialog.openQuestion(null, "Confirm New Defaults", "Are you sure you would like to overwrite the saved default values with the current values?"))
					saveOptimizationsAsDefaults();
			}
			
		});
		
		new Label(defaultsComp, SWT.NONE).setText(" ");
		
		Button defaults = new Button(defaultsComp, SWT.PUSH);
		defaults.setText("Set All To Default Value");
		defaults.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				widgetSelected(e);
			}

			public void widgetSelected(SelectionEvent e) 
			{
				loadDefaultFlags();
			}
		});
	}
	
	private void createFlagListControl(Composite parent)
	{	
		try
		{
			flagListComp = CompositeUtilities.createDefaultComposite(parent, 1, false);
			flagList = new Table(flagListComp, SWT.FULL_SELECTION | SWT.BORDER);
			flagList.setHeaderVisible(true);
			flagList.setLinesVisible(true);
			flagList.setLayoutData(CompositeUtilities.createNewGD(GridData.FILL_BOTH));
	
			TableColumn tableColumn = new TableColumn(flagList, SWT.NONE);
			tableColumn.setWidth(275);
			tableColumn.setMoveable(false);
			tableColumn.setResizable(false);
			tableColumn.setText("Available Optimizations");
				
			flagList.addSelectionListener(new SelectionListener()
			{
				public void widgetDefaultSelected(SelectionEvent e)
				{
					//widgetSelected(e);
				}
	
				public void widgetSelected(SelectionEvent e) 
				{
					handleEditor();
					String flag = flagList.getItem(flagList.getSelectionIndex()).getText().toString();
					//System.out.println("\n" + flag + "\n");
					
					title.setText(flag + ":");
					description.setText("\t" + optimizationData.get(flag).description[0]);
					description2.setText("\t" + optimizationData.get(flag).description[1]);
					try
					{
						image.setImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/optimizationDescriptions/" + flag + ".png")));
						image.setSize(image.computeSize(SWT.DEFAULT, SWT.DEFAULT));
					}
					catch(Exception ex)
					{		
						//ex.printStackTrace();
						image.setImage(null);
					}
					title.setSize(title.computeSize(SWT.DEFAULT, title.getFont().getFontData()[0].getHeight() + 5));
					description.setSize(description.computeSize(SWT.DEFAULT, SWT.DEFAULT));
					description2.setSize(description2.computeSize(SWT.DEFAULT, description2.getFont().getFontData()[0].getHeight()+ 15));
					title2.setSize(title2.computeSize(SWT.DEFAULT, title2.getFont().getFontData()[0].getHeight() + 5));
					//moreInfoButton.setSize(moreInfoButton.computeSize(SWT.DEFAULT, SWT.DEFAULT));
					
					exampleComp.setSize(exampleComp.computeSize(SWT.DEFAULT, SWT.DEFAULT));
				}
			});
			
			flagListComp.addListener(SWT.Resize, new Listener() 
			{
			    int width = -1;
			    public void handleEvent(Event e) 
			    {
				      int newWidth = flagListComp.getSize().x;
				      if (newWidth != width) 
				      {
				    	  flagList.setSize(SWT.DEFAULT, SWT.DEFAULT);
				    	  
				    	  Rectangle trim2 = flagList.computeTrim(0, 0, 0, flagList.getItemHeight() * 5);
				    	  GridData d2 = CompositeUtilities.createNewGD(0);
				    	  d2.heightHint = trim2.height;
				    	  flagList.setLayoutData(d2);
				    	  flagListComp.setSize(flagListComp.getSize().x, flagList.getItemHeight() * 8);
				      }
			    }
			  });
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void createFlagListButtonsControl(Composite parent)
	{
		try
		{
			flagListButtonsComp = CompositeUtilities.createDefaultComposite(parent, 1, false);
			Label l = new Label(flagListButtonsComp, 0);
			l.setText("");
			
			Button add = new Button(flagListButtonsComp, SWT.PUSH);
			add.setText("Add >");
			add.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
			
			add.addSelectionListener(new SelectionListener() 
			{
				public void widgetSelected(SelectionEvent event) 
				{		
					handleEditor();
					if(flagList.getSelectionIndex() == -1)
						return;
					
					String optimizationToBeAdded = flagList.getItem(flagList.getSelectionIndex()).getText().toString();
					
					OptimizationInfo optimizationInfo = optimizationData.get(optimizationToBeAdded);
					
					if(selectedFlags.getItemCount() > 0 && optimizationInfo.multipleAllowed == false)
					{
						for(int i = 0; i < selectedFlagsData.size(); ++i)
						{
							if(selectedFlagsData.get(i).getFlag().equals(optimizationToBeAdded))
							{
								MessageDialog.openInformation(new Shell(), "Optimization Selection Error", "\"" + optimizationToBeAdded + "\" can only be added once.");
								return;
							}
						}
					}
					
					if(optimizationInfo.noOtherFlagsAllowed)
					{
						if(selectedFlagsData.size() > 0)
						{
							if(MessageUtils.openQuestionWindow("Optimization Conflict", "The flag " + optimizationToBeAdded + " cannot be combined with any other flags. Would you like to replace all the selected flags with this one?"))
							{
								try
								{
									selectedFlagsData.removeAllElements();
									selectedFlags.removeAll();
									if(arguments != null)
										arguments.removeAll();
								}
								catch(Exception e)
								{
									e.printStackTrace();
								}
							}
							else
							{
								return;
							}
						}
					}
					
					for(int i = 0; i < selectedFlagsData.size(); ++i)
					{
						if(optimizationData.get(selectedFlagsData.get(i).getFlag()).noOtherFlagsAllowed)
						{
							if(MessageUtils.openQuestionWindow("Optimization Conflict", "The flag " + selectedFlagsData.get(i).getFlag() + " cannot be combined with any other flags. Would you like to replace all the selected flags with this one?"))
							{
								selectedFlagsData.removeAllElements();
								selectedFlags.removeAll();
								if(arguments != null)
									arguments.removeAll();
							}	
							else
								return;
							break;
						}
					}
								
					TableItem item;
					
					//If this flag needs arguments, open up the arguments wizard
					if(optimizationInfo.arguments != null && optimizationInfo.arguments.length > 0 && optimizationInfo.arguments[0] != null && !optimizationInfo.arguments[0].equals(""))
					{
						for(int i = 0; optimizationInfo.selectionValues != null && i < optimizationInfo.selectionValues.length; ++i)
						{
							if(optimizationInfo.selectionValues[i] != null && optimizationInfo.selectionValues[i].length == 0)
							{
								if(optimizationInfo.arguments[i].contains("Label") || optimizationInfo.arguments[i].contains("label") )
									MessageUtils.openErrorWindow("Code Error", "This optimization cannot be used since it requires a label and there are no labels in the source code.");
								else if(optimizationInfo.arguments[i].contains("Module") || optimizationInfo.arguments[i].contains("module") )
									MessageUtils.openErrorWindow("Code Error", "This optimization cannot be used since it requires a module name and there are no modules used in the source code.");
								
								return;
							}
						}
						
						
						argumentsWizard = new OptimizationArgumentWizard(optimizationToBeAdded, optimizationInfo, functionsCalled, null);
						if(EclipseResourceUtils.openWizard(argumentsWizard) == Window.CANCEL)
							return;
						
						item = new TableItem(selectedFlags, SWT.NONE);
						setValues(optimizationToBeAdded, argumentsWizard.value1, argumentsWizard.value2);
						
					}
					else
					{
						item = new TableItem(selectedFlags, SWT.NONE);
						setDefaultArgumentValuesForOptimization(optimizationToBeAdded);
					}
					
					FlagData addedFlagData = selectedFlagsData.get(selectedFlagsData.size() - 1);
					
					try
					{
						//if(addedFlagData.getArgs() != null && addedFlagData.getArgs().length > 1 &&
						//   addedFlagData.getArgs()[1] != null && addedFlagData.getArgs()[1] != "")
						if(optimizationInfo.arguments != null && optimizationInfo.arguments.length > 0 &&
						   optimizationInfo.arguments[0] != null)
						{
							String labelArg = "";
							if(addedFlagData.getFlag().equals("LoopInterchange"))
								labelArg = new String(argumentsWizard.value1 + " <-> " + argumentsWizard.value2);
							else if(optimizationInfo.arguments.length > 1 && addedFlagData.getFlag().equals("LoopUnrolling"))
								labelArg = new String(argumentsWizard.value1 + ": " + argumentsWizard.value2 + (argumentsWizard.value2.equals("FULLY")? "" : " bodies after unroll"));
							else if(optimizationInfo.arguments.length > 1)
								labelArg = new String(argumentsWizard.value1 + ": " + argumentsWizard.value2);
							else if(addedFlagData.getFlag().equals("InlineAllModules"))
								labelArg = new String("Depth: " + argumentsWizard.value1);
							else
								labelArg = new String(argumentsWizard.value1);
							item.setText(new String[]{optimizationToBeAdded, labelArg});
						}	
						else
							item.setText(new String[]{optimizationToBeAdded, ""});
					}
					catch(Exception e)
					{
						e.printStackTrace();
					}
				}
				public void widgetDefaultSelected(SelectionEvent event) 
				{
					widgetSelected(event);
				}
			});
			
			Label l3 = new Label(flagListButtonsComp, 0);
			l3.setText("");
			
			
			Button remove = new Button(flagListButtonsComp, SWT.PUSH);
			remove.setText("< Remove");
			remove.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
			
			remove.addSelectionListener(new SelectionListener() 
			{
				public void widgetSelected(SelectionEvent event) 
				{		
					handleEditor();
					if(selectedFlags.getSelectionIndex() == -1)
						return;
					selectedFlagsData.remove(selectedFlags.getSelectionIndex());
					int[] items = selectedFlags.getSelectionIndices();
					selectedFlags.remove(items);
					if( items != null )
						selectedFlags.select(items[0]);
					else if(selectedFlags.getItemCount() > 0)
					{
						selectedFlags.select(selectedFlags.getItemCount() - 1);
					}
					arguments.removeAll();
				}
				public void widgetDefaultSelected(SelectionEvent event) 
				{
					widgetSelected(event);
				}
			});
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void createSelectedFlagListControl(Composite parent)
	{
		try
		{
	    selectedFlagListComp = CompositeUtilities.createDefaultComposite(parent, 1, false);
		selectedFlags = new Table(selectedFlagListComp,SWT.FULL_SELECTION | SWT.BORDER);
		selectedFlags.setHeaderVisible(true);
		selectedFlags.setLinesVisible(true);

		TableColumn tableColumn = new TableColumn(selectedFlags, SWT.NONE);
		tableColumn.setWidth(225);
		tableColumn.setMoveable(true);
		tableColumn.setText("Selected Flags");
			
		TableColumn argColumn = new TableColumn(selectedFlags, SWT.NONE);
		argColumn.setWidth(160);
		argColumn.setMoveable(true);
		argColumn.setText("Arg");
		
		selectedFlags.addSelectionListener(new SelectionListener()
		{
			public void widgetSelected(SelectionEvent event) 
			{
				handleEditor();
				arguments.removeAll();
				for(int i = 0; i < selectedFlagsData.get(selectedFlags.getSelectionIndex()).getArgs().length; i += 2)
				{
					TableItem item = new TableItem(arguments, SWT.NONE);
					String labelArg = new String();
					item.setText(new String[] {selectedFlagsData.get(selectedFlags.getSelectionIndex()).getArgs()[i], selectedFlagsData.get(selectedFlags.getSelectionIndex()).getArgs()[i + 1]});
				}
			}
			public void widgetDefaultSelected(SelectionEvent event)
			{
				widgetSelected(event);
				
			}
		});
		
		selectedFlags.addMouseListener(new MouseListener()
		{

			public void mouseDoubleClick(MouseEvent e) 
			{
				editSelectedFlagArguments();
			}

			public void mouseDown(MouseEvent e) {				
			}

			public void mouseUp(MouseEvent e) {
			}
			
		});
		
		selectedFlagListComp.addListener(SWT.Resize, new Listener() 
		{
		    int width = -1;
		    public void handleEvent(Event e) 
		    {
		      int newWidth = selectedFlagListComp.getSize().x;
		      if (newWidth != width) 
		      {
		    	selectedFlags.setSize(SWT.DEFAULT, SWT.DEFAULT);
		        Rectangle trim = selectedFlags.computeTrim(0, 0, 0, selectedFlags.getItemHeight() * 5);
				GridData d = CompositeUtilities.createNewGD(0);
				d.heightHint = trim.height;
				selectedFlags.setLayoutData(d);
				selectedFlagListComp.setSize(selectedFlagListComp.getSize().x, selectedFlags.getItemHeight() * 8);
		      }
		    }
		  });
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void createSelectedFlagsButtonsControl(Composite parent)
	{
		try
		{
		Composite selectedFlagsButtonsComp = CompositeUtilities.createDefaultComposite(parent, 1, false);
		
		new Label(selectedFlagsButtonsComp, SWT.NONE).setText("");
		
		
		Button up = new Button(selectedFlagsButtonsComp, SWT.ARROW | SWT.UP);
		up.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
		
		Button edit = new Button(selectedFlagsButtonsComp, SWT.PUSH);
		edit.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
		edit.setText("Edit");
		
		Button down = new Button(selectedFlagsButtonsComp, SWT.ARROW | SWT.DOWN);
		down.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
		
		up.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleEditor();
				if(selectedFlags.getSelection() != null)
				{
					if(selectedFlags.getSelectionIndex() != 0)
					{
						String opt = selectedFlags.getSelection()[0].getText(0);
						String label = selectedFlags.getSelection()[0].getText(1);
					
						selectedFlags.getSelection()[0].setText(0, selectedFlags.getItem(selectedFlags.getSelectionIndex() - 1).getText(0));
						selectedFlags.getSelection()[0].setText(1, selectedFlags.getItem(selectedFlags.getSelectionIndex() - 1).getText(1));
						
						selectedFlags.getItem(selectedFlags.getSelectionIndex() - 1).setText(0, opt);
						selectedFlags.getItem(selectedFlags.getSelectionIndex() - 1).setText(1, label);
						
						FlagData fd = selectedFlagsData.get(selectedFlags.getSelectionIndex());
						fd = new FlagData(fd.getFlag(), fd.getArgs());
						selectedFlagsData.set(selectedFlags.getSelectionIndex(), selectedFlagsData.get(selectedFlags.getSelectionIndex() - 1));
						selectedFlagsData.set(selectedFlags.getSelectionIndex() - 1, fd);
						
						selectedFlags.setSelection(selectedFlags.getSelectionIndex() - 1);
					}
				}
			}
		});
		
		down.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleEditor();
				if(selectedFlags.getSelection() != null)
				{
					if(selectedFlags.getSelectionIndex() != selectedFlags.getItemCount() - 1)
					{
						String opt = selectedFlags.getSelection()[0].getText(0);
						String label = selectedFlags.getSelection()[0].getText(1);
					
						selectedFlags.getSelection()[0].setText(0, selectedFlags.getItem(selectedFlags.getSelectionIndex() + 1).getText(0));
						selectedFlags.getSelection()[0].setText(1, selectedFlags.getItem(selectedFlags.getSelectionIndex() + 1).getText(1));
						
						selectedFlags.getItem(selectedFlags.getSelectionIndex() + 1).setText(0, opt);
						selectedFlags.getItem(selectedFlags.getSelectionIndex() + 1).setText(1, label);
						
						FlagData fd = selectedFlagsData.get(selectedFlags.getSelectionIndex());
						fd = new FlagData(fd.getFlag(), fd.getArgs());
						selectedFlagsData.set(selectedFlags.getSelectionIndex(), selectedFlagsData.get(selectedFlags.getSelectionIndex() + 1));
						selectedFlagsData.set(selectedFlags.getSelectionIndex() + 1, fd);
						
						selectedFlags.setSelection(selectedFlags.getSelectionIndex() + 1);
					}
				}
			}
		});
		
		edit.addSelectionListener(new SelectionListener()
		{

			public void widgetDefaultSelected(SelectionEvent e) {
				// TODO Auto-generated method stub
				
			}

			public void widgetSelected(SelectionEvent e) 
			{
				editSelectedFlagArguments();
			}
			
		});
		
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void createDescriptionControl(Composite parent)
	{
		try
		{
			Composite selectedFlagListButtonsComp = new Composite(parent, SWT.NONE);
			GridLayout layout = new GridLayout();
			layout.numColumns = 1;
			selectedFlagListButtonsComp.setLayout(layout);
			selectedFlagListButtonsComp.setLayoutData(CompositeUtilities.createNewGD(GridData.GRAB_HORIZONTAL | GridData.GRAB_VERTICAL| GridData.HORIZONTAL_ALIGN_FILL | GridData.VERTICAL_ALIGN_FILL | SWT.FILL));
			
			title = new Label(selectedFlagListButtonsComp, SWT.HORIZONTAL | SWT.LEFT);
			title.setFont(new Font(null, title.getFont().getFontData()[0].name, 14, SWT.BOLD));
			description = new Label(selectedFlagListButtonsComp, SWT.HORIZONTAL | SWT.LEFT);
			description2 = new Label(selectedFlagListButtonsComp, SWT.HORIZONTAL | SWT.LEFT);
			title.setText("Optimization");
			description.setText(" ");
			description2.setText(" ");
			//Color c = new Color(null, 0, 0, 255);
			
			//title.setForeground(c);
			title.setSize(title.computeSize(SWT.DEFAULT, title.getFont().getFontData()[0].getHeight() + 5));
			description.setSize(description.computeSize(SWT.DEFAULT, SWT.DEFAULT));
			description2.setSize(description2.computeSize(SWT.DEFAULT, SWT.DEFAULT));
		
			title2 = new Label(selectedFlagListButtonsComp, SWT.HORIZONTAL | SWT.LEFT);
			title2.setFont(new Font(null, title2.getFont().getFontData()[0].name, 14, SWT.BOLD));
			title2.setText("Example:");
			title2.setSize(title2.computeSize(SWT.DEFAULT, title2.getFont().getFontData()[0].getHeight() + 5));
			
			exampleComp = CompositeUtilities.createDefaultComposite(selectedFlagListButtonsComp, 3, false, SWT.CENTER);
			exampleComp.setLayoutData(CompositeUtilities.createNewGD(SWT.CENTER));
			
			new Label(exampleComp, SWT.NONE).setText("\t");
			
			
			image = new Label(exampleComp, SWT.CENTER);
			image.setText("");
			image.setImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/optimizationDescriptions/" + "blank" + ".png")));
			
			//title.setSize(title.computeSize(SWT.DEFAULT, title.getFont().getFontData()[0].getHeight() + 5));
			//description.setSize(description.computeSize(SWT.DEFAULT, SWT.DEFAULT));
			//description2.setSize(description2.computeSize(SWT.DEFAULT, description2.getFont().getFontData()[0].getHeight() + 15 ));
			
			
			//Button moreInfoButton = new Button(selectedFlagListButtonsComp, SWT.PUSH);
			//moreInfoButton.setText("More Help");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void createFlagArgumentsControl(Composite parent)
	{
		try
		{
		Composite argumentsComp = CompositeUtilities.createDefaultComposite(parent, 2, false);
		
		arguments = new Table(argumentsComp, SWT.FULL_SELECTION | SWT.BORDER);
		arguments.setHeaderVisible(true);
		arguments.setLinesVisible(true);
		arguments.setLayoutData(new GridData(GridData.FILL_BOTH));
		
		TableColumn tableColumn = new TableColumn(arguments, SWT.NONE);
		tableColumn.setMoveable(false);
		tableColumn.setWidth(350);
		tableColumn.setText("Argument Name");
		
		TableColumn tableColumn2 = new TableColumn(arguments, SWT.NONE);
		tableColumn2.setMoveable(true);
		tableColumn2.setWidth(350);
		tableColumn2.setText("Value");
		
		TableItem item = new TableItem(arguments, SWT.NONE);
		TableItem item2 = new TableItem(arguments, SWT.NONE);
		TableItem item3 = new TableItem(arguments, SWT.NONE);
		
		editor = new TableEditor(arguments);
		editor.horizontalAlignment = SWT.LEFT;
		editor.grabHorizontal = true;
		editor.minimumWidth = 50;
		
		arguments.addMouseListener(new MouseListener()
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
					selectedFlagsData.get(selectedIndex).getArgs()[selectedRow * 2 + 1] = new String(text.getText());
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
					if (oldEditor != null) 
						oldEditor.dispose();
					
					//Get the item we clicked on
					final TableItem item = arguments.getItem(new Point(event.x, event.y));
					
					//Get the name of the optimization so we can access the vector for saving
					optimization = selectedFlagsData.get(selectedFlags.getSelectionIndex()).getFlag();//selectedFlags.getItem(selectedFlags.getSelectionIndex()).getText().toString();
					selectedIndex = selectedFlags.getSelectionIndex();
					if (item == null)
					{
						return;
					}					
					// The control that will be the editor must be a child of the Table
					final Text newEditor = new Text(arguments, SWT.NONE);
					
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
					
					selectedRow = arguments.getSelectionIndex();
					
					if(editColumn == 0)
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
			public void mouseUp(MouseEvent event)
			{
			}			
		});	
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void editSelectedFlagArguments()
	{
		try
		{
			if(selectedFlags.getSelectionIndex() > selectedFlags.getItemCount())
				return;
				
			FlagData fd = selectedFlagsData.get(selectedFlags.getSelectionIndex());				
			OptimizationInfo optimizationInfo = optimizationData.get(fd.getFlag());
			
			if(optimizationInfo.arguments == null || optimizationInfo.arguments.length == 0)
				return;
			
			String[] initialVals;
			if(optimizationInfo.arguments.length == 1)
				initialVals = new String[]{fd.getArgs()[1]};
			else
				initialVals = new String[]{fd.getArgs()[1], fd.getArgs()[3]};
			
			argumentsWizard = new OptimizationArgumentWizard(fd.getFlag(), optimizationInfo, functionsCalled, initialVals);
			
			if(EclipseResourceUtils.openWizard(argumentsWizard) == Window.CANCEL)
				return;
			
			fd.getArgs()[1] = argumentsWizard.value1;
			if(optimizationInfo.arguments.length > 1)
				fd.getArgs()[3] = argumentsWizard.value2;
			
			TableItem item = selectedFlags.getItem(selectedFlags.getSelectionIndex());
			
			String labelArg = "";
			if(fd.getFlag().equals("LoopInterchange"))
				labelArg = new String(argumentsWizard.value1 + " <-> " + argumentsWizard.value2);
			else if(optimizationInfo.arguments.length > 1 && fd.getFlag().equals("LoopUnrolling"))
				labelArg = new String(argumentsWizard.value1 + ": " + argumentsWizard.value2 + (argumentsWizard.value2.equals("FULLY")? "" : " bodies after unroll"));
			else if(optimizationInfo.arguments.length > 1)
				labelArg = new String(argumentsWizard.value1 + ": " + argumentsWizard.value2);
			else if(fd.getFlag().equals("InlineAllModules"))
				labelArg = new String("Depth: " + argumentsWizard.value1);
			else
				labelArg = new String(argumentsWizard.value1);
			
			item.setText(new String[]{fd.getFlag(), labelArg});
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void setDefaultArgumentValuesForOptimization(String flag)
	{
		try
		{
			//Set the values with whatever default values we stored in our optimizationData.
			setValues(flag, "", "");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
	}
	
	private void setValues(String name, String value1, String value2)
	{
		try
		{
			if(value1 != null && value1.length() > 0)
			{
				if(value2 != null && value2.length() > 0)
				{
					selectedFlagsData.add(new FlagData(name, new String[]{ optimizationData.get(name).arguments[0], value1, optimizationData.get(name).arguments[1], value2}));
				}
				else
				{
					selectedFlagsData.add(new FlagData(name, new String[]{ optimizationData.get(name).arguments[0], value1}));
				}
			}
			else
			{
				selectedFlagsData.add(new FlagData(name, new String[]{}));
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void saveSelectedFlagsToBuffer(StringBuffer buffer)
	{
		//Save all the flags and arguments that are in our selected 
		//flags list to the passed in stringbuffer.
		try
		{
			if(selectedFlagsData != null)
			{
				for(int i = 0; i < selectedFlagsData.size(); ++i)
				{
					buffer.append(selectedFlagsData.get(i).getFlag());
					for(int j = 1; j < selectedFlagsData.get(i).getArgs().length; j+=2)
					{
						buffer.append(" " + selectedFlagsData.get(i).getArgs()[j]);
					}
					buffer.append("\n");
				}
			}
			
			
			if(constantFlagsData != null)
			{
				for(int i = 0; i < constantFlagsData.size(); ++i)
				{
					buffer.append(constantFlagsData.get(i));
					buffer.append("\n");
				}
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public void saveFlags()
	{
		/*IFile file = null;// = CompositeUtilities.getSelectedFileInEditor();
		IFolder folder = ResourcesPlugin.getWorkspace().getRoot().getProject(file.getProject().getName()).getFolder(folderString + ".ROCCC/"); 
		try 
		{
			if(!folder.exists())
				folder.create(true, true, null);
		}
		catch (CoreException e1) 
		{
			e1.printStackTrace();
		}*/
		
		StringBuffer buffer = new StringBuffer();
		saveSelectedFlagsToBuffer(buffer);
		
		//Actually create the source file.
		try
		{
			if(savedOptimizationsFile.exists())
				savedOptimizationsFile.delete();

			FileUtils.createFileFromBuffer(buffer, savedOptimizationsFile);
		}
		catch(Exception e)
		{
			e.printStackTrace();
			MessageDialog.openInformation(new Shell(), "Error", "There was an error creating your file.");
		}
	}
	
	private void loadDefaultFlags()
	{
		//Grab the flags we have stored in the preference store.
		String flags = PreferenceUtils.getPreferenceString(defaultsPreference);
		
		//Put the flags in a stringbuffer and load them.
		StringBuffer buffer = new StringBuffer(flags);
		loadOptimizations(buffer);
	}
	

	private void saveOptimizationsAsDefaults()
	{
		//Put the selected flags into a stringbuffer.
		StringBuffer buffer = new StringBuffer();
		saveSelectedFlagsToBuffer(buffer);
		
		//Put the flags in the stringbuffer into the preference store.
		Activator.getDefault().getPreferenceStore().setValue(defaultsPreference, buffer.toString());
	}
	
	public void loadExistingFlags()
	{		
		try
		{
			//If there is a previous opt file storing previously used flags, open it and load the flags.
			if(savedOptimizationsFile.exists() && savedOptimizationsFile.canRead())
			{
				StringBuffer buffer = new StringBuffer();
				FileUtils.addFileContentsToBuffer(buffer, savedOptimizationsFile.getAbsolutePath());
				loadOptimizations(buffer);
			}
			else
			{
				//If there is no previous opt file containing flags, load the defaults instead.
				loadDefaultFlags();
			}
			
			flagList.select(0);
			flagList.notifyListeners(SWT.Selection, null);
			flagList.notifyListeners(SWT.Selection, null);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void loadOptimizations(StringBuffer buf)
	{
		try
		{
			//Clear our selected flags vector and table.
			selectedFlagsData = new Vector<FlagData>();
			selectedFlags.removeAll();

			//Read the buffer loading the flags until we don't have anything left in the buffer.
			while(buf.length() > 0)
			{
				//Get the flag name.
				String flag = StringUtils.getNextStringValue(buf);
				String arg1 = "";
				String arg2 = "";
				
				//If we got a value for the flag, lets see if its valid.
				if(flag != null && flag.length() > 0)
				{
					//Check to see if the flag we got is in our acceptable list.
					boolean shouldAccept = false;
					for(int i = 0; i < acceptedFlags.size(); ++i)
					{
						if(acceptedFlags.get(i).equals(flag))
						{
							shouldAccept = true;
							break;
						}
					}
					
					//If the flag should be accepted, let's see if we need to load arguments for it.
					if(shouldAccept)
					{
						//If it needs one argument let's load it.
						if(buf.length() > 0 && flagNeedsFirstArg(flag))
						{
							arg1 = StringUtils.getNextStringValue(buf);
							
							//If it needs a second argument, let's load it.
							if(buf.length() > 0 && flagNeedsSecondArg(flag))
								arg2 = StringUtils.getNextStringValue(buf);
						}
						
						//Make sure if the flag has a selection style that the value read in exists in the selection possibilities.
						OptimizationInfo optInfo = optimizationData.get(flag);
						
						if(optInfo.optimizationValueTypes != null && optInfo.optimizationValueTypes.length > 0 && optInfo.optimizationValueTypes[0] == OptimizationValueType.SELECTION)
						{
							boolean found = false;
							for(int i = 0; i < optInfo.selectionValues[0].length; ++i)
							{
								if(arg1.equals(optInfo.selectionValues[0][i]))
								{
									found = true;
									break;
								}
							}
							
							if(!found)
								continue;
						}
						if(optInfo.optimizationValueTypes != null && optInfo.optimizationValueTypes.length > 1 && optInfo.optimizationValueTypes[1] == OptimizationValueType.SELECTION)
						{
							boolean found = false;
							for(int i = 0; i < optInfo.selectionValues[1].length; ++i)
							{
								if(arg2.equals(optInfo.selectionValues[1][i]))
								{
									found = true;
									break;
								}
							}
							
							if(!found)
								continue;
						}
						
						//Save these values to our selectedFlags vector and table.
						setValues(flag, arg1, arg2);
						String argString = arg1;
						String labelArg = "";
						if(flag.equals("LoopInterchange"))
							labelArg = new String(arg1 + " <-> " + arg2);
						else if(flag.equals("LoopUnrolling"))
							labelArg = new String(arg1 + ": " + arg2 + (arg2.equals("FULLY")? "" : " bodies after unroll"));
						else if(arg2.length() > 0)
							labelArg = new String(arg1 + ": " + arg2);
						else if(flag.equals("InlineAllModules"))
							labelArg = new String("Depth: " + arg1);
						else
							labelArg = new String(arg1);
						
						new TableItem(selectedFlags, SWT.NONE).setText(new String[]{flag, labelArg});
					}
				}		
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private boolean flagNeedsFirstArg(String flag)
	{
		//If the flag is a flag we used in our available flags and
		//it has a description for the first argument, then this
		//flag needs at least one argument.
		if(optimizationData.containsKey(flag) == false)
			return false;
		
		return optimizationData.get(flag).arguments != null && optimizationData.get(flag).arguments.length > 0;
	}
	
	private boolean flagNeedsSecondArg(String flag)
	{
		//If the flag is a flag we used in our available flags and
		//it has a description for the second argument, then this
		//flag needs a second argument.
		if(optimizationData.containsKey(flag) == false)
			return false;
		
		return optimizationData.get(flag).arguments != null && optimizationData.get(flag).arguments.length > 1;
	}
	
	
	private void handleEditor()
	{
		//This function will save any values that we were editing
		//in the tables to our containers that house that data.
		
		try
		{
			if(selectedFlagsData == null)
				return;
			if(editor == null)
				return;
			if(editor.getEditor() == null)
				return;
			if(editor.getEditor().isDisposed())
				return;
			if(selectedIndex == -1)
				return;
			if(selectedFlagsData.size() == 0)
			{
				editor.getEditor().dispose() ;
				return ;
			}
			if(selectedFlagsData.get(selectedIndex) == null)
				return;
			String[] newStrings;
			newStrings = selectedFlagsData.get(selectedIndex).getArgs();
			if(newStrings == null)
				return;
			if(selectedRow * 2 + 1 > newStrings.length - 1)
				return;
			if(editor.getItem() == null || editor.getItem().getText(1) == null)
				return;
			newStrings[selectedRow * 2 + 1] = new String(editor.getItem().getText(1));
			selectedFlagsData.removeElementAt(selectedIndex);
			selectedFlagsData.add(selectedIndex, new FlagData(optimization, newStrings));
			
			
			if(newStrings.length > 1 && newStrings[1] != null && newStrings[1] != "")
			{
				String labelArg = new String();
				if(selectedFlagsData.get(selectedIndex).getFlag().equals("LoopInterchange"))
					labelArg = new String(newStrings[1] + " <-> " + newStrings[3]);
				else
					labelArg = new String(newStrings[1]);
				selectedFlags.getItem(selectedIndex).setText(new String[]{selectedFlagsData.get(selectedIndex).getFlag(), labelArg});
			}
			else
			{
				selectedFlags.getItem(selectedIndex).setText(new String[]{selectedFlagsData.get(selectedIndex).getFlag(), ""});
			}
			editor.getEditor().dispose();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
}


