package rocccplugin.actions;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

import rocccplugin.Activator;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.PreferenceUtils;

public class CancelCompile 
{
	public static void run(boolean displayMessages)
	{
		//Get the script for canceling.
		// Changed by Jason to handle the new binary distribution
		String executable = PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/scripts/cancel_build.sh" ;
		File script = new File(executable) ;
		if (!script.exists())
		{
			script = new File(PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/Install/roccc-compiler/bin/cancel_build.sh") ;
		}
		if(!script.exists())
		{
			if(displayMessages)
				MessageUtils.printlnConsoleError("Error: Script " + executable + " was not found. Build cannot be canceled.\n");
			return;
		}
			
		//Cancel the script.
		boolean canceled = CompilationPass.cancelCompile();
		
		if(canceled == false)
		{
			return;
		}
		
		try 
		{
			String[] cmdArray = new String[1];
			cmdArray[0] = executable;
			Process p = Runtime.getRuntime().exec(cmdArray);
			
//			BufferedReader cancelScriptOutput = new BufferedReader(new InputStreamReader(p.getInputStream())) ;
//			BufferedReader cancelScriptError = new BufferedReader(new InputStreamReader(p.getErrorStream())) ;
//			
//			while(cancelScriptOutput.ready() || cancelScriptError.ready() || !Activator.isProcessDone(p))
//			{
//				if (cancelScriptOutput.ready())
//				{
//					String line = cancelScriptOutput.readLine();
//					if (line != null)
//						MessageUtils.printlnConsoleMessage(line) ;
//				}
//				if (cancelScriptError.ready())
//				{
//					String line = cancelScriptError.readLine() ;
//					if (line != null)
//						MessageUtils.printlnConsoleError(line) ;
//				}
//			}
//			p.waitFor();			
			
		} 
		catch (IOException e) 
		{
			e.printStackTrace();
		}
		
		if(displayMessages)
			MessageUtils.printlnConsoleError("Current Build Canceled.\n");
	}
}
