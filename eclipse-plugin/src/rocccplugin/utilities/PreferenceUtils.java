package rocccplugin.utilities;

import org.eclipse.core.runtime.preferences.InstanceScope;
import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.dialogs.PreferencesUtil;
import org.eclipse.ui.preferences.ScopedPreferenceStore;

import rocccplugin.ROCCCPlugin;
import rocccplugin.preferences.PreferenceConstants;

public class PreferenceUtils 
{
	static IPreferenceStore store = null;
	
	public static IPreferenceStore getStore()
	{
		return store;
	}
	
	static public void initialize()
	{
		if (store == null) 
		{
		     store = new ScopedPreferenceStore(new InstanceScope(), ROCCCPlugin.getBundle().getSymbolicName());
		}
	}
	
	static public String getPreferenceString(String pref)
	{
		initialize();
		return store.getString(pref);
	}
	
	static public boolean getPreferenceBoolean(String pref)
	{
		initialize();
		return store.getBoolean(pref);
	}
	
	static public boolean queryUserForPreferenceChange()
	{
		initialize();
		String error;
		
		if(PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION).equals(""))
			error = "The distribution folder preference has not been set. This must be set in the ROCCC Preferences to begin.";
		else
			error = "The distribution folder selected is not a valid distribution folder.";
		
		boolean openPreferences = MessageUtils.openQuestionWindow("Distribution Error", error + "\n\n" + "Would you like to change this now?");
		
		return openPreferences;
	}
	
	static public void openPreferenceWindow()
	{
		initialize();
		GuiLockingUtils.unlockGui();
		PreferencesUtil.createPreferenceDialogOn(new Shell(), "rocccplugin.preferences.ROCCCPreferencePage", null, null).open();
	}

	public static void setValue(String pref, boolean value) 
	{
		initialize();
		store.setValue(pref, value);
	}
	
	public static void setValue(String pref, String value) 
	{
		initialize();
		store.setValue(pref, value);
	}
	
	public static void setDefaultValue(String pref, String value)
	{
		initialize();
		store.setDefault(pref, value);
	}
	
	public static void setDefaultValue(String pref, boolean value)
	{
		initialize();
		store.setDefault(pref, value);
	}
	
	public static void setDefaultValue(String pref, long value)
	{
		initialize();
		store.setDefault(pref, value);
	}
}
