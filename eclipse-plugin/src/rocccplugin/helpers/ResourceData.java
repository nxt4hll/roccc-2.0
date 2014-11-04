package rocccplugin.helpers;

public class ResourceData 
{
	public String name;
	public int num;
	
	public ResourceData(String name, int num)
	{
		this.name = name;
		this.num = num;
	}
	
	public boolean doesResourceExist(String res)
	{
		return res.compareTo(name) == 0;
	}
	
	public int getAmount()
	{
		return num;
	}
	
	public void setAmount(int num)
	{
		this.num = num;
	}
	
	
}
