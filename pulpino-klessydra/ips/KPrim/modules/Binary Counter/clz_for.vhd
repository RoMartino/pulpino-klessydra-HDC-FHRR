---------------------------
---------------------------
--░█████╗░██╗░░░░░███████╗
--██╔══██╗██║░░░░░╚════██║
--██║░░╚═╝██║░░░░░░░███╔═╝
--██║░░██╗██║░░░░░██╔══╝░░
--╚█████╔╝███████╗███████╗
--░╚════╝░╚══════╝╚══════╝
---------------------------
---------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use ieee.math_real.all;

entity CLZ_for is
    generic (
      size       : natural := 32;
      size_width : integer := integer(ceil(log2(real(size))))
    );
    Port (
      input  : in  std_logic_vector(size-1 downto 0);
      output : out std_logic_vector(size_width downto 0)
    );
end CLZ_for;

architecture Behavioral of CLZ_for is

    function count_leading_zeros(input : std_logic_vector) return integer is
        variable zero_count : integer := 0;
    begin
        for i in input'range loop
            if input(i) = '0' then
                zero_count := zero_count + 1;
            else
                exit;
            end if;
        end loop;
        return zero_count;
    end function;

    signal zero_count_internal : integer;

begin

    zero_count_internal <= count_leading_zeros(input);
    output <= std_logic_vector(to_unsigned(zero_count_internal, size_width+1));

end Behavioral;

