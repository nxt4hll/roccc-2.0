package rocccplugin.actions;

import java.io.BufferedReader;
import java.io.BufferedWriter ;
import java.io.File;
import java.io.InputStream ;
import java.io.OutputStream ;
import java.io.FileInputStream ;
import java.io.FileOutputStream ;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter ;
import java.io.FileReader ;
import java.io.FileWriter ;
import java.sql.SQLException;
import java.util.Date;
import java.util.Vector;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;
import org.osgi.framework.Version;

import rocccplugin.Activator;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.helpers.ResourceData;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.FileUtils;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.PreferenceUtils;
import rocccplugin.utilities.StringUtils;
import rocccplugin.views.DebugVariables;
import rocccplugin.views.ROCCC_IPCores;
import rocccplugin.wizards.CompileWizard;

public class CompilationPass 
{
	static Thread t;
	static boolean enabled = true;
	static boolean compilationAlive = false;
	static ReentrantLock lock = new ReentrantLock(true);
	static Process compileProcess;
	static String distributionDirectory;
	
	static String componentName = new String();
	static boolean preProcessSucceeded = false;
	static Vector<ResourceData> functionsCalled = new Vector<ResourceData>();
	static Vector<String> labelsPlaced = new Vector<String>();
	static File suifFile = null;
	
	
	static boolean containsSystemCalls = false;
	
	static final boolean CANCELED = false;
	
	public static String[] CreateOptCmd(String distributionDirectory, String libraryExtension, String lowOpts, String filePath)
	{
//		String[] optCmd = new String[13] ;
//		optCmd[0] = distributionDirectory + "/bin/opt" ;
//		optCmd[1] = "-load" ;
//		optCmd[2] = distributionDirectory + "/lib/FloatPass" + libraryExtension ;
//		optCmd[3] = "-load" ;
//		optCmd[4] = distributionDirectory + "/lib/PipelinePass" + libraryExtension ;
//		optCmd[5] = "-load" ;
//		optCmd[6] = distributionDirectory + "/lib/VHDLOutput" + libraryExtension ;
//		optCmd[7] = "-load" ;
//		optCmd[8] = distributionDirectory + "/lib/RocccIntrinsic" + libraryExtension ;
//		optCmd[9] = passes ;
//		optCmd[10] = "-f" ;
//		optCmd[11] = "-o" ;
//		optCmd[12] = "/dev/null" ;
		
		
		String passes = CompilationPass.CreatePasses(lowOpts) ;

		String[] splitPasses = passes.split(" ") ;
		
		String[] optCmd = new String[9 + splitPasses.length + 4] ;
		optCmd[0] = distributionDirectory + "/bin/opt" ;
		optCmd[1] = "-load" ;
		optCmd[2] = distributionDirectory + "/lib/FloatPass" + libraryExtension ;
		optCmd[3] = "-load" ;
		optCmd[4] = distributionDirectory + "/lib/PipelinePass" + libraryExtension ;
		optCmd[5] = "-load" ;
		optCmd[6] = distributionDirectory + "/lib/VHDLOutput" + libraryExtension ;
		optCmd[7] = "-load" ;
		optCmd[8] = distributionDirectory + "/lib/RocccIntrinsic" + libraryExtension ;
		
		for (int i = 0 ; i < splitPasses.length ; ++i)
		{
			optCmd[i+9] = splitPasses[i] ;
		}
		
		optCmd[9+splitPasses.length] = "-f" ;
		optCmd[9+splitPasses.length+1] = "-o" ;
		optCmd[9+splitPasses.length+2] = "/dev/null" ;
		optCmd[9+splitPasses.length+3] = filePath + "/hi_cirrf.c.bc" ;
		
		return optCmd ;
	}
	
	public static String CreatePasses(String lowOpts)
	{
		String passes = "" ;
		// Check for maximize precision
		if (lowOpts.contains("MaximizePrecision"))
		{
			passes += "-maximizePrecision " ;
		}
		passes += "-renameMem " ;
		passes += "-mem2reg " ;
		passes += "-removeExtends " ;
		passes += "-ROCCCfloat " ;
		passes += "-lutDependency " ;
		// Check for Arithmetic Balancing
		if (lowOpts.contains("ArithmeticBalancing"))
		{
			passes += "-rocccFunctionInfo -flattenOperations -dce " ;
		}
		passes += "-undefDetect " ;
		passes += "-functionVerify " ;
		passes += "-rocccCFGtoDFG " ;
		passes += "-detectLoops " ;
		// Check for FanoutTreeGeneration
		if (lowOpts.contains("FanoutTreeGeneration"))
		{
			passes += "-fanoutTree " ;
		}
		passes += "-fanoutAnalysis " ;
		passes += "-pipeline " ;
		passes += "-retime " ;
		// Check for CopyReduction
		if (lowOpts.contains("CopyReduction"))
		{
			passes += "-minimizeCopies " ;
		}
		passes += "-insertCopy " ;
		passes += "-arrayNorm " ;
		passes += "-vhdl " ;
		passes += "-systemToSystem " ;
		passes += "-printPortNames " ;
		passes += "-componentInfo " ;
		passes += "-deleteAll " ;
		return passes ;
	}
	
	public static Thread compileFile(final File fileToCompile, final boolean skipOptimizationWizard)
	{
		try
		{
			// Added by Jason
/*			LicenseManagmentUtils licenseUtils = new LicenseManagmentUtils();
			if (!licenseUtils.isGoodLicenseFile())
			{
				MessageUtils.openErrorWindow("License Validation Error", "Unable to locate your license file!");
				GuiLockingUtils.unlockGui();
				return null;
			}
			
			if(!licenseUtils.validateLicense())
			{
				MessageUtils.openErrorWindow("License Validation Error", "Your license is invalid.  Cannot compile.\n");
				GuiLockingUtils.unlockGui();
				return null;
			}
			*/
			// End of additions
			
			distributionDirectory = PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION);
			
			final String fileToCompilePath = fileToCompile.getAbsolutePath();			
			
			//Make sure we can add and edit files in the location the file to compile exists.
			if(checkFilePermissions(fileToCompilePath) == false)
			{
				MessageUtils.printlnConsoleError("Error: You do not have write permissions in the project file location. Cannot compile file " + fileToCompile.getName() + "\n");
				GuiLockingUtils.unlockGui();
				return null;
			}
			
			//Open the console and show that we are now compiling.
			MessageUtils.openConsoleView();
			MessageUtils.printlnConsoleMessage("Compiling " + fileToCompile.getName().toString() + "...");
			
			containsSystemCalls = false;
			
			DatabaseInterface.openConnection();
			DatabaseInterface.cleanupFailedCompilations();		   
			
			//Preprocess the compile file.
			if(runPreprocessing(fileToCompile) == false)
			{
				GuiLockingUtils.unlockGui();
				return null;
			}
			
			//If we should open the optimization wizard, handle that.
			final String componentType = getComponentTypeFromFile(fileToCompile);
			CompileWizard wizard =  new CompileWizard(fileToCompile, componentType, containsSystemCalls);
			
			if(!skipOptimizationWizard)
			{
				if(openOptimizationWizard(wizard, fileToCompile, componentType) == CANCELED)
				{
					MessageUtils.printlnConsoleError("Compilation Canceled.\n");
					GuiLockingUtils.unlockGui();
					return null;
				}
			}
			    
			//Before we do any compilation, let's cleanup any problems in the database first.
			DatabaseInterface.openConnection();
			
			String depth = "1";
			try
			{
				StringBuffer buffer = new StringBuffer();
				if(new File(wizard.getCompilerFlagsFile()).exists())
				{
					FileUtils.addFileContentsToBuffer(buffer, wizard.getCompilerFlagsFile());
					String line;
					while(true)
					{
						line = StringUtils.getNextStringValue(buffer);
						if(line == null || line.equals(""))
							break;
						if(line.equals("InlineAllModules"))
						{
							depth = StringUtils.getNextStringValue(buffer);
							if(depth.equals("INFINITE"))
								depth = "-1";
							break;
						}
					}
				}
			}
			catch(Exception e)
			{
				e.printStackTrace();
			}
			
			//Let's prepare to compile by not allowing any other compilations, getting the default display, and saving the distribution directory.
			setEnabled(false);
			final Display dis = Display.getCurrent();
			distributionDirectory = PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION);
			
			//Get all the necessary files we need to path to the compile script.
			final String filename = fileToCompile.getName().toString();
			final String compilerFlags = wizard.getCompilerFlagsFile();
			final String infoFile = !componentType.equals("SYSTEM")? wizard.getModuleInfoFile() : wizard.getSystemInfoFile();
			final String loCompilerFlags = wizard.getLoCompilerFlagsFile();
			final String debugValuesFile = DebugVariables.getDebugValuesFile(fileToCompile, !skipOptimizationWizard);
			final String preferenceFile = skipOptimizationWizard? fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "") + ".ROCCC/.preferenceFile" : getPreferenceFile(fileToCompile);
			final String pipelineFlags = fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "") + ".ROCCC/.timingInfo";
					
			//Create the compile info for this file in the database as well as any other file infos.
			createCompileInfo(fileToCompile, compilerFlags, loCompilerFlags, pipelineFlags, infoFile);
			
			try
			{
				int d = Integer.parseInt(depth);
				handlePreprocessingFileGeneration(functionsCalled, d);
			}
			catch(Exception e)
			{
				e.printStackTrace();
			}

			//If this is a module, delete it from the database.  Systems are ignored by the delete procedure.
			callDeleteScript(componentName, fileToCompile);
				
			// Changed by Jason to allow for the binary distribution
			
//			String prelimCompileScript = distributionDirectory + "/scripts/compile_hardware.sh" ;
//			if (new File(prelimCompileScript).exists() == false)
//			{
//				prelimCompileScript = distributionDirectory + "/Install/roccc-compiler/bin/compile_hardware.sh" ;
//			}
			
//			final String compileScript = prelimCompileScript ;
			
//			final String compileScript = distributionDirectory + "/Install/roccc-compiler/bin/compile_hardware.sh";
			
			//Put all the arguments we are going send to the compile script in an array so it knows how to 
			//split the arguments rather than relying on white space.
//			final String[] cmdArray = new String[8];
//			cmdArray[0] = compileScript;
//			cmdArray[1] = "" + fileToCompilePath + "";
//			cmdArray[2] = "" + compilerFlags + "";
//			cmdArray[3] = "" + infoFile + "";
//			cmdArray[4] = "" + preferenceFile + "";
//			cmdArray[5] = "" + loCompilerFlags + "";
//			cmdArray[6] = "" + debugValuesFile + "";
//			cmdArray[7] = "" + distributionDirectory + "";
			
			DatabaseInterface.closeConnection();
			
			//Set our shared variable to true to indication we are now compiling. 
			compilationAlive = true;
			
			t = new Thread(new Runnable()
			{	
				public void run()
				{
					//This will let us know if we errored somewhere during the compilation
					boolean errored = false;
				
					//We will run this once we are done compiling.
					Runnable postCompilationUpdate = new Runnable()
					{
						public void run()
						{
							try
							{
								//Cleanup the database for any files that didn't compile.
								if(DatabaseInterface.openConnection())
								{
									DatabaseInterface.updateAllListeners();
									DatabaseInterface.cleanupFailedCompilations();
									DatabaseInterface.closeConnection();
								}
								
								//If we are showing a message in the IPCores view saying the GUI is locked, let's
								//refresh the view now that the GUI should no longer be locked.
								if(ROCCC_IPCores.showingLockMessage)
								{
									//Close the views
									boolean IPCoresViewOpen = EclipseResourceUtils.isViewOpen(ROCCC_IPCores.ID);
									EclipseResourceUtils.closeView(ROCCC_IPCores.ID);
									boolean debugViewOpen = EclipseResourceUtils.isViewOpen(DebugVariables.ID);
									EclipseResourceUtils.closeView(DebugVariables.ID);
									
									GuiLockingUtils.unlockGui();
									
									//Open the views if they were previously open.
									if(IPCoresViewOpen)
										EclipseResourceUtils.openView(ROCCC_IPCores.ID);
									if(debugViewOpen)
										EclipseResourceUtils.openView(DebugVariables.ID);
								}
								else
								{
									GuiLockingUtils.unlockGui();
								}
								
								EclipseResourceUtils.updateProjectFiles();
							}
							catch(Exception e)
							{
								e.printStackTrace();
							}
						}
					};
					
					
					try
					{
						//This is where we actually run our compile.
						
						lock.lock() ;
						if (!compilationAlive)
						{
							lock.unlock() ;
							return ;
						}
					
						// The steps for compilation are the following:
						//  - Export the environment variables					
						//  - Create the vhdl directory if it does not already exist
						//  - Run createScript
						//  - Actually run the generated script
						//  - Remove the script
						//  - Compile through LLVM
						//  - Clean up extra files
					

						String filePath = fileToCompilePath.replace(fileToCompile.getName(), "") ;
						File workingDirectory = new File(filePath) ;
					
						Activator.OSType os = Activator.getOS();
						final String arch ;
						final String libraryExtension ;
												
						if (os == Activator.OSType.LION ||
							os == Activator.OSType.SNOW_LEOPARD ||
							os == Activator.OSType.LEOPARD ||
							os == Activator.OSType.MOUNTAIN_LION ||
							os == Activator.OSType.MAVERICK)
						{
							arch = "Darwin" ;
							libraryExtension = ".dylib" ;
						}
						else
						{
							arch = "Linux" ;
							libraryExtension = ".so.0" ;
						}
						final String originalPATH = System.getenv("PATH") ;
						final String originalLDLIBRARYPATH = System.getenv("LD_LIBRARY_PATH") ;
						final String originalDYLDLIBRARYPATH = System.getenv("DYLD_LIBRARY_PATH") ;
						String[] environment = new String[8] ;
						environment[0] = "ROCCC_LIBRARY_PATH=" + distributionDirectory + "/LocalFiles" ;
						environment[1] = "ROCCC_HOME=" + distributionDirectory ;
						environment[2] = "NCIHOME=" + distributionDirectory ;
						environment[3] = "MACHSUIFHOME=" + distributionDirectory ;
						environment[4] = "ARCH=" + arch ;
						environment[5] = "LD_LIBRARY_PATH=" + distributionDirectory + "/lib:" + originalLDLIBRARYPATH ;
						environment[6] = "PATH=" + distributionDirectory + "/bin:" + originalPATH ;
						environment[7] = "DYLD_LIBRARY_PATH=" + distributionDirectory + "/lib:" + originalDYLDLIBRARYPATH ;
					
						FileUtils.createFolder(filePath + "vhdl/") ;

						String[] createScript = new String[7] ;
						createScript[0] = distributionDirectory + "/bin/createScript" ;
						createScript[1] = fileToCompilePath ;
						createScript[2] = compilerFlags ;
						createScript[3] = infoFile ;
						createScript[4] = preferenceFile ;
						createScript[5] = debugValuesFile ;
						createScript[6] = distributionDirectory ;
										
						Process p1 = Runtime.getRuntime().exec(createScript, environment, workingDirectory) ;
						p1.waitFor() ;
					
						if (p1.exitValue() != 0)
						{
							errored = true ;
							lock.unlock();
							throw new Exception() ; // Just break out of the current block
						}
						
						// The previous step will have created a one shot script for us to run and stored it in the tmp directory under the distribution directory
						String[] chmodCmd = new String[3] ;
						chmodCmd[0] = "chmod" ;
						chmodCmd[1] = "700" ;
						chmodCmd[2] = distributionDirectory + "/tmp/compile_suif2hicirrf.sh" ;
						
						Process p2 = Runtime.getRuntime().exec(chmodCmd, environment, workingDirectory) ;
						p2.waitFor() ;
						
						if (p2.exitValue() != 0)
						{
							errored = true ;
							lock.unlock();
							throw new Exception() ; // Just break out of the current block
						}
						
						// Now actually execute the script
						String[] compileHicirrf = new String[2] ;
						compileHicirrf[0] = distributionDirectory + "/tmp/compile_suif2hicirrf.sh" ;
						compileHicirrf[1] = distributionDirectory ;
						
						Process p3 = Runtime.getRuntime().exec(compileHicirrf, environment, workingDirectory) ;
						BufferedReader compileScriptOutput = new BufferedReader(new InputStreamReader(p3.getInputStream())) ;
						BufferedReader compileScriptError = new BufferedReader(new InputStreamReader(p3.getErrorStream())) ;
						
						// Print out the error messages
						while (compileScriptOutput.ready() || compileScriptError.ready() || !Activator.isProcessDone(p3))
						{
							if (compileScriptOutput.ready())
							{
								String line = compileScriptOutput.readLine() ;
								if (line != null && !line.contains("indirect jmp without"))
									MessageUtils.printlnConsoleMessage(line) ;
							}
							if (compileScriptError.ready())
							{
								String line = compileScriptError.readLine();
								if (line != null && !line.contains("indirect jmp without")) 
									MessageUtils.printlnConsoleError(line) ;
							}
						}
					
//						while (compileScriptError.ready())
//						{
//							String line = compileScriptError.readLine();
//							if (line != null && !line.contains("indirect jmp without"))
//								MessageUtils.printlnConsoleError(line) ;
//						}

						p3.waitFor();
						
						if (p3.exitValue() != 0)
						{
							errored = true ;
							lock.unlock();
							throw new Exception() ;
						}
												
						// Remove the script from the file system
						String[] rmScript = new String[3] ;
						rmScript[0] = "rm" ;
						rmScript[1] = "-f" ;
						rmScript[2] = distributionDirectory + "/tmp/compile_suif2hicirrf.sh" ;
						
						Process p4 = Runtime.getRuntime().exec(rmScript, environment, workingDirectory) ;
						p4.waitFor() ;
						
						// Now compile to VHDL (this used to be done with compile_llvmtovhdl.sh and consists of the following steps)
						//  - run llvm-gcc on hi-cirrf.c
						//  - Create the passes that will be used as the low level optimizations

						String[] environmentLLVM = new String[9] ;
						environmentLLVM[0] = "ROCCC_LIBRARY_PATH=" + distributionDirectory + "/LocalFiles" ;
						environmentLLVM[1] = "ROCCC_HOME=" + distributionDirectory ;
						environmentLLVM[2] = "NCIHOME=" + distributionDirectory ;
						environmentLLVM[3] = "MACHSUIFHOME=" + distributionDirectory ;
						environmentLLVM[4] = "ARCH=" + arch ;
						environmentLLVM[5] = "LD_LIBRARY_PATH=" + distributionDirectory + "/lib:" + originalLDLIBRARYPATH ;
						environmentLLVM[6] = "PATH=" + distributionDirectory + "/bin/llvmGcc:" + distributionDirectory + "/bin:" + originalPATH ;
						environmentLLVM[7] = "DYLD_LIBRARY_PATH=" + distributionDirectory + "/lib:" + originalDYLDLIBRARYPATH ;
						environmentLLVM[8] = "DATABASE_PATH=" + distributionDirectory + "/LocalFiles/" ;
						
						String[] llvmgcc = new String[6] ;
						llvmgcc[0] = distributionDirectory + "/bin/llvmGcc/llvm-gcc" ;
						llvmgcc[1] = "-c" ;
						llvmgcc[2] = "-emit-llvm" ;
						llvmgcc[3] = filePath + "/hi_cirrf.c" ;
						llvmgcc[4] = "-o" ;
						llvmgcc[5] = filePath + "/hi_cirrf.c.bc" ;
						
						Process p5 = Runtime.getRuntime().exec(llvmgcc, environmentLLVM, workingDirectory) ;
						BufferedReader llvmgccOutput = new BufferedReader(new InputStreamReader(p5.getInputStream())) ;
						BufferedReader llvmgccError = new BufferedReader(new InputStreamReader(p5.getErrorStream())) ;
						p5.waitFor();
						
						while (llvmgccOutput.ready())
						{
							String line = llvmgccOutput.readLine();
							if (line != null && !line.contains("indirect jmp without"))
								MessageUtils.printlnConsoleError(line) ;
						}
						
						while (llvmgccError.ready())
						{
							String line = llvmgccError.readLine() ;
							if (line != null && !line.contains("indirect jmp without"))
								MessageUtils.printlnConsoleError(line) ;
						}

						// Construct a string that contains all of the passes
						StringBuffer lowFlagsBuffer = new StringBuffer();
						if(new File(loCompilerFlags).exists())
							FileUtils.addFileContentsToBuffer(lowFlagsBuffer, loCompilerFlags);
						String lowFlags = lowFlagsBuffer.toString();

						// Although the original script made use of file redirection, the actual command does not require it
						
						String[] optCmd = CompilationPass.CreateOptCmd(distributionDirectory, libraryExtension, lowFlags, filePath) ;
												
						Process p6 = Runtime.getRuntime().exec(optCmd, environmentLLVM, workingDirectory) ;
						BufferedReader optOutput = new BufferedReader(new InputStreamReader(p6.getInputStream())) ;
						BufferedReader optError = new BufferedReader(new InputStreamReader(p6.getErrorStream())) ;
												
						p6.waitFor();	
	
						// Print out any messages from running opt
						while (optOutput.ready())
						{
							if (optOutput.ready())
							{
								String line = optOutput.readLine() ;
								if (line != null)
								{
									MessageUtils.printlnConsoleMessage(line) ;
								}
							}
						}
					
						while (optError.ready())
						{
							String line = optError.readLine();
							if (line != null)
								MessageUtils.printlnConsoleError(line) ;
						}
												
						// If opt failed, remove the component from the hi-cirrf side of things
						if (p6.exitValue() != 0)
						{
							MessageUtils.printlnConsoleError("Low cirrf compilation failed") ;
							RemoveComponentPass.run(componentName) ;
							errored = true ;
							lock.unlock() ;
							throw new Exception() ;
						}

						// Moving all of the files into the vhdl directory is no longer needed
//						String[] mvCmd = new String[4] ;
//						mvCmd[0] = "mv" ;
//						mvCmd[1] = "-f" ;
//						mvCmd[2] = filePath + "/*.vhd?" ;
//						mvCmd[3] = filePath + "/vhdl/" ;
						
//						Process p7 = Runtime.getRuntime().exec(mvCmd, environmentLLVM, workingDirectory) ;
//						p7.waitFor() ;
																		
						// Clean up intermediate files
						String[] rmCmd2 = new String[4] ;
						rmCmd2[0] = "rm" ;
						rmCmd2[1] = "-f" ;
						rmCmd2[2] = filePath + "hi_cirrf*" ;
						rmCmd2[3] = filePath + "roccc.h" ;
												
						Process p8 = Runtime.getRuntime().exec(rmCmd2, environmentLLVM, workingDirectory) ;
						p8.waitFor() ;
						
						// Move suif file into the .ROCCC directory
						String[] mvCmd2 = new String[4] ;
						mvCmd2[0] = "mv" ;
						mvCmd2[1] = "-f" ;
						mvCmd2[2] = filePath + componentName + ".suif" ;
						mvCmd2[3] = filePath + ".ROCCC/" ;
						
						Process p9 = Runtime.getRuntime().exec(mvCmd2, environmentLLVM, workingDirectory) ;
						p9.waitFor() ;
						
						lock.unlock() ;
					}
					catch (Exception e)
					{
						//e.printStackTrace() ;
						errored = true ;
					}
					
					/*
					try 
					{		
						//Check our shared variable to see if we should cancel.
						lock.lock();
						if(!compilationAlive)
						{
							lock.unlock();
							return;	
						}
	
						//See that the compile script exists.
						File script = new File(compileScript);
						if(script.exists() == false)
						{
							MessageUtils.printlnConsoleError("Error: Script " + compileScript + " not found. Cannot compile.\n");
							dis.asyncExec(postCompilationUpdate);
							compilationAlive = false;
							lock.unlock();
							setEnabled(true);
							return;
						}
						
						//Run the compile script
						compileProcess = Runtime.getRuntime().exec(cmdArray);
						BufferedReader inputStream = new BufferedReader(new InputStreamReader(compileProcess.getInputStream()));
						BufferedReader errorStream = new BufferedReader (new InputStreamReader(compileProcess.getErrorStream()));
						
						//While the script is running, output any messages it sends.
						while(inputStream.ready() || !Activator.isProcessDone(compileProcess)) 
						{
							//If a message is ready, output it.
							if(inputStream.ready())
							{
								String line;
								if((line = inputStream.readLine()) != null)
									MessageUtils.printlnConsoleMessage(line);
							}
							
							//Let's unlock our lock to see if a cancel action was sent. If so, cancel.
							lock.unlock();
							lock.lock();
							if(!compilationAlive)
							{
								lock.unlock();
								return;	
							}
						}
		
						//Once the script is done, output all error messages. We do this after the script is done
						//to prevent any interleaving of the standard and error streams.
						while(errorStream.ready())
						{
							String line;
							if((line = errorStream.readLine()) != null)
							{
								if(!line.contains("indirect jmp without") && !line.equals("") && !line.equals(" "))
									MessageUtils.printlnConsoleError(line);
							}
							
							//Let's see if we were canceled by freeing the lock and seeing if anyone was waiting to lock it.
							lock.unlock();
							lock.lock();
							if(!compilationAlive)
							{
								lock.unlock();
								return;	
							}
						}
						
						inputStream.close();
						errorStream.close();
						
						//Let's see if we errored by checking the exit value of the process.
						errored = compileProcess.exitValue() == 0;
						
						lock.unlock();
					}
					catch (Exception err) 
					{
						err.printStackTrace();
					}
					*/
					//If we didn't error, copy the supplimentary VHDL files.
					if(!errored)
					{
						FileUtils.copyFile(new File(distributionDirectory + "/SupplementaryVHDL/ROCCChelper.vhdl"),
										   fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "vhdl/ROCCChelper.vhdl"));
						FileUtils.copyFile(new File(distributionDirectory + "/SupplementaryVHDL/microFifo.vhdl"), 
										   fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "vhdl/microFifo.vhdl")) ;
//						if(componentType.equals("SYSTEM"))
//							FileUtils.copyFile(new File(distributionDirectory + "/InferredBRAMFifo.vhdl"), fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "vhdl/InferredBRAMFifo.vhdl"));
					}
					
					//If we didn't skip the optimizations wizard, let's run our post compile update.
					if(!skipOptimizationWizard)
						dis.asyncExec(postCompilationUpdate);
					
					//Output either success or error
					if(!errored)
						MessageUtils.printlnConsoleSuccess("Compilation of " + filename + " finished.\n");
					else
						MessageUtils.printlnConsoleError("Compilation of " + filename + " failed after pre-processing.\n");
					
					if(!errored)
					{
						Display.getDefault().syncExec(new Runnable()
						{
							public void run()
							{
								DatabaseInterface.openConnection();
								GenerateCompilationReportPass.run(fileToCompile);
								DatabaseInterface.closeConnection();
								
								if(PreferenceUtils.getPreferenceBoolean(PreferenceConstants.OPEN_COMPILATION_REPORT_AFTER_COMPILE))
								{
									String srcFileFolder = FileUtils.getFolderOfFile(fileToCompile);
									EclipseResourceUtils.openFileInEditor(new File(srcFileFolder + componentName + "_Report.html"));
								}
							}
						});
					}
					//Reset our variables to show we are done.
					compilationAlive = false;
					setEnabled(true);
				}
			});
			t.start();
			return t;
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		return null;
	}
	
	private static void createCompileInfo(File f, String highCompilerFlagsFile, String lowCompilerFlagsFile, String pipelineFlagsFile, String infoFile)
	{
		try
		{
			DatabaseInterface.openConnection();
			
			if(DatabaseInterface.getComponentFromSourceFile(f) != null)
			{
				RemoveComponentPass.run(DatabaseInterface.getComponentFromSourceFile(f));
			}
		
			String timestamp = new Date().toString();
			
			String config = "Default";
			
			StringBuffer highFlagsBuffer = new StringBuffer();
			if(new File(highCompilerFlagsFile).exists())
				FileUtils.addFileContentsToBuffer(highFlagsBuffer, highCompilerFlagsFile);
			String highFlags = highFlagsBuffer.toString();
			
			StringBuffer lowFlagsBuffer = new StringBuffer();
			if(new File(lowCompilerFlagsFile).exists())
				FileUtils.addFileContentsToBuffer(lowFlagsBuffer, lowCompilerFlagsFile);
			String lowFlags = lowFlagsBuffer.toString();
			
			StringBuffer pipelineFlagsBuffer = new StringBuffer();
			if(new File(pipelineFlagsFile).exists())
				FileUtils.addFileContentsToBuffer(pipelineFlagsBuffer, pipelineFlagsFile);
			String pipelineFlags = pipelineFlagsBuffer.toString();
			
			StringBuffer infoFlagsBuffer = new StringBuffer();
			if(new File(infoFile).exists())
				FileUtils.addFileContentsToBuffer(infoFlagsBuffer, infoFile);
			String infoFlags = infoFlagsBuffer.toString();
			
			String ID = DatabaseInterface.createCompileInfo(config, timestamp, null, highFlags, lowFlags, pipelineFlags, infoFlags, false); 
			
			StringBuffer buffer = new StringBuffer();
			buffer.append(ID);
			
			FileUtils.createFileFromBuffer(buffer, new File(f.getAbsolutePath().replace(f.getName(), ".ROCCC/.compileInfo")));
			
			DatabaseInterface.createFileInfo(ID, f.getName(), "C_SOURCE", f.getAbsolutePath().replace(f.getName(), ""));
			if(suifFile != null)
				DatabaseInterface.createFileInfo(ID, suifFile.getName(), "SUIF_FILE", suifFile.getAbsolutePath().replace(suifFile.getName(), ""));
			
			for(int i = 0; i < functionsCalled.size(); ++i)
			{
				ResourceData data = functionsCalled.get(i);
				
				if(!DatabaseInterface.doesComponentExist(data.name))
					continue;
				
				DatabaseInterface.addResourceUsed(ID, "MODULE", new Integer(DatabaseInterface.getComponentID(data.name)).toString(), data.getAmount());
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		//DatabaseInterface.closeConnection();
	}
	
	public static String getComponentTypeFromFile(File fileToCompile)
	{
		String componentType = "";
		
		try
		{
			if(fileToCompile.getAbsolutePath().contains("/src/modules/"))
			{
				componentType = "MODULE";
			}
			else if(fileToCompile.getAbsolutePath().toString().contains("/src/systems/"))
			{
				componentType = "SYSTEM";
			}
			else if(fileToCompile.getAbsolutePath().toString().contains("/src/intrinsics/"))
			{
				componentType = fileToCompile.getAbsolutePath().toString().split("/src/intrinsics/")[1];
				componentType = componentType.split("/")[0];
				componentType = componentType.toUpperCase();
			}
			else
			{
				MessageDialog.openError(new Shell(), "Compiler Error", "You can only compile modules and systems. If you are selecting a module or system, make sure it is in a proper directory structure.");
				GuiLockingUtils.unlockGui();
				return null;
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return componentType;
	}
	
	public static void handlePreprocessingFileGeneration(Vector<ResourceData> functionsCalled, int depth)
	{
		try
		{
			StringBuffer buffer = new StringBuffer();
			
			Vector<String> modulesCalledInHierarchy = new Vector<String>();
			
			for(int i = 0; i < functionsCalled.size(); ++i)
			{
				if(!modulesCalledInHierarchy.contains(functionsCalled.get(i).name))
					modulesCalledInHierarchy.add(functionsCalled.get(i).name);
			
			}
			
			Vector<String> modulesVisited = new Vector<String>();
			
			//If we are given an infinte depth, just grab all the functions found through all the modules and add them
			//to our modulesCalledInHierarchy list and set the depth to 1 since we already grabbed all the modules in the hierarchy.
			
			if(depth == -1)
			{
				//System.out.println("Infinite Inline Detected, grabbing hierarchy...");
				Vector<String> newModules = new Vector<String>();
				Vector<String> seenModules = new Vector<String>();
				for(int i = 0; i < modulesCalledInHierarchy.size(); ++i)
				{	
					if(!DatabaseInterface.doesComponentExist(modulesCalledInHierarchy.get(i)))
					{
						String version = DatabaseInterface.versionCompiledOn(modulesCalledInHierarchy.get(i));
						if(version.equals("") || version.equals("NA"))
							continue;
						if(seenModules.contains(modulesCalledInHierarchy.get(i)))
							continue;
					}
						
					seenModules.add(modulesCalledInHierarchy.get(i));
					
					//System.out.println("Checking Heiarchy for " + modulesCalledInHierarchy.get(i));
					String[] allModulesForFunction = DatabaseInterface.getModulesCalledHierarchy(modulesCalledInHierarchy.get(i));
					for(int j = 0; j < allModulesForFunction.length; ++j)
					{
						if(!allModulesForFunction[j].equals(modulesCalledInHierarchy.get(i)) && !seenModules.contains(allModulesForFunction[j]))
						{
							//System.out.println("Fuction " + allModulesForFunction[j] + " found. Adding to list.");
							seenModules.add(allModulesForFunction[j]);
							newModules.add(allModulesForFunction[j]);
						}
					}
				}
				
				//Add all the new modules we found to our functions called list.
				for(int i = 0; i < newModules.size(); ++i)
					modulesCalledInHierarchy.add(newModules.get(i));
				depth = 1;
				
				//System.out.println("All modules found, transitioning to grabbing suif files.\n");
			}
			
			for(int i = 0; i < depth; ++i)
			{
				//Save all the modules the functions called in this iteration
				Vector<String> newModules = new Vector<String>();
				
				for(int j = 0; j < modulesCalledInHierarchy.size(); ++j)
				{
					//If we have already seen this module, skip it.
					if(modulesVisited.contains(modulesCalledInHierarchy.get(j)))
						continue;
					else if(!DatabaseInterface.doesComponentExist(modulesCalledInHierarchy.get(j)) || !DatabaseInterface.getComponentType(modulesCalledInHierarchy.get(j)).equals("MODULE"))
						continue;
					else if(DatabaseInterface.getSuifFile(modulesCalledInHierarchy.get(j)) == null || new File(DatabaseInterface.getSuifFile(modulesCalledInHierarchy.get(j))).exists() == false)
						continue;
					
					String version = DatabaseInterface.versionCompiledOn(modulesCalledInHierarchy.get(j));
					if(version.equals("") || version.equals("NA"))
						continue;
					
					modulesVisited.add(modulesCalledInHierarchy.get(j));
					
					//System.out.println("Grabbing suif for " + modulesCalledInHierarchy.get(j));
				
					//If this is the interation of the lining depth, get all the modules the current function calls.
					if(i != depth - 1)
					{
						String[] modulesCalledFromCurrent = DatabaseInterface.getModulesCalled(modulesCalledInHierarchy.get(j));
						for(int k = 0; k < modulesCalledFromCurrent.length; ++k)
							if(!modulesCalledFromCurrent.equals(modulesCalledInHierarchy.get(j)))
								newModules.add(modulesCalledFromCurrent[k]);
					}
					
					//Append all the suif files.
					String suif = DatabaseInterface.getSuifFile(modulesCalledInHierarchy.get(j));
					buffer.append(modulesCalledInHierarchy.get(j) + " " + suif + "\n");
				}
				
				//The next functions we are going to check are all the new ones we just found.
				modulesCalledInHierarchy = newModules;
			}
	
			//System.out.println("Writting Suif files.\n");
			FileUtils.createFileFromBuffer(buffer, new File(Activator.getDistributionFolder() + "/tmp/.preprocessInfo"));
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private static boolean runPreprocessing(final File fileToCompile)
	{
		try
		{
			functionsCalled.removeAllElements();
			labelsPlaced.removeAllElements();
			suifFile = null;
			
			// First, check locally for binary installs.  If we don't find it, then check the original location
			//  Added by Jason, the original code is inside the first if.
/*			
			String actualScript = PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/scripts/preprocess.sh" ;

			if (new File(actualScript).exists() == false)
			{
				actualScript = PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/Install/roccc-compiler/bin/preprocess.sh" ;
				if(new File(actualScript).exists() == false)
				{
					MessageUtils.printlnConsoleError("Error: Script " + actualScript + " does not exist. Cannot run.");
					return false;
				}
			
			}
			
			final String script = actualScript ;
*/				
//			final String script = PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/Install/roccc-compiler/bin/preprocess.sh";
				
			
			preProcessSucceeded = true;
			final Lock l = new ReentrantLock(true);
			final String fileLoc = fileToCompile.getAbsolutePath();
			final String distro = Activator.getDistributionFolder() + "/";
			final String fileName = fileToCompile.getName();
			final String fileBaseName = fileName.replace(fileName.substring(fileName.length()-2, fileName.length()), "") ;
			
			Thread t = new Thread(new Runnable()
			{
				public void run()
				{
					
					try
					{
						// The steps that the preprocess script does are the following:
						//  - Create the .ROCCC directory if it does not yet exist
						//  - Export all the necessary variables
						//  - Call gcc2suif
						//  - Add the name of the suif file to preprocessInfo
						//  - Execute suifdriver
						//  - Move the suif file into the correct location
						//  - Create the preprocessInfo file

						final String sourceDirectory = fileLoc.replace(fileToCompile.getName(), "") ;
						final String rocccDirectory = fileLoc.replace(fileToCompile.getName(), ".ROCCC") ;
												
						File workingDirectory = new File(sourceDirectory) ;
						FileUtils.createFolder(rocccDirectory) ;

						Activator.OSType os = Activator.getOS();
						final String arch ;
						if (os == Activator.OSType.LION || os == Activator.OSType.SNOW_LEOPARD || os == Activator.OSType.LEOPARD)
							arch = "Darwin" ;
						else
							arch = "Linux" ;
						final String originalPATH = System.getenv("PATH") ;
						final String originalLDLIBRARYPATH = System.getenv("LD_LIBRARY_PATH") ;
						final String originalDYLDLIBRARYPATH = System.getenv("DYLD_LIBRARY_PATH") ;
						String[] environment = new String[8] ;
						environment[0] = "ROCCC_LIBRARY_PATH=" + distro + "LocalFiles" ;
						environment[1] = "ROCCC_HOME=" + distro ;
						environment[2] = "NCIHOME=" + distro ;
						environment[3] = "MACHSUIFHOME=" + distro ;
						environment[4] = "ARCH=" + arch ;
						environment[5] = "LD_LIBRARY_PATH=" + distro + "/lib:" + originalLDLIBRARYPATH ;
						environment[6] = "PATH=" + distro + "/bin:" + originalPATH ;
						environment[7] = "DYLD_LIBRARY_PATH=" + distro + "/lib:" + originalDYLDLIBRARYPATH ;

						String[] gcc2suif = new String[3] ;
						gcc2suif[0] = distro + "bin/gcc2suif" ;
						gcc2suif[1] = distro ;
						gcc2suif[2] = fileToCompile.getName() ;
											
						Process p1 = Runtime.getRuntime().exec(gcc2suif, environment, workingDirectory) ;
						
						BufferedReader gcc2suifOut = new BufferedReader(new InputStreamReader(p1.getInputStream())) ;
						BufferedReader gcc2suifError = new BufferedReader(new InputStreamReader(p1.getErrorStream())) ;
						
						while (gcc2suifOut.ready() || !Activator.isProcessDone(p1))
						{
							if (gcc2suifOut.ready())
							{
								String line = gcc2suifOut.readLine() ;
								if (line != null)
								{
									MessageUtils.printlnConsoleMessage(line) ;
								}
							}
						}
						
						while (gcc2suifError.ready() || !Activator.isProcessDone(p1))
						{
							if (gcc2suifError.ready())
							{
								String line = gcc2suifError.readLine();
								if (line != null && !line.contains("indirect jmp without"))							
									MessageUtils.printlnConsoleMessage(line) ;
							}
						}
						p1.waitFor() ;
					
						if (p1.exitValue() != 0)
						{
							MessageUtils.printlnConsoleMessage("preProcess failed p1");
							preProcessSucceeded = false ;
						}
						StringBuffer preprocessInfo = new StringBuffer() ;
						preprocessInfo.append("SUIF_FILE " + rocccDirectory + "/" + fileBaseName + ".suif\n") ;
						
						String[] suifdriver = new String[3] ;
						suifdriver[0] = distro + "bin/suifdriver" ;
						suifdriver[1] = "-e" ;
						suifdriver[2] = "require gcc_preprocessing_transforms global_transforms ;" +
										"load " + fileBaseName + ".suif ; " +
										"EvalTransformPass ; RobyPreprocessingPass ;" ;
						Process p2 = Runtime.getRuntime().exec(suifdriver, environment, workingDirectory) ;
						BufferedReader suifdriverOutput = new BufferedReader(new InputStreamReader(p2.getInputStream())) ;
						BufferedReader suifdriverError = new BufferedReader(new InputStreamReader(p2.getErrorStream())) ;
						p2.waitFor() ;

						if (p2.exitValue() != 0)
						{
							MessageUtils.printlnConsoleMessage("preProcess failed p2");
							preProcessSucceeded = false ;
						}
						String nextLine = suifdriverOutput.readLine();
						while (nextLine != null)
						{
							preprocessInfo.append(nextLine + "\n") ;
							nextLine = suifdriverOutput.readLine() ;
						}
					
						nextLine = suifdriverError.readLine();
						while (nextLine != null) 
						{
							preprocessInfo.append(nextLine + "\n") ;
							nextLine = suifdriverError.readLine();
						}
						
						String[] mvCmd = new String[3] ;
						mvCmd[0] = "mv" ;
						mvCmd[1] = fileBaseName + ".suif" ;
						mvCmd[2] = fileLoc + "/.ROCCC/" ;
						Process p3 = Runtime.getRuntime().exec(mvCmd, environment, workingDirectory) ;
						p3.waitFor() ;						
						
						File preprocessInfoFile = new File(rocccDirectory + "/.preprocessInfo") ;
						FileUtils.createFileFromBuffer(preprocessInfo, preprocessInfoFile) ;
						
					}
					catch (Exception e)
					{
						e.printStackTrace();
					}
					if(!preProcessSucceeded)
						MessageUtils.printlnConsoleError("Compilation of " + fileToCompile.getName() + " failed during pre-processing.\n");
					l.lock();
					
					/*
					try 
					{		
						
						String[] cmdArray = new String[5];
						cmdArray[0] = script;
						cmdArray[1] = fileLoc;
						cmdArray[2] = distro;
						cmdArray[3] = fileLoc.replace(fileToCompile.getName(), ".ROCCC");
						cmdArray[4] = fileLoc.replace(fileToCompile.getName(), "");
										
						Process p = Runtime.getRuntime().exec(cmdArray);
						BufferedReader inputStream = new BufferedReader(new InputStreamReader(p.getInputStream()));
						BufferedReader errorStream = new BufferedReader (new InputStreamReader(p.getErrorStream()));
						
						while(!Activator.isProcessDone(p));
						
						p.waitFor();
						
						if(p.exitValue() == 0)
							preProcessSucceeded = false;
					
						while(errorStream.ready() && !preProcessSucceeded)
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
						
					}
					catch (Exception err) 
					{
						err.printStackTrace();
					}
					
					if(!preProcessSucceeded)
						MessageUtils.printlnConsoleError("Compilation of " + fileToCompile.getName() + " failed.\n");
					l.lock();
				*/	
				}
			});
			
			t.start();
		
			for(int count = 0; count < 5000 && l.tryLock(); ++count)
			{
				l.unlock();
				try { Thread.sleep(100); } 
				catch (InterruptedException e) { e.printStackTrace(); }
			}
			
			if(!preProcessSucceeded)
				return false;
			
			//Open the file
			File preProcessFile = new File(fileLoc.replace(fileToCompile.getName(), "") +  ".ROCCC/.preprocessInfo");
			if(preProcessFile.exists() == false)
			{
				MessageUtils.printlnConsoleError("Error: Cannot find preprocesser file " + preProcessFile.getName() + ". ROCCC cannot compile this file.");
				return false;
			}
			
			componentName = null;
			
			StringBuffer buffer = new StringBuffer();
			FileUtils.addFileContentsToBuffer(buffer, preProcessFile.getAbsolutePath());
			while(buffer.length() > 0)
			{
				String type = "";
				while(buffer.length() > 0 && !type.equals("FUNCTION") && !type.equals("SUIF_FILE") && !type.equals("LABEL") && !type.equals("COMPONENT_NAME"))
					type = StringUtils.getNextStringValue(buffer);
				
				if(type.equals("LABEL"))
					labelsPlaced.add(StringUtils.getNextStringValue(buffer));
				else if(type.equals("FUNCTION"))
				{
					boolean existed = false;
					String name = StringUtils.getNextStringValue(buffer);
					
					for(int i = 0; i < functionsCalled.size(); ++i)
					{
						if(functionsCalled.get(i).name.equals(name))
						{
							functionsCalled.get(i).setAmount(functionsCalled.get(i).getAmount() + 1);
							existed = true;
							break;
						}
					}	
					
					if(!existed)
						functionsCalled.add(new ResourceData(name, 1));
				}
				else if(type.equals("SUIF_FILE"))
					suifFile = new File(StringUtils.getNextStringValue(buffer));
				else if(type.equals("COMPONENT_NAME"))
					componentName = StringUtils.getNextStringValue(buffer);
			}
			
			
			DatabaseInterface.openConnection();
			
			//See if this component is trying to call a system.
			for(int i = 0; i < functionsCalled.size(); ++i)
			{
				if(DatabaseInterface.doesComponentExist(functionsCalled.get(i).name) && DatabaseInterface.getComponentType(functionsCalled.get(i).name).equals("SYSTEM"))
					containsSystemCalls = true;
			}
			
			//Write to the file the suifFiles for the function calls
			Vector<String> outOfDateComponents = new Vector<String>();
			for(int i = 0; i < functionsCalled.size(); ++i)
			{
				//String suif = DatabaseInterface.getSuifFile(functionsCalled.get(i));
				//buffer.append(functionsCalled.get(i) + " " + suif + "\n");
				if(!DatabaseInterface.doesComponentExist(functionsCalled.get(i).name))
					continue;
					
				String versionString = DatabaseInterface.versionCompiledOn(functionsCalled.get(i).name);
				if(versionString.equals("NA"))
					continue;
				
				if(versionString.equals("") || new Version(versionString).compareTo(Activator.getMinimumCompilerVersionNeeded()) < 0)
					outOfDateComponents.add(functionsCalled.get(i).name);
			}
			
			DatabaseInterface.closeConnection();
			//FileUtils.createFileFromBuffer(buffer, new File(Activator.getDistributionFolder() + "/tmp/.preprocessInfo"));
			
			if(outOfDateComponents.size() > 0)
			{
				String message = "The following sub-components in " + fileToCompile.getName() + " were compiled in a previous version of ROCCC:\n\n\t";
				for(int i = 0; i < outOfDateComponents.size(); ++i)
				{
					message += (i == 0? "" : ", ") + outOfDateComponents.get(i); 
				}
				
				message += "\n\nThese must be recompiled before they can be used.";
				
				MessageUtils.openErrorWindow("Out of Date Error", message);
				MessageUtils.printlnConsoleError("Compilation Canceled");
				preProcessSucceeded = false;
			}
			
			return preProcessSucceeded;
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return false;
	}
	
	public static boolean openOptimizationWizard(CompileWizard wizard, File fileToCompile, String componentType)
	{
		ISelection selection = null;
		IWorkbench wb = PlatformUI.getWorkbench();
		IWorkbenchWindow window = wb.getActiveWorkbenchWindow();
		
		try
		{
			wizard.init(functionsCalled, labelsPlaced);
				
			WizardDialog dialog = new WizardDialog(window.getShell(), wizard);
			dialog.setHelpAvailable(false);
			
		
		
			if(dialog.open() == Window.CANCEL)
			{
				return false;
			}
			
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return true;
	}
	
	public static boolean checkFilePermissions(String fileToCompilePath)
	{
		File f = new File(fileToCompilePath);
		if(!f.canWrite())
		{
			return false;
		}
		
		return true;
	}
	
	public static Process getCompileProcess()
	{
		return compileProcess;
	}
	
	private static String getPreferenceFile(File fileToCompile)
	{
		StringBuffer buffer = new StringBuffer();
	
		buffer.append("COMPILER_VERSION " + Activator.getCompilerVersion() + "\n");
	
		FileUtils.createFileFromBuffer(buffer, new File(FileUtils.getFolderOfFile(fileToCompile) + "/.ROCCC/.preferenceFile"));
		
		return fileToCompile.getAbsolutePath().replace(fileToCompile.getName(), "") + ".ROCCC/.preferenceFile";
	}
	
	private static void callDeleteScript(String componentName, File fileToCompile)
	{	
		//Check for a vhdl file to show this file was compiled.
		if(componentName == null)
			componentName = DatabaseInterface.getComponentFromSourceFile(fileToCompile);
		try 
		{
			if(!DatabaseInterface.doesIPCoreExist(componentName))
			{
				DatabaseInterface.closeConnection();
				return;
			}
		}
		catch (SQLException e1) 
		{
			e1.printStackTrace();
		}
		
		DatabaseInterface.closeConnection();
		
		// Added by Jason to handle the new binary structure.
		String prelimScript = PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/scripts/remove_module.sh" ;
		if (new File(prelimScript).exists() == false)
		{
			prelimScript = PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/Install/roccc-compiler/bin/remove_module.sh" ;
		}

		final String script = prelimScript ;
		
//		final String script = PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/Install/roccc-compiler/bin/remove_module.sh";
		
		final String scriptExecutable = script + " " + componentName + " " + distributionDirectory;
			
		final Lock l = new ReentrantLock(true);
		Thread t = new Thread(new Runnable()
		{
			public void run()
			{
				try 
				{		
					Process p = Runtime.getRuntime().exec(scriptExecutable);
					BufferedReader inputStream = new BufferedReader(new InputStreamReader(p.getInputStream()));
					BufferedReader errorStream = new BufferedReader (new InputStreamReader(p.getErrorStream()));
					
					while(inputStream.ready() || !Activator.isProcessDone(p)) 
					{
						String line;
						if((line = inputStream.readLine()) != null)
						{
							MessageUtils.printlnConsoleMessage(line);
						}
					}
					while(errorStream.ready())
					{
						String line;
						if((line = errorStream.readLine()) != null)
						{
							MessageUtils.printlnConsoleError(line);
						}
					}
					inputStream.close();
					errorStream.close();
					p.waitFor();
					
				}
				catch (Exception err) 
				{
					err.printStackTrace();
				}
				l.lock();
			}
		});
		t.start();
		
		//Keep grabbing the lock and releasing it until the script thread finally
		//grabs it indicating the script thread is finished. We will only try
		//this for a finite amount of time where after we will assume the script
		//thread failed and just continue on.
		for(int count = 0; count < 50 && l.tryLock(); ++count)
		{
			l.unlock();
			try { Thread.sleep(100); } 
			catch (InterruptedException e) { e.printStackTrace(); }
		}
	
		DatabaseInterface.openConnection();
		DatabaseInterface.removeComponent(componentName);
		DatabaseInterface.closeConnection();
	}
	
	public static boolean canCompile()
	{
		return !compilationAlive;
	}
	
	static void setEnabled(boolean e)
	{
		enabled = e;
	}
	
	public static boolean cancelCompile()
	{
		lock.lock();
		if(compilationAlive == false) 
		{
			lock.unlock();
			return false;
		}
		compilationAlive = false;
		enabled = true;

		lock.unlock();
		
		return true;
	}
}
