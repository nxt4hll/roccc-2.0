package rocccplugin.wizards;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.InputStreamReader;
import java.util.Vector;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Table;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

import rocccplugin.Activator;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.StringUtils;
import rocccplugin.wizardpages.AddComponentWizardPage;

public class AddComponentWizard extends Wizard implements INewWizard 
{
	//The only page for the wizard.
	private AddComponentWizardPage addComponentPage;	
	
	//Constructor
	public AddComponentWizard() 
	{
	}
	
	//Adds the addComponentWizardPage to the page list of the wizard.
	@Override
	public void addPages() 
	{
		//Add the component page to the page list.
		addComponentPage = new AddComponentWizardPage("Component Information Page");
		addPage(addComponentPage);
		this.setDefaultPageImageDescriptor(ImageDescriptor.createFromImage(new Image(null,  this.getClass().getResourceAsStream("/rocccplugin/images/addIPCore.png"))));
	}

	@Override
	public boolean performFinish() 
	{
		try
		{
			DatabaseInterface.openConnection();
			//Check for valid inputs first.
			if(validInputs() == false)
			{
				DatabaseInterface.closeConnection();
				return false;
			}		
			
			/////////////////////////////////////////////////////////
			//Everything checked out fine, add it to the database
			/////////////////////////////////////////////////////////
			
			//Add the component name and delay to the database.
			final String compName = addComponentPage.component_name.getText();
			int compDelay = Integer.parseInt(addComponentPage.delay.getText());
			String portOrder = new String();
			for(int i = 0; i < addComponentPage.ports.getItemCount(); ++i)
			{
				if(i > 0)
					portOrder += ", ";
				portOrder += "/* " + addComponentPage.ports.getItem(i).getText() + " */";
			}
			
			DatabaseInterface.addComponent(compName,compDelay, portOrder, "NA");
			
			//Add all the ports on the component to the database.
			for(int i = 0; i < addComponentPage.ports.getItemCount(); ++i)
			{
				String port_name = addComponentPage.ports.getItem(i).getText(0);
				String port_dir = addComponentPage.ports.getItem(i).getText(1);
				int port_size = Integer.parseInt(addComponentPage.ports.getItem(i).getText(2));
				String dataType = addComponentPage.ports.getItem(i).getText(3);
				DatabaseInterface.addPort(compName, port_name, port_dir, i + 1, port_size, dataType);
			}
			
			StringBuffer buffer = new StringBuffer();
			buffer.append(addComponentPage.component_name.getText() + "\n");
			buffer.append(addComponentPage.delay.getText() + "\n");
			
			for(int i = 0; i < addComponentPage.ports.getItemCount(); ++i)
			{
				buffer.append(addComponentPage.ports.getItem(i).getText(0) + " " +
							  addComponentPage.ports.getItem(i).getText(1) + " " +
							  addComponentPage.ports.getItem(i).getText(2) + " " +
							  addComponentPage.ports.getItem(i).getText(3) + "\n");
			}
			
			
			final File theFile = new File(Activator.getDistributionFolder() + "/newComponent.ROCCC");
			
			//Actually create the source file.
			try
			{
				if(theFile.exists())
					theFile.delete();
				
				FileWriter fstream = new FileWriter(theFile.toString());
		        BufferedWriter out = new BufferedWriter(fstream);
		        out.write(buffer.toString());
		        out.close();
			}
			catch(java.lang.Exception e)
			{
				e.printStackTrace();
				MessageDialog.openInformation(new Shell(), "Error", "There was an error creating your file.");
				DatabaseInterface.closeConnection();
				return true;
			}
			
			DatabaseInterface.closeConnection();
			
			//Call Script
			Thread t = new Thread(new Runnable()
			{
				public void run() 
				{
					// The steps that the script performs are as follows:
					//  - Set up the environment variables
					//  - Run gcc2suif on blank.c
					//  - Run suifdriver with the appropriate commands
					try 
					{
						Activator.OSType os = Activator.getOS();
						final String arch ;
						if (os == Activator.OSType.LION || os == Activator.OSType.SNOW_LEOPARD || os == Activator.OSType.LEOPARD)
							arch = "Darwin" ;
						else
							arch = "Linux" ;
					
						final String rootDirectory = Activator.getDistributionFolder() ;
						final String originalPATH = System.getenv("PATH") ;
						final String originalLDLIBRARYPATH = System.getenv("LD_LIBRARY_PATH") ;
						final String originalDYLDLIBRARYPATH = System.getenv("DYLD_LIBRARY_PATH") ;
						
						String[] environment = new String[7] ;
						environment[0] = "ROCCC_HOME=" + rootDirectory ;
						environment[1] = "NCIHOME=" + rootDirectory ;
						environment[2] = "MACHSUIFHOME=" + rootDirectory ;
						environment[3] = "ARCH=" + arch ;
						environment[4] = "LD_LIBRARY_PATH=" + rootDirectory + "/lib:" + originalLDLIBRARYPATH ;
						environment[5] = "PATH=" + rootDirectory + "/bin:" + originalPATH ;
						environment[6] = "DYLD_LIBRARY_PATH=" + rootDirectory + "/lib:" + originalDYLDLIBRARYPATH ;
					
						String[] gcc2suif = new String[3] ;
						gcc2suif[0] = rootDirectory + "/bin/gcc2suif" ;
						gcc2suif[1] = rootDirectory ;
						gcc2suif[2] = rootDirectory + "/LocalFiles/blank.c" ;
					
						Process p1 = Runtime.getRuntime().exec(gcc2suif, environment) ;
						p1.waitFor() ;
					
//						BufferedReader gcc2suifOut = new BufferedReader(new InputStreamReader(p1.getInputStream())) ;
//						BufferedReader gcc2suifError = new BufferedReader(new InputStreamReader(p1.getErrorStream())) ;
//					
//						while (gcc2suifOut.ready() || !Activator.isProcessDone(p1))
//						{
//							if (gcc2suifOut.ready())
//							{
//								String line = gcc2suifOut.readLine() ;
//								if (line != null)
//								{
//									MessageUtils.printlnConsoleMessage(line) ;
//								}
//							}
//						}
					
//						while (gcc2suifError.ready())
//						{
//							String line = gcc2suifError.readLine();
//							if (line != null)
//								MessageUtils.printlnConsoleMessage(line) ;
//						}
						
						String[] suifdriver = new String[3] ;
						suifdriver[0] = rootDirectory + "/bin/suifdriver" ;
						suifdriver[1] = "-e" ;
						suifdriver[2] = "require basicnodes suifnodes cfenodes transforms control_flow_analysis ;" +
										"require jasonOutputPass libraryOutputPass global_transforms utility_transforms array_transforms ;" +
										"require bit_vector_dataflow_analysis gcc_preprocessing_transforms verifyRoccc ;" +
										"require preprocessing_transforms data_dependence_analysis ;" +
										"require fifoIdentification ;" +
										"load " + rootDirectory + "/LocalFiles/blank.suif ; " + 
										"CleanRepositoryPass " + rootDirectory + "/LocalFiles ;" +
										"AddModulePass " + rootDirectory + "/newComponent.ROCCC " + rootDirectory + "/LocalFiles ; " +
										"DumpHeaderPass " + rootDirectory + "/LocalFiles ;" ;
						
						Process p2 = Runtime.getRuntime().exec(suifdriver, environment) ;
						p2.waitFor() ;
						
//						BufferedReader suifdriverOut = new BufferedReader(new InputStreamReader(p2.getInputStream())) ;
//						BufferedReader suifdriverError = new BufferedReader(new InputStreamReader(p2.getErrorStream())) ;
//						
//						while (suifdriverOut.ready() || !Activator.isProcessDone(p2))
//						{
//							if (suifdriverOut.ready())
//							{
//								String line = suifdriverOut.readLine() ;
//								if (line != null)
//								{
//									MessageUtils.printlnConsoleMessage(line) ;
//								}
//							}
//						}
//						while (suifdriverError.ready())
//						{
//							String line = suifdriverError.readLine();
//							if (line != null)
//								MessageUtils.printlnConsoleMessage(line) ;
//						}
					}
					catch (Exception e)
					{
						e.printStackTrace() ;
					}
					/*
					try
					{
						
						String executable = Activator.getDistributionFolder() + "/scripts/add_module.sh" ;
						if (new File(executable).exists() == false)
						{
							executable = Activator.getDistributionFolder() + "/Install/roccc-compiler/bin/add_module.sh";
						}
						
						File script = new File(executable);
						if(script.exists() == false)
						{
							MessageUtils.printlnConsoleError("Error: Script " + executable + " does not exist, cannot add component to roccc-library.h\n");
							GuiLockingUtils.unlockGui();
							return;
						}
						
						String[] cmdArray = new String[3];
						
						cmdArray[0] = executable;
						cmdArray[1] = theFile.getPath().toString();
						cmdArray[2] = Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION);
						
						Process p = Runtime.getRuntime().exec(cmdArray);
							
						BufferedReader inputStream = new BufferedReader(new InputStreamReader(p.getInputStream()));
						BufferedReader errorStream = new BufferedReader (new InputStreamReader(p.getErrorStream()));
					
						while(inputStream.ready() || !Activator.isProcessDone(p)) 
						{
							if(inputStream.ready())
							{
								String line;
								if((line = inputStream.readLine()) != null)
								{
									MessageUtils.printlnConsoleMessage(line);
								}
							}
						}
		
						while(errorStream.ready())
						{
							String line;
							if((line = errorStream.readLine()) != null)
							{
								if(!line.contains("indirect jmp without") && !line.equals("") && !line.equals(" "))
								{
									MessageUtils.printlnConsoleError(line);
								}
							}
						}
						
						inputStream.close();
						errorStream.close();
						
						theFile.delete();
						
						MessageUtils.printlnConsoleSuccess("Component Imported to Database.\n");
						GuiLockingUtils.unlockGui();
					}
					catch(Exception e)
					{
						e.printStackTrace();
					}
					*/
				}	
			});
			
			t.start();
			
		}
		catch(java.lang.Exception e)
		{
			System.out.println(e.getMessage());
		}
		
		return true;
	}
	
	//Checks for valid input on the component wizard.
	private boolean validInputs()
	{
		try
		{
			String compName = addComponentPage.component_name.getText();
			String delay = addComponentPage.delay.getText();
			Table ports = addComponentPage.ports;
			int numComponentsInDatabase = DatabaseInterface.getNumComponents();
			
			String[] componentsInDatabase = DatabaseInterface.getAllComponents();
			
			//Check to see if there is a component in the database with the same already.
			for(int i = 0; i < componentsInDatabase.length; ++i)
			{
				if(componentsInDatabase[i].compareToIgnoreCase(compName) == 0)
				{
					//There was a component in the database with the same name, display error.
					MessageUtils.openErrorWindow("Matching Name Error", "There is a component in the " +
			                                                   "database with the name \"" + compName + "\" already.\n\n" + 
			                                                   "Choose a different component name.");
					return false;
				}
				
				String structName = DatabaseInterface.getStructName(componentsInDatabase[i]);
				if(structName != null && structName.equals(compName))
				{
					//There was a component in the database with the same name, display error.
					MessageUtils.openErrorWindow("Matching Name Error", "There is a struct name in the " +
							                             "database with the name \"" + compName + "\" already.\n\n" + 
							                             "Choose a different component name.");
					return false;
				}
			}
			
			//Check to see the name of the component starts with a alphabetic character.
			if(!Character.isLetter(compName.charAt(0)))
			{
				//It did not start with an alphabetic character.
				MessageUtils.openErrorWindow("Component Name Error", "The name \"" + compName + "\" is an invalid component name.\n\n" +
		                                                   "Make sure the component name starts with an alphabetic character.\n\n" +
		                                                   "Note: Names are not case sensitive.");	
				return false;
			}
			
			if(StringUtils.isCPlusPlusReservedWord(compName))
			{
				MessageUtils.openErrorWindow("Component Name Error", "Component name \"" + compName + "\" is reserved by C++.\n\n" +
		                                                   "Make sure the component name is not reserved by C++.");
				return false;
			}
			
			if(StringUtils.isROCCCReservedWord(compName))
			{
				//It did not start with an alphabetic character.
				MessageUtils.openErrorWindow("Component Name Error", "Component name \"" + compName + "\" is reserved by ROCCC.\n\n" +
														   "Make sure the component name does not start with \"ROCCC\" or \"suifTmp\".");
				return false;
			}
			
			//Check to make sure the delay is a valid number.
			for(int i = 0; i < delay.length(); ++i)
			{
				if(!Character.isDigit(delay.charAt(i)))
				{
					//One of the characters was not a number.
					MessageUtils.openErrorWindow("Latency Error", "Latency \"" + delay + "\" is invalid.\n\n" +
	                											"Make sure the latency is a positive integer number greater than 1.");
					return false;
				}				
			}
			
			//Check to see if the latency is greater than 1
			if(Integer.parseInt(delay) < 2)
			{
				MessageUtils.openErrorWindow("Latency Error", "Latency \"" + delay + "\" is invalid.\n\n" +
											 "Make sure the latency is a positive integer number greater than 1.");
				return false;
			}
			
			Vector<String> v = new Vector<String>();
			
			for(int i = 0; i < ports.getItemCount(); ++i)
			{			
				String portName = ports.getItem(i).getText(0);
				
				if(portName.equals(compName))
				{
					//Component name matches a port name
					MessageUtils.openErrorWindow("Matching Error", "One of the ports matches the component name. Make sure your port names do not match the component name.");
					return false;
				}
				
				if(v.contains(portName))
				{
					//We have matching port names.
					MessageUtils.openErrorWindow("Port Name Error", "There are more than one port with the same name: \"" + ports.getItem(i).getText(0) + "\"\n\n" +
	                									 "Make sure each port has a unique name.");
					return false;
				}
				else
					v.add(portName);
			
				if(portName.length() == 0)
				{
					//the port name was blank.
					MessageUtils.openErrorWindow("Port Name Error", "One of the ports has no name." +
	                									 "Make sure each port has a unique name with at least one character.");
					return false;
				}
				
				if(StringUtils.isCPlusPlusReservedWord(portName))
				{
					MessageUtils.openErrorWindow("Port Name Error", "Port name \"" + ports.getItem(i).getText(0) + "\" is reserved by C++.\n\n" +
			                                                   "Make sure the port names are not reserved by C++.");
					return false;
				}
				
				if(StringUtils.isROCCCReservedWord(portName))
				{
					MessageUtils.openErrorWindow("Port Name Error", "Port name \"" + ports.getItem(i).getText(0) + "\" is reserved by ROCCC.\n\n" +
                    										   "Make sure the port names are not reserved by ROCCC.");
					return false;
				}
				
				if(!Character.isLetter(portName.charAt(0)))
				{
					//The first character was not a letter.
					MessageUtils.openErrorWindow("Port Name Error", "Port Name \"" + ports.getItem(i).getText(0) + "\" is invalid.\n\n" +
	                									 "Make sure the port name starts with an alphabetic character.");
					return false;
				}
				
				if(!Character.isLetter(portName.charAt(portName.length() - 1)) && !Character.isDigit(portName.charAt(portName.length() - 1)))
				{
					//The first character was not a letter.
					MessageUtils.openErrorWindow("Port Name Error", "Port Name \"" + portName + "\" is invalid.\n\n" +
	                									 "Make sure the port name ends with an alphabetic character or number.");
					return false;
				}
				
				for(int j = 0; j < portName.length(); ++j)
				{
					if(!Character.isLetter(portName.charAt(j)) &&
					   !Character.isDigit(portName.charAt(j)) &&
					   portName.charAt(j) != '_')
					{
						//It did not start with an alphabetic character.
						MessageUtils.openErrorWindow( "Port Name Error", "Port name \"" + ports.getItem(i).getText(0) + "\" is invalid.\n\n" +
				                                             "Make sure the port name has no special characters.");
						return false;
					}
				}
				
				if(ports.getItem(i).getText(2).length() == 0)
				{
					//Port size was blank.
					MessageUtils.openErrorWindow("Port Size Error", "One of the port sizes has no value." +
	                									 "Make sure each port size is a positive integer.");
					return false;
				}	
				
				for(int j = 0; j < ports.getItem(i).getText(2).length(); ++j)
				{
					if(!Character.isDigit(ports.getItem(i).getText(2).charAt(j)))
					{
						MessageUtils.openErrorWindow("Port Size Error", "Port size \"" + ports.getItem(i).getText(2) + "\" is invalid.\n\n" +
                        									 "Make sure the port size is a positive integer.");
						return false;
					}
				}
				
				if(Integer.parseInt(ports.getItem(i).getText(2)) <= 0)
				{
					//Port size is not a positive integer.
					MessageUtils.openErrorWindow("Port Size Error", "One of the port sizes has a non positive value.\n\n" +
	                									 "Make sure each port size is a positive integer.");
					return false;
				}
				
				if(ports.getItem(i).getText(3).equals("float"))
				{
					int bitSize = Integer.parseInt(ports.getItem(i).getText(2));
					if(bitSize != 16 && bitSize != 32 && bitSize != 64)
					{
						MessageUtils.openErrorWindow("Float Port Size Error", "One of the ports is a float and does not have a port size of 16, 32 or 64.\n\nAll floats must be either 16, 32, or 64 bits.");
						
						return false;
					}
				}
				
			}
		}
		catch(java.lang.Exception e)
		{
			e.printStackTrace();
		}
		
		return true;
	}

	public void init(IWorkbench workbench, IStructuredSelection selection) 
	{
	
	}

}
