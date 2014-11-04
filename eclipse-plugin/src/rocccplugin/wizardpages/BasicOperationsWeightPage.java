package rocccplugin.wizardpages;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;
import java.util.Vector;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Scale;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

import rocccplugin.Activator;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.CompositeUtilities;
import rocccplugin.utilities.PreferenceUtils;
import rocccplugin.utilities.StringUtils;

public class BasicOperationsWeightPage extends WizardPage 
{
	public Map<String, Text> textBoxes;
	File fileToCompile;
	Map<String, String> keyToPreference;
	
	Composite advancedGroup;
	Composite basicGroup;
	
	Button basic;
	Button advanced;
	
	public float opsPerStage = 1;
	
	final int maxSliderValue = 50;
	Scale scale;
	
	public BasicOperationsWeightPage(String pageName, File fileToCompile) 
	{
		super(pageName);
		textBoxes = new TreeMap<String, Text>();
		
		setTitle("Frequency vs Area Tuning");
		setDescription("Set the values of the operations and fanout to tune the hardware to the target platform.");
	
		this.fileToCompile = fileToCompile;
		this.setPageComplete(true);
		
		keyToPreference = new TreeMap<String, String>();
		keyToPreference.put("Add", PreferenceConstants.ADD_WEIGHT);
		keyToPreference.put("Copy", PreferenceConstants.COPY_WEIGHT);
		keyToPreference.put("Sub", PreferenceConstants.SUB_WEIGHT);
		keyToPreference.put("Shift", PreferenceConstants.SHIFT_WEIGHT);
		keyToPreference.put("Mult", PreferenceConstants.MULT_WEIGHT);
		keyToPreference.put("AND", PreferenceConstants.AND_WEIGHT);
		keyToPreference.put("OR", PreferenceConstants.OR_WEIGHT);
		keyToPreference.put("Compare", PreferenceConstants.COMPARE_WEIGHT);
		keyToPreference.put("XOR", PreferenceConstants.XOR_WEIGHT);
		keyToPreference.put("Mux", PreferenceConstants.MUX_WEIGHT);
		
		//keyToPreference.put("Max Weight Per Cycle", PreferenceConstants.MAX_CYCLE_WEIGHT);
		keyToPreference.put("Max Unregistered Fanout", PreferenceConstants.MAX_FANOUT);
		keyToPreference.put("OperationsPerPipelineStage", PreferenceConstants.OPS_PER_PIPELINE_STAGE);
	}
	
	private void saveValuesAsDefaults()
	{
		try
		{
			Iterator<String> i = textBoxes.keySet().iterator();
			
			String defaultWeights = new String(); 
			
			int max = 0;
			int maxWeightPerCycle = 0;
			int maxFanout = 0;
			
			for(String key = i.next(); key != null ; key = (i.hasNext()? i.next() : null))
			{	
				if(textBoxes.get(key).getText().length() == 0)
				{
					MessageDialog.openError(new Shell(), "Weight Error", "The operation \"" + key + "\" has no weight value. All weights must be a positive integer.");
					return;
				}
				
				if(!StringUtils.isPositiveInt(textBoxes.get(key).getText()))
				{
					MessageDialog.openError(new Shell(), "Weight Error", "The operation \"" + key + "\" has an invalid weight. All weights must be a positive integer.");
					return;
				}
				
				int num = Integer.parseInt(textBoxes.get(key).getText());
				
				if(num == 0)
				{
					MessageDialog.openError(new Shell(), "Weight Error", "The operation \"" + key + "\" has a weight of zero. All weights must be a positive integer.");
					return;
				}
				
				if(Integer.parseInt(textBoxes.get(key).getText()) > max && !key.equals("Max Weight Per Cycle") && !key.equals("Max Unregistered Fanout"))
					max = Integer.parseInt(textBoxes.get(key).getText());
				if(Integer.parseInt(textBoxes.get(key).getText()) > maxWeightPerCycle && key == "Max Weight Per Cycle")
					maxWeightPerCycle = Integer.parseInt(textBoxes.get(key).getText());
			}
			
			/*if(max > maxWeightPerCycle)
			{
				MessageDialog.openError(new Shell(), "Weight Error", "The Max Weight Per Cycle value must be greater or equal to all other operation weight.");
				return;
			}*/
			
			int index = 0;
			
			i = textBoxes.keySet().iterator();
			
			for(String key = i.next(); key != null ; key = (i.hasNext()? i.next() : null), ++index)
			{
				Activator.getDefault().getPreferenceStore().setValue(keyToPreference.get(key), textBoxes.get(key).getText().toString());
			}
			
			if(keyToPreference.containsKey("OperationsPerPipelineStage"))
				Activator.getDefault().getPreferenceStore().setValue(keyToPreference.get("OperationsPerPipelineStage"), "" + opsPerStage);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private void restoreDefaults()
	{
		try
		{
			Iterator<String> i = textBoxes.keySet().iterator();
			
			int index = 0;
			
			for(String key = i.next(); key != null ; key = (i.hasNext()? i.next() : null), ++index)
			{
				if(textBoxes.containsKey(key) && keyToPreference.containsKey(key))
				{
					textBoxes.get(key).setText(PreferenceUtils.getPreferenceString(keyToPreference.get(key)));
				}
			}
			
			//Set the slider default value
			float ops = Float.parseFloat(PreferenceUtils.getPreferenceString(keyToPreference.get("OperationsPerPipelineStage")));
			int sliderValue = (int) (maxSliderValue - maxSliderValue / ops);
			scale.setSelection(sliderValue);
			scale.notifyListeners(SWT.Selection, null);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}

	public void createControl(Composite parent) 
	{
		Composite top = CompositeUtilities.createDefaultComposite(parent,1, false);
		setControl(top);
		
		
		//Create the tabbing buttons
		Group group = new Group(top, SWT.SHADOW_ETCHED_IN);
		group.setText("Mode:");
		group.setLayout(new GridLayout());
		GridData gd0 = CompositeUtilities.createNewGD(0);
		gd0.exclude = false;
		group.setLayoutData(gd0);
		
		createModeButtons(group);
		
		//Create the basic controls
		
		Group sliderGroup = new Group(top, SWT.SHADOW_ETCHED_IN);
		sliderGroup.setText("Pipelining Amount:");
		sliderGroup.setLayout(new GridLayout());
		sliderGroup.setLayoutData(CompositeUtilities.createNewGD(0));
		
		createSliderArea(sliderGroup);
		
		
		//Create the advanced controls
		advancedGroup = CompositeUtilities.createDefaultComposite(top, 1, false);
		
		Group group1 = new Group(advancedGroup, SWT.SHADOW_ETCHED_IN);
		group1.setText("Basic Operation Weights");
		group1.setLayout(new GridLayout());
		GridData gd = CompositeUtilities.createNewGD(0);
		gd.exclude = false;
		group1.setLayoutData(gd);
		
		//Create weights
		createOperationBoxes(group1);
		
		//Create group for max weight per clockcycle
		new Label(group1, SWT.NONE).setText("");
		createMaxCycles(group1);
		createFanoutArea(group1);
		createDefaultsButtons(top);
		
		
		basic.setSelection(true);
		basic.notifyListeners(SWT.Selection, null);
		
		loadValues();
	}
	
	private void loadValues()
	{
		try
		{
			restoreDefaults();
			
			String folderString = fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "");
			File timingInfoFile = new File(folderString + ".ROCCC/" + ".timingInfo");
		
			if(!timingInfoFile.exists() || !timingInfoFile.canRead())
				return;
			
			Vector<String> operations;
			Vector<String> values;
			
			operations = new Vector<String>();
			values = new Vector<String>();
			
			StringBuffer buf = new StringBuffer();
			try
			{
				FileInputStream fis = new FileInputStream(timingInfoFile.getAbsolutePath());
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
			
			while(buf.length() > 0)
			{
				String key = StringUtils.getNextStringValue(buf);
				
				key = key.equals("DesiredTiming")? "Max Weight Per Cycle" : key;
				key = key.equals("MaxFanoutRegistered")? "Max Unregistered Fanout" : key;
				key = key.equals("MaxFanout")? "Max Unregistered Fanout" : key;
				
				if(keyToPreference.containsKey(key) == false)
					continue;
				
				operations.add(key);
				values.add(StringUtils.getNextStringValue(buf));
			}
			
			for(int i = 0; i < operations.size(); ++i)
			{
				String key = operations.get(i);
				key = operations.get(i).equals("DesiredTiming")? "Max Weight Per Cycle" : key;
				key = operations.get(i).equals("MaxFanoutRegistered")? "Max Unregistered Fanout" : key;
				key = operations.get(i).equals("MaxFanout")? "Max Unregistered Fanout" : key;
				
				if(key.equals("OperationsPerPipelineStage"))
				{
					//Calculate what the slider value should be from the loaded operations per pipeline stage number.
					float ops = Float.parseFloat(values.get(i));
					int sliderValue = ops == maxSliderValue * 1000? maxSliderValue : (int) (maxSliderValue - maxSliderValue / ops);
					scale.setSelection(sliderValue);
					scale.notifyListeners(SWT.Selection, null);
				}
				else if(keyToPreference.containsKey(key))
					textBoxes.get(key).setText(values.get(i));
				
			}
		}
		catch(Exception e1)
		{
			e1.printStackTrace();
		}
		
	}
	
	private void createModeButtons(Composite parent)
	{
		Composite comp = CompositeUtilities.createDefaultComposite(parent, 5, false);
		
		new Label(comp, SWT.NONE).setText("Basic:");
		basic = new Button(comp, SWT.RADIO);
		
		new Label(comp, SWT.NONE).setText(" ");
		
		new Label(comp, SWT.NONE).setText("Advanced:");
		advanced = new Button(comp, SWT.RADIO);
	
		basic.setSelection(true);
		
		basic.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{	
			}
			
			public void widgetSelected(SelectionEvent e) 
			{		
				GridData d = (GridData) advancedGroup.getLayoutData();
				//d.exclude = true;
				advancedGroup.setVisible(false);
				advancedGroup.layout(false);
			}
		});
		
		advanced.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{	
			}
			
			public void widgetSelected(SelectionEvent e) 
			{
				GridData d = (GridData) advancedGroup.getLayoutData();
				//d.exclude = false;
				advancedGroup.setVisible(true);
				advancedGroup.layout(false);
			}
		});
	}
	
	private void createSliderArea(Composite parent)
	{
		Composite sliderComp = CompositeUtilities.createDefaultComposite(parent, 7, false);
		
		//Create the Pipeline image on the left side of the slider
		new Label(sliderComp, SWT.NONE).setText(" ");
		Label pipelined = new Label(sliderComp, SWT.NONE);
		pipelined.setImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/pipelined.png")));
		
		final int arrowStepSize = 1;
		
		Button left = new Button(sliderComp, SWT.PUSH | SWT.ARROW_LEFT);
		left.setText("<");
		left.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
			}
			
			public void widgetSelected(SelectionEvent e) 
			{
				int sel = scale.getSelection();
				sel = sel % arrowStepSize == 0? sel - arrowStepSize : sel / arrowStepSize * arrowStepSize;	
				scale.setSelection(Math.max(0, sel));
				scale.notifyListeners(SWT.Selection, null);
			}
			
		});
		
		//Create the slider with its necessary values.
		scale = new Scale(sliderComp, SWT.HORIZONTAL);
		
		scale.setMinimum(0);
		scale.setMaximum(maxSliderValue);
		scale.setIncrement(1);
		scale.setPageIncrement(1);
		
		scale.setLayoutData(CompositeUtilities.createNewGD(0));
		
		Button right = new Button(sliderComp, SWT.PUSH | SWT.ARROW_RIGHT);
		right.setText(">");
		right.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
			}
			
			public void widgetSelected(SelectionEvent e) 
			{
				int sel = scale.getSelection();
				sel = sel % arrowStepSize == 0? sel + arrowStepSize : sel / arrowStepSize * arrowStepSize + arrowStepSize;	
				scale.setSelection(Math.min(maxSliderValue, sel));
				scale.notifyListeners(SWT.Selection, null);
			}
			
		});
		//right.setLayoutData(CompositeUtilities.createNewGD(0));
		
		//Create the Compacted image on the right side of the slider.
		Label compacted = new Label(sliderComp, SWT.NONE);
		compacted.setImage(new Image(null, this.getClass().getResourceAsStream("/rocccplugin/images/compacted.png")));
		new Label(sliderComp, SWT.NONE).setText(" ");
		
		//Create the description location that will tell how many operations we will try to bundle per pipeline stage.
		final Label description = new Label(parent, SWT.NONE);
		description.setText("This will attempt to put on average 1.0 operation per pipeline stage.");
		
		scale.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
			}

			public void widgetSelected(SelectionEvent e) 
			{
				//Calculate the operations per pipeline stage the slider represents.
				float value = Math.max(maxSliderValue - scale.getSelection(), 0.001f) / (float)maxSliderValue;
				float opsPerPipelineStage = value == 1.0f? 1: (float)((1f / value));
								
				//This line is here because of a float rounding error.
				if(maxSliderValue == scale.getSelection())
					opsPerPipelineStage = maxSliderValue * 1000;
				
				//Text t = textBoxes.get("Average Operations Per Pipeline Stage");
				//t.setText("" + opsPerPipelineStage);

				if(scale.getSelection() == maxSliderValue)
					description.setText("This will attempt to put all operations into a single pipeline stage."); 
				else if(value == 1f)
					description.setText("This will attempt to put on average 1.0 operation per pipeline stage."); 
				else
					description.setText("This will attempt to put on average " + opsPerPipelineStage + " operations per pipeline stage."); 
				
				description.setLayoutData(CompositeUtilities.createNewGD(0));
				
				opsPerStage = opsPerPipelineStage;
			}
			
		});
	}
	
	private void createFanoutArea(Composite parent)
	{
		Composite fanoutComp = CompositeUtilities.createDefaultComposite(parent, 5, false);
		
		createTextField(fanoutComp, "Max Unregistered Fanout");
	}
	
	private void createMaxCycles(Composite parent)
	{
		//Composite boxesComp = CompositeUtilities.createDefaultComposite(parent, 5, false);
	
		//createTextField(boxesComp, "Average Operations Per Pipeline Stage");
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
					saveValuesAsDefaults();
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
				restoreDefaults();
			}
		});
	}
	
	private void createOperationBoxes(Composite parent)
	{
		Composite spacer = CompositeUtilities.createDefaultComposite(parent, 14, false);
		Composite boxesComp = CompositeUtilities.createDefaultComposite(spacer, 2, false);
		Composite blank = CompositeUtilities.createDefaultComposite(spacer, 1, false);
		Composite boxesComp2 = CompositeUtilities.createDefaultComposite(spacer, 2, false);
		Composite blank2 = CompositeUtilities.createDefaultComposite(spacer, 1, false);
		Composite boxesComp3 = CompositeUtilities.createDefaultComposite(spacer, 2, false);
		Composite blank3 = CompositeUtilities.createDefaultComposite(spacer, 1, false);
		Composite boxesComp4 = CompositeUtilities.createDefaultComposite(spacer, 2, false);
		Composite blank4 = CompositeUtilities.createDefaultComposite(spacer, 1, false);
		Composite boxesComp5 = CompositeUtilities.createDefaultComposite(spacer, 2, false);
		
		
		createTextField(boxesComp, "Add");
		createTextField(boxesComp, "Copy");
		createTextField(boxesComp2, "Sub");
		createTextField(boxesComp2, "Shift");
		createTextField(boxesComp3, "Mult");
		createTextField(boxesComp3, "AND");
		createTextField(boxesComp4, "OR");
		createTextField(boxesComp4, "Mux");
		createTextField(boxesComp5, "Compare");
		createTextField(boxesComp5, "XOR");
		
		blank.pack();
		blank2.pack();
		blank3.pack();
		
		
		new Label(blank, SWT.HORIZONTAL).setText(" ");
	}
	
	private void leaveGap(Composite parent)
	{
		new Label(parent, SWT.HORIZONTAL).setText(" ");
	}
	
	private void createTextField(Composite parent, String text)
	{
		try
		{
			Label l = new Label(parent, SWT.NONE);
			l.setText(text);
			textBoxes.put(text, new Text(parent, SWT.BORDER));
			textBoxes.get(text).setText("1");
			Text t = textBoxes.get(text);
			
			t.setSize(100, SWT.DEFAULT);
	        Rectangle trim = t.computeTrim(0, 0, 70, SWT.DEFAULT);
			GridData d = CompositeUtilities.createNewGD(0, false, true, SWT.CENTER);
			d.widthHint = trim.width;
			t.setLayoutData(d);
	
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}	

	private int getMaxWeight()
	{
		Iterator<String> i = textBoxes.keySet().iterator();
		
		int max = 0;
		
		for(String key = i.next(); key != null ; key = (i.hasNext()? i.next(): null))
		{
			String value = textBoxes.get(key).getText();
			
			int num = Integer.parseInt(textBoxes.get(key).getText());
			
			if(!StringUtils.isPositiveInt(value))
			{
				continue;
			}	
			
			if(key.equals("Max Weight Per Cycle"))
				;
			else if(num > max && !key.equals("Max Unregistered Fanout"))
					max = num;
		}
		
		return max;
	}
	
	private float getWeightAverage()
	{
		Iterator<String> i = textBoxes.keySet().iterator();
		
		int total = 0;
		int count = 0;
		
		for(String key = i.next(); key != null ; key = (i.hasNext()? i.next(): null))
		{
			String value = textBoxes.get(key).getText();
			
			int num = Integer.parseInt(textBoxes.get(key).getText());
			
			if(!StringUtils.isPositiveInt(value))
			{
				continue;
			}	
			
			if(!key.equals("Max Weight Per Cycle") && !key.equals("Max Unregistered Fanout"))
			{
				total += num;
				++count;
			}
				
		}
		
		return (float)total / count;
	}
}
