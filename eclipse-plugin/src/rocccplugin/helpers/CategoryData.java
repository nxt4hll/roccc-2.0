package rocccplugin.helpers;

import java.util.Vector;

public class CategoryData 
{
	public String name;
	public Vector<ResourceData> rData;
	
	CategoryData()
	{
		rData = new Vector<ResourceData>();
		name = "";
	}
	
	public CategoryData(String name)
	{	
		rData = new Vector<ResourceData>();
		this.name = name;
	}
	
	public boolean doesResourceExist(String res)
	{
		for(int i = 0; i < rData.size(); ++i)
		{
			if(rData.get(i).doesResourceExist(res))
				return true;
		}
		return false;
	}
	
	public ResourceData getResource(String res)
	{
		for(int i = 0; i < rData.size(); ++i)
		{
			if(rData.get(i).doesResourceExist(res))
			{
				return rData.get(i);
			}
		}
		return null;
	}
	
	public ResourceData getFirstAvailableResource()
	{
		for(int i = 0; i < rData.size(); ++i)
		{
			if(rData.get(i).num > 0)
				return rData.get(i);
		}
		return null;
	}
	
	public void addResource(String res, int num)
	{		
		rData.add(new ResourceData(res, num));
	}
	
	public int getAmountInCategory()
	{
		int total = 0;
		for(int i = 0; i < rData.size(); ++i)
		{
			total += rData.get(i).getAmount();
		}
		
		return total;
	}
}
