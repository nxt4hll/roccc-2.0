package rocccplugin.preferences;

import org.eclipse.core.runtime.preferences.AbstractPreferenceInitializer;

import rocccplugin.Activator;
import rocccplugin.utilities.PreferenceUtils;

/**
 * Class used to initialize default preference values.
 */
public class PreferenceInitializer extends AbstractPreferenceInitializer 
{
	public void initializeDefaultPreferences() 
	{
		PreferenceUtils.initialize();
		PreferenceUtils.setDefaultValue(PreferenceConstants.ROCCC_DISTRIBUTION, "");
		
		PreferenceUtils.setDefaultValue(PreferenceConstants.SEPARATE_TEMPORARY_ARRAYS, true);
		PreferenceUtils.setDefaultValue(PreferenceConstants.MAXIMIZE_PRECISION, true);
		
		PreferenceUtils.setDefaultValue(PreferenceConstants.DEFAULT_HIGH_OPTIMIZATIONS, "MultiplyByConstElimination\nDivisionByConstElimination");
		PreferenceUtils.setDefaultValue(PreferenceConstants.DEFAULT_LOW_OPTIMIZATIONS, "ArithmeticBalancing\nCopyReduction");
		
		PreferenceUtils.setDefaultValue(PreferenceConstants.DEFAULT_BASIC_WEIGHTS, "1.1.1.1.1.1.1.1.1.1.1");
		
		PreferenceUtils.setDefaultValue(PreferenceConstants.ADD_WEIGHT, "1");
		PreferenceUtils.setDefaultValue(PreferenceConstants.SUB_WEIGHT, "1");
		PreferenceUtils.setDefaultValue(PreferenceConstants.MULT_WEIGHT, "1");
		PreferenceUtils.setDefaultValue(PreferenceConstants.COMPARE_WEIGHT, "1");
		PreferenceUtils.setDefaultValue(PreferenceConstants.MUX_WEIGHT, "1");
		PreferenceUtils.setDefaultValue(PreferenceConstants.COPY_WEIGHT, "1");
		PreferenceUtils.setDefaultValue(PreferenceConstants.SHIFT_WEIGHT, "1");
		PreferenceUtils.setDefaultValue(PreferenceConstants.AND_WEIGHT, "1");
		PreferenceUtils.setDefaultValue(PreferenceConstants.OR_WEIGHT, "1");
		PreferenceUtils.setDefaultValue(PreferenceConstants.XOR_WEIGHT, "1");
		PreferenceUtils.setDefaultValue(PreferenceConstants.OPS_PER_PIPELINE_STAGE, "3.3333333333333");
		
		PreferenceUtils.setDefaultValue(PreferenceConstants.MAX_CYCLE_WEIGHT, "1");
		PreferenceUtils.setDefaultValue(PreferenceConstants.MAX_FANOUT, "100");
		
		PreferenceUtils.setDefaultValue(PreferenceConstants.FULLY_UPDATE, true);
		
		PreferenceUtils.setDefaultValue(PreferenceConstants.DATE_SINCE_LAST_UPDATE_CHECK, System.currentTimeMillis());
		PreferenceUtils.setDefaultValue(PreferenceConstants.LAST_COMPILER_VERSION, "NA");
		PreferenceUtils.setDefaultValue(PreferenceConstants.LAST_GUI_VERSION, "NA");
		PreferenceUtils.setDefaultValue(PreferenceConstants.DECLINED_NEW_UPDATE, false);
		PreferenceUtils.setDefaultValue(PreferenceConstants.NEVER_HAD_VALID_DISTRO, true);
		
		PreferenceUtils.setDefaultValue(PreferenceConstants.AUTOMATICALLY_CHECK_FOR_UPDATES, true);
		PreferenceUtils.setDefaultValue(PreferenceConstants.OPEN_COMPILATION_REPORT_AFTER_COMPILE, true);
		
		PreferenceUtils.setDefaultValue(PreferenceConstants.USER_NAME, "No Input");
		PreferenceUtils.setDefaultValue(PreferenceConstants.USER_ORGANIZATION, "No Input");
		PreferenceUtils.setDefaultValue(PreferenceConstants.USER_EMAIL, "No Input");
		
		PreferenceUtils.setDefaultValue(PreferenceConstants.LICENSE_FILE_PATH, Activator.getDistributionFolder() + "/License.lic");
	}

}
