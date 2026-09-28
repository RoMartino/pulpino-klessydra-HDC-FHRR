library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;

-- The divider unit take as input: a dividend, a divisor_reg and an enable signal.
-- All these inputs are registered in the first clock cycle. 
-- The division starts from the second clock cycles.
-- This is a synchronous unit, so it takes as input also clock and reset signals.
-- Finally, the division result is ready when the output signal 
-- division_finished_out is high. The result is available in the remainder reg.
entity divider16 is
    Port ( dividend_i               : in  STD_LOGIC_VECTOR(47 downto 0);
           divisor_i                : in  STD_LOGIC_VECTOR(47 downto 0);
           reset                    : in  STD_LOGIC;
           clk                      : in  STD_LOGIC;
           div_enable_i             : in  STD_LOGIC;
           division_finished_out    : out STD_LOGIC;
          result                   : out STD_LOGIC_VECTOR(95 downto 0) --sh96
           );
end divider16;

architecture Behavioral of divider16 is
  constant PRECISION_BIT_WIDTH : integer := 16;
-- Input registers
signal divisor_reg          : std_logic_vector( 47 downto 0);                -- Dividend register. 32 bits register that store the content of the input signal dividend_i
signal div_enable_reg       : std_logic;                                    -- Enable signal register. 1 bit register that store  the content of the input signal div_enable_i

-- Divider internal registers
signal count                : integer range 48 downto 0;                    -- Counter register. The counter shows the remaining division steps.
signal count_wire           : integer range 48 downto 0;                    -- Counter wire. Counter register input.
signal R                    : std_logic_vector(95 downto 0);                -- Remainder register. 64 bit, in the Most Significant Word (MSW) we have the remainder, in the LSW the quotient. Note that in the first clock cycle the remainder is initialized with the dividend in its LSW.
signal S                    : std_logic_vector(48 downto 0);                -- Difference signal. Output of the subtractor unit 1 (RemainderMSW - divisor_reg)

-- Shifter
signal new_R            : std_logic_vector(95 downto 0);                    -- Signal containing the remainder shifted dynamically. Output of the dynamic shifter
signal division_finished_wire:std_logic;
-- attribute dont_touch           : string;
-- attribute dont_touch of R : signal is "true";
begin


-- SUBTRACTOR
S   <= std_logic_vector(('0' & unsigned(R(94 downto 47))) - ('0' & unsigned(divisor_reg))); 


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

        -- When the counter reach 32, the division is completed: division_finished = '1' and the result is available in remainder
        if count_wire= 48 then
            division_finished_wire<='1';            
        end if;
    end if;
end process;

-------------------------------------------------------------------------------------
----------------------------------- SHIFTER -----------------------------------------
-------------------------------------------------------------------------------------

result_proc:process(all)
begin
    if (S(48) = '1') then
       new_R <= R(94 downto 0) & '0';
    else
       new_R <= S(47 downto 0) & R(46 downto 0) & '1';

    end if;
end process;
-------------------------------------------------------------------------------------
------------------------------------ SYNCR ------------------------------------------
-------------------------------------------------------------------------------------
--Division Synchronous Process
Divider_sync : process(clk, reset)
begin
  if reset = '0' then
    count           <= 0; 
    R               <= (others => '0');
    divisor_reg     <= (others => '0');
    div_enable_reg  <= '0';
    division_finished_out<='0';
 
  elsif rising_edge(clk) then
    division_finished_out <= division_finished_wire;
    result <= R; -- Expose the full 64-bit remainder register (MSW = remainder, LSW = quotient)

    if div_enable_i = '1' and div_enable_reg = '0' then  -- start only on rising edge of enable
        R(47 downto 0)      <= dividend_i;
        R(95 downto 48 )     <= (others => '0');  -- clear upper bits of R
        divisor_reg         <= divisor_i;
        div_enable_reg      <= '1';
        count               <= 0;
    elsif div_enable_reg = '1' then
        R       <= new_R;
        count   <= count_wire;

        if count_wire = 48 then
            div_enable_reg <= '0';  -- stop division after done
        end if;
    end if;
    
    
end if;

end process;
end Behavioral;
--------------------------------------------------------------------------------------------------------------------------------
-- ieee packages ------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;
use ieee.math_real.all;
use std.textio.all;

-- local packages -----------------
use work.riscv_klessydra.all;
--use work.klessydra_parameters.all;



-- ================================================
-- Tree Adder Component Definition (INCLUDED INLINE)
-- ================================================
entity tree_adder is
  generic (
    NUM_INPUTS : integer;
    DATA_WIDTH : integer
  );
  port (
    clk      : in std_logic;
    rst_n    : in std_logic;
    in_data  : in  unsigned((NUM_INPUTS*DATA_WIDTH)-1 downto 0);
    out_data : out unsigned(((NUM_INPUTS/2)*(DATA_WIDTH))-1 downto 0)
  );
end tree_adder;
architecture parallel of tree_adder is
  type sum_array_t is array (0 to (NUM_INPUTS/2)-1) of unsigned(DATA_WIDTH - 1 downto 0);
  signal sum_array : sum_array_t;
begin
  gen_adders: for i in 0 to (NUM_INPUTS/2 - 1) generate
  begin
    sum_array(i) <=
      resize(in_data((2*i+2)*DATA_WIDTH-1 downto (2*i+1)*DATA_WIDTH), DATA_WIDTH) +
      resize(in_data((2*i+1)*DATA_WIDTH-1 downto (2*i)*DATA_WIDTH), DATA_WIDTH);
  end generate;

  process(clk, rst_n)
  begin
    if rst_n = '0' then
      out_data <= (others => '0');
    elsif rising_edge(clk) then
      for i in 0 to (NUM_INPUTS/2 - 1) loop
        out_data((i+1)*(DATA_WIDTH)-1 downto i*(DATA_WIDTH)) <= sum_array(i);
      end loop;
    end if;
  end process;
end parallel;
-------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------

-- =====================================================================
-- [Op-A5b] CORDIC vectoring atan2 unit (shovel-ready alternative backend)
-- ---------------------------------------------------------------------
-- Pipelined N-stage CORDIC vectoring computes angle = atan2(y_in, x_in)
-- in Q1.16 signed radians from Cartesian Q1.16 inputs. Replaces the
-- divider16 + atan_lut chain when divider_kind = "cordic".
--
-- Latency: NUM_ITERATIONS + 1 cycles (one pre-rotation stage, one stage
--          per iteration). Throughput: 1 result/cycle when continuously
--          enabled. For NUM_ITERATIONS=16, this collapses CLIP from
--          ~50*D/P to (16 + D/P) cycles per element.
--
-- Quantisation: not bit-exact with the radix-2 + atan_lut path; expect
-- residual <= 1 ULP per Q0.8 sample. Default backend remains "radix2"
-- so the bit-exact regression baseline is preserved.
-- =====================================================================
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

entity cordic_vectoring is
  generic (
    INPUT_WIDTH    : integer := 32;   -- Q1.16 signed datapath
    NUM_ITERATIONS : integer := 16
  );
  port (
    clk        : in  std_logic;
    rst_n      : in  std_logic;
    enable_i   : in  std_logic;
    -- I/O are std_logic_vector to interoperate cleanly with the surrounding
    -- array_3d signal infrastructure (which is built on std_logic_vector).
    -- The architecture casts to signed internally for arithmetic.
    x_in       : in  std_logic_vector(INPUT_WIDTH - 1 downto 0);  -- "real" component, Q1.16
    y_in       : in  std_logic_vector(INPUT_WIDTH - 1 downto 0);  -- "imag" component, Q1.16
    angle_out  : out std_logic_vector(INPUT_WIDTH - 1 downto 0);  -- atan2 result, Q1.16 rad
    valid_out  : out std_logic
  );
end entity;

architecture pipelined of cordic_vectoring is

  -- Pre-computed atan(2^-i) table in Q1.16 signed radians.
  type cordic_table_t is array (0 to NUM_ITERATIONS - 1)
                          of signed(INPUT_WIDTH - 1 downto 0);

  function gen_cordic_table return cordic_table_t is
    variable t : cordic_table_t;
  begin
    for i in 0 to NUM_ITERATIONS - 1 loop
      t(i) := to_signed(integer(arctan(2.0 ** (-i)) * real(2 ** 16)),
                        INPUT_WIDTH);
    end loop;
    return t;
  end function;

  constant atan_table : cordic_table_t := gen_cordic_table;

  -- pi (Q1.16 signed) for the pre-rotation stage. arctan magnitudes saturate
  -- to ~pi/2; +/- pi covers the 2nd/3rd quadrant pre-rotation.
  constant PI_Q116    : signed(INPUT_WIDTH - 1 downto 0) :=
    to_signed(integer(MATH_PI * real(2 ** 16)), INPUT_WIDTH);

  -- One register set per pipeline stage. Stage 0 holds the pre-rotated
  -- inputs; stages 1..NUM_ITERATIONS hold the iterated values.
  type stage_array_t is array (0 to NUM_ITERATIONS)
                          of signed(INPUT_WIDTH - 1 downto 0);
  signal x_stage : stage_array_t;
  signal y_stage : stage_array_t;
  signal z_stage : stage_array_t;

  -- valid tracker delayed alongside the datapath
  signal valid_pipe : std_logic_vector(NUM_ITERATIONS downto 0);

begin

  cordic_proc : process(clk, rst_n)
    variable x_signed   : signed(INPUT_WIDTH - 1 downto 0);
    variable y_signed   : signed(INPUT_WIDTH - 1 downto 0);
    variable y_shifted  : signed(INPUT_WIDTH - 1 downto 0);
    variable x_shifted  : signed(INPUT_WIDTH - 1 downto 0);
  begin
    if rst_n = '0' then
      for s in 0 to NUM_ITERATIONS loop
        x_stage(s) <= (others => '0');
        y_stage(s) <= (others => '0');
        z_stage(s) <= (others => '0');
      end loop;
      valid_pipe <= (others => '0');
    elsif rising_edge(clk) then

      x_signed := signed(x_in);
      y_signed := signed(y_in);

      -- ---- Stage 0: pre-rotation into [-pi/2, +pi/2] -----------------
      -- If x_in < 0, rotate by +/- pi so that the iterative core only
      -- needs to converge over the right half-plane. Sign of y_in picks
      -- the rotation direction so the final z lands in (-pi, +pi].
      if x_signed(INPUT_WIDTH - 1) = '1' then
        x_stage(0) <= -x_signed;
        y_stage(0) <= -y_signed;
        if y_signed(INPUT_WIDTH - 1) = '0' then
          z_stage(0) <=  PI_Q116;
        else
          z_stage(0) <= -PI_Q116;
        end if;
      else
        x_stage(0) <= x_signed;
        y_stage(0) <= y_signed;
        z_stage(0) <= (others => '0');
      end if;
      valid_pipe(0) <= enable_i;

      -- ---- Stages 1..N: iterative micro-rotations --------------------
      for i in 0 to NUM_ITERATIONS - 1 loop
        -- Arithmetic shift right by i (sign-extending). shift_right on
        -- signed already preserves the sign bit.
        x_shifted := shift_right(x_stage(i), i);
        y_shifted := shift_right(y_stage(i), i);
        if y_stage(i)(INPUT_WIDTH - 1) = '1' then
          -- y < 0: rotate counter-clockwise
          x_stage(i + 1) <= x_stage(i) - y_shifted;
          y_stage(i + 1) <= y_stage(i) + x_shifted;
          z_stage(i + 1) <= z_stage(i) - atan_table(i);
        else
          -- y >= 0: rotate clockwise
          x_stage(i + 1) <= x_stage(i) + y_shifted;
          y_stage(i + 1) <= y_stage(i) - x_shifted;
          z_stage(i + 1) <= z_stage(i) + atan_table(i);
        end if;
        valid_pipe(i + 1) <= valid_pipe(i);
      end loop;

    end if;
  end process;

  angle_out <= std_logic_vector(z_stage(NUM_ITERATIONS));
  valid_out <= valid_pipe(NUM_ITERATIONS);

end architecture;
-------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------

-- ieee packages ------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;
use ieee.math_real.all;
use std.textio.all;

-- local packages -----------------
use work.riscv_klessydra.all;
--use work.klessydra_parameters.all;

-- HDC  pinout --------------------
entity HDC_Unit is

  generic(
    THREAD_POOL_SIZE      : natural;
    accl_en               : natural;
    replicate_accl_en     : natural;
    multithreaded_accl_en : natural;
    SPM_NUM               : natural; 
    Addr_Width            : natural;
    SIMD                  : natural;
    --------------------------------
    ACCL_NUM              : natural;
    FU_NUM                : natural;
    TPS_CEIL              : natural;
    TPS_BUF_CEIL          : natural;
    SPM_ADDR_WID          : natural;
    SIMD_BITS             : natural;
    Data_Width            : natural;
    SIMD_Width            : natural;
    HDCU_PERF_EN          : natural
    --------------------------------
  );
  port (
  -- Core Signals
    clk_i, rst_ni              : in std_logic;
  -- Processing Pipeline Signals
    rs1_to_sc                  : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
    rs2_to_sc                  : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
    rd_to_sc                   : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
  -- CSR Signals
    HVSIZE                     : in  array_2d(THREAD_POOL_SIZE-1 downto 0)(Addr_Width downto 0);
    MVTYPE                     : in  array_2d(THREAD_POOL_SIZE-1 downto 0)(3 downto 0);
    MPSCLFAC                   : in  array_2d(THREAD_POOL_SIZE-1 downto 0)(4 downto 0);
    hdc_except_data            : out array_2d(ACCL_NUM-1 downto 0)(31 downto 0);
  -- Program Counter Signals
    hdc_taken_branch           : out std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_except_condition       : out std_logic_vector(ACCL_NUM-1 downto 0);
  -- ID_Stage Signals
    decoded_instruction_DSP    : in  std_logic_vector(DSP_UNIT_INSTR_SET_SIZE-1 downto 0);
    harc_EXEC                  : in  natural range THREAD_POOL_SIZE-1 downto 0;
    pc_IE                      : in  std_logic_vector(31 downto 0);
    RS1_Data_IE                : in  std_logic_vector(31 downto 0);
    RS2_Data_IE                : in  std_logic_vector(31 downto 0);
    RD_Data_IE                 : in  std_logic_vector(Addr_Width -1 downto 0);
    hdc_instr_req              : in  std_logic_vector(ACCL_NUM-1 downto 0);
    spm_rs1                    : in  std_logic;
    spm_rs2                    : in  std_logic;
    vec_read_rs1_ID            : in  std_logic;
    vec_read_rs2_ID            : in  std_logic;
    vec_write_rd_ID            : in  std_logic;
    busy_hdc                   : out std_logic_vector(ACCL_NUM-1 downto 0);

  --counter signal declarations:
    hdcu_performance_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0); 
    hdcu_bind_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_bundle_perf_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_clip_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_sim_perf_counter      : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_perm_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_enc_perf_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
  
    -- Scratchpad Interface Signals
    hdc_data_gnt_i             : in  std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_sci_wr_gnt             : in  std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_sc_data_read           : in  array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(SIMD_Width-1 downto 0);
    hdc_we_word                : out array_2d(ACCL_NUM-1 downto 0)(SIMD-1 downto 0);
    hdc_sc_read_addr           : out array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(Addr_Width-1 downto 0);
    hdc_to_sc                  : out array_3d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0)(1 downto 0);
    hdc_sc_data_write_wire     : out array_2d(ACCL_NUM-1 downto 0)(SIMD_Width-1 downto 0);
    hdc_sc_write_addr          : out array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
    hdc_sci_we                 : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    hdc_sci_req                : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    -- tracer signals
    state_HDC                  : out array_2d(ACCL_NUM-1 downto 0)(1 downto 0)
  );

end entity;  

------------------------------------------

architecture HDC of HDC_Unit is

  subtype harc_range is natural range THREAD_POOL_SIZE-1 downto 0;
  subtype accl_range is integer range ACCL_NUM-1 downto 0;
  subtype fu_range   is integer range FU_NUM-1 downto 0;    --TODO, probabilmente FU_NUM è da cambiare
  subtype simd_range   is integer range SIMD-1 downto 0; 
  signal nextstate_HDC : array_2d(accl_range)(1 downto 0);

  -- Virtual Parallelism Signals
  signal halt_hart                       : std_logic_vector(accl_range); -- halts the thread when the requested functional unit is in use
  signal fu_req                          : array_2D(accl_range)(8 downto 0); -- Each threa has request bits equal to the total number of FUs
  signal fu_gnt                          : array_2D(accl_range)(8 downto 0); -- Each threa has grant bits equal to the total number of FUs
  signal fu_gnt_wire                     : array_2D(accl_range)(8 downto 0); -- Each threa has grant bits equal to the total number of FUs
  signal fu_gnt_en                       : array_2D(accl_range)(8 downto 0); -- Enable the giving of the grant to the thread pointed at by the issue buffer
  signal fu_rd_ptr                       : array_2D(8 downto 0)(TPS_BUF_CEIL-1 downto 0); -- five rd pointers each has a number of bits equal to ceil(log2(THREAD_POOL_SIZE-1))
  signal fu_wr_ptr                       : array_2D(8 downto 0)(TPS_BUF_CEIL-1 downto 0); -- five rd pointers each has a number of bits equal to ceil(log2(THREAD_POOL_SIZE-1))
  signal fu_issue_buffer                 : array_3D(8 downto 0)(THREAD_POOL_SIZE-2 downto 0)(TPS_CEIL-1 downto 0);
  signal hdc_sc_data_write_wire_int      : array_2d(accl_range)(SIMD_Width-1 downto 0);
  signal hdc_sc_data_write_int           : array_2d(accl_range)(SIMD_Width-1 downto 0);
  signal vec_write_rd_HDC                : std_logic_vector(accl_range);  -- Indicates whether the result being written is a vector or a scalar
  signal vec_read_rs1_HDC                : std_logic_vector(accl_range);  -- Indicates whether the operand being read is a vector or a scalar
  signal vec_read_rs2_HDC                : std_logic_vector(accl_range);  -- Indicates whether the operand being read is a vector or a scalar
  signal wb_ready                        : std_logic_vector(accl_range);
  signal halt_hdc                        : std_logic_vector(accl_range);
  signal halt_hdc_lat                    : std_logic_vector(accl_range);
  signal recover_state                   : std_logic_vector(accl_range);
  signal recover_state_wires             : std_logic_vector(accl_range);
  signal hdc_data_gnt_i_lat              : std_logic_vector(accl_range);
  signal hdc_except_data_wire            : array_2d(accl_range)(31 downto 0);
  signal decoded_instruction_DSP_lat     : array_2d(accl_range)(DSP_UNIT_INSTR_SET_SIZE -1 downto 0);
  signal overflow_rs1_sc                 : array_2d(accl_range)(Addr_Width downto 0);
  signal overflow_rs2_sc                 : array_2d(accl_range)(Addr_Width downto 0);
  signal overflow_rd_sc                  : array_2d(accl_range)(Addr_Width downto 0);
  signal hdc_rs1_to_sc                   : array_2d(accl_range)(SPM_ADDR_WID-1 downto 0);
  signal hdc_rs2_to_sc                   : array_2d(accl_range)(SPM_ADDR_WID-1 downto 0);
  signal hdc_rd_to_sc                    : array_2d(accl_range)(SPM_ADDR_WID-1 downto 0);
  signal hdc_sc_data_read_mask           : array_2d(accl_range)(SIMD_Width-1 downto 0);
  signal RS1_Data_IE_lat                 : array_2d(accl_range)(31 downto 0);
  signal RS2_Data_IE_lat                 : array_2d(accl_range)(31 downto 0);
  signal RS1_Data_ptr                    : array_2d(accl_range)(31 downto 0);
  signal RS2_Data_ptr                    : array_2d(accl_range)(31 downto 0);

  signal RD_Data_IE_lat                  : array_2d(accl_range)(Addr_Width -1 downto 0);
  signal BV_ptr                          : array_2d(THREAD_POOL_SIZE-1 downto 0)(Addr_Width downto 0);
  signal HVSIZE_READ                     : array_2d(accl_range)(Addr_Width downto 0);  -- Bytes remaining to read
  signal HVSIZE_READ_lat                 : array_2d(accl_range)(Addr_Width downto 0);  -- Bytes remaining to read
  signal HVSIZE_READ_MASK                : array_2d(accl_range)(Addr_Width downto 0);  -- Bytes remaining to read
  signal HVSIZE_READ_init                : array_2d(accl_range)(Addr_Width downto 0); -- condition
  signal HVSIZE_READ_init_clip                : array_2d(accl_range)(Addr_Width downto 0); -- condition
  signal HVSIZE_WRITE                    : array_2d(accl_range)(Addr_Width downto 0);  -- Bytes remaining to write
  signal MPSCLFAC_HDC                    : array_2d(accl_range)(4 downto 0);
  signal busy_hdc_internal               : std_logic_vector(accl_range);
  signal busy_HDC_internal_lat           : std_logic_vector(accl_range);
  signal rf_rs2                          : std_logic_vector(accl_range);
  signal SIMD_RD_BYTES_wire              : array_2d_int(accl_range);
  signal SIMD_RD_BYTES                   : array_2d_int(accl_range);
  
  ------------------ BUNDLING-- ------------------
  

    ------------------ FHRR BUNDLING-- ------------------
  constant INTEGER_BIT_WIDTH                  : integer := 2;
  constant FRACTIONAL_BIT_WIDTH               : integer := 16;
  constant INDEX_BIT_WIDTH                    : integer := 8;
  constant PI_REAL                            : real := 3.14159265358979323846;

  -- ============================================================================
  -- [Op-S1] Configurable LUT geometry knobs. Defaults reflect the post-Op-A1+A3
  -- optimised sizes. Changing these constants allows synthesis-time area/accuracy
  -- sweeps without RTL edits — useful for the future vivado-fpga-engineer M4-q
  -- breakdown campaign (e.g. measure LUT savings of dropping cos to 33-entry
  -- /1-octant + folded sym, or atan to 128-entry / Q1.13 width).
  -- ----------------------------------------------------------------------------
  -- Post-Op-A1: cos LUT stored only for the [0, pi/2] quadrant + boundary.
  -- 65 entries cover indices 0..64; quadrants 1..3 derived via 2-bit decode +
  -- conditional negation (see cos_lut_lookup function below).
  constant COS_LUT_QUADRANT_DEPTH             : integer := 65;
  -- Post-Op-A3: atan LUT stores arctan(40*i/255) for i in 0..255 as 17-bit
  -- unsigned (max value arctan(40) ~ 1.5458 rad ~= 0x18BFA fits in 17 bits).
  constant ATAN_LUT_DEPTH                     : integer := 256;
  constant ATAN_LUT_WIDTH                     : integer := 17;
  -- ============================================================================

  -- ============================================================================
  -- [Op-L5] ENCODE wide-streaming via dual SPM read ports (shovel-ready).
  -- ----------------------------------------------------------------------------
  -- The default ENCODE datapath streams one HV row per cycle: SPM channel 0 is
  -- used to read the (broadcast) scalar byte for the current feature, and
  -- channel 1 reads the SIMD-packed HV chunk. Latency = F*D/SIMD + ~5.
  --
  -- Architectural insight: the scalar is constant across the whole row, so it
  -- can be pre-loaded into a small register cache during a setup phase. With
  -- channel 0 freed, the main loop reads TWO HV rows per cycle (channels 0/1)
  -- and a 2x multiplier bank produces two contributions that collapse into a
  -- 3-operand accumulator add. Resulting latency: F + F*D/(2*SIMD) + ~5,
  -- ~2x speedup for typical F=8, D=1024 (1024 -> 520 cycles for SIMD=8).
  --
  -- LUT cost: 2x multiplier banks. With Op-A4 already mapping the multipliers
  -- to DSP48E2, this is +1 DSP/lane (still <2% of ZCU106 budget) and only the
  -- accumulator's 3-operand adder consumes incremental LUT.
  --
  -- Activation: set ENCODE_WIDE_STREAM = true. Default false keeps the legacy
  -- single-port datapath bit-exact (regression baseline = 5527 cycles for the
  -- fhrr_encode_test). Future activation requires updating the dsp_init/dsp_exec
  -- FSM (setup-then-process) and the SPM address arithmetic for the second row,
  -- which is left as future work; the present scaffolding only wires the
  -- compute-path scaffolding (signals + dual mult/accum generate branches) so
  -- the synthesis cost and timing of the wide datapath can be characterised.
  constant ENCODE_WIDE_STREAM                 : boolean := false;
  -- Upper bound for the per-feature scalar cache. MPSCLFAC is a 5-bit CSR,
  -- so the maximum number of features per ENCODE is 31. We size the cache
  -- to 32 entries (next power of two) so the indexing fits in 5 bits.
  constant ENC_SCALAR_CACHE_DEPTH             : integer := 32;
  -- ============================================================================

  -- ============================================================================
  -- [Op-S2 / Op-A5b] Divider backend selector. The radix-2 baseline is the
  -- default and remains the bit-exact reference for fhrr_clip_test. The
  -- CORDIC vectoring backend (Op-A5b) is wired and synthesisable; switching
  -- divider_kind to "cordic" replaces the divider16 + atan_lut chain with a
  -- pipelined 16-stage CORDIC, collapsing CLIP latency from ~50*D/P to
  -- (16 + D/P) cycles per element. Numerical tolerance: residual <= 1 ULP
  -- in Q0.8 phase, so the strict bit-exact regression will need its
  -- comparator relaxed when running with the CORDIC backend.
  -- Allowed values: "radix2"        (default, baseline; bit-exact reference).
  --                 "radix4_shared" (Op-A5a, future): 24-cycle radix-4 SRT,
  --                                  shared 4-instance scheduling on SIMD lanes.
  --                 "cordic"        (Op-A5b, shovel-ready): pipelined CORDIC
  --                                  vectoring, replaces divider + atan-LUT.
  constant divider_kind                       : string := "radix2";
  -- ============================================================================

  type trig_lut_t is array (0 to 255) of std_logic_vector(Data_Width - 1 downto 0);

  -- Op-A3 area optimization (2026-05-07): atan_lut compact storage.
  -- All atan(40*i/255) entries in Q1.16 are non-negative and bounded above by
  -- arctan(40) ~= 1.5458 rad -> Q1.16 magnitude <= 0x18BFA, fits in 17 bits.
  -- Storing as unsigned(ATAN_LUT_WIDTH-1 downto 0) removes 15 always-zero MSBs per
  -- entry (256*15 = 3840 bits saved). Sign is applied externally in the CLIP
  -- datapath, so this is bit-exact with the previous 32-bit signed storage.
  type atan_lut_t is array (0 to ATAN_LUT_DEPTH - 1) of unsigned(ATAN_LUT_WIDTH - 1 downto 0);

  function real_to_sfixed32(value : real; fractional_bits : natural) return std_logic_vector is
    variable scaled_value : integer;
  begin
    if value >= 0.0 then
      scaled_value := integer(value * real(2**fractional_bits) + 0.5);
    else
      scaled_value := integer(value * real(2**fractional_bits) - 0.5);
    end if;

    return std_logic_vector(to_signed(scaled_value, Data_Width));
  end function;

  -- Op-A3: helper that converts a non-negative real value into a 17-bit
  -- unsigned Q1.16 fixed-point word. Used only for atan_lut, where every
  -- entry is guaranteed non-negative by construction. Uses floor(x + 0.5)
  -- explicitly so the half-way case at value=0.0 rounds deterministically
  -- to 0 across simulators (some integer(real) implementations round 0.5
  -- to 0, others to 1 - we cannot rely on that for a bit-exact match
  -- with the reference hex table).
  function real_to_ufixed17(value : real; fractional_bits : natural) return unsigned is
    variable scaled_value : integer;
  begin
    scaled_value := integer(floor(value * real(2**fractional_bits) + 0.5));
    return to_unsigned(scaled_value, 17);
  end function;

  function gen_sine_lut_bundle return trig_lut_t is
    variable lut   : trig_lut_t;
    variable angle : real;
  begin
    for i in lut'range loop
      angle  := (2.0 * PI_REAL * real(i)) / 256.0;
      lut(i) := real_to_sfixed32(sin(angle), FRACTIONAL_BIT_WIDTH);
    end loop;

    return lut;
  end function;

  function gen_cosine_lut return trig_lut_t is
    variable lut   : trig_lut_t;
    variable angle : real;
  begin
    for i in lut'range loop
      angle  := (2.0 * PI_REAL * real(i)) / 256.0;
      lut(i) := real_to_sfixed32(cos(angle), FRACTIONAL_BIT_WIDTH);
    end loop;

    return lut;
  end function;

  -- Op-A3: emits a 17-bit unsigned ROM. Entries are non-negative magnitudes
  -- of arctan(40*i/255); external sign is applied at the CLIP read site.
  function gen_atan_lut return atan_lut_t is
    variable lut         : atan_lut_t;
    variable tangent_val : real;
  begin
    for i in lut'range loop
      tangent_val := (40.0 * real(i)) / 255.0;
      lut(i)      := real_to_ufixed17(arctan(tangent_val), FRACTIONAL_BIT_WIDTH);
    end loop;

    return lut;
  end function;

  -- ===========================================================================
  -- Trigonometric ROM (Op-A1 + Op-A2 + Op-A7 area optimization, 2026-05-07)
  -- ---------------------------------------------------------------------------
  -- A single 65-entry first-quadrant cosine ROM (Q1.16, indices 0..64) replaces
  -- the previous three 256-entry constants (sine_lut_bun, cosine_lut_bun,
  -- cosine_lut). For any 8-bit phase index k in [0, 256), cos(2*pi*k/256) is
  -- recovered through quadrant decoding (top 2 bits = quadrant, low 6 bits =
  -- in-quadrant offset) with a sign flip in quadrants 1 and 2:
  --   q=00, k in [  0, 64): addr = lo            sign = +
  --   q=01, k in [ 64,128): addr = 64 - lo       sign = -
  --   q=10, k in [128,192): addr = lo            sign = -
  --   q=11, k in [192,256): addr = 64 - lo       sign = +
  -- sin(2*pi*k/256) is recovered via sin(x) = cos(pi/2 - x), i.e.
  -- sin_lut_lookup(k) = cos_lut_lookup((64 - k) mod 256).
  -- Bit-exact with the original 3 ROMs by construction (cos[64]=cos[192]=0 is
  -- preserved through the quadrant-1/3 path because ROM[64]=0).
  -- ===========================================================================
  -- [Op-S1] Quadrant-folded cosine ROM: depth parameterized via COS_LUT_QUADRANT_DEPTH.
  type cos_q1_lut_t is array (0 to COS_LUT_QUADRANT_DEPTH - 1) of std_logic_vector(Data_Width - 1 downto 0);
  constant cosine_lut_q1 : cos_q1_lut_t := (
    0 => x"00010000",
    1 => x"0000FFEC",
    2 => x"0000FFB1",
    3 => x"0000FF4E",
    4 => x"0000FEC4",
    5 => x"0000FE13",
    6 => x"0000FD3B",
    7 => x"0000FC3B",
    8 => x"0000FB15",
    9 => x"0000F9C8",
    10 => x"0000F854",
    11 => x"0000F6BA",
    12 => x"0000F4FA",
    13 => x"0000F314",
    14 => x"0000F109",
    15 => x"0000EED9",
    16 => x"0000EC83",
    17 => x"0000EA0A",
    18 => x"0000E76C",
    19 => x"0000E4AA",
    20 => x"0000E1C6",
    21 => x"0000DEBE",
    22 => x"0000DB94",
    23 => x"0000D848",
    24 => x"0000D4DB",
    25 => x"0000D14D",
    26 => x"0000CD9F",
    27 => x"0000C9D1",
    28 => x"0000C5E4",
    29 => x"0000C1D8",
    30 => x"0000BDAF",
    31 => x"0000B968",
    32 => x"0000B505",
    33 => x"0000B086",
    34 => x"0000ABEB",
    35 => x"0000A736",
    36 => x"0000A268",
    37 => x"00009D80",
    38 => x"00009880",
    39 => x"00009368",
    40 => x"00008E3A",
    41 => x"000088F6",
    42 => x"0000839C",
    43 => x"00007E2F",
    44 => x"000078AD",
    45 => x"0000731A",
    46 => x"00006D74",
    47 => x"000067BE",
    48 => x"000061F8",
    49 => x"00005C22",
    50 => x"0000563E",
    51 => x"0000504D",
    52 => x"00004A50",
    53 => x"00004447",
    54 => x"00003E34",
    55 => x"00003817",
    56 => x"000031F1",
    57 => x"00002BC4",
    58 => x"00002590",
    59 => x"00001F56",
    60 => x"00001918",
    61 => x"000012D5",
    62 => x"00000C90",
    63 => x"00000648",
    64 => x"00000000"
  );

  -- Quadrant-decoded cosine lookup. Combinational, no extra cycle.
  function cos_lut_lookup(k : integer range 0 to 255)
    return std_logic_vector is
    variable q    : integer range 0 to 3;
    variable lo   : integer range 0 to 63;
    variable addr : integer range 0 to 64;
    variable raw  : signed(Data_Width - 1 downto 0);
  begin
    q  := k / 64;
    lo := k mod 64;
    case q is
      when 0 =>
        addr := lo;
        raw  :=  signed(cosine_lut_q1(addr));
      when 1 =>
        addr := 64 - lo;
        raw  := -signed(cosine_lut_q1(addr));
      when 2 =>
        addr := lo;
        raw  := -signed(cosine_lut_q1(addr));
      when others =>  -- q = 3
        addr := 64 - lo;
        raw  :=  signed(cosine_lut_q1(addr));
    end case;
    return std_logic_vector(raw);
  end function;

  -- Sine via sin(x) = cos(pi/2 - x); pi/2 corresponds to phase index 64.
  function sin_lut_lookup(k : integer range 0 to 255)
    return std_logic_vector is
    variable kc : integer range 0 to 255;
  begin
    kc := (64 - k) mod 256;          -- VHDL mod is non-negative for positive divisor
    return cos_lut_lookup(kc);
  end function;

  -- Op-A3 (2026-05-07): atan_lut compacted from 256x32 signed to 256x17
  -- unsigned. Entries are now generated by gen_atan_lut at elaboration time
  -- (no behavioural change vs. the previous hand-coded hex table - legacy
  -- table archived in RTL-FHRR_Unit.vhd.backup for traceability).
  constant atan_lut      : atan_lut_t := gen_atan_lut;
 
  signal bundle_en                        : std_logic_vector(accl_range); -- enables the use of the adders
  signal bundle_en_wire                   : std_logic_vector(accl_range); -- enables the use of the adders
  signal bundle_en_pending                : std_logic_vector(accl_range); -- signal to preserve the request to access the adder "multhithreaded mode" only
  signal bundle_en_pending_wire           : std_logic_vector(accl_range); -- signal to preserve the request to access the adder "multhithreaded mode" only
  signal busy_bundle                      : std_logic; -- busy signal active only when the FU is shared and currently in use 
  signal busy_bundle_wire                 : std_logic; -- busy signal active only when the FU is shared and currently in use 

  signal bundle_stage_1_en               : std_logic_vector(accl_range);
  signal bundle_stage_2_en               : std_logic_vector(accl_range);
  signal bundle_stage_3_en               : std_logic_vector(accl_range);

  --signal hdcu_in_bundle_operands         : array_3d(fu_range)(1 downto 0)(SIMD_Width - 1 downto 0);
  signal hdcu_in_bundle_operand            : array_2d(fu_range)((8*SIMD_Width)/Data_Width-1 downto 0);
  signal hdcu_in_bundled                   : array_2d(fu_range)(SIMD_Width - 1 downto 0);
  signal bundle_phase_hold                 : array_2d(accl_range)(SIMD_Width - 1 downto 0);
  signal hdcu_in_bundle_operands_masked  : array_3d(fu_range)(1 downto 0)((INTEGER_BIT_WIDTH + FRACTIONAL_BIT_WIDTH)*SIMD - 1 downto 0);

  signal index_operand_0                      : array_2d(fu_range)(INDEX_BIT_WIDTH*SIMD - 1  downto 0);

  signal hdcu_in_bundle_operand_real     : array_2d(fu_range)(SIMD_Width - 1 downto 0);
  signal hdcu_in_bundle_operand_imag     : array_2d(fu_range)(SIMD_Width - 1 downto 0);

  signal real_accum                           : array_2d(fu_range)(SIMD_Width - 1 downto 0);
  signal imag_accum                           : array_2d(fu_range)(SIMD_Width - 1 downto 0);
  signal hdcu_bundle_perf_counter_int    : array_2d(accl_range)(31 downto 0);

  ------------------------------------------------
  ------------------ BINDING ---------------------

  signal busy_bind                        : std_logic;                     -- busy signal active only when the FU is shared and currently in use 
  signal busy_bind_wire                   : std_logic;                     -- busy signal active only when the FU is shared and currently in use 
  signal bind_en                           : std_logic_vector(accl_range);  -- enables the use of the multipliers
  signal bind_en_wire                     : std_logic_vector(accl_range);  -- enables the use of the multipliers
  signal bind_en_pending                  : std_logic_vector(accl_range);  -- signal to preserve the request to access the multiplier "multhithreaded mode" only
  signal bind_en_pending_wire             : std_logic_vector(accl_range);  -- signal to preserve the request to access the multiplier "multhithreaded mode" only

  signal bind_stage_1_en                  : std_logic_vector(accl_range);
  signal bind_stage_2_en                  : std_logic_vector(accl_range);
  signal bind_stage_3_en                  : std_logic_vector(accl_range);

  signal hdcu_in_bind_operands            : array_3d(fu_range)(1 downto 0)((8*SIMD_Width)/Data_Width-1 downto 0);
  signal hdcu_in_bind_operands_lat        : array_3d(fu_range)(1 downto 0)((8*SIMD_Width)/Data_Width-1 downto 0);
  signal hdc_out_bind_results_wire        : array_2d(fu_range)((8*SIMD_Width)/Data_Width-1 downto 0);
  signal hdc_out_bind_results             : array_2d(fu_range)((8*SIMD_Width)/Data_Width-1 downto 0);
  signal hdcu_bind_perf_counter_int    : array_2d(accl_range)(31 downto 0);

  ------------------------------------------------

  ------------------ FHRR_ENCODING ---------------------
  signal busy_enc                        : std_logic;                     -- busy signal active only when the FU is shared and currently in use 
  signal busy_enc_wire                   : std_logic;                     -- busy signal active only when the FU is shared and currently in use 
  signal enc_en                          : std_logic_vector(accl_range);  -- enables the use of the multipliers
  signal enc_en_wire                     : std_logic_vector(accl_range);  -- enables the use of the multipliers
  signal enc_en_pending                  : std_logic_vector(accl_range);  -- signal to preserve the request to access the multiplier "multhithreaded mode" only
  signal enc_en_pending_wire             : std_logic_vector(accl_range);  -- signal to preserve the request to access the multiplier "multhithreaded mode" only

  signal enc_stage_1_en                   : std_logic_vector(accl_range);
  signal enc_stage_2_en                   : std_logic_vector(accl_range);
  signal enc_stage_3_en                   : std_logic_vector(accl_range);
  signal enc_stage_4_en                   : std_logic_vector(accl_range);
  signal enc_row_count                    : array_2d(accl_range)(Data_Width - 1 downto 0);


  signal hdcu_in_enc_fpp_operands             : array_3d(fu_range)(1 downto 0)(SIMD_Width - (Data_Width-8)*SIMD_Width/Data_Width -1 downto 0);
  signal mult_result                          : array_2d(fu_range)(2*SIMD_Width - (Data_Width-8)*2*SIMD_Width/Data_Width -1 downto 0) ; -- eg in 32 bit operands, 64-bit multiplication result
  -- [Op-L5] Wide-stream dormant scaffolding. mult_result_b is the second
  -- multiplier-bank product (feature f+1 contribution) consumed when
  -- ENCODE_WIDE_STREAM = true. Sized identically to mult_result so DSP
  -- inference rules (Op-A4) apply uniformly. When the wide-stream branch is
  -- not generated the signal is unused and Vivado prunes it; the declaration
  -- remains compile-clean even with the dormant constant set to false.
  signal mult_result_b                        : array_2d(fu_range)(2*SIMD_Width - (Data_Width-8)*2*SIMD_Width/Data_Width -1 downto 0);
  signal trunc_mul_results_b                  : array_2d(fu_range)(SIMD_Width - (Data_Width-8)*SIMD_Width/Data_Width -1 downto 0);
  -- [Op-L5] Per-feature scalar register cache. Filled during the setup phase
  -- of the wide-stream datapath via SPM channel 0; in the main loop both
  -- channel 0 and channel 1 are repurposed to fetch HV rows in parallel and
  -- the scalars are sourced from this register file (one byte per feature).
  -- Indexed by feature-pair counter inside the multiplier_unit_wide branch.
  signal enc_scalar_cache                     : array_2d(fu_range)(8*ENC_SCALAR_CACHE_DEPTH - 1 downto 0);
  -- [Op-L5] Feature-pair counter (0..F/2-1) used by the wide-stream FSM to
  -- index the scalar cache and address the two HV rows.
  signal enc_feature_pair_idx                 : array_2d(fu_range)(4 downto 0);
  -- [Op-A4] Force Vivado to map the ENCODE 8x8 signed multiplier to DSP48E2 slices
  -- instead of LUT-based multipliers. This is a synthesis attribute (Xilinx-specific):
  -- ignored by ModelSim during simulation (so the bit-exact behaviour is unchanged),
  -- but Vivado infers a DSP slice per lane in synth, freeing ~110 LUT/lane.
  -- For SIMD=32 (paper config) this saves ~3.5k LUT, replacing them with 32 DSP slices
  -- (out of 1728 available on ZCU106 = 1.85% DSP utilisation).
  attribute use_dsp : string;
  attribute use_dsp of mult_result : signal is "yes";
  -- [Op-L5] Apply the same DSP-inference attribute to the second multiplier
  -- bank, so the dormant wide-stream datapath synthesises onto DSP48E2 slices
  -- rather than LUT-based multipliers when ENCODE_WIDE_STREAM = true.
  attribute use_dsp of mult_result_b : signal is "yes";
  signal trunc_mul_results                    : array_2d(fu_range)(SIMD_Width - (Data_Width-8)*SIMD_Width/Data_Width -1 downto 0);
  signal trunc_mul_results_temp               : array_2d(fu_range)(7 downto 0);
  signal fhr_dot_product                      : array_2d(fu_range)(7 downto 0);

  signal accumulator_wire                     : array_2d(fu_range)(SIMD_Width - (Data_Width-8)*SIMD_Width/Data_Width -1 downto 0);
  signal accumulator_reg                      : array_2d(fu_range)(SIMD_Width - (Data_Width-8)*SIMD_Width/Data_Width -1 downto 0);
  signal hdc_out_enc_result                   : array_2d(fu_range)(SIMD_Width - (Data_Width-8)*SIMD_Width/Data_Width -1 downto 0);  

  
  constant TOTAL_BIT_WIDTH     : integer := 32;
  constant PRECISION_BIT_WIDTH : integer := 16;

  signal hdcu_enc_perf_counter_int    : array_2d(accl_range)(31 downto 0);
  ------------------------------------------------

  ------------------ SIMILARITY ------------------
  signal busy_sim                        : std_logic; 
  signal busy_sim_wire                   : std_logic;
  signal sim_en                          : std_logic_vector(accl_range); 
  signal sim_en_wire                     : std_logic_vector(accl_range);
  signal sim_en_pending                  : std_logic_vector(accl_range); 
  signal sim_en_pending_wire             : std_logic_vector(accl_range);

  signal sim_stage_1_en                  : std_logic_vector(accl_range);
  signal sim_stage_2_en                  : std_logic_vector(accl_range);
  signal sim_stage_3_en                  : std_logic_vector(accl_range);
  -- [Op-L1] Extra pipeline stage enable for SIMD>=4 (mid-tree register split).
  signal sim_stage_4_en                  : std_logic_vector(accl_range);

  ---------------cosine similarity-------------------------------
  
  --FUNCTIONS USED IN FOR COSINE SIMILARITY
  -- Function: clog2
  -- Returns the ceiling of log2(n). Evaluated at synthesis time.
  ----------------------------------------------------------------------------
    function clog2(n : integer) return integer is
    variable i : integer := 0;
    variable v : integer := n - 1;
    begin
      while v > 0 loop
        v := v / 2;
        i := i + 1;
      end loop;
      return i;
    end function;

    function log2_power_of_2(input_val : integer) return integer is
    variable tmp_val : integer := input_val;
    variable result : integer := 0;
        begin
          for i in 0 to 31 loop
            exit when tmp_val <= 1;
            tmp_val := tmp_val / 2;
            result := result + 1;
          end loop;
        return result;
    end function;
  
    function max(a : integer; b : integer) return integer is
    begin
      if a > b then
        return a;
      else
        return b;
      end if;
    end function;
  
    -- Helper functions to compute the width at each stage.
    ----------------------------------------------------------------------------
    function stage_in_width(level: integer) return integer is
        variable num_in : integer := SIMD_Width/Data_Width / (2 ** level);
    begin
        return num_in * Data_Width;
    end function;
    
    function stage_out_width(level: integer) return integer is
        variable num_in : integer := SIMD_Width /Data_Width/ (2 ** level);
    begin
        return (num_in / 2) * Data_Width ;
    end function;


    constant NUM_LEVELS            : integer := max(1, clog2(SIMD_Width / Data_Width)); --making sure that numlevels can't be 1 to avoid compile time error in declaring stage array signal
    constant MAX_WIDTH             : integer := Simd_Width;     -- Used for intermediate stage sizing.

    -- [Op-L1] Extra mid-tree pipeline stage in the SIMILARITY reduction.
    -- When NUM_LEVELS >= 2 (i.e. SIMD >= 4) the combinational chain
    --   delta_sub -> cos_lut -> tree-adder(log2(SIMD) levels) -> accumulator-add
    -- is broken by an extra register between the per-grant tree-adder output
    -- and the cosine_accumulator. For SIMD=2 (NUM_LEVELS=1) the extra stage
    -- is disabled so the latency formula is unchanged for the current build.
    -- VHDL-93 friendly conditional via a small function; max() keeps it pure.
    function sim_pipe_extra_fn(levels : integer) return integer is
    begin
      if levels >= 2 then
        return 1;
      else
        return 0;
      end if;
    end function;
    constant SIM_PIPE_EXTRA        : integer := sim_pipe_extra_fn(NUM_LEVELS);

    signal hdcu_in_sim_operands            : array_3d(fu_range)(1 downto 0)(SIMD_Width - (Data_Width-8)*SIMD_Width/Data_Width -1 downto 0);
    signal delta                           :  array_2d(fu_range)(SIMD_Width - (Data_Width-8)*SIMD_Width/Data_Width - 1 downto 0);
    signal delta_map                       :  array_2d(fu_range)(SIMD_Width - (Data_Width-8)*SIMD_Width/Data_Width downto 0);
    signal cosine_diff                     :  array_2d(fu_range)(SIMD_Width - 1 downto 0);
    signal HV_ELEMENT                      : array_2d(accl_range)(Data_Width -1 downto 0); 
    
    --for multistage tree adder
    type stage_array_type is array (natural range <>) of unsigned(MAX_WIDTH-1 downto 0);    
    signal stage_array: stage_array_type(0 to NUM_LEVELS-1) := (others => (others => '0'));
    signal valid_pipe : std_logic_vector(NUM_LEVELS downto 0) := (others => '0');
    
    component tree_adder is
      generic (
        NUM_INPUTS : integer;
        DATA_WIDTH : integer
      );
      port (
        clk      : in std_logic;
        rst_n    : in std_logic;
        in_data  : in  unsigned((NUM_INPUTS*DATA_WIDTH)-1 downto 0);
        out_data : out unsigned(((NUM_INPUTS/2)*DATA_WIDTH)-1 downto 0)
      );
    end component;

    -- Signal Declarations for accumulation
    signal cosine_diff_flat   : unsigned(SIMD_Width - 1 downto 0);
    signal cosine_accumulator_wire            :  array_2d(fu_range)(Data_Width-1 downto 0);
    signal cosine_accumulator_reg            :  array_2d(fu_range)(Data_Width-1  downto 0);
    signal cosine_accumulator_avg            :  array_2d(fu_range)(Data_Width-1 downto 0);
    -- Per-grant combinational sum of cosine_lut values across the SIMD lanes.
    signal sim_cos_sum_wire                  : array_2d(fu_range)(Data_Width - 1 downto 0);
    -- [Op-L1] Registered copy of sim_cos_sum_wire used when SIM_PIPE_EXTRA=1
    -- to cut the long combinational LUT+tree-adder chain before the
    -- cosine accumulator. For SIMD=2 it is unused (synth will optimise away).
    signal sim_cos_sum_reg                   : array_2d(fu_range)(Data_Width - 1 downto 0);
    -- [Op-L1] One-cycle-delayed grant-valid token used to gate the accumulator
    -- when consuming sim_cos_sum_reg.
    signal sim_cos_sum_valid                 : std_logic_vector(fu_range);
    signal sim_chunk_count                   : array_2d(fu_range)(Data_Width - 1 downto 0);

    -- [SIMILARITY timing fix 20260507] Pre-computed/registered geometry of the
    -- similarity accumulation, so the long combinational chain
    --     HV_ELEMENT_reg --> log2_power_of_2 + variable barrel-shifter -->
    --     cosine_accumulator_avg_reg
    -- (77 logic levels in the post-Op-N2 round-3 timing report) is broken
    -- into shorter sequential stages. Both signals are derived from
    -- HVSIZE(harc_EXEC) once at HVSIM dispatch (in dsp_init) and remain
    -- stable for the duration of the SIMILARITY iteration.
    signal sim_hv_log2                       : array_2d(accl_range)(4 downto 0); -- log2(HV_ELEMENT), max 31
    signal sim_valid_chunks                  : array_2d(accl_range)(Data_Width - 1 downto 0);

    signal hdcu_sim_perf_counter_int    : array_2d(accl_range)(31 downto 0);
  
  --------------------------------------------------------------

  -------------------------- CLIPPING --------------------------
  signal busy_clip                       : std_logic;
  signal busy_clip_wire                  : std_logic;
  signal clip_en                         : std_logic_vector(accl_range);
  signal clip_en_wire                    : std_logic_vector(accl_range);
  signal clip_en_pending                 : std_logic_vector(accl_range);
  signal clip_en_pending_wire            : std_logic_vector(accl_range);
  
  signal clip_stage_1_en                 : std_logic_vector(accl_range);
  signal clip_stage_2_en                 : std_logic_vector(accl_range);
  signal clip_stage_3_en                 : std_logic_vector(accl_range);
  signal hdcu_in_clip_operands             : array_3d(fu_range)(1 downto 0)(SIMD_Width - 1 downto 0);
  
  signal tangent                                  : array_2d(fu_range)(SIMD_Width-1 downto 0);
  signal tang_map                                 : array_2d(fu_range)(SIMD_Width-1 downto 0);
  
   
-- signal dividend_scalar : std_logic_vector(Data_Width + PRECISION_BIT_WIDTH -1 downto 0);
-- signal divisor_scalar  : std_logic_vector(Data_Width + PRECISION_BIT_WIDTH -1 downto 0);
-- signal div_enable      : std_logic;
-- signal div_done        : std_logic;
-- signal div_result      : std_logic_vector((32+16)*2 -1 downto 0);

signal dividend         : array_3d(fu_range)(SIMD -1 downto 0)((Data_Width + PRECISION_BIT_WIDTH) -1 downto 0);
signal divisor          : array_3d(fu_range)(SIMD -1 downto 0)((Data_Width + PRECISION_BIT_WIDTH) -1 downto 0);
signal div_enable      :  array_3d(fu_range)(SIMD -1 downto 0)(1 downto 0);
signal div_done        : array_3d(fu_range)(SIMD -1 downto 0)(1 downto 0);
signal div_done_lat    : array_3d(accl_range)(SIMD -1 downto 0)(1 downto 0);
signal div_done_lat2   : array_3d(accl_range)(SIMD -1 downto 0)(1 downto 0);
signal div_result      : array_3d(fu_range)(SIMD -1 downto 0)((32+16)*2 -1 downto 0);
signal div_negate      : array_3d(fu_range)(SIMD -1 downto 0)(0 downto 0);

-- [Op-A5b] CORDIC vectoring backend per-lane I/O. Synthesised away when
-- divider_kind /= "cordic" because the corresponding generate evaluates
-- to false and these signals end up unconnected stubs (driven to 0 in
-- divider_radix2_gen for bit-level tooling friendliness).
signal cordic_angle    : array_3d(fu_range)(SIMD -1 downto 0)(Data_Width -1 downto 0);
signal cordic_valid    : array_3d(fu_range)(SIMD -1 downto 0)(0 downto 0);

 -- Divider component declaration
    component divider16
        Port (
            dividend_i             : in  STD_LOGIC_VECTOR(47 downto 0);
            divisor_i              : in  STD_LOGIC_VECTOR(47 downto 0);
            reset                  : in  STD_LOGIC;
            clk                    : in  STD_LOGIC;
            div_enable_i           : in  STD_LOGIC;
            division_finished_out  : out STD_LOGIC;
            result                 : out STD_LOGIC_VECTOR(95 downto 0)
        );
    end component;

  signal hdcu_out_clip_results             : array_2d(fu_range)(SIMD_Width - 1 downto 0);

  signal harc_f                            : array_2d_int(accl_range);
  -----------------------------------------------------------------

signal hdcu_clip_perf_counter_int    : array_2d(accl_range)(31 downto 0); -- Performance counter for the HDCU
  -------------------------------------------------------------------------

  ------------------ PERMUTATION ---------------------
  -- [Op-N2 Phase A] Cyclic-shift FU (HVPERM). Shape mirrors BIND but rs1 carries
  -- a scalar shift amount (vec_read_rs1_ID = '0'); only rs2 is read from SPM.
  signal busy_perm                   : std_logic;
  signal busy_perm_wire              : std_logic;
  signal perm_en                     : std_logic_vector(accl_range);
  signal perm_en_wire                : std_logic_vector(accl_range);
  signal perm_en_pending             : std_logic_vector(accl_range);
  signal perm_en_pending_wire        : std_logic_vector(accl_range);
  signal perm_stage_1_en             : std_logic_vector(accl_range);
  -- [Op-N2 Phase B] Two-cycle pipeline: stage_1 latches the SPM word into
  -- hdcu_in_perm_operand_b (chunk_b for the current output), stage_2 enables
  -- the writeback once chunk_a (current SPM word) is also valid.
  signal perm_stage_2_en             : std_logic_vector(accl_range);
  -- [Op-N2 Phase A bug fix] match the BIND packed-byte layout: each 32-bit SPM lane
  -- holds 1 phase byte in its low 8 bits; the FU operates on a packed representation
  -- of SIMD bytes = (8*SIMD_Width)/Data_Width bits, so within-chunk byte rotation
  -- equals 1-phase rotation per step.
  signal hdcu_in_perm_operand        : array_2d(fu_range)((8*SIMD_Width)/Data_Width - 1 downto 0);
  -- [Op-N2 Phase B] Pipelined "previous chunk" register. The SPM bank can only
  -- serve one address per cycle (the SCI's two DSP read channels share the same
  -- physical bank read port — see RTL-Scratchpad_Memory_Interface.vhd, port 1
  -- overwrites port 0's address). So Phase B streams chunks sequentially on
  -- channel 1 only, latching last cycle's chunk as chunk_b and using the
  -- current SPM word as chunk_a (next neighbour). The splice combines them.
  signal hdcu_in_perm_operand_b      : array_2d(fu_range)((8*SIMD_Width)/Data_Width - 1 downto 0);
  signal hdc_out_perm_results_wire   : array_2d(fu_range)((8*SIMD_Width)/Data_Width - 1 downto 0);
  signal hdc_out_perm_results        : array_2d(fu_range)((8*SIMD_Width)/Data_Width - 1 downto 0);
  signal hdcu_perm_perf_counter_int  : array_2d(accl_range)(31 downto 0);
  -- [Op-N2 Phase B] Latched cyclic-shift parameters (computed at dsp_init from
  -- RS1[7:0] and HVSIZE so dsp_exec address arithmetic and the splice mux do not
  -- recompute divisions on the critical path).
  --   perm_chunk_count_int : N = HVSIZE_bytes / SIMD_RD_BYTES (number of chunks)
  --   perm_chunk_shift_int : shift in chunks = (shift_phase / SIMD) mod N
  --   perm_intra_shift_int : shift in phases inside a chunk = shift_phase mod SIMD
  -- Initialised to 0 so the perm_shift_comb mux is well-defined at sim start
  -- (and synth tools see a defined power-up value); the reset block in the
  -- main FSM does not write integer types.
  signal perm_chunk_count_int        : array_2d_int(accl_range) := (others => 0);
  signal perm_chunk_shift_int        : array_2d_int(accl_range) := (others => 0);
  signal perm_intra_shift_int        : array_2d_int(accl_range) := (others => 0);
  -- [Op-N2 Phase B timing fix 20260507] Registered chunk-index counter to
  -- replace the combinational div+mod+mul chain on the SPM read-address
  -- critical path (was ~196 logic levels / 152 CARRY8 chains, WNS=-22.124 ns
  -- @ P=1). The counter is initialised at HVPERM dispatch (perm_en rising
  -- edge) and incremented mod-N on each successful SPM grant in dsp_exec.
  -- The remaining combinational expression is a single multiply by
  -- SIMD_RD_BYTES (a power-of-2 shift in synthesis) plus one base add.
  --   perm_next_read_chunk_int(h) : chunk index used by hdc_sc_read_addr(h)(1)
  --                                 in the next dsp_exec cycle (k-th read).
  --   perm_read_chunk_idx_int(h)  : reserved for diagnostics / future use
  --                                 (current read index, mirrors the SW model
  --                                 of the read counter k).
  signal perm_read_chunk_idx_int     : array_2d_int(accl_range) := (others => 0);
  signal perm_next_read_chunk_int    : array_2d_int(accl_range) := (others => 0);
  -- One-cycle-delayed copy of perm_en used to detect the registered rising
  -- edge of perm_en (and so the first cycle in which perm_chunk_count_int /
  -- perm_chunk_shift_int are guaranteed valid).
  signal perm_en_prev                : std_logic_vector(accl_range) := (others => '0');
  ----------------------------------------------------

  ---------------- HDCU Performance Counters ----------------
signal hdcu_performance_counter_int        : array_2d(accl_range)(31 downto 0); -- Performance counter for the HDCU

---------------------------------- HDC ARCHITECTURE BEGIN -------------------------------------------

begin
  busy_hdc <= busy_hdc_internal;

  bundle_phase_hold_latch : process (all)
  begin
    if rst_ni = '0' then
      bundle_phase_hold <= (others => (others => '0'));
    else
      for h in accl_range loop
        if decoded_instruction_DSP_lat(h)(HVBUNDLE_bit_position) = '1' and wb_ready(h) = '1'
           and HVSIZE_WRITE(h) /= (Addr_Width downto 0 => 'U')
           and (((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_WRITE(h)))) / SIMD_RD_BYTES_wire(h)) mod 2 = 0) then
          bundle_phase_hold(h) <= hdc_sc_data_read(h)(1);
        end if;
      end loop;
    end if;
  end process;

  HDC_replicated : for h in accl_range generate
    harc_f(h) <= 0 when multithreaded_accl_en = 1 else h;
  
  ----------------------------- Sequential Stage of HDC Unit ----------------------------------------

  dsp_exec_Unit : process(clk_i, rst_ni)  -- single cycle unit, fully synchronous 
    variable enc_next_row : integer;
  begin
    if rst_ni = '0' then
      rf_rs2(h)     <= '0';
      recover_state(h) <= '0';
    elsif rising_edge(clk_i) then
      
      HVSIZE_READ_lat(h) <= HVSIZE_READ(h);

      if hdc_instr_req(h) = '1' or busy_HDC_internal_lat(h) = '1' then  

        case state_HDC(h) is

          when dsp_init =>

            -------------------------------------------------------------
            --  ██╗███╗   ██╗██╗████████╗    ██████╗ ███████╗██████╗   --
            --  ██║████╗  ██║██║╚══██╔══╝    ██╔══██╗██╔════╝██╔══██╗  --
            --  ██║██╔██╗ ██║██║   ██║       ██║  ██║███████╗██████╔╝  --
            --  ██║██║╚██╗██║██║   ██║       ██║  ██║╚════██║██╔═══╝   --
            --  ██║██║ ╚████║██║   ██║       ██████╔╝███████║██║       --
            --  ╚═╝╚═╝  ╚═══╝╚═╝   ╚═╝       ╚═════╝ ╚══════╝╚═╝       -- 
            -------------------------------------------------------------

            if decoded_instruction_DSP(HVCLIP_bit_position  ) = '1'   then
              rf_rs2(h) <= '1';
            else
              rf_rs2(h) <= '0';  
            end if;

            -- We backup data from decode stage since they will get updated
            HVSIZE_READ_MASK(h) <= HVSIZE(harc_EXEC);
            MPSCLFAC_HDC(h) <= MPSCLFAC(harc_EXEC); -- Contiene il numero di classi

            -- When the decoded instruction is a bundle, we need to multiply the HVSIZE_WRITE by (Data_Width/COUNTERS_NUMBER)
            if decoded_instruction_DSP(HVBUNDLE_bit_position)    = '1' then
            
             
                
             HVSIZE_WRITE(h) <= std_logic_vector(shift_left(resize(unsigned(HVSIZE(harc_EXEC)), HVSIZE_WRITE(h)'length), 1));

            
            elsif decoded_instruction_DSP(HVSIM_bit_position)    = '1'  then
             
            HVSIZE_WRITE(h) <= std_logic_vector(to_unsigned(4, HVSIZE_WRITE(h)'length));
                  
            --FHRR ENCODING
            elsif decoded_instruction_DSP(HVENC_bit_position )  = '1' then 
            HVSIZE_WRITE(h) <= std_logic_vector(resize(unsigned(HVSIZE(h)), HVSIZE_WRITE(h)'length));
                   
            elsif decoded_instruction_DSP(HVBIND_bit_position )  = '1' then
            HVSIZE_WRITE(h) <= std_logic_vector(resize(unsigned(HVSIZE(h)), HVSIZE_WRITE(h)'length));

             elsif decoded_instruction_DSP(HVCLIP_bit_position )  = '1' then
            HVSIZE_WRITE(h) <= std_logic_vector(resize(unsigned(HVSIZE(h)), HVSIZE_WRITE(h)'length));

            -- [Op-N2 Phase A] HVPERM writes one HV worth of bytes (same shape as BIND)
            elsif decoded_instruction_DSP(HVPERM_bit_position) = '1' then
            HVSIZE_WRITE(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)), HVSIZE_WRITE(h)'length));
            else
              HVSIZE_WRITE(h) <= HVSIZE(harc_EXEC);
            end if;

            decoded_instruction_DSP_lat(h)  <= decoded_instruction_DSP;

            vec_write_rd_HDC(h) <= vec_write_rd_ID;

            vec_read_rs1_HDC(h) <= vec_read_rs1_ID;
            vec_read_rs2_HDC(h) <= vec_read_rs2_ID;

            hdc_rs1_to_sc(h) <= rs1_to_sc;
            hdc_rs2_to_sc(h) <= rs2_to_sc;
            hdc_rd_to_sc(h)  <= rd_to_sc;
            RD_Data_IE_lat(h) <= RD_Data_IE;

            -- [Op-N2 Phase B] Capture HVPERM cyclic-shift parameters at dsp_init.
            -- shift_phase is taken from RS1[7:0] (unsigned, normalized mod D in SW).
            -- chunk_count = HVSIZE_bytes / SIMD_RD_BYTES (= N, number of SIMD chunks).
            -- chunk_shift = (shift_phase / SIMD) mod N
            -- intra_shift = shift_phase mod SIMD
            -- Note: SIMD>0 is enforced by the package; HVSIZE>0 by the dsp_init guard.
            if decoded_instruction_DSP(HVPERM_bit_position) = '1' then
              if to_integer(unsigned(HVSIZE(harc_EXEC))) > 0 and
                 SIMD_RD_BYTES_wire(h) > 0 then
                perm_chunk_count_int(h) <= to_integer(unsigned(HVSIZE(harc_EXEC))) / SIMD_RD_BYTES_wire(h);
                if (to_integer(unsigned(HVSIZE(harc_EXEC))) / SIMD_RD_BYTES_wire(h)) > 0 then
                  perm_chunk_shift_int(h) <=
                    (to_integer(unsigned(RS1_Data_IE(15 downto 0))) / SIMD)
                    mod (to_integer(unsigned(HVSIZE(harc_EXEC))) / SIMD_RD_BYTES_wire(h));
                else
                  perm_chunk_shift_int(h) <= 0;
                end if;
                perm_intra_shift_int(h) <= to_integer(unsigned(RS1_Data_IE(15 downto 0))) mod SIMD;
              else
                perm_chunk_count_int(h) <= 0;
                perm_chunk_shift_int(h) <= 0;
                perm_intra_shift_int(h) <= 0;
              end if;
            end if;
            
            

            -- Increment the read addresses if there is a data grant
            if hdc_data_gnt_i(h) = '1' then
              
              ------------------ Source Register 1 ------------------
              if vec_read_rs1_ID = '1'  then
                --FHRR ENCODING
                if decoded_instruction_DSP(HVENC_bit_position ) = '1'then 
                  RS1_Data_IE_lat(h) <= RS1_Data_IE;
                  RS1_Data_ptr(h) <= RS1_Data_IE;
                
                 --FHRR BUNDLING
                elsif decoded_instruction_DSP(HVBUNDLE_bit_position ) = '1'then 
                  RS1_Data_IE_lat(h) <= RS1_Data_IE;
                
                --CLIPPING BUNDLING
                elsif decoded_instruction_DSP(HVCLIP_bit_position ) = '1'then 
                  RS1_Data_IE_lat(h) <= RS1_Data_IE;
                 
                else
                  RS1_Data_IE_lat(h) <= std_logic_vector(unsigned(RS1_Data_IE) + SIMD_RD_BYTES_wire(h));  -- source 1 address increment
                end if;

              else
                RS1_Data_IE_lat(h) <= RS1_Data_IE;
              end if;
              -------------------------------------------------------

              ------------------ Source Register 2 ------------------
              if vec_read_rs2_ID = '1'  then


                -- FHRR ENCODING
                if decoded_instruction_DSP(HVENC_bit_position) = '1'then
                  RS2_Data_IE_lat(h) <= RS2_Data_IE;
                  RS2_Data_ptr(h) <= RS2_Data_IE;

                -- FHRR BUNDLING
                elsif decoded_instruction_DSP(HVBUNDLE_bit_position) = '1'then
                RS2_Data_IE_lat(h) <= RS2_Data_IE;

                 -- FHRR BUNDLING
                elsif decoded_instruction_DSP(HVCLIP_bit_position) = '1'then
                RS2_Data_IE_lat(h) <= RS2_Data_IE;
                -- [Op-N2 Phase B] HVPERM keeps RS2_Data_IE_lat pinned at the SPM
                -- base address. dsp_exec computes addr_b/addr_a from chunk_i and
                -- the latched chunk_shift via modular arithmetic, so a per-cycle
                -- linear increment would corrupt the wrap-around.
                elsif decoded_instruction_DSP(HVPERM_bit_position) = '1' then
                  RS2_Data_IE_lat(h) <= RS2_Data_IE;
                else
                  RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE) + SIMD_RD_BYTES_wire(h));
                end if;
              else
                RS2_Data_IE_lat(h) <= RS2_Data_IE;
              end if;
              -------------------------------------------------------

              -- Decrement the vector elements that have already been operated on   
              if  (decoded_instruction_DSP(HVCLIP_bit_position)    = '1') then
                    HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC) )*(32+2+PRECISION_BIT_WIDTH)  + 8*SIMD_RD_BYTES_wire(h) , HVSIZE_READ(h)'length));
                    HVSIZE_READ_init(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC) )*(32+2+PRECISION_BIT_WIDTH)  + 8*SIMD_RD_BYTES_wire(h) , HVSIZE_READ(h)'length));
                    HVSIZE_READ_init_clip(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC) )*(32+2+PRECISION_BIT_WIDTH)  + 8*SIMD_RD_BYTES_wire(h) , HVSIZE_READ(h)'length));
              --FHRR ENCODING
              elsif decoded_instruction_DSP(HVENC_bit_position )  = '1' then
                
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * unsigned(MPSCLFAC(h)), HVSIZE_READ(h)'length)); 
                HVSIZE_READ_init(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * unsigned(MPSCLFAC(h)), HVSIZE_READ(h)'length)); 
              
             
              ---COSINE SIMILARITY
              elsif decoded_instruction_DSP(HVSIM_bit_position )  = '1' then
                if SIMD > 1 then
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC))+ log2_power_of_2(SIMD)*SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
                else 
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC))+ 4, HVSIZE_READ(h)'length));
                end if;
              HV_ELEMENT(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC))/4,  HV_ELEMENT(h)'length));
              -- [SIMILARITY timing fix] Pre-compute log2(HV_ELEMENT) and HV_ELEMENT/SIMD
              -- once at dispatch so cosine_accumulator_unit doesn't synthesise the priority
              -- encoder + variable barrel-shifter on every accumulator cycle.
              sim_hv_log2(h)      <= std_logic_vector(to_unsigned(
                                       log2_power_of_2(to_integer(unsigned(HVSIZE(harc_EXEC))/4)),
                                       sim_hv_log2(h)'length));
              sim_valid_chunks(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC))/4 / SIMD,
                                                            sim_valid_chunks(h)'length));
             elsif decoded_instruction_DSP(HVBUNDLE_bit_position) = '1' then
                  HVSIZE_READ(h) <= std_logic_vector(shift_left(resize(unsigned(HVSIZE(harc_EXEC)), HVSIZE_READ(h)'length), 1));
                  HVSIZE_READ_init(h) <= std_logic_vector(shift_left(resize(unsigned(HVSIZE(harc_EXEC)), HVSIZE_READ(h)'length), 1));
  

              elsif decoded_instruction_DSP(HVBIND_bit_position )  = '1' then
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC) ) + SIMD_RD_BYTES_wire(h) , HVSIZE_READ(h)'length));

              -- [Op-N2 Phase B] HVPERM read window = HV size + one SIMD chunk worth.
              -- HVSIZE_READ_init is also captured so dsp_exec can derive the current
              -- output chunk index as (init - read)/SIMD_RD_BYTES.
              elsif decoded_instruction_DSP(HVPERM_bit_position) = '1' then
                HVSIZE_READ(h)      <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) + SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
                HVSIZE_READ_init(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) + SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));

              else
                if unsigned(HVSIZE(harc_EXEC)) >= SIMD_RD_BYTES_wire(h) then
                  HVSIZE_READ(h) <= std_logic_vector(unsigned(HVSIZE(harc_EXEC)) - SIMD_RD_BYTES_wire(h));       -- decrement by SIMD_BYTE Execution Capability
                else
                  HVSIZE_READ(h) <= (others => '0');                                                             -- decrement the remaining bytes
                end if;
              end if;

            -- If there is no data grant, we keep the addresses the same
            else

              RS1_Data_IE_lat(h) <= RS1_Data_IE;
              RS2_Data_IE_lat(h) <= RS2_Data_IE;

              -- Preserve the operation context even when the first read grant is delayed
              -- by preceding kmemld/kmemstr traffic. Otherwise HVSIZE_READ stays unknown
              -- and the HDC request collapses before the scratchpad becomes available.
              if decoded_instruction_DSP(HVCLIP_bit_position) = '1' then
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * (32+2+PRECISION_BIT_WIDTH) + 8*SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
                HVSIZE_READ_init(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * (32+2+PRECISION_BIT_WIDTH) + 8*SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
                HVSIZE_READ_init_clip(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * (32+2+PRECISION_BIT_WIDTH) + 8*SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
              elsif decoded_instruction_DSP(HVENC_bit_position) = '1' then
                RS1_Data_ptr(h) <= RS1_Data_IE;
                RS2_Data_ptr(h) <= RS2_Data_IE;
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * unsigned(MPSCLFAC(h)), HVSIZE_READ(h)'length));
                HVSIZE_READ_init(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * unsigned(MPSCLFAC(h)), HVSIZE_READ(h)'length));
              elsif decoded_instruction_DSP(HVSIM_bit_position) = '1' then
                if SIMD > 1 then
                  HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) + log2_power_of_2(SIMD)*SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
                else
                  HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) + 4, HVSIZE_READ(h)'length));
                end if;
                HV_ELEMENT(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC))/4, HV_ELEMENT(h)'length));
                -- [SIMILARITY timing fix] mirror the granted-branch pre-computation.
                sim_hv_log2(h)      <= std_logic_vector(to_unsigned(
                                         log2_power_of_2(to_integer(unsigned(HVSIZE(harc_EXEC))/4)),
                                         sim_hv_log2(h)'length));
                sim_valid_chunks(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC))/4 / SIMD,
                                                              sim_valid_chunks(h)'length));
              elsif decoded_instruction_DSP(HVBUNDLE_bit_position) = '1' then
                HVSIZE_READ(h) <= std_logic_vector(shift_left(resize(unsigned(HVSIZE(harc_EXEC)), HVSIZE_READ(h)'length), 1));
                HVSIZE_READ_init(h) <= std_logic_vector(shift_left(resize(unsigned(HVSIZE(harc_EXEC)), HVSIZE_READ(h)'length), 1));
              elsif decoded_instruction_DSP(HVBIND_bit_position) = '1' then
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) + SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
              -- [Op-N2 Phase B] HVPERM no-grant arming: same shape as the granted
              -- branch above so the FU stays armed until the SPM grants the read.
              -- [20260928 permfix] The dsp_init read (chunk_shift) was NOT granted, so
              -- dsp_exec must re-issue it: one extra chunk read (HVSIZE + 2*S) keeps the
              -- same spare read of the granted branch, otherwise the FSM leaves dsp_exec
              -- one writeback early (last output chunk never written).
              elsif decoded_instruction_DSP(HVPERM_bit_position) = '1' then
                HVSIZE_READ(h)      <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) + 2*SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
                HVSIZE_READ_init(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) + 2*SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
              end if;

            end if;

           ---------------------------------------------------------------------------

          when dsp_exec =>
            recover_state(h) <= recover_state_wires(h);
            if halt_hdc(h) = '1' and halt_hdc_lat(h) = '0' then
              hdc_sc_data_write_int(h) <= hdc_sc_data_write_wire_int(h);
            end if;

            --------------------------------------------------------------------------
            --  ██╗  ██╗██╗    ██╗      ██╗      ██████╗  ██████╗ ██████╗ ███████╗  --
            --  ██║  ██║██║    ██║      ██║     ██╔═══██╗██╔═══██╗██╔══██╗██╔════╝  --
            --  ███████║██║ █╗ ██║█████╗██║     ██║   ██║██║   ██║██████╔╝███████╗  --
            --  ██╔══██║██║███╗██║╚════╝██║     ██║   ██║██║   ██║██╔═══╝ ╚════██║  --
            --  ██║  ██║╚███╔███╔╝      ███████╗╚██████╔╝╚██████╔╝██║     ███████║  --
            --  ╚═╝  ╚═╝ ╚══╝╚══╝       ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝     ╚══════╝  --            
            --------------------------------------------------------------------------

            if halt_hdc(h) = '0' then

              ------------------------------- Increment the write address when we have a result as a vector -------------------------------

              if vec_write_rd_HDC(h) = '1' and wb_ready(h) = '1' then
                RD_Data_IE_lat(h)  <= std_logic_vector(unsigned(RD_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h)); -- destination address increment
              end if;
              
              ------------------------------- Decrement the number of bytes to write -------------------------------
              if wb_ready(h) = '1' then
                
                if to_integer(unsigned(HVSIZE_WRITE(h))) >= SIMD_RD_BYTES_wire(h) then
                  HVSIZE_WRITE(h) <= std_logic_vector(unsigned(HVSIZE_WRITE(h)) - SIMD_RD_BYTES_wire(h));    -- decrement by SIMD_BYTE Execution Capability 
                else
                  HVSIZE_WRITE(h) <= (others => '0');                                                        -- decrement the remaining bytes
                end if;
                
              end if;
              
              ------------------------------- Increment the read addresses -------------------------------

              if to_integer(unsigned(HVSIZE_READ(h))) >= SIMD_RD_BYTES_wire(h) and hdc_data_gnt_i(h) = '1' then -- Increment the addresses untill all the vector elements are operated fetched
                
                ----------------------------------------- Source Register 1 -----------------------------------------
                if vec_read_rs1_HDC(h) = '1' then
                            
                    --FHRR ENCODING
                  if decoded_instruction_DSP(HVENC_bit_position ) = '1' then
                    if HVSIZE_READ(h) /= (Addr_Width downto 0 => 'U') and
                       HVSIZE_READ_init(h) /= (Addr_Width downto 0 => 'U') then
                      enc_next_row := ((to_integer(unsigned(HVSIZE_READ_init(h))) -
                                        to_integer(unsigned(HVSIZE_READ(h)))) /
                                       SIMD_RD_BYTES_wire(h)) + 1;
                      RS1_Data_IE_lat(h) <= std_logic_vector(
                        unsigned(RS1_Data_ptr(h)) + to_unsigned(enc_next_row * 4, RS1_Data_IE_lat(h)'length));
                    end if;

                   elsif decoded_instruction_DSP(HVCLIP_bit_position ) = '1' then
                        if (unsigned(HVSIZE_READ(h)) = (unsigned(HVSIZE_READ_init(h)) - SIMD_RD_BYTES_wire(h)*(32+2+PRECISION_BIT_WIDTH))) and (HVSIZE_READ(h) /= (Addr_Width downto 0 => 'U') ) then 
                        RS1_Data_IE_lat(h) <= std_logic_vector(unsigned(RS1_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h));
                        HVSIZE_READ_init(h) <= HVSIZE_READ(h); --init update so it can make calculation for next one
                        end if;

                  else -- Se non sto eseguendo un KDOTP incremento RS1 normalmente

                    RS1_Data_IE_lat(h) <= std_logic_vector(unsigned(RS1_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h)); -- source 1 address increment

                  end if; 
                  
                end if; 
                
                ----------------------------------------- Source Register 2 -----------------------------------------
                if vec_read_rs2_HDC(h) = '1' then
                  
                    --FHRR ENCODING 
                  if decoded_instruction_DSP(HVENC_bit_position ) = '1' then
                    if HVSIZE_READ(h) /= (Addr_Width downto 0 => 'U') and
                       HVSIZE_READ_init(h) /= (Addr_Width downto 0 => 'U') then
                      enc_next_row := ((to_integer(unsigned(HVSIZE_READ_init(h))) -
                                        to_integer(unsigned(HVSIZE_READ(h)))) /
                                       SIMD_RD_BYTES_wire(h)) + 1;
                      RS2_Data_IE_lat(h) <= std_logic_vector(
                        unsigned(RS2_Data_ptr(h)) +
                        to_unsigned(enc_next_row * to_integer(unsigned(HVSIZE(h))), RS2_Data_IE_lat(h)'length));
                    end if;
                    

                    elsif decoded_instruction_DSP(HVBUNDLE_bit_position) = '1' then
                    
                      if (((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_READ(h)))) / SIMD_RD_BYTES_wire(h)) mod 2 = 0) and (HVSIZE_READ(h) /= (Addr_Width downto 0 => 'U') )
                         and ((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_READ(h)))) / SIMD_RD_BYTES_wire(h)) >= 0
                         then
                          RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h));

                        
                      end if;

                     elsif decoded_instruction_DSP(HVCLIP_bit_position ) = '1' then
                        if (unsigned(HVSIZE_READ(h)) = (unsigned(HVSIZE_READ_init(h)) - SIMD_RD_BYTES_wire(h)*(32+2+PRECISION_BIT_WIDTH))) and (HVSIZE_READ(h) /= (Addr_Width downto 0 => 'U') ) then
                        RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h));
                        end if;

                    -- [Op-N2 Phase B] HVPERM keeps RS2_Data_IE_lat pinned at the
                    -- SPM base. dsp_exec computes the per-cycle SPM addresses
                    -- modulo N from the chunk index, so a linear increment here
                    -- would corrupt the wrap-around.
                    elsif decoded_instruction_DSP(HVPERM_bit_position) = '1' then
                      RS2_Data_IE_lat(h) <= RS2_Data_IE_lat(h);

                  else

                    RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h)); -- source 2 address increment

                  end if;
                end if;

              end if;

              -------------------------- Decrement the vector elements that have already been operated on ---------------------------------------
              if hdc_data_gnt_i(h) = '1' then
                
                if to_integer(unsigned(HVSIZE_READ(h))) >= SIMD_RD_BYTES_wire(h) then
                  
                  HVSIZE_READ(h) <= std_logic_vector(unsigned(HVSIZE_READ(h)) - SIMD_RD_BYTES_wire(h)); -- decrement by SIMD_BYTE Execution Capability

                end if;

              end if;

              -- Advance the div-trigger window when a division completes.
              -- This is unconditional (no grant required) so it is never missed.
              -- Guard with HVSIZE_WRITE > SIMD_RD_BYTES to avoid scheduling a
              -- spurious trigger after the very last element, which would cause
              -- an extra division, a spurious wb_ready and HVSIZE_WRITE underflow.
              if decoded_instruction_DSP_lat(h)(HVCLIP_bit_position) = '1' then
                if div_done(h)(0)(1) = '1' and
                   to_integer(unsigned(HVSIZE_WRITE(h))) > SIMD_RD_BYTES_wire(h) then
                  HVSIZE_READ_init_clip(h) <= std_logic_vector(unsigned(HVSIZE_READ_lat(h)) - SIMD_RD_BYTES_wire(h));
                end if;
              end if;

            end if;

          when others =>
            null;
        end case;
      end if;
    end if;
  end process;


--------------------------------------------------------------------------------------------------------------------------
-- ██████╗ ███████╗██████╗ ███████╗               ██████╗ ██████╗ ██╗   ██╗███╗   ██╗████████╗███████╗██████╗ ███████╗  --
-- ██╔══██╗██╔════╝██╔══██╗██╔════╝              ██╔════╝██╔═══██╗██║   ██║████╗  ██║╚══██╔══╝██╔════╝██╔══██╗██╔════╝  --
-- ██████╔╝█████╗  ██████╔╝█████╗      █████╗    ██║     ██║   ██║██║   ██║██╔██╗ ██║   ██║   █████╗  ██████╔╝███████╗  --
-- ██╔═══╝ ██╔══╝  ██╔══██╗██╔══╝      ╚════╝    ██║     ██║   ██║██║   ██║██║╚██╗██║   ██║   ██╔══╝  ██╔══██╗╚════██║  --
-- ██║     ███████╗██║  ██║██║                   ╚██████╗╚██████╔╝╚██████╔╝██║ ╚████║   ██║   ███████╗██║  ██║███████║  --
-- ╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝                    ╚═════╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚══════╝  --
--------------------------------------------------------------------------------------------------------------------------                                                                                                                   

  gen_perf_counter: if HDCU_PERF_EN = 1 generate
    HDC_Perf_Counters : process(clk_i, rst_ni)
    begin
      if rst_ni = '0' then

        hdcu_performance_counter_int(h)  <= std_logic_vector(to_unsigned(0, hdcu_performance_counter_int(h)'length));
        hdcu_bundle_perf_counter_int(h)  <= std_logic_vector(to_unsigned(0, hdcu_bundle_perf_counter_int(h)'length));
        hdcu_bind_perf_counter_int(h)    <= std_logic_vector(to_unsigned(0, hdcu_bind_perf_counter_int(h)'length));
        hdcu_sim_perf_counter_int(h)     <= std_logic_vector(to_unsigned(0, hdcu_sim_perf_counter_int(h)'length));
        hdcu_clip_perf_counter_int(h)    <= std_logic_vector(to_unsigned(0, hdcu_clip_perf_counter_int(h)'length));
        -- [Op-N2 Phase A] permutation perf counter reset
        hdcu_perm_perf_counter_int(h)    <= std_logic_vector(to_unsigned(0, hdcu_perm_perf_counter_int(h)'length));

        hdcu_enc_perf_counter_int(h)  <= std_logic_vector(to_unsigned(0, hdcu_enc_perf_counter_int(h)'length));

      elsif rising_edge(clk_i) then

        -- Reset the performance counters before a new operation starts
        if bundle_en_wire(h) = '1' then
          hdcu_bundle_perf_counter_int(h)  <= std_logic_vector(to_unsigned(0, hdcu_bundle_perf_counter_int(h)'length));
        elsif bind_en_wire(h) = '1' then
          hdcu_bind_perf_counter_int(h)    <= std_logic_vector(to_unsigned(0, hdcu_bind_perf_counter_int(h)'length));
        elsif sim_en_wire(h) = '1' then
          hdcu_sim_perf_counter_int(h)     <= std_logic_vector(to_unsigned(0, hdcu_sim_perf_counter_int(h)'length));
        elsif clip_en_wire(h) = '1' then
          hdcu_clip_perf_counter_int(h)    <= std_logic_vector(to_unsigned(0, hdcu_clip_perf_counter_int(h)'length));

        elsif enc_en_wire(h) = '1' then
          hdcu_enc_perf_counter_int(h)  <= std_logic_vector(to_unsigned(0, hdcu_enc_perf_counter_int(h)'length));
        -- [Op-N2 Phase A] reset perm counter when a new HVPERM is dispatched
        elsif perm_en_wire(h) = '1' then
          hdcu_perm_perf_counter_int(h)    <= std_logic_vector(to_unsigned(0, hdcu_perm_perf_counter_int(h)'length));
        end if;

        -- Counter for the HDCU
        if busy_hdc(h) = '1' then
          hdcu_performance_counter_int(h) <= std_logic_vector(unsigned(hdcu_performance_counter_int(h)) + 1);
        end if;

        -- Counter for each of the FU
        if bundle_en(h) = '1' then
          hdcu_bundle_perf_counter_int(h) <= std_logic_vector(unsigned(hdcu_bundle_perf_counter_int(h)) + 1);
        elsif bind_en(h) = '1' then
          hdcu_bind_perf_counter_int(h) <= std_logic_vector(unsigned(hdcu_bind_perf_counter_int(h)) + 1);
        elsif sim_en(h) = '1'  then
          hdcu_sim_perf_counter_int(h) <= std_logic_vector(unsigned(hdcu_sim_perf_counter_int(h)) + 1);
        elsif clip_en(h) = '1' then
          hdcu_clip_perf_counter_int(h) <= std_logic_vector(unsigned(hdcu_clip_perf_counter_int(h)) + 1);
        elsif enc_en(h) = '1' then
          hdcu_enc_perf_counter_int(h) <= std_logic_vector(unsigned(hdcu_enc_perf_counter_int(h)) + 1);
        -- [Op-N2 Phase A] increment perm counter while HVPERM is in flight
        elsif perm_en(h) = '1' then
          hdcu_perm_perf_counter_int(h) <= std_logic_vector(unsigned(hdcu_perm_perf_counter_int(h)) + 1);
        end if;

      end if;
    end process;

    HDCU_Perf_Counters_Comb : process(all)
    begin
      if(rst_ni = '0') then
        hdcu_performance_counter(h)  <= (others => '0');
        hdcu_bundle_perf_counter(h)  <= (others => '0');
        hdcu_bind_perf_counter(h)    <= (others => '0');
        hdcu_sim_perf_counter(h)     <= (others => '0');
        hdcu_clip_perf_counter(h)    <= (others => '0');
        hdcu_perm_perf_counter(h)    <= (others => '0');
        hdcu_enc_perf_counter(h)  <= (others => '0');
      else
        hdcu_performance_counter(h)  <= hdcu_performance_counter_int(h);
        hdcu_bundle_perf_counter(h)  <= hdcu_bundle_perf_counter_int(h);
        hdcu_bind_perf_counter(h)    <= hdcu_bind_perf_counter_int(h);
        hdcu_sim_perf_counter(h)     <= hdcu_sim_perf_counter_int(h);
        hdcu_clip_perf_counter(h)    <= hdcu_clip_perf_counter_int(h);
        -- [Op-N2 Phase A] expose the permutation FU counter
        hdcu_perm_perf_counter(h)    <= hdcu_perm_perf_counter_int(h);
        hdcu_enc_perf_counter(h)  <= hdcu_enc_perf_counter_int(h);
      end if;

    end process;
  end generate gen_perf_counter;


  ------------ Combinational Stage of HDC Unit ----------------------------------------------------------------------
  HDC_Excpt_Cntrl_Unit_comb : process(all)
  
  variable busy_HDC_internal_wires : std_logic;
  variable hdc_except_condition_wires : std_logic_vector(harc_range);
  variable hdc_taken_branch_wires : std_logic_vector(harc_range);  
      
  begin
    busy_HDC_internal_wires        := '0';
    hdc_except_condition_wires(h)  := '0';
    hdc_taken_branch_wires(h)      := '0';
    wb_ready(h)                    <= '0';
    halt_hdc(h)                    <= '0';
    nextstate_HDC(h)               <= dsp_init;
    recover_state_wires(h)         <= recover_state(h);
    hdc_except_data_wire(h)        <= hdc_except_data(h);
    overflow_rs1_sc(h)             <= (others => '0');
    overflow_rs2_sc(h)             <= (others => '0');
    overflow_rd_sc(h)              <= (others => '0');
    hdc_we_word(h)                 <= (others => '0');
    hdc_sci_req(h)                 <= (others => '0');
    hdc_sci_we(h)                  <= (others => '0');
    hdc_sc_write_addr(h)           <= (others => '0');
    hdc_sc_read_addr(h)            <= (others => (others => '0'));
    hdc_to_sc(h)                   <= (others => (others => '0'));

    if hdc_instr_req(h) = '1' or busy_HDC_internal_lat(h) = '1' then
      case state_HDC(h) is

        when dsp_init =>

          ---------------------------------------------------------------------------------------------------------------------
          --  ███████╗██╗  ██╗ ██████╗██████╗ ████████╗    ██╗  ██╗ █████╗ ███╗   ██╗██████╗ ██╗     ██╗███╗   ██╗ ██████╗   --
          --  ██╔════╝╚██╗██╔╝██╔════╝██╔══██╗╚══██╔══╝    ██║  ██║██╔══██╗████╗  ██║██╔══██╗██║     ██║████╗  ██║██╔════╝   --
          --  █████╗   ╚███╔╝ ██║     ██████╔╝   ██║       ███████║███████║██╔██╗ ██║██║  ██║██║     ██║██╔██╗ ██║██║  ███╗  --
          --  ██╔══╝   ██╔██╗ ██║     ██╔═══╝    ██║       ██╔══██║██╔══██║██║╚██╗██║██║  ██║██║     ██║██║╚██╗██║██║   ██║  -- 
          --  ███████╗██╔╝ ██╗╚██████╗██║        ██║       ██║  ██║██║  ██║██║ ╚████║██████╔╝███████╗██║██║ ╚████║╚██████╔╝  --
          --  ╚══════╝╚═╝  ╚═╝ ╚═════╝╚═╝        ╚═╝       ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝   --
          ---------------------------------------------------------------------------------------------------------------------

          overflow_rs1_sc(h) <= std_logic_vector('0' & unsigned(RS1_Data_IE(Addr_Width -1 downto 0)) + unsigned(HVSIZE(harc_EXEC)) -1);
          overflow_rs2_sc(h) <= std_logic_vector('0' & unsigned(RS2_Data_IE(Addr_Width -1 downto 0)) + unsigned(HVSIZE(harc_EXEC)) -1);
          overflow_rd_sc(h)  <= std_logic_vector('0' & unsigned(RD_Data_IE(Addr_Width  -1 downto 0)) + unsigned(HVSIZE(harc_EXEC)) -1);
          if HVSIZE(harc_EXEC) = (0 to Addr_Width => '0') then
            null;
          elsif HVSIZE(harc_EXEC)(1 downto 0) /= "00" and MVTYPE(harc_EXEC)(3 downto 2) = "10" then  -- Set exception if the number of bytes are not divisible by four
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_wires(h)     := '1';    
            hdc_except_data_wire(h) <= ILLEGAL_VECTOR_SIZE_EXCEPT_CODE;
          elsif HVSIZE(harc_EXEC)(0) /= '0' and MVTYPE(harc_EXEC)(3 downto 2) = "01" then            -- Set exception if the number of bytes are not divisible by two
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_wires(h)     := '1';
            hdc_except_data_wire(h) <= ILLEGAL_VECTOR_SIZE_EXCEPT_CODE;
          elsif (rs1_to_sc  = "100" and vec_read_rs1_ID = '1') or
            (rs2_to_sc  = "100" and vec_read_rs2_ID = '1') or
             rd_to_sc   = "100" then     -- Set exception for non scratchpad access
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_wires(h)     := '1';    
            hdc_except_data_wire(h) <= ILLEGAL_ADDRESS_EXCEPT_CODE;
          elsif rs1_to_sc = rs2_to_sc and vec_read_rs1_ID = '1' and vec_read_rs2_ID = '1' then               -- Set exception for same read access
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_wires(h)     := '1';    
            hdc_except_data_wire(h) <= READ_SAME_SCARTCHPAD_EXCEPT_CODE;    
          elsif (overflow_rs1_sc(h)(Addr_Width) = '1' and vec_read_rs1_ID = '1') or (overflow_rs2_sc(h)(Addr_Width) = '1' and  vec_read_rs2_ID = '1') then -- Set exception if reading overflows the scratchpad's address
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_wires(h)     := '1';    
            hdc_except_data_wire(h) <= SCRATCHPAD_OVERFLOW_EXCEPT_CODE;
          elsif overflow_rd_sc(h)(Addr_Width) = '1'  and vec_write_rd_ID = '1' then           -- Set exception if reading overflows the scratchpad's address, scalar writes are excluded
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_wires(h)     := '1';    
            hdc_except_data_wire(h) <= SCRATCHPAD_OVERFLOW_EXCEPT_CODE;
          else
            if halt_hart(h) = '0' then
              nextstate_HDC(h) <= dsp_exec;
            else
              nextstate_HDC(h) <= dsp_halt_hart;
            end if;
            busy_HDC_internal_wires := '1';
          end if;

          if rs1_to_sc /= "100" and spm_rs1 = '1' and halt_hart(h) = '0' then
            hdc_sci_req(h)(to_integer(unsigned(rs1_to_sc))) <= '1';
            hdc_to_sc(h)(to_integer(unsigned(rs1_to_sc)))(0) <= '1';
            hdc_sc_read_addr(h)(0) <= RS1_Data_IE(Addr_Width-1 downto 0);
          end if;
          if rs2_to_sc /= "100" and spm_rs2 = '1' and rs1_to_Sc /= rs2_to_sc and halt_hart(h) = '0' then   -- Do not send a read request if the second operand accesses the same spm as the first,
            hdc_sci_req(h)(to_integer(unsigned(rs2_to_sc))) <= '1';
            hdc_to_sc(h)(to_integer(unsigned(rs2_to_sc)))(1) <= '1';
            hdc_sc_read_addr(h)(1) <= RS2_Data_IE(Addr_Width-1 downto 0);
          end if;

          -- [Op-N2 Phase B] HVPERM dsp_init address override (single channel).
          -- The SPM bank only has one physical read port per cycle, so Phase B
          -- streams chunks sequentially on channel 1 (same as Phase A) and the
          -- splice mux combines a latched previous-chunk register with the
          -- current SPM word. The first dsp_init read fetches chunk number
          -- (chunk_shift mod N) which becomes chunk_b for output 0; the next
          -- read in dsp_exec fetches chunk_a for output 0 and chunk_b for
          -- output 1, etc. Addresses are derived combinationally from
          -- RS1_Data_IE[7:0] and HVSIZE because the latched perm_chunk_*_int
          -- signals are not yet valid in this very same cycle.
          if decoded_instruction_DSP(HVPERM_bit_position) = '1' and
             rs2_to_sc /= "100" and spm_rs2 = '1' and halt_hart(h) = '0' then
            if to_integer(unsigned(HVSIZE(harc_EXEC))) > 0 and SIMD_RD_BYTES_wire(h) > 0 then
              hdc_sc_read_addr(h)(1) <= std_logic_vector(resize(
                unsigned(RS2_Data_IE(Addr_Width-1 downto 0)) +
                to_unsigned(
                  ((to_integer(unsigned(RS1_Data_IE(15 downto 0))) / SIMD)
                    mod (to_integer(unsigned(HVSIZE(harc_EXEC))) / SIMD_RD_BYTES_wire(h)))
                  * SIMD_RD_BYTES_wire(h),
                  Addr_Width),
                Addr_Width));
            end if;
          end if;
        
         when dsp_halt_hart =>

           if halt_hart(h) = '0' then
             nextstate_HDC(h) <= dsp_exec;
           else
             nextstate_HDC(h) <= dsp_halt_hart;
           end if;
           busy_HDC_internal_wires := '1';

         when dsp_exec =>

           -----------------------------------------------------------------------------------------------------------------------
           --   ██████╗███╗   ██╗████████╗██████╗ ██╗         ██╗  ██╗ █████╗ ███╗   ██╗██████╗ ██╗     ██╗███╗   ██╗ ██████╗   --
           --  ██╔════╝████╗  ██║╚══██╔══╝██╔══██╗██║         ██║  ██║██╔══██╗████╗  ██║██╔══██╗██║     ██║████╗  ██║██╔════╝   --
           --  ██║     ██╔██╗ ██║   ██║   ██████╔╝██║         ███████║███████║██╔██╗ ██║██║  ██║██║     ██║██╔██╗ ██║██║  ███╗  --
           --  ██║     ██║╚██╗██║   ██║   ██╔══██╗██║         ██╔══██║██╔══██║██║╚██╗██║██║  ██║██║     ██║██║╚██╗██║██║   ██║  --
           --  ╚██████╗██║ ╚████║   ██║   ██║  ██║███████╗    ██║  ██║██║  ██║██║ ╚████║██████╔╝███████╗██║██║ ╚████║╚██████╔╝  --
           --   ╚═════╝╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝╚══════╝    ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝   --
           -----------------------------------------------------------------------------------------------------------------------

           ------ SMP BANK ENABLER --------------------------------------------------------------------------------------------------
           -- the following enables the appropriate banks to write the SIMD output, depending whether the result is a vector or a  --
           -- scalar, and adjusts the enabler appropriately based on the SIMD size. If the bytes to write are greater than SIMD*4  --
           -- then all banks are enabaled, else we perform the selective bank enabling as shown below under the 'elsif' clause     --
           --------------------------------------------------------------------------------------------------------------------------

           if (hdc_sci_wr_gnt(h) = '0' and wb_ready(h) = '1') then
             halt_hdc(h) <= '1';
             recover_state_wires(h) <= '1';
           elsif unsigned(HVSIZE_WRITE(h)) <= SIMD_RD_BYTES(h) then
             recover_state_wires(h) <= '0';
           end if;

           if vec_write_rd_HDC(h) = '1' and  hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h)))) = '1' then
             if (unsigned(HVSIZE_WRITE(h)) >= (SIMD)*4+1)then  -- 
               hdc_we_word(h) <= (others => '1');
             elsif  unsigned(HVSIZE_WRITE(h)) >= 1 then
               for i in 0 to SIMD-1 loop
                 if i <= to_integer(unsigned(HVSIZE_WRITE(h))-1)/4 then -- Four because of the number of bytes per word
                   if to_integer(unsigned(hdc_sc_write_addr(h)(SIMD_BITS+1 downto 0))/4 + i) < SIMD then
                     hdc_we_word(h)(to_integer(unsigned(hdc_sc_write_addr(h)(SIMD_BITS+1 downto 0))/4 + i)) <= '1';
                   elsif to_integer(unsigned(hdc_sc_write_addr(h)(SIMD_BITS+1 downto 0))/4 + i) >= SIMD then
                     hdc_we_word(h)(to_integer(unsigned(hdc_sc_write_addr(h)(SIMD_BITS+1 downto 0))/4 + i - SIMD)) <= '1';
                   end if;
                 end if;
               end loop;
             end if;
           elsif vec_write_rd_HDC(h) = '0' and  hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h)))) = '1' then
             hdc_we_word(h)(to_integer(unsigned(hdc_sc_write_addr(h)(SIMD_BITS+1 downto 0))/4)) <= '1';
           end if;
           -------------------------------------------------------------------------------------------------------------------------


           --------------------------- FHRR BUNDLING ---------------------------------
           if decoded_instruction_DSP_lat(h)(HVBUNDLE_bit_position)  = '1' then
            if bundle_stage_2_en(h) = '1' then 
              wb_ready(h) <= '1';
            elsif recover_state(h) = '1' then
              wb_ready(h) <= '1';  
            end if;
            if HVSIZE_READ(h) > (0 to Addr_Width => '0') and enc_stage_1_en(h) = '0' then
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))(0) <= '1';
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))(1) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))  <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))  <= '1';
              hdc_sc_read_addr(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              hdc_sc_read_addr(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0);
            end if;
            if HVSIZE_WRITE(h) > (0 to Addr_Width => '0') then
              nextstate_HDC(h) <= dsp_exec;
              busy_HDC_internal_wires := '1';
            end if;
            if bundle_stage_2_en(h) = '1' or recover_state(h) = '1' then
              hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h))))    <= '1';
              hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
            end if;
          end if;
           ----------------------------------------------------------------------

           ----------------------------- BINDING --------------------------------
           if decoded_instruction_DSP_lat(h)(HVBIND_bit_position)    = '1' then 
             if bind_en(h) = '1' and bind_stage_1_en(h) = '1' then
               wb_ready(h) <= '1';
             elsif recover_state(h) = '1' then
               wb_ready(h) <= '1';
             end if;
             if HVSIZE_READ(h) > (0 to Addr_Width => '0') then
               hdc_to_sc(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))(0) <= '1';
               hdc_to_sc(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))(1) <= '1';
               hdc_sci_req(h)(to_integer(unsigned(hdc_rs1_to_sc(h)))) <= '1';
               hdc_sci_req(h)(to_integer(unsigned(hdc_rs2_to_sc(h)))) <= '1';
               hdc_sc_read_addr(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0); 
               hdc_sc_read_addr(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              --  nextstate_HDC(h) <= dsp_exec;
              --  busy_HDC_internal_wires := '1';
               
              if HVSIZE_WRITE(h) > (0 to Addr_Width => '0') then
              nextstate_HDC(h) <= dsp_exec;
              busy_HDC_internal_wires := '1';
              end if;
             end if;
             if wb_ready(h) = '1' then
               hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h)))) <= '1';
               hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
             end if;
           end if;
           ----------------------------------------------------------------------------

           

           ----------------------------- ENCODING --------------------------------
           if decoded_instruction_DSP_lat(h)(HVENC_bit_position )  = '1' then
            if enc_stage_4_en(h) = '1'  then 
              wb_ready(h) <= '1';

            elsif recover_state(h) = '1' then
              wb_ready(h) <= '1';  
            end if;
            if HVSIZE_READ(h) > (0 to Addr_Width => '0') then
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))(0) <= '1';
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))(1) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))  <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))  <= '1';
              hdc_sc_read_addr(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              hdc_sc_read_addr(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0);
            end if;
            if HVSIZE_WRITE(h) > (0 to Addr_Width => '0') then
              nextstate_HDC(h) <= dsp_exec;
              busy_HDC_internal_wires := '1';
            end if;
            if wb_ready(h) = '1' then
              hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h))))    <= '1';
              hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
            end if;
          end if;
           ----------------------------------------------------------------------------
          
           ----------------------------- SIMILARITY -----------------------------------
           if decoded_instruction_DSP_lat(h)(HVSIM_bit_position)    = '1' then
             -- [Op-L1] Mid-tree register split: when SIM_PIPE_EXTRA=1 the result
             -- needs one extra cycle to drain through sim_cos_sum_reg into the
             -- cosine accumulator before the scratchpad write-back fires, so
             -- wb_ready is gated on sim_stage_3_en instead of sim_stage_2_en.
             -- For SIM_PIPE_EXTRA=0 (SIMD<=2) the trigger is unchanged.
             if SIM_PIPE_EXTRA = 1 then
               if sim_stage_3_en(h) = '1' then
                 wb_ready(h) <= '1';
               elsif recover_state(h) = '1' then
                 wb_ready(h) <= '1';
               end if;
             else
               if sim_stage_2_en(h) = '1' then
                 wb_ready(h) <= '1';
               elsif recover_state(h) = '1' then
                 wb_ready(h) <= '1';
               end if;
             end if;
             if HVSIZE_READ(h) > (0 to Addr_Width => '0') then
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))(0) <= '1';
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))(1) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs1_to_sc(h)))) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs2_to_sc(h)))) <= '1';
              hdc_sc_read_addr(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              hdc_sc_read_addr(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0);
             end if;
             if HVSIZE_WRITE(h) > (0 to Addr_Width => '0') then
              nextstate_HDC(h) <= dsp_exec;
              busy_HDC_internal_wires := '1';
             end if;
             if wb_ready(h) = '1' then
               hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h)))) <= '1';
               hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
             end if;
           end if;
           -------------------------------------------------------------------------

           ---------------------------- CLIPPING -----------------------------------
           if decoded_instruction_DSP_lat(h)(HVCLIP_bit_position)  = '1' then
            if clip_stage_3_en(h) = '1' then
              wb_ready(h) <= '1';
            elsif recover_state(h) = '1' then
              wb_ready(h) <= '1';
            end if;
            if HVSIZE_READ(h) > (0 to Addr_Width => '0') then
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))(0) <= '1';
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))(1) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs1_to_sc(h)))) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs2_to_sc(h)))) <= '1';
              hdc_sc_read_addr(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              hdc_sc_read_addr(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0);
            end if;
            if HVSIZE_WRITE(h) > (0 to Addr_Width => '0') then
              nextstate_HDC(h) <= dsp_exec;
              busy_HDC_internal_wires := '1';
            end if;
            if wb_ready(h) = '1' then
              hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h))))    <= '1';
              hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
            end if;
          end if;
          ------------------------------------------------------------------------

          ---------------------------- PERMUTATION (Op-N2 Phase B) ----------------
          -- HVPERM: rs1 = scalar shift amount (no SPM read), rs2 = source HV base,
          -- rd  = destination HV base. Phase B streams chunks sequentially on
          -- SPM channel 1 (single physical read port per bank) and uses the
          -- latched previous-chunk register hdcu_in_perm_operand_b together
          -- with the current MAPPING_IN unpack (hdcu_in_perm_operand) as the
          -- (chunk_b, chunk_a) pair fed to the splice mux.
          --   addr  = base + ((read_k + chunk_shift) mod N) * SIMD_RD_BYTES
          -- where read_k counts the SPM read events (0 at dsp_init, 1..N in
          -- dsp_exec). The k-th read provides chunk_a for output (k-1) and
          -- chunk_b for output k. RS2_Data_IE_lat is pinned to base; modular
          -- arithmetic wraps the cycle that produces the final chunk_a.
          -- [Phase B 20260507] Pipelined two-cycle latency, single channel.
          if decoded_instruction_DSP_lat(h)(HVPERM_bit_position) = '1' then
            -- Phase B: writeback fires when stage_2 is high (chunk_a current,
            -- chunk_b latched). Stage_1 alone is the first SPM grant, which
            -- only loads chunk_b for output 0 -- not enough yet.
            if perm_en(h) = '1' and perm_stage_2_en(h) = '1' then
              wb_ready(h) <= '1';
            elsif recover_state(h) = '1' then
              wb_ready(h) <= '1';
            end if;
            if HVSIZE_READ(h) > (0 to Addr_Width => '0') then
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))(1) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs2_to_sc(h)))) <= '1';
              -- [Op-N2 Phase B timing fix 20260507] Use the registered chunk
              -- index (perm_next_read_chunk_int) instead of recomputing
              -- ((HVSIZE_READ_init - HVSIZE_READ)/S + 1 + chunk_shift) mod N
              -- combinationally each cycle. The remaining arithmetic is a
              -- single multiply by SIMD_RD_BYTES_wire(h); since SIMD ∈
              -- {1,2,4,8,16,32} the multiplicand is a power of 2 and Vivado
              -- maps this to a shifter (no carry chain). The base add then
              -- collapses to a single Addr_Width-bit ripple, replacing the
              -- ~152 CARRY8 chain reported in the round-3 timing analysis.
              if perm_chunk_count_int(h) > 0 and SIMD_RD_BYTES_wire(h) > 0 then
                hdc_sc_read_addr(h)(1) <= std_logic_vector(resize(
                  unsigned(RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0)) +
                  to_unsigned(
                    perm_next_read_chunk_int(h) * SIMD_RD_BYTES_wire(h),
                    Addr_Width),
                  Addr_Width));
              else
                hdc_sc_read_addr(h)(1) <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              end if;
              if HVSIZE_WRITE(h) > (0 to Addr_Width => '0') then
                nextstate_HDC(h) <= dsp_exec;
                busy_HDC_internal_wires := '1';
              end if;
            end if;
            if wb_ready(h) = '1' then
              hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h)))) <= '1';
              hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
            end if;
          end if;
          ------------------------------------------------------------------------


     
           -------------------------------------------------------------------------

        when others =>
           null;
       end case;
     end if;
      
    busy_HDC_internal(h)    <= busy_HDC_internal_wires;
    hdc_except_condition(h) <= hdc_except_condition_wires(h);
    hdc_taken_branch(h)     <= hdc_taken_branch_wires(h);
      
  end process;

  ---------------------------------------------------------------------------------------------------------------------------------------------------------
  --  ██████╗ ██╗██████╗ ███████╗██╗     ██╗███╗   ██╗███████╗     ██████╗ ██████╗ ███╗   ██╗████████╗██████╗  ██████╗ ██╗     ██╗     ███████╗██████╗   --
  --  ██╔══██╗██║██╔══██╗██╔════╝██║     ██║████╗  ██║██╔════╝    ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔══██╗██╔═══██╗██║     ██║     ██╔════╝██╔══██╗  --
  --  ██████╔╝██║██████╔╝█████╗  ██║     ██║██╔██╗ ██║█████╗      ██║     ██║   ██║██╔██╗ ██║   ██║   ██████╔╝██║   ██║██║     ██║     █████╗  ██████╔╝  --
  --  ██╔═══╝ ██║██╔═══╝ ██╔══╝  ██║     ██║██║╚██╗██║██╔══╝      ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══██╗██║   ██║██║     ██║     ██╔══╝  ██╔══██╗  --
  --  ██║     ██║██║     ███████╗███████╗██║██║ ╚████║███████╗    ╚██████╗╚██████╔╝██║ ╚████║   ██║   ██║  ██║╚██████╔╝███████╗███████╗███████╗██║  ██║  --
  --  ╚═╝     ╚═╝╚═╝     ╚══════╝╚══════╝╚═╝╚═╝  ╚═══╝╚══════╝     ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚══════╝╚══════╝╚═╝  ╚═╝  --
  ---------------------------------------------------------------------------------------------------------------------------------------------------------

  -- ============================================================================
  -- [Op-C1 conservative cleanup] FSM stage-enable controller.
  --
  -- The FU stage-enable signals form an implicit table: each FU has 1..N pipeline
  -- stages, and the propagation rule is mostly uniform:
  --     stage_1_en   <= hdc_data_gnt_i_lat AND <fu>_en
  --     stage_(k+1)  <= stage_k                              (default cascade)
  --
  -- with two FU-specific exceptions:
  --   - ENCODE: stage_3 gates on (enc_row_count == MPSCLFAC) to mark the last row.
  --   - SIMILARITY: stage_2 gates on (HVSIZE_READ_lat == 0) to fire wb only after
  --                  the full HV has been streamed.
  --
  -- A future full Op-C1 refactor would express this as a 2-D constant table
  --     fu_stages_table : array(fu_kind_t, stage_idx) of stage_descriptor;
  -- driven by a single generic process. That refactor was deferred (see plan
  -- risk register: "Alta probabilita' + Critico impatto") because every read
  -- site of a stage-enable signal — and there are >40 across this file — would
  -- need to migrate to the table view, with bit-exact regression on a SIMD-8/32
  -- build that we currently can't simulate.
  --
  -- This iteration delivers:
  --   * Centralised documentation of the FSM table (this comment).
  --   * Visually grouped reset / propagation / fu-specific blocks (no semantic
  --     change).
  --   * Op-N2 Phase B perm_stage_2_en cleanly integrated alongside the
  --     existing 5 FU patterns.
  --
  -- ============================================================================
  fsm_HDC_pipeline_controller : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then

      -- === Pipeline scaffolding ===========================================
      hdc_data_gnt_i_lat(h)    <= '0';
      state_HDC(h)             <= dsp_init;

      -- === Per-FU stage-enable reset (table row 0: clear-all) =============
      --     FU       :  stage_1  stage_2  stage_3  stage_4
      --     BUNDLE   :    0        0        -        -
      bundle_stage_1_en(h)     <= '0';
      bundle_stage_2_en(h)     <= '0';
      --     BIND     :    0        0        0        -
      bind_stage_1_en(h)       <= '0';
      bind_stage_2_en(h)       <= '0';
      bind_stage_3_en(h)       <= '0';
      --     ENCODE   :    0        0        0        0
      enc_stage_1_en(h)        <= '0';
      enc_stage_2_en(h)        <= '0';
      enc_stage_3_en(h)        <= '0';
      enc_stage_4_en(h)        <= '0';
      enc_row_count(h)         <= (others => '0');
      --     SIMILAR  :    0        0        0        0   [Op-L1: stage_4 reserved]
      sim_stage_1_en(h)        <= '0';
      sim_stage_2_en(h)        <= '0';
      sim_stage_3_en(h)        <= '0';
      sim_stage_4_en(h)        <= '0';
      --     CLIP     :    0        0        0        -
      clip_stage_1_en(h)       <= '0';
      clip_stage_2_en(h)       <= '0';
      clip_stage_3_en(h)       <= '0';
      --     PERM     :    0        0        -        -   [Op-N2 Phase A+B]
      perm_stage_1_en(h)       <= '0';
      perm_stage_2_en(h)       <= '0';

    elsif rising_edge(clk_i) then

      hdc_data_gnt_i_lat(h)    <= hdc_data_gnt_i(h);

     
      ------------FHRR BUNDLING ----------------
      bundle_stage_1_en(h)     <= hdc_data_gnt_i_lat(h) and bundle_en(h);
      bundle_stage_2_en(h)     <= bundle_stage_1_en(h);
      --------------------------------------
      ------------ BINDING -----------------
      bind_stage_1_en(h)       <= hdc_data_gnt_i_lat(h) and bind_en(h);
      bind_stage_2_en(h)       <= bind_stage_1_en(h);
      bind_stage_3_en(h)       <= bind_stage_2_en(h);
      --------------------------------------

      ------------ ENCODING -----------------
      enc_stage_1_en(h)       <= hdc_data_gnt_i_lat(h) and enc_en(h);
      enc_stage_2_en(h)       <=  enc_stage_1_en(h);
      if enc_en_wire(h) = '1' and enc_en(h) = '0' then
        enc_row_count(h) <= (others => '0');
      elsif enc_en(h) = '1' and enc_stage_1_en(h) = '1' then
        enc_row_count(h) <= std_logic_vector(unsigned(enc_row_count(h)) + 1);
      end if;
      if enc_stage_2_en(h) = '1' and
         to_integer(unsigned(enc_row_count(h))) = to_integer(unsigned(MPSCLFAC(h))) then
        enc_stage_3_en(h) <= enc_stage_2_en(h);
      
      else 
        enc_stage_3_en(h) <= '0';
      end if;
      enc_stage_4_en(h)       <= enc_stage_3_en(h);

      --------------------------------------
      
      ------------- SIMILARITY ----------------
      sim_stage_1_en(h)      <= hdc_data_gnt_i_lat(h) and sim_en(h);
      if (to_integer(unsigned(HVSIZE_READ_lat(h)))) = 0 and SIMD > 1 and HVSIZE_READ_lat(h) /= (Addr_Width downto 0 => 'U') then
      sim_stage_2_en(h)       <= sim_stage_1_en(h);
      elsif to_integer(unsigned(HVSIZE_READ_lat(h)) - 4)= 0  and SIMD=1 and HVSIZE_READ_lat(h) /= (Addr_Width downto 0 => 'U')   then
      sim_stage_2_en(h)       <= sim_stage_1_en(h);
      else
      sim_stage_2_en(h)     <= '0';
      end if;

      -- [Op-L1] sim_stage_3_en is a +1-cycle-delayed copy of sim_stage_2_en.
      -- It is the wb_ready trigger when SIM_PIPE_EXTRA=1 so that the result
      -- has one extra cycle to propagate through the registered tree-adder
      -- output (sim_cos_sum_reg) into cosine_accumulator_avg before the
      -- scratchpad write-back fires. Driven unconditionally so synthesis sees
      -- a stable propagation chain regardless of SIMD.
      sim_stage_3_en(h)      <= sim_stage_2_en(h);
      sim_stage_4_en(h)      <= sim_stage_3_en(h);
      -------------------------------------

      ------------- CLIPPING ------------------
      -- Stage 1: fires on each SPM data grant (loads operands, triggers divider).
      clip_stage_1_en(h) <= hdc_data_gnt_i_lat(h) and clip_en(h);
      -- Stage 2: fires one cycle after the 48-cycle divider completes (tangent
      --          is valid; fsm_atan_sync will latch hdcu_out_clip_results).
      -- Note: when multithreaded_accl_en=0, fu_range=accl_range so h is a valid fu index.
      clip_stage_2_en(h) <= div_done_lat2(h)(0)(1) and clip_en(h);
      -- Stage 3: one cycle after stage 2 -- sets wb_ready so the result is
      --          written back to the scratchpad.
      clip_stage_3_en(h) <= clip_stage_2_en(h);
      ---------------------------------------

      ------------- PERMUTATION (Op-N2 Phase B) -------
      -- Two-cycle pipeline. Stage_1 fires when the FIRST SPM grant lands
      -- (hdcu_in_perm_operand carries chunk_b for output 0) and is also used
      -- to clock the previous-chunk latch hdcu_in_perm_operand_b. Stage_2
      -- fires one cycle later, when the SECOND SPM grant lands and the splice
      -- between (hdcu_in_perm_operand_b = chunk_b, hdcu_in_perm_operand =
      -- chunk_a) is valid; stage_2 drives wb_ready.
      perm_stage_1_en(h) <= hdc_data_gnt_i_lat(h) and perm_en(h);
      perm_stage_2_en(h) <= perm_stage_1_en(h);
      ---------------------------------------

      halt_hdc_lat(h)          <= halt_hdc(h);
      state_HDC(h)             <= nextstate_HDC(h);
      busy_HDC_internal_lat(h) <= busy_HDC_internal(h);
      SIMD_RD_BYTES(h)         <= SIMD_RD_BYTES_wire(h);
      hdc_except_data(h)       <= hdc_except_data_wire(h);

    end if;
  end process;

  -- [Op-N2 Phase B timing fix 20260507] Maintain a registered chunk index
  -- for HVPERM SPM reads, replacing the combinational div+mod+mul chain
  -- that caused WNS=-22.124 ns @ P=1 in the round-3 synth report. The
  -- counter is primed at HVPERM dispatch (perm_en_wire rising edge in
  -- dsp_init) by recomputing chunk_shift+1 directly from the combinational
  -- inputs (RS1_Data_IE[7:0], HVSIZE(harc_EXEC), SIMD), since the
  -- registered perm_chunk_count_int / perm_chunk_shift_int latched in
  -- dsp_init are not yet visible on this same edge. The first dsp_exec
  -- cycle then issues read_k=1 with chunk index (chunk_shift + 1) mod N
  -- (read_0 was already issued in dsp_init, at chunk index chunk_shift).
  -- After the init the counter increments by 1 mod N at every successful
  -- SPM grant, using the now-valid registered perm_chunk_count_int. The
  -- prime arithmetic is purely sequential (off the read-address path);
  -- the remaining combinational arithmetic on hdc_sc_read_addr(h)(1) is a
  -- single multiply by SIMD_RD_BYTES (power-of-2: synthesises to a
  -- shifter, no carry chain) plus one base-address adder.
  perm_chunk_idx_sync : process(clk_i, rst_ni)
    variable v_chunk_count_init : integer;
    variable v_chunk_shift_raw  : integer;
    variable v_chunk_shift_init : integer;
  begin
    if rst_ni = '0' then
      perm_read_chunk_idx_int(h)  <= 0;
      perm_next_read_chunk_int(h) <= 0;
      perm_en_prev(h)             <= '0';
    elsif rising_edge(clk_i) then
      perm_en_prev(h) <= perm_en(h);
      -- HVPERM dispatch: perm_en_wire rises in dsp_init while perm_en is
      -- still 0. We recompute N and chunk_shift+1 from the combinational
      -- decode signals; this duplicates a divide once per dispatch in a
      -- purely sequential context (off the SPM read-address critical path).
      if perm_en_wire(h) = '1' and perm_en(h) = '0' then
        -- [Phase B timing fix 2 — 20260507] Replace variable-divisor mod
        -- operations with compare-subtract / compare-select. The SW glue
        -- (permute_hw / permute_sw) normalizes shift mod D, so the raw
        -- chunk_shift = (shift / SIMD) is provably < chunk_count. A single
        -- subtract is therefore defensive only (no-op in normal use). The
        -- subsequent (+1) mod chunk_count is replaced by a compare-with-(N-1)
        -- and select-to-0 — single LUT level vs ~70 CARRY8 of an integer mod.
        -- This collapses the priming critical path that the previous fix had
        -- relocated from HVSIZE_READ → SPM addr to MVSIZE_reg → perm_next_read_chunk.
        if to_integer(unsigned(HVSIZE(harc_EXEC))) > 0 and
           SIMD_RD_BYTES_wire(h) > 0 then
          v_chunk_count_init :=
            to_integer(unsigned(HVSIZE(harc_EXEC))) / SIMD_RD_BYTES_wire(h);
        else
          v_chunk_count_init := 0;
        end if;
        if v_chunk_count_init > 0 then
          -- chunk_shift_raw = RS1[7:0] / SIMD (SIMD is a compile-time
          -- power-of-2 constant -> synthesises to a barrel-shifter)
          v_chunk_shift_raw := to_integer(unsigned(RS1_Data_IE(15 downto 0))) / SIMD;
          -- Single-subtract defensive mod (SW glue makes this a no-op).
          if v_chunk_shift_raw >= v_chunk_count_init then
            v_chunk_shift_init := v_chunk_shift_raw - v_chunk_count_init;
          else
            v_chunk_shift_init := v_chunk_shift_raw;
          end if;
          -- (chunk_shift + 1) mod chunk_count via compare-select.
          -- [20260928 permfix] Only if the dsp_init read of chunk_shift was granted
          -- in this cycle; otherwise dsp_exec must start again from chunk_shift
          -- (SPM busy, e.g. a preceding kmemld still writing: HV >= 64 B), else the
          -- whole output is shifted by one chunk.
          if hdc_data_gnt_i(h) = '0' then
            perm_next_read_chunk_int(h) <= v_chunk_shift_init;
          elsif v_chunk_shift_init = v_chunk_count_init - 1 then
            perm_next_read_chunk_int(h) <= 0;
          else
            perm_next_read_chunk_int(h) <= v_chunk_shift_init + 1;
          end if;
        else
          perm_next_read_chunk_int(h) <= 0;
        end if;
        perm_read_chunk_idx_int(h) <= 0;
      elsif perm_en(h) = '1' and hdc_data_gnt_i(h) = '1' and
            perm_chunk_count_int(h) > 0 then
        -- Each grant advances the read chunk index by 1, mod N.
        if perm_next_read_chunk_int(h) = perm_chunk_count_int(h) - 1 then
          perm_next_read_chunk_int(h) <= 0;
        else
          perm_next_read_chunk_int(h) <= perm_next_read_chunk_int(h) + 1;
        end if;
        if perm_read_chunk_idx_int(h) = perm_chunk_count_int(h) - 1 then
          perm_read_chunk_idx_int(h) <= 0;
        else
          perm_read_chunk_idx_int(h) <= perm_read_chunk_idx_int(h) + 1;
        end if;
      end if;
    end if;
  end process;

  HDC_FU_ENABLER_SYNC : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then

      ------- FHRR BUNDLING ----------
      bundle_en(h)         <= '0';
      bundle_en_pending(h) <= '0';
      ---------------------------

      ------- BINDING -----------
      bind_en(h)           <= '0';
      bind_en_pending(h)   <= '0';
      ---------------------------

       ------- ENCODING -----------
       enc_en(h)           <= '0';
       enc_en_pending(h)   <= '0';
       ---------------------------
      
      ------- SIMILARITY --------
      sim_en(h)           <= '0';
      sim_en_pending(h)   <= '0';  
      ---------------------------

      ------- CLIPPING ----------
      clip_en(h)          <= '0';
      clip_en_pending(h)  <= '0';
      div_done_lat(h)     <= (others => (others => '0'));
      div_done_lat2(h)    <= (others => (others => '0'));
      ---------------------------

      ------- PERMUTATION (Op-N2) ----------
      perm_en(h)          <= '0';
      perm_en_pending(h)  <= '0';
      ---------------------------

    elsif rising_edge(clk_i) then

      ------------ FHRR BUNDLING ------------ 
      bundle_en(h)           <= bundle_en_wire(h);
      bundle_en_pending(h)   <= bundle_en_pending_wire(h);
      ---------------------------------- 
      ------------ BINDING ---------------
      bind_en(h)           <= bind_en_wire(h); 
      bind_en_pending(h)   <= bind_en_pending_wire(h);
      ------------------------------------

      ------------ ENCODING ---------------
      enc_en(h)           <= enc_en_wire(h); 
      enc_en_pending(h)   <= enc_en_pending_wire(h);
      ------------------------------------


      ------------ SIMILARITY ------------
      sim_en(h)           <= sim_en_wire(h); 
      sim_en_pending(h)   <= sim_en_pending_wire(h);  
      ------------------------------------

      ------------ CLIPPING --------------
      clip_en(h)          <= clip_en_wire(h);
      clip_en_pending(h)  <= clip_en_pending_wire(h);

      ------------ PERMUTATION (Op-N2) ----
      perm_en(h)          <= perm_en_wire(h);
      perm_en_pending(h)  <= perm_en_pending_wire(h);
      ----------------------------------
      -- Latch div_done by two cycles:
      --   div_done_lat  = div_done delayed 1 cycle (used for tangent capture; result is valid here)
      --   div_done_lat2 = div_done delayed 2 cycles (used for clip_stage_2_en; tangent is stable)
      for i in 0 to SIMD-1 loop
          div_done_lat(h)(i)(1)  <= div_done(h)(i)(1);
          div_done_lat2(h)(i)(1) <= div_done_lat(h)(i)(1);
          if div_done_lat(h)(i)(1) = '1' then
            report "HDC_FU_ENABLER: div_done_lat fired h=" & integer'image(h) & " i=" & integer'image(i) &
                   " tangent=0x" & to_hstring(tangent(0));
          end if;
          if div_done_lat2(h)(i)(1) = '1' then
            report "HDC_FU_ENABLER: div_done_lat2 fired h=" & integer'image(h) & " i=" & integer'image(i) &
                   " tangent=0x" & to_hstring(tangent(0)) &
                   " clip_stage_2_en=" & std_logic'image(clip_stage_2_en(h));
          end if;
      end loop;
      ------------------------------------


    end if;

  end process;

end generate HDC_replicated;

  -------------------------------------------------------------------------------------------------------------------------------------------
  --  ███████╗██╗   ██╗     █████╗  ██████╗ ██████╗███████╗███████╗███████╗    ██╗  ██╗ █████╗ ███╗   ██╗██████╗ ██╗     ███████╗██████╗   --
  --  ██╔════╝██║   ██║    ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝██╔════╝    ██║  ██║██╔══██╗████╗  ██║██╔══██╗██║     ██╔════╝██╔══██╗  --
  --  █████╗  ██║   ██║    ███████║██║     ██║     █████╗  ███████╗███████╗    ███████║███████║██╔██╗ ██║██║  ██║██║     █████╗  ██████╔╝  --
  --  ██╔══╝  ██║   ██║    ██╔══██║██║     ██║     ██╔══╝  ╚════██║╚════██║    ██╔══██║██╔══██║██║╚██╗██║██║  ██║██║     ██╔══╝  ██╔══██╗  --
  --  ██║     ╚██████╔╝    ██║  ██║╚██████╗╚██████╗███████╗███████║███████║    ██║  ██║██║  ██║██║ ╚████║██████╔╝███████╗███████╗██║  ██║  --
  --  ╚═╝      ╚═════╝     ╚═╝  ╚═╝ ╚═════╝ ╚═════╝╚══════╝╚══════╝╚══════╝    ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝  --
  -------------------------------------------------------------------------------------------------------------------------------------------

FU_HANDLER_MC : if multithreaded_accl_en = 0 generate
  HDC_FU_ENABLER_comb : process(all)
  begin
    for h in accl_range loop


      ---------- FHRR BUNDLING -------------
      bundle_en_wire(h)<= bundle_en(h);
      ---------------------------------

      ---------- BINDING --------------
      bind_en_wire(h)   <= bind_en(h); 
      ---------------------------------

      ---------- ENCODING --------------
      enc_en_wire(h)   <= enc_en(h); 
      ---------------------------------

      ---------- SIMILARITY -----------
      sim_en_wire(h)   <= sim_en(h);       
      ---------------------------------

      ---------- CLIPPING -------------
      clip_en_wire(h)  <= clip_en(h);
      ---------------------------------

      ---------- PERMUTATION (Op-N2) --
      perm_en_wire(h)  <= perm_en(h);
      ---------------------------------

      halt_hart(h)     <= '0';


      ----------------- FHRR BUNDLING ---------------------------
      if bundle_en(h) = '1' and busy_HDC_internal(h) = '0' then
        bundle_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- BINDING ----------------------------
      if bind_en(h) = '1' and busy_HDC_internal(h) = '0' then
        bind_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- ENCODING ----------------------------
      if enc_en(h) = '1' and busy_HDC_internal(h) = '0' then
        enc_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- SIMILARITY -------------------------
      if sim_en(h) = '1' and busy_HDC_internal(h) = '0' then
        sim_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- CLIPPING ---------------------------
      if clip_en(h) = '1' and busy_HDC_internal(h) = '0' then
        clip_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- PERMUTATION (Op-N2) ----------------
      if perm_en(h) = '1' and busy_HDC_internal(h) = '0' then
        perm_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------



      if hdc_instr_req(h) = '1' or busy_HDC_internal_lat(h) = '1' then

        case state_HDC(h) is

          when dsp_init =>

            ------------------------ FHRR BUNDLING ---------------------------------
            if decoded_instruction_DSP(HVBUNDLE_bit_position)    = '1' then 
              bundle_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ------------------------ BINDING ----------------------------------
            elsif decoded_instruction_DSP(HVBIND_bit_position) = '1' then
              bind_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ------------------------ ENCODING ----------------------------------
            elsif decoded_instruction_DSP(HVENC_bit_position  ) = '1' then
              enc_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ---------------------- SIMILARITY ---------------------------------
            elsif decoded_instruction_DSP(HVSIM_bit_position)    = '1' then
              sim_en_wire(h) <= '1';                                         
            -------------------------------------------------------------------

            ---------------------- CLIPPING -----------------------------------
            elsif decoded_instruction_DSP(HVCLIP_bit_position  ) = '1' then
              clip_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ---------------------- PERMUTATION (Op-N2) ------------------------
            elsif decoded_instruction_DSP(HVPERM_bit_position) = '1' then
              perm_en_wire(h) <= '1';
            -------------------------------------------------------------------

            end if;
          when others =>
            null;
        end case;
      end if;
    end loop;
  end process;
end generate FU_HANDLER_MC;

FU_HANDLER_MT : if multithreaded_accl_en = 1 generate
  HDC_FU_ENABLER_comb : process(all)
  begin

    for h in accl_range loop

      
      -- ----------------------------------------------
      ------------- FHRR BUNDLING -----------------------
      bundle_en_wire(h)               <= bundle_en(h);
      bundle_en_pending_wire(h)       <= bundle_en_pending(h);
      ----------------------------------------------
      ------------- BINDING ------------------------
      bind_en_wire(h)                 <= bind_en(h);
      bind_en_pending_wire(h)         <= bind_en_pending(h);
      ----------------------------------------------

      ------------- ENCODING ------------------------
      
      enc_en_wire(h)                 <= enc_en(h);
      enc_en_pending_wire(h)         <= enc_en_pending(h);
      ----------------------------------------------
      
      ------------- SIMILARITY ---------------------
      sim_en_wire(h)                 <= sim_en(h);  
      sim_en_pending_wire(h)         <= sim_en_pending(h);        
      ----------------------------------------------

      ------------- CLIPPING -----------------------
      clip_en_wire(h)                <= clip_en(h);
      clip_en_pending_wire(h)        <= clip_en_pending(h);
      ---------------------------------------------

      ------------- PERMUTATION (Op-N2 Phase A) -----
      -- TODO Op-N2 Phase B: full MT arbitration for HVPERM via fu_req(7)/fu_gnt(7)
      perm_en_wire(h)                <= perm_en(h);
      perm_en_pending_wire(h)        <= perm_en_pending(h);
      ----------------------------------------------

      fu_req(h)                      <= (others => '0');
      halt_hart(h)                   <= '0';
      
      

       -----------------FHRR BUNDLING ---------------------------
       if bundle_en(h) = '1' and busy_HDC_internal(h) = '0' then
        bundle_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- BINDING ----------------------------
      if bind_en(h) = '1' and busy_HDC_internal(h) = '0' then
        bind_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- ENCODING ----------------------------
      if enc_en(h) = '1' and busy_HDC_internal(h) = '0' then
        enc_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ---------------- SIMILARITY --------------------------
      if sim_en(h) = '1' and busy_HDC_internal(h) = '0' then
        sim_en_wire(h) <= '0';   
      end if;                                 
      -----------------------------------------------------

      ----------------- CLIPPING --------------------------
      if clip_en(h) = '1' and busy_HDC_internal(h) = '0' then
        clip_en_wire(h) <= '0';
      end if;
      -----------------------------------------------------

      ----------------- PERMUTATION (Op-N2) ----------------
      if perm_en(h) = '1' and busy_HDC_internal(h) = '0' then
        perm_en_wire(h) <= '0';
      end if;
      -----------------------------------------------------



      if hdc_instr_req(h) = '1' or busy_HDC_internal_lat(h) = '1' then

        case state_HDC(h) is

          when dsp_init =>



             ------------------------ FHRR BUNDLING --------------------------------
             if decoded_instruction_DSP(HVBUNDLE_bit_position)    = '1' then
              if busy_bundle = '0' and bundle_en_pending = (accl_range => '0') then 
                bundle_en_wire(h) <= '1';
              else
                bundle_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(0) <= '1';
              end if;
            ------------------------------------------------------------------


            ---------------------- BINDING -----------------------------------
            elsif decoded_instruction_DSP(HVBIND_bit_position) = '1' then
              if busy_bind = '0' and bind_en_pending = (accl_range => '0') then 
                bind_en_wire(h) <= '1';
              else
                bind_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(2) <= '1';
              end if;
            ------------------------------------------------------------------

            ---------------------- ENCODING -----------------------------------
            elsif decoded_instruction_DSP(HVENC_bit_position  ) = '1' then
              if busy_enc = '0' and enc_en_pending = (accl_range => '0') then 
                enc_en_wire(h) <= '1';
              else
                enc_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(3) <= '1';
              end if;
            ------------------------------------------------------------------

            ----------------------- SIMILARITY -------------------------------
            elsif decoded_instruction_DSP(HVSIM_bit_position)    = '1' then
              if busy_sim = '0' and sim_en_pending = (accl_range => '0') then 
                sim_en_wire(h) <= '1';                                        
              else
                sim_en_pending_wire(h) <= '1';                                   
                halt_hart(h) <= '1';
                fu_req(h)(5) <= '1';
              end if;
            -------------------------------------------------------------------

            ----------------------- CLIPPING -----------------------------------
            elsif decoded_instruction_DSP(HVCLIP_bit_position  ) = '1' then
              if busy_clip = '0' and clip_en_pending = (accl_range => '0') then
                clip_en_wire(h) <= '1';
              else
                clip_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(6) <= '1';
              end if;
            --------------------------------------------------------------------

            ----------------------- PERMUTATION (Op-N2 Phase A) ----------------
            -- Phase A: drive perm_en directly (no fu_req(7) arbitration). Production
            -- builds run with multithreaded_accl_en=0 so this branch is essentially
            -- unexercised; Phase B will add proper fu_req(7)/fu_gnt(7) handshaking.
            elsif decoded_instruction_DSP(HVPERM_bit_position) = '1' then
              perm_en_wire(h) <= '1';
            --------------------------------------------------------------------

            end if;

          when dsp_halt_hart =>
  
            if fu_gnt(h)(0) = '1' then
              bundle_en_wire(h) <= '1';
              bundle_en_pending_wire(h) <= '0';
            elsif bundle_en_pending(h) = '1' and fu_gnt(h)(0) = '0'  then
              halt_hart(h) <= '1';
            end if;

            if fu_gnt(h)(2) = '1' then
              bind_en_wire(h) <= '1';
              bind_en_pending_wire(h) <= '0';
            elsif bind_en_pending(h) = '1' and fu_gnt(h)(2) = '0'  then
              halt_hart(h) <= '1';
            end if;

            if fu_gnt(h)(3) = '1' then
              enc_en_wire(h) <= '1';
              enc_en_pending_wire(h) <= '0';
            elsif enc_en_pending(h) = '1' and fu_gnt(h)(3) = '0'  then
              halt_hart(h) <= '1';
            end if;

            if fu_gnt(h)(5) = '1' then
              sim_en_wire(h) <= '1';
              sim_en_pending_wire(h) <= '0';
            elsif sim_en_pending(h) = '1' and fu_gnt(h)(5) = '0'  then
              halt_hart(h) <= '1';
            end if;

            if fu_gnt(h)(6) = '1' then
              clip_en_wire(h) <= '1';
              clip_en_pending_wire(h) <= '0';
            elsif clip_en_pending(h) = '1' and fu_gnt(h)(6) = '0'  then
              halt_hart(h) <= '1';
            end if;
            
        --7 was from permutation

        --8 was for ass. search

          when others =>
            null;
        end case;
      end if;
    end loop;
  end process;

  FU_Issue_Buffer_sync : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then
      fu_rd_ptr  <= (others => (others => '0'));
      fu_wr_ptr  <= (others => (others => '0'));
      fu_gnt     <= (others => (others => '0'));
    elsif rising_edge(clk_i) then
      fu_gnt <= fu_gnt_wire;
      for h in accl_range loop
        for i in 0 to 4 loop  -- Loop index 'i' is for the total number of different functional units (regardless what SIMD config is set)
          if fu_req(h)(i) = '1' then  -- if a reservation was made, to use a functional unit
            --to_integer(unsigned(fu_issue_buffer(i)(to_integer(unsigned(fu_wr_ptr(i)))))) <= h;  -- store the thread_ID in its corresponding buffer at the fu_wr_ptr position
            --fu_issue_buffer(to_integer(unsigned(fu_wr_ptr(i))))(i) <= std_logic_vector(unsigned(h));  -- store the thread_ID in its corresponding buffer at the fu_wr_ptr position
            fu_issue_buffer(i)(to_integer(unsigned(fu_wr_ptr(i))))  <= std_logic_vector(to_unsigned(h,TPS_CEIL));
            if unsigned(fu_wr_ptr(i)) = THREAD_POOL_SIZE - 2 then -- increment the pointer wr logic
              fu_wr_ptr(i) <= (others => '0');
            else
              fu_wr_ptr(i) <= std_logic_vector(unsigned(fu_wr_ptr(i)) + 1);
            end if;
          end if;
          case state_HDC(h) is
            when dsp_halt_hart =>
              if fu_gnt_en(h)(i) = '1' then
                if unsigned(fu_rd_ptr(i)) = THREAD_POOL_SIZE - 2 then  -- increment the read pointer
                  fu_rd_ptr(i) <= (others => '0');
                else
                  fu_rd_ptr(i) <= std_logic_vector(unsigned(fu_rd_ptr(i)) + 1);
                end if;
              end if;
            when others =>
             null;
          end case;
        end loop;
      end loop;
    end if;
  end process;

  FU_Issue_Buffer_comb : process(all)
  begin
    for h in accl_range loop
      fu_gnt_wire(h) <= (others => '0');
      fu_gnt_en(h)   <= (others => '0');

     

       ------------------- FHRR BUNDLING -----------------------
       if bundle_en_pending_wire(h) = '1' and busy_bundle_wire = '0' then
        fu_gnt_en(h)(0) <= '1';
      end if;
      ----------------------------------------------------

      ------------------- BINDING ------------------------
      if bind_en_pending_wire(h) = '1' and busy_bind_wire = '0' then
        fu_gnt_en(h)(2) <= '1';
      end if;
      ----------------------------------------------------

      ------------------- ENCODING ------------------------
      if enc_en_pending_wire(h) = '1' and busy_enc_wire = '0' then
        fu_gnt_en(h)(3) <= '1';
      end if;
      ----------------------------------------------------

      -------------------- SIMILARITY --------------------
      if sim_en_pending_wire(h) = '1' and busy_sim_wire = '0' then
        fu_gnt_en(h)(5) <= '1';
      end if;
      ----------------------------------------------------

      -------------------- CLIPPING ----------------------
      if clip_en_pending_wire(h) = '1' and busy_clip_wire = '0' then
        fu_gnt_en(h)(6) <= '1';
      end if;
      ----------------------------------------------------
      
      case state_HDC(h) is
        when dsp_halt_hart =>
          for i in 0 to 4 loop 
            if fu_gnt_en(h)(i) = '1' then
              fu_gnt_wire(to_integer(unsigned(fu_issue_buffer(i)(to_integer(unsigned(fu_rd_ptr(i)))))))(i) <= '1'; -- give a grant to fu_gnt(h)(i), such that the 'h' index points to the thread in "fu_issue_buffer"
            end if;
          end loop;
        when others =>
          null;
      end case;
    end loop;
  end process;


  HDC_BUSY_FU_SYNC : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then
      
    elsif rising_edge(clk_i) then

    

      --------FHRR BUNDLING ----------
      busy_bundle  <= busy_bundle_wire;
      ----------------------------
 

      -------- BINDING ------------
      busy_bind    <= busy_bind_wire;
      -----------------------------

      -------- ENCODING ------------
      busy_enc    <= busy_enc_wire;
      ----------------------------- 

      
      -------- SIMILARITY --------
      busy_sim    <= busy_sim_wire;
      ----------------------------

      -------- CLIPPING ----------
      busy_clip   <= busy_clip_wire;
      ----------------------------

      -------- PERMUTATION (Op-N2) -
      busy_perm   <= busy_perm_wire;
      ----------------------------

    end if;
  end process;

end generate FU_HANDLER_MT;

-------- FHRR BUNDLING ----------
busy_bundle_wire <= '1' when multithreaded_accl_en = 1 and bundle_en_wire   /= (accl_range => '0') else '0';
----------------------------

-------- BINDING -----------
busy_bind_wire <= '1' when multithreaded_accl_en = 1 and bind_en_wire   /= (accl_range => '0') else '0';
----------------------------

-------- ENCODING -----------
busy_enc_wire <= '1' when multithreaded_accl_en = 1 and enc_en_wire   /= (accl_range => '0') else '0';
----------------------------

-------- SIMILARITY --------
busy_sim_wire <= '1' when multithreaded_accl_en = 1 and sim_en_wire   /= (accl_range => '0') else '0';
----------------------------

-------- CLIPPING ----------
busy_clip_wire <= '1' when multithreaded_accl_en = 1 and clip_en_wire /= (accl_range => '0') else '0';
----------------------------

-------- PERMUTATION (Op-N2) -
busy_perm_wire <= '1' when multithreaded_accl_en = 1 and perm_en_wire /= (accl_range => '0') else '0';
----------------------------



  -----------------------------------------------------------------
  --  ███╗   ███╗ █████╗ ██████╗ ██████╗ ██╗███╗   ██╗ ██████╗   --
  --  ████╗ ████║██╔══██╗██╔══██╗██╔══██╗██║████╗  ██║██╔════╝   --
  --  ██╔████╔██║███████║██████╔╝██████╔╝██║██╔██╗ ██║██║  ███╗  --
  --  ██║╚██╔╝██║██╔══██║██╔═══╝ ██╔═══╝ ██║██║╚██╗██║██║   ██║  --
  --  ██║ ╚═╝ ██║██║  ██║██║     ██║     ██║██║ ╚████║╚██████╔╝  --
  --  ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝     ╚═╝╚═╝  ╚═══╝ ╚═════╝   --
  -----------------------------------------------------------------

MULTICORE_OUT_MAPPER : if multithreaded_accl_en = 0 generate
MAPPER_replicated : for h in fu_range generate

  MAPPING_OUT_UNIT_comb : process(all)
  begin
      hdc_sc_data_write_wire_int(h)  <= (others => '0');
      hdc_sc_data_write_wire(h)      <= hdc_sc_data_write_wire_int(h);
      -- Questo segnale stabilisce il numero di byte che la HDC è in grado di leggere in un ciclo di clock
      -- in funzione del parallelismo (SIMD). Nel caso della ricerca assorciativa questa  


      SIMD_RD_BYTES_wire(h)          <= SIMD*(Data_Width/8);

      if hdc_instr_req(h) = '1' or busy_HDC_internal_lat(h) = '1' then
        case state_HDC(h) is
          
          when dsp_init =>


          when dsp_exec =>

        ---------------------------FHRR BUNDLING ------------------------------
            if decoded_instruction_DSP_lat(h)(HVBUNDLE_bit_position) = '1' then
                
              for i in 0 to SIMD-1 loop
		              if ((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_WRITE(h)))) / SIMD_RD_BYTES_wire(h)) >= 0 and HVSIZE_WRITE(h) /= (Addr_Width downto 0 => 'U') then
		                if (((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_WRITE(h)))) / SIMD_RD_BYTES_wire(h)) mod 2 = 0) then
                    hdc_sc_data_write_wire_int(h)(Data_Width + Data_Width*i -1 downto Data_Width*i) <=
                      std_logic_vector(signed(sin_lut_lookup(to_integer(unsigned(hdc_sc_data_read(h)(1)((8 - 1) + Data_Width * i downto Data_Width * i))))) +
                                       signed(hdc_sc_data_read(h)(0)(Data_Width + Data_Width*i -1 downto Data_Width*i)));
	                else
	                    hdc_sc_data_write_wire_int(h)(Data_Width + Data_Width*i -1 downto Data_Width*i) <=
                        std_logic_vector(signed(cos_lut_lookup(to_integer(unsigned(bundle_phase_hold(h)((8 - 1) + Data_Width * i downto Data_Width * i))))) +
                                         signed(hdc_sc_data_read(h)(0)(Data_Width + Data_Width*i -1 downto Data_Width*i)));
	                end if;
	            end if;
            end loop;
              end if;
            -------------------------------------------------------------------

            --------------------------- BINDING --------------------------------
            if (decoded_instruction_DSP_lat(h)(HVBIND_bit_position)    = '1' ) then
            
              for i in 0 to SIMD-1 loop
                if bind_en(h) = '1' and bind_stage_1_en(h) = '1' then
                  hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_bind_results_wire(h)((8*(i+1))-1 downto 8*i);
                else
                  hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_bind_results(h)((8*(i+1))-1 downto 8*i);
                end if;
                
              end loop;
              end if;
            --------------------------------------------------------------------
            

            --------------------------- ENCODING --------------------------------
            if (decoded_instruction_DSP_lat(h)(HVENC_bit_position )    = '1' ) then

              for i in 0 to SIMD-1 loop
                
                hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_enc_result(h)((8*(i+1))-1 downto 8*i);
                
              end loop;
            end if;
            --------------------------------------------------------------------
            -------------------------- SIMILARITY -------------------------------
            if decoded_instruction_DSP_lat(h)(HVSIM_bit_position)      = '1' then
              hdc_sc_data_write_wire_int(h)(Data_Width -1  downto 0) <= cosine_accumulator_avg(h);
            end if;
            ---------------------------------------------------------------------

            ------------------------ CLIPPING -----------------------------------
            if decoded_instruction_DSP_lat(h)(HVCLIP_bit_position  )   = '1' then
              hdc_sc_data_write_wire_int(h)(SIMD_Width - 1 downto 0) <= hdcu_out_clip_results(h)(SIMD_Width - 1 downto 0) ;
            end if;
            ----------------------------------------------------------------------

            ------------------------ PERMUTATION (Op-N2) -------------------------
            -- Unpack the SIMD packed bytes back into the SPM 32-bit-strided layout
            -- (low byte of each 32-bit slot). Mirrors the BIND output mapping and
            -- ensures dst[i].low_byte = the rotated phase.
            if decoded_instruction_DSP_lat(h)(HVPERM_bit_position) = '1' then
              for i in 0 to SIMD-1 loop
                if perm_en(h) = '1' and perm_stage_2_en(h) = '1' then
                  hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_perm_results_wire(h)((8*(i+1))-1 downto 8*i);
                else
                  hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_perm_results(h)((8*(i+1))-1 downto 8*i);
                end if;
              end loop;
            end if;
            ----------------------------------------------------------------------

            if halt_hdc(h) = '0' and halt_hdc_lat(h) = '1' then
              hdc_sc_data_write_wire(h) <= hdc_sc_data_write_int(h);
            end if;
          when others =>
            null;
        end case;
      end if;
  end process;

end generate;
end generate;

MULTITHREAD_OUT_MAPPER : if multithreaded_accl_en = 1 generate
  MAPPING_OUT_UNIT_comb : process(all)
  begin
    for h in 0 to (ACCL_NUM - FU_NUM) loop
      hdc_sc_data_write_wire_int(h)  <= (others => '0');
      hdc_sc_data_write_wire(h)      <= hdc_sc_data_write_wire_int(h);
      SIMD_RD_BYTES_wire(h)          <= SIMD*(Data_Width/8);

      if hdc_instr_req(h) = '1' or busy_HDC_internal_lat(h) = '1' then
        
        case state_HDC(h) is
          
          when dsp_init =>

         
          when dsp_exec =>
            ------------------------- FHRR BUNDLING --------------------------------
            if decoded_instruction_DSP_lat(h)(HVBUNDLE_bit_position) = '1' then
                for i in 0 to SIMD-1 loop
		                  if ((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_WRITE(h)))) / SIMD_RD_BYTES_wire(h)) >= 0 and HVSIZE_WRITE(h) /= (Addr_Width downto 0 => 'U') then
		                    if (((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_WRITE(h)))) / SIMD_RD_BYTES_wire(h)) mod 2 = 0) then
	                        hdc_sc_data_write_wire_int(h)(Data_Width + Data_Width*i -1 downto Data_Width*i) <=
                            std_logic_vector(signed(sin_lut_lookup(to_integer(unsigned(hdc_sc_data_read(h)(1)((8 - 1) + Data_Width * i downto Data_Width * i))))) +
                                             signed(hdc_sc_data_read(h)(0)(Data_Width + Data_Width*i -1 downto Data_Width*i)));
	                    else
	                        hdc_sc_data_write_wire_int(h)(Data_Width + Data_Width*i -1 downto Data_Width*i) <=
                            std_logic_vector(signed(cos_lut_lookup(to_integer(unsigned(bundle_phase_hold(h)((8 - 1) + Data_Width * i downto Data_Width * i))))) +
                                             signed(hdc_sc_data_read(h)(0)(Data_Width + Data_Width*i -1 downto Data_Width*i)));
	                    end if;
	                end if;
                end loop;
                  end if;

            ----------------------------------------------------------------

            ------------------------- BINDING --------------------------------
            if (decoded_instruction_DSP_lat(h)(HVBIND_bit_position)    = '1' ) then
              for i in 0 to SIMD-1 loop
                if bind_en(h) = '1' and bind_stage_1_en(h) = '1' then
                  hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_bind_results_wire(0)((8*(i+1))-1 downto 8*i);
                else
                  hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_bind_results(0)((8*(i+1))-1 downto 8*i);
                end if;
                
              end loop;
            end if;
            -------------------------------------------------------------------

            ------------------------- ENCODING --------------------------------
            if (decoded_instruction_DSP_lat(h)(HVENC_bit_position )    = '1' ) then
              --hdc_sc_data_write_wire_int(h)(SIMD_Width - (Data_Width-8)*SIMD_Width/Data_Width -1 downto 0) <=  hdc_out_enc_result(h);

              for i in 0 to SIMD-1 loop
                
                hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_enc_result(0)((8*(i+1))-1 downto 8*i);
                
              end loop;
            end if;
            -------------------------------------------------------------------
            
            ------------------------- SIMILARITY ------------------------------
            if decoded_instruction_DSP_lat(h)(HVSIM_bit_position)      = '1' then
              hdc_sc_data_write_wire_int(h)(Data_Width -1 downto 0) <=  cosine_accumulator_avg(0);              
            end if;
            -------------------------------------------------------------------

            ------------------------- CLIPPING --------------------------------
            if decoded_instruction_DSP_lat(h)(HVCLIP_bit_position  )   = '1' then
              hdc_sc_data_write_wire_int(h)(SIMD_Width - 1 downto 0) <= hdcu_out_clip_results(0)(SIMD_Width - 1 downto 0) ;

            end if;
            -------------------------------------------------------------------

            ------------------------- PERMUTATION (Op-N2) ---------------------
            -- MT mode: FU index fixed to 0; same packed-byte unpacking as MC.
            if decoded_instruction_DSP_lat(h)(HVPERM_bit_position) = '1' then
              for i in 0 to SIMD-1 loop
                if perm_en(h) = '1' and perm_stage_2_en(h) = '1' then
                  hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_perm_results_wire(0)((8*(i+1))-1 downto 8*i);
                else
                  hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_perm_results(0)((8*(i+1))-1 downto 8*i);
                end if;
              end loop;
            end if;
            -------------------------------------------------------------------

            if halt_hdc(h) = '0' and halt_hdc_lat(h) = '1' then
              hdc_sc_data_write_wire(h) <= hdc_sc_data_write_int(h);
            end if;

          when others =>
            null;

        end case;
      end if;
    end loop;
  end process;
end generate;


FU_replicated : for f in fu_range generate

  HDC_MAPPING_IN_UNIT_comb : process(all)
  
  variable h : integer;

  begin

    -------------------FHRR BUNDLING ---------------------
    hdcu_in_bundle_operand(f)     <= (others =>  '0');
    -------------------------------------------------


    ------------------- BINDING --------------------
    hdcu_in_bind_operands(f)        <= (others => (others => '0'));
    --------------------------------------------------
    
    ------------------- ENCODING --------------------
    hdcu_in_enc_fpp_operands(f)        <= (others => (others => '0'));
    --------------------------------------------------

    ------------------- SIMILARITY -------------------
    hdcu_in_sim_operands(f)         <= (others => (others => '0'));
    -------------------------------------------------

    ------------------- CLIPPING ---------------------
    hdcu_in_clip_operands(f)       <= (others => (others => '0'));
    -------------------------------------------------

    ------------------- PERMUTATION (Op-N2) ----------
    hdcu_in_perm_operand(f)        <= (others => '0');
    -- [Op-N2 Phase B] hdcu_in_perm_operand_b is the registered "previous chunk"
    -- and is driven by perm_chunk_b_latch_sync below; do NOT default-assign
    -- here (it must hold its registered value across the combinational defaults
    -- of MAPPING_IN_UNIT_comb).
    -------------------------------------------------

    for g in 0 to (ACCL_NUM - FU_NUM) loop

      if multithreaded_accl_en = 1 then
        h := g;  -- set the spm rd/wr ports equal to the "for-loop"
      elsif multithreaded_accl_en = 0 then
        h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
      end if;

      if hdc_instr_req(h) = '1' or busy_HDC_internal_lat(h) = '1' then
        case state_HDC(h) is
          
          when dsp_exec =>

            ---------------------------- FHRR BUNDLING --------------------------------------
             if decoded_instruction_DSP_lat(h)(HVBUNDLE_bit_position) = '1' then
              hdcu_in_bundled(f) <= hdc_sc_data_read(h)(0);
            
                 for i in 0 to SIMD-1 loop
                hdcu_in_bundle_operand(f)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(1)((8-1)+32*i downto 32*i);
                
                end loop;
            
              end if;
            --------
            -------------------------------------------------------------------------

            ----------------------------- BINDING -----------------------------------
            if decoded_instruction_DSP_lat(h)(HVBIND_bit_position)  = '1' then 
              
              for i in 0 to SIMD-1 loop
                hdcu_in_bind_operands(f)(0)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(0)((8-1)+32*i downto 32*i);
                hdcu_in_bind_operands(f)(1)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(1)((8-1)+32*i downto 32*i);
            end loop;
            
            end if;
            ------------------------------------------------------------------------

            ----------------------------- ENCODING -----------------------------------
            if decoded_instruction_DSP_lat(h)(HVENC_bit_position )  = '1' then
                         
            hdcu_in_enc_fpp_operands(f)(0)(7 downto 0) <= hdc_sc_data_read(h)(0)(7 downto 0);


            for i in 0 to SIMD-1 loop
              --hdcu_in_enc_fpp_operands(f)(0)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(0)((8-1)+32*i downto 32*i);
              hdcu_in_enc_fpp_operands(f)(1)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(1)((8-1)+32*i downto 32*i);
            end loop;
            end if; 
            ------------------------------------------------------------------------

            ----------------------------- SIMILARITY -------------------------------
           

            if (decoded_instruction_DSP_lat(h)(HVSIM_bit_position)  = '1')  then

              for i in 0 to SIMD-1 loop
                hdcu_in_sim_operands(f)(0)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(0)((8-1)+32*i downto 32*i);
                hdcu_in_sim_operands(f)(1)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(1)((8-1)+32*i downto 32*i);
            end loop;
            

        end if;
            ------------------------------------------------------------------------

             --------------------------- CLIPPING ----------------------------------
             if decoded_instruction_DSP_lat(h)(HVCLIP_bit_position  ) = '1' then
              hdcu_in_clip_operands(f)(0) <= hdc_sc_data_read(h)(0);
              hdcu_in_clip_operands(f)(1) <= hdc_sc_data_read(h)(1);
            end if;
            ------------------------------------------------------------------------

            --------------------------- PERMUTATION (Op-N2 Phase B) ---------------
            -- HVPERM Phase B streams chunks sequentially on SPM channel 1.
            -- The current SPM word maps to hdcu_in_perm_operand combinationally;
            -- perm_chunk_b_latch_sync registers the previous-cycle value into
            -- hdcu_in_perm_operand_b so the splice mux has both halves at once.
            -- Each 32-bit SPM lane carries one phase byte in its low 8 bits;
            -- the mapping packs the SIMD low-bytes into consecutive bytes,
            -- mirroring the BIND second-operand mapping above.
            -- [Phase A bug fix 20260507] kept: chunk channel is 1.
            -- [Phase B 20260507] single-channel pipelined; chunk_b sourced via
            -- a dedicated synchronous latch instead of a 2nd SPM read port.
            if decoded_instruction_DSP_lat(h)(HVPERM_bit_position) = '1' then
              for i in 0 to SIMD-1 loop
                hdcu_in_perm_operand(f)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(1)((8-1)+32*i downto 32*i);
              end loop;
            end if;
            ------------------------------------------------------------------------
            

          when others =>
            null;
        end case;
      end if;
    end loop;
  end process;

  bind_operand_latch_sync : process(clk_i, rst_ni)
    variable h : integer;
  begin
    if rst_ni = '0' then
      -- [Fix-MDRV] Scope reset to (f) slice only; whole-array reset here causes
      -- multiple-driver DRC when FU_replicated generates more than one instance.
      hdcu_in_bind_operands_lat(f) <= (others => (others => '0'));
    elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;
        else
          h := f;
        end if;

        if halt_hdc_lat(h) = '0' then
          -- Keep bind operands stable after the scratchpad read window closes.
          if bind_stage_1_en(h) = '1' then
            hdcu_in_bind_operands_lat(f) <= hdcu_in_bind_operands(f);
          end if;
        end if;
      end loop;
    end if;
  end process;

  ----------------------------------------------------------------------------------------------------------
  -- ██████╗ ██╗   ██╗███╗   ██╗██████╗ ██╗     ██╗███╗   ██╗ ██████╗     ██╗   ██╗███╗   ██╗██╗████████╗ --
  -- ██╔══██╗██║   ██║████╗  ██║██╔══██╗██║     ██║████╗  ██║██╔════╝     ██║   ██║████╗  ██║██║╚══██╔══╝ --
  -- ██████╔╝██║   ██║██╔██╗ ██║██║  ██║██║     ██║██╔██╗ ██║██║  ███╗    ██║   ██║██╔██╗ ██║██║   ██║    --
  -- ██╔══██╗██║   ██║██║╚██╗██║██║  ██║██║     ██║██║╚██╗██║██║   ██║    ██║   ██║██║╚██╗██║██║   ██║    --
  -- ██████╔╝╚██████╔╝██║ ╚████║██████╔╝███████╗██║██║ ╚████║╚██████╔╝    ╚██████╔╝██║ ╚████║██║   ██║    -- 
  -- ╚═════╝  ╚═════╝ ╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝      ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --                                                                                                
  ----------------------------------------------------------------------------------------------------------




  fsm_polar2cart_sync : process (clk_i, rst_ni)
    variable h : integer;
    variable phase_idx : natural range 0 to 255;
    variable phase_sin : signed(Data_Width - 1 downto 0);
    variable phase_cos : signed(Data_Width - 1 downto 0);
    variable bundled_lane : signed(Data_Width - 1 downto 0);
  begin
    if rst_ni = '0' then
      -- [Fix-MDRV] Scope reset to (f) slice only to avoid multiple-driver DRC.
      hdcu_in_bundle_operand_real(f) <= (others => '0');
      hdcu_in_bundle_operand_imag(f) <= (others => '0');
      real_accum(f) <= (others => '0');
      imag_accum(f) <= (others => '0');
  
    elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;
        else
          h := f;
        end if;
  
        if halt_hdc_lat(h) = '0' then
          if bundle_en(h) = '1' and (bundle_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
  
            --index_val := unsigned(index_operand_0(f));
  
            for i in 0 to SIMD - 1 loop
              phase_idx := to_integer(unsigned(hdc_sc_data_read(h)(1)((8 - 1) + Data_Width * i downto Data_Width * i)));
              phase_sin := signed(sin_lut_lookup(phase_idx));
              phase_cos := signed(cos_lut_lookup(phase_idx));
              bundled_lane := signed(hdc_sc_data_read(h)(0)((Data_Width - 1) + Data_Width * i downto Data_Width * i));

              hdcu_in_bundle_operand_imag(f)((Data_Width - 1) + Data_Width * i downto Data_Width * i) <= std_logic_vector(phase_sin);
              hdcu_in_bundle_operand_real(f)((Data_Width - 1) + Data_Width * i downto Data_Width * i) <= std_logic_vector(phase_cos);
              imag_accum(f)((Data_Width - 1) + Data_Width * i downto Data_Width * i) <= std_logic_vector(phase_sin + bundled_lane);
              real_accum(f)((Data_Width - 1) + Data_Width * i downto Data_Width * i) <= std_logic_vector(phase_cos + bundled_lane);
            end loop;
  
          end if;
        end if;
      end loop;
    end if;
  end process;
  

    fsm_comps_accum : process (all)
    begin
      null;
    end process;



  --------------------------------------------------------------------------------------------
  -- ██████╗ ██╗███╗   ██╗██████╗ ██╗███╗   ██╗ ██████╗     ██╗   ██╗███╗   ██╗██╗████████╗ --
  -- ██╔══██╗██║████╗  ██║██╔══██╗██║████╗  ██║██╔════╝     ██║   ██║████╗  ██║██║╚══██╔══╝ --
  -- ██████╔╝██║██╔██╗ ██║██║  ██║██║██╔██╗ ██║██║  ███╗    ██║   ██║██╔██╗ ██║██║   ██║    --
  -- ██╔══██╗██║██║╚██╗██║██║  ██║██║██║╚██╗██║██║   ██║    ██║   ██║██║╚██╗██║██║   ██║    --
  -- ██████╔╝██║██║ ╚████║██████╔╝██║██║ ╚████║╚██████╔╝    ╚██████╔╝██║ ╚████║██║   ██║    --
  -- ╚═════╝ ╚═╝╚═╝  ╚═══╝╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝      ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --                                                                                                                                                            
  --------------------------------------------------------------------------------------------

  bind_add_comb : process(all)
    variable h : integer;
  begin
    hdc_out_bind_results_wire(f) <= (others => '0');
    for g in 0 to (ACCL_NUM - FU_NUM) loop
      if multithreaded_accl_en = 1 then
        h := g;
      else
        h := f;
      end if;

      if halt_hdc_lat(h) = '0' then
        for i in 0 to SIMD-1 loop
          hdc_out_bind_results_wire(f)((8-1) + 8*i downto 8*i) <= std_logic_vector(unsigned(hdcu_in_bind_operands(f)(0)((8-1) + 8*i downto 8*i)) + unsigned(hdcu_in_bind_operands(f)(1)((8-1) + 8*i downto 8*i)));
        end loop;
      end if;
    end loop;
  end process;

  fsm_MUL_STAGE_1 : process(clk_i,rst_ni)
  variable h : integer;
  begin
    if rst_ni = '0' then
      -- [Fix-MDRV] Scope reset to (f) slice only (same issue as bind_operand_latch_sync).
      hdc_out_bind_results(f) <= (others =>'0');
    elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then
          h := f;  -- set the spm rd/wr ports equal to the "for-generate"
        end if;
        if halt_hdc_lat(h) = '0' then
          if bind_stage_2_en(h) = '1' or recover_state_wires(h) = '1' then
            for i in 0 to SIMD-1 loop
                hdc_out_bind_results(f)((8-1) + 8*i downto 8*i)  <= std_logic_vector(unsigned(hdcu_in_bind_operands_lat(f)(0)((8-1) + 8*i downto 8*i))+ unsigned(hdcu_in_bind_operands_lat(f)(1)((8-1) + 8*i downto 8*i)));
            end loop;
          end if;
        end if;
      end loop;
    end if;
  end process;


-- ==========================================
--   FHRR PERMUTATION (Op-N2 Phase B)
-- ==========================================
-- [Op-N2 Phase B] Cross-chunk cyclic barrel. The FU reads two consecutive
-- chunks per cycle from the rs2 bank (chunk_b on channel 1, chunk_a on
-- channel 0; the dsp_init / dsp_exec branches drive both addresses with the
-- modular arithmetic) and splices them with a 2*SIMD-byte barrel mux.
-- For each output lane j in [0, SIMD-1]:
--     src_idx = j + intra_shift     -- in [0, 2*SIMD-1]
--     out[j] = chunk_b[src_idx]                        if src_idx <  SIMD
--              chunk_a[src_idx - SIMD]                 if src_idx >= SIMD
-- Phase A's within-chunk barrel is the special case intra_shift in [0, SIMD-1]
-- with chunk_a unused (when intra_shift = 0) or partially used (otherwise).
-- ISA semantics (Plate 1995, Kleyko 2022): rho_k(a)[i] = a[(i - k) mod D].
  perm_shift_comb : process(all)
    variable h_var       : integer;
    variable intra_shift : integer;
    variable src_idx     : integer;
  begin
    -- Splice the packed-byte representations of two consecutive SPM chunks.
    -- Phase B uses the FU's pipeline latch:
    --   hdcu_in_perm_operand_b = chunk_b ("high", current chunk for output i),
    --                            sourced from the SPM read issued one cycle
    --                            earlier and registered into the latch
    --   hdcu_in_perm_operand   = chunk_a ("low", next chunk i+1 mod N),
    --                            taken combinationally from the current SPM
    --                            data via MAPPING_IN_UNIT.
    -- intra_shift is latched at dsp_init. The variable is unconstrained
    -- (32-bit integer) so out-of-range latched values cannot trip the
    -- simulator before the FU is properly initialised; the splice mux is
    -- gated by 0 <= intra_shift < SIMD anyway.
    hdc_out_perm_results_wire(f) <= (others => '0');
    for g in 0 to (ACCL_NUM - FU_NUM) loop
      if multithreaded_accl_en = 1 then
        h_var := g;
      else
        h_var := f;
      end if;
      if halt_hdc_lat(h_var) = '0' then
        intra_shift := perm_intra_shift_int(h_var);
        if intra_shift < 0 or intra_shift >= SIMD then
          intra_shift := 0;
        end if;
        for i in 0 to SIMD - 1 loop
          src_idx := i + intra_shift;
          if src_idx < SIMD then
            hdc_out_perm_results_wire(f)((i+1)*8 - 1 downto i*8) <=
              hdcu_in_perm_operand_b(f)((src_idx+1)*8 - 1 downto src_idx*8);
          else
            hdc_out_perm_results_wire(f)((i+1)*8 - 1 downto i*8) <=
              hdcu_in_perm_operand(f)((src_idx-SIMD+1)*8 - 1 downto (src_idx-SIMD)*8);
          end if;
        end loop;
      end if;
    end loop;
  end process perm_shift_comb;

  perm_result_latch : process(clk_i, rst_ni)
    variable h_var : integer;
  begin
    if rst_ni = '0' then
      hdc_out_perm_results(f) <= (others => '0');
    elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h_var := g;
        else
          h_var := f;
        end if;
        if halt_hdc_lat(h_var) = '0' then
          -- Phase B: register the splice when stage_2 is high (the cycle the
          -- current chunk_a + latched chunk_b form a valid output).
          if perm_stage_2_en(h_var) = '1' or recover_state_wires(h_var) = '1' then
            hdc_out_perm_results(f) <= hdc_out_perm_results_wire(f);
          end if;
        end if;
      end loop;
    end if;
  end process perm_result_latch;

  -- [Op-N2 Phase B] Pipeline latch for the previous-cycle SPM word. When
  -- perm_stage_1_en = '1' the FU has just received a fresh SPM grant; the
  -- combinational hdcu_in_perm_operand carries this cycle's "chunk" packed
  -- byte representation. Capturing it into hdcu_in_perm_operand_b makes it
  -- visible as "chunk_b" to perm_shift_comb on the very next cycle, where
  -- the freshly arriving SPM word becomes "chunk_a". This implements the
  -- single-channel sequential streaming the SPI requires (the SPM bank
  -- cannot serve two distinct addresses in the same cycle).
  perm_chunk_b_latch_sync : process(clk_i, rst_ni)
    variable h_var : integer;
  begin
    if rst_ni = '0' then
      hdcu_in_perm_operand_b(f) <= (others => '0');
    elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h_var := g;
        else
          h_var := f;
        end if;
        if halt_hdc_lat(h_var) = '0' then
          if perm_stage_1_en(h_var) = '1' then
            hdcu_in_perm_operand_b(f) <= hdcu_in_perm_operand(f);
          end if;
        end if;
      end loop;
    end if;
  end process perm_chunk_b_latch_sync;


-- ==========================================
--       FHRR ENCODING
-- ==========================================

 -- ============================================================================
 -- [Op-L5] ENCODE multiplier datapath. Two mutually exclusive variants:
 --   * default (ENCODE_WIDE_STREAM = false): single-row streaming. Channel 0
 --     supplies the broadcast scalar, channel 1 supplies the SIMD-packed HV
 --     row. One contribution per cycle per lane. This is the bit-exact
 --     reference for fhrr_encode_test (5527 cycles).
 --   * wide-stream (ENCODE_WIDE_STREAM = true, dormant): two HV rows per
 --     cycle. Scalars are sourced from enc_scalar_cache (pre-loaded during a
 --     setup phase) and channel 0 is repurposed to read the second HV row.
 --     Two multiplier banks (mult_result, mult_result_b) feed a 3-operand
 --     accumulator add for ~2x speedup. The wide-stream branch is wired but
 --     idle: the FSM signals that gate it (enc_stage_*_en, hdc_sc_read_addr
 --     for the dual rows, scalar-cache fill phase) need a coordinated edit
 --     in the dsp_init/dsp_exec FSM before the optimisation can be activated
 --     end-to-end. The present scaffolding is sized so synthesis can
 --     characterise the area/timing impact of the wide datapath.
 -- ============================================================================
 enc_mult_default : if not ENCODE_WIDE_STREAM generate
   -- Encode multiplier: compute scalar[row] * hv[row][lane] (16-bit signed) for every lane.
   -- Gated on hdc_data_gnt_i_lat so sc_data_read is the valid grant data for the current row.
   -- Compute products directly from sc_data_read to bypass hdcu_in_enc_fpp_operands which
   -- suffers from multi-driver resolution when the process lives inside FU_replicated.
   multiplier_unit_comb : process(all)
       variable h : integer;
       variable scalar_byte : signed(7 downto 0);
       variable hv_byte : signed(7 downto 0);
   begin
     mult_result(f) <= (others => '0');
     for g in 0 to (ACCL_NUM - FU_NUM) loop
       if multithreaded_accl_en = 1 then
         h := g;
       else
         h := f;
       end if;
       if halt_hdc_lat(h) = '0' then
         if enc_en(h) = '1' and (enc_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
           scalar_byte := signed(hdc_sc_data_read(h)(0)(7 downto 0));
           for i in 0 to SIMD-1 loop
             hv_byte := signed(hdc_sc_data_read(h)(1)((8-1)+Data_Width*i downto Data_Width*i));
             -- [Op-A4] Direct signed*signed multiplication (8-bit * 8-bit = 16-bit signed).
             -- Replaces the previous to_integer round-trip; this form is the canonical
             -- pattern Vivado DSP-inference looks for when use_dsp="yes" is set on
             -- mult_result. Bit-exact identical to the previous integer-based path.
             mult_result(f)(8*2*(i+1)-1 downto 8*2*i) <=
               std_logic_vector(scalar_byte * hv_byte);
           end loop;
         end if;
       end if;
     end loop;
   end process;

   -- Retain the top 8 bits of the 16-bit signed product (arithmetic >>8) per lane.
   -- [Op-P1] Removed the per-cycle unconditional `<= (others => '0')` default.
   -- Previously the register was rewritten to all-zeros every cycle (then optionally
   -- overwritten by the conditional assignment), forcing toggle activity even when
   -- ENCODE was idle. Now the register holds its value while ENCODE is idle,
   -- letting Vivado infer a clock-enable on this register and quiescing it for
   -- power. Functional behaviour is identical: the next ENCODE operation overwrites
   -- the register before it is read by accumulator_unit (gated on enc_stage_2_en).
   multiplier_unit : process(clk_i, rst_ni)
     variable h : integer;
   begin
     if rst_ni = '0' then
       trunc_mul_results(f) <= (others => '0');
     elsif rising_edge(clk_i) then
       for g in 0 to (ACCL_NUM - FU_NUM) loop
         if multithreaded_accl_en = 1 then
           h := g;
         else
           h := f;
         end if;
         if halt_hdc_lat(h) = '0' then
           if enc_en(h) = '1' and (enc_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
             for i in 0 to SIMD-1 loop
               trunc_mul_results(f)(8*i+7 downto 8*i) <=
                 mult_result(f)(8*2*i+15 downto 8*2*i+8);
             end loop;
           end if;
         end if;
       end loop;
     end if;
   end process;
 end generate enc_mult_default;

 -- ============================================================================
 -- [Op-L5] Wide-stream multiplier datapath (dormant). Two contributions per
 -- cycle: scalar[2k]*hv[2k][i] (bank A, mult_result) and
 -- scalar[2k+1]*hv[2k+1][i] (bank B, mult_result_b). The scalars are looked
 -- up from enc_scalar_cache using enc_feature_pair_idx; both HV rows arrive
 -- in the same cycle on hdc_sc_data_read(h)(0) and hdc_sc_data_read(h)(1).
 -- Latency goal post-activation: F + F*D/(2*SIMD) + 5 cycles.
 -- ============================================================================
 enc_mult_wide : if ENCODE_WIDE_STREAM generate
   multiplier_unit_wide_comb : process(all)
       variable h            : integer;
       variable scalar_a     : signed(7 downto 0);
       variable scalar_b     : signed(7 downto 0);
       variable hv_a_byte    : signed(7 downto 0);
       variable hv_b_byte    : signed(7 downto 0);
       variable pair_idx_int : integer range 0 to ENC_SCALAR_CACHE_DEPTH - 1;
   begin
     mult_result(f)   <= (others => '0');
     mult_result_b(f) <= (others => '0');
     for g in 0 to (ACCL_NUM - FU_NUM) loop
       if multithreaded_accl_en = 1 then
         h := g;
       else
         h := f;
       end if;
       if halt_hdc_lat(h) = '0' then
         if enc_en(h) = '1' and (enc_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
           pair_idx_int := to_integer(unsigned(enc_feature_pair_idx(h)));
           -- Two consecutive scalars per pair: 2k and 2k+1.
           scalar_a := signed(enc_scalar_cache(f)(8*(2*pair_idx_int)   + 7 downto 8*(2*pair_idx_int)));
           scalar_b := signed(enc_scalar_cache(f)(8*(2*pair_idx_int+1) + 7 downto 8*(2*pair_idx_int+1)));
           for i in 0 to SIMD-1 loop
             hv_a_byte := signed(hdc_sc_data_read(h)(0)((8-1)+Data_Width*i downto Data_Width*i));
             hv_b_byte := signed(hdc_sc_data_read(h)(1)((8-1)+Data_Width*i downto Data_Width*i));
             mult_result(f)  (8*2*(i+1)-1 downto 8*2*i) <= std_logic_vector(scalar_a * hv_a_byte);
             mult_result_b(f)(8*2*(i+1)-1 downto 8*2*i) <= std_logic_vector(scalar_b * hv_b_byte);
           end loop;
         end if;
       end if;
     end loop;
   end process;

   multiplier_unit_wide : process(clk_i, rst_ni)
     variable h : integer;
   begin
     if rst_ni = '0' then
       trunc_mul_results(f)   <= (others => '0');
       trunc_mul_results_b(f) <= (others => '0');
     elsif rising_edge(clk_i) then
       for g in 0 to (ACCL_NUM - FU_NUM) loop
         if multithreaded_accl_en = 1 then
           h := g;
         else
           h := f;
         end if;
         if halt_hdc_lat(h) = '0' then
           if enc_en(h) = '1' and (enc_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
             for i in 0 to SIMD-1 loop
               trunc_mul_results(f)(8*i+7 downto 8*i) <=
                 mult_result(f)(8*2*i+15 downto 8*2*i+8);
               trunc_mul_results_b(f)(8*i+7 downto 8*i) <=
                 mult_result_b(f)(8*2*i+15 downto 8*2*i+8);
             end loop;
           end if;
         end if;
       end loop;
     end if;
   end process;
 end generate enc_mult_wide;

	 -- Encode output mapping: expose the accumulator register to the writeback MUX.
	 accumulator_output_multi_lane : if SIMD > 1 generate
	   accumulator_unit_comb : process(all)
	   begin
	     accumulator_wire(f) <= (others => '0');
	     hdc_out_enc_result(f) <= accumulator_reg(f);
	   end process;
	 end generate;

	 accumulator_output_single_lane : if SIMD = 1 generate
	   accumulator_unit_comb : process(all)
	   begin
	     accumulator_wire(f) <= (others => '0');
	     hdc_out_enc_result(f) <= accumulator_reg(f);
	   end process;
	 end generate;
 
 -- ============================================================================
 -- [Op-L5] Accumulator. Default uses the single-bank product (one
 -- contribution per cycle); the wide-stream branch is a 3-operand sum that
 -- folds in trunc_mul_results_b. Both branches share the same reset / clear
 -- semantics (clear at the rising edge of enc_en_wire) so downstream
 -- writeback and accumulator_output mappings are unchanged.
 -- ============================================================================
 enc_acc_default : if not ENCODE_WIDE_STREAM generate
   -- Accumulator Unit Sync Process.
   -- Encode reference: acc[lane] = sum_{row=0..MPSCLFAC-1} (scalar[row] * hv[row][lane]) >> 8, modulo 256.
   -- enc_stage_1_en is aligned with the SPM row data; trunc_mul_results is registered from
   -- that data and accumulated one cycle later on enc_stage_2_en.
   accumulator_unit : process(clk_i, rst_ni)
     variable h : integer;
   begin
     if rst_ni = '0' then
       accumulator_reg(f) <= (others => '0');
     elsif rising_edge(clk_i) then
       for g in 0 to (ACCL_NUM - FU_NUM) loop
         if multithreaded_accl_en = 1 then
           h := g;
         else
           h := f;
         end if;
         if halt_hdc_lat(h) = '0' then
           if enc_en_wire(h) = '1' and enc_en(h) = '0' then
             accumulator_reg(f) <= (others => '0');
           elsif enc_en(h) = '1' and (enc_stage_2_en(h) = '1' or recover_state_wires(h) = '1') then
             for i in 0 to SIMD-1 loop
               accumulator_reg(f)(8*i+7 downto 8*i) <=
                 std_logic_vector(unsigned(accumulator_reg(f)(8*i+7 downto 8*i)) +
                                  unsigned(trunc_mul_results(f)(8*i+7 downto 8*i)));
             end loop;
           end if;
         end if;
       end loop;
     end if;
   end process;
 end generate enc_acc_default;

 -- [Op-L5] 3-operand accumulator add for the wide-stream datapath. Two
 -- contributions land per cycle (banks A and B) and the running sum picks up
 -- both in a single update. Modulo-256 wrap is preserved: a (+a) (+b) chain
 -- on 8-bit unsigned operands matches the bit-exact reference modulo 256
 -- regardless of associativity.
 enc_acc_wide : if ENCODE_WIDE_STREAM generate
   accumulator_unit_wide : process(clk_i, rst_ni)
     variable h : integer;
   begin
     if rst_ni = '0' then
       accumulator_reg(f) <= (others => '0');
     elsif rising_edge(clk_i) then
       for g in 0 to (ACCL_NUM - FU_NUM) loop
         if multithreaded_accl_en = 1 then
           h := g;
         else
           h := f;
         end if;
         if halt_hdc_lat(h) = '0' then
           if enc_en_wire(h) = '1' and enc_en(h) = '0' then
             accumulator_reg(f) <= (others => '0');
           elsif enc_en(h) = '1' and (enc_stage_2_en(h) = '1' or recover_state_wires(h) = '1') then
             for i in 0 to SIMD-1 loop
               accumulator_reg(f)(8*i+7 downto 8*i) <=
                 std_logic_vector(unsigned(accumulator_reg(f)(8*i+7 downto 8*i)) +
                                  unsigned(trunc_mul_results(f)(8*i+7 downto 8*i)) +
                                  unsigned(trunc_mul_results_b(f)(8*i+7 downto 8*i)));
             end loop;
           end if;
         end if;
       end loop;
     end if;
   end process;

   -- [Op-L5] Scalar-cache fill register. During the setup phase the FSM must
   -- raise enc_en alongside enc_stage_1_en while parking the feature-pair
   -- counter, so that one byte per feature is written into enc_scalar_cache
   -- from hdc_sc_data_read(h)(0)(7 downto 0). When the FSM transitions into
   -- the main loop the cache becomes read-only and is consumed by
   -- multiplier_unit_wide_comb above. The setup-phase write is not
   -- coordinated by the present scaffolding (the FSM rewrite is future work);
   -- the register is written only on a dedicated stage_setup_en signal which
   -- is driven low here, so the cache holds its reset value when the wide
   -- stream is not actually exercised by the FSM.
   enc_scalar_cache_unit : process(clk_i, rst_ni)
     variable h : integer;
   begin
     if rst_ni = '0' then
       enc_scalar_cache(f)         <= (others => '0');
       enc_feature_pair_idx(f)     <= (others => '0');
     elsif rising_edge(clk_i) then
       -- Placeholder: the cache and the pair counter are clock-enabled. The
       -- actual write/increment events will be driven by a dedicated set of
       -- FSM enables (enc_stage_setup_en, enc_stage_pair_advance_en) when
       -- the dsp_init/dsp_exec rewrite lands. Until then the registers hold
       -- their reset value and the wide-stream branch operates on zeros, so
       -- this scaffolding is purely structural for synthesis characterisation.
       null;
     end if;
   end process;
 end generate enc_acc_wide;

 
  ---------------------------------------------------------------------------------------------------------------
  -- ███████╗██╗███╗   ███╗██╗██╗      █████╗ ██████╗ ██╗████████╗██╗   ██╗    ██╗   ██╗███╗   ██╗██╗████████╗ --
  -- ██╔════╝██║████╗ ████║██║██║     ██╔══██╗██╔══██╗██║╚══██╔══╝╚██╗ ██╔╝    ██║   ██║████╗  ██║██║╚══██╔══╝ --
  -- ███████╗██║██╔████╔██║██║██║     ███████║██████╔╝██║   ██║    ╚████╔╝     ██║   ██║██╔██╗ ██║██║   ██║    --
  -- ╚════██║██║██║╚██╔╝██║██║██║     ██╔══██║██╔══██╗██║   ██║     ╚██╔╝      ██║   ██║██║╚██╗██║██║   ██║    --
  -- ███████║██║██║ ╚═╝ ██║██║███████╗██║  ██║██║  ██║██║   ██║      ██║       ╚██████╔╝██║ ╚████║██║   ██║    --
  -- ╚══════╝╚═╝╚═╝     ╚═╝╚═╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝   ╚═╝      ╚═╝        ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --
  ---------------------------------------------------------------------------------------------------------------
 
 fsm_HDC_cosine_delta : process(all)
 variable h : integer;
 begin
  -- [Fix-MDRV] Scope default to (f) slice only to avoid multiple-driver issues.
  delta(f) <= (others => '0');
    for g in 0 to (ACCL_NUM - FU_NUM) loop
       if multithreaded_accl_en = 1 then
         h := g;  -- set the spm rd/wr ports equal to the "for-loop"
       elsif multithreaded_accl_en = 0 then
         h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
       end if;
       if halt_hdc_lat(h) = '0' then
         if sim_en(h) = '1' and (sim_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
         
           for i in 0 to SIMD-1 loop
                  
               delta(f)((8-1)+8*(i) downto 8*(i)) 
               <= std_logic_vector(unsigned(hdcu_in_sim_operands(f)(0)(7+8*(i)  downto 8*(i))) - unsigned(hdcu_in_sim_operands(f)(1)(7+8*(i)  downto 8*(i))));
               end loop;
               
         end if;
       end if;
     end loop;
  
 end process;
 
 
 -- Similarity reference: out = (sum_i cos_lut(|phase_a[i] - phase_b[i]|)) >> log2(N)
 --
 -- Implementation notes:
 --   * Data for grant k appears on sc_data_read when hdc_data_gnt_i_lat='1' in cycle k+1.
 --   * The FSM also issues one more grant than strictly needed (HVSIZE_READ_init adds a
 --     log2(SIMD)*SIMD_RD_BYTES / +4 pad to make room for the old pipeline), so the trailing
 --     grants return out-of-range data. The "valid window" is therefore
 --       HVSIZE_READ > HVSIZE_READ_init - HVSIZE   (== log2(SIMD)*SIMD_RD_BYTES for SIMD>1,
 --                                                  ==  4                        for SIMD=1)
 --   * The last valid sample is when HVSIZE_READ reaches that threshold plus one chunk, i.e.
 --     HVSIZE_READ == 2*threshold (SIMD>1) or threshold+4 (SIMD=1) won't work cleanly -- easier
 --     to check HVSIZE_READ == 2*SIMD_RD_BYTES_wire (SIMD>1) or HVSIZE_READ == 8 (SIMD=1) being
 --     the cycle whose grant carries the last SIMD-lane chunk.
 -- The per-grant partial cosine sum is computed combinationally from the already-present
 -- comb signal `delta` (which is driven by hdcu_in_sim_operands, i.e. the current
 -- sc_data_read). The result feeds the sync accumulator.

 -- Compute the per-grant cosine sum directly from hdcu_in_sim_operands to avoid
 -- relying on the `delta` signal which is gated on sim_stage_1_en and therefore
 -- produces 0 on the first grant cycle (before stage_1_en has risen).
 sim_partial_sum_comb : process(all)
   variable h : integer;
   variable sum : signed(Data_Width - 1 downto 0);
   variable lane_delta : signed(7 downto 0);
   variable lane_idx : integer range 0 to 255;
 begin
   sim_cos_sum_wire(f) <= (others => '0');
   for g in 0 to (ACCL_NUM - FU_NUM) loop
     if multithreaded_accl_en = 1 then
       h := g;
     else
       h := f;
     end if;
     if halt_hdc_lat(h) = '0' and sim_en(h) = '1' and
        (hdc_data_gnt_i_lat(h) = '1' or recover_state_wires(h) = '1') then
       sum := (others => '0');
       for i in 0 to SIMD - 1 loop
         lane_delta := signed(hdc_sc_data_read(h)(0)((8-1)+Data_Width*i downto Data_Width*i)) -
                       signed(hdc_sc_data_read(h)(1)((8-1)+Data_Width*i downto Data_Width*i));
         if lane_delta(7) = '1' then
           lane_idx := to_integer(unsigned(std_logic_vector(-lane_delta)));
         else
           lane_idx := to_integer(unsigned(std_logic_vector(lane_delta)));
         end if;
         sum := sum + signed(cos_lut_lookup(lane_idx));
       end loop;
       sim_cos_sum_wire(f) <= std_logic_vector(sum);
     end if;
   end loop;
 end process;

 -- [Op-L1] SIMILARITY tree-adder pipeline split.
 --
 --   When SIM_PIPE_EXTRA = 1 (i.e. NUM_LEVELS >= 2, i.e. SIMD >= 4) we register
 --   the per-grant tree-adder output (sim_cos_sum_wire) one cycle before the
 --   cosine accumulator consumes it. This breaks the long combinational chain
 --     delta_sub -> cos_lut -> log2(SIMD)-level tree adder -> accumulator add
 --   into two shorter paths:
 --     stage A: delta_sub -> cos_lut -> tree adder        -> sim_cos_sum_reg
 --     stage B: sim_cos_sum_reg                           -> accumulator add
 --   recovering fmax at the cost of +1 cycle similarity latency.
 --
 --   For SIM_PIPE_EXTRA = 0 (SIMD <= 2) sim_cos_sum_reg/_valid are still
 --   driven (one cycle behind sim_cos_sum_wire / sim_stage_1_en) but the
 --   accumulator branch below ignores them and consumes sim_cos_sum_wire
 --   directly so the legacy latency is preserved bit-for-bit.
 sim_cos_sum_register_unit : process(clk_i, rst_ni)
   variable h : integer;
 begin
   if rst_ni = '0' then
     sim_cos_sum_reg(f)   <= (others => '0');
     sim_cos_sum_valid(f) <= '0';
   elsif rising_edge(clk_i) then
     -- Default: clear valid each cycle so a stalled grant cannot replay.
     sim_cos_sum_valid(f) <= '0';
     for g in 0 to (ACCL_NUM - FU_NUM) loop
       if multithreaded_accl_en = 1 then
         h := g;
       else
         h := f;
       end if;
       if halt_hdc_lat(h) = '0' then
         if sim_en_wire(h) = '1' and sim_en(h) = '0' then
           sim_cos_sum_reg(f)   <= (others => '0');
           sim_cos_sum_valid(f) <= '0';
         elsif sim_en(h) = '1' and (sim_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
           sim_cos_sum_reg(f)   <= sim_cos_sum_wire(f);
           sim_cos_sum_valid(f) <= '1';
         end if;
       end if;
     end loop;
   end if;
 end process;

 cosine_accumulator_unit : process(clk_i, rst_ni)
   variable h : integer;
   variable sum_next : signed(Data_Width - 1 downto 0);
   variable hv_log2 : integer;
   variable valid_chunks : integer;
   variable chunk_count_int : integer;
   variable acc_input  : signed(Data_Width - 1 downto 0);
   variable do_acc     : boolean;
 begin
   if rst_ni = '0' then
     cosine_accumulator_reg(f) <= (others => '0');
     cosine_accumulator_avg(f) <= (others => '0');
     sim_chunk_count(f) <= (others => '0');
   elsif rising_edge(clk_i) then
     for g in 0 to (ACCL_NUM - FU_NUM) loop
       if multithreaded_accl_en = 1 then
         h := g;
       else
         h := f;
       end if;

       if halt_hdc_lat(h) = '0' then
         if sim_en_wire(h) = '1' and sim_en(h) = '0' then
           cosine_accumulator_reg(f) <= (others => '0');
           cosine_accumulator_avg(f) <= (others => '0');
           sim_chunk_count(f) <= (others => '0');
         else
           -- [Op-L1] Select the accumulator input depending on the pipeline depth.
           -- SIM_PIPE_EXTRA=1 -> consume the registered sum (one cycle delayed)
           --                    gated by the synchronous valid token.
           -- SIM_PIPE_EXTRA=0 -> consume the combinational wire directly,
           --                    gated as before by sim_stage_1_en.
           if SIM_PIPE_EXTRA = 1 then
             acc_input := signed(sim_cos_sum_reg(f));
             do_acc    := (sim_en(h) = '1') and
                          ((sim_cos_sum_valid(f) = '1') or (recover_state_wires(h) = '1'));
           else
             acc_input := signed(sim_cos_sum_wire(f));
             do_acc    := (sim_en(h) = '1') and
                          ((sim_stage_1_en(h) = '1') or (recover_state_wires(h) = '1'));
           end if;

           if do_acc then
             -- [SIMILARITY timing fix] Use the pre-computed registered geometry
             -- (sim_valid_chunks, sim_hv_log2) instead of recomputing them every
             -- cycle from HV_ELEMENT. Eliminates the long comb chain
             -- HV_ELEMENT -> log2/divide -> cosine_accumulator_avg.
             if HV_ELEMENT(h) /= (Data_Width - 1 downto 0 => 'U') then
               valid_chunks := to_integer(unsigned(sim_valid_chunks(h)));
               chunk_count_int := to_integer(unsigned(sim_chunk_count(f)));
             else
               valid_chunks := 0;
               chunk_count_int := 0;
             end if;

             if valid_chunks > 0 and chunk_count_int < valid_chunks then
               sum_next := signed(cosine_accumulator_reg(f)) + acc_input;
               cosine_accumulator_reg(f) <= std_logic_vector(sum_next);
               if chunk_count_int = valid_chunks - 1 then
                 hv_log2 := to_integer(unsigned(sim_hv_log2(h)));
                 cosine_accumulator_avg(f) <= std_logic_vector(shift_right(sum_next, hv_log2));
               end if;
               sim_chunk_count(f) <= std_logic_vector(unsigned(sim_chunk_count(f)) + 1);
             end if;
           end if;
         end if;
       end if;
     end loop;
   end if;
 end process;

 -- Comb process retained only to drive cosine_accumulator_wire for legacy observers.
 cosine_accumulator_unit_comb : process(all)
 begin
   cosine_accumulator_wire(f) <= cosine_accumulator_reg(f);
 end process;



  
  --------------------------------------------------------------------------------------------------
  --  ██████╗██╗     ██╗██████╗ ██████╗ ██╗███╗   ██╗ ██████╗     ██╗   ██╗███╗   ██╗██╗████████╗ --
  -- ██╔════╝██║     ██║██╔══██╗██╔══██╗██║████╗  ██║██╔════╝     ██║   ██║████╗  ██║██║╚══██╔══╝ --
  -- ██║     ██║     ██║██████╔╝██████╔╝██║██╔██╗ ██║██║  ███╗    ██║   ██║██╔██╗ ██║██║   ██║    -- 
  -- ██║     ██║     ██║██╔═══╝ ██╔═══╝ ██║██║╚██╗██║██║   ██║    ██║   ██║██║╚██╗██║██║   ██║    -- 
  -- ╚██████╗███████╗██║██║     ██║     ██║██║ ╚████║╚██████╔╝    ╚██████╔╝██║ ╚████║██║   ██║    -- 
  --  ╚═════╝╚══════╝╚═╝╚═╝     ╚═╝     ╚═╝╚═╝  ╚═══╝ ╚═════╝      ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --  
  --------------------------------------------------------------------------------------------------





-- fsm_ratio_img_real: process(all)
--   variable h : integer;
-- begin
--     tangent <= (others => (others => '0'));  -- Initialize tangent to zero
    
--     for g in 0 to (ACCL_NUM - FU_NUM) loop
--         if multithreaded_accl_en = 1 then
--             h := g;  -- Use loop index
--         elsif multithreaded_accl_en = 0 then
--             h := f;  -- Use "for-generate" index
--         end if;
        
--         if halt_hdc_lat(h) = '0' then
--             if clip_en(h) = '1' and (clip_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
            
--                 for i in 0 to SIMD-1 loop
                
--                     -- Check for division by zero
--                     if unsigned(hdcu_in_clip_operands(f)(1)((Data_Width-1)+Data_Width*(i) downto Data_Width*(i))) /= 0 then
--                         -- Compute tangent = imag_sum / real_sum
--                         tangent(f)((Data_Width-1)+Data_Width*(i) downto Data_Width*(i)) <= 
--                             std_logic_vector(resize(shift_left(unsigned(hdcu_in_clip_operands(f)(0)((Data_Width-1)+Data_Width*(i) downto Data_Width*(i))), PRECISION_BIT_WIDTH) / 
--                             unsigned(hdcu_in_clip_operands(f)(1)((Data_Width-1)+Data_Width*(i) downto Data_Width*(i))), Data_Width));
--                     else
--                         -- Avoid division by zero, set tangent to zero
--                         tangent(f)((Data_Width-1)+Data_Width*(i) downto Data_Width*(i)) <= (others => '0');
--                     end if;
                
--                 end loop;
            
--             end if;
--         end if;
--     end loop;

-- end process;

-- [Op-S2] Divider backend dispatch. divider_kind selects the implementation;
-- currently only the radix-2 baseline is wired. Future Op-A5a/A5b can add
-- alternative backends as additional generate branches without touching the
-- surrounding CLIP FSM (the dividend/divisor/result/done/enable signals are
-- already the FU's contract surface).
divider_radix2_gen : if divider_kind = "radix2" generate
    divider_block : for i in 0 to SIMD-1 generate
        divider_i: entity work.divider16
            port map (
                dividend_i            => dividend(f)(i),
                divisor_i             => divisor(f)(i),
                reset                 => rst_ni,
                clk                   => clk_i,
                div_enable_i          => div_enable(f)(i)(1),
                division_finished_out => div_done(f)(i)(1),
                result                => div_result(f)(i)
            );
        -- Tie off the CORDIC backend's per-lane outputs when this branch
        -- is active; keeps simulators from reporting unresolved 'U' on
        -- diagnostic taps.
        cordic_angle(f)(i) <= (others => '0');
        cordic_valid(f)(i) <= (others => '0');
    end generate;
end generate;
-- divider_radix4_shared_gen : if divider_kind = "radix4_shared" generate ... end generate;  -- Op-A5a future
-- [Op-A5b] CORDIC vectoring backend. Replaces the divider16 + atan_lut
-- chain with a pipelined N-stage CORDIC. div_done(f)(i)(1) is reused as
-- the "result valid" flag so the surrounding CLIP FSM (clip_stage_2_en
-- gating, div_done_lat tracker) does not need to know which backend is
-- active. div_result is unused on this path; the angle is consumed
-- directly from cordic_angle in the CLIP datapath process below.
divider_cordic_gen : if divider_kind = "cordic" generate
    cordic_block : for i in 0 to SIMD-1 generate
        cordic_i: entity work.cordic_vectoring
            generic map (
                INPUT_WIDTH    => Data_Width,
                NUM_ITERATIONS => 16
            )
            port map (
                clk       => clk_i,
                rst_n     => rst_ni,
                enable_i  => div_enable(f)(i)(1),
                -- In the radix-2 path, dividend = imag<<16 (Q1.16 imag)
                -- and divisor = real (Q1.0 then sign-extended). For
                -- CORDIC vectoring atan2(y, x) the inputs are direct
                -- Q1.16 Cartesian components; we feed the original 32-bit
                -- imag/real fields straight through without <<16 pre-scaling.
                x_in      => hdcu_in_clip_operands(f)(1)
                                    ((Data_Width-1)+Data_Width*i downto Data_Width*i),
                y_in      => hdcu_in_clip_operands(f)(0)
                                    ((Data_Width-1)+Data_Width*i downto Data_Width*i),
                angle_out => cordic_angle(f)(i),
                valid_out => cordic_valid(f)(i)(0)
            );
        -- Bridge the CORDIC's "valid_out" into the existing div_done
        -- contract so clip_stage_2_en / div_done_lat downstream logic
        -- is backend-agnostic.
        div_done(f)(i)(1)  <= cordic_valid(f)(i)(0);
        -- div_result is not used by the CORDIC path; tie-off keeps
        -- bit-level diagnostic taps stable.
        div_result(f)(i)   <= (others => '0');
    end generate;
end generate;


-- Single instance of divider
-- divider_i: entity work.divider16
--     port map (
--         dividend_i            => dividend_scalar,
--         divisor_i             => divisor_scalar,
--         reset                 => rst_ni,
--         clk                   => clk_i,
--         div_enable_i          => div_enable,
--         division_finished_out => div_done,
--         result                => div_result
--     );

fsm_ratio_img_real : process(clk_i, rst_ni)
    variable h : integer;
    variable tang_abs   : std_logic_vector(Data_Width-1 downto 0);
    variable idx        : integer range 0 to 255;
    variable tang_val   : std_logic_vector(Data_Width-1 downto 0);
begin
    if rst_ni = '0' then
        dividend(f) <= (others => (others => '0'));
        divisor(f) <=  (others => (others => '0'));
        
        -- dividend_scalar <= (others => '0');
        -- divisor_scalar <= (others => '0');
        
        div_enable(f) <= (others => (others => '0'));
        div_negate(f) <= (others => (others => '0'));
        tangent <= (others => (others => '0'));
        hdcu_out_clip_results(f) <= (others => '0');
    elsif rising_edge(clk_i) then
        --div_enable <= (others => '0');  -- Default: clear enables

        for g in 0 to (ACCL_NUM - FU_NUM) loop
            if multithreaded_accl_en = 1 then
                h := g;
            else
                h := f;
            end if;

            if halt_hdc_lat(h) = '0' then
                if clip_en(h) = '1' and (clip_stage_1_en(h) = '1' or recover_state_wires(h) = '1')   then
                    for i in 0 to SIMD-1 loop
                        -- Signed division: pass |imag|<<16 and |real| to unsigned divider, track sign for negation.
                        -- Load operands one cycle BEFORE asserting div_enable (so the divider sees stable values).
                        -- Load when HVSIZE_READ_lat + 2*SIMD_RD_BYTES = HVSIZE_READ_init_clip (one cycle before div_enable).
                        if (unsigned(HVSIZE_READ_lat(h)) + 2*SIMD_RD_BYTES_wire(h) = unsigned(HVSIZE_READ_init_clip(h))) or
                           (unsigned(HVSIZE_READ_lat(h)) + SIMD_RD_BYTES_wire(h) = unsigned(HVSIZE_READ_init_clip(h))) then
                            if hdcu_in_clip_operands(f)(0)((Data_Width-1)+Data_Width*i) = '1' then
                                dividend(f)(i) <= std_logic_vector(
                                    resize(unsigned(std_logic_vector(
                                        -signed(hdcu_in_clip_operands(f)(0)((Data_Width-1)+Data_Width*i downto Data_Width*i))
                                    )), Data_Width + PRECISION_BIT_WIDTH) sll PRECISION_BIT_WIDTH
                                );
                            else
                                dividend(f)(i) <= std_logic_vector(
                                    resize(unsigned(hdcu_in_clip_operands(f)(0)((Data_Width-1)+Data_Width*i downto Data_Width*i)),
                                        Data_Width + PRECISION_BIT_WIDTH) sll PRECISION_BIT_WIDTH
                                );
                            end if;
                            if hdcu_in_clip_operands(f)(1)((Data_Width-1)+Data_Width*i) = '1' then
                                divisor(f)(i) <= std_logic_vector(resize(
                                    unsigned(std_logic_vector(
                                        -signed(hdcu_in_clip_operands(f)(1)((Data_Width-1)+Data_Width*i downto Data_Width*i))
                                    )),
                                    Data_Width + PRECISION_BIT_WIDTH));
                            else
                                divisor(f)(i) <= std_logic_vector(resize(
                                    unsigned(hdcu_in_clip_operands(f)(1)((Data_Width-1)+Data_Width*i downto Data_Width*i)),
                                    Data_Width + PRECISION_BIT_WIDTH));
                            end if;
                        end if;
                        -- Trigger the divider on the first read grant of each element.
                        -- Operands were loaded the previous cycle so they are stable at this point.
                        if signed(hdcu_in_clip_operands(f)(1)((Data_Width-1)+Data_Width*i downto Data_Width*i)) /= 0 and
                          (unsigned(HVSIZE_READ_lat(h)) + SIMD_RD_BYTES_wire(h) = unsigned(HVSIZE_READ_init_clip(h))) then
                            div_enable(f)(i)(1) <= '1';
                            -- Latch sign: negate result if imag and real have different signs.
                            -- Must be latched HERE (at div_enable time) not overwritten on every cycle.
                            div_negate(f)(i)(0) <= hdcu_in_clip_operands(f)(0)((Data_Width-1)+Data_Width*i) xor
                                                   hdcu_in_clip_operands(f)(1)((Data_Width-1)+Data_Width*i);
                        else
                            div_enable(f)(i)(1) <= '0';
                        end if;
                    end loop;
                end if;

                -- Compute atan directly when div_done_lat fires.
                -- Radix-2 backend: div_done_lat fires one cycle after the 48th
                -- shift; div_result holds the |imag|/|real| quotient and the
                -- atan_lut converts it (with the latched sign) into Q1.16 rad.
                -- CORDIC backend (Op-A5b): div_done(f)(i)(1) is bridged from
                -- cordic_valid; cordic_angle already holds the Q1.16 atan2
                -- result over the full [-pi, +pi] range, so no LUT lookup or
                -- sign re-application is needed.
                if clip_en(h) = '1' then
                    for i in 0 to SIMD-1 loop
                        if div_done_lat(f)(i)(1) = '1' then
                            if divider_kind = "cordic" then
                                -- [Op-A5b] CORDIC path: angle is already a
                                -- Q1.16 signed radian value covering the full
                                -- atan2 range. No further conversion needed
                                -- to match the radix-2 backend's Q1.16-rad
                                -- output contract. Residual error vs. the
                                -- radix-2 + atan_lut chain is bounded by the
                                -- CORDIC truncation (<= 2^-N rad ~ 1.5e-5 for
                                -- N=16), which translates to <=1 ULP in the
                                -- Q0.8 phase that downstream encode emits.
                                hdcu_out_clip_results(f)((Data_Width-1)+Data_Width*i downto Data_Width*i)
                                    <= cordic_angle(f)(i);
                            else
                                -- Radix-2 + atan_lut path (default).
                                -- Compute signed tangent from quotient and negate flag
                                if div_negate(f)(i)(0) = '1' then
                                    tang_val := std_logic_vector(-signed(div_result(f)(i)(Data_Width-1 downto 0)));
                                else
                                    tang_val := div_result(f)(i)(Data_Width-1 downto 0);
                                end if;
                                -- Match the software reference:
                                --   magnitude = abs(tangent)
                                --   lut_index = uint8_t(magnitude >> 13) when magnitude < 40<<16
                                --   clipped   = +/- atan_lut[lut_index]
                                if tang_val(Data_Width-1) = '1' then
                                    tang_abs := std_logic_vector(-signed(tang_val));
                                else
                                    tang_abs := tang_val;
                                end if;
                                idx := to_integer(unsigned(tang_abs(PRECISION_BIT_WIDTH+4 downto PRECISION_BIT_WIDTH-3)));

                                -- LUT lookup with saturation on magnitude, then apply sign.
                                -- Op-A3: atan_lut is now unsigned(16 downto 0); zero-extend to
                                -- Data_Width before optional negation (entries are non-negative
                                -- magnitudes, so resize(unsigned, Data_Width) is a zero-pad).
                                if tang_val(Data_Width-1) = '1' then
                                    if unsigned(tang_abs) < to_unsigned(2621440, Data_Width) then
                                        hdcu_out_clip_results(f)((Data_Width-1)+Data_Width*i downto Data_Width*i)
                                            <= std_logic_vector(-signed(resize(atan_lut(idx), Data_Width)));
                                    else
                                        hdcu_out_clip_results(f)((Data_Width-1)+Data_Width*i downto Data_Width*i)
                                            <= std_logic_vector(-signed(resize(atan_lut(255), Data_Width)));
                                    end if;
                                else
                                    if unsigned(tang_abs) < to_unsigned(2621440, Data_Width) then
                                        hdcu_out_clip_results(f)((Data_Width-1)+Data_Width*i downto Data_Width*i)
                                            <= std_logic_vector(resize(atan_lut(idx), Data_Width));
                                    else
                                        hdcu_out_clip_results(f)((Data_Width-1)+Data_Width*i downto Data_Width*i)
                                            <= std_logic_vector(resize(atan_lut(255), Data_Width));
                                    end if;
                                end if;
                            end if;
                        end if;
                    end loop;
                end if;

                --     -- Extract operands
                --     --dividend_scalar <= hdcu_in_clip_operands(f)(0)((Data_Width-1) downto 0);
                --     dividend_scalar(31 + PRECISION_BIT_WIDTH downto PRECISION_BIT_WIDTH) <= hdcu_in_clip_operands(f)(0)(Data_Width-1 downto 0);
                --     divisor_scalar (31 downto 0 )  <= hdcu_in_clip_operands(f)(1)((Data_Width-1) downto 0);

                --  if unsigned(divisor_scalar) /= 0 and (
                --             (unsigned(HVSIZE_READ_lat(h)) + 4 = unsigned(HVSIZE_READ_init_clip(h))) or 
                --             (
                --                 (unsigned(HVSIZE_READ_lat(h)) + 8 + PRECISION_BIT_WIDTH = unsigned(HVSIZE_READ_init(h))) and 
                --                 (unsigned(HVSIZE_READ_init_clip(h)) /= unsigned(HVSIZE_READ_init(h))) and (unsigned(HVSIZE_READ(h))-4 /= 0)
                --             )
                --         ) then

                      
                --         div_enable <= '1';  -- Trigger divider
                --     --elsif unsigned(HVSIZE_READ(h)) = (unsigned(HVSIZE_READ_init(h)) - SIMD_RD_BYTES_wire(h)*33) then
                --      else 
                --       div_enable <= '0';
                --     end if;

                --        --if div_done = '1' then
                --         if (unsigned(HVSIZE_READ_lat(h)) + 12 + PRECISION_BIT_WIDTH = unsigned(HVSIZE_READ_init(h))) and 
                --                 (unsigned(HVSIZE_READ_init_clip(h)) /= unsigned(HVSIZE_READ_init(h))) then 
                --           tangent(f)((Data_Width-1) downto 0) <= div_result(Data_Width-1  downto 0 );  -- store quotient
                --       end if;
                end if;
        end loop;
    end if;
end process;



fsm_atan_comb : process(all)
variable h : integer;
begin
  tang_map <= (others => (others => '0'));
  
    for g in 0 to (ACCL_NUM - FU_NUM) loop
      if multithreaded_accl_en = 1 then
        h := g;  -- set the spm rd/wr ports equal to the "for-loop"
      elsif multithreaded_accl_en = 0 then
        h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
      end if;
      if halt_hdc_lat(h) = '0' then
        if clip_en(h) = '1' and (clip_stage_2_en(h) = '1' or recover_state_wires(h) = '1') then
         
          for i in 0 to SIMD-1 loop
           if (tangent(f)((Data_Width-1) + Data_Width * i ) = '1') then 
             tang_map(f)((Data_Width-1) + Data_Width * i downto (Data_Width - (PRECISION_BIT_WIDTH + 5)) + Data_Width * i)  <= std_logic_vector(-signed(tangent(f)((Data_Width-1)+Data_Width*(i)-(Data_Width - 5-PRECISION_BIT_WIDTH) downto Data_Width*(i)) ));
           else 
           
             tang_map(f)((Data_Width-1) + Data_Width * i downto (Data_Width - (PRECISION_BIT_WIDTH + 5)) + Data_Width * i) 
                 <= tangent(f)((Data_Width-1)+Data_Width*(i)-(Data_Width - 5-PRECISION_BIT_WIDTH) downto Data_Width*(i)) ;
         
         
           end if;
         
         end loop;
         
        end if;
      end if;
    end loop;
  
end process;


-- fsm_atan_sync removed: atan computation is now done directly in fsm_ratio_img_real
-- at div_done_lat time to avoid multi-driver conflicts on hdcu_out_clip_results.
 --------------------------------------------------------------------------------------------------------------------------
  
  

end generate FU_replicated;
end HDC;
--------------------------------- END of HDC architecture ------------------------------------
