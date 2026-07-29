
-- ieee packages ------------
library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

-- Square root Newton Raphson
entity sqrt_nr is
  generic (
    size : natural := 32  -- Default to 32 bits, can be adjusted as needed
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
end sqrt_nr;

architecture Behavioral of sqrt_nr is

  constant size_width : integer := integer(ceil(log2(real(size))));

  type STATE_TYPE is (IDLE, CALCULATE);

  signal state       : STATE_TYPE := IDLE;
  signal root        : unsigned(size-1 downto 0);
  signal radicand    : unsigned(size-1 downto 0);
  signal num         : unsigned(size-1 downto 0) := (others => '0');
  signal x           : unsigned(size-1 downto 0) := (others => '0');
  signal x_2         : unsigned((size*2)-1 downto 0) := (others => '0'); -- This signal represents x^2
  signal xp1_2       : unsigned((size*2)-1 downto 0) := (others => '0'); -- This signal represents (x+1)^2
  signal x_temp      : unsigned(size downto 0) := (others => '0');
  signal convergence : std_logic;

  signal clz     : std_logic_vector(size_width downto 0);
  signal index   : integer range 0 to size-1;

  signal div_finished   : std_logic;
  signal result_div     : std_logic_vector(size-1 downto 0);
  signal div_enable     : std_logic;

  component CLZ_top is
    generic (
      size       : natural;
      size_width : integer
    );
    Port (
      input  : in  std_logic_vector(size-1 downto 0);
      output : out std_logic_vector(size_width downto 0)
    );
  end component;

  component divider is
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
      result_rem            : out std_logic_vector(size-1 downto 0)  -- Decomment if you want to see the result
    );
  end component;

begin

  divider_inst : divider
    generic map(
      divider_implementation => 5,
      size                   => size
    )
    port map(
      reset        => not rst_ni,
      clk          => clk_i,
      dividend_i   => std_logic_vector(number),
      divisor_i    => std_logic_vector(root),
      div_enable   => (start and not convergence) or div_enable,
      div_finished => div_finished,
      result_div   => result_div,
      result_rem   => open
    );

  CLZ_Inst : CLZ_top
    generic map(
      size       => size,
      size_width => size_width
    )
    port map (
      input  => number,
      output => clz
    );

  index  <= (size-1-(to_integer(unsigned(clz))-1))/2;

  ready    <= '0' when start = '1' or busy = '1' else '1';
  x_temp   <= '0' & (x + unsigned(result_div)) when div_finished else x & '0';
  root     <= (index+1 to size-1 => '0') & '1' & (0 to index-1 => '0') when start else x;
  radicand <= unsigned(number) when start else num;
  x_2      <= root*root;
  xp1_2    <= (root+1)*(root+1);
  convergence <= '1' when (x_2(size-1 downto 0) <= radicand) and (xp1_2(size-1 downto 0) >= radicand) else '0';

  process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then
      state      <= IDLE;
      num        <= (others => '0');
      x          <= (others => '0');
      sqrt_res   <= (others => '0');
      busy       <= '0';
      div_enable <= '0';
    elsif rising_edge(clk_i) then
      case state is
        when IDLE =>
          div_enable <= '0';
          x <= (others => '0');
          if convergence then
            sqrt_res   <= std_logic_vector(root);
            if (xp1_2(size-1 downto 0) = radicand) then
              sqrt_res <= std_logic_vector(root+1);
            end if;
            busy  <= '0';
            state <= IDLE;
          elsif start = '1' then
            num      <= unsigned(number);
            x(index) <= '1';
            busy     <= '1';
            state    <= CALCULATE;
          end if;

        when CALCULATE =>
          -- Check for convergence
          if (convergence and div_finished) then
            sqrt_res   <= std_logic_vector(root);
            if (xp1_2(size-1 downto 0) = radicand) then
              sqrt_res <= std_logic_vector(root+1);
            end if;
            busy  <= '0';
            state <= IDLE;
          else
            div_enable <= div_finished;
            x     <= x_temp(size downto 1); -- division by 2 for newton raphson is done when "x_temp" is assigned to "x"
            state <= CALCULATE; -- Continue iterating
          end if;

        when others =>
          state <= IDLE;
      end case;
    end if;
  end process;

end architecture Behavioral;
