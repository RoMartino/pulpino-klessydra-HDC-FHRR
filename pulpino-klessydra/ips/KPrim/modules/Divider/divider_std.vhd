-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
--██╗░░░██╗░█████╗░██████╗░██╗░█████╗░██████╗░██╗░░░░░███████╗
--██║░░░██║██╔══██╗██╔══██╗██║██╔══██╗██╔══██╗██║░░░░░██╔════╝
--╚██╗░██╔╝███████║██████╔╝██║███████║██████╦╝██║░░░░░█████╗░░
--░╚████╔╝░██╔══██║██╔══██╗██║██╔══██║██╔══██╗██║░░░░░██╔══╝░░
--░░╚██╔╝░░██║░░██║██║░░██║██║██║░░██║██████╦╝███████╗███████╗
--░░░╚═╝░░░╚═╝░░╚═╝╚═╝░░╚═╝╚═╝╚═╝░░╚═╝╚═════╝░╚══════╝╚══════╝
--
--██╗░░░░░░█████╗░████████╗███████╗███╗░░██╗░█████╗░██╗░░░██╗         ██████╗░██╗██╗░░░██╗██╗██████╗░███████╗██████╗░
--██║░░░░░██╔══██╗╚══██╔══╝██╔════╝████╗░██║██╔══██╗╚██╗░██╔╝         ██╔══██╗██║██║░░░██║██║██╔══██╗██╔════╝██╔══██╗
--██║░░░░░███████║░░░██║░░░█████╗░░██╔██╗██║██║░░╚═╝░╚████╔╝░         ██║░░██║██║╚██╗░██╔╝██║██║░░██║█████╗░░██████╔╝
--██║░░░░░██╔══██║░░░██║░░░██╔══╝░░██║╚████║██║░░██╗░░╚██╔╝░░         ██║░░██║██║░╚████╔╝░██║██║░░██║██╔══╝░░██╔══██╗
--███████╗██║░░██║░░░██║░░░███████╗██║░╚███║╚█████╔╝░░░██║░░░         ██████╔╝██║░░╚██╔╝░░██║██████╔╝███████╗██║░░██║
--╚══════╝╚═╝░░╚═╝░░░╚═╝░░░╚══════╝╚═╝░░╚══╝░╚════╝░░░░╚═╝░░░         ╚═════╝░╚═╝░░░╚═╝░░░╚═╝╚═════╝░╚══════╝╚═╝░░╚═╝
--
--░██████╗████████╗░█████╗░███╗░░██╗██████╗░░█████╗░██████╗░██████╗░
--██╔════╝╚══██╔══╝██╔══██╗████╗░██║██╔══██╗██╔══██╗██╔══██╗██╔══██╗
--╚█████╗░░░░██║░░░███████║██╔██╗██║██║░░██║███████║██████╔╝██║░░██║
--░╚═══██╗░░░██║░░░██╔══██║██║╚████║██║░░██║██╔══██║██╔══██╗██║░░██║
--██████╔╝░░░██║░░░██║░░██║██║░╚███║██████╔╝██║░░██║██║░░██║██████╔╝
--╚═════╝░░░░╚═╝░░░╚═╝░░╚═╝╚═╝░░╚══╝╚═════╝░╚═╝░░╚═╝╚═╝░░╚═╝╚═════╝░
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;

-- The divider unit take as input: a dividend, a divisor_reg and an enable signal.
-- All these inputs are stored in registered in the first clock cycle. 
-- The division starts from the second clock cycles.
-- The division result is ready when the output signal 
-- division_finished_out is high. The result is available in the remainder reg.
entity divider_std is
    Port ( dividend_i               : in  STD_LOGIC_VECTOR(31 downto 0);
           divisor_i                : in  STD_LOGIC_VECTOR(31 downto 0);
           reset                    : in  STD_LOGIC;
           clk                      : in  STD_LOGIC;
           div_enable_i             : in  STD_LOGIC;
           division_finished_out    : out STD_LOGIC;
           result                   : out std_logic_vector(63 downto 0)       -- Decomment if you want to see the result
           );
end divider_std;

architecture Behavioral of divider_std is
component CLZ_64
    Port (  input : in  STD_LOGIC_VECTOR(63 downto 0);
            output: out STD_LOGIC_VECTOR(5 downto 0)
           );
end component;

component CLZ_32
    Port (  input : in  STD_LOGIC_VECTOR(31 downto 0);
            output: out STD_LOGIC_VECTOR(4 downto 0)
           );
end component;

component CLZ
    Port (  input : in  STD_LOGIC_VECTOR(31 downto 0);
            output: out STD_LOGIC_VECTOR(4 downto 0)
           );
end component;

component Dynamic_Shifter
    Port (  input : in STD_LOGIC_VECTOR (63 downto 0);
           shift_amt : in unsigned(4 downto 0);
           output : out STD_LOGIC_VECTOR (63 downto 0)
           );
end component;

-- Divider signals
signal divisor_reg          : std_logic_vector(31 downto 0);                --  Divisor register. 
signal R                    : std_logic_vector(63 downto 0);                -- Remainder register. 64 bit, in the Most Significant Word (MSW) we have the remainder, in the LSW the quotient. Note that in the first clock cycle the remainder is initialized with the dividend in its LSW.
signal div_enable_reg       : std_logic;                                    --  Enable signal register. 
signal dividend_wire        : std_logic_vector(31 downto 0);                --  Dividend Input Register
signal divisor_wire         : std_logic_vector(31 downto 0);                --  Divisor Input Register
signal div_enable_wire      : std_logic;                        
signal count                : integer;                                      -- Counter register. The counter shows the remaining division steps.
signal count_wire           : integer;                                      -- Counter wire. Counter register input.
signal remaining_shifts     : integer;                                      -- Counter wire. Counter register input.
signal S                    : std_logic_vector(32 downto 0);                -- Difference signal. Output of the subtractor unit 1 (RemainderMSW - divisor_reg)

-- Count Leading Zeros units
signal clz_divisor          : integer;                                      -- Output of the 32 bits Count Leading Zero unit used to detect the divisor_reg MSOne.
signal clz_divisor_wire     : integer;                                      -- Output of the 32 bits Count Leading Zero unit used to detect the divisor_reg MSOne.
signal clz_remainder        : integer;                                      -- Output of the 64 bits Count Leading Zero unit used to detect the Remainder MSOne.
signal clz_remainder_wire   : integer;                                      -- Output of the 64 bits Count Leading Zero unit used to detect the Remainder MSOne.
signal shift                : integer;                                      -- Difference signal. Output of the subtractor unit 2 (clz_remainder - clz_divisor - 1)
signal shift_amt            : unsigned(4 downto 0);                         -- Difference signal. Output of the subtractor unit 2 (clz_remainder - clz_divisor - 1)
signal clz_rem_in           : std_logic_vector(63 downto 0);                -- CLZ64 input signal
signal clz_remainder_vec    : std_logic_vector(5 downto 0);                 -- CLZ64 output
signal clz_divisor_vec      : std_logic_vector(4 downto 0);                 -- CLZ32 output

-- Shifter
signal shifted_R            : std_logic_vector(63 downto 0);                -- Signal containing the remainder shifted dynamically. Output of the dynamic shifter
signal new_R                : std_logic_vector(63 downto 0);                -- Signal containing the remainder shifted dynamically. Output of the dynamic shifter
signal shifter_enable       : std_logic;                                    -- Difference signal. Output of the subtractor unit 2 (clz_remainder - clz_divisor - 1)
signal limited_shift        : std_logic;                                    -- Difference signal. Output of the subtractor unit 2 (clz_remainder - clz_divisor - 1)
signal division_finished_wire:std_logic;
attribute dont_touch           : string;    
attribute dont_touch of R : signal is "true";                               -- Comment if the division result is an output
constant allzeros           : std_logic_vector(32 downto 0) := (others=>'0'); 
begin

CLZ_64_Bit: CLZ_64 port map( input => R, output => clz_remainder_vec);              -- CLZ-64: used for R register
--CLZ_32_Bit: CLZ_32 port map( input => divisor_wire, output => clz_divisor_vec);     -- CLZ-32: used for R register
CLZ_32_Bit: CLZ port map( input => divisor_wire, output => clz_divisor_vec);     -- CLZ-32: used for R register

Dyn_shifter: Dynamic_Shifter port map( input => R, shift_amt=> shift_amt, output => shifted_R); -- Dynamic shifter: three multiplexers
divisor_wire      <= divisor_i;
dividend_wire     <= dividend_i;
div_enable_wire   <= div_enable_i;

-------------------------------------------------------------------------------------
-- DIVISOR LEADING ZERO COUNTER 32 BIT DATA
clz_divisor_wire  <= to_integer(unsigned(clz_divisor_vec));

-- REMAINDER LEADING ZERO COUNTER 64 BIT DATA 
clz_remainder_wire  <= to_integer(unsigned(clz_remainder_vec));

-- SUBTRACTOR
S   <= std_logic_vector(('0' & unsigned(shifted_R(62 downto 31))) - ('0' & unsigned(divisor_reg))); 


-------------------------------------------------------------------------------------
------------------------------------- COUNTER ---------------------------------------
-------------------------------------------------------------------------------------
counter_handler:process(all)
begin
    count_wire        <= count;
    division_finished_wire<='0';
    -- The counter is enabled (incremented) only when the division is started
    if div_enable_reg='1' then
        count_wire <= count + 1;

        -- If the Shifter is enabled, then dynamic shift is performed and the counter is updated by shift_amount
        if (shifter_enable='1') then
                count_wire<= count + shift +1;
        end if;

        -- When the counter reach 32, the division is completed: division_finished = '1' and the result is available in remainder
        if count_wire=32 or limited_shift='1' then
            division_finished_wire<='1';            
        end if;
    end if;
end process;

-------------------------------------------------------------------------------------
----------------------------------- SHIFTER -----------------------------------------
-------------------------------------------------------------------------------------
shifter_control: process(all)
begin
    shifter_enable   <= '0'; 
    limited_shift    <= '0';
    shift            <= clz_remainder_wire-clz_divisor-1;  
    shift_amt        <= to_unsigned(shift, shift_amt'length);

    -- If Remainder LZs > Divisor LZs, the shifter is enabled. 
    if (shift>0) then 
        shifter_enable <= '1';
        -- Shift amount is limited to 32
        if shift >  (31-count) then
            shift_amt       <= to_unsigned((31 - count), shift_amt'length);
            limited_shift   <='1';
        end if;
   else
        shift_amt        <= "00000"; 
   end if;   

end process;

result_proc:process(all)
begin
    if (S(32) = '1') then
       new_R <= shifted_R(62 downto 0) & '0';
    else
       new_R <= S(31 downto 0) & shifted_R(30 downto 0) & '1';
    end if;
end process;
-------------------------------------------------------------------------------------
------------------------------------ SYNCR ------------------------------------------
-------------------------------------------------------------------------------------
--Division Synchronous Process
Divider_sync : process(clk, reset)
begin
  if reset = '1' then
    count           <= 0; 
    R               <= (others => '0');
    divisor_reg     <= (others => '0');
    div_enable_reg  <= '0';
    division_finished_out<='0';
  elsif rising_edge(clk) then
          R                       <= allzeros(31 downto 0) & dividend_wire; 
          divisor_reg             <= divisor_wire;
          div_enable_reg          <= div_enable_wire;
          count                   <= 0;
          clz_divisor             <= clz_divisor_wire; 
          division_finished_out   <= division_finished_wire;
           
        if (division_finished_wire ='1' and div_enable_i='1') then   --Uncomment if the division result is an output
          result <= new_R;
        end if;
        if (div_enable_reg='1') then
            count <= count_wire;
            R     <= new_R;
        end if;
              
  end if;
end process;
end Behavioral;



