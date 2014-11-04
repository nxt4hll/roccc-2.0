package rocccplugin.wizards;

import java.io.File;
import java.util.Map;
import java.util.TreeMap;
import java.util.Vector;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.FileUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.StringUtils;
import rocccplugin.wizardpages.GenerateTestbenchWizardPage;

public class GenerateTestbenchWizard extends Wizard implements INewWizard 
{
	GenerateTestbenchWizardPage wizardPage;
	String componentName;
	public File fileToGenerateTestbenchFor;
	public String[] scalarNames;
	public Vector<Vector<String>> scalarValues;
	public String[] outputScalars;
	public String[] inputTestFiles;
	public String[] outputTestFiles;
	public String[] relativeSafeInputTestFiles;
	public String[] relativeSafeOutputTestFiles;
	public Map<String, String> clocks;
	public Vector<Vector<String>> outputScalarValues;
	public String systemTestValuesAvailable;
	boolean isSystem;
	
	public GenerateTestbenchWizard(String compName, File file, boolean isSystem)
	{
		componentName = compName;
		fileToGenerateTestbenchFor = file;
		this.isSystem = isSystem;
	}
	
	public void addPages()  
	{
		File testbenchInfoFile = new File(FileUtils.getFolderOfFile(fileToGenerateTestbenchFor) + ".ROCCC/.testbenchInfo");
		wizardPage = new GenerateTestbenchWizardPage("Testbench Generation for " + componentName, componentName, fileToGenerateTestbenchFor, testbenchInfoFile, isSystem);
		addPage(wizardPage);
		this.setDefaultPageImageDescriptor(ImageDescriptor.createFromImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/testbench.png"))));
	}
	
	private boolean check2DMemoryMappingFile(String file)
	{
		try
		{
			int[] dimensions = new int[2];
			StringBuffer fileContents = new StringBuffer();
			FileUtils.addFileContentsToBuffer(fileContents, file);
			
			String line = new String(" ");
			String value = new String(" ");
			
			int maxWidth = -1;
			
			line = StringUtils.getNextLine(fileContents);
			while(!line.equals(""))
			{
				dimensions[0] = 0;
				StringBuffer lineBuffer = new StringBuffer(line);
				value = StringUtils.getNextStringValue(lineBuffer);
				while(!value.equals(""))
				{
					if(!StringUtils.isANumber(value) && !StringUtils.isAHexValue(value))
					{
						MessageDialog.openError(new Shell(), "Output File Error", "The value \"" + value + "\" in test file \"" + file + "\" is not a valid number or hex value.");
						return false;
					}
					
					++dimensions[0];
					value = StringUtils.getNextStringValue(lineBuffer);
				}
				
				if(maxWidth == -1)
					maxWidth = dimensions[0];
				else if(maxWidth != dimensions[0])
				{
					MessageDialog.openError(new Shell(), "Output File Error", "The width in test file \"" + file + "\" is not consistant for all the rows of values.");
					return false;
				}	
				++dimensions[1];
				line = StringUtils.getNextLine(fileContents);
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return true;
	}
	
	private boolean check1DMemoryMappingFile(String file)
	{
		int[] dimensions = new int[2];
		StringBuffer fileContents = new StringBuffer();
		FileUtils.addFileContentsToBuffer(fileContents, file);
		
		String value = new String(" ");
		
		dimensions[0] = 0;
		value = StringUtils.getNextStringValue(fileContents);
		while(!value.equals(""))
		{
			if(!StringUtils.isANumber(value) && !StringUtils.isAHexValue(value))
			{
				MessageDialog.openError(new Shell(), "Test File Error", "The value \"" + value + "\" in test file \"" + file + "\" is not a valid number or hex value.");
				return false;
			}
			
			++dimensions[0];
			value = StringUtils.getNextStringValue(fileContents);
		}
		
		return true;
	}
	
	private boolean checkTestFileForCorrectness(String file, String stream, boolean outputStream)
	{
		try
		{
			if(outputStream)
			{
				return check1DMemoryMappingFile(file);
			}
			else
			{
				return check1DMemoryMappingFile(file);
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		return true;
	}
	
	public String[] getRelativeInputStreamTestFiles()
	{
		return relativeSafeInputTestFiles;
	}
	
	public String[] getRelativeOutputStreamTestFiles()
	{
		return relativeSafeOutputTestFiles;
	}
	
	public boolean performFinish() 
	{
		try
		{
			scalarNames = wizardPage.scalarNames;
			scalarValues = wizardPage.scalarValues;
			outputScalars = wizardPage.outputNames;
			outputScalarValues = wizardPage.outputValues;
			
			String stream;
			String[] inputStreams = DatabaseInterface.getInputStreams(componentName);
			String[] outputStreams = DatabaseInterface.getOutputStreams(componentName);
			
			if(isSystem)
			{
				inputTestFiles = wizardPage.inputTestFiles == null? new String[0] : new String[wizardPage.inputTestFiles.length];
				outputTestFiles = wizardPage.outputTestFiles == null? new String[0] : new String[wizardPage.outputTestFiles.length];
				relativeSafeInputTestFiles = new String[inputTestFiles.length];
				relativeSafeOutputTestFiles = new String[outputTestFiles.length];
				for(int i = 0; i < inputTestFiles.length; ++i)
				{
					stream = inputStreams[i];
					inputTestFiles[i] = wizardPage.inputTestFiles[i].getText();
					relativeSafeInputTestFiles[i] = inputTestFiles[i];
					File f = new File(inputTestFiles[i]);
					if(inputTestFiles[i].startsWith(".\\") || inputTestFiles[i].startsWith("./"))
						f = new File(FileUtils.getFolderOfFile(fileToGenerateTestbenchFor) + inputTestFiles[i]);
					
					if(!f.exists())
					{
						MessageDialog.openError(new Shell(), "Test File Error", "The test file \"" + inputTestFiles[i] + "\" does not exist.");
						return false;
					}
					
					inputTestFiles[i] = f.getAbsolutePath();
					
					if(checkTestFileForCorrectness(inputTestFiles[i], stream, false) == false)
						return false;
				}
				
				for(int i = 0; i < outputTestFiles.length; ++i)
				{
					stream = outputStreams[i];
					outputTestFiles[i] = wizardPage.outputTestFiles[i].getText();
					relativeSafeOutputTestFiles[i] = outputTestFiles[i];
					File f = new File(outputTestFiles[i]);
					if(outputTestFiles[i].startsWith(".\\") || outputTestFiles[i].startsWith("./"))
						f = new File(FileUtils.getFolderOfFile(fileToGenerateTestbenchFor) + outputTestFiles[i]);
						
					if(!f.exists())
					{
						MessageDialog.openError(new Shell(), "Test File Error", "The test file \"" + outputTestFiles[i] + "\" does not exist.");
						return false;
					}
					
					outputTestFiles[i] = f.getAbsolutePath();
					
					if(checkTestFileForCorrectness(outputTestFiles[i], stream, true) == false)
						return false;
				}
			}
			
			//Check to see if they are all valid characters
			for(int i = 0; i < scalarValues.size(); ++i)
			{
				for(int j = 0; j < scalarValues.get(i).size(); ++j)
				{
					if((!StringUtils.isANumber(scalarValues.get(i).get(j)) && !StringUtils.isAHexValue(scalarValues.get(i).get(j))) || scalarValues.get(i).get(j).length() == 0)
					{
						MessageDialog.openError(new Shell(), "Input Scalar Error", "The value for " + scalarNames[i] + " on test set " + (j + 1) + " is not a valid decimal, floating point, or hexidecimal value.");
						return false;
					}
				}
			}
			
			
			for(int i = 0; i < outputScalars.length; ++i)
			{
				for(int j = 0; j < outputScalarValues.get(i).size(); ++j)
				{
					if((!StringUtils.isANumber(outputScalarValues.get(i).get(j)) && !StringUtils.isAHexValue(outputScalarValues.get(i).get(j))) || outputScalarValues.get(i).get(j).length() == 0)
					{
						MessageDialog.openError(new Shell(), "Output Scalar Error", "The value for " + outputScalars[i] + " on test set " + (j + 1) + " is not a valid decimal, floating point, or hexidecimal value.");
						return false;
					}
				}
			}
			
			if(isSystem)
			{
				/*maxMemoryAvailable = wizardPage.memoryBox.getText();
				//Check memory value
				if(!Activator.isAllNumbers(maxMemoryAvailable))
				{
					MessageDialog.openError(new Shell(), "Memory Error", "The max memory available for streams is an invalid number. It must be a positive integer.");
					return false;	
				}*/
				
				clocks = new TreeMap<String, String>();
				Map<String, Text> clks = wizardPage.clks;
				String keys[] = new String[10];
				keys = clks.keySet().toArray(keys);
				boolean failed = false;
				for(int i = 0; i < clks.size(); ++i)
				{
					String value = clks.get(keys[i]).getText();
					
					clocks.put(keys[i], value);

					if(!StringUtils.isPositiveInt(value) && !failed)
					{
						MessageUtils.openErrorWindow("Clock Error", "The value for clock " + keys[i] + " is not a positive integer.");
						failed = true;
					}
				}
				
				if(failed)
					return false;
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return true;
	}

	public void init(IWorkbench workbench, IStructuredSelection selection) 
	{
	
	}

}
