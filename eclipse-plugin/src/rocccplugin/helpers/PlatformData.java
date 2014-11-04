package rocccplugin.helpers;

import java.util.Vector;

public class PlatformData 
{
	public Vector<CategoryData> cData;
	public String name;
	
	public PlatformData(String name)
	{
		this.name = name;
		cData = new Vector<CategoryData>();
	}
	
	public boolean doesResourceExist(String res)
	{
		for(int i = 0; i < cData.size(); ++i)
		{
			if(cData.get(i).doesResourceExist(res))
				return true;
		}
		return false;
	}
	
	public String getCategory(String res)
	{
		for(int i = 0; i < cData.size(); ++i)
		{
			if(cData.get(i).doesResourceExist(res))
			{
				return cData.get(i).name;
			}
		}
		return "";
	}
	
	public ResourceData getResource(String res)
	{
		ResourceData data;
		for(int i = 0; i < cData.size(); ++i)
		{
			data = cData.get(i).getResource(res);
			if(data != null)
			{
				return data;
			}
		}
		return null;
	}
	
	public void addResource(String category, String resource, int num)
	{
		for(int i = 0; i < cData.size(); ++i)
		{
			if(cData.get(i).name.compareTo(category) == 0)
			{
				cData.get(i).addResource(resource, num);
				return;
			}
		}	
		
		cData.add(new CategoryData(category));
		cData.lastElement().addResource(resource, num);
	}
	
	public int numberOfCategorys()
	{
		return cData.size();
	}
	
	public int getAmountInCategory(String category)
	{
		for(int i = 0; i < cData.size(); ++i)
		{
			if(cData.get(i).name.compareTo(category) == 0)
				return cData.get(i).getAmountInCategory();
		}
		return 0;
	}
	
	public CategoryData getCategory(int index)
	{
		return cData.get(index);
	}
	
	public CategoryData getCategoryData(String cat)
	{
		for(int i = 0; i < cData.size(); ++i)
		{
			if(cData.get(i).name.compareTo(cat) == 0)
			{
				return cData.get(i);
			}
		}
		return null;
	}
	
	public int getCategoryIndex(String category)
	{
		for(int i = 0; i < cData.size(); ++i)
		{
			if(cData.get(i).name.compareTo(category) == 0)
			{
				return i;
			}
		}
		return -1;
	}
}
