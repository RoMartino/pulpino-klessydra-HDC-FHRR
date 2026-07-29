----------------------------------------------------------------------------------------------------------
--  HDC Unit(s) --                                                                                      --
--  Author(s): Abdallah Cheikh abdallah.cheikh@uniroma1.it (abdallah93.as@gmail.com)                    --
--                                                                                                      --
--  Date Modified: 02-04-2020                                                                           --
----------------------------------------------------------------------------------------------------------
--  The HDC unit executes on vectors fetched from local-low-latency-wide-bus scratchpad memories.       --
--  The HDC has five functional units, adder/subtractor, multiplier, right arith/logic shifter,         --
--  accumulator, and ReLu each of which supports three integer data types (8-bit, 16-bit and 32-bits)   --
--  The data parallelism of the HDC is defined by the SIMD parameter in the PKG file. Increasing the    --
--  data level parallelism increasess the number of banks per SPM as well, as the number of functional  --
--  units. To increase the instruction level parallelism, the replicated_accl_en parameter must be      --
--  set. Setting it will provide a dedicated hardware accelerator for each hart,                        --
--  Custom CSRs are implemented for the accelerator unit                                                --
----------------------------------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;

use work.riscv_klessydra.all;

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
    out_data : out unsigned(((NUM_INPUTS/2)*(DATA_WIDTH+1))-1 downto 0)
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
entity FHRR_Unit is

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
    hdc_taken_branch_FHRR      : out std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_except_condition       : out std_logic_vector(ACCL_NUM-1 downto 0);
  -- ID_Stage Signals
    decoded_instruction_FHRR   : in  std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0);
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
    hdcu_enc_perf_counter      : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
  
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

architecture FHRR of FHRR_Unit is

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
  signal decoded_instruction_FHRR_lat    : array_2d(accl_range)(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0);
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
  signal HVSIZE_READ_init_clip           : array_2d(accl_range)(Addr_Width downto 0); -- condition
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
 
 
  signal bundle_en                        : std_logic_vector(accl_range); -- enables the use of the adders
  signal bundle_en_wire                   : std_logic_vector(accl_range); -- enables the use of the adders
  signal bundle_en_pending                : std_logic_vector(accl_range); -- signal to preserve the request to access the adder "multhithreaded mode" only
  signal bundle_en_pending_wire           : std_logic_vector(accl_range); -- signal to preserve the request to access the adder "multhithreaded mode" only
  signal busy_bundle                      : std_logic; -- busy signal active only when the FU is shared and currently in use 
  signal busy_bundle_wire                 : std_logic; -- busy signal active only when the FU is shared and currently in use 

  signal bundle_stage_1_en               : std_logic_vector(accl_range);
  signal bundle_stage_2_en               : std_logic_vector(accl_range);
  signal bundle_stage_3_en               : std_logic_vector(accl_range);

  signal hdcu_in_bundle_operand            : array_2d(fu_range)((8*SIMD_Width)/Data_Width-1 downto 0);
  signal hdcu_in_bundled                   : array_2d(fu_range)(SIMD_Width - 1 downto 0);
  signal hdcu_in_bundle_operands_masked    : array_3d(fu_range)(1 downto 0)((INTEGER_BIT_WIDTH + FRACTIONAL_BIT_WIDTH)*SIMD - 1 downto 0);

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
  signal bind_en                          : std_logic_vector(accl_range);  -- enables the use of the multipliers
  signal bind_en_wire                     : std_logic_vector(accl_range);  -- enables the use of the multipliers
  signal bind_en_pending                  : std_logic_vector(accl_range);  -- signal to preserve the request to access the multiplier "multhithreaded mode" only
  signal bind_en_pending_wire             : std_logic_vector(accl_range);  -- signal to preserve the request to access the multiplier "multhithreaded mode" only

  signal bind_stage_1_en                  : std_logic_vector(accl_range);
  signal bind_stage_2_en                  : std_logic_vector(accl_range);

  signal hdcu_in_bind_operands            : array_3d(fu_range)(1 downto 0)((8*SIMD_Width)/Data_Width-1 downto 0);
  signal hdc_out_bind_results             : array_2d(fu_range)((8*SIMD_Width)/Data_Width-1 downto 0);
  signal hdcu_bind_perf_counter_int       : array_2d(accl_range)(31 downto 0);

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


  signal hdcu_in_enc_fpp_operands             : array_3d(fu_range)(1 downto 0)(SIMD_Width - (Data_Width-8)*SIMD_Width/Data_Width -1 downto 0);
  signal mult_result                          : array_2d(fu_range)(2*SIMD_Width - (Data_Width-8)*2*SIMD_Width/Data_Width -1 downto 0) ; -- eg in 32 bit operands, 64-bit multiplication result
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
          for i in 0 to 31 loop  -- assuming max 32-bit input
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
signal div_result      : array_3d(fu_range)(SIMD -1 downto 0)((32+16)*2 -1 downto 0);

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

  ---------------- HDCU Performance Counters ----------------
signal hdcu_performance_counter_int        : array_2d(accl_range)(31 downto 0); -- Performance counter for the HDCU

---------------------------------- HDC ARCHITECTURE BEGIN -------------------------------------------

begin
  busy_hdc <= busy_hdc_internal;

  HDC_replicated : for h in accl_range generate
    harc_f(h) <= 0 when multithreaded_accl_en = 1 else h;
  
  ----------------------------- Sequential Stage of HDC Unit ----------------------------------------

  dsp_exec_Unit : process(clk_i, rst_ni)  -- single cycle unit, fully synchronous 
  
  begin
    if rst_ni = '0' then
      rf_rs2(h)     <= '0';
      recover_state(h) <= '0';
    elsif rising_edge(clk_i) then
      
      HVSIZE_READ_lat(h) <= HVSIZE_READ(h);

      if hdc_instr_req(h) = '1' or busy_HDC_internal_lat(h) = '1' then  

        case state_HDC(h) is

          when hdc_init =>

            -------------------------------------------------------------
            --  ██╗███╗   ██╗██╗████████╗    ██████╗ ███████╗██████╗   --
            --  ██║████╗  ██║██║╚══██╔══╝    ██╔══██╗██╔════╝██╔══██╗  --
            --  ██║██╔██╗ ██║██║   ██║       ██║  ██║███████╗██████╔╝  --
            --  ██║██║╚██╗██║██║   ██║       ██║  ██║╚════██║██╔═══╝   --
            --  ██║██║ ╚████║██║   ██║       ██████╔╝███████║██║       --
            --  ╚═╝╚═╝  ╚═══╝╚═╝   ╚═╝       ╚═════╝ ╚══════╝╚═╝       -- 
            -------------------------------------------------------------

            if decoded_instruction_FHRR(HVCLIP_bit_position_FHRR  ) = '1'   then
              rf_rs2(h) <= '1';
            else
              rf_rs2(h) <= '0';  
            end if;

            -- We backup data from decode stage since they will get updated
            HVSIZE_READ_MASK(h) <= HVSIZE(harc_EXEC);
            MPSCLFAC_HDC(h) <= MPSCLFAC(harc_EXEC); -- Contiene il numero di classi

            -- When the decoded instruction is a bundle, we need to multiply the HVSIZE_WRITE by (Data_Width/COUNTERS_NUMBER)
            if decoded_instruction_FHRR(HVBUNDLE_bit_position_FHRR)    = '1' then
            
             
                
             HVSIZE_WRITE(h) <= std_logic_vector(resize(unsigned(HVSIZE(h)), HVSIZE_WRITE(h)'length));

            
            elsif decoded_instruction_FHRR(HVSIM_bit_position_FHRR)    = '1'  then
             
            HVSIZE_WRITE(h) <= std_logic_vector(to_unsigned(4, HVSIZE_WRITE(h)'length));
                  
            --FHRR ENCODING
            elsif decoded_instruction_FHRR(HVENC_bit_position )  = '1' then 
            HVSIZE_WRITE(h) <= std_logic_vector(resize(unsigned(HVSIZE(h)), HVSIZE_WRITE(h)'length));
                   
            elsif decoded_instruction_FHRR(HVBIND_bit_position_FHRR )  = '1' then 
            HVSIZE_WRITE(h) <= std_logic_vector(resize(unsigned(HVSIZE(h)), HVSIZE_WRITE(h)'length));

             elsif decoded_instruction_FHRR(HVCLIP_bit_position_FHRR )  = '1' then 
            HVSIZE_WRITE(h) <= std_logic_vector(resize(unsigned(HVSIZE(h)), HVSIZE_WRITE(h)'length));
            else
              HVSIZE_WRITE(h) <= HVSIZE(harc_EXEC);
            end if;

            decoded_instruction_FHRR_lat(h)  <= decoded_instruction_FHRR;
            
            vec_write_rd_HDC(h) <= vec_write_rd_ID;

            vec_read_rs1_HDC(h) <= vec_read_rs1_ID;
            vec_read_rs2_HDC(h) <= vec_read_rs2_ID;
          
            hdc_rs1_to_sc(h) <= rs1_to_sc;
            hdc_rs2_to_sc(h) <= rs2_to_sc;
            hdc_rd_to_sc(h)  <= rd_to_sc;
            RD_Data_IE_lat(h) <= RD_Data_IE;
            
            

            -- Increment the read addresses if there is a data grant
            if hdc_data_gnt_i(h) = '1' then
              
              ------------------ Source Register 1 ------------------
              if vec_read_rs1_ID = '1'  then
                --FHRR ENCODING
                if decoded_instruction_FHRR(HVENC_bit_position ) = '1'then 
                  RS1_Data_IE_lat(h) <= RS1_Data_IE;
                  RS1_Data_ptr(h) <= RS1_Data_IE;
                
                 --FHRR BUNDLING
                elsif decoded_instruction_FHRR(HVBUNDLE_bit_position_FHRR ) = '1'then 
                  RS1_Data_IE_lat(h) <= RS1_Data_IE;
                
                --CLIPPING BUNDLING
                elsif decoded_instruction_FHRR(HVCLIP_bit_position_FHRR ) = '1'then 
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
                if decoded_instruction_FHRR(HVENC_bit_position) = '1'then 
                  RS2_Data_IE_lat(h) <= RS2_Data_IE;
                  RS2_Data_ptr(h) <= RS2_Data_IE;
                
                -- FHRR BUNDLING
                elsif decoded_instruction_FHRR(HVBUNDLE_bit_position_FHRR) = '1'then 
                RS2_Data_IE_lat(h) <= RS2_Data_IE;
                 
                 -- FHRR BUNDLING
                elsif decoded_instruction_FHRR(HVCLIP_bit_position_FHRR) = '1'then 
                RS2_Data_IE_lat(h) <= RS2_Data_IE;
                else
                  RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE) + SIMD_RD_BYTES_wire(h)); 
                end if;
              else
                RS2_Data_IE_lat(h) <= RS2_Data_IE;
              end if;
              -------------------------------------------------------

              -- Decrement the vector elements that have already been operated on   
              if  (decoded_instruction_FHRR(HVCLIP_bit_position_FHRR)    = '1') then
                    HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC) )*(32+2+PRECISION_BIT_WIDTH)  + 8*SIMD_RD_BYTES_wire(h) , HVSIZE_READ(h)'length));
                    HVSIZE_READ_init(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC) )*(32+2+PRECISION_BIT_WIDTH)  + 8*SIMD_RD_BYTES_wire(h) , HVSIZE_READ(h)'length));
                    HVSIZE_READ_init_clip(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC) )*(32+2+PRECISION_BIT_WIDTH)  + 8*SIMD_RD_BYTES_wire(h) , HVSIZE_READ(h)'length));
              --FHRR ENCODING
              elsif decoded_instruction_FHRR(HVENC_bit_position )  = '1' then
                
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * unsigned(MPSCLFAC(h)), HVSIZE_READ(h)'length)); 
                HVSIZE_READ_init(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * unsigned(MPSCLFAC(h)), HVSIZE_READ(h)'length)); 
              
             
              ---COSINE SIMILARITY
              elsif decoded_instruction_FHRR(HVSIM_bit_position_FHRR )  = '1' then
                if SIMD > 1 then
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC))+ log2_power_of_2(SIMD)*SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
                else 
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC))+ 4, HVSIZE_READ(h)'length));
                end if;
              HV_ELEMENT(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC))/4,  HV_ELEMENT(h)'length));
             elsif decoded_instruction_FHRR(HVBUNDLE_bit_position_FHRR) = '1' then
                  HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)), HVSIZE_READ(h)'length));
                  HVSIZE_READ_init(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)), HVSIZE_READ(h)'length));
  

              elsif decoded_instruction_FHRR(HVBIND_bit_position_FHRR )  = '1' then
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC) ) + SIMD_RD_BYTES_wire(h) , HVSIZE_READ(h)'length));
                
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

            end if;

           ---------------------------------------------------------------------------

          when hdc_exec =>
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
                  if decoded_instruction_FHRR(HVENC_bit_position ) = '1' then
                   

                    if (unsigned(HVSIZE_READ(h)) = (unsigned(HVSIZE_READ_init(h)) - ((unsigned(MPSCLFAC(h)) - 3) * SIMD_RD_BYTES_wire(h)))) then
                        RS1_Data_IE_lat(h) <= RS1_Data_ptr(h) ; -- source 1 address reset
                        --BV_ptr(h) <= std_logic_vector(unsigned(BV_ptr(h)) + 1);
                    else
                    RS1_Data_IE_lat(h) <= std_logic_vector(unsigned(RS1_Data_IE_lat(h)) + 4); -- source 1 address increment
                    end if;

                    if (unsigned(HVSIZE_READ(h)) = (unsigned(HVSIZE_READ_init(h)) - ((unsigned(MPSCLFAC(h))) * SIMD_RD_BYTES_wire(h)))) then
                      HVSIZE_READ_init(h) <= HVSIZE_READ(h);
                    end if;

                   elsif decoded_instruction_FHRR(HVCLIP_bit_position_FHRR ) = '1' then
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
                  if decoded_instruction_FHRR(HVENC_bit_position ) = '1' then
                    
                    if (unsigned(HVSIZE_READ(h)) = (unsigned(HVSIZE_READ_init(h)) - ((unsigned(MPSCLFAC(h)) - 3) * SIMD_RD_BYTES_wire(h)))) then
                    
                      RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_ptr(h)) + SIMD_RD_BYTES_wire(h));
                      RS2_Data_ptr(h) <= std_logic_vector(unsigned(RS2_Data_ptr(h)) + SIMD_RD_BYTES_wire(h));
                     
                    else 
                    RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + unsigned(HVSIZE(h)));
                    RS2_Data_ptr(h) <= RS2_Data_ptr(h);

                    end if;
                    

                    elsif decoded_instruction_FHRR(HVBUNDLE_bit_position_FHRR) = '1' then
                    
                      if (((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_READ(h)))) / SIMD_RD_BYTES_wire(h)) mod 2 = 0) and (HVSIZE_READ(h) /= (Addr_Width downto 0 => 'U') )
                         and ((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_READ(h)))) / SIMD_RD_BYTES_wire(h)) >= 0
                         then
                          RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h));

                        
                      end if;

                     elsif decoded_instruction_FHRR(HVCLIP_bit_position_FHRR ) = '1' then
                        if (unsigned(HVSIZE_READ(h)) = (unsigned(HVSIZE_READ_init(h)) - SIMD_RD_BYTES_wire(h)*(32+2+PRECISION_BIT_WIDTH))) and (HVSIZE_READ(h) /= (Addr_Width downto 0 => 'U') ) then 
                        RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h));
                        end if;
                                      
                  else

                    RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h)); -- source 2 address increment

                  end if;
                end if;

              end if;

              -------------------------- Decrement the vector elements that have already been operated on ---------------------------------------
              if hdc_data_gnt_i(h) = '1' then
                
                if to_integer(unsigned(HVSIZE_READ(h))) >= SIMD_RD_BYTES_wire(h) then
                  
                  HVSIZE_READ(h) <= std_logic_vector(unsigned(HVSIZE_READ(h)) - SIMD_RD_BYTES_wire(h)); -- decrement by SIMD_BYTE Execution Capability
                    if decoded_instruction_FHRR(HVENC_bit_position ) = '1' and enc_stage_2_en(h) =  '0' then
                    HVSIZE_READ(h) <= HVSIZE_READ(h);
                      end if;
                 

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
        hdcu_enc_perf_counter(h)  <= hdcu_enc_perf_counter_int(h);
      end if;

    end process;
  end generate gen_perf_counter;


  ------------ Combinational Stage of HDC Unit ----------------------------------------------------------------------
  HDC_Excpt_Cntrl_Unit_comb : process(all)
  
  variable busy_HDC_internal_wires : std_logic;
  variable hdc_except_condition_wires : std_logic_vector(harc_range);
  variable hdc_taken_branch_FHRR_wires : std_logic_vector(harc_range);  
      
  begin

    busy_HDC_internal_wires        := '0';
    hdc_except_condition_wires(h)  := '0';
    hdc_taken_branch_FHRR_wires(h) := '0';
    wb_ready(h)                    <= '0';
    halt_hdc(h)                    <= '0';
    nextstate_HDC(h)               <= hdc_init;
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

        when hdc_init =>

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
            hdc_taken_branch_FHRR_wires(h):= '1';    
            hdc_except_data_wire(h) <= ILLEGAL_VECTOR_SIZE_EXCEPT_CODE;
          elsif HVSIZE(harc_EXEC)(0) /= '0' and MVTYPE(harc_EXEC)(3 downto 2) = "01" then            -- Set exception if the number of bytes are not divisible by two
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_FHRR_wires(h):= '1';
            hdc_except_data_wire(h) <= ILLEGAL_VECTOR_SIZE_EXCEPT_CODE;
          elsif (rs1_to_sc  = "100" and vec_read_rs1_ID = '1') or
            (rs2_to_sc  = "100" and vec_read_rs2_ID = '1') or
             rd_to_sc   = "100" then     -- Set exception for non scratchpad access
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_FHRR_wires(h):= '1';    
            hdc_except_data_wire(h) <= ILLEGAL_ADDRESS_EXCEPT_CODE;
          elsif rs1_to_sc = rs2_to_sc and vec_read_rs1_ID = '1' and vec_read_rs2_ID = '1' then               -- Set exception for same read access
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_FHRR_wires(h):= '1';    
            hdc_except_data_wire(h) <= READ_SAME_SCARTCHPAD_EXCEPT_CODE;    
          elsif (overflow_rs1_sc(h)(Addr_Width) = '1' and vec_read_rs1_ID = '1') or (overflow_rs2_sc(h)(Addr_Width) = '1' and  vec_read_rs2_ID = '1') then -- Set exception if reading overflows the scratchpad's address
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_FHRR_wires(h):= '1';    
            hdc_except_data_wire(h) <= SCRATCHPAD_OVERFLOW_EXCEPT_CODE;
          elsif overflow_rd_sc(h)(Addr_Width) = '1'  and vec_write_rd_ID = '1' then           -- Set exception if reading overflows the scratchpad's address, scalar writes are excluded
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_FHRR_wires(h):= '1';    
            hdc_except_data_wire(h) <= SCRATCHPAD_OVERFLOW_EXCEPT_CODE;
          else
            if halt_hart(h) = '0' then
              nextstate_HDC(h) <= hdc_exec;
            else
              nextstate_HDC(h) <= hdc_halt_hart;
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
        
         when hdc_halt_hart =>

           if halt_hart(h) = '0' then
             nextstate_HDC(h) <= hdc_exec;
           else
             nextstate_HDC(h) <= hdc_halt_hart;
           end if;
           busy_HDC_internal_wires := '1';

         when hdc_exec =>

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
           if decoded_instruction_FHRR_lat(h)(HVBUNDLE_bit_position_FHRR)  = '1' then
            if bundle_stage_2_en(h) = '1' then 
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
              nextstate_HDC(h) <= hdc_exec;
              busy_HDC_internal_wires := '1';
            end if;
            if wb_ready(h) = '1' then
              hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h))))    <= '1';
              hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
            end if;
          end if;
           ----------------------------------------------------------------------

           ----------------------------- BINDING --------------------------------
           if decoded_instruction_FHRR_lat(h)(HVBIND_bit_position_FHRR)    = '1' then 
             if bind_stage_2_en(h) = '1' then 
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
              nextstate_HDC(h) <= hdc_exec;
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
           if decoded_instruction_FHRR_lat(h)(HVENC_bit_position )  = '1' then
            if enc_stage_3_en(h) = '1'  then 
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
              nextstate_HDC(h) <= hdc_exec;
              busy_HDC_internal_wires := '1';
            end if;
            if wb_ready(h) = '1' then
              hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h))))    <= '1';
              hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
            end if;
          end if;
           ----------------------------------------------------------------------------
          
           ----------------------------- SIMILARITY -----------------------------------
           if decoded_instruction_FHRR_lat(h)(HVSIM_bit_position_FHRR)    = '1' then 
             if sim_stage_2_en(h) = '1' then 
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
              nextstate_HDC(h) <= hdc_exec;
              busy_HDC_internal_wires := '1';
             end if;
             if wb_ready(h) = '1' then
               hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h)))) <= '1';
               hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
             end if;
           end if;
           -------------------------------------------------------------------------

           ---------------------------- CLIPPING -----------------------------------
           if decoded_instruction_FHRR_lat(h)(HVCLIP_bit_position_FHRR)  = '1' then
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
              nextstate_HDC(h) <= hdc_exec;
              busy_HDC_internal_wires := '1';
            end if;
            if wb_ready(h) = '1' then
              hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h))))    <= '1';
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
    hdc_taken_branch_FHRR(h)<= hdc_taken_branch_FHRR_wires(h);
      
  end process;

  ---------------------------------------------------------------------------------------------------------------------------------------------------------
  --  ██████╗ ██╗██████╗ ███████╗██╗     ██╗███╗   ██╗███████╗     ██████╗ ██████╗ ███╗   ██╗████████╗██████╗  ██████╗ ██╗     ██╗     ███████╗██████╗   --
  --  ██╔══██╗██║██╔══██╗██╔════╝██║     ██║████╗  ██║██╔════╝    ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔══██╗██╔═══██╗██║     ██║     ██╔════╝██╔══██╗  --
  --  ██████╔╝██║██████╔╝█████╗  ██║     ██║██╔██╗ ██║█████╗      ██║     ██║   ██║██╔██╗ ██║   ██║   ██████╔╝██║   ██║██║     ██║     █████╗  ██████╔╝  --
  --  ██╔═══╝ ██║██╔═══╝ ██╔══╝  ██║     ██║██║╚██╗██║██╔══╝      ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══██╗██║   ██║██║     ██║     ██╔══╝  ██╔══██╗  --
  --  ██║     ██║██║     ███████╗███████╗██║██║ ╚████║███████╗    ╚██████╗╚██████╔╝██║ ╚████║   ██║   ██║  ██║╚██████╔╝███████╗███████╗███████╗██║  ██║  --
  --  ╚═╝     ╚═╝╚═╝     ╚══════╝╚══════╝╚═╝╚═╝  ╚═══╝╚══════╝     ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚══════╝╚══════╝╚═╝  ╚═╝  --
  ---------------------------------------------------------------------------------------------------------------------------------------------------------

  fsm_HDC_pipeline_controller : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then
      
      hdc_data_gnt_i_lat(h)    <= '0';


      ------------ FHRR BUNDLING ------------
      bundle_stage_1_en(h)     <= '0';
      bundle_stage_2_en(h)     <= '0';
      -------------------------------------

      ------------ BINDING ----------------
      bind_stage_1_en(h)        <= '0';
      bind_stage_2_en(h)        <= '0';
      -------------------------------------

      ------------ ENCODING ----------------
      enc_stage_1_en(h)        <= '0';
      enc_stage_2_en(h)        <= '0';
      enc_stage_3_en(h)        <= '0';

      -------------------------------------

      ------------ SIMILARITY ---------------
      sim_stage_1_en(h)        <= '0';
      sim_stage_2_en(h)        <= '0';
      sim_stage_3_en(h)        <= '0';
      ---------------------------------------

      ------------ CLIPPING -----------------
      clip_stage_1_en(h)       <= '0';
      clip_stage_2_en(h)       <= '0';
      clip_stage_3_en(h)       <= '0';
      ---------------------------------------


      state_HDC(h)             <= hdc_init;

    elsif rising_edge(clk_i) then

      hdc_data_gnt_i_lat(h)    <= hdc_data_gnt_i(h);

     
      ------------FHRR BUNDLING ----------------
      bundle_stage_1_en(h)     <= hdc_data_gnt_i_lat(h) and bundle_en(h);
      if (to_integer(unsigned(HVSIZE_READ_lat(h)))) /= 0 then
      bundle_stage_2_en(h)     <= bundle_stage_1_en(h);
      else 
      bundle_stage_2_en(h)     <= '0';
      end if;
      --------------------------------------
      ------------ BINDING -----------------
      bind_stage_1_en(h)       <= hdc_data_gnt_i_lat(h) and bind_en(h);
      bind_stage_2_en(h)       <= bind_stage_1_en(h);
      --------------------------------------

      ------------ ENCODING -----------------
      enc_stage_1_en(h)       <= hdc_data_gnt_i_lat(h) and enc_en(h);
      enc_stage_2_en(h)       <=  enc_stage_1_en(h);
      if (unsigned(HVSIZE_READ(h)) = (unsigned(HVSIZE_READ_init(h)) - ((unsigned(MPSCLFAC(h))) * SIMD_RD_BYTES_wire(h)))) then
        enc_stage_3_en(h) <= enc_stage_2_en(h);
      
      else 
        enc_stage_3_en(h) <= '0';
      end if;

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
        
      -------------------------------------

      ------------- CLIPPING ------------------
      clip_stage_1_en(h)      <= hdc_data_gnt_i_lat(h) and clip_en(h);
    
     clip_stage_2_en(h)      <= clip_stage_1_en(h);
       --if (unsigned(HVSIZE_READ(h)) = (unsigned(HVSIZE_READ_init(h)) - SIMD_RD_BYTES_wire(h)*34)) and (HVSIZE_READ(h) /= (Addr_Width downto 0 => 'U') )then
        if ((unsigned(HVSIZE_READ_lat(h)) + 4*SIMD_RD_BYTES_wire(h) + PRECISION_BIT_WIDTH = unsigned(HVSIZE_READ_init(h))) and 
                               ( (unsigned(HVSIZE_READ_init_clip(h)) /= unsigned(HVSIZE_READ_init(h)))) ) or (to_integer(unsigned(HVSIZE_READ_lat(h)) )= 0 ) then 
        clip_stage_3_en(h) <= clip_stage_2_en(h);
      
      else 
        clip_stage_3_en(h) <= '0';
    end if;
      ---------------------------------------


      halt_hdc_lat(h)          <= halt_hdc(h);
      state_HDC(h)             <= nextstate_HDC(h);
      busy_HDC_internal_lat(h) <= busy_HDC_internal(h);
      SIMD_RD_BYTES(h)         <= SIMD_RD_BYTES_wire(h);
      hdc_except_data(h)       <= hdc_except_data_wire(h);

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



      if hdc_instr_req(h) = '1' or busy_HDC_internal_lat(h) = '1' then

        case state_HDC(h) is

          when hdc_init =>

            ------------------------ FHRR BUNDLING ---------------------------------
            if decoded_instruction_FHRR(HVBUNDLE_bit_position_FHRR)    = '1' then 
              bundle_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ------------------------ BINDING ----------------------------------
            elsif decoded_instruction_FHRR(HVBIND_bit_position_FHRR) = '1' then
              bind_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ------------------------ ENCODING ----------------------------------
            elsif decoded_instruction_FHRR(HVENC_bit_position  ) = '1' then
              enc_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ---------------------- SIMILARITY ---------------------------------
            elsif decoded_instruction_FHRR(HVSIM_bit_position_FHRR)    = '1' then
              sim_en_wire(h) <= '1';                                         
            -------------------------------------------------------------------

            ---------------------- CLIPPING -----------------------------------
            elsif decoded_instruction_FHRR(HVCLIP_bit_position_FHRR  ) = '1' then
              clip_en_wire(h) <= '1';
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

  

      if hdc_instr_req(h) = '1' or busy_HDC_internal_lat(h) = '1' then

        case state_HDC(h) is

          when hdc_init =>

            

             ------------------------ FHRR BUNDLING --------------------------------
             if decoded_instruction_FHRR(HVBUNDLE_bit_position_FHRR)    = '1' then
              if busy_bundle = '0' and bundle_en_pending = (accl_range => '0') then 
                bundle_en_wire(h) <= '1';
              else
                bundle_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(0) <= '1';
              end if;
            ------------------------------------------------------------------


            ---------------------- BINDING -----------------------------------
            elsif decoded_instruction_FHRR(HVBIND_bit_position_FHRR) = '1' then
              if busy_bind = '0' and bind_en_pending = (accl_range => '0') then 
                bind_en_wire(h) <= '1';
              else
                bind_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(2) <= '1';
              end if;
            ------------------------------------------------------------------

            ---------------------- ENCODING -----------------------------------
            elsif decoded_instruction_FHRR(HVENC_bit_position  ) = '1' then
              if busy_enc = '0' and enc_en_pending = (accl_range => '0') then 
                enc_en_wire(h) <= '1';
              else
                enc_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(3) <= '1';
              end if;
            ------------------------------------------------------------------

            ----------------------- SIMILARITY -------------------------------
            elsif decoded_instruction_FHRR(HVSIM_bit_position_FHRR)    = '1' then
              if busy_sim = '0' and sim_en_pending = (accl_range => '0') then 
                sim_en_wire(h) <= '1';                                        
              else
                sim_en_pending_wire(h) <= '1';                                   
                halt_hart(h) <= '1';
                fu_req(h)(5) <= '1';
              end if;
            -------------------------------------------------------------------

            ----------------------- CLIPPING -----------------------------------
            elsif decoded_instruction_FHRR(HVCLIP_bit_position_FHRR  ) = '1' then
              if busy_clip = '0' and clip_en_pending = (accl_range => '0') then 
                clip_en_wire(h) <= '1';
              else
                clip_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(6) <= '1';
              end if;
            --------------------------------------------------------------------


            end if;

          when hdc_halt_hart =>
  
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
            when hdc_halt_hart =>
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
        when hdc_halt_hart =>
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
          
          when hdc_init =>


          when hdc_exec =>

        ---------------------------FHRR BUNDLING ------------------------------
            if decoded_instruction_FHRR_lat(h)(HVBUNDLE_bit_position_FHRR) = '1' then
                
              for i in 0 to SIMD-1 loop
              if ((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_READ_lat(h)))) / SIMD_RD_BYTES_wire(h)) >= 0 and HVSIZE_READ_lat(h) /= (Addr_Width downto 0 => 'U') then
                if (((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_READ_lat(h)))) / SIMD_RD_BYTES_wire(h)) mod 2 = 0) then
                    hdc_sc_data_write_wire_int(h)(Data_Width + Data_Width*i -1 downto Data_Width*i) <= imag_accum(h)(Data_Width + Data_Width*i -1 downto Data_Width*i);
                else
                    hdc_sc_data_write_wire_int(h)(Data_Width + Data_Width*i -1 downto Data_Width*i) <= real_accum(h)(Data_Width + Data_Width*i -1 downto Data_Width*i);
                end if;
            end if;
            end loop;
              end if;
            -------------------------------------------------------------------

            --------------------------- BINDING --------------------------------
            if (decoded_instruction_FHRR_lat(h)(HVBIND_bit_position_FHRR)    = '1' ) then
            
              for i in 0 to SIMD-1 loop
                
                hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_bind_results(h)((8*(i+1))-1 downto 8*i);
                
              end loop;
              end if;
            --------------------------------------------------------------------
            

            --------------------------- ENCODING --------------------------------
            if (decoded_instruction_FHRR_lat(h)(HVENC_bit_position )    = '1' ) then

              for i in 0 to SIMD-1 loop
                
                hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_enc_result(h)((8*(i+1))-1 downto 8*i);
                
              end loop;
            end if;
            --------------------------------------------------------------------
            -------------------------- SIMILARITY -------------------------------
            if decoded_instruction_FHRR_lat(h)(HVSIM_bit_position_FHRR)      = '1' then
              hdc_sc_data_write_wire_int(h)(Data_Width -1  downto 0) <=  cosine_accumulator_avg(h);              
            end if;
            ---------------------------------------------------------------------

            ------------------------ CLIPPING -----------------------------------
            if decoded_instruction_FHRR_lat(h)(HVCLIP_bit_position_FHRR  )   = '1' then
              hdc_sc_data_write_wire_int(h)(SIMD_Width - 1 downto 0) <= hdcu_out_clip_results(h)(SIMD_Width - 1 downto 0) ;
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
          
          when hdc_init =>

         
          when hdc_exec =>
            ------------------------- FHRR BUNDLING --------------------------------
            if decoded_instruction_FHRR_lat(h)(HVBUNDLE_bit_position_FHRR) = '1' then
                for i in 0 to SIMD-1 loop
                  if ((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_READ_lat(h)))) / SIMD_RD_BYTES_wire(h)) >= 0 and HVSIZE_READ_lat(h) /= (Addr_Width downto 0 => 'U') then
                    if (((to_integer(unsigned(HVSIZE_READ_init(h))) - to_integer(unsigned(HVSIZE_READ_lat(h)))) / SIMD_RD_BYTES_wire(h)) mod 2 = 0) then
                        hdc_sc_data_write_wire_int(h)(Data_Width + Data_Width*i -1 downto Data_Width*i) <= imag_accum(0)(Data_Width + Data_Width*i -1 downto Data_Width*i);
                    else
                        hdc_sc_data_write_wire_int(h)(Data_Width + Data_Width*i -1 downto Data_Width*i) <= real_accum(0)(Data_Width + Data_Width*i -1 downto Data_Width*i);
                    end if;
                end if;
                end loop;
                  end if;

            ----------------------------------------------------------------

            ------------------------- BINDING --------------------------------
            if (decoded_instruction_FHRR_lat(h)(HVBIND_bit_position_FHRR)    = '1' ) then
              for i in 0 to SIMD-1 loop
                
                hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_bind_results(0)((8*(i+1))-1 downto 8*i);
                
              end loop;
            end if;
            -------------------------------------------------------------------

            ------------------------- ENCODING --------------------------------
            if (decoded_instruction_FHRR_lat(h)(HVENC_bit_position )    = '1' ) then
              --hdc_sc_data_write_wire_int(h)(SIMD_Width - (Data_Width-8)*SIMD_Width/Data_Width -1 downto 0) <=  hdc_out_enc_result(h);

              for i in 0 to SIMD-1 loop
                
                hdc_sc_data_write_wire_int(h)((8-1)+32*i downto 32*i) <= hdc_out_enc_result(0)((8*(i+1))-1 downto 8*i);
                
              end loop;
            end if;
            -------------------------------------------------------------------
            
            ------------------------- SIMILARITY ------------------------------
            if decoded_instruction_FHRR_lat(h)(HVSIM_bit_position_FHRR)      = '1' then
              hdc_sc_data_write_wire_int(h)(Data_Width -1 downto 0) <=  cosine_accumulator_avg(0);              
            end if;
            -------------------------------------------------------------------

            ------------------------- CLIPPING --------------------------------
            if decoded_instruction_FHRR_lat(h)(HVCLIP_bit_position_FHRR  )   = '1' then
              hdc_sc_data_write_wire_int(h)(SIMD_Width - 1 downto 0) <= hdcu_out_clip_results(0)(SIMD_Width - 1 downto 0) ;

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

    for g in 0 to (ACCL_NUM - FU_NUM) loop

      if multithreaded_accl_en = 1 then
        h := g;  -- set the spm rd/wr ports equal to the "for-loop"
      elsif multithreaded_accl_en = 0 then
        h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
      end if;

      if hdc_instr_req(h) = '1' or busy_HDC_internal_lat(h) = '1' then
        case state_HDC(h) is
          
          when hdc_exec =>

            ---------------------------- FHRR BUNDLING --------------------------------------
             if decoded_instruction_FHRR_lat(h)(HVBUNDLE_bit_position_FHRR) = '1' then
              hdcu_in_bundled(f) <= hdc_sc_data_read(h)(0);
            
                 for i in 0 to SIMD-1 loop
                hdcu_in_bundle_operand(f)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(1)((8-1)+32*i downto 32*i);
                
                end loop;
            
              end if;
            --------
            -------------------------------------------------------------------------

            ----------------------------- BINDING -----------------------------------
            if decoded_instruction_FHRR_lat(h)(HVBIND_bit_position_FHRR)  = '1' then 
              
              for i in 0 to SIMD-1 loop
                hdcu_in_bind_operands(f)(0)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(0)((8-1)+32*i downto 32*i);
                hdcu_in_bind_operands(f)(1)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(1)((8-1)+32*i downto 32*i);
            end loop;
            
            end if;
            ------------------------------------------------------------------------

            ----------------------------- ENCODING -----------------------------------
            if decoded_instruction_FHRR_lat(h)(HVENC_bit_position )  = '1' then
                         
            hdcu_in_enc_fpp_operands(f)(0)(7 downto 0) <= hdc_sc_data_read(h)(0)(7 downto 0);


            for i in 0 to SIMD-1 loop
              --hdcu_in_enc_fpp_operands(f)(0)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(0)((8-1)+32*i downto 32*i);
              hdcu_in_enc_fpp_operands(f)(1)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(1)((8-1)+32*i downto 32*i);
            end loop;
            end if; 
            ------------------------------------------------------------------------

            ----------------------------- SIMILARITY -------------------------------
           

            if (decoded_instruction_FHRR_lat(h)(HVSIM_bit_position_FHRR)  = '1')  then

              for i in 0 to SIMD-1 loop
                hdcu_in_sim_operands(f)(0)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(0)((8-1)+32*i downto 32*i);
                hdcu_in_sim_operands(f)(1)((8*(i+1))-1 downto 8*i) <= hdc_sc_data_read(h)(1)((8-1)+32*i downto 32*i);
            end loop;
            

        end if;
            ------------------------------------------------------------------------

             --------------------------- CLIPPING ----------------------------------
             if decoded_instruction_FHRR_lat(h)(HVCLIP_bit_position_FHRR  ) = '1' then
              hdcu_in_clip_operands(f)(0) <= hdc_sc_data_read(h)(0);
              hdcu_in_clip_operands(f)(1) <= hdc_sc_data_read(h)(1);
            end if;
            ------------------------------------------------------------------------
            

          when others =>
            null;
        end case;
      end if;
    end loop;
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
  begin
    if rst_ni = '0' then
      hdcu_in_bundle_operand_real <= (others => (others => '0'));
      hdcu_in_bundle_operand_imag <= (others => (others => '0'));
  
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
  
              if hdcu_in_bundle_operand(f)((8 - 1) + 8 * (i)) = '1' then
                hdcu_in_bundle_operand_imag(f)(Data_Width*i + Data_width - 1 downto Data_Width*i) <= std_logic_vector(-signed(sine_lut_bun(to_integer(unsigned(-signed(hdcu_in_bundle_operand(f)((8-1)+8*(i)  downto 8*(i) )))))));
                hdcu_in_bundle_operand_real(f)(Data_Width*i + Data_width - 1 downto Data_Width*i) <= std_logic_vector(cosine_lut_bun(to_integer(unsigned(-signed(hdcu_in_bundle_operand(f)((8-1)+8*(i)  downto 8*(i) ))))));
              else
                hdcu_in_bundle_operand_imag(f)(Data_Width*i + Data_width - 1 downto Data_Width*i) <= std_logic_vector(sine_lut_bun(to_integer(unsigned(hdcu_in_bundle_operand(f)((8-1)+8*(i)  downto 8*(i))))));
                hdcu_in_bundle_operand_real(f)(Data_Width*i + Data_width - 1 downto Data_Width*i) <= std_logic_vector(cosine_lut_bun(to_integer(unsigned(hdcu_in_bundle_operand(f)((8-1)+8*(i)  downto 8*(i))))));
              end if;
            end loop;
  
          end if;
        end if;
      end loop;
    end if;
  end process;
  

    fsm_comps_accum : process (all)
      variable h : integer;
    begin
      real_accum <= (others => (others => '0'));
      imag_accum <= (others => (others => '0'));
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g; -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then
          h := f; -- set the spm rd/wr ports equal to the "for-generate" 
        end if;
        if halt_hdc_lat(h) = '0' then
          if bundle_en(h) = '1' and (bundle_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then

            for i in 0 to SIMD - 1 loop

              real_accum(f)((Data_Width - 1) + Data_Width * (i) downto Data_Width * (i)) <= std_logic_vector(unsigned(hdcu_in_bundle_operand_real(f)((Data_Width - 1) + Data_Width * (i) downto Data_Width * (i))) + unsigned(hdcu_in_bundled(f)((Data_Width - 1) + Data_Width * (i) downto Data_Width * (i))));
              imag_accum(f)((Data_Width - 1) + Data_Width * (i) downto Data_Width * (i)) <= std_logic_vector(unsigned(hdcu_in_bundle_operand_imag(f)((Data_Width - 1) + Data_Width * (i) downto Data_Width * (i))) + unsigned(hdcu_in_bundled(f)((Data_Width - 1) + Data_Width * (i) downto Data_Width * (i))));

            end loop;

          end if;
        end if;
      end loop;

    end process;


  
  --------------------------------------------------------------------------------------------
  -- ██████╗ ██╗███╗   ██╗██████╗ ██╗███╗   ██╗ ██████╗     ██╗   ██╗███╗   ██╗██╗████████╗ --
  -- ██╔══██╗██║████╗  ██║██╔══██╗██║████╗  ██║██╔════╝     ██║   ██║████╗  ██║██║╚══██╔══╝ --
  -- ██████╔╝██║██╔██╗ ██║██║  ██║██║██╔██╗ ██║██║  ███╗    ██║   ██║██╔██╗ ██║██║   ██║    --
  -- ██╔══██╗██║██║╚██╗██║██║  ██║██║██║╚██╗██║██║   ██║    ██║   ██║██║╚██╗██║██║   ██║    --
  -- ██████╔╝██║██║ ╚████║██████╔╝██║██║ ╚████║╚██████╔╝    ╚██████╔╝██║ ╚████║██║   ██║    --
  -- ╚═════╝ ╚═╝╚═╝  ╚═══╝╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝      ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --                                                                                                                                                            
  --------------------------------------------------------------------------------------------

  fsm_MUL_STAGE_1 : process(clk_i,rst_ni)
  variable h : integer;
  begin
    if rst_ni = '0' then
      hdc_out_bind_results <= (others => (others =>'0'));
    elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;
        if halt_hdc_lat(h) = '0' then
          if bind_en(h) = '1' and (bind_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
            for i in 0 to SIMD-1 loop
                hdc_out_bind_results(f)((8-1) + 8*i downto 8*i)  <= std_logic_vector(unsigned(hdcu_in_bind_operands(f)(0)((8-1) + 8*i downto 8*i))+ unsigned(hdcu_in_bind_operands(f)(1)((8-1) + 8*i downto 8*i)));      
            end loop;
          end if;
        end if;
      end loop;
    end if;
  end process;


-- ==========================================
--       FHRR ENCODING
-- ==========================================

 -- Multiplier Unit: Comb process
 multiplier_unit_comb: process(all)
 variable h : integer;
 begin
     mult_result <= (others => (others =>'0'));
     for g in 0 to (ACCL_NUM - FU_NUM) loop
      if multithreaded_accl_en = 1 then
        h := g;  -- set the spm rd/wr ports equal to the "for-loop"
      elsif multithreaded_accl_en = 0 then
        h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
      end if;
      if halt_hdc_lat(h) = '0' then
        if enc_en(h) = '1' and (enc_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
          

            for i in 0 to SIMD-1 loop
     
              -- Perform multiplication and store the full 64-bit product
         
                mult_result(f)((8-1)*2 + 8*(i)*2 + 1 downto 8*(i)*2) <=std_logic_vector( 
                signed(hdcu_in_enc_fpp_operands(f)(0)((8-1)  downto 0))*                             
                signed(hdcu_in_enc_fpp_operands(f)(1)((8-1) + 8*i downto 8*i)));
            
            end loop; 
          
        end if;
       end if;
      end loop;
 end process;
 
 -- Multiplier Unit: synchronous process
 multiplier_unit : process(clk_i, rst_ni)
 variable h : integer;
 begin
     if rst_ni = '0' then            
        trunc_mul_results <= (others => (others => '0'));
     elsif rising_edge(clk_i) then
          trunc_mul_results <= (others => (others => '0'));
          
           for g in 0 to (ACCL_NUM - FU_NUM) loop
            if multithreaded_accl_en = 1 then
              h := g;  -- set the spm rd/wr ports equal to the "for-loop"
            elsif multithreaded_accl_en = 0 then
              h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
            end if;
            if halt_hdc_lat(h) = '0' then
              if enc_en(h) = '1' and (enc_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
              for i in 0 to SIMD-1 loop
             -- Extract the lower 32 bits of the multiplication result
                
                trunc_mul_results (f)((8-1) + 8*i downto 8*i)  <= 
                mult_result(f)(8*2 + 8*2*i  -1 downto  2*8*i+8); --MAINTING 8BIT PRECISION
              
               end loop;
             end if ;
             end if;
          end loop;  
         
     end if;
 end process;

 -- Accumulator Unit Comb process
 accumulator_unit_comb:process(all)
 variable h : integer;
 begin
  --accumulator_wire <= (others => (others => '0'));
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;
        -- if halt_hdc_lat(h) = '0' then
        --   if enc_en(h) = '1' and (enc_stage_2_en(h) = '1' or recover_state_wires(h)  = '1') then
        --    --for i in 0 to SIMD-1 loop
        
        --       accumulator_wire(f) <= std_logic_vector(unsigned(accumulator_reg(f)) + unsigned(trunc_mul_results(f)));                              
       
        --    --end loop;

          
        --   end if;
        -- end if;
        accumulator_wire(f)   <= std_logic_vector(unsigned(accumulator_reg(f)) + unsigned(trunc_mul_results(f)));
        hdc_out_enc_result(h) <= accumulator_reg(h);
        end loop;   
        
    
 end process;
 
 -- Accumulator Unit Sync Process
 accumulator_unit : process(clk_i, rst_ni)
 variable h : integer;
 begin
     if rst_ni = '0' then
         accumulator_reg <= (others => (others => '0'));
         
     elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;         if halt_hdc_lat(h) = '0' then
          -- if enc_en(h) = '1' and (enc_stage_2_en(h) = '1' or recover_state_wires(h) = '1') then
            
             
          accumulator_reg <= accumulator_wire;   
           
          -- elsif enc_stage_3_en(h) = '1' then
          if enc_stage_3_en(h) = '1' then

           accumulator_reg(h)  <= trunc_mul_results(h) ;
           
          elsif unsigned(HVSIZE_READ(h))= unsigned(HVSIZE(h)) * unsigned(MPSCLFAC(h)) then
            accumulator_reg(h) <= (others => '0');
          end if;      
        end if;
       end loop;
      
       
     end if;
 end process;

 
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
  delta <= (others => (others => '0'));
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
 
 
 fsm_HDC_cosine_sim_sync : process(clk_i, rst_ni)
 variable h : integer;
 variable index : integer range 0 to 255; -- Declare the variable for index
 begin
   if rst_ni = '0' then
    cosine_diff <= (others => (others => '0'));
   elsif rising_edge(clk_i) then
     for g in 0 to (ACCL_NUM - FU_NUM) loop
       if multithreaded_accl_en = 1 then
         h := g;  -- set the spm rd/wr ports equal to the "for-loop"
       elsif multithreaded_accl_en = 0 then
         h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
       end if;
       if halt_hdc_lat(h) = '0' then
         if sim_en(h) = '1' and (sim_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
           for i in 0 to SIMD-1 loop
            
            if delta(f)((8-1) + 8 * i ) = '1' then 
            
            cosine_diff(f)((Data_Width-1)+Data_Width*(i) downto Data_Width*(i) ) <= cosine_lut(to_integer(unsigned(-signed(delta(f)((8-1)+8*(i)  downto 8*(i) )))));
            else
            
            -- Use the index to retrieve the cosine value from the LUT
            cosine_diff(f)((Data_Width-1)+Data_Width*(i) downto Data_Width*(i) ) <= cosine_lut(to_integer(unsigned((delta(f)((8-1)+8*(i)  downto 8*(i) )))));
            
            end if;
           end loop;
         
         end if;
       end if;
     end loop;
   end if;
 end process;


 fsm_HDC_cosine_diff_treeAdder : process(all)
 variable h : integer;
begin
 cosine_diff_flat  <= (others => '0');
 for g in 0 to (ACCL_NUM - FU_NUM) loop
   if multithreaded_accl_en = 1 then
     h := g;
   elsif multithreaded_accl_en = 0 then
     h := f;
   end if;
   if halt_hdc_lat(h) = '0' then
     if sim_en(h) = '1' and (sim_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
       if SIMD > 1 then 
       -- Flatten the SIMD-wide cosine_diff(f) vector
       for i in 0 to SIMD - 1 loop
        cosine_diff_flat((i+1)*DATA_WIDTH - 1 downto i*DATA_WIDTH) <=
        unsigned(cosine_diff(f)((i+1)*DATA_WIDTH - 1 downto i*DATA_WIDTH));
      
       end loop;
        end if;
     end if;
   end if;
 end loop;
end process;

--GENERATION OF TREE ADDER
gen_stages: for level in 0 to NUM_LEVELS-1 generate
    constant num_in : integer := Simd_Width/Data_Width / (2 ** level);
    constant in_w   : integer := stage_in_width(level);
    constant out_w  : integer := stage_out_width(level);
    constant current_width : integer := Data_Width ;  -- width of each input element at this stage
    signal stage_in_sig  : unsigned(in_w-1 downto 0);
    signal stage_out_sig : unsigned(out_w-1 downto 0);
  begin

      gen_first_stage: if level = 0 generate
        stage_in_sig <= unsigned(cosine_diff(0)(in_w-1 downto 0));

        tree_adder_inst: tree_adder
            generic map ( 
            NUM_INPUTS => num_in,
            DATA_WIDTH => current_width 
            )
            port map (
            clk      => clk_i,
            rst_n    => rst_ni,
            in_data  => stage_in_sig,
            out_data => stage_out_sig
            );
            
        stage_array(level)(out_w-1 downto 0) <= stage_out_sig;
        stage_array(level)(MAX_WIDTH-1 downto out_w) <= (others => '0');
      end generate;

      gen_other_stages: if level > 0 generate
        stage_in_sig <= stage_array(level-1)(in_w-1 downto 0);
        tree_adder_inst: tree_adder
            generic map ( 
            NUM_INPUTS => num_in,
            DATA_WIDTH => Data_Width  -- 1 bit is added for each level to account for the carry
            )
            port map (
            clk      => clk_i,
            rst_n    => rst_ni,
            in_data  => stage_in_sig,
            out_data => stage_out_sig
            );
                
            stage_array(level)(out_w-1 downto 0) <= stage_out_sig;
            stage_array(level)(MAX_WIDTH-1 downto out_w) <= (others => '0');
     
      end generate;

  end generate gen_stages;



-- Accumulator Unit Comb process
 cosine_accumulator_unit_comb:process(all)
 variable h : integer;
 
 begin
  cosine_accumulator_wire <= (others => (others => '0'));
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;
        if halt_hdc_lat(h) = '0' then
          if sim_en(h) = '1' and (sim_stage_1_en(h) = '1' or recover_state_wires(h)  = '1') then
            if SIMD > 1 then 
              cosine_accumulator_wire(f) <= std_logic_vector(unsigned(cosine_accumulator_reg(f)) + unsigned(stage_array(NUM_LEVELS -1)(Data_Width -1 downto 0)));  
            else 
               cosine_accumulator_wire(f) <= std_logic_vector(unsigned(cosine_accumulator_reg(f)) + unsigned(cosine_diff(f)));  
            end if;  
      
              

          end if;
        end if;
        
        end loop;           
 end process;
 
 -- Accumulator Unit Sync Process
 cosine_accumulator_unit : process(clk_i, rst_ni)
 variable h : integer;
 variable hv_element_int : integer;
 variable hv_element_log2 : integer;
 begin
     if rst_ni = '0' then
         cosine_accumulator_reg <= (others => (others => '0'));
         cosine_accumulator_avg <= (others => (others => '0'));
     elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;        
         if halt_hdc_lat(h) = '0' then
          if sim_en(h) = '1' and (sim_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then

          cosine_accumulator_reg(f) <= cosine_accumulator_wire(f);   
              
          
          --FOR 1/D, HV_ELEMENT is the number elements in hypervector
          if ( (to_integer(unsigned(HVSIZE_READ_lat(h)) - 4) = 0 and SIMD = 1 and HVSIZE_READ_lat(h) /= (Addr_Width downto 0 => 'U')) 
                  or 
                  (to_integer(unsigned(HVSIZE_READ_lat(h))) = 0 and SIMD > 1 and HVSIZE_READ_lat(h) /= (Addr_Width downto 0 => 'U')) ) then

                  hv_element_int := to_integer(unsigned(HV_ELEMENT(h)));
                  hv_element_log2 := log2_power_of_2(hv_element_int);
                  
                  cosine_accumulator_avg(f) <= std_logic_vector( signed(cosine_accumulator_wire(f)) srl hv_element_log2 );

              end if;
        end if;
        end if;
       end loop;
     end if;
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
begin
    if rst_ni = '0' then
        dividend(f) <= (others => (others => '0'));
        divisor(f) <=  (others => (others => '0'));
        
        -- dividend_scalar <= (others => '0');
        -- divisor_scalar <= (others => '0');
        
        div_enable(f) <= (others => (others => '0'));
        div_done(f) <= (others => (others => '0'));
        tangent <= (others => (others => '0'));
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
                        dividend(f)(i)(31 + PRECISION_BIT_WIDTH downto PRECISION_BIT_WIDTH) <= hdcu_in_clip_operands(f)(0)((Data_Width-1)+Data_Width*i downto Data_Width*i);
                        divisor(f)(i)(Data_Width -1 downto 0) <= hdcu_in_clip_operands(f)(1)((Data_Width-1)+Data_Width*i downto Data_Width*i);




                        
                          if unsigned(divisor(f)(i)) /= 0 and   (
                          (unsigned(HVSIZE_READ_lat(h)) + SIMD_RD_BYTES_wire(h) = unsigned(HVSIZE_READ_init_clip(h))) or 
                          (
                              (unsigned(HVSIZE_READ_lat(h)) + 2*SIMD_RD_BYTES_wire(h) + PRECISION_BIT_WIDTH = unsigned(HVSIZE_READ_init(h))) and 
                              (unsigned(HVSIZE_READ_init_clip(h)) /= unsigned(HVSIZE_READ_init(h))) and (unsigned(HVSIZE_READ(h))-SIMD_RD_BYTES_wire(h) /= 0)
                          )
                                   ) then  
                                    div_enable(f)(i)(1) <= '1';  -- Pulse divider
                                    else 
                                    div_enable(f)(i)(1) <= '0'; 
                            end if;

                       


                        if (unsigned(HVSIZE_READ_lat(h)) + 3*SIMD_RD_BYTES_wire(h) + PRECISION_BIT_WIDTH = unsigned(HVSIZE_READ_init(h))) and 
                                (unsigned(HVSIZE_READ_init_clip(h)) /= unsigned(HVSIZE_READ_init(h))) then 
                          tangent(f)((Data_Width-1)+Data_Width*i downto Data_Width*i)  <= div_result(f)(i)((Data_Width-1) downto 0 ) ;  -- store quotient
                          end if;
                    end loop;

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


fsm_atan_sync : process(clk_i, rst_ni)
variable h : integer;
variable index : integer range 0 to 255; -- Declare the variable for index

begin
  if rst_ni = '0' then
    hdcu_out_clip_results <= (others => (others => '0'));
  elsif rising_edge(clk_i) then
    for g in 0 to (ACCL_NUM - FU_NUM) loop
      if multithreaded_accl_en = 1 then
        h := g;  -- set the spm rd/wr ports equal to the "for-loop"
      elsif multithreaded_accl_en = 0 then
        h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
      end if;
      if halt_hdc_lat(h) = '0' then
        if clip_en(h) = '1' and (clip_stage_2_en(h) = '1' or recover_state_wires(h) = '1') then
          for i in 0 to SIMD-1 loop
           
           --extracting 8bit msb for index
           index := to_integer(unsigned(tang_map(f)((Data_Width-1)+Data_Width*(i)  downto (Data_Width)+Data_Width*(i) - 8)));
           
       
           -- Use the index to retrieve the atan value from the LUT
           if (tangent(f)((Data_Width-1) + Data_Width * i ) = '1') then 
            if to_integer(unsigned(tangent(f)((Data_Width-1) + Data_Width * i downto Data_Width*i ))) < 2621440 then 
             hdcu_out_clip_results(f)((Data_Width-1)+Data_Width*(i) downto Data_Width*(i) ) <= std_logic_vector(-signed(atan_lut(index)));
             else
              hdcu_out_clip_results(f)((Data_Width-1)+Data_Width*(i) downto Data_Width*(i) ) <= std_logic_vector(-signed(atan_lut(255)));
            end if;
             else 
             if to_integer(unsigned(tangent(f)((Data_Width-1) + Data_Width * i downto Data_Width*i ))) < 2621440 then
               hdcu_out_clip_results(f)((Data_Width-1)+Data_Width*(i) downto Data_Width*(i) ) <= atan_lut(index);
              else 
               hdcu_out_clip_results(f)((Data_Width-1)+Data_Width*(i) downto Data_Width*(i) ) <= atan_lut(255);
             end if;
               end if;
           
              
          end loop;
        end if;
      end if;
    end loop;
  end if;
end process;
 -------------------------------------------------------------------------------------------------------------------------
end generate FU_replicated;
end FHRR;
--------------------------------- END of HDC architecture ------------------------------------

