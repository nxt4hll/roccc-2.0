package rocccplugin.actions;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;

import org.eclipse.swt.widgets.Display;

import rocccplugin.Activator;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.MessageUtils;

public class RemoveComponentPass 
{
	static public void run(String componentName)
	{
		// The steps that the script performs are as follows:
		//  - Set up the environment variables
		//  - Run gcc2suif on blank.c
		//  - Run suifdriver with the appropriate commands
		
		Activator.OSType os = Activator.getOS();
		final String arch ;
		if (os == Activator.OSType.LION || os == Activator.OSType.SNOW_LEOPARD || os == Activator.OSType.LEOPARD)
			arch = "Darwin" ;
		else
			arch = "Linux" ;
		
		final String innerComponentName = componentName ;
		final String rootDirectory = Activator.getDistributionFolder() ;
		final String originalPATH = System.getenv("PATH") ;
		final String originalLDLIBRARYPATH = System.getenv("LD_LIBRARY_PATH") ;
		final String originalDYLDLIBRARYPATH = System.getenv("DYLD_LIBRARY_PATH") ;
		
		Thread t = new Thread(new Runnable()
		{
			public void run()
			{
				try 
				{
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
					
//					BufferedReader gcc2suifOut = new BufferedReader(new InputStreamReader(p1.getInputStream())) ;
//					BufferedReader gcc2suifError = new BufferedReader(new InputStreamReader(p1.getErrorStream())) ;
//					
//					while (gcc2suifOut.ready() || !Activator.isProcessDone(p1))
//					{
//						if (gcc2suifOut.ready())
//						{
//							String line = gcc2suifOut.readLine() ;
//							if (line != null)
//							{
//								MessageUtils.printlnConsoleMessage(line) ;
//							}
//						}
//					}
//					
//					while (gcc2suifError.ready())
//					{
//						String line = gcc2suifError.readLine();
//						if (line != null)
//							MessageUtils.printlnConsoleMessage(line) ;
//					}

					String[] suifdriver = new String[3] ;
					suifdriver[0] = rootDirectory + "/bin/suifdriver" ;
					suifdriver[1] = "-e" ;
					suifdriver[2] = "require basicnodes suifnodes cfenodes transforms control_flow_analysis ;"  +
									"require jasonOutputPass libraryOutputPass global_transforms utility_transforms array_transforms ; " +
									"require bit_vector_dataflow_analysis gcc_preprocessing_transforms verifyRoccc ;" +
									"require preprocessing_transforms data_dependence_analysis ;" +
									"require fifoIdentification ;" +
									"load " + rootDirectory + "/LocalFiles/blank.suif ;" +
									"CleanRepositoryPass " + rootDirectory + "/LocalFiles ;" +
									"RemoveModulePass " + innerComponentName + " " + rootDirectory + "/LocalFiles ;" +
									"DumpHeaderPass " + rootDirectory + "/LocalFiles ;" ;
					
					Process p2 = Runtime.getRuntime().exec(suifdriver, environment) ;
					p2.waitFor() ;
										
//					BufferedReader suifdriverOut = new BufferedReader(new InputStreamReader(p2.getInputStream())) ;
//					BufferedReader suifdriverError = new BufferedReader(new InputStreamReader(p2.getErrorStream())) ;
//					
//					while (suifdriverOut.ready() || !Activator.isProcessDone(p2))
//					{
//						if (suifdriverOut.ready())
//						{
//							String line = suifdriverOut.readLine() ;
//							if (line != null)
//							{
//								MessageUtils.printlnConsoleMessage(line) ;
//							}
//						}
//					}
//					
//					while (suifdriverError.ready())
//					{
//						String line = suifdriverError.readLine();
//						if (line != null)
//							MessageUtils.printlnConsoleMessage(line) ;
//					}

					
				}
				catch(Exception e)
				{
					e.printStackTrace() ;
				}
			}
			
		}) ;
		
		t.start() ;
		try
		{
			t.join(5000) ;
		}
		catch (InterruptedException e)
		{
			e.printStackTrace() ;
		}
		DatabaseInterface.removeComponent(componentName) ;
		
		/*
		String script = Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/scripts/remove_module.sh" ;
		if (new File(script).exists() == false)
		{
			script = Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/Install/roccc-compiler/bin/remove_module.sh";
		}
		
		File scriptFile = new File(script);
		if(scriptFile.exists() == false)
		{
			MessageUtils.printlnConsoleError("Error: Script " + script + " does not exist, cannot remove component from roccc-library.h\n");
			return;
		}
		
		String compName = componentName;
		final String executable = script + " " + compName + " " + Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION);
		
		final Display dis = Display.getDefault();
		
		Thread t = new Thread(new Runnable()
		{
			private boolean isProcessDone(Process p)
			{
				boolean ret = false;
				try
				{
					p.exitValue();
					ret = true;
				}
				catch(IllegalThreadStateException e)
				{
				}
				return ret;
			}
			public void run()
			{
				try 
				{		
					Process p = Runtime.getRuntime().exec(executable);
					BufferedReader inputStream = new BufferedReader(new InputStreamReader(p.getInputStream()));
					BufferedReader errorStream = new BufferedReader (new InputStreamReader(p.getErrorStream()));
					
					while(inputStream.ready() || !isProcessDone(p)) 
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
			}
		});

		t.start();
		
		try 
		{
			t.join(5000);
		} 
		catch (InterruptedException e) 
		{
			e.printStackTrace();
		}
		
		DatabaseInterface.removeComponent(componentName);
		*/
	}
}
