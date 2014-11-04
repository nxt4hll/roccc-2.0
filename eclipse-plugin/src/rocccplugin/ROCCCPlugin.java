package rocccplugin;

import org.osgi.framework.Bundle;
import org.osgi.framework.Version;

import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.PreferenceUtils;

public class ROCCCPlugin 
{
	static String distributionFolder = "";

	private boolean library_loaded;
	
	public static Version getVersionNumber()
	{
		return Activator.bc.getBundle().getVersion();
	}
	
	public static Bundle getBundle()
	{
		return Activator.bc.getBundle();
	}
	
	public static boolean distributionChanged()
	{
		return PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION).equals(distributionFolder);
	}
	
	public static void handleDistributionChange() 
	{
		distributionFolder = PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION);
		Activator.compilerAndPluginNeedChecking();
	}
}
 