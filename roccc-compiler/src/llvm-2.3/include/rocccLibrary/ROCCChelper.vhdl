-- This contains a bunch of helper functions to get std_logic working.

library IEEE ;
use IEEE.STD_LOGIC_1164.all ;
use IEEE.STD_LOGIC_UNSIGNED.all;
use IEEE.STD_LOGIC_ARITH.all;

package HelperFunctions is

  function ROCCCMUL (L , R : std_logic_vector; size : integer)
    return std_logic_vector;
  function ROCCCSUB (L , R : std_logic_vector; size : integer)
    return std_logic_vector;
  function ROCCCADD (L , R : std_logic_vector; size : integer)
    return std_logic_vector;

  function ROCCCMUL ( L, R : std_logic ; size : integer)
    return std_logic;
  function ROCCCSUB ( L, R : std_logic ; size : integer)
    return std_logic;
  function ROCCCADD ( L, R : std_logic ; size : integer)
    return std_logic;
  
end HelperFunctions ;

package body HelperFunctions is

  function ROCCCMUL (L, R : std_logic_vector; size : integer)
    return std_logic_vector is
    variable LU : unsigned(L'length - 1 downto 0);
    variable RU : unsigned(L'length - 1 downto 0);
    variable bigTemp : unsigned((L'length * 2) - 1 downto 0);
  begin 
    LU := UNSIGNED(L) ;
    RU := UNSIGNED(R) ;
    bigTemp := LU * RU ;
    return std_logic_vector(bigTemp(size - 1 downto 0));
  end ROCCCMUL;
  

  function ROCCCMUL (L, R : std_logic ; size : integer)
    return std_logic is
    variable LV  : std_logic_vector(0 downto 0);
    variable RV  : std_logic_vector(0 downto 0);
    variable LU : unsigned(0 downto 0) ;
    variable RU : unsigned(0 downto 0);
    variable bigTemp : unsigned(1 downto 0);
  begin
    LV(0) := L ;
    RV(0) := R ;
    LU := UNSIGNED(LV) ;
    RU := UNSIGNED(RV) ;
    bigTemp := LU * RU ;
    return std_logic(bigTemp(0)) ;
  end ROCCCMUL ;
    
  function ROCCCADD (L, R : std_logic_vector; size : integer)
    return std_logic_vector is
    variable LU : unsigned(L'length - 1 downto 0);
    variable RU : unsigned(L'length - 1 downto 0);
    variable bigTemp : unsigned(L'length - 1 downto 0);
  begin 
    LU := UNSIGNED(L) ;
    RU := UNSIGNED(R) ;
    bigTemp := LU + RU ;
    return std_logic_vector(bigTemp(size - 1 downto 0));
  end ROCCCADD;
  

  function ROCCCADD (L, R : std_logic ; size : integer)
    return std_logic is
    variable LV  : std_logic_vector(0 downto 0);
    variable RV  : std_logic_vector(0 downto 0);
    variable LU : unsigned(0 downto 0) ;
    variable RU : unsigned(0 downto 0);
    variable bigTemp : unsigned(0 downto 0);
  begin
    LV(0) := L ;
    RV(0) := R ;
    LU := UNSIGNED(LV) ;
    RU := UNSIGNED(RV) ;
    bigTemp := LU + RU ;
    return std_logic(bigTemp(0)) ;
  end ROCCCADD ;

    
  function ROCCCSUB (L, R : std_logic_vector; size : integer)
    return std_logic_vector is
    variable LU : unsigned(L'length - 1 downto 0);
    variable RU : unsigned(L'length - 1 downto 0);
    variable bigTemp : unsigned(L'length - 1 downto 0);
  begin 
    LU := UNSIGNED(L) ;
    RU := UNSIGNED(R) ;
    bigTemp := LU - RU ;
    return std_logic_vector(bigTemp(size - 1 downto 0));
  end ROCCCSUB;

  function ROCCCSUB (L, R : std_logic ; size : integer)
    return std_logic is
    variable LV  : std_logic_vector(0 downto 0);
    variable RV  : std_logic_vector(0 downto 0);
    variable LU : unsigned(0 downto 0) ;
    variable RU : unsigned(0 downto 0);
    variable bigTemp : unsigned(0 downto 0);
  begin
    LV(0) := L ;
    RV(0) := R ;
    LU := UNSIGNED(LV) ;
    RU := UNSIGNED(RV) ;
    bigTemp := LU - RU ;
    return std_logic(bigTemp(0)) ;
  end ROCCCSUB ;  
  
end HelperFunctions;

-- This contains a bunch of helper functions to get std_logic working.

library IEEE ;
use IEEE.STD_LOGIC_1164.all ;
use IEEE.STD_LOGIC_UNSIGNED.all;
use IEEE.STD_LOGIC_ARITH.all;

package HelperFunctions_Unsigned is
  function ROCCC_ULT(L, R : std_logic_vector; size : integer)
    return boolean;
  function ROCCC_UGT(L, R : std_logic_vector; size : integer)
    return boolean;
  function ROCCC_ULTE(L, R : std_logic_vector; size : integer)
    return boolean;
  function ROCCC_UGTE(L, R : std_logic_vector; size : integer)
    return boolean;
                                   
  function ROCCC_ULT(L, R : std_logic; size : integer)
    return boolean;
  function ROCCC_UGT(L, R : std_logic; size : integer)
    return boolean;
  function ROCCC_ULTE(L, R : std_logic; size : integer)
    return boolean;
  function ROCCC_UGTE(L, R : std_logic; size : integer)
    return boolean;

end HelperFunctions_Unsigned ;

package body HelperFunctions_Unsigned is

  function ROCCC_ULT (L, R : std_logic_vector; size : integer)
    return boolean is
  begin 
    return (L < R);
  end ROCCC_ULT;
  

  function ROCCC_ULT(L, R : std_logic ; size : integer)
    return boolean is
  begin
    return (L < R) ;
  end ROCCC_ULT ;
    
  function ROCCC_UGT (L, R : std_logic_vector; size : integer)
    return boolean is
  begin 
    return (L > R);
  end ROCCC_UGT ;

  function ROCCC_UGT (L, R : std_logic; size : integer)
    return boolean is
  begin 
    return (L > R);
  end ROCCC_UGT;  
    
  function ROCCC_ULTE (L, R : std_logic_vector; size : integer)
    return boolean is
  begin 
    return (L <= R);
  end ROCCC_ULTE;

  function ROCCC_ULTE (L, R : std_logic; size : integer)
    return boolean is
  begin 
    return (L <= R);
  end ROCCC_ULTE;
  
  function ROCCC_UGTE (L, R : std_logic_vector; size : integer)
    return boolean is
  begin 
    return (L >= R);
  end ROCCC_UGTE;

  function ROCCC_UGTE (L, R : std_logic; size : integer)
    return boolean is
  begin 
    return (L >= R);
  end ROCCC_UGTE;
  
end HelperFunctions_Unsigned;

-- This contains a bunch of helper functions to get std_logic working.

library IEEE ;
use IEEE.STD_LOGIC_1164.all ;
use IEEE.STD_LOGIC_SIGNED.all;
use IEEE.STD_LOGIC_ARITH.all;

package HelperFunctions_Signed is
  function ROCCC_SLT(L, R : std_logic_vector; size : integer)
    return boolean;
  function ROCCC_SGT(L, R : std_logic_vector; size : integer)
    return boolean;
  function ROCCC_SLTE(L, R : std_logic_vector; size : integer)
    return boolean;
  function ROCCC_SGTE(L, R : std_logic_vector; size : integer)
    return boolean;

  function ROCCC_SLT(L, R : std_logic; size : integer)
    return boolean;
  function ROCCC_SGT(L, R : std_logic; size : integer)
    return boolean;
  function ROCCC_SLTE(L, R : std_logic; size : integer)
    return boolean;
  function ROCCC_SGTE(L, R : std_logic; size : integer)
    return boolean;
end HelperFunctions_Signed ;

package body HelperFunctions_Signed is

  function ROCCC_SLT (L, R : std_logic_vector; size : integer)
    return boolean is
  begin 
    return (L < R);
  end ROCCC_SLT;

  function ROCCC_SLT (L, R : std_logic; size : integer)
    return boolean is
  begin 
    return (L < R);
  end ROCCC_SLT;
  
  function ROCCC_SGT (L, R : std_logic_vector; size : integer)
    return boolean is
  begin 
    return (L > R);
  end ROCCC_SGT;

  function ROCCC_SGT (L, R : std_logic; size : integer)
    return boolean is
  begin 
    return (L > R);
  end ROCCC_SGT;

  function ROCCC_SLTE (L, R : std_logic_vector; size : integer)
    return boolean is
  begin 
    return (L <= R);
  end ROCCC_SLTE;
  
  function ROCCC_SLTE (L, R : std_logic; size : integer)
    return boolean is
  begin 
    return (L <= R);
  end ROCCC_SLTE;
  
  function ROCCC_SGTE (L, R : std_logic_vector; size : integer)
    return boolean is
  begin 
    return (L >= R);
  end ROCCC_SGTE;

  function ROCCC_SGTE (L, R : std_logic; size : integer)
    return boolean is
  begin 
    return (L >= R);
  end ROCCC_SGTE;
  
end HelperFunctions_Signed;
