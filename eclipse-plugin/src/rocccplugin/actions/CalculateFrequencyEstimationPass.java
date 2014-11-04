package rocccplugin.actions;

import java.util.Map;
import java.util.TreeMap;

import rocccplugin.database.DatabaseInterface;

public class CalculateFrequencyEstimationPass 
{
	static int MAX_MODULE_FREQUENCY = 500;
	static int MAX_SYSTEM_FREQUENCY = 200;
	
	public static int run(String componentName)
	{
		Map<String, Integer> resourcesUsed = DatabaseInterface.getNonModuleResourcesUsed(componentName);
		String[] resourceNames = new String[0];
		resourceNames = resourcesUsed.keySet().toArray(resourceNames);
		
		Map<String, Integer> resourceMap = new TreeMap<String, Integer>();
		
		for(int i = 0; i < resourcesUsed.size(); ++i)
		{
			//Get the type of the resource, we don't care what the bit size is.
			String resourceType = resourceNames[i].split("\\s+")[0];
		
			if(resourceType.equals("MUL"))
				resourceType = "MULT";
			if(resourceType.equals("SHL") || resourceType.equals("SHR"))
				resourceType = "SHIFT";
			else if(resourceType.equals("GT") || resourceType.equals("EQ") || resourceType.equals("GTE") ||
					resourceType.equals("LT") || resourceType.equals("NE") || resourceType.equals("LTE"))
				resourceType = "COMPARE";
			else if(resourceType.equals("REGISTER"))
				resourceType = "COPY";
			
			//Add the amount of the resource type to our resource type in our map.
			if(resourceMap.containsKey(resourceType))
				resourceMap.put(resourceType, resourceMap.get(resourceType) + resourcesUsed.get(resourceNames[i]));
			else
				resourceMap.put(resourceType, resourcesUsed.get(resourceNames[i]));
		}
		
		Map<String, Integer> resourceWeights;
		resourceWeights = DatabaseInterface.getResourceWeightsForCompile(componentName);
		
		int totalWeight = 0;
		int totalResources = 0;
		int maxUsed = 0;
		
		String[] resourceTypes = new String[0];
		resourceTypes = resourceMap.keySet().toArray(resourceTypes);
		
		for(int i = 0; i < resourceTypes.length; ++i)
		{
			int numResourcesForType = resourceMap.get(resourceTypes[i]);
			totalWeight += numResourcesForType * resourceWeights.get(resourceTypes[i]);
			if(resourceWeights.get(resourceTypes[i]) > maxUsed)
				maxUsed = resourceWeights.get(resourceTypes[i]);
			totalResources += numResourcesForType;
		}
		
		String ops = DatabaseInterface.getOpsPerPipelineStageUsed(componentName);
		if(ops == null)
			return -1;
		
		float opsPerStage = Float.parseFloat(ops);
		opsPerStage = Math.min(opsPerStage, totalResources);
		
		int maxFreq = MAX_MODULE_FREQUENCY;
		if(DatabaseInterface.getComponentType(componentName).equals("SYSTEM"))
			maxFreq = MAX_SYSTEM_FREQUENCY;
		
		int estimatedFrequency = maxFreq;
			
		if(totalResources != 0)
		{
			float desiredWeightPerStage = totalWeight / totalResources * opsPerStage;
			desiredWeightPerStage = Math.max(desiredWeightPerStage, maxUsed); 
			estimatedFrequency = Math.min(maxFreq, (int)(66 / desiredWeightPerStage * 150));
		}
		
		String[] modulesCalled = DatabaseInterface.getModulesUsed(componentName);
		
		for(int i = 0; i < modulesCalled.length; ++i)
		{
			int freq = run(modulesCalled[i]);
			estimatedFrequency = Math.min(freq, estimatedFrequency);
		}
		
		return estimatedFrequency;
	}
}
