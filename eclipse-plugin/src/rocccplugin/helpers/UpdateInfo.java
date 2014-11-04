package rocccplugin.helpers;

import java.util.Vector;

import org.osgi.framework.Version;

public class UpdateInfo 
{
	static public Vector<UpdateInfo> updateInfo = new Vector<UpdateInfo>();
	
	public String pluginName;
	public String urlLocation;
	public String pluginLocation;
	public Version version;
	
	public UpdateInfo(String name, String url, Version v)
	{
		pluginName = name;
		urlLocation = url;
		version = v;
	}
	
	static public void addUpdateInfo(String name, String url, Version v)
	{
		updateInfo.add(new UpdateInfo(name, url, v));
	}
}
