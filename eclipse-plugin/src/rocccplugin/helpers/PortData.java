package rocccplugin.helpers;

import java.util.Vector;

public class PortData 
{
	public String vhdlName;
	public String readableName;
	public String type;
	public PortDirection dir;
	public int bitSize;
	public Vector<Integer> addressesMappedTo;
	
	public PortData(String rName,  String type, String vName, PortDirection dir, int bitSize)
	{
		this.vhdlName = vName;
		this.readableName = rName;
		this.type = type;
		this.dir = dir;
		this.bitSize = bitSize;
		addressesMappedTo = new Vector<Integer>();
	}
	
	public void addAddresses(int startAddress, int numRegisters)
	{
		for(int i = 0; i < numRegisters; ++i)
			addressesMappedTo.add(startAddress + i);
	}
}
