package rocccplugin.helpers;

public class FlagData
{
	private String flag;
	private String[] args;
	
	public FlagData()
	{
	}
	
	public FlagData(String f, String[] data)
	{
		flag = f; 
		args = data;
	}
	
	public void setFlag(String f)
	{
		flag = f;
	}
	
	public void setArgs(String[] data)
	{
		args = data;
	}
	
	public String getFlag()
	{
		return flag;
	}
	
	public String[] getArgs()
	{
		return args;
	}
}
