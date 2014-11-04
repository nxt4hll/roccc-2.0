package rocccplugin;

import java.io.BufferedReader;
import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLEncoder;
import java.util.List;
import java.util.Vector;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.FileLocator;
import org.eclipse.core.runtime.Path;
import org.eclipse.core.runtime.Platform;
import org.eclipse.jface.dialogs.IDialogConstants;
import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.IEditorDescriptor;
import org.eclipse.ui.IPerspectiveDescriptor;
import org.eclipse.ui.IViewPart;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.browser.IWorkbenchBrowserSupport;
import org.eclipse.ui.internal.editors.text.EditorsPlugin;
import org.eclipse.ui.part.FileEditorInput;
import org.eclipse.ui.plugin.AbstractUIPlugin;
import org.eclipse.ui.texteditor.AnnotationPreference;
import org.eclipse.ui.texteditor.MarkerAnnotationPreferences;
import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.Version;

import rocccplugin.actions.CancelCompile;
import rocccplugin.actions.ResetDatabase;
import rocccplugin.helpers.UpdateInfo;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.FileUtils;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.PreferenceUtils;
import rocccplugin.utilities.StringUtils;
/**
 * The activator class controls the plug-in life cycle
 */

public class Activator extends AbstractUIPlugin implements org.eclipse.ui.IStartup
{
	public enum VersionStatus
	{
		COMPILER_UPDATE_NEEDED,
		PLUGIN_UPDATE_NEEDED,
		VERSIONS_SYNCED,
		CORRUPTION_ERROR
	}
	
	public enum ComponentType
	{ 
		BLOCK, 
		MODULE,
		SYSTEM,
		INTRINSIC,
		UNKNOWN
	}
	
	public enum OSType
	{
		LEOPARD,
		SNOW_LEOPARD,
		LION,
		MOUNTAIN_LION,
		MAVERICK,
		UBUNTU_32,
		UBUNTU_64,
		CENTOS_32,
		CENTOS_64,
		UNKNOWN
	}
  
	// The plug-in ID 
	public static final String PLUGIN_ID = "ROCCCplugin";
	private static Version minimumCompilerVersionForPlugin = new Version("0.6.0");
	
	private boolean library_loaded;
	private static VersionStatus compilerAndPluginStatus = null;
	public static BundleContext bc;
	// The shared instance
	private static Activator plugin;
	public static boolean declinedNewUpdate = false;
	
	public static Vector<InputStream> additionClasses;
	public static Vector<String> additionImages;
	
	// The OS we are running on
	public static boolean OSCached = false ;
	public static OSType determinedOS = OSType.UNKNOWN ;
	
	//The constructor
	public Activator() 
	{
		plugin = this;
	}

	// This function queries the system and determines what OS we are running on
	//  in order to determine what to call in some scripts
	public static OSType getOS()
	{
		if (OSCached == true)
			return determinedOS ;
		
		String OSName    = System.getProperty("os.name") ;
		String OSVersion = System.getProperty("os.version") ;
		String OSArch    = System.getProperty("os.arch") ;
				
		if (OSName.equals("Mac OS X"))
		{
			if (OSVersion.contains("10.5"))
				determinedOS = OSType.LEOPARD ;
			else if (OSVersion.contains("10.6"))
				determinedOS = OSType.SNOW_LEOPARD ;
			else if (OSVersion.contains("10.7"))
				determinedOS = OSType.LION ;
			else if (OSVersion.contains("10.8"))
				determinedOS = OSType.MOUNTAIN_LION ;
			else if (OSVersion.contains("10.9"))
				determinedOS = OSType.MAVERICK ;
		}
		else if (OSName.equals("Linux"))
		{
			if (OSArch.contains("64")) 
				determinedOS = OSType.UBUNTU_64 ;
			else
				determinedOS = OSType.UBUNTU_32 ;
		}
		OSCached = true ;
		return determinedOS ;
	}
	
	public static String getOSString()
	{
		OSType os = getOS() ;
		switch(os)
		{
		case LEOPARD:
			return "Leopard" ;
		case SNOW_LEOPARD:
			return "SnowLeopard" ;
		case LION:
			return "Lion" ;
		case MOUNTAIN_LION:
			return "MountainLion" ;
		case MAVERICK:
			return "Maverick" ;
		case UBUNTU_32:
			return "Ubuntu32" ;
		case UBUNTU_64:
			return "Ubuntu64" ;
		case CENTOS_32:
			return "CentOS32" ;
		case CENTOS_64:
			return "CentOS64" ;
		case UNKNOWN:
			return "Unknown" ;
		default:
			return "Unknown" ;
		}
	}
 
	@Override
	protected void initializeDefaultPreferences(IPreferenceStore store) 
	{
		try
		{
			PreferenceUtils.initialize();
			PreferenceUtils.setDefaultValue(PreferenceConstants.ROCCC_DISTRIBUTION, "");
			
			PreferenceUtils.setDefaultValue(PreferenceConstants.SEPARATE_TEMPORARY_ARRAYS, true);
			PreferenceUtils.setDefaultValue(PreferenceConstants.MAXIMIZE_PRECISION, true);
			
			PreferenceUtils.setDefaultValue(PreferenceConstants.DEFAULT_HIGH_OPTIMIZATIONS, "MultiplyByConstElimination\nDivisionByConstElimination");
			PreferenceUtils.setDefaultValue(PreferenceConstants.DEFAULT_LOW_OPTIMIZATIONS, "ArithmeticBalancing\nCopyReduction");
			
			PreferenceUtils.setDefaultValue(PreferenceConstants.DEFAULT_BASIC_WEIGHTS, "1.1.1.1.1.1.1.1.1.1.1");
			
			PreferenceUtils.setDefaultValue(PreferenceConstants.ADD_WEIGHT, "23");
			PreferenceUtils.setDefaultValue(PreferenceConstants.SUB_WEIGHT, "23");
			PreferenceUtils.setDefaultValue(PreferenceConstants.MULT_WEIGHT, "42");
			PreferenceUtils.setDefaultValue(PreferenceConstants.COMPARE_WEIGHT, "21");
			PreferenceUtils.setDefaultValue(PreferenceConstants.MUX_WEIGHT, "14");
			PreferenceUtils.setDefaultValue(PreferenceConstants.COPY_WEIGHT, "12");
			PreferenceUtils.setDefaultValue(PreferenceConstants.SHIFT_WEIGHT, "12");
			PreferenceUtils.setDefaultValue(PreferenceConstants.AND_WEIGHT, "17");
			PreferenceUtils.setDefaultValue(PreferenceConstants.OR_WEIGHT, "17");
			PreferenceUtils.setDefaultValue(PreferenceConstants.XOR_WEIGHT, "17");
			PreferenceUtils.setDefaultValue(PreferenceConstants.OPS_PER_PIPELINE_STAGE, "3.3333333333333");
			
			PreferenceUtils.setDefaultValue(PreferenceConstants.MAX_CYCLE_WEIGHT, "1");
			PreferenceUtils.setDefaultValue(PreferenceConstants.MAX_FANOUT, "100");
			
			PreferenceUtils.setDefaultValue(PreferenceConstants.FULLY_UPDATE, true);
			
			PreferenceUtils.setDefaultValue(PreferenceConstants.DATE_SINCE_LAST_UPDATE_CHECK, System.currentTimeMillis());
			PreferenceUtils.setDefaultValue(PreferenceConstants.LAST_COMPILER_VERSION, "NA");
			PreferenceUtils.setDefaultValue(PreferenceConstants.LAST_GUI_VERSION, "NA");
			PreferenceUtils.setDefaultValue(PreferenceConstants.DECLINED_NEW_UPDATE, false);
			PreferenceUtils.setDefaultValue(PreferenceConstants.AUTOMATICALLY_CHECK_FOR_UPDATES, true);
			PreferenceUtils.setDefaultValue(PreferenceConstants.NEVER_HAD_VALID_DISTRO, true);
			
			PreferenceUtils.setDefaultValue(PreferenceConstants.OPEN_COMPILATION_REPORT_AFTER_COMPILE, true);
			
			PreferenceUtils.setDefaultValue(PreferenceConstants.USER_NAME, "No Input");
			PreferenceUtils.setDefaultValue(PreferenceConstants.USER_ORGANIZATION, "No Input");
			PreferenceUtils.setDefaultValue(PreferenceConstants.USER_EMAIL, "No Input");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public void start(BundleContext context) throws Exception 
	{
		super.start(context);
		bc = context;
		library_loaded = false;
		MessageUtils.initialize();
	}

	public void stop(BundleContext context) throws Exception 
	{
		try
		{
			CancelCompile.run(false);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		plugin = null;
		bc = context;
		super.stop(context);
		
	}
	
	public static Activator getDefault() 
	{
		return plugin;
	}
	
	public boolean isLibraryOpened()
	{
		return library_loaded;
	}
	
	public void setLibraryClosed()
	{
		library_loaded = false;
	}
	
	public static Version getMinimumCompilerVersionNeeded()
	{
		return minimumCompilerVersionForPlugin;
	}
	
	public static boolean areCompilerAndPluginSynced()
	{
		boolean cached = compilerAndPluginStatus != null;
		VersionStatus status = checkCompilerAndPluginStatus();
		
		if(status.equals(VersionStatus.COMPILER_UPDATE_NEEDED))
		{
			MessageUtils.openErrorWindow("Compiler Update Needed", "The ROCCC Compiler is at a lower version than this plugin supports. Please check for updates or change distribution folders to a newer version.");
			return false;
		}
		else if(status.equals(VersionStatus.PLUGIN_UPDATE_NEEDED))
		{
			MessageUtils.openErrorWindow("Plugin Update Needed", "The ROCCC Plugin is at a lower version than this compiler supports. Please check for updates or change the plugin to a newer version.");
			return false;
		} 
		else if(status.equals(VersionStatus.CORRUPTION_ERROR))
		{
			if(cached)
				MessageUtils.openErrorWindow("Distribution Error", "There was an error reading the version file. Your distribution may be corrupt and may need to be reinstalled.");
			return false;
		}
		
		return true; 
	}
	
	public static VersionStatus checkCompilerAndPluginStatus()
	{
		//If we have not checked the versions yet, lets check them.
		if(compilerAndPluginStatus == null)
		{
			String[] compatiblePluginVersions = Activator.getCompatiblePluginVersionsForCompiler();
			
			if(compatiblePluginVersions == null)
			{
				compilerAndPluginStatus = VersionStatus.CORRUPTION_ERROR;
				return compilerAndPluginStatus;
			}
			
			//Get the maximum and minimum plugin versions that are compatible with the compiler version.
			Version currentCompilerVersion = new Version(compatiblePluginVersions[0]);
			Version minimumPluginVersionForCompiler = new Version(compatiblePluginVersions[1]);
			
			//Get the version of the plugin
			Version pluginVersion = bc.getBundle().getVersion();
			
			if(pluginVersion.compareTo(minimumPluginVersionForCompiler) < 0)
			{
				compilerAndPluginStatus = VersionStatus.PLUGIN_UPDATE_NEEDED;
			}
			else if(minimumCompilerVersionForPlugin.compareTo(currentCompilerVersion) > 0)
			{
				compilerAndPluginStatus = VersionStatus.COMPILER_UPDATE_NEEDED;
			}
			else
				compilerAndPluginStatus = VersionStatus.VERSIONS_SYNCED;
		}
		
		return compilerAndPluginStatus;
	}
	
	public static void compilerAndPluginNeedChecking()
	{
		compilerAndPluginStatus = null;
	}
	
	public static boolean isProcessDone(Process p)
	{
		//Check the exit value of the process. If it fails it means it is still running.
		boolean ret = false;
		try
		{
			p.exitValue();
			ret = true;
		}
		catch(IllegalThreadStateException e){}
		return ret;
	}
		
	public static String getPluginFolder()
	{
		String pluginFolder;
		
        URL url = Platform.getBundle("ROCCCplugin").getEntry("/");
        try 
        {
            url = FileLocator.resolve(url);
        }
        catch(IOException ex) 
        {
            ex.printStackTrace();
        }

        File file = new File(url.getPath());
       
        pluginFolder = file.getPath().replaceFirst("file:", "");
        pluginFolder = pluginFolder.substring(0, pluginFolder.length() - 1);
        
        return pluginFolder.substring(0, pluginFolder.lastIndexOf("/") + 1);
	}
	
	public static String getCompilerVersion()
	{
		String localVersion = "";
		try
		{
			StringBuffer buffer = new StringBuffer();
			// First, we should check to see if we are the binary distribution, then we should 
			//  check to see if we are a standard distribution
			if (new File(Activator.getDistributionFolder() + "/.VERSION").exists() == true)
			{
				FileUtils.addFileContentsToBuffer(buffer, Activator.getDistributionFolder() + "/.VERSION") ;
			}
			else if (new File(Activator.getDistributionFolder() + "/Install/roccc-compiler/src/VERSION").exists() == true)
			{
				FileUtils.addFileContentsToBuffer(buffer, Activator.getDistributionFolder() + "/Install/roccc-compiler/src/VERSION") ;
			}
			else
			{
				throw new Exception();
			}
			localVersion = StringUtils.getNextStringValue(buffer) ;
				
				// original code is below
//			if(new File(Activator.getDistributionFolder() + "/Install/roccc-compiler/src/VERSION").exists() == false)
//			{
//				throw new Exception();
//			}
//			FileUtils.addFileContentsToBuffer(buffer, Activator.getDistributionFolder() + "/Install/roccc-compiler/src/VERSION");
//			localVersion = StringUtils.getNextStringValue(buffer);
		}
		catch(Exception e)
		{
			MessageUtils.openErrorWindow("Distribution Error", "There was an error reading the ROCCC Compiler version file. Your distribution may be corrupt and may need to be reinstalled.");
			return null;
		}
		
		Version local = new Version(localVersion);
		
		if(local.getQualifier() == null || local.getQualifier().equals(""))
			local = new Version(local.toString() + ".0");
		
		return local.toString();
	}
	
	public static String[] getCompatiblePluginVersionsForCompiler()
	{
		String[] versions = new String[2];
		try
		{
			StringBuffer buffer = new StringBuffer();
			if (new File(Activator.getDistributionFolder() + "/.VERSION").exists() == true)
			{
				FileUtils.addFileContentsToBuffer(buffer, Activator.getDistributionFolder()+ "/.VERSION") ;
			}
			else if (new File(Activator.getDistributionFolder() + "/Install/roccc-compiler/src/VERSION").exists() == true)
			{
				FileUtils.addFileContentsToBuffer(buffer, Activator.getDistributionFolder() + "/Install/roccc-compiler/src/VERSION") ;
			}
			else
			{
				throw new Exception() ;
			}
			versions[0] = StringUtils.getNextStringValue(buffer) ;
			versions[1] = StringUtils.getNextStringValue(buffer) ;
			if (versions[1] == null || versions[1] =="")
			{
				versions[1] = versions[0] ;
			}
			// Original Code follows
			/*
			if(new File(Activator.getDistributionFolder() + "/Install/roccc-compiler/src/VERSION").exists() == false)
			{
				throw new Exception();
			}
			FileUtils.addFileContentsToBuffer(buffer, Activator.getDistributionFolder() + "/Install/roccc-compiler/src/VERSION");
			versions[0] = StringUtils.getNextStringValue(buffer);
			versions[1] = StringUtils.getNextStringValue(buffer);
			if(versions[1] == null || versions[1] == "")
				versions[1] = versions[0];
			*/
		}
		catch(Exception e)
		{
			MessageUtils.openErrorWindow("Distribution Error", "There was an error reading the ROCCC Compiler version file. Your distribution may be corrupt and may need to be reinstalled.");
			return null;
		}
		
		return versions;
	}

	public static String getDistributionFolder()
	{
		return PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION);
	}
	
	public static String getLicenseFilePath()
	{
		return PreferenceUtils.getPreferenceString(PreferenceConstants.LICENSE_FILE_PATH);
	}
	
	static public Boolean isModule(File sourceFile)
	{
		if(sourceFile.getAbsolutePath().contains("/src/modules/"))
		{
			return true;
		}
		else if(sourceFile.getAbsolutePath().contains("/src/systems/"))
		{
			return false;
		}
		else
		{
			return false;
		}
	}
	
	static public Boolean isIntrinsic(File fileToRunTestbenchOn)
	{
		if(fileToRunTestbenchOn.getAbsolutePath().contains("/src/intrinsics/"))
		{
			return true;
		}
		return false;
	}
	
	static public String getOptFile(File sourceFile)
	{
		//Check for a .opt file.
		File dir = new File(sourceFile.getAbsolutePath().replace(sourceFile.getName(), "") + "/.ROCCC/");
		
		if(!dir.exists())
			return null;
		
		FilenameFilter filter = new FilenameFilter()
		{
			public boolean accept(File dir, String name)
			{
				return name.endsWith(".opt");
			}
		};
		
		try
		{
			return sourceFile.getAbsolutePath().replace(sourceFile.getName(), "") + "/.ROCCC/" + dir.list(filter)[0];
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return null;
	}
	
	static public boolean testDistributionFolder(String directory)
	{
		//Get the llvm_path that is set in the preference menu.
		String llvm_path = directory + "/LocalFiles/vhdlLibrary.sql3";
		
		try
		{
			//Try to open the database that holds the components.
			String database = llvm_path;
			File f = new File(database);
			if( f.exists() )
			{
			  return true;
			}
			else
				throw new java.lang.Exception("");
		}
		catch(java.lang.Exception e)
		{
		}
		return false;
	}
	
	public static void showStackTrace()
	{
		try
		{
			throw new Exception("Stack Trace!");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static ImageDescriptor getImageDescriptor(String path) 
	{
		return imageDescriptorFromPlugin(PLUGIN_ID, path);
	}
	
	public IPreferenceStore getPreferenceStore()
	{
		PreferenceUtils.initialize();
		return PreferenceUtils.getStore();
	}
	
	public void handlePerspectiveSettingUp()
	{
		Display.getDefault().syncExec(new Runnable()
		{
			public void run() 
			{
				if(EclipseResourceUtils.isViewOpen("org.eclipse.ui.internal.introview"))
					EclipseResourceUtils.closeView("org.eclipse.ui.internal.introview");
				
				try
				{
					IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
					if(window == null)
						return;
					//Get the active page on the window.
					IWorkbenchPage page = window.getActivePage();
					if(page == null)
						return;					

					page.setPerspective(PlatformUI.getWorkbench().getPerspectiveRegistry().findPerspectiveWithId("ROCCCplugin.ROCCCPerspective"));
					
					while(page.getPerspective().getId().equals("ROCCCplugin.ROCCCPerspective") == false);
				}
				catch(Exception e)
				{
					e.printStackTrace();
				}
			}
		});
	}
	
	public void handleWelcomeScreen(final String userName, final String organization, final String email)
	{
		//Run this in the default display so that it uses the Eclipse workbench.
		Display.getDefault().syncExec(new Runnable()
		{
			public void run() 
			{
				try 
				{
					//Test the internet connection seeing if we can find the ROCCC page.
					URL internetChecker = new URL("http://jacquardcomputing.com/");
					internetChecker.openStream().close();
					
					//Get the link for the changelist page for the current version.
					final String ID = "ROCCCBrowser";
					final URL url;
					
					String method = PreferenceUtils.getPreferenceString(PreferenceConstants.LAST_GUI_VERSION).equals("NA")? "Install" : "Update";
					String version = ROCCCPlugin.getVersionNumber().toString();
					String previousVersion = PreferenceUtils.getPreferenceString(PreferenceConstants.LAST_GUI_VERSION);
					
					String urlString = "http://jacquardcomputing.com/Installs/newInstall.php";
					
					String postString = URLEncoder.encode("method", "UTF-8") + "=" + URLEncoder.encode(method, "UTF-8") + "&" + 
								 URLEncoder.encode("previousVersion", "UTF-8") + "=" + URLEncoder.encode(previousVersion, "UTF-8") + "&" + 
								 URLEncoder.encode("version", "UTF-8") + "=" + URLEncoder.encode(version, "UTF-8") + "&" + 
								 URLEncoder.encode("usersName", "UTF-8") + "=" + URLEncoder.encode(userName, "UTF-8") + "&" + 
								 URLEncoder.encode("organization", "UTF-8") + "=" + URLEncoder.encode(organization, "UTF-8") + "&" + 
								 URLEncoder.encode("usersEmail", "UTF-8") + "=" + URLEncoder.encode(email, "UTF-8");
					
					
					//Send the data to the webpage for logging.
					URLConnection connection = new URL(urlString).openConnection();
					connection.setDoOutput(true);
					connection.setDoInput(true);
					connection.setUseCaches(false);
					connection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
					
					OutputStreamWriter wr = new OutputStreamWriter(connection.getOutputStream());
					wr.write(postString);
					wr.flush();
					
					BufferedReader rd = new BufferedReader(new InputStreamReader(connection.getInputStream()));
				    
					String line;
				    while ((line = rd.readLine()) != null) {}

					wr.close();
					rd.close();
					
					urlString = "http://jacquardcomputing.com/roccc/welcome-to-roccc-2-0/";
					
					//Open a browser inside Eclipse to the changelist page.
					url = new URL(urlString);
					//See if we can reach the url first, if not, we will except out.
					url.openStream().close();
					PlatformUI.getWorkbench().getBrowserSupport().createBrowser(IWorkbenchBrowserSupport.AS_EDITOR | IWorkbenchBrowserSupport.NAVIGATION_BAR,  ID, "Welcome to ROCCC - Version " + ROCCCPlugin.getVersionNumber().toString(), "Welcome to ROCCC - Version " + ROCCCPlugin.getVersionNumber().toString()).openURL(url);
				} 
				catch (Exception e) 
				{
				}
			}
		});
	}
	
	private void removeErrorAnnotationsFromEditor()
	{
		try
		{
			//Get the list of annotation preferences from the Editor plugin.
			MarkerAnnotationPreferences map = EditorsPlugin.getDefault().getMarkerAnnotationPreferences();
			List l = map.getAnnotationPreferences();

			for(int i = 0; i < l.size(); ++i)
			{
				final AnnotationPreference ap = ((AnnotationPreference)(l.get(i)));
				
				//If the annotation type is for the C++ index marker, let's turn it off.
				if(ap.getAnnotationType().toString().equals("org.eclipse.cdt.ui.indexmarker"))
				{
					Display.getDefault().syncExec(new Runnable()
					{
						public void run() 
						{
							EditorsPlugin.getDefault().getPreferenceStore().setValue(ap.getOverviewRulerPreferenceKey(), false);
							EditorsPlugin.getDefault().getPreferenceStore().setValue(ap.getVerticalRulerPreferenceKey(), false);
							EditorsPlugin.getDefault().getPreferenceStore().setValue(ap.getTextPreferenceKey(), false);
						}
					});
					
					break;
				}
			}
			
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private boolean isOldROCCCPluginInstalled()
	{
		Bundle[] plugins = Activator.bc.getBundles();
		
		for(int i = 0; i < plugins.length; ++i)
		{
			if(plugins[i].getSymbolicName().equals("ROCCCplugin") ||
			   (plugins[i].getSymbolicName().equals("ROCCCPlugin") && 
			    plugins[i].getVersion().compareTo(ROCCCPlugin.getVersionNumber()) < 0))
			{
				return true;
			}
		}
		
		return false;
	}
	
	private void registerInstalledROCCCPlugins()
	{
		Bundle[] plugins = Activator.bc.getBundles();
		additionImages = new Vector<String>();
		additionClasses = new Vector<InputStream>();
		
		for(int i = 0; i < plugins.length; ++i)
		{	
			//If we found a ROCCC plugin that is not this one, register it
			if(plugins[i].getSymbolicName().startsWith("ROCCC") && plugins[i] != bc.getBundle())
			{				
				//Get update description
				String updateDescription = (String)plugins[i].getHeaders().get(org.osgi.framework.Constants.BUNDLE_DESCRIPTION);
				if(updateDescription == null)
					continue;
				
				//Parse the values split by the vertical slash character.
				String[] parsedValues = updateDescription.split("\\|");
				
				if(parsedValues == null || parsedValues.length == 0)
					continue;
				
				// The updateLocation now must be based upon the os
				//Extract the update information from the description
				String name = parsedValues[0];
				//String updateLocation = parsedValues[1];
				String updateLocation = "http://www.jacquardcomputing.com/downloads/" + Activator.getOSString() + parsedValues[1] ;
				String currentVersion = parsedValues[2];
				String additionImage = parsedValues[3];
				
				//Add the values into the update info. 
				UpdateInfo.addUpdateInfo(name, updateLocation, new Version(currentVersion));
				additionImages.add(additionImage);
				try 
				{
					additionClasses.add(plugins[i].getBundleContext().getBundle().getResource(additionImage).openStream());
				} 
				catch (IOException e) 
				{
					//e.printStackTrace();
					additionClasses.add(null);
				}
				
			}	
		}
	}
	
	//This function is called right after the plugin is registered and started. 
	//This allows us to do things without user input such as check for updates or
	//see if this is the first time they have started the plugin.
	public void earlyStartup() 
	{		
		try
		{
			GuiLockingUtils.lockGui("ROCCC is running its startup process. Please wait.");
			
			PreferenceUtils.initialize();
			MessageUtils.printlnConsoleSuccess("Welcome to ROCCC 2.0 version " + ROCCCPlugin.getVersionNumber().toString() + "!\n");

			//Make sure no old ROCCC plugins are installed.
			if(isOldROCCCPluginInstalled())
			{
				Display.getDefault().syncExec(new Runnable()
				{
					public void run() 
					{
						MessageUtils.openErrorWindow("Plugin Error", "You have an older version of the ROCCC plugin installed with the current one. You need to remove the old plugin and restart Eclipse with the command 'eclipse -clean'.");	
					}	
				});
				
				return;
			}
			
			GuiLockingUtils.setLockMessage("ROCCC is registering installed plugins. Please Wait.");
			registerInstalledROCCCPlugins();

			//Check to see if the gui is at a newer version.
			boolean guiChanged = PreferenceUtils.getPreferenceString(PreferenceConstants.LAST_GUI_VERSION).equals(ROCCCPlugin.getVersionNumber().toString()) == false;
			final boolean newInstall = PreferenceUtils.getPreferenceString(PreferenceConstants.LAST_GUI_VERSION).equals("NA");
			
			boolean noDistroDirectory = PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION).equals("");
			boolean validDistroDirectory = Activator.testDistributionFolder(PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION));
			
			//If this is a new install, open the IPCores and Debug views.
			String[] userInfo = new String[]{PreferenceUtils.getPreferenceString(PreferenceConstants.USER_NAME), PreferenceUtils.getPreferenceString(PreferenceConstants.USER_ORGANIZATION), PreferenceUtils.getPreferenceString(PreferenceConstants.USER_EMAIL)};
			
			if(newInstall || guiChanged)
			{
				removeErrorAnnotationsFromEditor();
			}
			
	
			
			//If the previously loaded plugin does not match the current plugins version, open a browser to the welcome page for that version.
			if(guiChanged)
			{
				GuiLockingUtils.setLockMessage("ROCCC is opening the welcome page. Please wait.");
				handleWelcomeScreen(userInfo[0], userInfo[1], userInfo[2]);
				
				if(PreferenceUtils.getPreferenceString(PreferenceConstants.LAST_COMPILER_VERSION).equals(ROCCCPlugin.getVersionNumber().toString()) == false && validDistroDirectory)
				{
					Display.getDefault().syncExec(new Runnable()
					{
						public void run() 
						{
							if(MessageUtils.openQuestionWindow("ROCCC Update", "It is highly suggested to reset the ROCCC database after updating to this version.\n\nWould you like to reset the database now?"))
							{
								GuiLockingUtils.setLockMessage("ROCCC is reseting the database. Please wait.");
								ResetDatabase.run();
							}
						}
					});
				}
				
				//Save the new GUI version number and also allow for auto update checking.
				PreferenceUtils.setValue(PreferenceConstants.LAST_GUI_VERSION, ROCCCPlugin.getVersionNumber().toString());
				PreferenceUtils.setValue(PreferenceConstants.DECLINED_NEW_UPDATE, false);
			}
			
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		GuiLockingUtils.unlockGui();
	}
}
