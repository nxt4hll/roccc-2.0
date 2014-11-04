LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.std_logic_unsigned.all;
USE ieee.numeric_std.ALL;


entity InputStream is
    generic (
      CHANNEL_BITWIDTH: integer;
      NUM_CHANNELS : integer;
      CONCURRENT_MEM_ACCESS : integer;
      NUM_MEMORY_ELEMENTS : integer;
      STREAM_NAME : string
      );
    port (
      clk : in STD_LOGIC;
      rst : in STD_LOGIC;
      full_in : in STD_LOGIC;
      write_enable_out : out STD_LOGIC;
      channel_out : out STD_LOGIC_VECTOR(NUM_CHANNELS * CHANNEL_BITWIDTH - 1 downto 0);
      address_in : in STD_LOGIC_VECTOR(NUM_CHANNELS * 32 - 1 downto 0);
      read_in : in STD_LOGIC;
      memory : in STD_LOGIC_VECTOR(NUM_MEMORY_ELEMENTS * CHANNEL_BITWIDTH - 1 downto 0)
      );
end entity;
architecture behavior of InputStream is
begin

process(clk, rst)
  variable address : std_logic_vector(31 downto 0);
begin
  if( rst = '1' ) then
	 write_enable_out <= '0';
  elsif( clk'event and clk = '1' ) then
    write_enable_out <= '0';
    if( read_in = '1' ) then
		for n in 1 to NUM_CHANNELS
		loop
		  address := address_in(n*32-1 downto (n-1)*32);
		  if( conv_integer(address) < NUM_MEMORY_ELEMENTS ) then
          write_enable_out <= '1';
		    channel_out(CHANNEL_BITWIDTH*n-1 downto CHANNEL_BITWIDTH*(n-1)) <= memory((conv_integer(address)+1)*CHANNEL_BITWIDTH - 1 downto conv_integer(address)*CHANNEL_BITWIDTH);
		  else
		    report "Request for "&STREAM_NAME&"["&integer'image(conv_integer(address))&"] is out of bounds! "&STREAM_NAME&" is "&integer'image(NUM_MEMORY_ELEMENTS)&" elements!";
		  end if;
		end loop;
	  end if;
  end if;
end process;

end architecture;

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.std_logic_unsigned.all;
USE ieee.numeric_std.ALL;
USE work.output_memory_types.ALL;

entity OutputStream is
    generic (
      CHANNEL_BITWIDTH: integer;
      NUM_MEMORY_ELEMENTS: integer;
      NUM_CHANNELS : integer;
      STREAM_NAME: string
      );
    port (
      clk : in STD_LOGIC;
      rst : in STD_LOGIC;
      done_in : in STD_LOGIC;
      empty_in : in STD_LOGIC;
      read_enable_out : out STD_LOGIC;
      channel_in : in STD_LOGIC_VECTOR((NUM_CHANNELS * CHANNEL_BITWIDTH) - 1 downto 0);
      address_in : in STD_LOGIC_VECTOR(NUM_CHANNELS * 32 - 1 downto 0);
      read_in : in STD_LOGIC;
      OUTPUT_CORRECT : in t_1D_output_memory_type(0 to NUM_MEMORY_ELEMENTS - 1)
      );
end entity;
architecture behavior of OutputStream is
  type BRAM_STATE_TYPE is (S_EMPTY, S_READY, S_READ);
  signal bram_state : BRAM_STATE_TYPE;
begin
  process(clk, rst)
    variable can_process : boolean := false;
    type OUTPUT_MEMORY_TYPE is array (0 to NUM_MEMORY_ELEMENTS-1) of STD_LOGIC_VECTOR(CHANNEL_BITWIDTH - 1 downto 0);
    variable OUTPUT_MEMORY : OUTPUT_MEMORY_TYPE;
  begin
    if( rst = '1' ) then
	   OUTPUT_MEMORY := (others=>(others=>'0'));
		bram_state <= S_EMPTY;
		read_enable_out <= '0';
		can_process := true;
    elsif( clk'event and clk = '1' ) then
		read_enable_out <= '0';
	   case bram_state is
		  when S_EMPTY =>
			 if( Empty_in = '0' ) then
			   read_enable_out <= '1';
				bram_state <= S_READY;
			 end if;
		  when S_READY =>
			 read_enable_out <= '1';
			 bram_state <= S_READ;
		  when S_READ =>
			 --read?
			 if( Empty_in = '0' ) then
			   read_enable_out <= '1';
			 else
			   bram_state <= S_EMPTY;
			 end if;
		  when others =>
			 bram_state <= S_EMPTY;
		end case;
		if( read_in = '1' ) then
		  for n in 1 to NUM_CHANNELS
		  loop
		    if( conv_integer(address_in(n * 32 - 1 downto (n-1) * 32)) < NUM_MEMORY_ELEMENTS ) then
			   OUTPUT_MEMORY(conv_integer(address_in(n * 32 - 1 downto (n-1) * 32))) := channel_in(n * CHANNEL_BITWIDTH - 1 downto (n-1) * CHANNEL_BITWIDTH);
			 else
			   report "Address too large in "&STREAM_NAME&"["&integer'image(conv_integer(address_in(n * 32 - 1 downto (n-1) * 32)))&"]";
			 end if;
		  end loop;
      end if;
	   if( done_in = '1' and can_process ) then
		  can_process := false;
		  check_output: for i in 0 to NUM_MEMORY_ELEMENTS-1
	     loop
		    if( OUTPUT_CORRECT(i)'length <= 32 and OUTPUT_MEMORY(i)'length <= 32 ) then
		      assert OUTPUT_MEMORY(i) = OUTPUT_CORRECT(i)(OUTPUT_MEMORY(i)'length-1 downto 0) report STREAM_NAME&"["&integer'image(i)&"] incorrect! Expected "&integer'image(conv_integer(OUTPUT_CORRECT(i)))&", got "&integer'image(conv_integer(OUTPUT_MEMORY(i)))&"." severity error;
		    else
                      assert OUTPUT_MEMORY(i) = OUTPUT_CORRECT(i)(OUTPUT_MEMORY(i)'length-1 downto 0) report STREAM_NAME&"["&integer'image(i)&"] incorrect!" ;
                    end if;
		  end loop;
		  report "Finished processing output on "&STREAM_NAME&".";
	   end if;
    end if;
  end process;
end architecture;
