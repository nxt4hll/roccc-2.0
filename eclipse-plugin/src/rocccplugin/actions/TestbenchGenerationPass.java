package rocccplugin.actions;

import java.io.File;
import java.util.Map;
import java.util.Vector;

import org.eclipse.jface.window.Window;

import rocccplugin.Activator;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.helpers.GenerationFunctions;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.FileUtils;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.MessageUtils;
import rocccplugin.utilities.StringUtils;
import rocccplugin.wizards.GenerateTestbenchWizard;

public class TestbenchGenerationPass 
{
	static String componentName;
	static String tab = "  ";
	static File fileToGenerateTestbenchFor;
	static String[] inputScalarNames;
	static Vector<Vector<String>> inputScalarValues;
	static String[] outputScalars;
	static String[] outputScalarVHDLNames;
	static String[] outputScalarsPortSizes;
	static Vector<Vector<String>> outputScalarValues;
	static public String[] inputTestFiles;
	static public String[] outputTestFiles;
	static Vector<String> oneBitStreamChannels;
	static Map<String, String> clocks;
	
	static Vector<Integer> specialBurstAddressGeneratorsUsed;
	
	static int maxChannelBitwidth = 0;
	static String maxStreamMemory;
	static boolean isSystem;
	
	public static void run(File sourceFile)
	{
		maxChannelBitwidth = 0;
		fileToGenerateTestbenchFor = sourceFile;
		componentName = DatabaseInterface.getComponentFromSourceFile(sourceFile);
		
		oneBitStreamChannels = new Vector<String>();
		
		MessageUtils.printlnConsoleMessage("Beginning testbench generation for " + fileToGenerateTestbenchFor.getName() + "...");
		if(getTestBenchInfo() == false)
		{
			DatabaseInterface.closeConnection();
			MessageUtils.printlnConsoleError("Testbench Generation Canceled.\n");
			GuiLockingUtils.unlockGui();
			return;
		}
		
		//Generate the testbench file
		generateTestbenchFile(fileToGenerateTestbenchFor);

		if(DatabaseInterface.getComponentType(DatabaseInterface.getComponentFromSourceFile(fileToGenerateTestbenchFor)).equals("SYSTEM"))
			FileUtils.copyFile(new File(Activator.getDistributionFolder() + "/SupplementaryVHDL/StreamHandler.vhdl"), fileToGenerateTestbenchFor.getAbsolutePath().replace(fileToGenerateTestbenchFor.getName(), "vhdl/StreamHandler.vhdl"));
		
		EclipseResourceUtils.updateProjectFiles();
		
		MessageUtils.printlnConsoleMessage("Writing testbench to '" + FileUtils.getFolderOfFile(sourceFile) + "vhdl/" + DatabaseInterface.getComponentFromSourceFile(sourceFile) + "_testbench.vhdl'");
		MessageUtils.printlnConsoleSuccess("Testbench generation complete.\n");
		
		DatabaseInterface.closeConnection();
		GuiLockingUtils.unlockGui();
		return;
	}

	static private void saveTestBenchInfo(File fileToGenerateTestbenchFor)
	{
		StringBuffer buffer = new StringBuffer();
	
		if(isSystem)
		{
			String[] inputStreams = DatabaseInterface.getInputStreams(componentName);
			
			for(int i = 0; i < inputStreams.length; ++i)
			{
				buffer.append("STREAM_DATA " + inputStreams[i] + " " + inputTestFiles[i] + "\n");
			}
			
			buffer.append("\n");
			
			String[] outputStreams = DatabaseInterface.getOutputStreams(componentName);
			
			for(int i = 0; i < outputStreams.length; ++i)
			{
				buffer.append("STREAM_DATA " + outputStreams[i] + " " + outputTestFiles[i] + "\n");
			}
			
			buffer.append("\n");
			
			buffer.append("CLOCK clk " + clocks.get("clk") + "\n\n");
			
			String[] clockNames = DatabaseInterface.getSystemClocks(componentName);
			
			for(int i = 0; i < clockNames.length; ++i)
			{
				buffer.append("CLOCK " + clockNames[i] + " " + clocks.get(clockNames[i]) + "\n");
			}
			
			buffer.append("\n");
		}
		
		for(int i = 0; i < inputScalarNames.length; ++i)
		{
			buffer.append("SCALAR_DATA " + inputScalarNames[i] + " " + inputScalarValues.get(0).size());
			for(int j = 0; j < inputScalarValues.get(i).size(); ++j)
				buffer.append(" " + inputScalarValues.get(i).get(j));
			buffer.append("\n");
		}
		
		for(int i = 0; i < outputScalars.length; ++i)
		{
			buffer.append("SCALAR_DATA " + outputScalars[i] + " " + outputScalarValues.get(0).size());
			for(int j = 0; j < outputScalarValues.get(i).size(); ++j)
				buffer.append(" " + outputScalarValues.get(i).get(j));
			buffer.append("\n");
		}
		
		FileUtils.createFileFromBuffer(buffer, new File(fileToGenerateTestbenchFor.getAbsolutePath().replace(fileToGenerateTestbenchFor.getName(), ".ROCCC/.testbenchInfo")));
	}
	
	static private boolean getTestBenchInfo()
	{
		if(DatabaseInterface.openConnection() == false)
			return false;
		
		
		isSystem = DatabaseInterface.getComponentType(componentName).equals("SYSTEM");
		
		GenerateTestbenchWizard wizard = new GenerateTestbenchWizard(componentName, fileToGenerateTestbenchFor, isSystem);
		
		if(EclipseResourceUtils.openWizard(wizard) == Window.CANCEL)
			return false;
		
		inputScalarNames = wizard.scalarNames;
		inputScalarValues = wizard.scalarValues;
		outputScalars = wizard.outputScalars;
		outputScalarValues = wizard.outputScalarValues;
		clocks = wizard.clocks;
		
		if(isSystem)
		{
			inputTestFiles = wizard.getRelativeInputStreamTestFiles();
			outputTestFiles = wizard.getRelativeOutputStreamTestFiles();
		}
		
		saveTestBenchInfo(fileToGenerateTestbenchFor);
		
		if(isSystem)
		{
			inputTestFiles = wizard.inputTestFiles;
			outputTestFiles = wizard.outputTestFiles;
		}
		return true;
	}
	
	
	static private void generateTestbenchFile(File fileToGenerateTestbenchFor)
	{	
		String fileFolder = fileToGenerateTestbenchFor.getAbsolutePath().replace(fileToGenerateTestbenchFor.getName(), "");
		specialBurstAddressGeneratorsUsed = new Vector<Integer>();
		
		try
		{	
			if(isSystem)
			{
				GenerationFunctions.determineNecessaryBurstAddressGenerators(componentName, specialBurstAddressGeneratorsUsed);
				
				StringBuffer additionalBuffer = new StringBuffer();
				GenerationFunctions.generateSpecializedBurstAddressGenerators(additionalBuffer, specialBurstAddressGeneratorsUsed);
				GenerationFunctions.generateSynchInterfaceGeneric(additionalBuffer, 32);
				
				FileUtils.createFileFromBuffer(additionalBuffer, new File(FileUtils.getFolderOfFile(fileToGenerateTestbenchFor) + "vhdl/testbench_burstaddress_additions.vhdl"));
			}
			
			StringBuffer buffer = new StringBuffer();
			generateHeader(buffer);
			generateComponent(buffer);
			generateBurstAddressComponent(buffer);
			generateSignals(buffer);
			generateStreamHandler(buffer);
			generateStreamMappings(buffer);
			generateUUTPortMap(buffer);
			if(isSystem)
			{
				generateOutputStreamExpectedValues(buffer);
				generateSynchInterfacePortMaps(buffer, componentName);
			}
			generateClockProcesses(buffer);
			
			FileUtils.createFileFromBuffer(buffer, new File(FileUtils.getFolderOfFile(fileToGenerateTestbenchFor) + "vhdl/" + componentName + "_testbench.vhdl"));
			EclipseResourceUtils.updateProjectFiles();
			
			//This will open the file up in the Eclipse editor.
			//EclipseResourceUtils.openFileInEditor(new File(FileUtils.getFolderOfFile(fileToGenerateTestbenchFor) + "vhdl/" + componentName + "_testbench.vhdl"));
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	static private void generateHeader(StringBuffer buffer)
	{
		//Figure out the max channel bitwidth
		if(isSystem)
		{
			String[] outputStreams = DatabaseInterface.getOutputStreams(componentName);
			
			for(int i = 0; i < outputStreams.length; ++i)
			{
				int channelWidth = DatabaseInterface.getStreamPortSize(componentName, outputStreams[i], DatabaseInterface.getStreamChannels(componentName, outputStreams[i])[0]);
				if(channelWidth > maxChannelBitwidth)
					maxChannelBitwidth = channelWidth;
			}
		}

		if(isSystem)
		{
			buffer.append("LIBRARY ieee;\n");
			buffer.append("USE ieee.std_logic_1164.ALL;\n");
			buffer.append("USE ieee.numeric_std.ALL;\n\n");
			
			buffer.append("package output_memory_types is\n");
			buffer.append("constant MAX_CHANNEL_BITWIDTH : integer := " + maxChannelBitwidth + ";\n");
			buffer.append("type t_2D_output_memory_type is array (natural range<>, natural range<>) of std_logic_vector(MAX_CHANNEL_BITWIDTH - 1 downto 0);\n");
			buffer.append("type t_1D_output_memory_type is array (natural range<>) of std_logic_vector(MAX_CHANNEL_BITWIDTH - 1 downto 0);\n");
			buffer.append("end package;\n\n");
		}
		
		buffer.append("LIBRARY ieee;\n");
		buffer.append("USE ieee.std_logic_1164.ALL;\n");
		buffer.append("USE ieee.std_logic_unsigned.all;\n");
		if(isSystem)
			buffer.append("USE work.output_memory_types.ALL;\n");
		buffer.append("\n");
	}
	
	static private void generateComponent(StringBuffer buffer)
	{
		buffer.append("ENTITY " + componentName + "_tb IS\n");
		buffer.append("END " + componentName + "_tb;\n\n");
		buffer.append("ARCHITECTURE behavior OF " + componentName + "_tb IS\n\n");
		
		buffer.append(tab + "COMPONENT " + componentName + "\n");
		buffer.append(tab + "port (\n");
		buffer.append(tab + tab + "clk : in STD_LOGIC;\n");
		buffer.append(tab + tab + "rst : in STD_LOGIC;\n");
		buffer.append(tab + tab + "inputReady : in STD_LOGIC;\n");
		buffer.append(tab + tab + "outputReady : out STD_LOGIC;\n");
		buffer.append(tab + tab + "done : out STD_LOGIC;\n");
		buffer.append(tab + tab + "stall : in STD_LOGIC;\n");
		
		int count = DatabaseInterface.getTotalPorts(componentName);
		for(int i = 0; i < count; ++i)
		{
			String vhdlName = DatabaseInterface.getVHDLNameFromOrder(componentName, i);
			String dir = DatabaseInterface.getPortDirectionFromVHDLName(componentName, vhdlName);
			buffer.append(tab + tab + vhdlName + " : " + (dir.equals("OUT")? "out" : "in") + " STD_LOGIC");
			int bitSize  = DatabaseInterface.getPortSizeFromVHDLName(componentName, vhdlName);
			if(bitSize > 1)
				buffer.append("_VECTOR(" + (bitSize - 1) + " downto 0)");
			if(i != count - 1)
				buffer.append(";");
			buffer.append("\n");
		}
		
		buffer.append(tab + tab + ");\n");
		buffer.append(tab + "END COMPONENT;\n\n");
	}
	
	static private void generateBurstAddressComponent(StringBuffer buffer)
	{
		for(int i = 0; i < specialBurstAddressGeneratorsUsed.size(); ++i)
		{
			buffer.append("component SpecialBurstAddressGen" + specialBurstAddressGeneratorsUsed.get(i) + "Channels is\n");
			buffer.append(tab + "port(\n");
			buffer.append(tab + tab + "clk : in STD_LOGIC;\n");
			buffer.append(tab + tab + "rst : in STD_LOGIC;\n\n");
			
			buffer.append(tab + tab + "burst_address_in : in STD_LOGIC_VECTOR(31 downto 0);\n");
			buffer.append(tab + tab + "burst_count_in : in STD_LOGIC_VECTOR(31 downto 0);\n");
			buffer.append(tab + tab + "burst_valid_in : in STD_LOGIC;\n");
			buffer.append(tab + tab + "burst_stall_out : out STD_LOGIC;\n\n");
			
			buffer.append(tab + tab + "address_valid_out : out STD_LOGIC;\n");
			
			for(int j = 0; j < specialBurstAddressGeneratorsUsed.get(i); ++j)
			{
				buffer.append(tab + tab + "address_stall_channel" + j + "_in : in STD_LOGIC;\n");
			}
			
			for(int j = 0; j < specialBurstAddressGeneratorsUsed.get(i); ++j)
			{
				if(j > 0)
					buffer.append(";");
				buffer.append("\n");
				buffer.append(tab + tab + "address_channel" + j + "_out : out STD_LOGIC_VECTOR(31 downto 0)");
			}
			
			buffer.append("\n");
			buffer.append(tab + tab + ");\n");
			buffer.append("end component;\n\n");
		}
		
	}
	
	static private void generateSignals(StringBuffer buffer)
	{
		generateInputSignals(buffer);
		generateOutputSignals(buffer);
	}
	
	static private void generateInputSignals(StringBuffer buffer)
	{
		buffer.append(tab + "-- Input Signals\n");
		buffer.append(tab + "signal clk : STD_LOGIC := '0';\n");
		buffer.append(tab + "signal rst : STD_LOGIC := '1';\n");
		buffer.append(tab + "signal inputReady : STD_LOGIC := '0';\n");
		buffer.append(tab + "signal stall : STD_LOGIC := '0';\n");
		
		String[] streams = DatabaseInterface.getStreams(componentName);
		for(int i = 0; i < streams.length; ++i)
		{
			String[] streamPortNames = DatabaseInterface.getStreamPorts(componentName, streams[i]);
			for(int j = 0; j < streamPortNames.length; ++j)
			{
				String direction = DatabaseInterface.getStreamPortDirection(componentName, streams[i], streamPortNames[j]).equals("IN")? "in" : "out";
				if(direction.equals("out"))
					continue;
				buffer.append(tab + "signal " + streamPortNames[j] + " : STD_LOGIC");
				int size = DatabaseInterface.getStreamPortSize(componentName, streams[i], streamPortNames[j]);
				
				boolean isChannel = DatabaseInterface.getStreamPortType(componentName, streams[i], streamPortNames[j]).equals("STREAM_CHANNEL");
				
				if(size > 1 || isChannel)
				{
					if(size == 1 && isChannel)
						oneBitStreamChannels.add(streamPortNames[j]);
					buffer.append("_VECTOR(" + (size - 1) + " downto 0) := (others => '0');\n");
				}
				else
				{
					buffer.append(" := '0';\n");
				}
			}
		}
		
		int numPorts = DatabaseInterface.getNumPorts(componentName);
		for(int i = 0; i < numPorts; ++i)
		{
			String VHDLPortName = DatabaseInterface.getVHDLPortName(componentName, i);
			String portName = DatabaseInterface.getPortName(componentName, i);
			String direction = DatabaseInterface.getPortDirection(componentName, portName).equals("IN")? "in" : "out";
			if(direction.equals("out"))
				continue;
			
			buffer.append(tab + "signal " + VHDLPortName + " : STD_LOGIC");
			
			int bitSize = DatabaseInterface.getPortSize(componentName, portName);
			if(bitSize > 1)
			{
				buffer.append("_VECTOR(" + (bitSize - 1) + " downto 0) := (others => '0');\n");
			}
			else
			{
				buffer.append(" := '0';\n");
			}
		}
		
		buffer.append("\n");
	}
	
	static private void generateOutputSignals(StringBuffer buffer)
	{
		buffer.append(tab + "-- Output Signals\n");
		buffer.append(tab + "signal outputReady : STD_LOGIC;\n");
		buffer.append(tab + "signal done : STD_LOGIC;\n");
			
		String[] outputStreams = DatabaseInterface.getOutputStreams(componentName);
		for(int i = 0; i < outputStreams.length; ++i)
		{
			buffer.append(tab + "signal " + outputStreams[i] + "_done : STD_LOGIC;\n");
		}
		
		String[] streams = DatabaseInterface.getStreams(componentName);
		for(int i = 0; i < streams.length; ++i)
		{
			String[] streamPortNames = DatabaseInterface.getStreamPorts(componentName, streams[i]);
			for(int j = 0; j < streamPortNames.length; ++j)
			{
				String direction = DatabaseInterface.getStreamPortDirection(componentName, streams[i], streamPortNames[j]).equals("IN")? "in" : "out";
				if(direction.equals("in"))
					continue;
				buffer.append(tab + "signal " + streamPortNames[j] + " : STD_LOGIC");
				int size = DatabaseInterface.getStreamPortSize(componentName, streams[i], streamPortNames[j]);

				boolean isChannel = DatabaseInterface.getStreamPortType(componentName, streams[i], streamPortNames[j]).equals("STREAM_CHANNEL");
				
				if(size > 1 || isChannel)
				{
					if(size == 1 && isChannel)
						oneBitStreamChannels.add(streamPortNames[j]);
					buffer.append("_VECTOR(" + (size - 1) + " downto 0)");
				}
				
				buffer.append(";\n");
			}
		}
		
		int numPorts = DatabaseInterface.getNumPorts(componentName);
		for(int i = 0; i < numPorts; ++i)
		{
			String portName = DatabaseInterface.getPortName(componentName, i);
			String VHDLPortName = DatabaseInterface.getVHDLPortName(componentName, i);
			String direction = DatabaseInterface.getPortDirection(componentName, portName).equals("IN")? "in" : "out";
			if(direction.equals("in"))
				continue;
			
			buffer.append(tab + "signal " + VHDLPortName + " : STD_LOGIC");
			
			int bitSize = DatabaseInterface.getPortSize(componentName, portName);
			if(bitSize > 1)
			{
				buffer.append("_VECTOR(" + (bitSize - 1) + " downto 0)");
			}
			
			buffer.append(";\n");
		}
		
		String[] debugPorts = DatabaseInterface.getDebugPorts(componentName);
		for(int i = 0; i < debugPorts.length; ++i)
		{
			buffer.append(tab + "signal " + debugPorts[i] + " : STD_LOGIC");
			
			int bitSize = DatabaseInterface.getDebugPortSize(componentName, debugPorts[i]);
			if(bitSize > 1)
			{
				buffer.append("_VECTOR(" + (bitSize - 1) + " downto 0)");
			}
			
			buffer.append(";\n");
		}
		
		buffer.append("\n");
	}
	
	static private void generateStreamHandler(StringBuffer buffer)
	{
		try
		{			
			buffer.append(tab + "-- Clock period definitions\n");
		
			if(clocks == null || clocks.size() == 0)
				buffer.append(tab + "constant clk_period : time := 1ns;\n");
			else
				buffer.append(tab + "constant clk_period : time := " + clocks.get("clk") + "ns;\n");
			
			if(isSystem)
			{
				String[] clockNames = DatabaseInterface.getSystemClocks(componentName);
				
				for(int i = 0; i < clockNames.length; ++i)
				{					
					buffer.append(tab + "constant " + clockNames[i] + "_clk_period : time := " + clocks.get(clockNames[i]) + "ns; -- " + clockNames[i] + " Clock Period Definition\n");
				}
			}
			
			buffer.append("\n");
			
			if(isSystem)
			{
				buffer.append(tab + "component InputStream is\n");
				buffer.append(tab + tab + "generic (\n");
				buffer.append(tab + tab + tab + "CHANNEL_BITWIDTH: integer;\n");
				buffer.append(tab + tab + tab + "NUM_CHANNELS : integer;\n");
				buffer.append(tab + tab + tab + "CONCURRENT_MEM_ACCESS : integer;\n");
				buffer.append(tab + tab + tab + "NUM_MEMORY_ELEMENTS : integer;\n");
				buffer.append(tab + tab + tab + "STREAM_NAME : string\n");
				buffer.append(tab + tab + tab + ");\n");
				buffer.append(tab + tab + "port (\n");
				buffer.append(tab + tab + tab + "clk : in STD_LOGIC;\n");
				buffer.append(tab + tab + tab + "rst : in STD_LOGIC;\n");
				
				buffer.append(tab + tab + tab + "full_in : in STD_LOGIC;\n");
				buffer.append(tab + tab + tab + "write_enable_out : out STD_LOGIC;\n");
				buffer.append(tab + tab + tab + "channel_out : out STD_LOGIC_VECTOR(NUM_CHANNELS * CHANNEL_BITWIDTH - 1 downto 0);\n");
				buffer.append(tab + tab + tab + "address_in : in STD_LOGIC_VECTOR(NUM_CHANNELS * 32 - 1 downto 0);\n");
				buffer.append(tab + tab + tab + "read_in : in STD_LOGIC;\n");
				buffer.append(tab + tab + tab + "memory : in STD_LOGIC_VECTOR(NUM_MEMORY_ELEMENTS * CHANNEL_BITWIDTH - 1 downto 0)\n");
				
				buffer.append(tab + tab + tab + ");\n");
				buffer.append(tab + tab + "end component;\n\n");
				
				//Output Stream component
				buffer.append(tab + "component OutputStream is\n");
				buffer.append(tab + tab + "generic (\n");
				buffer.append(tab + tab + tab + "CHANNEL_BITWIDTH: integer;\n");
				buffer.append(tab + tab + tab + "NUM_MEMORY_ELEMENTS: integer;\n");
				buffer.append(tab + tab + tab + "NUM_CHANNELS : integer;\n");
				buffer.append(tab + tab + tab + "STREAM_NAME: string\n");
				buffer.append(tab + tab + tab + ");\n");
				buffer.append(tab + tab + "port (\n");
				buffer.append(tab + tab + tab + "clk : in STD_LOGIC;\n");
				buffer.append(tab + tab + tab + "rst : in STD_LOGIC;\n");
				buffer.append(tab + tab + tab + "done_in : in STD_LOGIC;\n");
				
				buffer.append(tab + tab + tab + "empty_in : in STD_LOGIC;\n");
				buffer.append(tab + tab + tab + "read_enable_out : out STD_LOGIC;\n");
				buffer.append(tab + tab + tab + "channel_in : in STD_LOGIC_VECTOR((NUM_CHANNELS * CHANNEL_BITWIDTH) - 1 downto 0);\n");
				buffer.append(tab + tab + tab + "address_in : in STD_LOGIC_VECTOR(NUM_CHANNELS * 32 - 1 downto 0);\n");
				buffer.append(tab + tab + tab + "read_in : in STD_LOGIC;\n");
				buffer.append(tab + tab + tab + "OUTPUT_CORRECT : in t_1D_output_memory_type(0 to NUM_MEMORY_ELEMENTS - 1)\n");
				
				buffer.append(tab + tab + tab + ");\n");
				buffer.append(tab + tab + "end component;\n\n");
				
				//MicroFifo component
				buffer.append(tab + "component MicroFifo is\n");
				buffer.append(tab + "generic (\n");
				buffer.append(tab + tab + "ADDRESS_WIDTH : POSITIVE;\n");
				buffer.append(tab + tab + "DATA_WIDTH : POSITIVE;\n");
				buffer.append(tab + tab + "ALMOST_FULL_COUNT : NATURAL;\n");
				buffer.append(tab + tab + "ALMOST_EMPTY_COUNT : NATURAL\n");
				buffer.append(tab + ");\n");
				buffer.append(tab + "port(\n");
				buffer.append(tab + tab + "clk : in STD_LOGIC;\n");
				buffer.append(tab + tab + "rst : in STD_LOGIC;\n");
				buffer.append(tab + tab + "data_in : in STD_LOGIC_VECTOR(DATA_WIDTH-1 downto 0);\n");
				buffer.append(tab + tab + "valid_in : in STD_LOGIC;\n");
				buffer.append(tab + tab + "full_out : out STD_LOGIC;\n");
				buffer.append(tab + tab + "data_out : out STD_LOGIC_VECTOR(DATA_WIDTH-1 downto 0);\n");
				buffer.append(tab + tab + "read_enable_in : in STD_LOGIC;\n");
				buffer.append(tab + tab + "empty_out : out STD_LOGIC\n");
				buffer.append(tab + ");\n");
				buffer.append(tab + "end component;\n\n");
			}
			
			buffer.append(tab + tab + "function TO_STRING(VALUE : STD_LOGIC_VECTOR) return STRING is\n");
			buffer.append(tab + tab + tab + "alias ivalue : STD_LOGIC_VECTOR(1 to value'length) is value;\n");
			buffer.append(tab + tab + tab + "variable result : STRING(1 to value'length);\n");
			buffer.append(tab + tab + "begin\n");
			buffer.append(tab + tab + tab + "if value'length < 1 then\n");
			buffer.append(tab + tab + tab + tab + "return \"\";\n");
			buffer.append(tab + tab + tab + "else\n");
			buffer.append(tab + tab + tab + tab + "for i in ivalue'range loop\n");
			buffer.append(tab + tab + tab + tab + tab + "if ivalue(i) = '0' then\n");
			buffer.append(tab + tab + tab + tab + tab + tab + "result(i) := '0';\n");
			buffer.append(tab + tab + tab + tab + tab + "else\n");
			buffer.append(tab + tab + tab + tab + tab + tab + "result(i) := '1';\n");
			buffer.append(tab + tab + tab + tab + tab + "end if;\n");
			buffer.append(tab + tab + tab + tab + "end loop;\n");
			buffer.append(tab + tab + tab + tab + "return result;\n");
			buffer.append(tab + tab + tab + "end if;\n");
			buffer.append(tab + tab + "end function to_string;\n\n");
			
			
			buffer.append(tab + tab + "function TO_STRING(VALUE : STD_LOGIC) return STRING is\n");
			buffer.append(tab + tab + "begin\n");
			buffer.append(tab + tab + tab + "if( VALUE = '0') then\n");
			buffer.append(tab + tab + tab + tab + "return \"0\";\n");
			buffer.append(tab + tab + tab + "elsif( VALUE = '1') then\n");
			buffer.append(tab + tab + tab + tab + "return \"1\";\n");
			buffer.append(tab + tab + tab + "elsif( VALUE = 'X' ) then\n");
			buffer.append(tab + tab + tab + tab + "return \"X\";\n");
			buffer.append(tab + tab + tab + "elsif( VALUE = 'U' ) then\n");
			buffer.append(tab + tab + tab + tab + "return \"U\";\n");
			buffer.append(tab + tab + tab + "else\n");
			buffer.append(tab + tab + tab + tab + "return \"N\";\n");
			buffer.append(tab + tab + tab + "end if;\n");
			buffer.append(tab + tab + "end function TO_STRING;\n\n");
			
			buffer.append(tab + tab + "component SynchInterfaceGeneric is\n");
			buffer.append(tab + tab + "generic(\n");
			buffer.append(tab + tab + tab + "INPUT_DATA_WIDTH : INTEGER;\n");
			buffer.append(tab + tab + tab + "OUTPUT_DATA_WIDTH : INTEGER\n");
			buffer.append(tab + tab + ");\n");
			buffer.append(tab + tab + "port (\n");
			buffer.append(tab + tab + tab + "clk : in STD_LOGIC;\n");
			buffer.append(tab + tab + tab + "rst : in STD_LOGIC;\n");
			buffer.append(tab + tab + tab + "data_in : in STD_LOGIC_VECTOR(INPUT_DATA_WIDTH - 1 downto 0);\n");
			buffer.append(tab + tab + tab + "data_empty : in STD_LOGIC;\n");
			buffer.append(tab + tab + tab + "data_read : out STD_LOGIC;\n");
			buffer.append(tab + tab + tab + "address_in : in STD_LOGIC_VECTOR(31 downto 0);\n");
			buffer.append(tab + tab + tab + "address_valid : in STD_LOGIC;\n");
			buffer.append(tab + tab + tab + "stallAddress : out STD_LOGIC;\n");
			buffer.append(tab + tab + tab + "addressPop : out STD_LOGIC;\n");
			buffer.append(tab + tab + tab + "mc_stall_in : in STD_LOGIC;\n");
			buffer.append(tab + tab + tab + "mc_vadr_out : out STD_LOGIC_VECTOR(31 downto 0);\n");
			buffer.append(tab + tab + tab + "mc_valid_out : out STD_LOGIC;\n");
			buffer.append(tab + tab + tab + "mc_data_out : out STD_LOGIC_VECTOR(OUTPUT_DATA_WIDTH - 1 downto 0)\n");
			buffer.append(tab + tab + ");\n");
			buffer.append(tab + tab + "end component;\n\n");
				
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	static private void generateStreamMappings(StringBuffer buffer)
	{
		try
		{
			String[] streams = DatabaseInterface.getInputStreams(componentName);
			
			if(streams.length > 0)
				buffer.append(tab + "-- Stream Packing\n");
			
			for(int i = 0; i < streams.length; ++i)
			{
				int numChannels = DatabaseInterface.getNumStreamChannels(componentName, streams[i]);
				if(numChannels == 0)
					continue;
			
				buffer.append(tab + "constant " + streams[i] + "_stream_NUM_CHANNELS : integer := " + numChannels + ";\n");
				
				String[] streamChannels = DatabaseInterface.getStreamChannels(componentName, streams[i]);
			
				String[] addressBases = DatabaseInterface.getStreamPortsOfType(componentName, streams[i], "STREAM_ADDRESS_BASE");
				
				buffer.append(tab + "signal " + streams[i] + "_stream_channel_out : STD_LOGIC_VECTOR(");
				buffer.append(streams[i] + "_stream_NUM_CHANNELS * " + streamChannels[0] + "'length - 1 downto 0);\n");
				buffer.append(tab + "signal " + streams[i] + "_stream_address_in : STD_LOGIC_VECTOR(");
				buffer.append(streams[i] + "_stream_NUM_CHANNELS * " + "32 - 1 downto 0);\n\n");
				buffer.append(tab + "signal " + streams[i] + "_address_translator_address_valid_out : STD_LOGIC;\n\n");
				
				
			}
			
			String[] outputStreams = DatabaseInterface.getOutputStreams(componentName);
			for(int i = 0; i < outputStreams.length; ++i)
			{
				String[] streamChannels = DatabaseInterface.getStreamChannels(componentName, outputStreams[i]);
				
				buffer.append(tab + "constant " + outputStreams[i] + "_stream_NUM_CHANNELS : integer := " + DatabaseInterface.getNumStreamChannels(componentName, outputStreams[i]) + ";\n");
				
				buffer.append(tab + "signal " + outputStreams[i] + "_stream_channel_out : STD_LOGIC_VECTOR(");
				buffer.append(outputStreams[i] + "_stream_NUM_CHANNELS * " + streamChannels[0] + "'length - 1 downto 0);\n");
				
				//buffer.append(tab + "signal " + outputStreams[i] + "_stream_address_out : STD_LOGIC_VECTOR(");
				//buffer.append(outputStreams[i] + "_stream_NUM_CHANNELS * 32 - 1 downto 0);\n\n");
				
				buffer.append(tab + "signal " + outputStreams[i] + "_address_translator_address_valid_out : STD_LOGIC;\n\n");
				
				for(int j = 0; j < streamChannels.length; ++j)
				{
					buffer.append(tab + "signal " + outputStreams[i] + "_" + j + "_stream_read_enable_out : STD_LOGIC;\n");
					buffer.append(tab + "signal " + outputStreams[i] + "_" + j + "_stream_data_valid : STD_LOGIC;\n");
					buffer.append(tab + "signal " + outputStreams[i] + "_" + j + "_address_translator_address_stall_in : STD_LOGIC;\n");
					buffer.append(tab + "signal " + outputStreams[i] + "_" + j + "_address_translator_address_out : STD_LOGIC_VECTOR(31 downto 0);\n");
					buffer.append(tab + "signal " + outputStreams[i] + "_" + j + "_address_fifo_data_out : STD_LOGIC_VECTOR(31 downto 0);\n");
					buffer.append(tab + "signal " + outputStreams[i] + "_" + j + "_address_fifo_read_enable_in : STD_LOGIC;\n");
					buffer.append(tab + "signal " + outputStreams[i] + "_" + j + "_address_fifo_empty_out : STD_LOGIC;\n\n");
				}
				
				int channelSize = DatabaseInterface.getStreamPortSize(componentName, outputStreams[i], streamChannels[0]);
				int numChannels = DatabaseInterface.getNumStreamChannels(componentName, outputStreams[i]);
				
				for(int j = 0; j < numChannels; ++j)
			    {
					buffer.append(tab + "signal " + outputStreams[i] + "_" + j + "_address_side_valid : std_logic;\n");
					buffer.append(tab + "signal " + outputStreams[i] + "_" + j + "_address_fifo_data_out_2 : std_logic_vector(31 downto 0);\n");
					buffer.append(tab + "signal " + outputStreams[i] + "_" + j + "_mc_valid_out : std_logic;\n");
					buffer.append(tab + "signal " + outputStreams[i] + "_" + j + "_mc_data_out : STD_LOGIC_VECTOR(" + (channelSize - 1) + " downto 0);\n\n");
				}
			}
			
			buffer.append(tab + "BEGIN\n\n");
			
			for(int i = 0; i < streams.length; ++i)
			{
				int numChannels = DatabaseInterface.getNumStreamChannels(componentName, streams[i]);
				if(numChannels == 0)
					continue;
							
				String[] streamChannels = DatabaseInterface.getStreamChannels(componentName, streams[i]);
				
				//Start input file getting
				String inputTestFile = inputTestFiles[i];
				Vector<String> fileContents = new Vector<String>();
				
				//Reverse the input values
				
				StringBuffer unreversedValues = new StringBuffer();
				FileUtils.addFileContentsToBuffer(unreversedValues, inputTestFile);
				
				String value = StringUtils.getNextStringValue(unreversedValues);
				
				while(!value.equals(""))
				{
					fileContents.add(value);
					value = StringUtils.getNextStringValue(unreversedValues);
				}
				
				StringBuffer inputMemory = new StringBuffer();
				
				int channelSize = DatabaseInterface.getStreamPortSize(componentName, streams[i], streamChannels[0]);
				
				int numElements = 0;
				for(int j = fileContents.size() - 1; j >= 0; --j)
				{
					numElements++;
					if(j != fileContents.size() - 1)
						inputMemory.append(",\n");
					inputMemory.append(tab + tab + tab + "memory(" + (((j + 1) * channelSize) - 1) + " downto " + (j * channelSize) + ") => \"" + StringUtils.numberToBinary(fileContents.get(j), channelSize, streams[i]) + "\"");
				}
				//End input file getting.
				
				String[] nonChannels = DatabaseInterface.getNonChannelStreamPorts(componentName, streams[i]);
				
				buffer.append(tab + tab + streams[i] + "_stream : InputStream generic map (\n");
				buffer.append(tab + tab + tab + "CHANNEL_BITWIDTH => " + streamChannels[0] + "'length,\n");
				buffer.append(tab + tab + tab + "NUM_CHANNELS => " + streams[i] + "_stream_NUM_CHANNELS,\n");
				buffer.append(tab + tab + tab + "CONCURRENT_MEM_ACCESS => " + getConcurrentMemoryAccesses(streams[i]) + ",\n");
				buffer.append(tab + tab + tab + "NUM_MEMORY_ELEMENTS => " + numElements + ",\n");
				buffer.append(tab + tab + tab + "STREAM_NAME => \"" + streams[i] + "\"\n");
				buffer.append(tab + tab + tab + ")\n");
				buffer.append(tab + tab + "port map (\n");
				buffer.append(tab + tab + tab + "clk => " + nonChannels[0] + ",\n");
				buffer.append(tab + tab + tab + "rst =>rst,\n");
				
				buffer.append(tab + tab + tab + "full_in => " + nonChannels[1] + ",\n");
				buffer.append(tab + tab + tab + "write_enable_out => " + nonChannels[4] + ",\n");
				buffer.append(tab + tab + tab + "channel_out => " + streams[i] + "_stream_channel_out,\n");
				buffer.append(tab + tab + tab + "address_in => " + streams[i] + "_stream_address_in" + ",\n");
				buffer.append(tab + tab + tab + "read_in => " + streams[i] + "_address_translator_address_valid_out,\n");
				//buffer.append(tab + tab + tab + "read_in => " + nonChannels[3] + ",\n");
				//buffer.append(tab + tab + tab + "memory => \"");
				
				buffer.append(inputMemory);
				
				buffer.append("\n");
				buffer.append(tab + tab + tab + ");\n\n");
				
				for(int j = numChannels - 1; j >= 0; --j)
				{
					String streamChannel = streamChannels[j];
				
					buffer.append(tab + tab + streamChannel + " <= " + streams[i] + "_" +
							"stream_channel_out(" + (j + 1) + " * ");
					buffer.append(streamChannel + "'length - 1 downto " + j + " * " + streamChannel + "'length);\n");
				}
				
				
				// Original code
				/*
				String[] streamAddresses = DatabaseInterface.getStreamAddresses(componentName, streams[i]);
				
				
				for(int j = streamAddresses.length - 1; j >= 0; --j)
				{
					String streamAddress = streamAddresses[j];
					buffer.append(tab + tab + streams[i] + "_stream_address_in(" + (j + 1) + " * " + streamAddress + "'length - 1 downto " + j + " * " + streamAddress + "'length) <= " + streamAddress + ";\n");
				}
				*/
				/*
				String[] channelPorts = DatabaseInterface.getStreamChannelPorts(componentName, outputStreams[i]);
				
				GenerationFunctions.portMapSpecializedBurstAddressGenerator(buffer, outputStreams[i] + "_address_translator_output_" + i, componentName, outputStreams[i], false);
				
				*/
				buffer.append("\n");
				
				// Added by Jason
				GenerationFunctions.portMapSpecializedBurstAddressGenerator(buffer, streams[i] + "_address_translator" + i, componentName, streams[i], true);
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
	}
	
	static private void generateInputScalarValues(StringBuffer buffer, int setNumber)
	{		
		for(int i = 0; i < inputScalarNames.length; ++i)
		{	
			buffer.append(tab + tab + tab + DatabaseInterface.getVHDLPortName(componentName, inputScalarNames[i]));
			buffer.append(" <= ");
			
			int bitSize = DatabaseInterface.getPortSize(componentName, inputScalarNames[i]);
			
			String binary = StringUtils.numberToBinary(inputScalarValues.get(i).get(setNumber), bitSize, DatabaseInterface.getPortDataType(componentName, inputScalarNames[i]));
			
			if(binary.length() == 1)
				buffer.append("'" + binary + "'; -- " + inputScalarValues.get(i).get(setNumber));
			else
				buffer.append("\"" + binary + "\"; --  " + inputScalarValues.get(i).get(setNumber));
			
			buffer.append("\n");
		}
		buffer.append("\n");
	}
	
	static private int getConcurrentMemoryAccesses(String streamName)
	{
		StringBuffer buffer = new StringBuffer();
		String fileName = fileToGenerateTestbenchFor.getName();
		FileUtils.addFileContentsToBuffer(buffer, fileToGenerateTestbenchFor.getAbsolutePath().replace(fileName, ".ROCCC/.streamInfo"));
		
		boolean isSystolicArray = isSystemSystolicArray();
		
		while(buffer.length() > 0)
		{
			String type = StringUtils.getNextStringValue(buffer);
			if(type.equals("INPUT"))
			{
				String stream = StringUtils.getNextStringValue(buffer);
				StringUtils.getNextStringValue(buffer);
				int numMemoryAccesses = Integer.parseInt(StringUtils.getNextStringValue(buffer));
				if(stream.equals(streamName.replace((isSystolicArray? "_input" : ""), "")))
				{
					return numMemoryAccesses;
				}
			}
			else if(type.equals("OUTPUT"))
			{
				String stream = StringUtils.getNextStringValue(buffer);
				int numMemoryAccesses = Integer.parseInt(StringUtils.getNextStringValue(buffer));
				if(stream.equals(streamName))
					return numMemoryAccesses;
			}
		}
		
		return 1;
	}
	
	static private boolean isSystemSystolicArray()
	{
		StringBuffer buffer = new StringBuffer();
		
		String filePath = fileToGenerateTestbenchFor.getAbsolutePath();
		String fileName = fileToGenerateTestbenchFor.getName();
		
		FileUtils.addFileContentsToBuffer(buffer, Activator.getOptFile(fileToGenerateTestbenchFor));
		
		while(buffer.length() > 0)
		{
			String value = StringUtils.getNextStringValue(buffer);
			if(value.equals("SystolicArrayGeneration"))
			{
				return true;
			}
		}
		
		return false;
	}
	
	static private void generateUUTPortMap(StringBuffer buffer)
	{
		buffer.append(tab + tab + "-- Instantiate Unit Under Test (UUT)\n");
		buffer.append(tab + tab + "uut : " + componentName + " PORT MAP (\n");
		buffer.append(tab + tab + tab + "clk,\n");
		buffer.append(tab + tab + tab + "rst,\n");
		buffer.append(tab + tab + tab + "inputReady,\n");
		buffer.append(tab + tab + tab + "outputReady,\n");
		buffer.append(tab + tab + tab + "done,\n");
		buffer.append(tab + tab + tab + "stall,\n");
	
		int count = DatabaseInterface.getTotalPorts(componentName);
		for(int i = 0; i < count; ++i)
		{
			String vhdlName = DatabaseInterface.getVHDLNameFromOrder(componentName, i);
			buffer.append(tab + tab + tab + vhdlName);		
						
			if(oneBitStreamChannels.contains(vhdlName))
				buffer.append("(0)");
			if(i != count - 1)
				buffer.append(",");
			buffer.append("\n");
		}
		
		buffer.append(tab + tab + tab + ");\n\n");		
	}
	
	static private void generateSynchInterfacePortMaps(StringBuffer buffer, String component)
	{
		String[] outputStreams = DatabaseInterface.getOutputStreams(component);
		
		for(int i = 0; i < outputStreams.length; ++i)
		{
			String stream = outputStreams[i];
			int numChannels = DatabaseInterface.getNumStreamChannels(component, stream);
			String[] streamChannels = DatabaseInterface.getStreamChannels(component, stream);
			String[] nonChannelPorts = DatabaseInterface.getNonChannelStreamPorts(component, stream);
			int streamChannelWidth = DatabaseInterface.getStreamBitSize(component, stream);
			
			for(int j = 0; j < numChannels; ++j)
			{
				buffer.append(tab + "U_" + stream + "_" + j + "_Synch : SynchInterfaceGeneric\n");
				buffer.append(tab + "generic map(\n");
				buffer.append(tab + tab + "INPUT_DATA_WIDTH => " + streamChannelWidth + ",\n");
				buffer.append(tab + tab + "OUTPUT_DATA_WIDTH => " + streamChannelWidth + "\n");
				buffer.append(tab + ") port map(\n");
				buffer.append(tab + tab + "clk => clk,\n");
				buffer.append(tab + tab + "rst => rst,\n");
				buffer.append(tab + tab + "data_in => " + streamChannels[j] + ",\n");
				buffer.append(tab + tab + "data_empty => " + nonChannelPorts[DatabaseInterface.STOP_ACCESS_PORT] + ",\n");
				buffer.append(tab + tab + "data_read => " + nonChannelPorts[DatabaseInterface.ENABLE_ACCESS_PORT] + ",\n");
				buffer.append(tab + tab + "address_in => " + stream + "_" + j + "_address_fifo_data_out,\n");
				buffer.append(tab + tab + "address_valid => " + stream + "_" + j + "_address_side_valid,\n");
				buffer.append(tab + tab + "stallAddress => open,\n");
				buffer.append(tab + tab + "addressPop => " + stream + "_" + j + "_address_fifo_read_enable_in,\n");
				buffer.append(tab + tab + "mc_stall_in => '0',\n");
				buffer.append(tab + tab + "mc_vadr_out => " + stream + "_" + j + "_address_fifo_data_out_2,\n");
				buffer.append(tab + tab + "mc_valid_out => " + stream + "_" + j + "_mc_valid_out,\n");
				buffer.append(tab + tab + "mc_data_out => " + stream + "_" + j + "_mc_data_out\n");
				buffer.append(tab + ");\n\n");
			}
		}
	}
	
	static private void generateOutputChecks(StringBuffer buffer, int setNum)
	{
		for(int i = 0; i < outputScalars.length; ++i)
		{	
			buffer.append(tab + tab + tab + "assert (" + DatabaseInterface.getVHDLPortName(componentName, outputScalars[i]) + " = ");
			
			String vhdlValue;
			
			if(StringUtils.isAHexValue(outputScalarValues.get(i).get(setNum)))
			{
				vhdlValue = outputScalarValues.get(i).get(setNum).substring(1);
				if(vhdlValue.charAt(0) == 'x' || vhdlValue.charAt(0) == 'X')
					vhdlValue = vhdlValue.substring(1);
				buffer.append("x\"" + vhdlValue + "\"");
			}
			else
			{
				int bitSize = DatabaseInterface.getPortSize(componentName, outputScalars[i]);
				vhdlValue = StringUtils.numberToBinary(outputScalarValues.get(i).get(setNum), bitSize, DatabaseInterface.getPortDataType(componentName, outputScalars[i]));
				
				buffer.append((bitSize == 1? "'" : "\"") + vhdlValue + (bitSize == 1? "'" : "\""));
			}
			
			buffer.append(") report \"Test Set " + (setNum + 1) + " failure: " + outputScalars[i] + " = " + outputScalarValues.get(i).get(setNum) + " failed. Actual value = \" & to_string(" + DatabaseInterface.getVHDLPortName(componentName, outputScalars[i]) + ") severity warning;\n");
		}
	}
	
	static private void generateOutputStreamExpectedValues(StringBuffer buffer)
	{
		try
		{
			String[] outputStreams = DatabaseInterface.getOutputStreams(componentName);
			
			
			for(int i = 0; i < outputStreams.length; ++i)
			{
				String[] nonChannelPorts = DatabaseInterface.getNonChannelStreamPorts(componentName, outputStreams[i]);
				String[] streamChannels = DatabaseInterface.getStreamChannels(componentName, outputStreams[i]);
				
				for(int j = 0; j < streamChannels.length; ++j)
					buffer.append(tab + outputStreams[i] + "_" + j + "_address_side_valid <= not " + outputStreams[i] + "_" + j + "_address_fifo_empty_out;\n");
				
				buffer.append(tab + outputStreams[i] + "_done <= done and " + nonChannelPorts[DatabaseInterface.STOP_ACCESS_PORT]);                                  
				for(int j = 0; j < streamChannels.length; ++j)
				{
					buffer.append(" and " + outputStreams[i] + "_" + j + "_address_fifo_empty_out");
				}
				buffer.append(";\n\n");
					
				for(int j = 0; j < streamChannels.length; ++j)
					buffer.append(tab + outputStreams[i] + "_" + j + "_stream_data_valid <= '1' ;\n\n");
				
				/*buffer.append(tab + "process(" + nonChannelPorts[0] + ", rst)\n");
				buffer.append(tab + "begin\n");
				buffer.append(tab + tab + "if(rst = '1') then\n");
				for(int j = 0; j < streamChannels.length; ++j)
					buffer.append(tab + tab + tab + outputStreams[i] + "_" + j + "_stream_data_valid <= '0';\n");
				buffer.append(tab + tab + "elsif(" + nonChannelPorts[0] + "'event and " + nonChannelPorts[0] + " = '1') then\n");
				for(int j = 0; j < streamChannels.length; ++j)
					buffer.append(tab + tab + tab + outputStreams[i] + "_" + j + "_stream_data_valid <= " + outputStreams[i] + "_" + j + "_stream_read_enable_out;\n");
				buffer.append(tab + tab + "end if;\n");
				buffer.append(tab + "end process;\n\n");*/
				
				
				
				
				
				/*for(int j = 0; j < streamChannels.length; ++j)
				{
					buffer.append(tab + outputStreams[i] + "_" + j + "_address_fifo_read_enable_in <= " + outputStreams[i] + "_" + j + "_stream_read_enable_out and not " + nonChannelPorts[DatabaseInterface.STOP_ACCESS_PORT] + " and not " + outputStreams[i] + "_" + j + "_address_fifo_empty_out;\n");
					buffer.append(tab + nonChannelPorts[DatabaseInterface.ENABLE_ACCESS_PORT] + " <= " + outputStreams[i] + "_" + j + "_address_fifo_read_enable_in;\n\n");
				}*/
				
				int[] dimensions = new int[2];		
				
				String outputTestFile = outputTestFiles[i];
				
				StringBuffer memoryValues = new StringBuffer();
				 
				//if(indices.length == 1)
				generate1DMemoryMappingFromFile(memoryValues, outputTestFile, dimensions, maxChannelBitwidth, outputStreams[i]);
				//else
				//	generate2DMemoryMappingFromFile(memoryValues, outputTestFile, dimensions, maxChannelBitwidth, outputStreams[i]);
				
				buffer.append(tab + outputStreams[i] + "_stream_out : OutputStream\n");
				buffer.append(tab + "generic map (\n");
				buffer.append(tab + tab + "CHANNEL_BITWIDTH => " + DatabaseInterface.getStreamPortSize(componentName, outputStreams[i], DatabaseInterface.getStreamChannels(componentName, outputStreams[i])[0]) + ",\n");
				buffer.append(tab + tab + "NUM_MEMORY_ELEMENTS => " + dimensions[0] + ",\n");
				buffer.append(tab + tab + "NUM_CHANNELS => " + DatabaseInterface.getNumStreamChannels(componentName, outputStreams[i]) + ",\n");
				buffer.append(tab + tab + "STREAM_NAME => \"" + outputStreams[i] + "\"\n");
				buffer.append(tab + tab + ")\n");
				
				
				
				buffer.append(tab + "port map (\n");
				buffer.append(tab + tab + "clk => " + nonChannelPorts[DatabaseInterface.CROSS_CLK_PORT] + ",\n");
				buffer.append(tab + tab + "rst => rst,\n");
				buffer.append(tab + tab + "done_in => " + outputStreams[i] + "_done,\n");
				
				buffer.append(tab + tab + "empty_in => " + nonChannelPorts[DatabaseInterface.STOP_ACCESS_PORT]);			
					
				for(int j = 0; j < streamChannels.length; ++j)
					buffer.append(" or " + outputStreams[i] + "_" + j + "_address_fifo_empty_out");
				buffer.append(",\n");
				
				buffer.append(tab + tab + "read_enable_out => " + outputStreams[i] + "_0_stream_read_enable_out");
				
				//for(int j = 1; j < streamChannels.length; ++j)
				//buffer.append(" and " + outputStreams[i] + "_" + j + "_stream_read_enable_out");
				buffer.append(",\n");
				
				buffer.append(tab + tab + "channel_in => " + outputStreams[i] + "_" + 0 + "_mc_data_out");
				
				for(int j = 1; j < streamChannels.length; ++j)
					buffer.append(" & " + outputStreams[i] + "_" + j + "_mc_data_out");
				
				buffer.append(",\n");
				
				buffer.append(tab + tab + "address_in => " + outputStreams[i] + "_0_address_fifo_data_out_2");
				for(int j = 1; j < streamChannels.length; ++j)
					buffer.append(" & " + outputStreams[i] + "_" + j + "_address_fifo_data_out_2");
				 
				buffer.append(",\n");
				
				//for(int j = 1; j < streamChannels.length; ++j)
				//	buffer.append(" " + outputStreams[i] + "_" + j + "_stream_fifo_data_out,\n");
				
				buffer.append(tab + tab + "read_in => " + outputStreams[i] + "_0_stream_data_valid and not " + outputStreams[i] + "_0_address_fifo_empty_out,\n");
				buffer.append(tab + tab + "OUTPUT_CORRECT =>\n");
				
				
				buffer.append(memoryValues);
				buffer.append(tab + tab + ");\n\n");
			
				int numChannels = DatabaseInterface.getNumStreamChannels(componentName, outputStreams[i]);
				
				for(int j = numChannels - 1; j >= 0; --j)
				{
					String streamChannel = DatabaseInterface.getStreamChannels(componentName, outputStreams[i])[j];
				
					buffer.append(tab + tab + outputStreams[i] + "_stream_channel_out(" + (j + 1) + " * ");
					buffer.append(streamChannel + "'length - 1 downto " + j + " * " + streamChannel + "'length) <= " + streamChannel + ";\n");
				}
				/*for(int j = DatabaseInterface.getNumStreamAddresses(componentName, outputStreams[i]) - 1; j >= 0; --j)
				{
					String streamAddress = DatabaseInterface.getStreamAddresses(componentName, outputStreams[i])[j];
					buffer.append(tab + tab + outputStreams[i] + "_stream_address_out(" + (j + 1) + " * " + streamAddress + "'length - 1 downto " + j + " * " + streamAddress + "'length) <= " + streamAddress + ";\n");
				}*/
				buffer.append("\n");
				
				String[] channelPorts = DatabaseInterface.getStreamChannelPorts(componentName, outputStreams[i]);
				
				GenerationFunctions.portMapSpecializedBurstAddressGenerator(buffer, outputStreams[i] + "_address_translator_output_" + i, componentName, outputStreams[i], false);
				
				for(int j = 0; j < channelPorts.length / 3; ++j)
				{
					/*buffer.append(tab + outputStreams[i] + "_address_translator" + j + " : BurstAddressGen PORT MAP(\n");
					buffer.append(tab + tab + "clk => clk,\n");
					buffer.append(tab + tab + "rst => rst,\n");
					buffer.append(tab + tab + "base_address_in => " + channelPorts[j * 3 + 1] + ",\n");
					buffer.append(tab + tab + "burst_size_in => " + channelPorts[j * 3 + 2] + ",\n");
					buffer.append(tab + tab + "burst_valid_in => " + nonChannelPorts[DatabaseInterface.ADDRESS_READY_PORT] + ",\n");
					buffer.append(tab + tab + "burst_stall_out => " + nonChannelPorts[DatabaseInterface.STREAM_ADDRESS_STALL_PORT] + ",\n");
					buffer.append(tab + tab + "address_valid_out => " + outputStreams[i] + "_address_translator_address_valid_out,\n");
					buffer.append(tab + tab + "address_out => " + outputStreams[i] + "_" + j + "_address_translator_address_out,\n");
					buffer.append(tab + tab + "address_stall_in => " + outputStreams[i] + "_" + j + "_address_translator_address_stall_in\n");
					buffer.append(tab + ");\n\n");*/
					
					buffer.append(tab + outputStreams[i] + "_" + j + "_address_lilo : MicroFifo\n");
					buffer.append(tab + tab + "generic map(\n");
					buffer.append(tab + tab + tab + "ADDRESS_WIDTH => 8,\n");
					buffer.append(tab + tab + tab + "DATA_WIDTH => 32,\n");
					buffer.append(tab + tab + tab + "ALMOST_FULL_COUNT => 0,\n");
					buffer.append(tab + tab + tab + "ALMOST_EMPTY_COUNT => 0\n");
					buffer.append(tab + tab + ") port map(\n");
					buffer.append(tab + tab + tab + "clk => clk,\n");
					buffer.append(tab + tab + tab + "rst => rst,\n");
					buffer.append(tab + tab + tab + "data_in => " + outputStreams[i] + "_" + j + "_address_translator_address_out,\n");
					buffer.append(tab + tab + tab + "valid_in => " + outputStreams[i] + "_address_translator_address_valid_out,\n");
					buffer.append(tab + tab + tab + "full_out => " + outputStreams[i] + "_" + j + "_address_translator_address_stall_in,\n");
					buffer.append(tab + tab + tab + "data_out => " + outputStreams[i] + "_" + j + "_address_fifo_data_out,\n");
					buffer.append(tab + tab + tab + "read_enable_in => " + outputStreams[i] + "_" + j + "_address_fifo_read_enable_in,\n");
					buffer.append(tab + tab + tab + "empty_out => " + outputStreams[i] + "_" + j + "_address_fifo_empty_out\n");
					buffer.append(tab + ");\n\n");
 				}	
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	
	static private void generate1DMemoryMappingFromFile(StringBuffer buffer, String file, int[] dimensions, int channelWidth, String stream)
	{
		StringBuffer fileContents = new StringBuffer();
		FileUtils.addFileContentsToBuffer(fileContents, file);
		
		//NEED TO GET ONE LINE FROM THE FILE AT A TIME
		String value = new String(" ");
		
		dimensions[0] = 0;
		value = StringUtils.getNextStringValue(fileContents);
		buffer.append(tab + tab + "(\n");
		while(!value.equals(""))
		{
			buffer.append(tab + tab + "\"" + StringUtils.numberToBinary(value, channelWidth, DatabaseInterface.getStreamDataType(componentName, stream)) + "\"");
			++dimensions[0];
			value = StringUtils.getNextStringValue(fileContents);
			if(!value.equals(""))
				buffer.append(",\n");
		}
		buffer.append(")");
		buffer.append("\n");	
	}
	
	static private void generateClockProcesses(StringBuffer buffer)
	{
		buffer.append(tab + tab + "-- Clock Process Definition\n");
		buffer.append(tab + tab + "clk_process : process\n");
		buffer.append(tab + tab + "begin\n");
		buffer.append(tab + tab + tab + "clk <= '0';\n");
		buffer.append(tab + tab + tab + "wait for clk_period / 2;\n");
		buffer.append(tab + tab + tab + "clk <= '1';\n");
		buffer.append(tab + tab + tab + "wait for clk_period / 2;\n");
		buffer.append(tab + tab + "end process;\n\n");
		
		if(isSystem)
		{
			String[] clkNames = DatabaseInterface.getSystemClocks(componentName);
			
			for(int i = 0; i < clkNames.length; ++i)
			{
				buffer.append(tab + tab + "-- " + clkNames[i] + " Clock Process Definition\n");
				buffer.append(tab + tab + clkNames[i] + "_process : process\n");
				buffer.append(tab + tab + "begin\n");
				buffer.append(tab + tab + tab + clkNames[i] + " <= '0';\n");
				buffer.append(tab + tab + tab + "wait for " + clkNames[i] + "_clk_period / 2;\n");
				buffer.append(tab + tab + tab + clkNames[i] + " <= '1';\n");
				buffer.append(tab + tab + tab + "wait for " + clkNames[i] + "_clk_period / 2;\n");
				buffer.append(tab + tab + "end process;\n\n");
			}
			
			
			/*
			String[] inputStreams = DatabaseInterface.getInputStreams(componentName);
			String[] outputStreams = DatabaseInterface.getOutputStreams(componentName);
			
			for(int i = 0; i < inputStreams.length; ++i)
			{
				String[] nonStreamChannelPorts = DatabaseInterface.getNonChannelStreamPorts(componentName, inputStreams[i]);
				
				buffer.append(tab + tab + "-- Stream " + inputStreams[i] + " Clock Process Definition\n");
				buffer.append(tab + tab + nonStreamChannelPorts[0] + "_process : process\n");
				buffer.append(tab + tab + "begin\n");
				buffer.append(tab + tab + tab + nonStreamChannelPorts[0] + " <= '0';\n");
				buffer.append(tab + tab + tab + "wait for " + nonStreamChannelPorts[0] + "_clk_period / 2;\n");
				buffer.append(tab + tab + tab + nonStreamChannelPorts[0] + " <= '1';\n");
				buffer.append(tab + tab + tab + "wait for " + nonStreamChannelPorts[0] + "_clk_period / 2;\n");
				buffer.append(tab + tab + "end process;\n\n");
			}
				
			for(int i = 0; i < outputStreams.length; ++i)
			{
				String[] nonStreamChannelPorts = DatabaseInterface.getNonChannelStreamPorts(componentName, outputStreams[i]);
				
				buffer.append(tab + tab + "-- Stream " + outputStreams[i] + " Clock Process Definition\n");
				buffer.append(tab + tab + nonStreamChannelPorts[0] + "_process : process\n");
				buffer.append(tab + tab + "begin\n");
				buffer.append(tab + tab + tab + nonStreamChannelPorts[0] + " <= '0';\n");
				buffer.append(tab + tab + tab + "wait for " + nonStreamChannelPorts[0] + "_clk_period / 2;\n");
				buffer.append(tab + tab + tab + nonStreamChannelPorts[0] + " <= '1';\n");
				buffer.append(tab + tab + tab + "wait for " + nonStreamChannelPorts[0] + "_clk_period / 2;\n");
				buffer.append(tab + tab + "end process;\n\n");
			}*/
			
			
		}
		
		buffer.append(tab + tab + "-- Stimulus Process\n");
		buffer.append(tab + tab + "stim_proc : process\n");
		buffer.append(tab + tab + "begin\n");
		buffer.append(tab + tab + tab + "wait until clk'event and clk = '1';\n");
		buffer.append(tab + tab + tab + "inputReady <= " + (isSystem? "'1'" : "'0'") + ";\n");
		buffer.append(tab + tab + tab + "stall <= '0';\n");
		buffer.append(tab + tab + tab + "wait for clk_period * 10;\n");
		buffer.append(tab + tab + tab + "rst <= '1';\n");
		buffer.append(tab + tab + tab + "wait for clk_period * 10;\n");
		buffer.append(tab + tab + tab + "rst <= '0';\n\n");
		
		//Input Values
		int numInputs = inputScalarValues.size() > 0?  inputScalarValues.get(0).size() : 0;
		if(isSystem)
			numInputs = 1;
		
		for(int i = 0; i < numInputs; ++i)
		{
			buffer.append(tab + tab + tab + "--Test Set " + (i + 1) + "\n");
			generateInputScalarValues(buffer, i);
		
			buffer.append(tab + tab + tab + "inputReady <= '1';\n");
			buffer.append(tab + tab + tab + "wait for clk_period;\n\n");
		}
		if(!isSystem)	
			buffer.append(tab + tab + tab + "inputReady <= '0';\n");
		
		buffer.append(tab + tab + tab + "wait;\n");
		buffer.append(tab + tab + "end process;\n\n");
		
		if(!isSystem || outputScalarValues.size() > 0)
		{
			//Output checking
			buffer.append(tab + tab + "OutputProcess : process\n");
			buffer.append(tab + tab + "begin\n");
			
			buffer.append(tab + tab + tab + "wait until clk'event and clk ='1' and " + ("rst = ") + "'1';\n");
			
			buffer.append(tab + tab + tab + "wait until clk'event and clk ='1' and " + (isSystem? "done = " : "outputReady = ") + "'1';\n");
			
			int numOutputs = outputScalarValues.size() > 0? outputScalarValues.get(0).size() : 0;
			
			if(numInputs > 0 || numOutputs > 0)
			{
				buffer.append("\n" + tab + tab + tab + "--Test Set 1\n");
				generateOutputChecks(buffer, 0);
				//buffer.append("\n");
			}
			
			for(int i = 1; i < numOutputs; ++i)
			{	
				buffer.append(tab + tab + tab + "wait until clk'event and clk = '1' and outputReady = '1';\n\n");
			
				buffer.append(tab + tab + tab + "--Test Set " + (i + 1) + "\n");
				generateOutputChecks(buffer, i);
				
			}
			
			buffer.append(tab + tab + tab + "report \"Done processing " + componentName + ". Any errors are reported above.\";\n");
			
			buffer.append(tab + tab + tab + "wait;\n");
			buffer.append(tab + tab + "end process;\n\n");
		}
		
		
		
		buffer.append(tab + "END behavior;\n");
	}
}
