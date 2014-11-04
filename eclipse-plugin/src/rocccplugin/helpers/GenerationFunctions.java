package rocccplugin.helpers;

import java.util.Vector;

import rocccplugin.database.DatabaseInterface;

public class GenerationFunctions 
{
	static String tab = "\t";
	
	static void appendFileHeaders(StringBuffer buffer)
	{
		buffer.append("LIBRARY ieee;\n");
		buffer.append("USE ieee.std_logic_1164.ALL;\n");
		buffer.append("USE ieee.std_logic_unsigned.all;\n");
		
		buffer.append("USE ieee.numeric_std.ALL;\n\n");	
	}
	
	static void appendHelperFunctionHeaders(StringBuffer buffer)
	{
		buffer.append("use work.HelperFunctions.all;\n");
		buffer.append("use work.HelperFunctions_Unsigned.all;\n");
		buffer.append("use work.HelperFunctions_Signed.all;\n\n");
	}
	
	public static void determineNecessaryBurstAddressGenerators(String componentName, Vector<Integer> specialBurstAddressGeneratorsUsed)
	{
		String[] inputStreams = DatabaseInterface.getInputStreams(componentName);
		
		for(int i = 0; i < inputStreams.length; ++i)
		{
			int dataChannels = DatabaseInterface.getNumStreamChannels(componentName, inputStreams[i]);
			int addressChannels = DatabaseInterface.getStreamPortsOfType(componentName, inputStreams[i], "STREAM_ADDRESS_BASE").length;
			
			if(addressChannels == 1)
			{					
				if(!specialBurstAddressGeneratorsUsed.contains(dataChannels))
					specialBurstAddressGeneratorsUsed.add(dataChannels);
			}
			else if(!specialBurstAddressGeneratorsUsed.contains(1))
					specialBurstAddressGeneratorsUsed.add(1);
				
		}
		
		String[] outputStreams = DatabaseInterface.getOutputStreams(componentName);
		
		for(int i = 0; i < outputStreams.length; ++i)
		{
			int dataChannels = DatabaseInterface.getNumStreamChannels(componentName, outputStreams[i]);
			int addressChannels = DatabaseInterface.getStreamPortsOfType(componentName, outputStreams[i], "STREAM_ADDRESS_BASE").length;
			
			if(addressChannels == 1)
			{					
				if(!specialBurstAddressGeneratorsUsed.contains(dataChannels))
					specialBurstAddressGeneratorsUsed.add(dataChannels);
			}
			else if(!specialBurstAddressGeneratorsUsed.contains(1))
					specialBurstAddressGeneratorsUsed.add(1);
				
		}
	}
	
	public static void generateSpecializedBurstAddressGenerators(StringBuffer buffer, Vector<Integer> dataChannels)
	{
		for(int i = 0; i < dataChannels.size(); ++i)
		{
			appendFileHeaders(buffer);
			appendHelperFunctionHeaders(buffer);
			
			//Entity Phase
			buffer.append("entity SpecialBurstAddressGen" + dataChannels.get(i) + "Channels is\n");
			buffer.append(tab + "port(\n");
			buffer.append(tab + tab + "clk : in STD_LOGIC;\n");
			buffer.append(tab + tab + "rst : in STD_LOGIC;\n\n");
			
			buffer.append(tab + tab + "burst_address_in : in STD_LOGIC_VECTOR(31 downto 0);\n");
			buffer.append(tab + tab + "burst_count_in : in STD_LOGIC_VECTOR(31 downto 0);\n");
			buffer.append(tab + tab + "burst_valid_in : in STD_LOGIC;\n");
			buffer.append(tab + tab + "burst_stall_out : out STD_LOGIC;\n\n");
			
			buffer.append(tab + tab + "address_valid_out : out STD_LOGIC;\n");
			
			for(int j = 0; j < dataChannels.get(i); ++j)
			{
				buffer.append(tab + tab + "address_stall_channel" + j + "_in : in STD_LOGIC;\n");
			}
			
			for(int j = 0; j < dataChannels.get(i); ++j)
			{
				if(j > 0)
					buffer.append(";");
				buffer.append("\n");
				buffer.append(tab + tab + "address_channel" + j + "_out : out STD_LOGIC_VECTOR(31 downto 0)");
			}
			
			buffer.append("\n");
			buffer.append(tab + tab + ");\n");
			buffer.append("end SpecialBurstAddressGen" + dataChannels.get(i) + "Channels;\n\n");
		
			//Architecture Phase
			buffer.append("architecture Generated of SpecialBurstAddressGen" + dataChannels.get(i) + "Channels is\n\n");
			
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
			
			buffer.append(tab + "signal base_value : std_logic_vector(31 downto 0);\n");
			buffer.append(tab + "signal burst_value : std_logic_vector(31 downto 0);\n");
			buffer.append(tab + "signal micro_read_enable_in : std_logic;\n");
			buffer.append(tab + "signal micro_empty_out : std_logic;\n\n");
			
			buffer.append(tab + "type STATE_TYPE is (S_READY, S_READ, S_POP);\n\n");
			
			buffer.append(tab + "signal state : STATE_TYPE;\n\n");
			
			buffer.append(tab + "signal cur_address : std_logic_vector(31 downto 0);\n");
			buffer.append(tab + "signal end_address : std_logic_vector(31 downto 0);\n");
			
			buffer.append("begin\n\n");
			
			buffer.append(tab + "micro_lilo : MicroFifo\n");
			buffer.append(tab + tab + "generic map(\n");
			buffer.append(tab + tab + tab + tab + "ADDRESS_WIDTH => 4,\n");
			buffer.append(tab + tab + tab + tab + "DATA_WIDTH => 64,\n");
			buffer.append(tab + tab + tab + tab + "ALMOST_FULL_COUNT => 3,\n");
			buffer.append(tab + tab + tab + tab + "ALMOST_EMPTY_COUNT => 0\n");
			buffer.append(tab + tab + tab + ")\n");
			buffer.append(tab + tab + "port map(\n");
			buffer.append(tab + tab + tab + "clk => clk,\n");
			buffer.append(tab + tab + tab + "rst => rst,\n");
			buffer.append(tab + tab + tab + "data_in(31 downto 0) => burst_address_in,\n");
			buffer.append(tab + tab + tab + "data_in(63 downto 32) => burst_count_in,\n");
			buffer.append(tab + tab + tab + "valid_in => burst_valid_in,\n");
			buffer.append(tab + tab + tab + "full_out => burst_stall_out,\n");
			buffer.append(tab + tab + tab + "data_out(31 downto 0) => base_value,\n");
			buffer.append(tab + tab + tab + "data_out(63 downto 32) => burst_value,\n");
			buffer.append(tab + tab + tab + "read_enable_in => micro_read_enable_in,\n");
			buffer.append(tab + tab + tab + "empty_out => micro_empty_out\n");
			buffer.append(tab + tab + ");\n\n");
			
			buffer.append(tab + "process(clk, rst)\n");
			buffer.append(tab + "begin\n");
			buffer.append(tab + tab + "if (rst = '1') then\n");
			buffer.append(tab + tab + tab + "address_valid_out <= '0';\n");
			
			for(int j = 0; j < dataChannels.get(i); ++j)
			{
				buffer.append(tab + tab + tab + "address_channel" + j + "_out <= (others => '0');\n");
			}
			
			buffer.append(tab + tab + tab + "cur_address <= (others => '0');\n");
			buffer.append(tab + tab + tab + "end_address <= (others => '0');\n");
			buffer.append(tab + tab + tab + "micro_read_enable_in <= '0';\n");
			buffer.append(tab + tab + tab + "state <= S_READY;\n\n");
			
			buffer.append(tab + tab + "elsif (clk'event and clk ='1') then\n");
			buffer.append(tab + tab + tab + "address_valid_out <= '0';\n");
			buffer.append(tab + tab + tab + "micro_read_enable_in <= '0';\n\n");
			
			buffer.append(tab + tab + tab + "case state is\n\n");
			buffer.append(tab + tab + tab + tab + "when S_READY =>\n\n");
			
			buffer.append(tab + tab + tab + tab + tab + "if(micro_empty_out = '0') then\n");
			buffer.append(tab + tab + tab + tab + tab + tab + "state <= S_POP;\n");
			buffer.append(tab + tab + tab + tab + tab + tab + "micro_read_enable_in <= '1';\n");
			buffer.append(tab + tab + tab + tab + tab + "else\n");
			buffer.append(tab + tab + tab + tab + tab + tab + "state <= S_READY;\n");
			buffer.append(tab + tab + tab + tab + tab + "end if;\n\n");
			
			buffer.append(tab + tab + tab + tab + "when S_POP =>\n");
			buffer.append(tab + tab + tab + tab + tab + "cur_address <= base_value;\n");
			buffer.append(tab + tab + tab + tab + tab + "end_address <= ROCCCADD(base_value, burst_value, 32);\n");
			buffer.append(tab + tab + tab + tab + tab + "state <= S_READ;\n");
			buffer.append(tab + tab + tab + tab + "when S_READ =>\n\n");
			
			buffer.append(tab + tab + tab + tab + tab + "if(");
			
			for(int j = 0; j < dataChannels.get(i); ++j)
			{
				if(j > 0)
					buffer.append(" and ");
				buffer.append("address_stall_channel" + j + "_in = '0'");
			}
			
			buffer.append(") then\n");
			buffer.append(tab + tab + tab + tab + tab + tab + "address_valid_out <= '1';\n");
			buffer.append(tab + tab + tab + tab + tab + tab + "address_channel0_out <= cur_address;\n");
			
			for(int j = 1; j < dataChannels.get(i); ++j)
			{
				//ONLY WORKS FOR CHANNELS < 9!!!
				buffer.append(tab + tab + tab + tab + tab + tab + "address_channel" + j + "_out <= ROCCCADD(cur_address, x\"0000000" + j + "\", 32);\n");
			}
			
			buffer.append(tab + tab + tab + tab + tab + tab + "cur_address <= ROCCCADD(cur_address, x\"0000000" + dataChannels.get(i) + "\", 32);\n");
			buffer.append(tab + tab + tab + tab + tab + tab + "if(ROCCCADD(cur_address, x\"0000000" + dataChannels.get(i) + "\", 32) >= end_address) then\n");
			buffer.append(tab + tab + tab + tab + tab + tab + "if(micro_empty_out = '0') then\n");
			buffer.append(tab + tab + tab + tab + tab + tab + tab + "state <= S_POP;\n");
			buffer.append(tab + tab + tab + tab + tab + tab + tab + "micro_read_enable_in <= '1';\n");
			buffer.append(tab + tab + tab + tab + tab + tab + "else\n");
			buffer.append(tab + tab + tab + tab + tab + tab + tab + "state <= S_READY;\n");
			buffer.append(tab + tab + tab + tab + tab + tab + "end if;\n");
			buffer.append(tab + tab + tab + tab + tab + "else\n");
			buffer.append(tab + tab + tab + tab + tab + tab + "state <= S_READ;\n");
			buffer.append(tab + tab + tab + tab + tab + "end if;\n");
			buffer.append(tab + tab + tab + tab + "else\n");
			buffer.append(tab + tab + tab + tab + tab + "state <= S_READ;\n");
			buffer.append(tab + tab + tab + tab + "end if;\n\n");
			
			buffer.append(tab + tab + tab + "when others =>\n");
			buffer.append(tab + tab + tab + tab + "state <= S_READY;\n");
			buffer.append(tab + tab + "end case;\n");
			
			buffer.append(tab + tab + "end if;\n");
			buffer.append(tab + "end process;\n\n");
			buffer.append("end Generated;\n\n");
		}		
	}

	public static void portMapSpecializedBurstAddressGenerator(StringBuffer buffer, String generatorName, String componentName, String stream, boolean inputStream)
	{
		String[] channelPorts = DatabaseInterface.getStreamChannelPorts(componentName, stream);
		String[] nonChannels = DatabaseInterface.getNonChannelStreamPorts(componentName, stream);
		String[] streamChannels = DatabaseInterface.getStreamChannels(componentName, stream);
		int channelSize = DatabaseInterface.getStreamPortSize(componentName, stream, streamChannels[0]);
		int numChannels = DatabaseInterface.getNumStreamChannels(componentName, stream);
		
		if(DatabaseInterface.getStreamPortsOfType(componentName, stream, "STREAM_ADDRESS_BASE").length > 1)
		{
			for(int j = 0; j < channelPorts.length / 3; ++j)
			{
				buffer.append(tab + generatorName + j + " : SpecialBurstAddressGen1Channels PORT MAP(\n");
				buffer.append(tab + tab + "clk => clk,\n");
				buffer.append(tab + tab + "rst => rst,\n");
				
				buffer.append(tab + tab + "burst_address_in => " + channelPorts[j * 3 + 1] + ",\n");
				buffer.append(tab + tab + "burst_count_in => " + channelPorts[j * 3 + 2] + ",\n");
				buffer.append(tab + tab + "burst_valid_in => " + nonChannels[DatabaseInterface.ADDRESS_READY_PORT] + ",\n");
				buffer.append(tab + tab + "burst_stall_out => " + nonChannels[DatabaseInterface.STREAM_ADDRESS_STALL_PORT] + ",\n");
				buffer.append(tab + tab + "address_valid_out => " + stream + "_address_translator_address_valid_out,\n");
				
//				buffer.append(tab + tab + "address_stall_channel0_in => '0',\n");
				if(inputStream)
				{
					buffer.append(tab + tab + "address_stall_channel0_in => '0',\n") ;
					buffer.append(tab + tab + "address_channel0_out => " + stream + "_stream_address_in(" + (((j + 1) * 32) - 1) + " downto " + (j * 32) + ")\n");
				}
				else
				{
					buffer.append(tab + tab + "address_stall_channel0_in => " + stream + "_" + j + "_address_translator_address_stall_in,\n") ;
					buffer.append(tab + tab + "address_channel0_out => " + stream + "_" + j + "_address_translator_address_out\n");
				}
				
//				buffer.append("\n");
				buffer.append(tab + ");\n\n");
			}
		}
		else
		{
			//Generate Special Burst Generator
			
			buffer.append(tab + generatorName + " : SpecialBurstAddressGen" + numChannels + "Channels PORT MAP(\n");
			buffer.append(tab + tab + "clk => clk,\n");
			buffer.append(tab + tab + "rst => rst,\n");
			buffer.append(tab + tab + "burst_address_in => " + channelPorts[1] + ",\n");
			buffer.append(tab + tab + "burst_count_in => " + channelPorts[2] + ",\n");
			buffer.append(tab + tab + "burst_valid_in => " + nonChannels[DatabaseInterface.ADDRESS_READY_PORT] + ",\n");
			buffer.append(tab + tab + "burst_stall_out => " + nonChannels[DatabaseInterface.STREAM_ADDRESS_STALL_PORT] + ",\n");
			buffer.append(tab + tab + "address_valid_out => " + stream + "_address_translator_address_valid_out,\n");
			
			for(int j = 0; j < numChannels; ++j)
			{
				if (inputStream)
				{
					buffer.append(tab + tab + "address_stall_channel" + j + "_in => '0',\n");
				}
				else
				{
					buffer.append(tab + tab + "address_stall_channel" + j + "_in => " + stream + "_" + j + "_address_translator_address_stall_in,\n") ;
				}
			}
			
			for(int j = 0; j < numChannels; ++j)
			{
				if(j > 0)
					buffer.append(",\n");
				
				if(inputStream)
					buffer.append(tab + tab + "address_channel" + j + "_out => " + stream + "_stream_address_in(" + (((j + 1) * 32) - 1) + " downto " + (j * 32) + ")");
				else
					buffer.append(tab + tab + "address_channel" + j + "_out => " + stream + "_" + j + "_address_translator_address_out");
			}
			buffer.append("\n");
			buffer.append(tab + ");\n\n");
			
		}
	}

	public static void generateSynchInterfaceGeneric(StringBuffer buffer, int outputAddressSize)
	{
		appendFileHeaders(buffer);
		appendHelperFunctionHeaders(buffer);
		
		buffer.append("entity SynchInterfaceGeneric is\n");
		buffer.append(tab + "generic (\n");
		buffer.append(tab + tab + "INPUT_DATA_WIDTH : INTEGER;\n");
		buffer.append(tab + tab + "OUTPUT_DATA_WIDTH : INTEGER\n");
		buffer.append(tab + ");\n");
		buffer.append(tab + "port (\n");
		buffer.append(tab + tab + "clk : in STD_LOGIC;\n");
		buffer.append(tab + tab + "rst : in STD_LOGIC;\n");
		buffer.append(tab + tab + "data_in : in STD_LOGIC_VECTOR(INPUT_DATA_WIDTH - 1 downto 0);\n");
		buffer.append(tab + tab + "data_empty : in STD_LOGIC;\n");
		buffer.append(tab + tab + "data_read : out STD_LOGIC;\n");
		buffer.append(tab + tab + "address_in : in STD_LOGIC_VECTOR(" + (outputAddressSize - 1) + " downto 0);\n");
		buffer.append(tab + tab + "address_valid : in STD_LOGIC;\n");
		buffer.append(tab + tab + "stallAddress : out STD_LOGIC;\n");
		buffer.append(tab + tab + "addressPop : out STD_LOGIC;\n");
		buffer.append(tab + tab + "mc_stall_in : in STD_LOGIC;\n");
		buffer.append(tab + tab + "mc_vadr_out : out STD_LOGIC_VECTOR(" + (outputAddressSize - 1) + " downto 0);\n");
		buffer.append(tab + tab + "mc_valid_out : out STD_LOGIC;\n");
		buffer.append(tab + tab + "mc_data_out : out STD_LOGIC_VECTOR(OUTPUT_DATA_WIDTH - 1 downto 0)\n");
		buffer.append(tab + ");\n");
		buffer.append("end SynchInterfaceGeneric;\n\n");
	
		buffer.append("architecture Handwritten of SynchInterfaceGeneric is\n\n");
		buffer.append(tab + "type State is (ST0, ST1, ST2);\n");
		buffer.append(tab + "signal currentState : State;\n");
		buffer.append(tab + "signal data_internal : std_logic_vector(OUTPUT_DATA_WIDTH - 1 downto 0);\n\n");
		
		buffer.append("begin\n\n");
		buffer.append(tab + "L0: if (OUTPUT_DATA_WIDTH > INPUT_DATA_WIDTH) generate\n");
		buffer.append(tab + "data_internal(OUTPUT_DATA_WIDTH - 1 downto INPUT_DATA_WIDTH) <= (others => '0');\n");
		buffer.append(tab + "data_internal(INPUT_DATA_WIDTH - 1 downto 0) <= data_in ;\n");
		buffer.append(tab + "end generate L0;\n\n");
		
		buffer.append(tab + "L1: if (INPUT_DATA_WIDTH >= OUTPUT_DATA_WIDTH) generate\n");
		buffer.append(tab + tab + "data_internal <= data_in(OUTPUT_DATA_WIDTH - 1 downto 0);\n");
		buffer.append(tab + "end generate L1;\n\n");
		
		buffer.append(tab + "stallAddress <= mc_stall_in;\n\n");
		
		buffer.append(tab + "process(clk, rst)\n");
		buffer.append(tab + "begin\n");
		buffer.append(tab + tab + "if (rst = '1') then\n");
		buffer.append(tab + tab + tab + "mc_vadr_out <= (others => '0');\n");
		buffer.append(tab + tab + tab + "mc_valid_out <= '0';\n");
		buffer.append(tab + tab + tab + "mc_data_out <= (others => '0');\n");
		buffer.append(tab + tab + tab + "data_read <= '0';\n");
		buffer.append(tab + tab + tab + "addressPop <= '0';\n\n");
		
		buffer.append(tab + tab + tab + "currentState <= ST0;\n\n");
		
		buffer.append(tab + tab + "elsif (clk'event and clk = '1') then\n\n");
		buffer.append(tab + tab + tab + "data_read <= '0' ;\n");
		buffer.append(tab + tab + tab + "addressPop <= '0' ;\n");
		buffer.append(tab + tab + tab + "mc_valid_out <= '0';\n\n");
		buffer.append(tab + tab + tab + "case currentState is\n");
		buffer.append(tab + tab + tab + tab + "when ST0 =>\n");
		buffer.append(tab + tab + tab + tab + tab + "if (mc_stall_in = '0' and data_empty = '0' and address_valid = '1')\n");
		buffer.append(tab + tab + tab + tab + tab + "then\n");
		buffer.append(tab + tab + tab + tab + tab + tab + "data_read <= '1';\n");
		buffer.append(tab + tab + tab + tab + tab + tab + "currentState <= ST1;\n");
		buffer.append(tab + tab + tab + tab + tab + "end if;\n");
		buffer.append(tab + tab + tab + tab + "when ST1 =>\n");
		buffer.append(tab + tab + tab + tab + tab + "currentState <= ST2;\n");
		buffer.append(tab + tab + tab + tab + "when ST2 =>\n");
		buffer.append(tab + tab + tab + tab + tab + "addressPop <= '1';\n");
		buffer.append(tab + tab + tab + tab + tab + "mc_vadr_out <= address_in;\n");
		buffer.append(tab + tab + tab + tab + tab + "mc_valid_out <= '1';\n");
		buffer.append(tab + tab + tab + tab + tab + "mc_data_out <= data_internal;\n");
		buffer.append(tab + tab + tab + tab + tab + "currentState <= ST0;\n");
		buffer.append(tab + tab + tab + tab + "when others =>\n");
		buffer.append(tab + tab + tab + tab + tab + "currentState <= ST0;\n");
		buffer.append(tab + tab + tab + "end case;\n");
		buffer.append(tab + tab + "end if;\n");
		buffer.append(tab + "end process;\n\n");
		buffer.append("end Handwritten;\n");
	}
}
