package rocccplugin.database;

/*This class represents an interface into the sqlite3 database being used to store ROCCC component information.
 * 
 * Call openLibrary(filename) before performing any other action; openLibrary(filename) will close any
 *    currently open files before opening filename.
 *    
 *  Prefer native methods (or non natives with no overhead, see heading below)
 *    over the non-native methods; the non-native methods call 2 or more native methods
 *    to do the same work, so if it possible to save the names of components and ports in between calls,
 *    prefer to do so to reduce database queries. 
 *    
 *  In order to use this interface, it is necessary to have a shared libary named
 *      DatabaseInterface.dll (windows), or
 *      libDatabaseInterface.so (linux)
 *    in the working directory of the plugin. For ROCCC plugins, this library will be automatically inserted
 *    into the jar file of the plugin by the ROCCC install script.
 * 
 */

import java.io.File;
import java.sql.*;
import java.util.Date;
import java.util.Map;
import java.util.TreeMap;
import java.util.Vector;

import org.eclipse.jface.util.IPropertyChangeListener;
import org.eclipse.jface.util.PropertyChangeEvent;

import rocccplugin.ROCCCPlugin;
import rocccplugin.preferences.PreferenceConstants;
import rocccplugin.utilities.PreferenceUtils;
import rocccplugin.utilities.StringUtils;

public class DatabaseInterface 
{
	private static Connection conn; 
	private static Statement stat;

	private static boolean connectionOpen = false;
	
	private static Vector<IPropertyChangeListener> listeners = new Vector<IPropertyChangeListener>();	

	private DatabaseInterface()
	{
		listeners = new Vector<IPropertyChangeListener>();
	}

	public static void addPropertyChangeListener(IPropertyChangeListener listener)
	{
		if( !listeners.contains(listener) )
			listeners.add(listener);
	}
	
	public static void removePropertyChangeListener(IPropertyChangeListener listener)
	{
		listeners.remove(listener);
	}
	
	static public void updateAllListeners()
	{
		updateListeners();
	}
	
	private static void updateListeners()
	{
		try
		{
			//If we have no connection open to the database, return.
			if(!connectionOpen)
				return;
			
			//Send a "change event" to all the listeners so they will update.
			PropertyChangeEvent event = new PropertyChangeEvent(listeners, "database" , null , null);
			for(int i = 0; i < listeners.size(); ++i)
			{
				listeners.get(i).propertyChange(event);
			}
		}
		catch(Exception e)
		{
			e.printStackTrace(); 
		}
	}
	
	private final static void openLibrary(String Filename)
	{
		try
		{
			Class.forName("org.sqlite.JDBC");
			conn = DriverManager.getConnection("jdbc:sqlite:" + Filename );
			stat = conn.createStatement();
			
			updateListeners();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	} 
	
	private final static boolean openConnection(boolean showFailureWarning)
	{
		//If the distribution we have cached is different than the distribution preference,
		//it was probably changed so let's handle that new change.
		if(ROCCCPlugin.distributionChanged())
		{
			ROCCCPlugin.handleDistributionChange();	
			DatabaseInterface.updateAllListeners();
			connectionOpen = false;
		}
		
		//If the database is already open, then just return.
		if (connectionOpen == true)
			return true;
		
		try
		{
			//Try to open the database using the preference location the user set.
			File database = new File(PreferenceUtils.getPreferenceString(PreferenceConstants.ROCCC_DISTRIBUTION) + "/LocalFiles/vhdlLibrary.sql3");
			if( database.exists() )
			{
				DatabaseInterface.openLibrary(database.getAbsolutePath());
				connectionOpen = true;
			}
			else
				throw new java.lang.Exception("Database Not Found.");
		}
		catch(Exception e)
		{		
			//Ask if the user would like to change the preferences.
			if(showFailureWarning && PreferenceUtils.queryUserForPreferenceChange())
				PreferenceUtils.openPreferenceWindow();
			
			//We could not load the database.
			connectionOpen = false;
		}
		
		return connectionOpen;
	}
	
	public final static boolean openConnection()
	{
		return openConnection(true);
	}
		
	public final static boolean openConnectionWithoutWarnings()
	{
		return openConnection(false);
	}
	
	public final static void closeConnection()
	{
		//If there appears to be no connection, just return.
		if(!connectionOpen)	
			return;
		
		try 
		{
			//Close the connections.
			stat.close();
			conn.close();
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
		}	
		
		connectionOpen = false;
	}
	
	public final static boolean isOpen()
	{
		return connectionOpen;
	}
	
	public static void cleanupFailedCompilations()
	{
		try
		{
			//Find all ids that have isCompiled as false
			ResultSet rs = stat.executeQuery("SELECT * FROM CompileInfo WHERE isCompiled='false';");
			rs.next();

			Vector<Integer> badIDs = new Vector<Integer>();
			
			//Add all these values to a vector since we cannot make another database call 
			//without losing the values in this current resultSet.
			while(!rs.isAfterLast())
			{
				int id = rs.getInt("id");
				badIDs.add(new Integer(id));
				rs.next();
			}
			
			//For each id, clean up anything that has that id in the database.
			for(int i = 0; i < badIDs.size(); ++i)
			{
				stat.execute("DELETE FROM Ports WHERE id = " + badIDs.get(i));
				stat.execute("DELETE FROM ResourcesUsed WHERE id = " + badIDs.get(i));
				stat.execute("DELETE FROM ResourcesCalled WHERE id = " + badIDs.get(i));
				stat.execute("DELETE FROM FileInfo WHERE id = " + badIDs.get(i));
				stat.execute("DELETE FROM CompileInfo WHERE id = " + badIDs.get(i));
				stat.execute("DELETE FROM ComponentInfo WHERE id = " + badIDs.get(i));
				//stat.execute("DELETE FROM StreamInfo WHERE id = " + badIDs.get(i));
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static String getDatabaseVersion()
	{
		try
		{
			ResultSet rs;
			try
			{
				rs = stat.executeQuery("SELECT name FROM sqlite_master WHERE name='DatabaseVersion';");
				rs.next();
				if (rs.isAfterLast())
					return "NA";
			}
			catch(Exception e)
			{
				e.printStackTrace();
				System.out.println("");
			}
		      
		    rs = stat.executeQuery("SELECT * FROM DatabaseVersion;");
		    rs.next();
		      
		    if(rs.isAfterLast())
		    	return "NA";
		      
		    return rs.getString("version");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			e.getMessage();
		}
	
		return "NA";
	}
	
	public final static String createCompileInfo(String config, String timestamp, String version, String highFlags, String lowFlags, String pipelineFlags, String streamFlags, boolean isCompiled)
	{
		try
		{
			//If the values are passed in are not null, put quotes around them, otherwise leave them as null.
			if(config != null) 
				config = ("'" + config + "'");
			if(timestamp != null)
				timestamp = ("'" + timestamp + "'");
			if(version != null) 
				version = ("'" + version + "'");
			if(highFlags != null) 
				highFlags = ("'" + highFlags + "'");
			if(lowFlags != null) 
				lowFlags = ("'" + lowFlags + "'");
			if(pipelineFlags != null)
				pipelineFlags = ("'" + pipelineFlags + "'");
			if(streamFlags != null) 
				streamFlags = ("'" + streamFlags + "'");	
			
			//Make a compile info with the passed in values.
			String sql = "INSERT INTO CompileInfo(configurationName, timestamp, compilerVersion, highFlags, lowFlags, pipeliningFlags, streamFlags, isCompiled) VALUES(" + config + "," + timestamp + "," + version + "," + highFlags + "," + lowFlags + "," + pipelineFlags + "," + streamFlags +",'" + isCompiled + "');";
			stat.execute(sql);
			
			//Get the id of the compile info we just created.
			sql = "SELECT * FROM CompileInfo WHERE id = LAST_INSERT_ROWID();";
			ResultSet rs = stat.executeQuery(sql);
			rs.next();
			
			return rs.getString("id");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return "";
		}
	}
	
	public final static void createFileInfo(String ID, String name, String type, String loc)
	{
		try
		{
			//Create a file info with the values passed in.
			String sql = "INSERT INTO FileInfo(id, fileName, fileType, location) VALUES('" + ID + "','" + name + "','" + type + "','" + loc + "');";
			stat.execute(sql);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public final static void addResourceUsed(String ID, String type, String idOfResource, int numUsed)
	{
		try
		{
			//Create a file info with the values passed in.
			String sql = "INSERT INTO ResourcesCalled(id, resourceID, resourceType, numUsed) VALUES('" + ID + "'," + idOfResource + ",'" + type + "'," + numUsed + ");";
			stat.execute(sql);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public final static String getStructName(String componentName)
	{
		try
		{
			//Get the struct name of the component passed in.
			ResultSet rs = stat.executeQuery("SELECT * FROM ComponentInfo WHERE id='" + getComponentID(componentName) + "';");
			rs.next();
			return rs.getString("structName");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		return "";
	}
	
	public final static String[] getModulesUsedHierarchy(String componentName)
	{
		try
		{
			openConnection();
			Vector<String> orderedFiles = new Vector<String>();
			Map<String, String> fileMap = new TreeMap<String, String>();
			getModulesUsedRecursive(componentName, orderedFiles, fileMap);
			String[] modules = new String[orderedFiles.size()];
			orderedFiles.toArray(modules);
			return modules;
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		return null;
	}
	
	public final static String[] getModulesCalledHierarchy(String componentName)
	{
		try
		{
			openConnection();
			Vector<String> orderedFiles = new Vector<String>();
			Map<String, String> fileMap = new TreeMap<String, String>();
			getModulesCalledRecursive(componentName, orderedFiles, fileMap);
			String[] modules = new String[orderedFiles.size()];
			orderedFiles.toArray(modules);
			return modules;
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		return null;
	}
	
	private static void getModulesUsedRecursive(String compName, Vector<String> list, Map<String, String> map)
	{
		try
		{
			String[] modules = DatabaseInterface.getModulesUsed(compName);
			
			if(modules == null)
				return;
			
			for(int i = 0; i < modules.length; ++i)
			{
				if(map.containsKey(modules[i]))
					continue;
				map.put(modules[i], modules[i]);
				getModulesUsedRecursive(modules[i], list, map);
				list.add(modules[i]);
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private static void getModulesCalledRecursive(String compName, Vector<String> list, Map<String, String> map)
	{
		try
		{
			String[] modules = DatabaseInterface.getModulesCalled(compName);
			
			if(modules == null)
				return;
			
			for(int i = 0; i < modules.length; ++i)
			{
				if(map.containsKey(modules[i]))
					continue;
				map.put(modules[i], modules[i]);
				getModulesCalledRecursive(modules[i], list, map);
				list.add(modules[i]);
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	
	public final static int getComponentID(String componentName)
	{	
		try
		{
			//Get the componentID of the componentName.
			ResultSet rs = stat.executeQuery("SELECT id FROM ComponentInfo WHERE componentName='" + componentName + "';");
			rs.next();
					
			if(rs.isAfterLast())
				return -1;
			else
				return rs.getInt("id");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return -1;
		}
	}
	
	
	public final static String getComponentFromID(int id)
	{
		try
		{
			//Get the components name that matches the passed in ID.
			ResultSet rs = stat.executeQuery("SELECT componentName FROM ComponentInfo WHERE id=" + id + ";");
			rs.next();
					
			return rs.getString("componentName");
		}
		catch(Exception e)
		{
			return null;
		}
	}
	
	
	public static String getVHDLSource(String componentName)
	{
		try
		{	
			//If there is no component with that name, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Grab the vhdl source file location for the given component.
			ResultSet rs = stat.executeQuery("SELECT * FROM FileInfo WHERE fileType = 'VHDL_SOURCE' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			//If nothing exists for that, return null.
			if(rs.isAfterLast())
				return null;
			
			return rs.getString("location") + rs.getString("fileName");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}

	public static String getSuifFile(String componentName)
	{
		try
		{	
			//If there is no component with that name, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Grab the suif source file location for the given component.
			ResultSet rs = stat.executeQuery("SELECT * FROM FileInfo WHERE fileType = 'SUIF_FILE' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			//If no suif file exists, return null.
			if(rs.isAfterLast())
				return null;
			
			return rs.getString("location") + rs.getString("fileName");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	
	public final static String versionCompiledOn(String componentName)
	{
		try
		{
			//If there is no component with that name, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			ResultSet rs = stat.executeQuery("SELECT compilerVersion FROM CompileInfo WHERE id='" + getComponentID(componentName) + "';");
			rs.next();
			
			//If no result set is returned, return null.
			if(rs.isAfterLast())
				return null;
			
			return rs.getString("compilerVersion");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public final static boolean activateComponent(String componentName, String delay)
	{
		try 
		{
			//Attempt to activate the component.  If it switches to being activate, return true, otherwise return value.
			int count = stat.executeUpdate("UPDATE ComponentInfo SET active=1 WHERE id='" + getComponentID(componentName) + "' AND delay=" + delay + ";");
			return count > 0;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
	}
	
	//Used for intrinsics that use only one bitsize
	public final static boolean deactivateComponents(String type, int bitSize)
	{
		try 
		{
			//Get all of the components that are of the given type.
			ResultSet rs = stat.executeQuery("SELECT * FROM ComponentInfo WHERE type='" + type + "';");
			rs.next();
			
			//Store those component names in a vector.
			Vector<String> comps = new Vector<String>();
			for(;!rs.isAfterLast(); rs.next())
				comps.add(rs.getString("componentName"));
			
			//Check all of the components of that type and see if they match the bitsize.
			for(int i = 0; i < comps.size(); ++i)
			{
				rs = stat.executeQuery("SELECT * FROM Ports WHERE componentName='" + comps.get(i) + "';");
				rs.next();
				
				//If one of the components is a voter, we need to skip the error port.
				if(type.contains("VOTE"))
					rs.next();
				
				//If the bit width matches the passed in bit width, deactivate it.
				if(rs.getInt("bitwidth") == bitSize)
					stat.executeUpdate("UPDATE ComponentInfo SET active=0 WHERE componentName='" + comps.get(i) + "';");
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
		
		return true;
	}
	
	//Used for intrinsics that use two bitsizes
	public final static boolean deactivateComponents(String type, int bitSize, int bitSize2)
	{
		try 
		{
			//Get all of the components that are of the given type.
			ResultSet rs = stat.executeQuery("SELECT * FROM ComponentInfo WHERE type='" + type + "';");
			rs.next();
			
			//Store those component names in a vector.
			Vector<String> comps = new Vector<String>();
			for(;!rs.isAfterLast(); rs.next())
				comps.add(rs.getString("componentName"));
			
			//Check all of the components of that type and see if they match the bit sizes passed in.
			for(int i = 0; i < comps.size(); ++i)
			{
				rs = stat.executeQuery("SELECT * FROM Ports WHERE componentName='" + comps.get(i) + "';");
				rs.next();
			
				//If the first bitsize matches, let's check the second.
				if(rs.getInt("bitwidth") == bitSize)
				{
					//If the second bitsize matches, deactivate it.
					rs.next();
					if(rs.getInt("bitwidth") == bitSize2)
					{
						stat.executeUpdate("UPDATE ComponentInfo SET active=0 WHERE componentName='" + comps.get(i) + "';");
					}
				}
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
		
		return true;
	}
	
	//Returns relevant data for component of the given type.
	public final static String[][] getComponentsOfType(String type)
	{
		try
		{
			//Get all the components of the given type.
			ResultSet rs = stat.executeQuery("SELECT * FROM ComponentInfo WHERE type='" + type + "';");
			rs.next();
			
			//Store the component name, delay, description, and if its active for each component of the matching type.
			Vector<String[]> v = new Vector<String[]>();
			while(!rs.isAfterLast())
			{
				String[] values = new String[4];
				values[0] = rs.getString("componentName");
				values[1] = Integer.toString(rs.getInt("delay"));
				values[2] = rs.getString("description");
				values[3] = rs.getBoolean("active")? "*" : "";
	 		
				v.add(values);
				
				rs.next();
			}
			
			//Put these values in a 2-d string array and return it.
			String[][] returnVal = new String[v.size()][4];
			for(int i = 0; i < returnVal.length; ++i)
			{
				returnVal[i][0] = v.get(i)[0];
				returnVal[i][1] = v.get(i)[1];
				returnVal[i][2] = v.get(i)[2];
				returnVal[i][3] = v.get(i)[3];
			}
			
			return returnVal; 
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public final static String getBitSizeOfIntrinsic(String name)
	{
		try
		{
			//If the component doesn't exist, return null
			if(!doesComponentExist(name))
				return null;
			
			//Get the bitsize of the intrinsic and return it.
			int id = getComponentID(name);	
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + id +"';");
			rs.next();
					
			return Integer.toString(rs.getInt("bitwidth"));
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	//Gets the second bit size necessary for the intrinsic (only for casting intrinsics)
	public final static String getBitSizeOfIntrinsic2(String name)
	{
		try
		{	
			//Query the component
			boolean checkSecondPort = true;
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(name) +"';");
			rs.next();
			
			//Skip the first bit size value and return the second.
			if(checkSecondPort)
				rs.next();
			return Integer.toString(rs.getInt("bitwidth"));
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public final static String[] getSystemClocks(String componentName)
	{
		try 
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS numClocks FROM Ports WHERE id='" + getComponentID(componentName) +"' AND type GLOB '*CLK';");
			rs.next();
			int num = rs.getInt("numClocks");
			
			String[] clks = new String[num];
			
			rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(componentName) +"' AND type GLOB '*CLK';");
			rs.next();
			
			for(int i = 0; i < num; ++i)
			{
				clks[i] = rs.getString("vhdlName");
				rs.next();
			}
			
			return clks;
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return null;
	}
	
	//Returns all the modules that are used in this component and its sub-components.
	public final static String[] getModulesUsed(String componentName)
	{
		try 
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Get the number of modules this component uses.
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS numModules FROM ResourcesUsed WHERE resourceType='MODULE' AND id='" + getComponentID(componentName) +"';");
			rs.next();
			int num = rs.getInt("numModules");
			
			//Make an array of the necessary size and grab all the resources this component uses.
			String[] modules = new String[num];
			rs = stat.executeQuery("SELECT * FROM ResourcesUsed WHERE resourceType='MODULE' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			//Store all the resource id's in a vector since we cannot query the resource name in the middle
			//of reading all these id's.
			Vector<Integer> resourceIDs = new Vector<Integer>();
			for(int i = 0; i < num; ++i)
			{
				resourceIDs.add(new Integer(rs.getInt("resourceID")));
				rs.next();
			}
			
			//Get the component names and put them in our array to return. 
			for(int i = 0; i < num; ++i)
			{
				modules[i] = getComponentFromID(resourceIDs.get(i));
			}
			return modules;
		
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
		}
		
		return null;
	}
	
	//Returns all the modules that are used in this component and its sub-components.
	public final static String[] getModulesCalled(String componentName)
	{
		try 
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Get the number of modules this component uses.
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS numModules FROM ResourcesCalled WHERE resourceType='MODULE' AND id='" + getComponentID(componentName) +"';");
			rs.next();
			int num = rs.getInt("numModules");
			
			//Make an array of the necessary size and grab all the resources this component uses.
			String[] modules = new String[num];
			rs = stat.executeQuery("SELECT * FROM ResourcesCalled WHERE resourceType='MODULE' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			//Store all the resource id's in a vector since we cannot query the resource name in the middle
			//of reading all these id's.
			Vector<Integer> resourceIDs = new Vector<Integer>();
			for(int i = 0; i < num; ++i)
			{
				resourceIDs.add(new Integer(rs.getInt("resourceID")));
				rs.next();
			}
			
			//Get the component names and put them in our array to return. 
			for(int i = 0; i < num; ++i)
			{
				modules[i] = getComponentFromID(resourceIDs.get(i));
			}
			return modules;
		
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
		}
		
		return null;
	}
	
	public final static String getNumElementsCalculationFormulaFromStreamInfo(String componentName, String streamName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM StreamInfo WHERE readableName='" + streamName + "' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			return rs.getString("NumElementsCalculationFormula");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return "";
	}

	public final static String getNumAccessedWindowElementsFromStreamInfo(String componentName, String streamName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM StreamInfo WHERE readableName='" + streamName + "' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			return rs.getString("NumAccessedWindowElements");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return "";
	}

	public final static String getNumTotalWindowElementsFromStreamInfo(String componentName, String streamName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM StreamInfo WHERE readableName='" + streamName + "' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			return rs.getString("NumTotalWindowElements");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return "";
	}
	
	public final static String getPortDataType(String componentName, String portName)
	{
		try
		{
			//If component does not exist, return "N/A"
			if(!doesComponentExist(componentName))
				return "N/A";
			
			//Get the data type of the component port and return it. 
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE readableName='" + portName + "' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			return rs.getString("dataType");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return "N/A";
	}
	
	public final static String getStreamDataType(String componentName, String streamName)
	{
		try
		{
			if(!doesComponentExist(componentName))
				return "N/A";
			
			//Get the data type of the component stream and return it.
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE readableName='" + streamName + "' AND type='STREAM_CHANNEL' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			return rs.getString("dataType");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return "N/A";
	}

	public final static int getNumComponents()
	{
		try
		{
			//Get the number of components in the database.
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS componentCount FROM ComponentInfo;");
			rs.next();
			return rs.getInt("componentCount");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return 0;
		}
	}

	public final static String getComponentType(String componentName)
	{
		try
		{
			//Get the component type of the component and return it.
			ResultSet rs = stat.executeQuery("SELECT type FROM ComponentInfo WHERE componentName='" + componentName + "';");
			rs.next();
		
			return rs.getString("type");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}

	//Get the name of the component of the file that is currently selected in the editor.
	//Used for compilation.
	/*public final static String getComponentFromSelectedFile()
	{
		return getComponentFromSourceFile(new File(Activator.getSelectedFileLocationInEditor()));
	}*/
	
	public static Map<String, Integer> getResourceWeightsForCompile(String componentName)
	{
		try
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Make an array of the necessary size and grab all the resources this component uses.
			ResultSet rs = stat.executeQuery("SELECT * FROM CompileInfo WHERE id='" + getComponentID(componentName) + "';");
			rs.next();
			
			Map<String, Integer> resourceWeights = new TreeMap<String, Integer>();
			
			StringBuffer pipeliningValues = new StringBuffer();
			pipeliningValues.append(rs.getString("pipeliningFlags"));
			
			int max = 0;
			//Get the component names and put them in our array to return. 
			while(pipeliningValues.length() > 0)
			{
				String op = StringUtils.getNextStringValue(pipeliningValues);
				
				if(op.equals("Add") || op.equals("AND") || op.equals("Sub") || op.equals("Mult") ||
				   op.equals("OR") || op.equals("Compare") || op.equals("Copy") || op.equals("Shift") || 
				   op.equals("Mux") || op.equals("XOR"))
				{
					int weight = Integer.parseInt(StringUtils.getNextStringValue(pipeliningValues));
					resourceWeights.put(op.toUpperCase(), weight);
					
					max = Math.max(max, weight);
				}
			}
			
			resourceWeights.put("REGISTER", max);
			
			return resourceWeights;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static Map<String, Integer> getResourcesUsed(String componentName)
	{
		try
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Make an array of the necessary size and grab all the resources this component uses.
			ResultSet rs = stat.executeQuery("SELECT * FROM ResourcesUsed WHERE id='" + getComponentID(componentName) + "';");
			rs.next();
			
			//Store all the resource id's in a vector since we cannot query the resource name in the middle
			//of reading all these id's.
			Vector<String> resourceIDs = new Vector<String>();
			Vector<String> resourceTypes = new Vector<String>();
			Vector<Integer> amountUsed = new Vector<Integer>();
			
			while(rs != null && !rs.isAfterLast())
			{
				resourceIDs.add(rs.getString("resourceID"));
				resourceTypes.add(rs.getString("resourceType"));
				amountUsed.add(rs.getInt("numUsed"));
				
				rs.next();
			}
			
			Map<String, Integer> resourcesUsed = new TreeMap<String, Integer>();
		
			//Get the component names and put them in our array to return. 
			for(int i = 0; i < resourceIDs.size(); ++i)
			{
				if(resourceTypes.get(i).equals("MODULE"))
				{
					resourcesUsed.put(getComponentFromID(Integer.parseInt(resourceIDs.get(i))), amountUsed.get(i));
				}
				else
				{
					resourcesUsed.put(resourceTypes.get(i), amountUsed.get(i));
				}
			}
			
			return resourcesUsed;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static Map<String, Integer> getNonModuleResourcesUsed(String componentName)
	{
		try
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Make an array of the necessary size and grab all the resources this component uses.
			ResultSet rs = stat.executeQuery("SELECT * FROM ResourcesUsed WHERE resourceType!='MODULE' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			//Store all the resource id's in a vector since we cannot query the resource name in the middle
			//of reading all these id's.
			Vector<String> resourceTypes = new Vector<String>();
			Vector<Integer> amountUsed = new Vector<Integer>();
			
			while(!rs.isAfterLast())
			{
				resourceTypes.add(rs.getString("resourceType"));
				amountUsed.add(rs.getInt("numUsed"));
				
				rs.next();
			}
			
			Map<String, Integer> resourcesUsed = new TreeMap<String, Integer>();
		
			//Get the component names and put them in our array to return. 
			for(int i = 0; i < resourceTypes.size(); ++i)
			{
				resourcesUsed.put(resourceTypes.get(i), amountUsed.get(i));
			}
			
			return resourcesUsed;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static String[] getHighLevelOptsUsed(String componentName)
	{
		try
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Get the number of modules this component uses.
			ResultSet rs = stat.executeQuery("SELECT * FROM CompileInfo WHERE id='" + getComponentID(componentName) +"';");
			rs.next();
			
			if(rs.isAfterLast())
				return null;
			
			String[] opts = rs.getString("highFlags").split("\\r?\\n");
			
			return opts;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static String[] getLowLevelOptsUsed(String componentName)
	{
		try
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Get the number of modules this component uses.
			ResultSet rs = stat.executeQuery("SELECT * FROM CompileInfo WHERE id='" + getComponentID(componentName) +"';");
			rs.next();
			
			if(rs.isAfterLast())
				return null;
			
			String[] opts = rs.getString("lowFlags").split("\\r?\\n");
			
			return opts;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static String getOpsPerPipelineStageUsed(String componentName)
	{
		try
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Get the number of modules this component uses.
			ResultSet rs = stat.executeQuery("SELECT * FROM CompileInfo WHERE id='" + getComponentID(componentName) +"';");
			rs.next();
			
			if(rs.isAfterLast())
				return null;
			
			String pipelineFlags = rs.getString("pipeliningFlags");
			
			if(pipelineFlags == null)
				return null;
			
			String[] opts = pipelineFlags.split("\\s+");
			
			return opts[opts.length - 1];
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static String[] getVHDLFilesGenerated(String componentName)
	{
		try
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Get the number of modules this component uses.
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS numFiles FROM FileInfo WHERE fileType LIKE 'VHDL_%' AND id='" + getComponentID(componentName) +"';");
			rs.next();
			int num = rs.getInt("numFiles");
			
			//Make an array of the necessary size and grab all the resources this component uses.
			rs = stat.executeQuery("SELECT * FROM FileInfo WHERE fileType LIKE 'VHDL_%' AND id='" + getComponentID(componentName) +"';");
			rs.next();
		
			String[] files = new String[num];
			//Get the component names and put them in our array to return. 
			for(int i = 0; i < num; ++i)
			{
				files[i] = rs.getString("fileName");
				rs.next();
			}
			
			return files;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static String getDateCompiled(String componentName)
	{
		try
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Get the number of modules this component uses.
			ResultSet rs = stat.executeQuery("SELECT * FROM CompileInfo WHERE id='" + getComponentID(componentName) +"';");
			rs.next();
			
			if(rs.isAfterLast())
				return null;
			
			return rs.getString("timestamp");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	//Get the component name from the given source file.
	public final static String getComponentFromSourceFile(File f)
	{
		try
		{
			//Get the folder and file name of the file.
			String folder = f.getAbsolutePath().replace(f.getName(), "");
			String name = f.getName();
			
			//Get the component that matches that folder and file name.
			ResultSet rs = stat.executeQuery("SELECT * FROM FileInfo WHERE location = '" + folder + "' AND fileName ='" + name + "';");
			rs.next();
			
			if(rs.isAfterLast())
				return null;
			else
				return getComponentFromID(rs.getInt("id"));
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	//Returns an array with all the components names in the database.
	public static final String[] getAllComponents()
	{
		try
		{
			//Make the array to return
			int num = getNumComponents();
			String[] componentNames = new String[num];
			
			//Query the database
			ResultSet rs = stat.executeQuery("SELECT * FROM ComponentInfo");
			rs.next();
			
			//Store all the names
			for(int i = 0; i < num; ++i)
			{
				componentNames[i] = rs.getString("componentName");
				rs.next();
			}
			
			return componentNames;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	
	public static final int getDelay(String componentName)
	{
		try
		{
			//If there is no component, return -1.
			if(!doesComponentExist(componentName))
				return -1;
			
			//Get the delay and return it.
			ResultSet rs = stat.executeQuery("SELECT delay FROM ComponentInfo WHERE id='" + getComponentID(componentName) + "';");
			rs.next();
			return rs.getInt("delay");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return -1;
		}
	}

	
	
	public static final int numberOfInputStreams(String componentName)
	{	
		try
		{
			if(!doesComponentExist(componentName))
				return -1;
			
			//Get the number of input streams and return it.
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS count FROM (SELECT DISTINCT readableName FROM Ports WHERE type='STREAM_CHANNEL' AND direction='IN' AND id='" + getComponentID(componentName) + "');");
			rs.next();
			return rs.getInt("count");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return -1;
		}
	}
	
	public static final int numberOfOutputStreams(String componentName)
	{
		try
		{
			if(!doesComponentExist(componentName))
				return -1;
			
			//Get the number of output streams and return it.
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS count FROM (SELECT DISTINCT readableName FROM Ports WHERE type='STREAM_CHANNEL' AND direction='OUT' AND id='" + getComponentID(componentName) + "');");
			rs.next();
			return rs.getInt("count");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return -1;
		}
	}
	
	public static final int getNumberOfStreams(String componentName)
	{
		try
		{
			if(!doesComponentExist(componentName))
				return -1;
			
			//Get the number of total streams and return it.
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS count FROM (SELECT DISTINCT readableName FROM Ports WHERE type='STREAM_CHANNEL' AND id='" + getComponentID(componentName) + "');");
			rs.next();
			return rs.getInt("count");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return 0;
		}
	}
	
	public static final String[] getStreams(String componentName)
	{
		try
		{
			int numStreams = getNumberOfStreams(componentName);
			
			ResultSet rs = stat.executeQuery("SELECT DISTINCT readableName FROM Ports WHERE type='STREAM_CHANNEL' AND id='" + getComponentID(componentName) + "' ORDER BY portNum;");
			rs.next();
			
			String[] streams = new String[numStreams];
			
			for(int i = 0; i < numStreams; ++i)
			{
				streams[i] = rs.getString("readableName");
				rs.next();
			}
			
			return streams;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final String[] getInputStreams(String componentName)
	{
		try
		{
			int numStreams = numberOfInputStreams(componentName);
			
			ResultSet rs = stat.executeQuery("SELECT DISTINCT readableName FROM Ports WHERE type='STREAM_CHANNEL' AND direction='IN' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			String[] streams = new String[numStreams];
			
			for(int i = 0; i < numStreams; ++i)
			{
				streams[i] = rs.getString("readableName");
				rs.next();
			}
			
			return streams;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final String[] getOutputStreams(String componentName)
	{
		try
		{
			int numStreams = numberOfOutputStreams(componentName);
			
			ResultSet rs = stat.executeQuery("SELECT DISTINCT readableName FROM Ports WHERE type='STREAM_CHANNEL' AND direction='OUT' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			String[] streams = new String[numStreams];
			
			for(int i = 0; i < numStreams; ++i)
			{
				streams[i] = rs.getString("readableName");
				rs.next();
			}
			
			return streams;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	/*public static final String[] getStreamIndices(String componentName)
	{
		try
		{
			Vector<String> indices = new Vector<String>();

			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(componentName) + "' AND type='STREAM_INDEX';");
			rs.next();
			
			while(!rs.isAfterLast())
			{
				indices.add(rs.getString("vhdlName"));
				rs.next();
			}
			
			String[] array = new String[0];
			
			array = indices.toArray(array);
			return array;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final String[] getInputStreamIndicies(String componentName)
	{
		try
		{
			Vector<String> indices = new Vector<String>();

			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(componentName) + "' AND direction='IN' AND type='STREAM_INDEX';");
			rs.next();
			
			while(!rs.isAfterLast())
			{
				indices.add(rs.getString("vhdlName"));
				rs.next();
			}
			
			String[] array = new String[0];
			
			array = indices.toArray(array);
			return array;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final String[] getOutputStreamIndicies(String componentName)
	{
		try
		{
			Vector<String> indices = new Vector<String>();

			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(componentName) + "' AND direction='OUT' AND type='STREAM_INDEX';");
			rs.next();
			
			while(!rs.isAfterLast())
			{
				indices.add(rs.getString("vhdlName"));
				rs.next();
			}
			
			String[] array = new String[0];
			
			array = indices.toArray(array);
			return array;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}*/
	
	public static final int getNumStreamPorts(String componentName, String streamName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS count FROM Ports WHERE id='" + getComponentID(componentName) + "' AND readableName='" + streamName + "';");
			rs.next();
			return rs.getInt("count");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return 0;
		}
	}
	
	public static final String[] getStreamPorts(String componentName, String streamName)
	{
		try
		{
			int numChannels = getNumStreamPorts(componentName, streamName);
			String[] channels = new String[numChannels];
			
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(componentName) + "' AND readableName='" + streamName + "';");
			rs.next();
			
			for(int i = 0; i < numChannels; ++i)
			{
				channels[i] = rs.getString("vhdlName");
				rs.next();
			}
			
			return channels;
			
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final int getNumDebugPorts(String componentName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS portCount FROM Ports WHERE type='DEBUG' AND id = '" + getComponentID(componentName) + "';");
			rs.next();

			return rs.getInt("portCount");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return 0;
	}
	
	public static final String[] getDebugPorts(String componentName)
	{
		try
		{
			int numDebugPorts = getNumDebugPorts(componentName);
			String[] ports = new String[numDebugPorts];
			
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE type='DEBUG' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			for(int i = 0; i < numDebugPorts; ++i)
			{
				ports[i] = rs.getString("vhdlName");
				rs.next();
			}
			
			return ports;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final int getNumPorts(String componentName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS portCount FROM Ports WHERE type='REGISTER' AND id ='" + getComponentID(componentName) + "';");
			rs.next();

			return rs.getInt("portCount");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return 0;
	}
	
	public static final String[] getPorts(String componentName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE type='REGISTER' AND id ='" + getComponentID(componentName) + "';");
			rs.next();
			
			Vector<String> ports = new Vector<String>();
			
			for(; rs.isAfterLast() == false; rs.next())
			{
				ports.add(rs.getString("readableName"));
			}
			
			String[] portArray = new String[0];
			portArray = ports.toArray(portArray);
			return portArray;
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return null;
	}
	
	public static final String[] getAllPortInformation(String componentName, String column)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE type='REGISTER' AND id ='" + getComponentID(componentName) + "';");
			rs.next();
			
			Vector<String> ports = new Vector<String>();
			
			for(; rs.isAfterLast() == false; rs.next())
			{
				ports.add(rs.getString(column));
			}
			
			String[] portArray = new String[1];
			portArray = ports.toArray(portArray);
			return portArray;
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return null;
	}
	
	public static final String getPortName(String componentName, int portNum)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE type='REGISTER' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			for(int i = 0; i < portNum; ++i)
			{
				rs.next();
			}
			return rs.getString("readableName");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final int getTotalPorts(String componentName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS portCount FROM Ports WHERE id = '" + getComponentID(componentName) + "';");
			rs.next();

			return rs.getInt("portCount");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return 0;
	}
	
	public static final String getVHDLNameFromOrder(String componentName, int num)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id = '" + getComponentID(componentName) + "' AND portNum=" + (num + 1) + ";");
			rs.next();

			return rs.getString("vhdlName");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return "";
	}
	
	public static final String[] getInputPorts(String componentName)
	{
		Vector<String> ports = new Vector<String>();
		
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE type='REGISTER' AND direction='IN' AND id='" + getComponentID(componentName) + "';");
			rs.next();
				
			for(; !rs.isAfterLast(); rs.next())
			{
				ports.add(rs.getString("readableName"));
			}
		}
		catch(Exception e)
		{
		
		}
		
		String[] array = new String[ports.size()];
		for(int i = 0; i < ports.size(); ++i)
			array[i] = ports.get(i);
		
		return array;
	}
	
	public static final String[] getOutputPorts(String componentName)
	{
		Vector<String> ports = new Vector<String>();
	
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE type='REGISTER' AND direction='OUT' AND id='" + getComponentID(componentName) + "';");
			rs.next();
				
			for(; !rs.isAfterLast(); rs.next())
			{
				ports.add(rs.getString("readableName"));
			}
		}
		catch(Exception e)
		{
		
		}
		
		String[] array = new String[ports.size()];
		for(int i = 0; i < ports.size(); ++i)
			array[i] = ports.get(i);
		
		return array;
	}
	
	public static final String[] getOutputVHDLPorts(String componentName)
	{
		Vector<String> ports = new Vector<String>();
		
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE type='REGISTER' AND direction='OUT' AND id='" + getComponentID(componentName) + "';");
			rs.next();
				
			for(; !rs.isAfterLast(); rs.next())
			{
				ports.add(rs.getString("vhdlName"));
			}
		}
		catch(Exception e)
		{
		
		}
		
		String[] array = new String[ports.size()];
		for(int i = 0; i < ports.size(); ++i)
			array[i] = ports.get(i);
		
		return array;
	}
	
	public static final String getVHDLPortName(String componentName, String readableName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE type='REGISTER' AND readableName='" + readableName + "' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			
			return rs.getString("vhdlName");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return "";
	}
	
	public static final String getVHDLPortName(String componentName, int portNum)
	{ 
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE type='REGISTER' AND id='" + getComponentID(componentName) + "';");
			rs.next();
			for(int i = 0; i < portNum; ++i)
			{
				rs.next();
			}
			return rs.getString("vhdlName");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final String getStreamPortDirection(String componentName, String streamName, String portName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT direction FROM Ports WHERE readableName='" + streamName +"' AND id='" + getComponentID(componentName) + "' AND vhdlName='" + portName + "';");
			rs.next();

			return rs.getString("direction");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final String getStreamPortType(String componentName, String streamName, String portName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT type FROM Ports WHERE readableName='" + streamName +"' AND id='" + getComponentID(componentName) + "' AND vhdlName='" + portName + "';");
			rs.next();

			return rs.getString("type");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final int getStreamPortSize(String componentName, String streamName, String portName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT bitwidth FROM Ports WHERE readableName='" + streamName +"' AND id='" + getComponentID(componentName) + "' AND vhdlName='" + portName + "';");
			rs.next();

			return rs.getInt("bitwidth");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return 0;
		}
	}
	
	public static final int getDebugPortSize(String componentName, String portName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT bitwidth FROM Ports WHERE type='DEBUG' AND id='" + getComponentID(componentName) + "' AND vhdlName='" + portName + "';");
			rs.next();
	
			return rs.getInt("bitwidth");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return 0;
		}
	}
	
	public static final String getPortDirectionFromVHDLName(String componentName, String vhdlName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(componentName) + "' AND vhdlName='" + vhdlName + "';");
			rs.next();

			return rs.getString("direction");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final String getPortDirection(String componentName, String portName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT direction FROM Ports WHERE type='REGISTER' AND id='" + getComponentID(componentName) + "' AND readableName='" + portName + "';");
			rs.next();

			return rs.getString("direction");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
		
	}
	
	public static final int getPortSize(String componentName, String portName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT bitwidth FROM Ports WHERE type='REGISTER' AND id='" + getComponentID(componentName) + "' AND readableName='" + portName + "';");
			rs.next();
	
			return rs.getInt("bitwidth");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return 0;
		}
	}
	
	public static final int getPortSizeFromVHDLName(String componentName, String vhdlName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(componentName) + "' AND vhdlName='" + vhdlName + "';");
			rs.next();
	
			return rs.getInt("bitwidth");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return 0;
		}
	}
	
	public static final boolean doesStreamExist(String componentName, String streamName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS count FROM Ports WHERE type='STREAM_CHANNEL' AND id='" + getComponentID(componentName) + "' AND readableName='" + streamName + "';");
			rs.next();
			return !rs.isAfterLast();
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
	}
	
	public static final int getStreamBitSize(String componentName, String streamName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT bitwidth FROM Ports WHERE type='STREAM_CHANNEL' AND id='" + getComponentID(componentName) + "' AND readableName='" + streamName + "';");
			rs.next();
	
			return rs.getInt("bitwidth");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return 0;
		}
	}

	public static final int getNumStreamChannels(String componentName, String streamName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS count FROM Ports WHERE type='STREAM_CHANNEL' AND id='" + getComponentID(componentName) + "' AND readableName='" + streamName + "';");
			rs.next();
			return rs.getInt("count");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return 0;
		}
	}
	
	public static final int getNumStreamAddresses(String componentName, String streamName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS count FROM Ports WHERE type='STREAM_ADDRESS_BASE' AND id='" + getComponentID(componentName) + "' AND readableName='" + streamName + "';");
			rs.next();
			return rs.getInt("count");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return 0;
		}
	}
	
	public static final int getNumStreamIndices(String componentName, String streamName)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS count FROM Ports WHERE type='STREAM_INDEX' AND id='" + getComponentID(componentName) + "' AND readableName='" + streamName + "';");
			rs.next();
			return rs.getInt("count");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return 0;
		}
	}
	
	public static final String[] getStreamIndices(String componentName, String streamName)
	{
		try
		{
			String[] indices = new String[getNumStreamIndices(componentName, streamName)]; 
			
			if(indices.length == 0)
				return indices;
			
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE type='STREAM_INDEX' AND id='" + getComponentID(componentName) + "' AND readableName='" + streamName + "';");
			rs.next();
			
			for(int i = 0; i < indices.length; ++i)
			{
				indices[i] = rs.getString("vhdlName");
				rs.next();
			}
			
			return indices;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	static public int CROSS_CLK_PORT = 0;
	static public int STOP_ACCESS_PORT = 1;
	static public int ADDRESS_PORT = 2;
	static public int ADDRESS_READY_PORT = 3;
	static public int ENABLE_ACCESS_PORT = 4;
	static public int STREAM_ADDRESS_STALL_PORT = 5;
	static public int STREAM_ADDRESS_CLK_PORT = 6;
	
	public static final String[] getNonChannelStreamPorts(String componentName, String streamName)
	{
		String[] nonChannels = new String[7];
		
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(componentName) + "' AND type<>'STREAM_CHANNEL' AND type<>'REGISTER' AND readableName='" + streamName + "';");
			rs.next();
			
			for(; !rs.isAfterLast(); rs.next())
			{
				if(rs.getString("type").equals("STREAM_CROSS_CLK"))
				{
					nonChannels[CROSS_CLK_PORT] = rs.getString("vhdlName");
				}
				else if(rs.getString("type").equals("STREAM_STOP_ACCESS"))
				{
					nonChannels[STOP_ACCESS_PORT] = rs.getString("vhdlName");
				}
				else if(rs.getString("type").equals("STREAM_ADDRESS"))
				{
					nonChannels[ADDRESS_PORT] = rs.getString("vhdlName");
				}
				else if(rs.getString("type").equals("STREAM_ADDRESS_RDY"))
				{
					nonChannels[ADDRESS_READY_PORT] = rs.getString("vhdlName");
				}
				else if(rs.getString("type").equals("STREAM_ENABLE_ACCESS"))
				{
					nonChannels[ENABLE_ACCESS_PORT] = rs.getString("vhdlName");
				}
				else if(rs.getString("type").equals("STREAM_ADDRESS_STALL"))
				{
					nonChannels[STREAM_ADDRESS_STALL_PORT] = rs.getString("vhdlName");
				}
				else if(rs.getString("type").equals("STREAM_ADDRESS_CLK"))
				{
					nonChannels[STREAM_ADDRESS_CLK_PORT] = rs.getString("vhdlName");
				}
			}
			
			return nonChannels;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final String[] getStreamChannelPorts(String componentName, String streamName)
	{
		if(!doesComponentExist(componentName))
			return null;
		if(!doesStreamExist(componentName, streamName))
			return null;
		
		int numChannels = getNumStreamChannels(componentName, streamName);
		
		String[] channelPorts = new String[numChannels * 3];
		String[] channels = DatabaseInterface.getStreamChannels(componentName, streamName);
		String[] channelBases = DatabaseInterface.getStreamPortsOfType(componentName, streamName, "STREAM_ADDRESS_BASE");
		String[] channelCounts = DatabaseInterface.getStreamPortsOfType(componentName, streamName, "STREAM_ADDRESS_COUNT");
		
		for(int i = 0; i < numChannels; ++i)
		{
			channelPorts[i * 3] = channels[i];
			channelPorts[i * 3 + 1] = channelBases.length == 1? channelBases[0] : channelBases[i];
			channelPorts[i * 3 + 2] = channelCounts.length == 1? channelCounts[0] : channelCounts[i];
		}
		
		return channelPorts;
	}
	
	public static final String[] getStreamPortsOfType(String componentName, String streamName, String type)
	{
		try 
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Get the number of modules this component uses.
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS numPorts FROM Ports WHERE type='" + type + "' AND readableName='" + streamName + "' AND id='" + getComponentID(componentName) +"';");
			rs.next();
			int num = rs.getInt("numPorts");
			
			//Make an array of the necessary size and grab all the resources this component uses.
			String[] ports = new String[num];
			rs = stat.executeQuery("SELECT * FROM Ports WHERE type='" + type + "' AND readableName='" + streamName + "' AND id='" + getComponentID(componentName) +"';");
			rs.next();
						
			//Get the component names and put them in our array to return. 
			for(int i = 0; i < num && !rs.isAfterLast(); ++i)
			{
				ports[i] = rs.getString("vhdlName");
				rs.next();
			}
			
			return ports;	
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
		}
		
		return null;
	}
	
	public static final String[] getStreamAddressBases(String componentName, String streamName)
	{
		try 
		{
			//If the component doesn't exist, return null.
			if(!doesComponentExist(componentName))
				return null;
			
			//Get the number of modules this component uses.
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS numPorts FROM Ports WHERE type='STREAM_ADDRESS_BASE' AND readableName='" + streamName + "' AND id='" + getComponentID(componentName) +"';");
			rs.next();
			int num = rs.getInt("numPorts");
			
			//Make an array of the necessary size and grab all the resources this component uses.
			String[] addressBases = new String[num];
			rs = stat.executeQuery("SELECT * FROM Ports WHERE type='STREAM_ADDRESS_BASE' AND readableName='" + streamName + "' AND id='" + getComponentID(componentName) +"';");
			rs.next();
						
			//Get the component names and put them in our array to return. 
			for(int i = 0; i < num && !rs.isAfterLast(); ++i)
			{
				addressBases[i] = rs.getString("vhdlName");
				rs.next();
			}
			
			return addressBases;	
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
		}
		
		return null;
	}
	
	public static final String getStreamAddressStall(String componentName, String streamName)
	{
		if(!doesComponentExist(componentName))
			return "";
		if(!doesStreamExist(componentName, streamName))
			return "";
		
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(componentName) + "' AND type='STREAM_ADDRESS_STALL' AND readableName='" + streamName + "';");
			rs.next();
			
			return rs.getString("vhdlName");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
		
		
	}
	
	public static final String[] getStreamChannels(String componentName, String streamName)
	{
		try
		{
			int numChannels = getNumStreamChannels(componentName, streamName);
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(componentName) + "' AND type='STREAM_CHANNEL' AND readableName='" + streamName + "';");
			rs.next();
			
			String[] channels = new String[numChannels];
			for(int i = 0; i < numChannels; ++i)
			{
				channels[i] = rs.getString("vhdlName");
				rs.next();
			}
			return channels;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final String[] getStreamAddresses(String componentName, String streamName)
	{
		try
		{
			int numChannels = getNumStreamAddresses(componentName, streamName);
			// Modified by Jason to add _BASE
			ResultSet rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(componentName) + "' AND type='STREAM_ADDRESS_BASE' AND readableName='" + streamName + "';");
			rs.next();
			
			String[] channels = new String[numChannels];
			for(int i = 0; i < numChannels; ++i)
			{
				channels[i] = rs.getString("vhdlName");
				rs.next();
			}
			return channels;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final boolean doesComponentHaveFullMatch(String componentName, int bitWidth, int delay)
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS ex FROM ComponentInfo WHERE id='" + getComponentID(componentName) + "' AND delay=" + Integer.toString(delay) + ";");
			rs.next();
			return rs.getInt("ex") > 0;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
	}
	
	public static final boolean removeComponent(String componentName)
	{	
		try
		{
			//Activator.getDefault().openLibrary();
			try 
			{
				stat.execute("DELETE FROM Ports WHERE id LIKE " + getComponentID(componentName) + "");
				stat.execute("DELETE FROM ResourcesUsed WHERE id LIKE " + getComponentID(componentName) + "");
				stat.execute("DELETE FROM ResourcesCalled WHERE id LIKE " + getComponentID(componentName) + "");
				stat.execute("DELETE FROM FileInfo WHERE id LIKE " + getComponentID(componentName) + "");
				stat.execute("DELETE FROM CompileInfo WHERE id LIKE " + getComponentID(componentName) + "");
				//stat.execute("DELETE FROM StreamInfo WHERE id LIKE " + getComponentID(componentName) + "");
				stat.execute("DELETE FROM ComponentInfo WHERE componentName LIKE " + "\"" + componentName +"\"");
			}
			
			catch (Exception e) 
			{
				e.printStackTrace();
				closeConnection();
				return false;
			}
			//closeConnection();
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
		return true;
	}
	
	public static final boolean removeComponent(String componentName, int bitSize, int delay)
	{
		try
		{
			stat.execute("DELETE FROM ComponentInfo WHERE componentName=" + "\"" + componentName +"\"");
			stat.execute("DELETE FROM Ports WHERE componentName= " + "\"" + componentName + "\"");
			//stat.execute("DELETE FROM ModulesUsed WHERE componentName= " + "\"" + componentName + "\"");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
		
		return true;
	}

	public static final boolean addComponent(String component_name, int delay) throws SQLException
	{
		// IP component_name exist exit
		// You may want to consider making the table unique in some way to prevent multiple instances
		// That would help avoid this check
		ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS ex FROM ComponentInfo WHERE componentName='" + component_name + "';");
		rs.next();
		if( rs.getInt("ex") > 0 ) 
			return false;

		// create the port table
		//stat.execute("CREATE TABLE " + component_name + "_Ports(port varchar(255), port_dir varchar(5), port_size int)");

		String timestamp = new Date().toString();
		String ID = DatabaseInterface.createCompileInfo(null, timestamp, null, null, null, null, null, true); 
		
		String sql = "INSERT INTO ComponentInfo(id, componentName, type, delay, active) VALUES(" + ID + ",'" + component_name + "','MODULE'," + delay + ", '1');";
		stat.execute(sql);
		
		//sql = "INSERT INTO Ports(componentName, type, delay) VALUES('" + component_name + "','MODULE'," + delay + ");";
		
		
		//updateListeners();
		return true;
	}

	public static final boolean addComponent(String component_name, int delay, String portOrder, String version)
	{
		try
		{
			int id =  getComponentID(component_name);
			
			if(id != -1)
			{
				return false;
			}
			
			String timestamp = new Date().toString();
			String ID = DatabaseInterface.createCompileInfo(null, timestamp, version, null, null, null, null, true); 
			
			String sql = "INSERT INTO ComponentInfo(id, componentName, type, portOrder, delay, active) VALUES(" + ID + ",'" + component_name + "','MODULE','" + portOrder + "'," + delay + ", '1');";
			stat.execute(sql);
			
			//updateListeners();
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
		return true;
	}
	
	//Needs to be case insensitive
	public static final boolean doesComponentExist(String name)
	{
		try
		{
			if(!conn.getMetaData().getTables(null, "SCHEMA", "ComponentInfo", null).next())
				return false;
			
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS count FROM ComponentInfo WHERE componentName='" + name + "';");
			rs.next();
			return rs.getInt("count") > 0;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
	}
	
	public static final boolean doesIPCoreExist(String name) throws SQLException
	{
		ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS ex FROM ComponentInfo WHERE componentName='" + name + "';");
		rs.next();
		return rs.getInt("ex") > 0;
	}
	
	public static final int getNumVHDLPorts(String componentName) throws SQLException
	{
		try
		{
			if(!doesComponentExist(componentName))
				return 0;
			
			ResultSet rs = stat.executeQuery("SELECT COUNT(*) AS numPorts FROM Ports WHERE id='" + getComponentID(componentName) + "';");
			rs.next();
			return rs.getInt("numPorts");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		return 0;
	}
	
	public static final String getPortOrder(String componentName)
	{
		ResultSet rs;
		try 
		{
			rs = stat.executeQuery("SELECT portOrder FROM ComponentInfo WHERE id='" + getComponentID(componentName) +"';");
			rs.next();
			return rs.getString("portOrder");
		} 
		catch (SQLException e) 
		{
			e.printStackTrace();
			return "";
		}
	}
	
	public static final String[] getVHDLPorts(String componentName, int index)
	{
		//TODO
		ResultSet rs;
		try 
		{
			//If the index is higher than the number of ports, return null.
			if(getNumVHDLPorts(componentName) <= index)
				return null;
			
			rs = stat.executeQuery("SELECT * FROM Ports WHERE id='" + getComponentID(componentName) + "' ORDER BY portNum");
			rs.next();
		} 
		catch (Exception e1) 
		{
			e1.printStackTrace();
			return null;
		}
		
		for(int i = 0; i < index; ++i)
		{
			try 
			{
				rs.next();
			} 
			catch (Exception e) 
			{
				e.printStackTrace();
			}
		}
		
		String[] values = new String[5];
	
		try 
		{
			values[0] = rs.getString("readableName"); //readableName
			values[1] = rs.getString("type"); //type
			values[2] = rs.getString("vhdlName"); //vhdlName
			values[3] = rs.getString("direction"); //direction
			values[4] = rs.getString("bitwidth"); //bitSize			
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
			return null;
		}
		
		return values;
	}

	public static final void removeIntrinsics()
	{
		try
		{
			ResultSet rs = stat.executeQuery("SELECT * FROM ComponentInfo WHERE type!='SYSTEM' AND type!='MODULE';");
			rs.next();
			
			Vector<String> comps = new Vector<String>();
			
			for(;!rs.isAfterLast();rs.next())
			{
				comps.add(rs.getString("componentName"));
			}
			
			for(int i = 0; i < comps.size(); ++i)
			{
				stat.executeUpdate("DELETE FROM Ports WHERE id='" + getComponentID(comps.get(i)) +"';"); 
				stat.executeUpdate("DELETE FROM CompileInfo WHERE id='" + getComponentID(comps.get(i)) +"';"); 
			}
			
			stat.executeUpdate("DELETE FROM ComponentInfo WHERE type !='SYSTEM' AND type !='MODULE';");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}

	public static final void addIntrinsic(String componentName, String type, int bitSize, int bitSize2, int delay, boolean active, String description)
	{
		try
		{
			int portNum = 1;
			
			String[] ports = {"a", "b", "result"};
			
			if(type.equals("INT_TO_FP"))
				ports = new String[]{"intInput", "", "fpOutput"};
			else if(type.equals("FP_TO_INT"))
				ports = new String[]{"fpInput", "", "intOutput"};
			else if(type.equals("FP_TO_FP"))
				ports = new String[]{"fpInput", "", "fpOutput"};
			
			String timestamp = new Date().toString();
			String ID = DatabaseInterface.createCompileInfo(null, timestamp, null, null, null, null, null, true); 
			
			String sql = "INSERT INTO ComponentInfo(id, componentName,type,delay,active,description) ";
			sql += "VALUES(" + ID + ",'" + componentName + "','" + type + "','" + delay + "'," + (active? 1 : 0) + ",'" + description + "');";
			
			stat.executeUpdate(sql);
			
			sql = "INSERT INTO Ports(id,type,portNum,vhdlName,direction,bitWidth) ";
			
			//if(type.equals("FP_TO_INT"))
			//	sql += "VALUES('" + componentName + "','REGISTER'," + (portNum++) + ",'" + ports[0] + "','IN'," + 32 + ");";
			//else
				sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[0] + "','IN'," + bitSize + ");";
			
			stat.executeUpdate(sql);
			
			/*if(!type.equals("INT_TO_FP") && !type.equals("FP_TO_INT"))
			{
				sql = "INSERT INTO Ports(componentName,type,portNum,vhdlName,direction,bitWidth) ";
				sql += "VALUES('" + componentName + "','REGISTER'," + (portNum++) + ",'" + ports[1] + "','IN'," + bitSize + ");";
				stat.executeUpdate(sql);
			}*/
			
			sql = "INSERT INTO Ports(id,type,portNum,vhdlName,direction,bitWidth) ";
			
			if(type.contains("EQUAL") || type.contains("THAN"))
				sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[2] + "','OUT'," + 1 + ");";
			else
				sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[2] + "','OUT'," + bitSize2 + ");";
			
			stat.executeUpdate(sql);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static final void addIntrinsic(String componentName, String type, int bitSize, int delay, boolean active, String description)
	{
		try
		{
			int portNum = 1;
			
			String[] ports = {"a", "b", "result"};
			
			String timestamp = new Date().toString();
			String ID = DatabaseInterface.createCompileInfo(null, timestamp, null, null, null, null, null, true); 
			
			//Add the component to the component table
			if(type.equals("INT_DIV"))
				ports = new String[]{"dividend", "divisor", "quotient"};
			
			String sql = "INSERT INTO ComponentInfo(id, componentName,type,delay,active,description) ";
			sql += "VALUES(" + ID + ",'" + componentName + "','" + type + "','" + delay + "'," + (active? 1 : 0) + ",'" + description + "');";
			
			stat.executeUpdate(sql);
			
		
			//Add the ports to the port table
			sql = "INSERT INTO Ports(id,type,portNum,vhdlName,direction,bitWidth) ";
			
			if(type.equals("FP_TO_INT"))
				sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[0] + "','IN'," + 32 + ");";
			else
				sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[0] + "','IN'," + bitSize + ");";
			
			stat.executeUpdate(sql);
			
			if(!type.equals("INT_TO_FP") && !type.equals("FP_TO_INT"))
			{
				sql = "INSERT INTO Ports(id,type,portNum,vhdlName,direction,bitWidth) ";
				sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[1] + "','IN'," + bitSize + ");";
				stat.executeUpdate(sql);
			}
			
			sql = "INSERT INTO Ports(id,type,portNum,vhdlName,direction,bitWidth) ";
			
			if(type.equals("INT_TO_FP"))
				sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[2] + "','OUT'," + 32 + ");";
			else if(type.contains("EQUAL") || type.contains("THAN"))
				sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[2] + "','OUT'," + 1 + ");";
			else
				sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[2] + "','OUT'," + bitSize + ");";
			
			stat.executeUpdate(sql);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static final void addVoterIntrinsic(String componentName, String type, int bitSize, int delay, boolean active, String description)
	{
		try
		{
			int portNum = 1;
			
			String[] ports = {"error", "val0_in", "val1_in", "val2_in", "val0_out", "val1_out", "val2_out"};
			
			String timestamp = new Date().toString();
			String ID = DatabaseInterface.createCompileInfo(null, timestamp, null, null, null, null, null, true); 
			
			String sql = "INSERT INTO ComponentInfo(id, componentName,type,delay,active,description) ";
			sql += "VALUES(" + ID + ",'" + componentName + "','" + type + "','" + delay + "'," + (active? 1 : 0) + ",'" + description + "');";
			
			stat.executeUpdate(sql);
			
		
			//Add the ports to the port table
			sql = "INSERT INTO Ports(id,type,portNum,vhdlName,direction,bitWidth) ";
			sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[0] + "','OUT'," + 1 + ");";
			stat.executeUpdate(sql);
			
			sql = "INSERT INTO Ports(id,type,portNum,vhdlName,direction,bitWidth) ";
			sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[1] + "','IN'," + bitSize + ");";
			stat.executeUpdate(sql);
			
			sql = "INSERT INTO Ports(id,type,portNum,vhdlName,direction,bitWidth) ";
			sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[2] + "','IN'," + bitSize + ");";
			stat.executeUpdate(sql);
			
			sql = "INSERT INTO Ports(id,type,portNum,vhdlName,direction,bitWidth) ";
			sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[3] + "','IN'," + bitSize + ");";
			stat.executeUpdate(sql);
			
			sql = "INSERT INTO Ports(id,type,portNum,vhdlName,direction,bitWidth) ";
			sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[4] + "','OUT'," + bitSize + ");";
			stat.executeUpdate(sql);
			
			sql = "INSERT INTO Ports(id,type,portNum,vhdlName,direction,bitWidth) ";
			sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[5] + "','OUT'," + bitSize + ");";
			stat.executeUpdate(sql);
			
			sql = "INSERT INTO Ports(id,type,portNum,vhdlName,direction,bitWidth) ";
			sql += "VALUES('" + ID + "','REGISTER'," + (portNum++) + ",'" + ports[6] + "','OUT'," + bitSize + ");";
			stat.executeUpdate(sql);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static final boolean addPort(String component_name, String port_name, String direction, int portNum, int size, String dataType) throws SQLException{
		//ResultSet rs = stat.executeQuery("SELECT ports AS ex FROM IPcores WHERE name='" + component_name + "';");
		//rs.next();
		int id = getComponentID(component_name);
		
		String sql = "INSERT INTO Ports(id,readableName,type,portNum,vhdlName,direction,bitwidth, dataType) ";
		sql += "VALUES('" + id + "','" + port_name + "','REGISTER'," + portNum + ",'" + port_name + "','" + direction + "'," + size +",'" + dataType +"');";
		if( stat.executeUpdate(sql) > 0 )
		{	
			//updateListeners();
			return true;
		}
		else
			return false;
	}
}
