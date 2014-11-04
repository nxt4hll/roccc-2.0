package rocccplugin.helpers;

public class ConnectionData 
{
	public String platform;
	public String category;
	public String[] data;
	
	public ConnectionData(String platform, String category)
	{
		this.platform = platform;
		this.category = category;
	}
	
	public ConnectionData(String platform, String category, String[] data)
	{
		this.platform = platform;
		this.category = category;
		this.data = data;
	}
	
	public boolean doesResourceExist(String res)
	{
		return data[0].compareTo(res) == 0;
	}
}
