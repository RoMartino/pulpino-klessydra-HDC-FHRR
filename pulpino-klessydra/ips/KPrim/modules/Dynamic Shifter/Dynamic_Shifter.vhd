----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 20.01.2023 11:33:06
-- Design Name: 
-- Module Name: Dynamic_Shifter - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
--  ██████╗░██╗░░░██╗███╗░░██╗░█████╗░███╗░░░███╗██╗░█████╗░      ░██████╗██╗░░██╗██╗███████╗████████╗███████╗██████╗░
--  ██╔══██╗╚██╗░██╔╝████╗░██║██╔══██╗████╗░████║██║██╔══██╗      ██╔════╝██║░░██║██║██╔════╝╚══██╔══╝██╔════╝██╔══██╗
--  ██║░░██║░╚████╔╝░██╔██╗██║███████║██╔████╔██║██║██║░░╚═╝      ╚█████╗░███████║██║█████╗░░░░░██║░░░█████╗░░██████╔╝
--  ██║░░██║░░╚██╔╝░░██║╚████║██╔══██║██║╚██╔╝██║██║██║░░██╗      ░╚═══██╗██╔══██║██║██╔══╝░░░░░██║░░░██╔══╝░░██╔══██╗
--  ██████╔╝░░░██║░░░██║░╚███║██║░░██║██║░╚═╝░██║██║╚█████╔╝      ██████╔╝██║░░██║██║██║░░░░░░░░██║░░░███████╗██║░░██║
--  ╚═════╝░░░░╚═╝░░░╚═╝░░╚══╝╚═╝░░╚═╝╚═╝░░░░░╚═╝╚═╝░╚════╝░      ╚═════╝░╚═╝░░╚═╝╚═╝╚═╝░░░░░░░░╚═╝░░░╚══════╝╚═╝░░╚═╝
----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;


entity Dynamic_Shifter is
  generic (
    size       : natural;
    size_width : natural
  );
  Port (
    input          : in  STD_LOGIC_VECTOR ((size*2)-1 downto 0);
    shift_amt      : in  unsigned(size_width-1 downto 0);
    shifter_enable : in  STD_LOGIC;
    output         : out STD_LOGIC_VECTOR ((size*2)-1 downto 0)
  );
end Dynamic_Shifter;

architecture Behavioral of Dynamic_Shifter is

  constant allzeros : std_logic_vector(size downto 0) := (others => '0');
begin
  shifter : process(shift_amt,input)
    variable temp : std_logic_vector((size*2)-1 downto 0);
  begin
    -- The shifter is realized as the cascade of three MUX. This reduce area consumption
    temp := input;
    if (shifter_enable = '0') then
      temp := temp;

      if size_width mod 2 = 1 then
        if shift_amt(size_width-1 downto size_width-1) = "1" then
          temp := temp((size*2)-(2**(size_width-1))-1 downto 0) & allzeros((2**(size_width-1))-1 downto 0);
        end if;
      end if;

      for i in 0 to (size_width/2)-1 loop -- size_width/2 was used in this loop because we are handling two bits of shift_amt at a time instead of '1'
        for j in 1 to 3 loop -- index 1 to 3 
          if shift_amt(i*2+1 downto i*2) = j then
            temp := temp ((size*2)-j*(4**i)-1 downto 0) & allzeros(j*(4**i)-1 downto 0);
          end if;
        end loop;
      end loop;

    end if;
    output <= temp;
  end process;

end Behavioral;

