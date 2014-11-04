package rocccplugin.preferences;

import org.eclipse.jface.preference.FileFieldEditor;
import org.eclipse.swt.widgets.Composite;

public class LicenseFileFieldEditor extends FileFieldEditor
{
	public LicenseFileFieldEditor(String licenseFilePath, String string,
			Composite licenseComp) {
		super(licenseFilePath, string, licenseComp);
	}
	
	/*
	 * Overriding the FileFieldEditor method so that it is a valid file
	 * even if the file doesn't exist.  Thus, users can save invalid file
	 * paths if they want to.  The page will still validate and give the
	 * 'OK' button.
	 */
	public boolean isValid()
	{
		return true;
	}
}
