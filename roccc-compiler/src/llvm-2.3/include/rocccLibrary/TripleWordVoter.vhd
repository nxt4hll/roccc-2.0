library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

---- Uncomment the following library declaration if instantiating
---- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity TripleWordVoter is
port(
  clk : in STD_LOGIC;
  rst : in STD_LOGIC;
  inputReady : in STD_LOGIC;
  outputReady : out STD_LOGIC;
  done : out STD_LOGIC;
  stall : in STD_LOGIC;
  error : out STD_LOGIC;
  val0_in : in STD_LOGIC_VECTOR(31 downto 0);
  val1_in : in STD_LOGIC_VECTOR(31 downto 0);
  val2_in : in STD_LOGIC_VECTOR(31 downto 0);
  val0_out : out STD_LOGIC_VECTOR(31 downto 0);
  val1_out : out STD_LOGIC_VECTOR(31 downto 0);
  val2_out : out STD_LOGIC_VECTOR(31 downto 0)
);
end TripleWordVoter;

architecture Behavioral of TripleWordVoter is
signal error0 : std_logic;
signal error1 : std_logic;
signal error2 : std_logic;
begin

error <= '1' when (error0 = '1' and error1 = '1') or (error1 = '1' and error2 = '1') or (error2 = '1' and error0 = '1') else
         '0';

process(clk, rst)
begin
  if( rst = '1' ) then
  elsif( clk'event and clk = '1' ) then
    val0_out <= (others=>'0');
	 error0 <= '1';
    if( val0_in = val1_in ) then
	   val0_out <= val0_in;
		error0 <= '0';
	 elsif( val1_in = val2_in ) then
	   val0_out <= val1_in;
		error0 <= '0';
	 elsif( val2_in = val0_in ) then
	   val0_out <= val2_in;
		error0 <= '0';
	 end if;
  end if;
end process;

process(clk, rst)
begin
  if( rst = '1' ) then
  elsif( clk'event and clk = '1' ) then
    val1_out <= (others=>'0');
	 error1 <= '1';
    if( val0_in = val1_in ) then
	   val1_out <= val0_in;
		error1 <= '0';
	 elsif( val1_in = val2_in ) then
	   val1_out <= val1_in;
		error1 <= '0';
	 elsif( val2_in = val0_in ) then
	   val1_out <= val2_in;
		error1 <= '0';
	 end if;
  end if;
end process;

process(clk, rst)
begin
  if( rst = '1' ) then
  elsif( clk'event and clk = '1' ) then
    val2_out <= (others=>'0');
	 error2 <= '1';
    if( val0_in = val1_in ) then
	   val2_out <= val0_in;
		error2 <= '0';
	 elsif( val1_in = val2_in ) then
	   val2_out <= val1_in;
		error2 <= '0';
	 elsif( val2_in = val0_in ) then
	   val2_out <= val2_in;
		error2 <= '0';
	 end if;
  end if;
end process;

end Behavioral;

