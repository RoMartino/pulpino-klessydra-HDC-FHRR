
-- ieee packages ------------
library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- Square root Newton Raphson
entity sqrt is
  generic (
    sqrt_implementation : natural := 0;
    size                : natural := 32  -- Default to 32 bits, can be adjusted as needed
  );
  Port (
    clk_i    : in  std_logic;
    rst_ni   : in  std_logic;
    start    : in  std_logic;
    number   : in  std_logic_vector(size-1 downto 0); -- Input number as std_logic_vector
    sqrt_res : out std_logic_vector(size-1 downto 0); -- Output square root as std_logic_vector
    busy     : out std_logic;
    ready    : out std_logic
  );
end sqrt;

architecture behavioral of sqrt is

  -- Square root Newton Raphson
  component sqrt_nr is
    generic (
      sqrt_implementation : natural := 0;
      size                : INTEGER := 32  -- Default to 32 bits, can be adjusted as needed
    );
    port (
      clk_i    : in  std_logic;
      rst_ni   : in  std_logic;
      start    : in  std_logic;
      number   : in  std_logic_vector(size-1 downto 0); -- Input number as std_logic_vector
      sqrt_res : out std_logic_vector(size-1 downto 0); -- Output square root as std_logic_vector
      busy     : out std_logic;
      ready    : out std_logic
    );
  end component;

begin

  sqrt_nr_gen : if (sqrt_implementation = 0) generate
  -- Square root Newton Raphson
  sqrt_nr_inst : sqrt_nr
    generic map(
      size     => size
    )
    port map(
      clk_i    => clk_i,
      rst_ni   => rst_ni,
      start    => start,
      number   => number,
      sqrt_res => sqrt_res,
      busy     => busy,
      ready    => ready
    );
  end generate;

end architecture behavioral;
