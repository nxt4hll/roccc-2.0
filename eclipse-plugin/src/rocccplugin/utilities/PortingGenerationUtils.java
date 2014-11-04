package rocccplugin.utilities;

import java.util.Vector;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.widgets.Shell;

import rocccplugin.database.DatabaseInterface;
import rocccplugin.helpers.PortData;
import rocccplugin.helpers.PortDirection;

public class PortingGenerationUtils 
{	
	public static int getPorts(Vector<PortData> portData, String componentName)
	{
		int numPorts = 0;
		
		try 
		{ 
			//Figure out how many ports are in the component.
			numPorts = DatabaseInterface.getNumVHDLPorts(componentName);
			
			//Add all of the other ports that ROCCC creates so we can map them.
			portData.add(new PortData("clk", "REGISTER", "clk", PortDirection.IN, 1));
			portData.add(new PortData("rst", "REGISTER", "rst", PortDirection.IN, 1));
			portData.add(new PortData("inputReady", "REGISTER", "inputReady", PortDirection.IN, 1));
			portData.add(new PortData("outputReady", "REGISTER", "outputReady", PortDirection.OUT, 1));
			portData.add(new PortData("done", "REGISTER", "done", PortDirection.OUT, 1));
			portData.add(new PortData("stall", "REGISTER", "stall", PortDirection.IN, 1));
			
			String[] inputScalars = DatabaseInterface.getInputPorts(componentName);
			
			//Add the component specific ports to the list to be used later.
			for(int i = 0; i < inputScalars.length; ++i)
			{
				portData.add(new PortData(inputScalars[i], "REGISTER", DatabaseInterface.getVHDLPortName(componentName, inputScalars[i]), 
						PortDirection.IN, DatabaseInterface.getPortSize(componentName, inputScalars[i])));
			}
			
			String[] outputScalars = DatabaseInterface.getOutputPorts(componentName);
			
			//Add the component specific ports to the list to be used later.
			for(int i = 0; i < outputScalars.length; ++i)
			{
				portData.add(new PortData(outputScalars[i], "REGISTER", DatabaseInterface.getVHDLPortName(componentName, outputScalars[i]), 
										  PortDirection.OUT, DatabaseInterface.getPortSize(componentName, outputScalars[i])));
			}
			
			String[] inputStreams = DatabaseInterface.getInputStreams(componentName);
			
			for(int i = 0; i < inputStreams.length; ++i)
			{
				portData.add(new PortData("valid_" + inputStreams[i], "INPUT_STREAM_VALID", "valid_" + inputStreams[i], PortDirection.IN, 1));
				portData.add(new PortData("data_" + inputStreams[i], "INPUT_STREAM_DATA", "data_" + inputStreams[i], 
						PortDirection.IN, DatabaseInterface.getStreamBitSize(componentName, inputStreams[i])));
				portData.add(new PortData("data_addr_" + inputStreams[i], "INPUT_STREAM_ADDR", "data_addr_" + inputStreams[i], PortDirection.IN, 32));
			}
			
			String[] outputStreams = DatabaseInterface.getOutputStreams(componentName);
			
			for(int i = 0; i < outputStreams.length; ++i)
			{
				portData.add(new PortData("data_" + outputStreams[i], "OUTPUT_STREAM_DATA", "data_" + outputStreams[i], PortDirection.OUT, DatabaseInterface.getStreamBitSize(componentName, outputStreams[i])));
				portData.add(new PortData("data_addr_" + outputStreams[i], "OUTPUT_STREAM_ADDR", "data_addr_" + outputStreams[i], PortDirection.IN, 32));
			}
			
			
		} 
		catch (Exception e) { e.printStackTrace(); }
		
		return numPorts;
	}
	
	public static int assignPorts(Vector<PortData> portData, int registerSize)
	{
		int startAddress = 0;
		
		try
		{	
			//Start assigning registers to the various signals except for some special ones
			//that we will map to special locations ourself.
			for(int i = 0; i < portData.size(); ++i)
			{			
				PortData port = portData.get(i);
				
				if(port.type.compareTo("REGISTER") == 0 &&
				   port.readableName.compareTo("clk") != 0 &&
				   port.readableName.compareTo("outputReady") != 0  &&
				   port.readableName.compareTo("stall") != 0)/* &&
				   !port.readableName.equals("inputReady") &&
				   !port.readableName.equals("rst"))*/
				{	
					int numRegisters = (registerSize - 1 + port.bitSize) / registerSize;
					port.addAddresses(startAddress, numRegisters);
					
					startAddress += numRegisters;
				}
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return startAddress;
	}
}
