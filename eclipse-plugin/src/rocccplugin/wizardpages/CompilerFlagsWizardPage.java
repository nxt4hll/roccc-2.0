
package rocccplugin.wizardpages;

import java.io.File;
import java.util.Vector;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.custom.TableEditor;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Table;

import rocccplugin.Activator;
import rocccplugin.composites.OptimizationSelector;
import rocccplugin.composites.OptimizationSelector.OptimizationInfo;
import rocccplugin.composites.OptimizationSelector.OptimizationValueType;
import rocccplugin.helpers.FlagData;
import rocccplugin.helpers.ResourceData;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.CompositeUtilities;
import rocccplugin.utilities.MessageUtils;

public class CompilerFlagsWizardPage extends WizardPage 
{
	TableEditor editor;
	static private File fileToCompile;
	static String componentType;
	public Vector<FlagData> v;
	private int selectedRow;
	private int selectedIndex;
	private String optimization;
	public Table flagList;
	public Table selectedFlags;
	public Table arguments;
	public Label description;
	public Label description2;
	public Label title;
	Vector<ResourceData> functionsCalled;
	Vector<String> labelsPlaced;
	Composite flagListButtonsComp;
	Composite flagListComp;
	Composite selectedFlagListComp;
	
	Vector<String> acceptedFlags;
	
	final String defaultsPreference = PreferenceConstants.DEFAULT_HIGH_OPTIMIZATIONS;
	
	boolean fileContainsSystemCalls;
	
	public OptimizationSelector optimizationSelector;
	private static File saveLocation;
	
	public CompilerFlagsWizardPage(String pageName, boolean containsSystemCalls, Vector<ResourceData> functionsCalled, Vector<String> labelsPlaced) 
	{
		super(pageName);
		
		try
		{
			String title = "Select System Compiler Flags For " + fileToCompile.getName();
			if(componentType.equals("MODULE"))
				title = "Select Module Compiler Flags For " + fileToCompile.getName();
			else if(componentType.equals("SYSTEM"))
				title = "Select System Compiler Flags For " + fileToCompile.getName();
			else
				title = "Select Intrinsic Compiler Flags For " + fileToCompile.getName();
			
			title = "Select High-Level Compiler Flags for " + fileToCompile.getName();
			
			fileContainsSystemCalls = containsSystemCalls;
			
			this.functionsCalled = functionsCalled;
			this.labelsPlaced = labelsPlaced;
			
			setTitle(title);
			setDescription("Please select the high level compiler flags for " + fileToCompile.getName() + ". " + (componentType.equals("MODULE")? " All loops will automatically be fully unrolled." : ""));
		}
		
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	static public void setParameters(File theFile, String componentType)
	{
		fileToCompile = theFile;
		CompilerFlagsWizardPage.componentType = componentType;
		String folderString = fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "");
		saveLocation = new File(folderString + ".ROCCC/" + fileToCompile.getName().replace(".c", ".opt"));
	}
	
	public void createControl(Composite parent)
	{
		try
		{
			String[] functionValues = new String[functionsCalled.size()];
			
			for(int i = 0; i < functionsCalled.size(); ++i)
			{
				functionValues[i] = functionsCalled.get(i).name;
			}
			
			String[] labels = new String[labelsPlaced.size()];
			
			for(int i = 0; i < labelsPlaced.size(); ++i)
			{
				labels[i] = labelsPlaced.get(i);
			}
			
			Composite top = CompositeUtilities.createDefaultComposite(parent,1, false);
			setControl(top);
			String folderString = fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "");
			optimizationSelector = new OptimizationSelector(top, fileToCompile, saveLocation, "High Level", functionsCalled);
		
			optimizationSelector.addFlags("MultiplyByConstElimination", null, null, new String[]{"Replace all integer multiplications by constants with equivalent shifts and additions.", ""}, null, null, true, false);
			optimizationSelector.addFlags("DivisionByConstElimination", null, null, new String[]{"Replace all integer divisions by constants of powers of two with equivalent shifts if applicable.", ""}, null, null, true, false);
			
			if(componentType.equals("SYSTEM"))
			{
				optimizationSelector.addFlags("FullyUnroll", null, null, new String[]{"Fully unroll all loops in the program.", ""}, null, null, false, false);
			}
			
			optimizationSelector.addFlags("InlineAllModules", new String[]{"Depth"}, new String[]{ "/*Enter a positive integer or INFINITE for how deep to inline*/"}, new String[]{"Inlines all module calls found in a depth amount that is specified.", ""}, new OptimizationValueType[]{OptimizationValueType.AMOUNT}, null, false, false);
			
			optimizationSelector.addFlags("InlineModule", new String[]{"Name of Module Call"}, new String[]{"/*Name*/"}, new String[]{"Removes the architectural restrictions imposed by black box construction allowing for current", "optimizations to transform the internals of the inlined module."}, new OptimizationValueType[]{OptimizationValueType.SELECTION}, new String[][]{functionValues}, true, false);
			
			if(componentType.equals("SYSTEM"))
			{
				optimizationSelector.addFlags("LoopFusion", null, null, new String[]{"Merge successive loops with the same bounds.", ""}, null, null, false, false);
				optimizationSelector.addFlags("LoopInterchange", new String[]{"First Loop Label", "Second Loop Label"}, new String[]{"/*Label1*/", "/*Label2*/"}, new String[]{"Switch the order of two nested loops, which must be identified in the C code with labels.", ""}, new OptimizationValueType[]{OptimizationValueType.SELECTION, OptimizationValueType.SELECTION}, new String[][]{labels, labels}, true, false);
				optimizationSelector.addFlags("LoopUnrolling", new String[]{"Loop Label", "Number of Loop Bodies After Unrolling"}, new String[]{"/*Label*/", "/*Enter a positive integer or enter FULLY to fully unroll*/"}, new String[]{"Unroll the loop at the given C label amount. If the loop has constant bounds, the loop can be fully unrolled.", ""}, new OptimizationValueType[]{OptimizationValueType.SELECTION, OptimizationValueType.AMOUNT}, new String[][]{labels, null}, true, false);	
			}

			optimizationSelector.addFlags("Redundancy", new String[]{"Label at Module Call", "DOUBLE or TRIPLE"}, new String[]{"/*Label*/", "/*DOUBLE or TRIPLE*/"}, new String[]{"Enable dual or triple redundancy for a module at a given C label.", ""}, new OptimizationValueType[]{OptimizationValueType.SELECTION, OptimizationValueType.SELECTION}, new String[][]{labels, new String[]{"DOUBLE", "TRIPLE"}}, true, false);

			if(componentType.equals("SYSTEM"))
			{
				optimizationSelector.addFlags("SystolicArrayGeneration", new String[]{"Outer Loop Label"}, new String[]{"/*Label*/"}, new String[]{"Transform a wavefront algorithm that works over a two-dimensional array into a one-dimensional hardware", "structure with feedback at every stage in order to increase the throughput while reducing hardware."}, new OptimizationValueType[]{OptimizationValueType.SELECTION}, new String[][]{labels}, false, true);
				optimizationSelector.addFlags("TemporalCommonSubExpressionElimination", null, null, new String[]{"Detect and remove common code across loop iterations to reduce the size of the generated code,", "requiring initial values for each piece of hardware eliminated."}, null, null, false, false);
			
			
				optimizationSelector.flagCannotBeCombinedWithOthers("SystolicArrayGeneration");
			}
			
			if(componentType.equals("MODULE"))
			{
				optimizationSelector.addConstantFlag("Export");
				optimizationSelector.addConstantFlag("FullyUnroll");
			}
			
			if(componentType.equals("SYSTEM") && fileContainsSystemCalls)
			{
				optimizationSelector.addConstantFlag("ComposedSystem");
			}
			
			//Give the preference that houses the default flags for this page.
			optimizationSelector.setDefaultsPreference(PreferenceConstants.DEFAULT_HIGH_OPTIMIZATIONS);
			
			optimizationSelector.loadExistingFlags();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
	}	
	
	public void saveFlags()
	{
		optimizationSelector.saveFlags();
	}
	
	public String getSavedFile()
	{
		return saveLocation.getAbsolutePath();
	}
	
	public Vector<FlagData> getSelectedFlags()
	{
		return optimizationSelector.selectedFlagsData;
	}

	/*public void loadExistingFlags()
	{
		String folderString = file.getProjectRelativePath().toString().replace(file.getName(), "");
		
		IFile f = ResourcesPlugin.getWorkspace().getRoot().getProject(file.getProject().getName()).getFile(folderString + ".ROCCC/" + file.getName().replace(".c", ".opt"));
		
		if(f.getRawLocation().toFile().exists() && f.getRawLocation().toFile().canRead())
		{
			StringBuffer buffer = new StringBuffer();
			Activator.addFileContentsToBuffer(buffer, ResourcesPlugin.getWorkspace().getRoot().getProject(file.getProject().getName()).getFile(folderString + ".ROCCC/" + file.getName().replace(".c", ".opt")).getRawLocation().toOSString());
			
			loadOptimizations(buffer);
		}
		else
		{
			loadDefaultOptimizations();
		}
	}
	
	static public void setParameters(IFile theFile, String componentType)
	{
		file = theFile;
		CompilerFlagsWizardPage.componentType = componentType;
	}
	
	public boolean isPageComplete()
	{
		return true;
	}
	
	private void getDefaultValues(String name)
	{
		if(v == null)
			v = new Vector<FlagData>();
		
		if(name.compareTo("LoopUnrolling") == 0)
		{
			if(!componentType.equals("SYSTEM"))
				//setValues(name, "/*Label", "");
			//else
				//setValues(name, "/*Label", "/*Enter a constant or enter FULLY to fully unroll");
		}
		else if(name.compareTo("FullyUnroll") == 0)
		{
			setValues(name, "", "");
		}
		else if(name.compareTo("LoopInterchange") == 0)
		{
			setValues(name, "/*Label1", "/*Label2");
		}
		else if(name.compareTo("Redundancy") == 0)
		{
			setValues(name, "/*Label", "/*DOUBLE or TRIPLE");
		}
		else if(name.compareTo("LoopFusion") == 0)
		{
			setValues(name, "", "");
		}
		else if(name.compareTo("MultiplyByConstElimination") == 0)
		{
			setValues(name, "", "");
		}
		else if(name.compareTo("DivisionByConstElimination") == 0)
		{
			setValues(name, "", "");
		}
		else if(name.compareTo("SystolicArrayGeneration") == 0)
		{
			setValues(name, "/*Label", "");
		}
		else if(name.compareTo("TemporalCommonSubExpressionElimination") == 0)
		{
			setValues(name, "", "");
		}
		else if(name.compareTo("Export") == 0)
		{
			setValues(name, "", "");
		}
		else if(name.compareTo("InlineModule") == 0)
		{
			setValues(name, "/*Module Name", "");
		}
	}
	
	private void setValues(String name, String value1, String value2)
	{		
		if(name.compareTo("LoopUnrolling") == 0)
		{
			if(!componentType.equals("SYSTEM"))
				v.add(new FlagData(name, new String[] {"Loop Label", value1}));
			else
				v.add(new FlagData(name, new String[] {"Loop Label", value1, "Number of Unrolled Loop Bodies", value2}));
		}
		else if(name.compareTo("FullyUnroll") == 0)
		{
			v.add(new FlagData(name, new String[]{}));
		}
		else if(name.compareTo("LoopInterchange") == 0)
		{
			v.add(new FlagData(name, new String[] {"First Loop Label", value1, "Second Loop Label", value2}));
		}
		else if(name.compareTo("Redundancy") == 0)
		{
			v.add(new FlagData(name, new String[] {"Label at Module Call", value1, "DOUBLE or TRIPLE", value2}));
		}
		else if(name.compareTo("LoopFusion") == 0)
		{
			v.add(new FlagData(name, new String[] {}));
		}
		else if(name.compareTo("MultiplyByConstElimination") == 0)
		{
			v.add(new FlagData(name, new String[] {}));
		}
		else if(name.compareTo("DivisionByConstElimination") == 0)
		{
			v.add(new FlagData(name, new String[] {}));
		}
		else if(name.compareTo("SystolicArrayGeneration") == 0)
		{
			v.add(new FlagData(name, new String[] {"Outer Loop Label", value1}));
		}
		else if(name.compareTo("TemporalCommonSubExpressionElimination") == 0)
		{
			v.add(new FlagData(name, new String[] {}));
		}
		else if(name.compareTo("Export") == 0)
		{
			v.add(new FlagData(name, new String[] {}));
		}
		else if(name.compareTo("InlineModule") == 0)
		{
			v.add(new FlagData(name, new String[]{"Module Name", value1}));
		}
	}
	
	private String getDescription(String flag)
	{	
		if(flag.compareTo("TemporalCommonSubExpressionElimination") == 0)
			return new String("Detect and remove common code across loop iterations to reduce the size of the generated code,");
		else if(flag.compareTo("LoopUnrolling") == 0)
		{
			if(!componentType.equals("SYSTEM"))
				return new String("Fully unroll the loop at the given C label. Loops cannot exist in modules so all loops must be fully unrolled.");
			else
				return new String("Unroll the loop at the given C label by an amount. If the loop has constant bounds, the loop can be fully unrolled.");
		
		}
		else if(flag.compareTo("FullyUnroll") == 0)
		{
			return new String("Fully unroll all loops in the program.");
		}
		else if(flag.compareTo("LoopInterchange") == 0)
			return new String("Switch the order of two nested loops, which must be identified in the C code with labels.");
		else if(flag.compareTo("LoopFusion") == 0)
			return new String("Merge successive loops with the same bounds.");
		else if(flag.compareTo("MultiplyByConstElimination") == 0)
			return new String("Replace all integer multiplications by constants with equivalent shifts and additions.");
		else if(flag.compareTo("DivisionByConstElimination") == 0)
			return new String("Replace all integer divisions by constants with equivalent shifts if applicable.");
		else if(flag.compareTo("Export") == 0) 
			return new String("Update the roccc-library.h file to allow this module to be used in other code.  Warning! ");
		else if(flag.compareTo("SystolicArrayGeneration") == 0)
			return new String("Transform a wavefront algorithm that works over a 2-dimensional array into a one-dimensional hardware");
		else if(flag.compareTo("Redundancy") == 0)
			return new String("Enable dual or triple redundancy for a module at a given C label.");
		else if(flag.compareTo("InlineModule") == 0)
			return new String("Inlines the modules call rather than using the compiled version.");
		
		return new String("");
	}
	
	private String getDescription2(String flag)
	{
		if(flag.compareTo("SystolicArrayGeneration") == 0)
			return new String("structure with feedback at every stage in order to increase throughput while reducing hardware.");
		else if(flag.compareTo("Export") == 0)
			return new String("If previously exported, the module must be deleted from the database to export again.");
		else if(flag.compareTo("TemporalCommonSubExpressionElimination") == 0)
			return new String("requiring initial values for each piece of hardware eliminated.");
		return new String("");
	}
	
	private void saveSelectedFlagsToBuffer(StringBuffer buffer)
	{
		try
		{
			if(v != null)
			{
				for(int i = 0; i < v.size(); ++i)
				{
					buffer.append(v.get(i).getFlag());
					for(int j = 1; j < v.get(i).getArgs().length; j+=2)
					{
						buffer.append(" " + v.get(i).getArgs()[j]);
					}
					if(!componentType.equals("SYSTEM") && v.get(i).getFlag().equals("LoopUnrolling"))
						buffer.append(" FULLY");
					buffer.append("\n");
				}
			}
			
			if(componentType.equals("MODULE"))
			{
				buffer.append("FullyUnroll\n");
				
			}
			
			buffer.append("Export\n");
			
			if(!componentType.equals("SYSTEM") && !componentType.equals("MODULE"))
			{
				buffer.append(componentType + "\n");
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void saveOptimizationsAsDefaults()
	{
		StringBuffer buffer = new StringBuffer();
		saveSelectedFlagsToBuffer(buffer);
		
		Activator.getDefault().getPreferenceStore().setValue(defaultsPreference, buffer.toString());
	}
	
	public void saveFlags()
	{
		StringBuffer buffer = new StringBuffer();
		saveSelectedFlagsToBuffer(buffer);
		
		IFile theFile;
		String folderString = file.getProjectRelativePath().toString().replace(file.getName(), "");
		
		IFolder folder = ResourcesPlugin.getWorkspace().getRoot().getProject(file.getProject().getName()).getFolder(folderString + ".ROCCC/"); 
		try 
		{
			if(!folder.exists())
				folder.create(true, true, null);
		}
		catch (CoreException e1) 
		{
			e1.printStackTrace();
		}
		
		theFile = ResourcesPlugin.getWorkspace().getRoot().getProject(file.getProject().getName()).getFile(folderString + ".ROCCC/" + file.getName().replace(".c", ".opt"));
		File fullFilePath = new File(theFile.getRawLocation().toOSString());
		//Actually create the source file.
		try
		{
			ByteArrayInputStream Bis1 = new ByteArrayInputStream(buffer.toString().getBytes("UTF-8"));
			fullFilePath = new File(theFile.getRawLocation().toOSString());
			if(theFile.exists())
				theFile.delete(true, true, null);
			if(fullFilePath.exists())
				fullFilePath.delete();
			theFile.create(Bis1, false, null);
		}
		catch(java.lang.Exception e)
		{
			e.printStackTrace();
			MessageDialog.openInformation(new Shell(), "Error", "There was an error creating your file.");
		}
	}
	
	private void loadDefaultOptimizations()
	{
		String flags = Activator.getDefault().getPreferenceStore().getString(defaultsPreference);
		
		StringBuffer buffer = new StringBuffer(flags);
		loadOptimizations(buffer);
	}
	
	private void loadOptimizations(StringBuffer buffer)
	{
		v = new Vector<FlagData>();
		selectedFlags.removeAll();
		
		while(buffer.length() > 1)
		{
			try
			{
				String flag = StringUtils.getNextStringValue(buffer);
				boolean validFlag = false; 
				
				for(int i = 0; i < acceptedFlags.size(); ++i)
				{
					if(acceptedFlags.get(i).equals(flag))
					{
						validFlag = true;
						break;
					}
				}
				
				if(!validFlag)
					continue;
				
				String value1 = new String();
				String value2 = new String();
				
				if(flag.compareTo("LoopUnrolling") == 0 || flag.compareTo("LoopInterchange") == 0 ||
				   flag.compareTo("SystolicArrayGeneration") == 0 || flag.compareTo("Redundancy") == 0 ||
				   flag.compareTo("InlineModule") == 0)
				{
					value1 = StringUtils.getNextStringValue(buffer);
				}
				
				if(flag.compareTo("LoopUnrolling") == 0 || flag.compareTo("LoopInterchange") == 0 ||
				   (flag.compareTo("Redundancy") == 0))
				{
					value2 = StringUtils.getNextStringValue(buffer);
				}
				
				String lookAhead = StringUtils.getNextStringValue(buffer);
				boolean isLastFlag = lookAhead.equals("");
				buffer.insert(0, lookAhead + " ");
				
				if(flag.equals("Export") || ((flag.equals("FullyUnroll") || flag.equals("LoopUnrolling") || flag.equals("LoopInterchange")) && componentType.equals("MODULE")) || 
				   flag.equals("INT_DIV") || flag.equals("INT_MOD") || 
				   flag.equals("FP_ADD") || flag.equals("FP_SUB")  || flag.equals("FP_MUL")  || 
				   flag.equals("FP_DIV") || flag.equals("FP_GREATER_THAN") || flag.equals("FP_LESS_THAN") ||
				   flag.equals("FP_EQUAL") || flag.equals("FP_GREATER_THAN_EQUAL") || flag.equals("FP_LESS_THAN_EQUAL") ||
				   flag.equals("FP_NOT_EQUAL") || flag.equals("FP_TO_INT") || flag.equals("INT_TO_FP") ||
				   flag.equals("FP_TO_FP"))// || (flag.equals("LoopFusion") && isLastFlag))
					continue;
				
				setValues(flag, value1, value2);
				TableItem item = new TableItem(selectedFlags, SWT.NONE);
				if(v.size() != 0 &&
				   v.get(v.size() - 1).getArgs() != null &&
				   v.get(v.size() - 1).getArgs().length > 1 &&
				   v.get(v.size() - 1).getArgs()[1] != null &&
				   v.get(v.size() - 1).getArgs()[1] != "")
				{
					String labelArg = "";
					if(v.get(v.size() - 1).getFlag().equals("LoopInterchange"))
						labelArg = new String(v.get(v.size() - 1).getArgs()[1] + " <-> " + v.get(v.size() - 1).getArgs()[3]);
					else
						labelArg = new String(v.get(v.size() - 1).getArgs()[1]);
					item.setText(new String[]{flag,  labelArg});
				}
				else
					item.setText(new String[]{flag, ""});
			}
			catch (Exception e) 
			{
				e.printStackTrace();
				return;
			}
		}
	}	
	
	public void createControl(Composite parent) 
	{
		Composite top = createDefaultComposite(parent,1);
		setControl(top);
		this.setPageComplete(false);

		Group group1 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group1.setText("High Level Compile Optimizations");
		group1.setLayout(new GridLayout());
		group1.setLayoutData(createNewGD(0));
		Composite composite1 = createDefaultComposite(group1,4);
		Composite cp12 = createDefaultComposite(group1, 1);
		
		createFlagListControl(composite1);
		createFlagListButtonsControl(composite1);
		createSelectedFlagListControl(composite1);
		createSelectedFlagsButtonsControl(composite1);
		createDescriptionControl(cp12);
		
		Group group2 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group2.setText("Arguments for Selected High Level Optimizations");
		group2.setLayout(new GridLayout());
		group2.setLayoutData(createNewGD(GridData.FILL_VERTICAL));
		Composite composite2 = createDefaultComposite(group2,1);
		
		createFlagArgumentsControl(composite2);
	
		createDefaultsButtons(top);
		
		flagListComp.notifyListeners(SWT.Resize, null);
		selectedFlagListComp.notifyListeners(SWT.Resize, null);
		
	}

	private void addFlag(String name)
	{
		new TableItem(flagList, SWT.NONE).setText(new String(name));
		acceptedFlags.add(name);
	}
	
	private void createDefaultsButtons(Composite parent)
	{
		Composite defaultsComp = CompositeUtilities.createDefaultComposite(parent, 3, false);
		new Label(parent, SWT.NONE);
		
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
				loadDefaultOptimizations();
			}
		});
	}
	
	private void createFlagListControl(Composite parent)
	{	
		acceptedFlags = new Vector<String>();
		
		flagListComp = createDefaultComposite(parent, 1);
		flagList = new Table(flagListComp, SWT.FULL_SELECTION | SWT.BORDER);
		flagList.setHeaderVisible(true);
		flagList.setLinesVisible(true);
		flagList.setLayoutData(createNewGD(GridData.FILL_BOTH));

		TableColumn tableColumn = new TableColumn(flagList, SWT.NONE);
		tableColumn.setWidth(275);
		tableColumn.setMoveable(false);
		tableColumn.setResizable(false);
		tableColumn.setText("Available Optimizations");
		
		//Do additions used by both modules and systems.
		
		addFlag("DivisionByConstElimination");
	
		if(componentType.equals("SYSTEM"))
		{
			addFlag("FullyUnroll");
			addFlag("LoopFusion");		
			addFlag("LoopInterchange");
			addFlag("LoopUnrolling");
		}
		
		addFlag("InlineModule");
		addFlag("MultiplyByConstElimination");
		addFlag("Redundancy");
		
		//Do additions for module specific compiler flags.
		if(componentType.equals("SYSTEM"))
		{
			addFlag("SystolicArrayGeneration");
			addFlag("TemporalCommonSubExpressionElimination");			
		}
		
		flagList.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e)
			{
			}

			public void widgetSelected(SelectionEvent e) 
			{
				handleEditor();
				String flag = flagList.getItem(flagList.getSelectionIndex()).getText().toString();
				title.setText(flag + ":");
				description.setText(getDescription(flag));
				description2.setText(getDescription2(flag));
				title.setSize(description.computeSize(SWT.DEFAULT, SWT.DEFAULT));
				description.setSize(description.computeSize(SWT.DEFAULT, SWT.DEFAULT));
				description2.setSize(description.computeSize(SWT.DEFAULT, SWT.DEFAULT));
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
			    	  GridData d2 = createNewGD(0);
			    	  d2.heightHint = trim2.height;
			    	  flagList.setLayoutData(d2);
			    	  flagListComp.setSize(flagListComp.getSize().x, flagList.getItemHeight() * 8);
			    	  //flagListButtonsComp.setSize(flagListButtonsComp.getSize().x, flagList.getItemHeight() * 11);
			      }
		    }
		  });
	}
	
	private void createFlagListButtonsControl(Composite parent)
	{
		flagListButtonsComp = createDefaultComposite(parent, 1);
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
				
				if(selectedFlags.getItemCount() > 0 && 
				   (flagList.getItem(flagList.getSelectionIndex()).getText().toString().compareTo("SystolicArrayGeneration") == 0 ||
							selectedFlags.getItem(0).getText().toString().compareTo("SystolicArrayGeneration") == 0))
				{
					if(MessageDialog.openQuestion(new Shell(), "Flag Selection", "SystolicArrayGeneration cannot be combined with other optimizations.\n\n" +
																					"Erase all selected optimizations and insert this one?") == true)
					{
						v.clear();
						selectedFlags.removeAll();
					}
					else
						return;
				}
				
				if(selectedFlags.getItemCount() > 0 && 
				   flagList.getItem(flagList.getSelectionIndex()).getText().toString().compareTo("Export") == 0)
				{
					for(int i = 0; i < v.size(); ++i)
					{
						if(v.get(i).getFlag().compareTo("Export") == 0)
						{
							//It did not start with an alphabetic character.
							MessageDialog.openInformation(new Shell(), "Optimization Selection Error", "\"Export\" has already been added to the selected optimizations list.");
							return;
						}
					}
				}
				
				if(selectedFlags.getItemCount() > 0 &&
				   flagList.getItem(flagList.getSelectionIndex()).getText().toString().equals("FullyUnroll"))
				{
					for(int i = 0; i < v.size(); ++i)
					{
						if(v.get(i).getFlag().compareTo("FullyUnroll") == 0)
						{
							//It did not start with an alphabetic character.
							MessageDialog.openInformation(new Shell(), "Optimization Selection Error", "\"FullyUnroll\" has already been added to the selected optimizations list.");
							return;
						}
						if(v.get(i).getFlag().compareTo("LoopUnrolling") == 0)
						{
							boolean replace = MessageDialog.openQuestion(new Shell(), "Optimization Selection", "You already have the optimization \"LoopUnrolling\" being used, replace those with \"FullyUnroll\"?");
							if(replace)
							{
								for(int j = i; j < v.size(); ++j)
								{
									if(v.get(j).getFlag().equals("LoopUnrolling"))
									{
										v.remove(j);
										selectedFlags.remove(j);
										--j;
									}
								}
								break;
							}
							else
								return;
						}
					}
				}
				
				if(selectedFlags.getItemCount() > 0 &&
				   flagList.getItem(flagList.getSelectionIndex()).getText().toString().equals("LoopUnrolling"))
				{
					for(int i = 0; i < v.size(); ++i)
					{
						if(v.get(i ).getFlag().compareTo("FullyUnroll") == 0)
						{
							//It did not start with an alphabetic character.
							boolean replace = MessageDialog.openQuestion(new Shell(), "Optimization Selection", "\"FullyUnroll\" has already been added to the selected optimizations list. Replace that with \"LoopUnrolling\"?");
							
							if(replace)
							{
								v.remove(i);
								selectedFlags.remove(i);
								break;
							}
							else
								return;
						}
					}
				}
				
				TableItem item = new TableItem(selectedFlags, SWT.NONE);
				String name = new String (flagList.getItem(flagList.getSelectionIndex()).getText().toString());
				getDefaultValues(flagList.getItem(flagList.getSelectionIndex()).getText().toString());
				
				if(v.get(v.size() - 1).getArgs() != null &&		
				   v.get(v.size() - 1).getArgs().length > 1 &&
				   v.get(v.size() - 1).getArgs()[1] != null &&
				   v.get(v.size() - 1).getArgs()[1] != "")
				{
					String labelArg = "";
					if(v.get(v.size() - 1).getFlag().equals("LoopInterchange"))
						labelArg = new String(v.get(v.size() - 1).getArgs()[1] + " <-> " + v.get(v.size() - 1).getArgs()[3]);
					else
						labelArg = new String(v.get(v.size() - 1).getArgs()[1]);
					item.setText(new String[]{name, labelArg});
				
				}	
				else
					item.setText(new String[]{name, ""});
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
				v.remove(selectedFlags.getSelectionIndex());
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
	
	private void createSelectedFlagListControl(Composite parent)
	{
	    selectedFlagListComp = createDefaultComposite(parent, 1);
		selectedFlags = new Table(selectedFlagListComp,SWT.FULL_SELECTION | SWT.BORDER);
		selectedFlags.setHeaderVisible(true);
		selectedFlags.setLinesVisible(true);

		TableColumn tableColumn = new TableColumn(selectedFlags, SWT.NONE);
		tableColumn.setWidth(260);
		tableColumn.setMoveable(true);
		tableColumn.setText("Selected Flags");
			
		TableColumn argColumn = new TableColumn(selectedFlags, SWT.NONE);
		argColumn.setWidth(125);
		argColumn.setMoveable(true);
		argColumn.setText("Label Arg");
		
		selectedFlags.addSelectionListener(new SelectionListener()
		{
			public void widgetSelected(SelectionEvent event) 
			{
				handleEditor();
				arguments.removeAll();
				for(int i = 0; i < v.get(selectedFlags.getSelectionIndex()).getArgs().length; i += 2)
				{
					TableItem item = new TableItem(arguments, SWT.NONE);
					String labelArg = new String();
					item.setText(new String[] {v.get(selectedFlags.getSelectionIndex()).getArgs()[i], v.get(selectedFlags.getSelectionIndex()).getArgs()[i + 1]});
				}
			}
			public void widgetDefaultSelected(SelectionEvent event)
			{
				widgetSelected(event);
				
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
				GridData d = createNewGD(0);
				d.heightHint = trim.height;
				selectedFlags.setLayoutData(d);
				selectedFlagListComp.setSize(selectedFlagListComp.getSize().x, selectedFlags.getItemHeight() * 8);
				//flagListButtonsComp.setSize(flagListButtonsComp.getSize().x, selectedFlags.getItemHeight() * 11);
		      }
		    }
		  });

		
		loadExistingFlags();
		//selectedFlagListComp.setLayoutData(new RowData(275, 300));
		//selectedFlags.setBounds(0, 0, selectedFlags.getBounds().width, 300);
	}
	
	private void createSelectedFlagsButtonsControl(Composite parent)
	{
		Composite selectedFlagsButtonsComp = CompositeUtilities.createDefaultComposite(parent, 1, false);
		
		new Label(selectedFlagsButtonsComp, SWT.NONE).setText("");
		
		
		Button up = new Button(selectedFlagsButtonsComp, SWT.ARROW | SWT.UP);
		up.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
		
		new Label(selectedFlagsButtonsComp, SWT.NONE).setText("");
		new Label(selectedFlagsButtonsComp, SWT.NONE).setText("");
		
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
						
						FlagData fd = v.get(selectedFlags.getSelectionIndex());
						fd = new FlagData(fd.getFlag(), fd.getArgs());
						v.set(selectedFlags.getSelectionIndex(), v.get(selectedFlags.getSelectionIndex() - 1));
						v.set(selectedFlags.getSelectionIndex() - 1, fd);
						
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
						
						FlagData fd = v.get(selectedFlags.getSelectionIndex());
						fd = new FlagData(fd.getFlag(), fd.getArgs());
						v.set(selectedFlags.getSelectionIndex(), v.get(selectedFlags.getSelectionIndex() + 1));
						v.set(selectedFlags.getSelectionIndex() + 1, fd);
						
						selectedFlags.setSelection(selectedFlags.getSelectionIndex() + 1);
					}
				}
			}
		});
	}
	
	private void createDescriptionControl(Composite parent)
	{
		//Composite selectedFlagListButtonsComp = createDefaultComposite(parent, 1);
		
		Composite selectedFlagListButtonsComp = new Composite(parent, SWT.NONE);
		GridLayout layout = new GridLayout();
		layout.numColumns = 1;
		selectedFlagListButtonsComp.setLayout(layout);
		selectedFlagListButtonsComp.setLayoutData(createNewGD(GridData.GRAB_HORIZONTAL | GridData.GRAB_VERTICAL| GridData.HORIZONTAL_ALIGN_FILL | GridData.VERTICAL_ALIGN_FILL | SWT.FILL));
		
		title = new Label(selectedFlagListButtonsComp, SWT.HORIZONTAL | SWT.LEFT);
		description = new Label(selectedFlagListButtonsComp, SWT.HORIZONTAL | SWT.LEFT);
		description2 = new Label(selectedFlagListButtonsComp, SWT.HORIZONTAL | SWT.LEFT);
		title.setText("");
		description.setText("");
		description2.setText("");
		Color c = new Color(null, 0, 0, 255);
		
		title.setForeground(c);
		description.setSize(description.computeSize(SWT.DEFAULT, SWT.DEFAULT));
		description2.setSize(description.computeSize(SWT.DEFAULT, SWT.DEFAULT));
	}
	
	private void createFlagArgumentsControl(Composite parent)
	{
		Composite argumentsComp = createDefaultComposite(parent, 2);
		
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
					v.get(selectedIndex).getArgs()[selectedRow * 2 + 1] = new String(text.getText());
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
					optimization = v.get(selectedFlags.getSelectionIndex()).getFlag();//selectedFlags.getItem(selectedFlags.getSelectionIndex()).getText().toString();
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
	
	private void createStage2Control(Composite parent)
	{
		Composite left = CompositeUtilities.createDefaultComposite(parent, 2, false);
		Composite middle = CompositeUtilities.createDefaultComposite(parent, 2, false);
		Composite right = CompositeUtilities.createDefaultComposite(parent, 2, false);
		
		String nextAreaToAddTo = new String("left");
		
		nextAreaToAddTo = addCheckMarkArea(left, middle, right, nextAreaToAddTo, "Maintain Precision");
		nextAreaToAddTo = addCheckMarkArea(left, middle, right, nextAreaToAddTo, "Arithmetic Balancing");
		nextAreaToAddTo = addCheckMarkArea(left, middle, right, nextAreaToAddTo, "Copy Reduction");
	}
	
	private String addCheckMarkArea(Composite left, Composite middle, Composite right, String nextAreaToAddTo, String text)
	{
		if(nextAreaToAddTo.equals("left"))
		{
			System.out.println("left");
			new Label(left, SWT.NONE).setText(text);
			new Button(left, SWT.CHECK);	
			nextAreaToAddTo = new String("middle");
		}
		else if(nextAreaToAddTo.equals("middle"))
		{
			System.out.println("m");
			new Label(middle, SWT.NONE).setText(text);
			new Button(middle, SWT.CHECK);
			nextAreaToAddTo = new String("right");
		}
		else
		{
			System.out.println("r");
			new Label(right, SWT.NONE).setText(text);
			new Button(right, SWT.CHECK);
			nextAreaToAddTo = new String("left");
		}
		
		return nextAreaToAddTo;
	}
	
	private void handleEditor()
	{
		try
		{
			if(v == null)
				return;
			if(editor == null)
				return;
			if(editor.getEditor() == null)
				return;
			if(editor.getEditor().isDisposed())
				return;
			if(selectedIndex == -1)
				return;
			if(v.size() == 0)
			{
				editor.getEditor().dispose() ;
				return ;
			}
			if(v.get(selectedIndex) == null)
				return;
			String[] newStrings;
			newStrings = v.get(selectedIndex).getArgs();
			if(newStrings == null)
				return;
			if(selectedRow * 2 + 1 > newStrings.length - 1)
				return;
			if(editor.getItem() == null || editor.getItem().getText(1) == null)
				return;
			newStrings[selectedRow * 2 + 1] = new String(editor.getItem().getText(1));
			v.removeElementAt(selectedIndex);
			v.add(selectedIndex, new FlagData(optimization, newStrings));
			
			
			if(newStrings.length > 1 && newStrings[1] != null && newStrings[1] != "")
			{
				String labelArg = new String();
				if(v.get(selectedIndex).getFlag().equals("LoopInterchange"))
					labelArg = new String(newStrings[1] + " <-> " + newStrings[3]);
				else
					labelArg = new String(newStrings[1]);
				selectedFlags.getItem(selectedIndex).setText(new String[]{v.get(selectedIndex).getFlag(), labelArg});
			}
			else
			{
				selectedFlags.getItem(selectedIndex).setText(new String[]{v.get(selectedIndex).getFlag(), ""});
			}
			editor.getEditor().dispose();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void createFlagDescriptionControl(Composite parent)
	{
		//Composite descriptionComp = createDefaultComposite(parent, 1);
		
	}
	
	private boolean validInputs(Text portName, Text portSize)
	{	
		return true;
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
		Composite composite = new Composite(parent, SWT.NONE);
		GridLayout layout = new GridLayout();
		layout.numColumns = numCols;
		layout.marginHeight = 2;
		composite.setLayout(layout);
		composite.setLayoutData(createNewGD(0));
		return composite;
	}*/
}
