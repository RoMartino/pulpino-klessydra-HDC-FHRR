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

entity CLZ_top is
    generic (
      size       : natural := 32;
      size_width : integer := integer(ceil(log2(real(size))))
    );
    Port (
      input  : in  std_logic_vector(size-1 downto 0);
      output : out std_logic_vector(size_width downto 0)
    );
end CLZ_top;
architecture Behavioral of CLZ_top is


  type array_2d  is array (integer range<>) of std_logic_vector;
  type array_3d  is array (integer range<>) of array_2d;

  --type natural_array is array (integer range <>) of natural;
  type natural_array is array (31 downto -1) of natural; -- the -ve index of the range is used "h-1" in the for generate loop below, where it will be the place holder for '0' during the first iteration

  type power_decomposition is record
      count       : natural;
      powers      : natural_array;      -- Array to store individual powers of two
      sums        : natural_array;      -- Array to store accumulated sums of these powers
  end record;

  function accumulate_vectors(signal z: array_2d; size_width : natural) return std_logic_vector is
    variable result_vector : std_logic_vector(size_width downto 0) := (others => '0'); -- Extra bit for potential overflow
    variable sum           : unsigned(size_width downto 0) := (others => '0'); -- Extra bit for carry
    variable current_vector: unsigned(size_width downto 0);
  begin
    -- Iterate over each element in the array
    for i in z'range loop
        -- Convert the current std_logic_vector to unsigned for addition
        current_vector := unsigned(z(i));
        -- Add the current vector to the sum
        sum := sum + current_vector;
    end loop;
    -- Convert the accumulated sum back to std_logic_vector
    result_vector := std_logic_vector(sum);
    return result_vector;
  end function;

  function integer_log2(value: natural) return natural is
    variable result   : natural := 0;
    variable temp_val : natural := value;
  begin
    while temp_val > 1 loop
      temp_val := temp_val / 2;
      result   := result + 1;
    end loop;
    return result;
  end function;

  function decompose_and_count_powers(value: natural) return power_decomposition is
      variable result_array: natural_array := (others => 0);  -- Array for powers of two
      variable sum_array   : natural_array := (others => 0);  -- Array for accumulated sums
      variable decomposition: power_decomposition;
      variable i: integer := 0;
      variable remaining: natural := value;
      variable power: natural;
      variable accum_sum: natural := 0;  -- Accumulator for sums
  begin
      -- Decompose value into powers of two
      while remaining > 0 loop
          power := 2**integer_log2(remaining);
          result_array(i) := power;
          accum_sum := accum_sum + power;  -- Accumulate sum
          sum_array(i) := accum_sum;       -- Store current sum
          remaining := remaining - power;
          i := i + 1;
      end loop;
      decomposition.count              := i;  -- Number of elements
      decomposition.powers(i-1 downto 0) := result_array(i-1 downto 0);  -- Store valid powers
      decomposition.sums(i-1 downto 0)   := sum_array(i-1 downto 0);     -- Store valid sums
      return decomposition;
  end decompose_and_count_powers;

  constant decomposition_result : power_decomposition := decompose_and_count_powers(size);
  constant powers               : natural_array       := decomposition_result.powers;
  constant power_sum            : natural_array       := decomposition_result.sums;
  constant clz_unit_count       : natural             := decomposition_result.count;

  component CLZ
    generic (
      size       : natural;
      size_width : natural;
      clz_width  : natural
    );
    Port (
      v_i    : in  std_logic_vector;
      v_cin  : in  std_logic;
      v_cout : out std_logic;
      z_o    : out std_logic_vector
    );
  end component;

  signal v  : std_logic_vector(size-1 downto 0);
  signal z  : array_2d(clz_unit_count-1 downto 0)(size_width downto 0);

  signal v_carry : std_logic_vector(clz_unit_count-1 downto 0);

begin

  v <= input;

  clz_gen : for h in clz_unit_count-1 downto 0 generate
  begin

    --process
    --begin
    --report "h= " & integer'image(h) & ", powers = " & integer'image(decomposition_result.powers(h)) severity note;
    --report "h= " & integer'image(h) & ", sums = "   & integer'image(decomposition_result.sums(h)) severity note;
    --wait for 100 ns;
    --end process;

    gen_clz_1 : if h = clz_unit_count-1 generate
      clz_i : clz
      generic map (
        size       => powers(h),
        size_width => size_width,
        clz_width  => integer_log2(powers(h))
      )
      port map (
        v_i    => v(power_sum(h)-1 downto power_sum(h-1)),
        v_cin  => '1',
        v_cout => v_carry(h),
        z_o    => z(h)
      );
    end generate;

    gen_clz_n : if h < clz_unit_count-1 generate
      clz_i : clz
      generic map (
        size       => powers(h),
        size_width => size_width,
        clz_width  => integer_log2(powers(h))
      )
      port map (
        v_i    => v(power_sum(h)-1 downto power_sum(h-1)),
        v_cin  => v_carry(h+1),
        v_cout => v_carry(h),
        z_o    => z(h)
      );
    end generate;

  end generate;

  --zero_upper_bits : process(z)
  --begin
  --  for h in 0 to clz_unit_count-1 loop
  --    z2(h)(size_width downto integer_log2(powers(h))+1) <= (others => '0');
  --  end loop;
  --end process;

  output <= accumulate_vectors(z,size_width);

end Behavioral;

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity CLZ is
  generic (
      size       : natural;
      size_width : natural;
      clz_width  : natural
  );
  Port (
    v_i    : in   std_logic_vector(size-1 downto 0);
    v_cin  : in   std_logic;
    v_cout : out  std_logic;
    z_o    : out  std_logic_vector(size_width downto 0)
  );
end CLZ;

architecture Behavioral of CLZ is
  type array_2d  is array (integer range<>) of std_logic_vector;
  type array_3d  is array (integer range<>) of array_2d;
begin


  --  ██████╗██╗     ███████╗       ██╗
  -- ██╔════╝██║     ╚══███╔╝      ███║
  -- ██║     ██║       ███╔╝ █████╗╚██║
  -- ██║     ██║      ███╔╝  ╚════╝ ██║
  -- ╚██████╗███████╗███████╗       ██║
  --  ╚═════╝╚══════╝╚══════╝       ╚═╝

  CLZ_1_GEN : if size = 1 generate 

    process(v_i)
    begin
      v_cout <= not(v_i(0));
      z_o <= (others => '0');
      z_o(clz_width downto 0) <= not(v_i);
    end process;

  end generate CLZ_1_GEN;


  --  ██████╗██╗     ███████╗      ██████╗ 
  -- ██╔════╝██║     ╚══███╔╝      ╚════██╗
  -- ██║     ██║       ███╔╝ █████╗ █████╔╝
  -- ██║     ██║      ███╔╝  ╚════╝██╔═══╝ 
  -- ╚██████╗███████╗███████╗      ███████╗
  --  ╚═════╝╚══════╝╚══════╝      ╚══════╝

  CLZ_2_GEN : if size = 2 generate 

    CLZ_2_Bit : process(v_i, v_cin)
      variable v_0 : std_logic_vector(0 downto 0);
      variable z_0 : std_logic_vector(0 downto 0);
    begin
      v_0(0) := not(v_i(0)) and not(v_i(1)); -- if the two adjacent bits are "00", propagate a '1'
      z_0(0) :=     v_i(0)  and not(v_i(1)); -- if the two adjacent bits are "01", propagate a '1'

      z_o    <= (others => '0');
      v_cout <= v_cin;
      if v_cin then 
        v_cout <= v_0(0);
        z_o(clz_width downto 0) <= v_0(0) & z_0(0);
        if (v_0(0)) then
          z_o(clz_width downto 0) <= v_0(0) & "0";
        end if;
      end if;
    end process;

  end generate CLZ_2_GEN;


  --  ██████╗██╗     ███████╗      ██╗  ██╗
  -- ██╔════╝██║     ╚══███╔╝      ██║  ██║
  -- ██║     ██║       ███╔╝ █████╗███████║
  -- ██║     ██║      ███╔╝  ╚════╝╚════██║
  -- ╚██████╗███████╗███████╗           ██║
  --  ╚═════╝╚══════╝╚══════╝           ╚═╝

  CLZ_4_GEN : if size = 4 generate 

    CLZ_4_Bit : process(v_i, v_cin)
      variable v_0 : std_logic_vector(1 downto 0);
      variable z_0 : std_logic_vector(1 downto 0);
      variable z_1 : std_logic_vector(1 downto 0);
      variable v_1 : std_logic_vector(0 downto 0);
    begin    

      for i in 0 to 1 loop
        -- CLZ bits 0,1
        v_0(i) := not(v_i(i*2)) and not(v_i((i*2)+1)); -- if the two adjacent bits are "00", propagate a '1'
        z_0(i) :=     v_i(i*2)  and not(v_i((i*2)+1)); -- if the two adjacent bits are "01", propagate a '1'
      end loop;

      v_1(0) :=  v_0(1) and v_0(0); -- AND tree stage 2 (stage 1 was the assignment to v_0 after the inversion of the inputs)
      z_1(0) := (z_0(0) and v_0(1)) or z_0(1); -- propagate a '1' to the even bits of z_1 if the input bits are: ("0001" or "01xx") 
      z_1(1) :=  v_0(1); -- propagate '1' to the odd bits (1-15) of z_1 if the pack of four input bits are "00xx"

      z_o    <= (others => '0');
      v_cout <= v_cin;
      if v_cin = '1' then 
        v_cout <= v_1(0);
        z_o(clz_width downto 0) <= v_1(0) & z_1;
        if (v_1(0)) then
          z_o(clz_width downto 0) <= v_1(0) & (0 to clz_width-1 => '0');
        end if;
      end if;
    end process;

  end generate CLZ_4_GEN;


  --  ██████╗██╗     ███████╗       █████╗ 
  -- ██╔════╝██║     ╚══███╔╝      ██╔══██╗
  -- ██║     ██║       ███╔╝ █████╗╚█████╔╝
  -- ██║     ██║      ███╔╝  ╚════╝██╔══██╗
  -- ╚██████╗███████╗███████╗      ╚█████╔╝
  --  ╚═════╝╚══════╝╚══════╝       ╚════╝ 

  CLZ_8_GEN : if size = 8 generate 

    CLZ_8_Bit : process(v_i, v_cin)
      variable v_0 : std_logic_vector(3 downto 0);
      variable z_0 : std_logic_vector(3 downto 0);
      variable z_1 : std_logic_vector(3 downto 0);
      variable v_1 : std_logic_vector(1 downto 0);
      variable z_2 : std_logic_vector(2 downto 0);
      variable v_2 : std_logic_vector(0 downto 0);
    begin

      for i in 0 to 3 loop
        -- CLZ bits 0,1
        v_0(i) := not(v_i(i*2)) and not(v_i((i*2)+1)); -- if the two adjacent bits are "00", propagate a '1'
        z_0(i) :=     v_i(i*2)  and not(v_i((i*2)+1)); -- if the two adjacent bits are "01", propagate a '1'
      end loop;

      -- 8 x 4 bit CLZs
      for i in 0 to 1 loop
          v_1(i)       :=  v_0((i*2)+1) and v_0(2*i); -- AND tree stage 2 (stage 1 was the assignment to v_0 after the inversion of the inputs)
          z_1(2*i)     := (z_0(2*i)     and v_0((i*2)+1)) or z_0((i*2)+1); -- propagate a '1' to the even bits of z_1 if the input bits are: ("0001" or "01xx") 
          z_1((i*2)+1) :=  v_0((i*2)+1); -- propagate '1' to the odd bits (1-15) of z_1 if the pack of four input bits are "00xx"
      end loop;

      -- 4 x 8 bit CLZs:
      v_2(0) :=  v_1(1) and v_1(0); -- AND tree stage 3
      z_2(0) := (z_1(0) and v_1(1)) or  z_1(2); -- propagate a '1' to bits 0,3,6,9  of z_2 if the input is "0000_0001" or "0000_01xx" or "0001_xxxx" or "01xx_xxxxx"
      z_2(1) := (z_1(1) and v_1(1)) or (z_1(3) and not v_1(1)); -- propagate a '1' to bits 1,4,7,10 of z_2 the input is "0000_00xx" or "00xx_xxxx"
      z_2(2) :=  v_1(1); -- propagate '1' to the bits 2,5,8,11 of z_2 if the input is "0000_xxxx"

      z_o    <= (others => '0');
      v_cout <= v_cin;
      if v_cin then 
        v_cout <= v_2(0);
        z_o(clz_width downto 0) <= v_2(0) & z_2;
        if (v_2(0)) then
          z_o(clz_width downto 0) <= v_2(0) & (0 to clz_width-1 => '0');
        end if;
      end if;
    end process;

  end generate CLZ_8_GEN;


  --  ██████╗██╗     ███████╗       ██╗ ██████╗ 
  -- ██╔════╝██║     ╚══███╔╝      ███║██╔════╝ 
  -- ██║     ██║       ███╔╝ █████╗╚██║███████╗ 
  -- ██║     ██║      ███╔╝  ╚════╝ ██║██╔═══██╗
  -- ╚██████╗███████╗███████╗       ██║╚██████╔╝
  --  ╚═════╝╚══════╝╚══════╝       ╚═╝ ╚═════╝ 

  CLZ_16_GEN : if size = 16 generate 

    CLZ_16_Bit : process(v_i, v_cin)
      variable v_0 : std_logic_vector(7 downto 0);
      variable z_0 : std_logic_vector(7 downto 0);
      variable z_1 : std_logic_vector(7 downto 0);
      variable v_1 : std_logic_vector(3 downto 0);
      variable z_2 : std_logic_vector(5 downto 0);
      variable v_2 : std_logic_vector(1 downto 0);
      variable z_3 : std_logic_vector(3 downto 0);
      variable v_3 : std_logic_vector(0 downto 0);
    begin

      for i in 0 to 7 loop
        -- CLZ bits 0,1
        v_0(i) := not(v_i(i*2)) and not(v_i((i*2)+1)); -- if the two adjacent bits are "00", propagate a '1'
        z_0(i) :=     v_i(i*2)  and not(v_i((i*2)+1)); -- if the two adjacent bits are "01", propagate a '1'
      end loop;

      -- 4 x 4 bit CLZs
      for i in 0 to 3 loop
        v_1(i)       := v_0((i*2)+1)  and v_0(2*i); -- AND tree stage 2 (stage 1 was the assignment to v_0 after the inversion of the inputs)
        z_1(2*i)     := (z_0(2*i)     and v_0((i*2)+1)) or z_0((i*2)+1); -- propagate a '1' to the even bits of z_1 if the input bits are: ("0001" or "01xx") 
        z_1((i*2)+1) := v_0((i*2)+1); -- propagate '1' to the odd bits (1-15) of z_1 if the pack of four input bits are "00xx"
      end loop;

      -- 2 x 8 bit CLZs:
      for i in 0 to 1 loop
         v_2(i)     := v_1((i*2)+1)  and v_1(2*i); -- AND tree stage 3
         z_2(3*i)   := (z_1(i*4)     and v_1((i*2)+1)) or  z_1((i*4)+2); -- propagate a '1' to bits 0,3,6,9  of z_2 if the input is "0000_0001" or "0000_01xx" or "0001_xxxx" or "01xx_xxxxx"
         z_2(3*i+1) := (z_1((i*4)+1) and v_1((i*2)+1)) or (z_1((i*4)+3) and not v_1((i*2)+1)); -- propagate a '1' to bits 1,4,7,10 of z_2 the input is "0000_00xx" or "00xx_xxxx"
         z_2(3*i+2) := v_1((i*2)+1); -- propagate '1' to the bits 2,5,8,11 of z_2 if the input is "0000_xxxx"
      end loop;

      -- 1 x 16 bits CLZs
      v_3(0) :=  v_2(1) and v_2(0); -- AND tree stage 4 becomes '1' when there are 16 leading zero bits
      z_3(0) := (z_2(0) and v_2(1)) or  z_2(3); -- propagate '1' to bits 0,4 of z_3 if the input is "00000000_00000001" or "00000000_000001xx" or "00000000_0001xxxx" or "00000000_01xxxxxx" or "00000001_xxxxxxxx" or "000001xx_xxxxxxxx" or "0001xxxx_xxxxxxxx" or "01xxxxxx_xxxxxxxx"
      z_3(1) := (z_2(1) and v_2(1)) or (z_2(4) and not v_2(1)); -- propagate '1' to bits 1,5 of z_3 if the input is "00000000_000000xx" or "00000000_00xxxxxx" or "000000xx_xxxxxxxx" or "00xxxxxx_xxxxxxxx"
      z_3(2) := (z_2(2) and v_2(1)) or (z_2(5) and not v_2(1)); -- propagate '1' to bits 2,6 of z_3 if the input is "00000000_0000xxxx" or "0000xxxx_xxxxxxxx"
      z_3(3) :=  v_2(1); -- propagate '1' to the bits 3,7 of z_3 if the input is "00000000_xxxxxxxx"

      z_o    <= (others => '0');
      v_cout <= v_cin;
      if v_cin then 
        v_cout <= v_3(0);
        z_o(clz_width downto 0) <= v_3(0) & z_3;
        if (v_3(0)) then
          z_o(clz_width downto 0) <= v_3(0) & (0 to clz_width-1 => '0');
        end if;
      end if;
    end process;

  end generate CLZ_16_GEN;


  --  ██████╗██╗     ███████╗      ██████╗ ██████╗ 
  -- ██╔════╝██║     ╚══███╔╝      ╚════██╗╚════██╗
  -- ██║     ██║       ███╔╝ █████╗ █████╔╝ █████╔╝
  -- ██║     ██║      ███╔╝  ╚════╝ ╚═══██╗██╔═══╝ 
  -- ╚██████╗███████╗███████╗      ██████╔╝███████╗
  --  ╚═════╝╚══════╝╚══════╝      ╚═════╝ ╚══════╝

  CLZ_32_GEN : if size = 32 generate 

    CLZ_32_Bit : process(v_i, v_cin)
      variable v_0      : std_logic_vector(15 downto 0);
      variable z_0      : std_logic_vector(15 downto 0);
      variable z_1      : std_logic_vector(15 downto 0);
      variable v_1      : std_logic_vector(7 downto 0);
      variable z_2      : std_logic_vector(11 downto 0);
      variable v_2      : std_logic_vector(3 downto 0);
      variable z_3      : std_logic_vector(7 downto 0);
      variable v_3      : std_logic_vector(1 downto 0);
      variable z_4      : std_logic_vector(4 downto 0);
      variable v_4      : std_logic_vector(0 downto 0);
    begin    
      -- For a 32 CLZs unit we use:
      -- 16 x 2 bit CLZs
      for i in 0 to 15 loop
          -- CLZ bits 0,1
          v_0(i):= not(v_i(i*2)) and not(v_i((i*2)+1)); -- if the two adjacent bits are "00", propagate a '1'
          z_0(i):=     v_i(i*2)  and not(v_i((i*2)+1)); -- if the two adjacent bits are "01", propagate a '1'
      end loop;

      -- 8 x 4 bit CLZs
      for i in 0 to 7 loop
          v_1(i)      :=  v_0((i*2)+1) and v_0(2*i); -- AND tree stage 2 (stage 1 was the assignment to v_0 after the inversion of the inputs)
          z_1(2*i)    := (z_0(2*i)     and v_0((i*2)+1)) or z_0((i*2)+1); -- propagate a '1' to the even bits of z_1 if the input bits are: ("0001" or "01xx") 
          z_1((i*2)+1):=  v_0((i*2)+1); -- propagate '1' to the odd bits (1-15) of z_1 if the pack of four input bits are "00xx"
      end loop;

      -- 4 x 8 bit CLZs:
      for i in 0 to 3 loop
           v_2(i)     :=  v_1((i*2)+1) and v_1(2*i); -- AND tree stage 3
           z_2(3*i)   := (z_1(i*4)     and v_1((i*2)+1)) or  z_1((i*4)+2); -- propagate a '1' to bits 0,3,6,9  of z_2 if the input is "0000_0001" or "0000_01xx" or "0001_xxxx" or "01xx_xxxxx"
           z_2(3*i+1) := (z_1((i*4)+1) and v_1((i*2)+1)) or (z_1((i*4)+3) and not(v_1((i*2)+1))); -- propagate a '1' to bits 1,4,7,10 of z_2 the input is "0000_00xx" or "00xx_xxxx"
           z_2(3*i+2) :=  v_1((i*2)+1); -- propagate '1' to the bits 2,5,8,11 of z_2 if the input is "0000_xxxx"
      end loop;

      -- 2 x 16 bits CLZs
      for i in 0 to 1 loop
          v_3(i)      :=  v_2((i*2)+1) and v_2(2*i); -- AND tree stage 4 becomes '1' when there are 16 leading zero bits
          z_3(4*i)    := (z_2((i*6))   and v_2((i*2)+1)) or  z_2((i*6)+3); -- propagate '1' to bits 0,4 of z_3 if the input is "00000000_00000001" or "00000000_000001xx" or "00000000_0001xxxx" or "00000000_01xxxxxx" or "00000001_xxxxxxxx" or "000001xx_xxxxxxxx" or "0001xxxx_xxxxxxxx" or "01xxxxxx_xxxxxxxx"
          z_3(4*i+1)  := (z_2((i*6)+1) and v_2((i*2)+1)) or (z_2((i*6)+4) and not(v_2((i*2)+1))); -- propagate '1' to bits 1,5 of z_3 if the input is "00000000_000000xx" or "00000000_00xxxxxx" or "000000xx_xxxxxxxx" or "00xxxxxx_xxxxxxxx"
          z_3(4*i+2)  := (z_2((i*6)+2) and v_2((i*2)+1)) or (z_2((i*6)+5) and not(v_2((i*2)+1))); -- propagate '1' to bits 2,6 of z_3 if the input is "00000000_0000xxxx" or "0000xxxx_xxxxxxxx"
          z_3(4*i+3)  :=  v_2((i*2)+1); -- propagate '1' to the bits 3,7 of z_3 if the input is "00000000_xxxxxxxx"
      end loop;

      -- 1 x 32 bits CLZ
      v_4(0) :=  v_3(1) and v_3(0); -- AND tree stage 5 becomes 1 if the input is all 0
      z_4(0) := (z_3(0) and v_3(1)) or  z_3(4); -- propagate '1' to bit 0 of z_4 if the input is "00000000_00000000_00000000_00000001" or "00000000_00000000_00000000_000001xx" or "00000000_00000000_00000000_0001xxxx" ..... or "01xxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx"
      z_4(1) := (z_3(1) and v_3(1)) or (z_3(5) and not(v_3(1))); -- propagate '1' to bit 1 of z_4 if the input is "00000000_00000000_00000000_000000xx" or "00000000_00000000_00000000_00xxxxxx" or "00000000_00000000_000000xx_xxxxxxxx" ..... or "00xxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx"
      z_4(2) := (z_3(2) and v_3(1)) or (z_3(6) and not(v_3(1))); -- propagate '1' to bit 2 of z_4 if the input is "00000000_00000000_00000000_0000xxxx" or "00000000_00000000_0000xxxx_xxxxxxxx" or "00000000_0000xxxx_xxxxxxxx_xxxxxxxx" or "0000xxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx"
      z_4(3) := (z_3(3) and v_3(1)) or (z_3(7) and not(v_3(1))); -- propagate '1' to bit 2 of z_4 if the input is "00000000_00000000_00000000_xxxxxxxx" or "00000000_xxxxxxxx_xxxxxxxx_xxxxxxxx"
      z_4(4) :=  v_3(1); -- propagate '1' to the bit 4 of z_4 if the input is "00000000_00000000_xxxxxxxx_xxxxxxxx"

      z_o    <= (others => '0');
      v_cout <= v_cin;
      if v_cin then 
        v_cout <= v_4(0);
        z_o(clz_width downto 0) <= v_4(0) & z_4;
        if (v_4(0)) then
          z_o(clz_width downto 0) <= v_4(0) & (0 to clz_width-1 => '0');
        end if;
      end if;
    end process;

  end generate CLZ_32_GEN;


  --  ██████╗██╗     ███████╗       ██████╗ ██╗  ██╗
  -- ██╔════╝██║     ╚══███╔╝      ██╔════╝ ██║  ██║
  -- ██║     ██║       ███╔╝ █████╗███████╗ ███████║
  -- ██║     ██║      ███╔╝  ╚════╝██╔═══██╗╚════██║
  -- ╚██████╗███████╗███████╗      ╚██████╔╝     ██║
  --  ╚═════╝╚══════╝╚══════╝       ╚═════╝      ╚═╝

  CLZ_64_GEN : if size = 64 generate

    -- REMAINDER LEADING ZERO COUNTER: 64 BIT DATA
    CLZ_64_Bit : process(v_i, v_cin)
      variable v_0 : std_logic_vector(31 downto 0);
      variable z_0 : std_logic_vector(31 downto 0);
      variable z_1 : std_logic_vector(31 downto 0);
      variable v_1 : std_logic_vector(15 downto 0);
      variable z_2 : std_logic_vector(23 downto 0);
      variable v_2 : std_logic_vector(7 downto 0);
      variable z_3 : std_logic_vector(15 downto 0);
      variable v_3 : std_logic_vector(3 downto 0);
      variable z_4 : std_logic_vector(9 downto 0);
      variable v_4 : std_logic_vector(1 downto 0);
      variable z_5 : std_logic_vector(5 downto 0);
      variable v_5 : std_logic_vector(0 downto 0);
    begin        

      -- 2 bit CLZs
      for i in 0 to 31 loop
        -- CLZ bits 0,1
        v_0(i):= not(v_i(i*2)) and not(v_i((i*2)+1));
        z_0(i):=     v_i(i*2)  and not(v_i((i*2)+1));
      end loop;

      -- 4 bit CLZs
      for i in 0 to 15 loop
        v_1(i)      := v_0((i*2)+1)  and v_0(2*i);
        z_1(2*i)    := (z_0(2*i)     and v_0((i*2)+1)) or (z_0((i*2)+1) and not (v_0((i*2)+1)));
        z_1((i*2)+1):= v_0((i*2)+1);
      end loop;

      -- 8 bit CLZs:
      for i in 0 to 7 loop
         v_2(i)     := v_1((i*2)+1)  and v_1(2*i);
         z_2(3*i)   := (z_1(i*4)     and v_1((i*2)+1)) or (z_1((i*4)+2) and not(v_1((i*2)+1)));
         z_2(3*i+1) := (z_1((i*4)+1) and v_1((i*2)+1)) or (z_1((i*4)+3) and not(v_1((i*2)+1)));
         z_2(3*i+2) := v_1((i*2)+1);
      end loop;

      -- 16 bit CLZs
      for i in 0 to 3 loop
        v_3(i)      := v_2((i*2)+1)  and v_2(2*i);        
        z_3(4*i)    := (z_2((i*6))   and v_2((i*2)+1)) or (z_2((i*6)+3) and not(v_2((i*2)+1)));
        z_3(4*i+1)  := (z_2((i*6)+1) and v_2((i*2)+1)) or (z_2((i*6)+4) and not(v_2((i*2)+1)));
        z_3(4*i+2)  := (z_2((i*6)+2) and v_2((i*2)+1)) or (z_2((i*6)+5) and not(v_2((i*2)+1)));        
        z_3(4*i+3)  := v_2((i*2)+1);
      end loop;

      -- 32 bits CLZ
      for i in 0 to 1 loop
        v_4(i)      := (v_3((i*2)+1) and v_3(2*i));
        z_4(5*i)    := (z_3(i*8)     and v_3((i*2)+1)) or (z_3((i*8)+4) and not(v_3((i*2)+1)));
        z_4(5*i+1)  := (z_3((i*8)+1) and v_3((i*2)+1)) or (z_3((i*8)+5) and not(v_3((i*2)+1)));
        z_4(5*i+2)  := (z_3((i*8)+2) and v_3((i*2)+1)) or (z_3((i*8)+6) and not(v_3((i*2)+1)));
        z_4(5*i+3)  := (z_3((i*8)+3) and v_3((i*2)+1)) or (z_3((i*8)+7) and not(v_3((i*2)+1)));
        z_4(5*i+4)  := (v_3((i*2)+1));
      end loop;

      -- 64 bits CLZ
      v_5(0)  := (v_4(1) and v_4(0));
      z_5(5)  := (v_4(1));
      z_5(4)  := (z_4(4) and v_4(1)) or (z_4(9) and not(v_4(1)));
      z_5(3)  := (z_4(3) and v_4(1)) or (z_4(8) and not(v_4(1)));
      z_5(2)  := (z_4(2) and v_4(1)) or (z_4(7) and not(v_4(1)));
      z_5(1)  := (z_4(1) and v_4(1)) or (z_4(6) and not(v_4(1)));
      z_5(0)  := (z_4(0) and v_4(1)) or (z_4(5) and not(v_4(1)));

      z_o    <= (others => '0');
      v_cout <= v_cin;
      if v_cin then 
        v_cout <= v_5(0);
        z_o(clz_width downto 0) <= v_5(0) & z_5;
        if (v_5(0)) then
          z_o(clz_width downto 0) <= v_5(0) & (0 to clz_width-1 => '0');
        end if;
      end if;

    end process;

  end generate CLZ_64_GEN;


  -- ██████╗  █████╗ ███████╗███████╗    ██████╗      ██████╗██╗     ███████╗
  -- ██╔══██╗██╔══██╗██╔════╝██╔════╝    ╚════██╗    ██╔════╝██║     ╚══███╔╝
  -- ██████╔╝███████║███████╗█████╗█████╗ █████╔╝    ██║     ██║       ███╔╝ 
  -- ██╔══██╗██╔══██║╚════██║██╔══╝╚════╝██╔═══╝     ██║     ██║      ███╔╝  
  -- ██████╔╝██║  ██║███████║███████╗    ███████╗    ╚██████╗███████╗███████╗
  -- ╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝    ╚══════╝     ╚═════╝╚══════╝╚══════╝

--  CLZ_GEN_Bit : process(v_i, v_cin)
--    variable v        : array_2d(size_width-1 downto 0)((size/2)-1 downto 0);
--    variable z        : array_3d(size_width-1 downto 0)((size/2)-1 downto 0)((size_width*2)-2 downto 0);
--    variable size_tmp : natural;
--  begin
--
--      size_tmp := size/2;
--      for j in 0 to size_tmp-1 loop
--        -- CLZ bits 0,1
--        v(0)(j)    := not(v_i(j*2)) and not(v_i((j*2)+1));
--        z(0)(j)(0) :=     v_i(j*2)  and not(v_i((j*2)+1));
--      end loop;
--      size_tmp := size/4;
--      for i in 0 to size_width-2 loop -- corresponds to the number of for loops
--        for j in 0 to size_tmp-1 loop -- corresponds to the loops size, that goes by half in every stage
--            v(i+1)(j)  := v(i)((2*j)+1) and v(i)(2*j);
--            for k in 0 to i loop -- corresponds to number of z lines in the loop except the last line
--              report "i=" & integer'image(i) & " j=" & integer'image(j) & " k=" & integer'image(k);
--              z(i+1)(j*(i+2))(k) := (z(i)(j*2*(i+1))(k) and v(i)((2*j)+1)) or (z(i)(j*2*(i+1))(k+i+1) and not v(i)((2*j)+1));
--              -- i=2, j=0, k=2
--              --z(3)(0)(2) <= ( (z(2)(0)(2) and v(2)(1) ) or ( z(2)(0)(5) and not v(2)(1) );
--            end loop;
--            z(i+1)(j*(i+2))(i+1) := v(i)((j*2)+1);
--        end loop;
--        size_tmp := size_tmp/2;
--      end loop;
--
--      z_o    <= (others => '0');
--      v_cout <= v_cin;
--      if v_cin then 
--        v_cout <= v(size_width-1)(0);
--        z_o(clz_width downto 0) <= v(size_width-1)(0) & z(size_width-1)(0)(size_width-1 downto 0);
--        if (v(4)(0)) then
--          z_o(clz_width downto 0) <= v(4)(0) & (0 to clz_width-1 => '0');
--        end if;
--      end if;
--
--  end process;


end Behavioral;