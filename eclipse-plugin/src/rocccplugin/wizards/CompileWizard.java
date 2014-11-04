package rocccplugin.wizards;

import java.io.File;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;
import java.util.Vector;

import org.eclipse.core.resources.IFile;
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
import rocccplugin.helpers.FlagData;
import rocccplugin.helpers.ResourceData;
import rocccplugin.utilities.FileUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.StringUtils;
import rocccplugin.wizardpages.BasicOperationsWeightPage;
import rocccplugin.wizardpages.CompilerFlagsWizardPage;
import rocccplugin.wizardpages.LoCompilerFlagsWizardPage;
import rocccplugin.wizardpages.SystemInfoWizardPage;

public class CompileWizard extends Wizard implements INewWizard 
{
	CompilerFlagsWizardPage compilerFlagsWizardPage;
	LoCompilerFlagsWizardPage loCompilerFlagsWizardPage;
	BasicOperationsWeightPage basicOperationsWeightPage;
	SystemInfoWizardPage systemInfoWizardPage;
	
	static File fileToCompile;
	static File streamInfoFilePath;
	static File moduleInfoFilePath;
	static Vector<ResourceData> functionsCalled;
	static Vector<String> labelsPlaced;
	String componentType;
	boolean fileContainsSystemCalls;
	
	
	
	public String getCompilerFlagsFile() 
	{
		return fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "") + ".ROCCC/" + fileToCompile.getName().replace(".c", "") + ".opt";
		//return fullFilePath.toString();
	}
	
	public String getSystemInfoFile()
	{
		return streamInfoFilePath.toString();
	}
	
	public String getModuleInfoFile()
	{
		return moduleInfoFilePath.toString();
	}
	
	public String getLoCompilerFlagsFile()
	{
		if(loCompilerFlagsWizardPage != null)
			return loCompilerFlagsWizardPage.getSavedFile();
		else
			return fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "") + ".ROCCC/.optlo";
	}
	
	public CompileWizard() 
	{
	}
	
	public CompileWizard(File fileToCompile, String componentType, boolean containsSystemCalls)
	{
		this.fileToCompile = fileToCompile;
		this.componentType = componentType;
		CompilerFlagsWizardPage.setParameters(fileToCompile, componentType);

		streamInfoFilePath = new File(fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "") + "/.ROCCC/.streamInfo");
		moduleInfoFilePath = new File(fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "") + "/.ROCCC/.moduleInfo");
		fileContainsSystemCalls = containsSystemCalls;
		this.setDefaultPageImageDescriptor(ImageDescriptor.createFromImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/buildToHardware.png"))));
	}
	
	@Override
	public void addPages() 
	{
		try
		{
			//If the database is not accessible, do nothing.
			if(!DatabaseInterface.openConnection())
				return;
			String title;
			if(componentType.equals("MODULE"))
				title = "Module Compiler Flags";
			else if(componentType.equals("SYSTEM"))
				title = "System Compiler Flags";
			else
				title = "Intrinsic Compiler Flags";
			
			String folderOfFileToCompile = fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "");
			
			//Add the component page to the page list.
			compilerFlagsWizardPage = new CompilerFlagsWizardPage(title, fileContainsSystemCalls, functionsCalled, labelsPlaced);
			loCompilerFlagsWizardPage = new LoCompilerFlagsWizardPage(title, fileToCompile, new File(fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), ".ROCCC/.optlo")));
			basicOperationsWeightPage = new BasicOperationsWeightPage("Basic Operations Weight", fileToCompile);
			addPage(compilerFlagsWizardPage);
			addPage(loCompilerFlagsWizardPage);
			addPage(basicOperationsWeightPage);
			
			//TODO
			if(componentType.equals("SYSTEM"))
			{
				systemInfoWizardPage = new SystemInfoWizardPage("System Stream Info", fileToCompile.getName(), fileToCompile);
				addPage(systemInfoWizardPage);
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private boolean arg1ShouldBeAString(String flag)
	{
		if(flag.compareTo("LoopUnrolling") == 0 || flag.compareTo("LoopInterchange") == 0 ||
		   flag.compareTo("SystolicArrayGeneration") == 0 || flag.compareTo("Redundancy") == 0 ||
		   flag.compareTo("InlineModule") == 0)
		{
			return true;
		}
		
		return false;
	}
	
	private boolean arg2ShouldBeAString(String flag)
	{
		if(flag.compareTo("LoopInterchange") == 0)
		{
			return true;
		}
		
		return false;
	}
	
	private boolean validCompilerFlagArguments(Vector<FlagData> v)
	{
		if(v == null)
			return true;
		
		for(int i = 0; i < v.size(); ++i)
		{
			String flag = v.get(i).getFlag();
			
			if(flag.compareTo("Redundancy") == 0)
			{
				if(v.get(i).getArgs()[3].compareTo("DOUBLE") != 0 &&
				   v.get(i).getArgs()[3].compareTo("TRIPLE") != 0)
				{
					//It did not start with an alphabetic character.
					MessageDialog.openError(new Shell(), "Optimization Argument Error", "In selected optimization \"" + flag + "\", there is an incorrect argument value.\n\n" +
			                                                   "Make sure the value is either \"DOUBLE\" or \"TRIPLE\".");
					return false;
				}
			}
			if(flag.equals("LoopInterchange"))
			{
				if(v.get(i).getArgs()[1].equals(v.get(i).getArgs()[3]))
				{
					MessageDialog.openError(new Shell(), "Optimization Argument Error", "In selected optimization \"" + flag + "\", the two loop labels are the same.");
					return false;
				}
			}
			if(arg1ShouldBeAString(flag))
			{
				if(v.get(i).getArgs()[1].compareTo("") == 0)
				{
					//It did not start with an alphabetic character.
					MessageDialog.openError(new Shell(), "Optimization Argument Error", "In selected optimization \"" + flag + "\", there is an argument value missing.\n\n" +
			                                                   "Make sure the argument has a value.");
					return false;
				}
				if(!Character.isLetter(v.get(i).getArgs()[1].charAt(0)) &&
					v.get(i).getArgs()[1].charAt(0) != '_')
				{
					//It did not start with an alphabetic character.
					MessageDialog.openError(new Shell(), "Optimization Argument Error", "In selected optimization \"" + flag + "\", argument \"" + v.get(i).getArgs()[1] + "\" is an invalid value.\n\n" +
			                                              "Make sure the argument starts with an alphabetic character.");
					return false;
				}
				for(int j = 0; j < v.get(i).getArgs()[1].length(); ++j)
				{
					if(!Character.isLetter(v.get(i).getArgs()[1].charAt(j)) &&
					   !Character.isDigit(v.get(i).getArgs()[1].charAt(j)) &&
					   v.get(i).getArgs()[1].charAt(j) != '_')
					{
						//It did not start with an alphabetic character.
						MessageDialog.openError(new Shell(), "Optimization Argument Error", "In selected optimization \"" + flag + "\", argument \"" + v.get(i).getArgs()[1] + "\" is an invalid value.\n\n" +
				                                              "Make sure no special characters are used.");
						return false;
					}
				}
			}
			else if(flag.compareTo("InlineAllModules") == 0)
			{
				if(v.get(i).getArgs()[1].compareTo("INFINITE") != 0)
				{
					if(v.get(i).getArgs()[1].compareTo("0") == 0)
					{
						//One of the characters was not a number.
						MessageDialog.openInformation(new Shell(), "Optimization Argument Error", "In selected optimization \"" + flag + "\", argument \"" + v.get(i).getArgs()[1] + 
													  "\" is not a valid number.\n\nMake sure the argument is a positive integer or is the value \"INFINITE\".");
						return false;
					}
					
					for(int j = 0; j < v.get(i).getArgs()[1].length(); ++j)
					{
						if(!Character.isDigit(v.get(i).getArgs()[1].charAt(j)))
						{
							//One of the characters was not a number.
							MessageDialog.openInformation(new Shell(), "Optimization Argument Error", "In selected optimization \"" + flag + "\", argument \"" + v.get(i).getArgs()[1] + 
														  "\" is not a valid number.\n\nMake sure the argument is a positive integer or is the value \"INFINITE\".");
							return false;
						}
					}
				}
			}
			
			if(arg2ShouldBeAString(flag))
			{
				if(v.get(i).getArgs()[3].compareTo("") == 0)
				{
					//It did not start with an alphabetic character.
					MessageDialog.openInformation(new Shell(), "Optimization Argument Error", "In selected optimization \"" + flag + "\", there is an argument value missing.\n\n" +
			                                                   "Make sure the argument has a value.");
					return false;
				}
				if(!Character.isLetter(v.get(i).getArgs()[3].charAt(0)))
				{
					//It did not start with an alphabetic character.
					MessageDialog.openInformation(new Shell(), "Optimization Argument Error", "In selected optimization \"" + flag + "\", argument \"" + v.get(i).getArgs()[3] + "\" is an invalid value.\n\n" +
			                                                   "Make sure the argument starts with an alphabetic character.");
					return false;
				}
			}
			else if(flag.compareTo("LoopUnrolling") == 0 && componentType.equals("SYSTEM"))
			{
				if(v.get(i).getArgs()[3].compareTo("") == 0)
				{
					//It did not start with an alphabetic character.
					MessageDialog.openInformation(new Shell(), "Optimization Argument Error", "In selected optimization \"" + flag + "\", there is an argument value missing.\n\n" +
			                                                   "Make sure the argument has a value.");
					return false;
				}
				if(v.get(i).getArgs()[3].compareTo("FULLY") != 0)
				{
					if(v.get(i).getArgs()[3].compareTo("0") == 0)
					{
						//One of the characters was not a number.
						MessageDialog.openInformation(new Shell(), "Optimization Argument Error", "In selected optimization \"" + flag + "\", argument \"" + v.get(i).getArgs()[3] + 
													  "\" is not a valid number.\n\nMake sure the argument is a positive integer or is the value \"FULLY\".");
						return false;
					}
					
					for(int j = 0; j < v.get(i).getArgs()[3].length(); ++j)
					{
						if(!Character.isDigit(v.get(i).getArgs()[3].charAt(j)))
						{
							//One of the characters was not a number.
							MessageDialog.openInformation(new Shell(), "Optimization Argument Error", "In selected optimization \"" + flag + "\", argument \"" + v.get(i).getArgs()[3] + 
														  "\" is not a valid number.\n\nMake sure the argument is a positive integer or is the value \"FULLY\".");
							return false;
						}
					}
				}
			}
			
		}
		return true;
	}

	private boolean combineRedundantLoopUnrolls(Vector<FlagData> v)
	{
		if(v == null)
			return true;
		
		Map<String, Vector<Integer> > map = new TreeMap<String, Vector<Integer> >();
		Vector<String> duplicates = new Vector<String>();
		
		for(int i = 0; i < v.size(); ++i)
		{
			if(v.get(i).getFlag().compareTo("LoopUnrolling") == 0)
			{
				if(map.containsKey(v.get(i).getArgs()[1]))
				{
					boolean exists = false;
					for(int j = 0; j < duplicates.size(); ++j)
					{
						if(duplicates.get(j).compareTo(v.get(i).getArgs()[1]) == 0)
						{
							exists = true;
							break;
						}
					}				
					
					if(exists == false)
					{
						duplicates.add(v.get(i).getArgs()[1]);
					}
					
					if(!componentType.equals("SYSTEM") || v.get(i).getArgs()[3].compareTo("FULLY") == 0)
						map.get(v.get(i).getArgs()[1]).add(new Integer(-1));
					else
						map.get(v.get(i).getArgs()[1]).add(new Integer(Integer.parseInt(v.get(i).getArgs()[3])));
					
				}
				else
				{
					Vector<Integer> insertVector = new Vector<Integer>();
					if(!componentType.equals("SYSTEM") || v.get(i).getArgs()[3].compareTo("FULLY") == 0)
						insertVector.add(new Integer(-1));
					else
						insertVector.add(new Integer(Integer.parseInt(v.get(i).getArgs()[3])));
					map.put(v.get(i).getArgs()[1], insertVector);
				}
			}
		}
		
		for(int i = 0; i < duplicates.size(); ++i)
		{
			boolean combine = MessageDialog.openQuestion(new Shell(), "Multiple Loop Unrolls On Same Label Error", 
													   "There are multiple loop unrolls on the label \"" + duplicates.get(i) +
													   "\". These must be resolved before compiling. Combine them all?");
			if(combine)
			{
				int total = 0;
				for(int j = 0; j < map.get(duplicates.get(i)).size(); ++j)
				{
					if(map.get(duplicates.get(i)).get(j) == -1)
					{
						total = -1;
					}
					if(total != -1)
					{
						total += map.get(duplicates.get(i)).get(j);
					}
					
				}
				
				for(int j = 0; j < v.size(); ++j)
				{
					if(v.get(j).getFlag().compareTo("LoopUnrolling") == 0)
					{
						if(v.get(j).getArgs()[1].compareTo(duplicates.get(i)) == 0)
						{
							v.remove(j);
							--j;
						}
					}
				}
				
				FlagData f = new FlagData();
				f.setFlag("LoopUnrolling");
				if(total == -1 && componentType.equals("SYSTEM"))
					f.setArgs(new String[]{"Loop Label", duplicates.get(i), "Number of times to unroll", "FULLY"});
				else if(total == -1)
					f.setArgs(new String[]{"Loop " +
							"Label", duplicates.get(i), "Number of times to unroll"});
				else
					f.setArgs(new String[]{"Loop Label", duplicates.get(i), "Number of times to unroll", Integer.toString(total)});
				v.add(f);
			}
			else
			{
				return false;
			}
		}
		
		return true;
	}
	
	private boolean preventMultipleRedundantCalls(Vector<FlagData> v)
	{
		if(v == null)
			return true;
		
		Vector<String> duplicates = new Vector<String>();
		
		for(int i = 0; i < v.size(); ++i)
		{
			if(v.get(i).getFlag().compareTo("Redundancy") == 0)
			{
				boolean exists = false;
				for(int j = 0; j < duplicates.size(); ++j)
				{
					if(duplicates.get(j).compareTo(v.get(i).getArgs()[1]) == 0)
					{
						
						MessageDialog.openError(null, "Multiple Redundant Calls Error", "Error: You have multiple Redundant optimzations on the same label " + v.get(i).getArgs()[1] + ".\nThis must be resolved before continuing.");
						return false;
					}
				}				
				
				if(exists == false)
				{
					duplicates.add(v.get(i).getArgs()[1]);
				}			
			}
		}
		
		return true;
	}
	
	private boolean checkForValidWeights()
	{
		Map<String, Text> text = basicOperationsWeightPage.textBoxes;
		
		try
		{
			Iterator<String> i = text.keySet().iterator();
			
			int max = -1;
			int maxWeight = -1;
			
			for(String key = i.next(); key != null ; key = (i.hasNext()? i.next(): null))
			{
				String value = text.get(key).getText();
				
				if(value.length() == 0)
				{
					MessageDialog.openError(new Shell(), "Weight Error", "The operation \"" + key + "\" has no weight value. All weights must be a positive integer.");
					return false;
				}
				
				int num = Integer.parseInt(text.get(key).getText());
				
				if(key.equals("Max Unregistered Fanout"))
				{
					if(!(StringUtils.isPositiveInt(value) || num == 0))
					{
						MessageDialog.openError(new Shell(), "Fanout Error", "The operation \"" + key + "\" has an invalid value. Fanout must be a non-negative integer.");
						return false;
					}
				}
				else
				{
					if(!StringUtils.isPositiveInt(value))
					{
						MessageDialog.openError(new Shell(), "Weight Error", "The operation \"" + key + "\" has an invalid weight. All weights must be a positive integer.");
						return false;
					}
					
					if(num == 0)
					{
						MessageDialog.openError(new Shell(), "Weight Error", "The operation \"" + key + "\" has a weight of zero. All weights must be a positive integer.");
						return false;
					}
				}
				
				
				/*if(key.equals("Max Weight Per Cycle"))
					maxWeight = num;
				else if(num > max && !key.equals("Max Unregistered Fanout"))
						max = num;*/
			}
			
			/*if(maxWeight < max)
			{
				MessageDialog.openError(new Shell(), "Weight Error", "The Max Weight Per Cycle value must be greater or equal to all other operation weight.");
				return false;
			}*/
			
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return true;
	}
	
	public boolean performFinish() 
	{
		//Check for valid values prior to compiling.
		if(checkValues() == false)
			return false;
		
		//createOptFile();
		compilerFlagsWizardPage.saveFlags();
		loCompilerFlagsWizardPage.saveFlags();
		createTimingInfoFile();
		
		//TODO
		if(componentType.equals("SYSTEM"))
			createStreamInfoFile();
		else
			createModuleInfoFile();
		
		return true;
	}
	
	private boolean checkForValidStreamInfo()
	{
		Vector<String[]> streamInfo = systemInfoWizardPage.streamInfo;
		Map<String, String> duplicates = new TreeMap<String, String>();
		
		Vector<String[]> outputStreamInfo = systemInfoWizardPage.outputStreamInfo;
		Map<String, String> outputDuplicates = new TreeMap<String, String>();
		
		
		for(int i = 0; i < streamInfo.size(); ++i)
		{
			//Check for valid variable name.
			if(!StringUtils.isValidVariableName(streamInfo.get(i)[0]))
			{
				MessageDialog.openError(new Shell(), "Stream Error", "Stream name " + streamInfo.get(i)[0] + " is invalid. Check your stream names before continuing.");
				return false;
			}
			
			if(!duplicates.containsKey(streamInfo.get(i)[0]))
			{
				duplicates.put(streamInfo.get(i)[0], streamInfo.get(i)[0]);
			}	
			else
			{
				MessageDialog.openError(new Shell(), "Stream Error", "There are duplicates of the stream name " + streamInfo.get(i)[0] + ". Make sure there are no duplicate stream names.");
				return false;
			}
			
			
			//Check for valid numeric values.
			if(!StringUtils.isPositiveInt(streamInfo.get(i)[1]))
			{
				MessageDialog.openError(new Shell(), "Stream Error", "Number of data channels for \"" + streamInfo.get(i)[0] + "\" is invalid. Value must be a positive integer.");
				return false;
			}
			
			//Check for valid numeric values.
			if(!StringUtils.isPositiveInt(streamInfo.get(i)[2]))
			{
				MessageDialog.openError(new Shell(), "Stream Error", "Number of address channels for \"" + streamInfo.get(i)[0] + "\" is invalid. Value must be a positive integer.");
				return false;
			}
			
			if(Integer.parseInt(streamInfo.get(i)[1]) != Integer.parseInt(streamInfo.get(i)[2]) && Integer.parseInt(streamInfo.get(i)[2]) != 1)
			{
				MessageDialog.openError(new Shell(), "Stream Error", "Number of address channels for \"" + streamInfo.get(i)[0] + "\" must be \"1\" or equal to the number of data channels.");
				return false;
			}
		}	
		
		for(int i = 0; i < outputStreamInfo.size(); ++i)
		{
			if(!StringUtils.isValidVariableName(outputStreamInfo.get(i)[0]))
			{
				MessageDialog.openError(new Shell(), "Stream Error", "Stream name " + outputStreamInfo.get(i)[0] + " is invalid. Check your stream names before continuing.");
				return false;
			}
			
			if(!outputDuplicates.containsKey(outputStreamInfo.get(i)[0]))
			{
				outputDuplicates.put(outputStreamInfo.get(i)[0], outputStreamInfo.get(i)[0]);
			}
			else
			{
				MessageDialog.openError(new Shell(), "Stream Error", "There are duplicates of the stream name " + outputStreamInfo.get(i)[0] + ". Make sure there are no duplicate stream names.");
				return false;
			}
			
			
			//Check for valid numeric values.
			if(!StringUtils.isPositiveInt(outputStreamInfo.get(i)[2]))
			{
				MessageDialog.openError(new Shell(), "Stream Error", "Number of address channels for \"" + outputStreamInfo.get(i)[0] + "\" is invalid. Value must be a positive integer.");
				return false;
			}
			if(Integer.parseInt(outputStreamInfo.get(i)[1]) != Integer.parseInt(outputStreamInfo.get(i)[2]) && Integer.parseInt(outputStreamInfo.get(i)[2]) != 1)
			{
				MessageDialog.openError(new Shell(), "Stream Error", "Number of address channels for \"" + outputStreamInfo.get(i)[0] + "\" must be \"1\" or equal to the number of data channels.");
				return false;
			}
			
			
			
			if(!StringUtils.isPositiveInt(outputStreamInfo.get(i)[1]))
			{
				MessageDialog.openError(new Shell(), "Stream Error", "Number of parallel output data channels for \"" + outputStreamInfo.get(i)[0] + "\" is invalid. Value must be a positive integer.");
				return false;
			}
			
			for(int j = 0; j < streamInfo.size(); ++j)
			{
				if(outputStreamInfo.get(i)[0].equals(streamInfo.get(j)[0]))
				{
					MessageDialog.openError(new Shell(), "Stream Error", "There are duplicates of the stream name " + outputStreamInfo.get(i)[0] + ". Make sure there are no duplicate stream names.");
					return false;
				}
			}
		}
		
		return true;
	}
	
	private boolean checkValues()
	{
		StringBuffer buf = new StringBuffer();
		Vector<FlagData> v = compilerFlagsWizardPage.getSelectedFlags();
		
		try
		{
			//Check for valid arguments
			if(validCompilerFlagArguments(v) == false)
				return false;
			
			//Do loop unroll combination check.
			if(combineRedundantLoopUnrolls(v) == false)
				return false;
			//Make sure there isn't multiple Redundant calls on the same label.
			if(preventMultipleRedundantCalls(v) == false)
				return false;
			
			//check for positive weights and such
			if(checkForValidWeights() == false)
				return false;
			
			//TODO
			if(componentType.equals("SYSTEM"))
			{
				if(checkForValidStreamInfo() == false)
					return false;
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
		
		return true;
	}
	
	private void createOptFile()
	{
		StringBuffer buf = new StringBuffer();
		Vector<FlagData> v = compilerFlagsWizardPage.v;
		
		try
		{
			if(v != null)
			{
				for(int i = 0; i < v.size(); ++i)
				{
					buf.append(v.get(i).getFlag());
					for(int j = 1; j < v.get(i).getArgs().length; j+=2)
					{
						buf.append(" " + v.get(i).getArgs()[j]);
					}
					buf.append("\n");
				}
			}
			
			if(componentType.equals("MODULE"))
				buf.append("FullyUnroll\n");
			
			if(fileContainsSystemCalls && componentType.equals("SYSTEM"))
				buf.append("ComposableSystem");
			
			buf.append("Export\n");
			
			if(componentType.equals("INTRINSIC"))
				buf.append(componentType + "\n");
			
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		IFile theFile;
		String folderString = fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "");
		
		File rocccFolder = new File(folderString + ".ROCCC/");
		if(!rocccFolder.exists())
			rocccFolder.mkdir();
		/*IFolder folder = ResourcesPlugin.getWorkspace().getRoot().getProject(fileToCompile.getProject().getName()).getFolder(folderString + ".ROCCC/"); 
		try 
		{
			if(!folder.exists())
				folder.create(true, true, null);
		}
		catch (CoreException e1) 
		{
			e1.printStackTrace();
		}*/
		
		//theFile = ResourcesPlugin.getWorkspace().getRoot().getProject(file.getProject().getName()).getFile(folderString + ".ROCCC/" + file.getName().replace(".c", ".opt"));

		//Actually create the source file.
		try
		{
			//ByteArrayInputStream Bis1 = new ByteArrayInputStream(buf.toString().getBytes("UTF-8"));
			//fullFilePath = new File(theFile.getRawLocation().toOSString());
			//if(theFile.exists())
				//theFile.delete(true, true, null);
			//if(fullFilePath.exists())
				//fullFilePath.delete();
			if(fileToCompile.exists())
				fileToCompile.delete();
			FileUtils.createFileFromBuffer(buf, new File(rocccFolder.getAbsolutePath() + fileToCompile.getName().replace(".c", ".opt")));
			//theFile.create(Bis1, false, null);
		}
		catch(java.lang.Exception e)
		{
			e.printStackTrace();
			MessageDialog.openInformation(new Shell(), "Error", "There was an error creating your file.");
		}
	}

	private boolean createTimingInfoFile()
	{
		String folderString = fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "");
		File optloFile = new File(folderString + ".ROCCC/.optlo");
		//ResourcesPlugin.getWorkspace().getRoot().getProject(file.getProject().getName()).getFile(folderString + ".ROCCC/.optlo");
		
		StringBuffer timinginfo = new StringBuffer();
		Map<String, Text> text = basicOperationsWeightPage.textBoxes;
		
		StringBuffer optloFileAdditions = new StringBuffer();
		FileUtils.addFileContentsToBuffer(optloFileAdditions, loCompilerFlagsWizardPage.getSavedFile());
		
		try
		{
			Iterator<String> i = text.keySet().iterator();
			
			for(String key = i.next(); key != null ; key = (i.hasNext()? i.next(): null))
			{
				String name = key;
				if(key.equals("Max Weight Per Cycle"))
					continue;
				name = key.equals("Max Weight Per Cycle")? "DesiredTiming" : name;
				name = key.equals("Max Unregistered Fanout")? "MaxFanoutRegistered" : name;
				timinginfo.append(name + " " + text.get(key).getText() + "\n");
				//optloFileAdditions.append(name + " " + text.get(key).getText() + "\n");
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		timinginfo.append("OperationsPerPipelineStage " + basicOperationsWeightPage.opsPerStage);
		
		//Activator.createResourceFileFromBuffer(optloFileAdditions, file, "/.ROCCC/.optlo");
		FileUtils.createFileFromBuffer(optloFileAdditions, optloFile);
		
		//IFile timingFile = ResourcesPlugin.getWorkspace().getRoot().getProject(file.getProject().getName()).getFile(folderString + ".ROCCC/.timingInfo");
		
		File timingInfoFile = new File(folderString + ".ROCCC/.timingInfo");
		
		try
		{
			/*ByteArrayInputStream Bis1 = new ByteArrayInputStream(timinginfo.toString().getBytes("UTF-8"));
			File filePath = new File(timingFile.getRawLocation().toOSString());
			if(timingFile.exists())
				timingFile.delete(true, true, null);
			if(filePath.exists())
				filePath.delete();
			timingFile.create(Bis1, false, null);*/
			if(timingInfoFile.exists())
				timingInfoFile.delete();
			FileUtils.createFileFromBuffer(timinginfo, timingInfoFile);
		}
		catch(java.lang.Exception e)
		{
			e.printStackTrace();
			MessageDialog.openError(new Shell(), "Error", "There was an error creating your file.");
			return false;
		}
		
		return true;
	}
	
	private boolean createStreamInfoFile()
	{
		Vector<String[]> streamInfo = systemInfoWizardPage.streamInfo;
		Vector<String[]> outputStreamInfo = systemInfoWizardPage.outputStreamInfo;
		StringBuffer buffer = new StringBuffer();
		String folderString = fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "");
	 	
	 	try
	 	{
		 	for(int i = 0; i < streamInfo.size(); ++i)
		 	{
		 		buffer.append("INPUT " + streamInfo.get(i)[0] + " " + streamInfo.get(i)[1] + " " + streamInfo.get(i)[2] + "\n");
		 	}
		 	for(int i = 0; i < outputStreamInfo.size(); ++i)
		 	{
		 		buffer.append("OUTPUT " + outputStreamInfo.get(i)[0] + " " + outputStreamInfo.get(i)[1] + " " + outputStreamInfo.get(i)[2]  + "\n");
		 	}
	 	}
	 	catch(Exception e)
	 	{
	 		e.printStackTrace();
	 	}
	 	
	 	//IFile streamInfoFile = ResourcesPlugin.getWorkspace().getRoot().getProject(file.getProject().getName()).getFile(folderString + ".ROCCC/.streamInfo");
		File streamInfoFile = new File(folderString + ".ROCCC/.streamInfo");
		
	 	try
		{
			/*ByteArrayInputStream Bis1 = new ByteArrayInputStream(buffer.toString().getBytes("UTF-8"));
			streamInfoFilePath = new File(streamInfoFile.getRawLocation().toOSString());
			if(streamInfoFile.exists())
				streamInfoFile.delete(true, true, null);
			if(streamInfoFilePath.exists())
				streamInfoFilePath.delete();
			streamInfoFile.create(Bis1, false, null);*/
	 		if(streamInfoFile.exists())
	 			streamInfoFile.delete();
			FileUtils.createFileFromBuffer(buffer, streamInfoFile);
		}
		catch(java.lang.Exception e)
		{
			e.printStackTrace();
			MessageDialog.openError(new Shell(), "Error", "There was an error creating your file.");
			return false;
		}
		
		return true;
	}
	
	private boolean createModuleInfoFile()
	{
		//Right now this function purely creates an empty file because we need to pass the compile script an info file.
		StringBuffer buffer = new StringBuffer();
		String folderString = fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "");
		//IFile moduleInfoFile = ResourcesPlugin.getWorkspace().getRoot().getProject(file.getProject().getName()).getFile(folderString + ".ROCCC/.moduleInfo");
		File moduleInfoFile = new File(folderString + ".ROCCC/.moduleInfo");
		
		try
		{
			/*ByteArrayInputStream Bis1 = new ByteArrayInputStream(buffer.toString().getBytes("UTF-8"));
			moduleInfoFilePath = new File(moduleInfoFile.getRawLocation().toOSString());
			if(moduleInfoFile.exists())
				moduleInfoFile.delete(true, true, null);
			if(moduleInfoFilePath.exists())
				moduleInfoFilePath.delete();
			moduleInfoFile.create(Bis1, false, null);*/
			
			if(moduleInfoFile.exists())
				moduleInfoFile.delete();
		}
		catch(java.lang.Exception e)
		{
			e.printStackTrace();
			MessageDialog.openError(new Shell(), "Error", "There was an error creating your file.");
			return false;
		}
		
		return true;
	}
	
	public void init(Vector<ResourceData> functionsCalled, Vector<String> labelsPlaced) 
	{
		this.functionsCalled = functionsCalled;
		this.labelsPlaced = labelsPlaced;
	}

	public void init(IWorkbench workbench, IStructuredSelection selection) {
		// TODO Auto-generated method stub
		
	}
}
