library IEEE ;
use IEEE.STD_LOGIC_1164.all ;
use IEEE.STD_LOGIC_UNSIGNED.all;
use IEEE.STD_LOGIC_ARITH.all;
use work.HelperFunctions.all;
use work.HelperFunctions_Unsigned.all;
use work.HelperFunctions_Signed.all;

entity BurstAddressGen is
port (
  clk : in STD_LOGIC;
  rst : in STD_LOGIC;
  --input burst data
  base_address_in : in STD_LOGIC_VECTOR(31 downto 0);
  burst_size_in : in STD_LOGIC_VECTOR(31 downto 0);
  burst_valid_in : in STD_LOGIC;
  --burst_address_read_out : out STD_LOGIC;
  burst_stall_out : out STD_LOGIC;
  --output addresses
  address_valid_out : out STD_LOGIC;
  address_out : out STD_LOGIC_VECTOR(31 downto 0);
  address_stall_in : in STD_LOGIC
);
end BurstAddressGen;

architecture Behavioral of BurstAddressGen is

--used when reading from an external BRAM
--type BRAM_STATE_TYPE is (S_EMPTY, S_READY, S_READ, S_STALL);
--signal bram_state : BRAM_STATE_TYPE;
--signal micro_valid_in : STD_LOGIC;
--signal micro_data_in : STD_LOGIC_VECTOR(63 downto 0);
--signal micro_full_out : STD_LOGIC;
signal base_value : STD_LOGIC_VECTOR(31 downto 0);
signal burst_value : STD_LOGIC_VECTOR(31 downto 0);
signal micro_read_enable_in : STD_LOGIC;
signal micro_empty_out : STD_LOGIC;

component MicroFifo is
generic (
  ADDRESS_WIDTH : POSITIVE;
  DATA_WIDTH : POSITIVE;
  ALMOST_FULL_COUNT : NATURAL;
  ALMOST_EMPTY_COUNT : NATURAL
);
port (
  clk : in STD_LOGIC;
  rst : in STD_LOGIC;
  data_in : in STD_LOGIC_VECTOR(DATA_WIDTH-1 downto 0);
  valid_in : in STD_LOGIC;
  full_out : out STD_LOGIC;
  data_out : out STD_LOGIC_VECTOR(DATA_WIDTH-1 downto 0);
  read_enable_in : in STD_LOGIC;
  empty_out : out STD_LOGIC
);
end component;


--type state_STATE_TYPE is (S_READY, S_READ, S_PROCESSING) ;
type state_STATE_TYPE is (S_READY, S_READ, S_PROCESSING, S_PROCESSING_LAST) ;
signal state : state_STATE_TYPE ;
signal cur_address : STD_LOGIC_VECTOR(31 downto 0);
signal end_address : STD_LOGIC_VECTOR(31 downto 0);
begin

--used when reading from an external BRAM
--process(clk, rst)
--begin
--  if( rst = '1' ) then
--    burst_address_read_out <= '0';
--	 micro_valid_in <= '0';
--	 micro_data_in <= (others=>'0');
--	 bram_state <= S_EMPTY;
--  elsif( clk'event and clk = '1' ) then
--    burst_address_read_out <= '0';
--	 micro_valid_in <= '0';
--    case bram_state is
--	 when S_EMPTY =>
--	   if( burst_valid_in = '1' and micro_full_out = '0' ) then
--		  burst_address_read_out <= '1';
--		  bram_state <= S_READY;
--		end if;
--	 when S_READY =>
--	   burst_address_read_out <= '1';
--		bram_state <= S_READ;
--	 when S_READ =>
--	   --read
--		micro_valid_in <= '1';
--		micro_data_in(31 downto 0) <= base_address_in;
--		micro_data_in(63 downto 32) <= burst_size_in;
--		if( burst_valid_in = '1' and micro_full_out = '0' ) then
--		  burst_address_read_out <= '1';
--		elsif( micro_full_out = '1' ) then
--		  bram_state <= S_STALL;
--		else
--		  bram_state <= S_EMPTY;
--		end if;
--	 when S_STALL =>
--	   --read
--		micro_valid_in <= '1';
--		micro_data_in(31 downto 0) <= base_address_in;
--		micro_data_in(63 downto 32) <= burst_size_in;
--		bram_state <= S_EMPTY;
--	 end case;
--  end if;
--end process;

micro : MicroFifo
  generic map(
    ADDRESS_WIDTH => 4,
    DATA_WIDTH => 64,
    ALMOST_FULL_COUNT => 3,
    ALMOST_EMPTY_COUNT => 0
  )
  port map(
    clk => clk,
    rst => rst,
	 data_in(31 downto 0) => base_address_in,
	 data_in(63 downto 32) => burst_size_in,
	 valid_in => burst_valid_in,
	 full_out => burst_stall_out,
--used when reading from an external BRAM
--    data_in => micro_data_in,
--    valid_in => micro_valid_in,
--    full_out => micro_full_out,
    data_out(31 downto 0) => base_value,
	 data_out(63 downto 32) => burst_value,
    read_enable_in => micro_read_enable_in,
    empty_out => micro_empty_out
  );

--process(clk, rst)
--begin
--  if( rst = '1' ) then
--	 address_valid_out <= '0';
--	 address_out <= (others=>'0');
--	 cur_address <= (others=>'0');
--	 end_address <= (others=>'0');
--	 micro_read_enable_in <= '0';
--	 state <= S_READY;
--  elsif( clk'event and clk = '1' ) then
--	 address_valid_out <= '0';
--	 micro_read_enable_in <= '0';
--    case state is
--	 when S_READY =>
--	   if( micro_empty_out = '0' ) then
--		  state <= S_READ;
--		  micro_read_enable_in <= '1';
--		end if;
--	 when S_READ =>
--	   state <= S_PROCESSING;
--		--micro_read_enable_in <= '1';
--	   cur_address <= base_value;
--		end_address <= 
--		  ROCCCSUB(
--	       ROCCCADD(
--	         base_value,
--		      burst_value,
--          32),
--		    "00000000000000000000000000000001",
--        32);
--	 when S_PROCESSING =>
--	   if( address_stall_in = '0' ) then
--		  address_valid_out <= '1';
--		  address_out <= cur_address;
--		  if( ROCCC_UGTE(cur_address, end_address, 32) ) then
--		    state <= S_READY;
--			 if( micro_empty_out = '0' ) then
--			   state <= S_READ;
--				micro_read_enable_in <= '1';
--			 end if;
--		  end if;
--		  --else
--		    cur_address <=
--		      ROCCCADD(
--			     cur_address,
--				  "00000000000000000000000000000001",
--				  32);
--		  --end if;
--		end if;
--	 end case;
--  end if;
--end process;

process(clk, rst)
begin
  if( rst = '1' ) then
	 address_valid_out <= '0';
	 address_out <= (others=>'0');
	 cur_address <= (others=>'0');
	 end_address <= (others=>'0');
	 micro_read_enable_in <= '0';
	 state <= S_READY;
  elsif( clk'event and clk = '1' ) then
	 address_valid_out <= '0';
	 micro_read_enable_in <= '0';
    case state is
	 when S_READY =>
	   if( micro_empty_out = '0' ) then
		  state <= S_READ;
		  micro_read_enable_in <= '1';
		end if;
	 when S_READ =>
	   state <= S_PROCESSING;
		--micro_read_enable_in <= '1';
	   cur_address <= base_value;
		end_address <= 
		  ROCCCSUB(
	       ROCCCADD(
	         base_value,
		      burst_value,
          32),
		    "00000000000000000000000000000001",
        32);
	 when S_PROCESSING =>
	   if( address_stall_in = '0' ) then
		  address_valid_out <= '1';
		  address_out <= cur_address;
		  if( cur_address = end_address ) then
		    state <= S_READY;
			 if( micro_empty_out = '0' ) then
			   state <= S_READ;
				micro_read_enable_in <= '1';
			 end if;
		  elsif( ROCCCADD(cur_address,"00000000000000000000000000000001",32) = end_address ) then
		    state <= S_PROCESSING;
			 if( micro_empty_out = '0' ) then
			   state <= S_PROCESSING_LAST;
				micro_read_enable_in <= '1';
			 end if;
		  end if;
		  cur_address <=
		    ROCCCADD(
			   cur_address,
			   "00000000000000000000000000000001",
			 32);
		end if;
	 when S_PROCESSING_LAST =>
	   if( address_stall_in = '0' ) then
		  address_valid_out <= '1';
		  address_out <= cur_address;
	     state <= S_PROCESSING;
		  cur_address <= base_value;
		  end_address <= 
		    ROCCCSUB(
	         ROCCCADD(
	           base_value,
		        burst_value,
            32),
		      "00000000000000000000000000000001",
          32);
		end if;
	 end case;
  end if;
end process;

end Behavioral;


