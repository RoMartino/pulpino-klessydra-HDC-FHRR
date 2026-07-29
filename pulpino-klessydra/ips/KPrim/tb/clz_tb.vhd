library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use ieee.math_real.all;

entity clz_tb is
end clz_tb;

architecture tb of clz_tb is

    constant size       : integer := 3;
    constant size_width : integer := integer(ceil(log2(real(size))));

    component CLZ_top
        generic(
            size       : natural;
            size_width : natural
        );
        port (
            input : in std_logic_vector(size-1 downto 0);
            output : out std_logic_vector(size_width downto 0)
        );
    end component;

    component CLZ_for
        generic(
            size       : natural;
            size_width : natural
        );
        port (
            input : in std_logic_vector(size-1 downto 0);
            output : out std_logic_vector(size_width downto 0)
        );
    end component;


    component fpu_ff
        generic(
            LEN         : natural
        );
        port (
            in_i        : in std_logic_vector(size-1 downto 0);
            first_one_o : out std_logic_vector(size_width-1 downto 0);
            no_ones_o   : out std_logic
        );
    end component;

    signal tb_input : std_logic_vector(size-1 downto 0);
    signal tb_output, expected_output : std_logic_vector(size_width downto 0);

    -- Function to calculate leading zeros
    function calc_leading_zeros(input : std_logic_vector) return std_logic_vector is
        variable result : integer := 0;
    begin
        for i in size-1 downto 0 loop
            if input(i) = '0' then
                result := result + 1;
            else
                exit;
            end if;
        end loop;
        return std_logic_vector(to_unsigned(result, size_width+1));
    end function;

    --procedure pseudo_rand(
    --    variable seed   : inout integer;
    --    variable output : out std_logic_vector;
    --    constant N      : in natural := 64 -- Defaulting to 64 bits if not specified
    --) is
    --    variable rnd    : integer;
    --    variable modulus: integer;
    --begin
    --    modulus := 2**(N-1) - 1;  -- Dynamically calculate modulus based on the bit size
    --    rnd := (214013 * seed + 2531011);   -- Generate a new pseudo-random number
    --    seed := rnd mod modulus;            -- Update seed, keeping it within the custom range
    --    output := std_logic_vector(to_unsigned(seed, N)); -- Creating std_logic_vector from seed
    --end procedure;

    procedure pseudo_rand(
        variable seed   : inout integer; -- Changed seed to natural if possible, otherwise ensure it's positive in usage
        variable output : out std_logic_vector;
        constant N      : in natural := 64 -- Default to 64 bits if not specified
    ) is
        variable rnd    : integer;
        variable rnd_unsigned : unsigned((N-1) downto 0);
    begin
        -- Linear congruential generator formula
        rnd := (214013 * seed + 2531011);
    
        -- Ensure rnd is non-negative by taking modulo 2**31 (a common modulus for ensuring non-negativity)
        rnd := rnd mod 2147483647; -- Modulus with 2^31-1 to prevent negative results
    
        -- Convert rnd to unsigned for bit manipulation
        rnd_unsigned := to_unsigned(rnd, N);
    
        -- Update seed with a non-negative value
        seed := to_integer(rnd_unsigned) mod 2147483647; -- Keep seed positive and within integer range
    
        -- Output the N least significant bits of the seed
        output := std_logic_vector(rnd_unsigned);
    end procedure;


    function to_slv_string(value : std_logic_vector) return string is
        variable str : string(1 to value'length);
    begin
        -- Loop over the std_logic_vector in the original order but fill the string in reverse order
        for i in value'range loop
            str(value'length - i) := character'VALUE(std_ulogic'IMAGE(value(i)));
        end loop;
        return str;
    end function;

    signal tb_inv : std_logic_vector(size-1 downto 0);

begin

    tb_inv <= not tb_input;

    uut: CLZ_top 
    generic map(
        size       => size,
        size_width => size_width
    )
    port map (
        input  => tb_input,
        output => tb_output
    );

    --uut: clz_for
    --generic map(
    --    size       => size,
    --    size_width => size_width
    --)
    --port map (
    --    input  => tb_input,
    --    output => tb_output
    --);

    --uut: fpu_ff
    --generic map(
    --    LEN       => size
    --)
    --port map (
    --    in_i        => tb_inv, -- Leading one counter, hence don't invert the input
    --    first_one_o => tb_output(size_width-1 downto 0),
    --    no_ones_o   => tb_output(size_width)
    --);

    stimulus: process
        variable seed : integer := 1;  -- Random seed
        variable random_value : std_logic_vector(size-1 downto 0);
        variable test_runs : integer := 0;
        variable errors    : integer := 0;
    begin
        for i in 1 to 10000 loop  -- Run 1000000 tests
            pseudo_rand(seed, random_value, size);
            tb_input <= random_value;
            wait for 20 ns;  -- Wait for the DUT to process (assuming a clock period of 10 ns)

            expected_output <= calc_leading_zeros(tb_input);
            wait for 10 ns;  -- Wait for the DUT to process

            test_runs := test_runs + 1;

            if tb_output = expected_output then
                report "i = " & integer'image(i) & ", PASS: Input = " & to_slv_string(tb_input) &
                       " Output = " & to_slv_string(tb_output)
                severity note;
            else
                errors := errors + 1;
                report "i = " & integer'image(i) & ", FAIL: Input = " & to_slv_string(tb_input) &
                       " Expected = " & to_slv_string(expected_output) &
                       " Got = " & to_slv_string(tb_output)
                --severity error;
                severity failure;
            end if;

            wait for 10 ns; -- Time between tests
        end loop;
        report "Test Runs = " & integer'image(test_runs) & ", Errors = " & integer'image(errors)
        severity note;
        wait;
    end process;
end tb;
