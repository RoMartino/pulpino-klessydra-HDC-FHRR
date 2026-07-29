library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

-- Divider Implementations:
-- '0' Combinational Divider
-- '1' Restoring Divider
-- '2' Var Latency divider (Standard)
-- '3' Var Latency divider (Limited Shifter)
-- '4' Var Latency divider (High Performance)
-- '5' Var Latency divider (High Frequency)

entity divider is
  generic (
    divider_implementation : natural := 5;
    size                   : natural := 32
  );
  Port (
    reset                 : in  std_logic;
    clk                   : in  std_logic;
    dividend_i            : in  std_logic_vector(size-1 downto 0);
    divisor_i             : in  std_logic_vector(size-1 downto 0);
    div_enable            : in  std_logic;
    div_finished          : out std_logic;
    result_div            : out std_logic_vector(size-1 downto 0); -- Decomment if you want to see the result
    result_rem            : out std_logic_vector(size-1 downto 0) -- Decomment if you want to see the result
  );
end divider;

architecture Behavioral of divider is

signal result : std_logic_vector((size*2)-1 downto 0); -- Decomment if you want to see the result

component divider_HF is
  generic (
    size : natural := 32
  );
  port (
    dividend_i            : in  std_logic_vector(size-1 downto 0);
    divisor_i             : in  std_logic_vector(size-1 downto 0);
    reset                 : in  std_logic;
    clk                   : in  std_logic;
    div_enable_i          : in  std_logic;
    division_finished_out : out std_logic;
    result                : out std_logic_vector((size*2)-1 downto 0) -- Decomment if you want to see the result
  );
end component divider_HF;

begin

  result_div <= result(size-1 downto 0);
  result_rem <= result((2*size)-1 downto size);


  COMB_DIV : if divider_implementation = 0 generate

    div_finished <= div_enable;

    process(all)
    begin
      result(size-1 downto 0)        <= std_logic_vector(unsigned(dividend_i) / unsigned(divisor_i));
      result((2*size)-1 downto size) <= std_logic_vector(unsigned(dividend_i) mod unsigned(divisor_i));
    end process;

  end generate COMB_DIV;

  VAR_LAT_DIV_HF : if divider_implementation = 5 generate

    divider_HF_inst : divider_HF
      generic map(
        size => size
      )
      port map(
        dividend_i            => dividend_i,
        divisor_i             => divisor_i,
        reset                 => reset,
        clk                   => clk,
        div_enable_i          => div_enable,
        division_finished_out => div_finished,
        result                => result
      );

  end generate VAR_LAT_DIV_HF;

end Behavioral;