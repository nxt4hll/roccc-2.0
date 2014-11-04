package rocccplugin.utilities;

import java.io.File;

import rocccplugin.database.DatabaseInterface;

public class ValidFileUtilities 
{
	public static boolean isPotentiallyValidROCCCFile(File fileToGenerateInterfaceFor)
	{
		if(!fileToGenerateInterfaceFor.getName().endsWith(".c") || fileToGenerateInterfaceFor.getName().equals("hi_cirrf.c"))
			return false;
		return true;
	}
	
	public static boolean isCompiledInDatabase(File fileToGenerateInterfaceFor)
	{
		String componentName = DatabaseInterface.getComponentFromSourceFile(fileToGenerateInterfaceFor);
		
		return componentName != null;
	}
	
	public static boolean compiledVHDLExistsForFile(File fileToGenerateInterfaceFor)
	{
		String componentName = DatabaseInterface.getComponentFromSourceFile(fileToGenerateInterfaceFor);
		
		return compiledVHDLExistsForComponent(componentName);
	}
	
	public static boolean compiledVHDLExistsForComponent(String componentName)
	{
		if(componentName == null)
			return false;
			
		String vhdlLocation = DatabaseInterface.getVHDLSource(componentName);
		
		if(vhdlLocation == null || !new File(vhdlLocation).exists())
			return false;
		
		return true;
	}
}
