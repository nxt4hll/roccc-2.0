-- This VHDL implementation of a BRAM fifo is from www.asic-world.com

-------------------------------------------------------------------------------
-- Function : Asynchronous FIFO (w/ 2 asynchronous clocks).
-- Coder    : Alex Claros F.
-- Date     : 15/May/2005.
-- Notes    : This implementation is based on the article
--           'Asynchronous FIFO in Virtex-II FPGAs'
--            writen by Peter Alfke. This TechXclusive
--            article can be downloaded from the
--            Xilinx website. It has some minor modifications.
-- Coder     : Deepak Kumar Tala (Verilog)
-- Translator: Alexander H Pham (VHDL)
-------------------------------------------------------------------------------

-- Grey counter definition
library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_UNSIGNED.all;
use IEEE.STD_LOGIC_ARITH.all;

entity GrayCounter is
  generic (
            COUNTER_WIDTH : integer
          ) ;
  port (
         GrayCount_out : out STD_LOGIC_VECTOR(COUNTER_WIDTH-1 downto 0) ;
         Enable_in     : in  STD_LOGIC ;
         Clear_in      : in  STD_LOGIC ;
         clk           : in  STD_LOGIC 
       ) ;
end GrayCounter;

architecture handwritten of GrayCounter is
  signal BinaryCount : std_logic_vector(COUNTER_WIDTH-1 downto 0);
begin  
  process(clk)
  begin
    if (clk'event and clk='1') then
      if (Clear_in = '1') then
        BinaryCount <= conv_std_logic_vector(1, COUNTER_WIDTH) ;
        GrayCount_out <= (others => '0') ;
      elsif (Enable_in = '1') then
        BinaryCount <= BinaryCount + 1 ;
        GrayCount_out <= BinaryCount(COUNTER_WIDTH-1) &
                         (BinaryCount(COUNTER_WIDTH-2 downto 0) xor
                          BinaryCount(COUNTER_WIDTH-1 downto 1)) ;
      end if;
    end if;
  end process ;
  
end handwritten;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
    
entity InferredBRAMFifo is
    generic (
        DATA_WIDTH :integer;
        DATA_DEPTH :integer
    );
    port (
        -- Reading port.
        Data_out    :out std_logic_vector (DATA_WIDTH-1 downto 0);
        Empty_out   :out std_logic;
        ReadEn_in   :in  std_logic;
        RClk        :in  std_logic;
        -- Writing port.
        Data_in     :in  std_logic_vector (DATA_WIDTH-1 downto 0);
        Full_out    :out std_logic;
        WriteEn_in  :in  std_logic;
        WClk        :in  std_logic;
	 
        Clear_in:in  std_logic
    );
end entity;
architecture rtl of InferredBRAMFifo is
    --function to discern internal address size from data depth
    function log2_ceil(N : integer) return integer is
	 begin
	   if( N <= 2 ) then
		  return 1;
		else
		  if( N mod 2 = 0 ) then
		    return 1 + log2_ceil(N/2);
		  else
		    return 1 + log2_ceil((N+1)/2);
		  end if;
		end if;
	 end function log2_ceil;
    ----/Internal connections & variables------
	 constant ADDR_WIDTH :integer := log2_ceil(DATA_DEPTH);

    type RAM is array (integer range <>)of std_logic_vector (DATA_WIDTH-1 downto 0);
    signal Mem : RAM (0 to DATA_DEPTH-1);
    attribute syn_ramstyle : string;
    attribute syn_ramstyle of Mem : signal is "block_ram";
    
    signal pNextWordToWrite     :std_logic_vector (ADDR_WIDTH-1 downto 0);
    signal pNextWordToRead      :std_logic_vector (ADDR_WIDTH-1 downto 0);
    signal EqualAddresses       :std_logic;
    signal NextWriteAddressEn   :std_logic;
    signal NextReadAddressEn    :std_logic;
    signal Set_Status           :std_logic;
    signal Rst_Status           :std_logic;
    signal Status               :std_logic;
    signal PresetFull           :std_logic;
    signal PresetEmpty          :std_logic;
    signal empty,full           :std_logic;
    
    component GrayCounter is
    generic (
        COUNTER_WIDTH :integer
    );
    port (
        GrayCount_out :out std_logic_vector (COUNTER_WIDTH-1 downto 0);
        Enable_in     :in  std_logic;  --Count enable.
        Clear_in      :in  std_logic;  --Count reset.
        clk           :in  std_logic
    );
    end component;
 
begin

    --------------Code--------------/
    --Data ports logic:
    --(Uses a dual-port RAM).
    --'Data_out' logic:
    process (RClk, Clear_in) begin
	     if( Clear_in = '1' ) then
		    Data_out <= (others=>'0');
		  elsif (rising_edge(RClk)) then
            if (ReadEn_in = '1' and empty = '0') then
                Data_out <= Mem(conv_integer(pNextWordToRead));
            end if;
        end if;
    end process;
            
    --'Data_in' logic:
    process (WClk) begin
        if (rising_edge(WClk)) then
            if (WriteEn_in = '1' and full = '0') then
                Mem(conv_integer(pNextWordToWrite)) <= Data_in;
            end if;
        end if;
    end process;

    --Fifo addresses support logic: 
    --'Next Addresses' enable logic:
    NextWriteAddressEn <= WriteEn_in and (not full);
    NextReadAddressEn  <= ReadEn_in  and (not empty);
           
    --Addreses (Gray counters) logic:
    GrayCounter_pWr : GrayCounter
	 generic map (
	   COUNTER_WIDTH => ADDR_WIDTH
	 )
    port map (
        GrayCount_out => pNextWordToWrite,
        Enable_in     => NextWriteAddressEn,
        Clear_in      => Clear_in,
        clk           => WClk
    );
       
    GrayCounter_pRd : GrayCounter
	 generic map (
	   COUNTER_WIDTH => ADDR_WIDTH
	 )
    port map (
        GrayCount_out => pNextWordToRead,
        Enable_in     => NextReadAddressEn,
        Clear_in      => Clear_in,
        clk           => RClk
    );

    --'EqualAddresses' logic:
    EqualAddresses <= '1' when (pNextWordToWrite = pNextWordToRead) else '0';

    --'Quadrant selectors' logic:
    process (pNextWordToWrite, pNextWordToRead)
        variable set_status_bit0 :std_logic;
        variable set_status_bit1 :std_logic;
        variable rst_status_bit0 :std_logic;
        variable rst_status_bit1 :std_logic;
    begin
        set_status_bit0 := pNextWordToWrite(ADDR_WIDTH-2) xnor pNextWordToRead(ADDR_WIDTH-1);
        set_status_bit1 := pNextWordToWrite(ADDR_WIDTH-1) xor  pNextWordToRead(ADDR_WIDTH-2);
        Set_Status <= set_status_bit0 and set_status_bit1;
        
        rst_status_bit0 := pNextWordToWrite(ADDR_WIDTH-2) xor  pNextWordToRead(ADDR_WIDTH-1);
        rst_status_bit1 := pNextWordToWrite(ADDR_WIDTH-1) xnor pNextWordToRead(ADDR_WIDTH-2);
        Rst_Status      <= rst_status_bit0 and rst_status_bit1;
    end process;
    
    --'Status' latch logic:
    process (Set_Status, Rst_Status, Clear_in) begin--D Latch w/ Asynchronous Clear & Preset.
        if (Rst_Status = '1' or Clear_in = '1') then
            Status <= '0';  --Going 'Empty'.
        elsif (Set_Status = '1') then
            Status <= '1';  --Going 'Full'.
        end if;
    end process;
    
    --'Full_out' logic for the writing port:
    PresetFull <= Status and EqualAddresses;  --'Full' Fifo.
    
    process (WClk, PresetFull) begin --D Flip-Flop w/ Asynchronous Preset.
        if (PresetFull = '1') then
            full <= '1';
        elsif (rising_edge(WClk)) then
            full <= '0';
        end if;
    end process;
    Full_out <= full;
    
    --'Empty_out' logic for the reading port:
    PresetEmpty <= not Status and EqualAddresses;  --'Empty' Fifo.
    
    process (RClk, PresetEmpty) begin --D Flip-Flop w/ Asynchronous Preset.
        if (PresetEmpty = '1') then
            empty <= '1';
        elsif (rising_edge(RClk)) then
            empty <= '0';
        end if;
    end process;
    
    Empty_out <= empty;
end architecture;

