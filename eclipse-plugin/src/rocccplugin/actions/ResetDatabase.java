package rocccplugin.actions;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;

import rocccplugin.Activator;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.MessageUtils;

public class ResetDatabase 
{
	static public void run()
	{
		String RootDirectory = Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION) ;
		String LocalFiles = RootDirectory + "/LocalFiles" ;
		String BinaryDirectory = RootDirectory + "/bin" ;
	
		// The tasks that the reset script must do are as follows:
		//  - Copy the blank.suif file over the repository.suif file
		//  - Copy the blank.h file over the roccc-library.h file
		//  - Remove the vhdlLibrary.sql3 file
		//  - Touch a new vhdlLibrary file
		//  - Run the initializeFP binary program
		
		String[] cpCmd1 = new String[3] ;
		cpCmd1[0] = "cp" ;
		cpCmd1[1] = LocalFiles + "/blank.suif" ;
		cpCmd1[2] = LocalFiles + "/repository.suif" ;
		
		String[] cpCmd2 = new String[3] ;
		cpCmd2[0] = "cp" ;		
		cpCmd2[1] = LocalFiles + "/blank.h" ;
		cpCmd2[2] = LocalFiles + "/roccc-library.h" ;
		
		String[] rmCmd = new String[2] ;
		rmCmd[0] = "rm" ;
		rmCmd[1] = LocalFiles + "/vhdlLibrary.sql3" ;
		
		String[] touchCmd = new String[2] ;
		touchCmd[0] = "touch" ;
		touchCmd[1] = LocalFiles + "/vhdlLibrary.sql3" ;
		
		String[] initializeCmd = new String[2] ;
		initializeCmd[0] = BinaryDirectory + "/initializeFP" ;
		initializeCmd[1] = LocalFiles + "/vhdlLibrary.sql3" ;
		
		try
		{
			Process p1 = Runtime.getRuntime().exec(cpCmd1) ;
			p1.waitFor() ;
			
			Process p2 = Runtime.getRuntime().exec(cpCmd2) ;
			p2.waitFor();
			
			Process p3 = Runtime.getRuntime().exec(rmCmd) ;
			p3.waitFor() ;
			
			Process p4 = Runtime.getRuntime().exec(touchCmd) ;
			p4.waitFor() ;
			
			Process p5 = Runtime.getRuntime().exec(initializeCmd) ;
			p5.waitFor() ;
		}
		catch (Exception e)
		{
			e.printStackTrace() ;
		}
		
		MessageUtils.printlnConsoleSuccess("Finished - ROCCC Database Reset.\n") ;
		
		/*
		String msg = "Finished - ROCCC Database Reset.\n";
		
		String executable = Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/scripts/reset-compiler.sh" ;

		if (new File(executable).exists() == false)
		{
			executable = Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/Install/roccc-compiler/bin/reset-compiler.sh";
		}
		
		File script = new File(executable);
		if(script.exists() == false)
		{
			MessageUtils.printlnConsoleError("Error: Script " + executable + " not found. Cannot compile.\n");
			return;
		}
		
		//Create a new process that calls the Reset Compiler script.
		try  
		{
			//Run the script.
			String[] cmdArray = new String[2];
			cmdArray[0] = executable;
			cmdArray[1] = Activator.getDefault().getPreferenceStore().getString(PreferenceConstants.ROCCC_DISTRIBUTION);
			
			Process p = Runtime.getRuntime().exec(cmdArray);
			//Get the input and error streams from the process running the script.
			BufferedReader input = new BufferedReader(new InputStreamReader(p.getInputStream()));
			BufferedReader error = new BufferedReader (new InputStreamReader(p.getErrorStream()));
			
			//Continue outputting the streams from the process until it is done. 
			while(input.ready() || !Activator.isProcessDone(p)) 
			{
				String line;
				if((line = input.readLine()) != null)
					MessageUtils.printlnConsoleMessage(line);
				
			}
			
			while(error.ready())
			{
				String line = "";
				if((line = error.readLine()) != null)
					MessageUtils.printlnConsoleError(line);
			}
			
			//Close the streams from the process.
			input.close();			
			error.close();

		} 
		catch (Exception e) 
		{
			e.printStackTrace();
		}
	
		MessageUtils.printlnConsoleSuccess(msg);	
		*/
	}
}
