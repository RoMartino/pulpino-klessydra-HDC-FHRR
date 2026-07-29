----------------------------------------------------------------------------------------------------------
--  DSP Unit(s) --                                                                                      --
--  Author(s): Abdallah Cheikh abdallah.cheikh@uniroma1.it (abdallah93.as@gmail.com)                    --
--                                                                                                      --
--  Date Modified: 02-04-2020                                                                           --
----------------------------------------------------------------------------------------------------------
--  The DSP unit executes on vectors fetched from local-low-latency-wide-bus scratchpad memories.       --
--  The DSP has five functional units, adder/subtractor, multiplier, right arith/logic shifter,         --
--  accumulator, and ReLu each of which supports three integer data types (8-bit, 16-bit and 32-bits)   --
--  The data parallelism of the DSP is defined by the SIMD parameter in the PKG file. Increasing the    --
--  data level parallelism increasess the number of banks per SPM as well, as the number of functional  --
--  units. To increase the instruction level parallelism, the replicated_accl_en parameter must be      --
--  set. Setting it will provide a dedicated hardware accelerator for each hart,                        --
--  Custom CSRs are implemented for the accelerator unit                                                --
----------------------------------------------------------------------------------------------------------

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

-- MCR  pinout --------------------
entity MCR_Unit is

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
    ACC_SIMD_BITS         : natural;
    Data_Width            : natural;
    SIMD_Width            : natural;
    MCR_Acc_SIMD_Width    : natural;
    HDCU_PERF_EN          : natural; -- Enable HDCU performance counters
    FP_Width_MCR          : natural; -- Fixed Point Width
    FP_frac_MCR           : natural  -- Fixed Point Fractional Bits
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

    hdcu_performance_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0); 
    hdcu_bind_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_bundle_perf_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_clip_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_sim_perf_counter      : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_perm_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_search_perf_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    

  -- Program Counter Signals
    hdc_taken_branch_MCR       : out std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_except_condition       : out std_logic_vector(ACCL_NUM-1 downto 0);
  -- ID_Stage Signals
    decoded_instruction_MCR    : in  std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0);
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
  -- Scratchpad Interface Signals
    dsp_data_gnt_i             : in  std_logic_vector(ACCL_NUM-1 downto 0);
    dsp_sci_wr_gnt             : in  std_logic_vector(ACCL_NUM-1 downto 0);
    dsp_sc_data_read           : in  array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(SIMD_Width-1 downto 0);
    dsp_sc_acc_data_read       : in  array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(MCR_Acc_SIMD_Width-1 downto 0);
    hdc_we_word_MCR            : out array_2d(ACCL_NUM-1 downto 0)(SIMD-1 downto 0);
    dsp_acc_we_word_MCR        : out array_2d(ACCL_NUM-1 downto 0)((MCR_Acc_SIMD_Width/32)-1 downto 0);

    hdc_sc_read_addr_MCR           : out array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(Addr_Width-1 downto 0);
    hdc_to_sc_MCR                  : out array_3d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0)(1 downto 0);
    hdc_sc_data_write_wire_MCR      : out array_2d(ACCL_NUM-1 downto 0)(SIMD_Width-1 downto 0);
    dsp_sc_acc_data_write_wire_MCR  : out array_2d(ACCL_NUM-1 downto 0)(MCR_Acc_SIMD_Width-1 downto 0);
    hdc_sc_write_addr_MCR           : out array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
    hdc_sci_we_MCR                 : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    hdc_sci_req_MCR                : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    -- tracer signals
    state_HDC                      : out array_2d(ACCL_NUM-1 downto 0)(1 downto 0)
  );

end entity;  

------------------------------------------

architecture MCR of MCR_Unit is

     ----------------------------------------------------------------------------
  -- Component declaration for tree_adder_stage.
  ----------------------------------------------------------------------------
  component tree_adder is
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
  end component;

  ----------------------------------------------------------------------------
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

  subtype harc_range is natural range THREAD_POOL_SIZE-1 downto 0;
  subtype accl_range is integer range ACCL_NUM-1 downto 0;
  subtype fu_range   is integer range FU_NUM-1 downto 0;    --TODO, probabilmente FU_NUM è da cambiare

  signal nextstate_DSP : array_2d(accl_range)(1 downto 0);

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
  signal dsp_sc_acc_data_write_wire_int  : array_2d(accl_range)(MCR_Acc_SIMD_Width-1 downto 0);
  signal dsp_sc_data_write_int           : array_2d(accl_range)(SIMD_Width-1 downto 0);
  signal dsp_sc_acc_data_write_int       : array_2d(accl_range)(MCR_Acc_SIMD_Width-1 downto 0);
  signal vec_write_rd_DSP                : std_logic_vector(accl_range);  -- Indicates whether the result being written is a vector or a scalar
  signal vec_read_rs1_DSP                : std_logic_vector(accl_range);  -- Indicates whether the operand being read is a vector or a scalar
  signal vec_read_rs2_DSP                : std_logic_vector(accl_range);  -- Indicates whether the operand being read is a vector or a scalar
  signal wb_ready                        : std_logic_vector(accl_range);
  signal halt_dsp                        : std_logic_vector(accl_range);
  signal halt_dsp_lat                    : std_logic_vector(accl_range);
  signal recover_state                   : std_logic_vector(accl_range);
  signal recover_state_wires             : std_logic_vector(accl_range);
  signal dsp_data_gnt_i_lat              : std_logic_vector(accl_range);
  signal hdc_except_data_wire            : array_2d(accl_range)(31 downto 0);
  signal decoded_instruction_lat         : array_2d(accl_range)(MCR_UNIT_INSTR_SET_SIZE -1 downto 0);
  signal overflow_rs1_sc                 : array_2d(accl_range)(Addr_Width downto 0);
  signal overflow_rs2_sc                 : array_2d(accl_range)(Addr_Width downto 0);
  signal overflow_rd_sc                  : array_2d(accl_range)(Addr_Width downto 0);
  signal dsp_rs1_to_sc_MCR               : array_2d(accl_range)(SPM_ADDR_WID-1 downto 0);
  signal dsp_rs2_to_sc_MCR               : array_2d(accl_range)(SPM_ADDR_WID-1 downto 0);
  signal dsp_rd_to_sc_MCR                : array_2d(accl_range)(SPM_ADDR_WID-1 downto 0);
  signal dsp_sc_data_read_mask           : array_2d(accl_range)(SIMD_Width-1 downto 0);
  signal RS1_Data_IE_lat                 : array_2d(accl_range)(31 downto 0);
  signal RS2_Data_IE_lat                 : array_2d(accl_range)(31 downto 0);
  signal RD_Data_IE_lat                  : array_2d(accl_range)(Addr_Width -1 downto 0);
  signal HVSIZE_READ_MCR                 : array_2d(accl_range)(Addr_Width downto 0);  -- Bytes remaining to read
  signal HVSIZE_READ_init_MCR            : array_2d(accl_range)(Addr_Width downto 0);  -- Bytes remaining to read
  signal HVSIZE_READ_lat_MCR             : array_2d(accl_range)(Addr_Width downto 0);  -- Bytes remaining to read
  signal HVSIZE_WRITE_MCR                : array_2d(accl_range)(Addr_Width downto 0);  -- Bytes remaining to write
  signal MPSCLFAC_DSP                    : array_2d(accl_range)(4 downto 0);
  signal busy_hdc_internal               : std_logic_vector(accl_range);
  signal busy_hdc_internal_lat           : std_logic_vector(accl_range);
  signal rf_rs2                          : std_logic_vector(accl_range);
  signal SIMD_RD_BYTES_wire_MCR          : array_2d_int(accl_range);
  signal SIMD_RD_BYTES_MCR               : array_2d_int(accl_range);
  
  ------------------ BUNDLING-- ------------------
  signal bundle_en                          : std_logic_vector(accl_range);  -- enables the use of the adders
  signal bundle_en_wire                     : std_logic_vector(accl_range);  -- enables the use of the adders
  signal bundle_en_pending                  : std_logic_vector(accl_range);  -- signal to preserve the request to access the adder "multhithreaded mode" only
  signal bundle_en_pending_wire             : std_logic_vector(accl_range);  -- signal to preserve the request to access the adder "multhithreaded mode" only
  signal busy_bundle                        : std_logic;                     -- busy signal active only when the FU is shared and currently in use 
  signal busy_bundle_wire                   : std_logic;                     -- busy signal active only when the FU is shared and currently in use 
  
  signal bundle_stage_1_en                  : std_logic_vector(accl_range);
  signal bundle_stage_2_en                  : std_logic_vector(accl_range);

  signal hdcu_in_acc_bundle_operand         : array_2d(fu_range)(MCR_Acc_SIMD_Width - 1 downto 0);  -- The first operand comes from the accumulator and has MCR_Acc_SIMD_Width bits
  signal hdcu_in_bundle_operand             : array_2d(fu_range)(SIMD_Width - 1 downto 0);          -- The second operand comes from one standard scratchpad and has SIMD_Width bits
  signal cosine_values                      : array_2d(fu_range)(MCR_Acc_SIMD_Width/2 - 1 downto 0);
  signal sine_values                        : array_2d(fu_range)(MCR_Acc_SIMD_Width/2 - 1 downto 0);
  signal bundling_sum_imag                  : array_2d(fu_range)(MCR_Acc_SIMD_Width/2 - 1 downto 0);
  signal bundling_sum_real                  : array_2d(fu_range)(MCR_Acc_SIMD_Width/2 - 1 downto 0);
  signal hdcu_out_bundle_results            : array_2d(fu_range)(MCR_Acc_SIMD_Width-1 downto 0);
  signal count_real_imag                    : array_2d_int(fu_range); -- Contiene il numero di contatori reali e immaginari per ogni classe
  signal count_real_imag_wire               : array_2d_int(fu_range); -- Contiene il numero di contatori reali e immaginari per ogni classe
  ------------------------------------------------

  ------------------ BINDING ---------------------
  signal busy_bind                        : std_logic;                     -- busy signal active only when the FU is shared and currently in use 
  signal busy_bind_wire                   : std_logic;                     -- busy signal active only when the FU is shared and currently in use 
  signal mul_en                           : std_logic_vector(accl_range);  -- enables the use of the multipliers
  signal bind_en_wire                     : std_logic_vector(accl_range);  -- enables the use of the multipliers
  signal bind_en_pending                  : std_logic_vector(accl_range);  -- signal to preserve the request to access the multiplier "multhithreaded mode" only
  signal bind_en_pending_wire             : std_logic_vector(accl_range);  -- signal to preserve the request to access the multiplier "multhithreaded mode" only

  signal mul_stage_1_en                   : std_logic_vector(accl_range);
  signal mul_stage_2_en                   : std_logic_vector(accl_range);

  signal hdcu_in_bind_operands            : array_3d(fu_range)(1 downto 0)(SIMD_Width-1 downto 0);
  signal dsp_out_mul_results              : array_2d(fu_range)(SIMD_Width-1 downto 0);
  ------------------------------------------------

    ------------------ SIMILARITY ------------------
    constant SIMILARITY_BITS                : natural := 13; -- Number of bits used to represent the similarity: max 2^13 = 8192;
    constant NUM_LEVELS                     : integer := clog2(SIMD_Width/MODULO_BITS); -- Number of levels in the similarity tree, where MODULO_BITS is the number of bits per SIMD element.
    constant MAX_WIDTH                      : integer := Simd_Width;     -- Used for intermediate stage sizing.
        ----------------------------------------------------------------------------
    -- Helper functions to compute the width at each stage.
    ----------------------------------------------------------------------------
    function stage_in_width(level: integer) return integer is
        variable num_in : integer := SIMD_Width/MODULO_BITS / (2 ** level);
    begin
        return num_in * (MODULO_BITS + level);
    end function;
    
    function stage_out_width(level: integer) return integer is
        variable num_in : integer := SIMD_Width/MODULO_BITS/ (2 ** level);
    begin
        return (num_in / 2) * (MODULO_BITS + level + 1);
    end function;

    type stage_array_type is array (natural range <>) of unsigned(MAX_WIDTH-1 downto 0);    
    signal stage_array: stage_array_type(0 to NUM_LEVELS-1) := (others => (others => '0'));
    signal valid_pipe : std_logic_vector(NUM_LEVELS downto 0) := (others => '0');

    signal busy_sim                        : std_logic; 
    signal busy_sim_wire                   : std_logic;
    signal sim_en                          : std_logic_vector(accl_range); 
    signal sim_en_wire                     : std_logic_vector(accl_range);
    signal sim_en_pending                  : std_logic_vector(accl_range); 
    signal sim_en_pending_wire             : std_logic_vector(accl_range);

    signal sim_stage_1_en                  : std_logic_vector(accl_range);
    signal sim_stage_2_en                  : std_logic_vector(accl_range);
    
    signal xor_out                         : array_2d(fu_range)(SIMD_Width - 1 downto 0);
    signal difference1                     : array_2d(fu_range)(SIMD_Width - 1 downto 0);
    signal difference2                     : array_2d(fu_range)(SIMD_Width - 1 downto 0);  
    signal minimum_difference              : array_2d(fu_range)(SIMD_Width - 1 downto 0);
    signal partial_sim_measure             : array_2d(fu_range)(SIMD_Width - 1 downto 0);
    
    signal sim_measure                     : array_2d(fu_range)(SIMD_Width - 1 downto 0);
    signal sim_measure_reg                 : array_2d(fu_range)(SIMD_Width - 1 downto 0);

    signal distance_accumulator            : array_2d(accl_range)(SIMILARITY_BITS - 1 downto 0);
    signal distance_accumulator_wire       : array_2d(accl_range)(SIMILARITY_BITS - 1 downto 0);

    signal hdcu_in_sim_operands            : array_3d(fu_range)(1 downto 0)(SIMD_Width - 1 downto 0);
    signal hdcu_out_sim_results            : array_2d(fu_range)(SIMILARITY_BITS - 1 downto 0);
    signal sim_levels                      : array_2d_int(accl_range); -- Similarity levels for each class
    signal sim_levels_wire                 : array_2d_int(accl_range); -- Similarity levels for each class
        signal sim_levels_search           : array_2d_int(accl_range); -- Similarity levels for each class
    signal sim_levels_search_wire          : array_2d_int(accl_range); -- Similarity levels for each class
    signal zero_pad : std_logic_vector(Data_Width - 2**MODULO_BITS - 1 downto 0) := (others=>'0');

  function log2(x : integer) return integer is
    begin
        return integer(ceil(log2(real(x))));
  end function;
  
  function popcount(x : std_logic_vector) return integer is
    variable count : integer := 0;
    begin
      for i in x'range loop
        if x(i) = '1' then
          count := count + 1;
        end if;
      end loop;
      return count;
  end function;
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
  signal hdcu_in_clip_operand_0          : array_2d(fu_range)(MCR_Acc_SIMD_Width - 1 downto 0);
  -- signal hdcu_in_clip_operand_1          : array_2d(fu_range)(Data_Width - 1 downto 0); 
  signal hdcu_out_clip_results           : array_2d(fu_range)(SIMD_Width - 1 downto 0);

  signal harc_f                          : array_2d_int(accl_range);

  signal max_val        : array_3d_signed (fu_range)((((SIMD*Data_Width)/MODULO_BITS) / 2)  - 1 downto 0)(FP_Width_MCR-1 downto 0); -- Q16.16
  signal max_val_wire   : array_3d_signed (fu_range)((((SIMD*Data_Width)/MODULO_BITS) / 2)  - 1 downto 0)(FP_Width_MCR-1 downto 0); -- Q16.16
  signal max_index      : array_3d_int (fu_range)((((SIMD*Data_Width)/MODULO_BITS) / 2)  - 1 downto 0);
  signal max_index_wire : array_3d_int (fu_range)((((SIMD*Data_Width)/MODULO_BITS) / 2)  - 1 downto 0);
  signal buff           : array_3d(fu_range)((((SIMD*Data_Width)/MODULO_BITS) / 2)  - 1 downto 0)(FP_Width_MCR-1 downto 0); -- store sum per pair
  signal buff_reg       : array_3d(fu_range)((((SIMD*Data_Width)/MODULO_BITS) / 2)  - 1 downto 0)(FP_Width_MCR-1 downto 0); -- store sum per pair
  signal real_in        : array_3d(fu_range)(((SIMD*Data_Width)/MODULO_BITS) / 2  - 1 downto 0)(FP_Width_MCR-1 downto 0);
  signal imag_in        : array_3d(fu_range)(((SIMD*Data_Width)/MODULO_BITS) / 2  - 1 downto 0)(FP_Width_MCR-1 downto 0);
  signal cos_val        : array_3d(fu_range)(((SIMD*Data_Width)/MODULO_BITS) / 2  - 1 downto 0)(FP_Width_MCR-1 downto 0);
  signal cos_val_reg    : array_3d(fu_range)(((SIMD*Data_Width)/MODULO_BITS) / 2  - 1 downto 0)(FP_Width_MCR-1 downto 0);
  signal sin_val        : array_3d(fu_range)(((SIMD*Data_Width)/MODULO_BITS) / 2  - 1 downto 0)(FP_Width_MCR-1 downto 0);
  signal sin_val_reg    : array_3d(fu_range)(((SIMD*Data_Width)/MODULO_BITS) / 2  - 1 downto 0)(FP_Width_MCR-1 downto 0);
  signal real_mult      : array_3d(fu_range)(((SIMD*Data_Width)/MODULO_BITS) / 2  - 1 downto 0)(FP_Width_MCR*2-1 downto 0);
  signal imag_mult      : array_3d(fu_range)(((SIMD*Data_Width)/MODULO_BITS) / 2  - 1 downto 0)(FP_Width_MCR*2-1 downto 0);
  signal real_fixed     : array_3d(fu_range)(((SIMD*Data_Width)/MODULO_BITS) / 2  - 1 downto 0)(FP_Width_MCR-1 downto 0);
  signal imag_fixed     : array_3d(fu_range)(((SIMD*Data_Width)/MODULO_BITS) / 2  - 1 downto 0)(FP_Width_MCR-1 downto 0);

  signal index          : array_2d_index(fu_range);
  signal index_reg      : array_2d_index(fu_range);
  signal index_reg_reg  : array_2d_index(fu_range);
  signal first_index    : array_2d_bool(fu_range);
  signal clip_done      : std_logic_vector(fu_range); -- Indica se la clip è terminata per ogni FU
  signal clip_done_reg  : std_logic_vector(fu_range); -- Indica se la clip è terminata per ogni FU

  -----------------------------------------------------------------

  -------------------------- PERMUTATION --------------------------
  signal busy_perm                       : std_logic;
  signal busy_perm_wire                  : std_logic;
  signal perm_en                         : std_logic_vector(accl_range);
  signal perm_en_wire                    : std_logic_vector(accl_range);
  signal perm_en_pending                 : std_logic_vector(accl_range);
  signal perm_en_pending_wire            : std_logic_vector(accl_range);
  
  signal perm_stage_1_en                 : std_logic_vector(accl_range);
  signal perm_stage_2_en                 : std_logic_vector(accl_range);
  
  signal hdcu_in_perm_operand_0          : array_2d(fu_range)(SIMD_Width - 1 downto 0); -- Contiene ipervettore
  signal hdcu_in_perm_operand_1          : array_2d(fu_range)(Addr_Width - 1 downto 0); -- Contiene immediato
  signal hdcu_out_perm_results           : array_2d(fu_range)(SIMD_Width - 1 downto 0); -- Contiene risultato
  signal buffer_reg                      : array_2d(fu_range)(Data_Width - 1 downto 0); -- Buffer per permutazione, contiene i bit shiftati fuori

  signal first_perm_addr                 : array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
  signal new_perm_offset                 : array_2d_int(accl_range);
  signal shift_amount                    : array_2d_shift_int(fu_range);
  -----------------------------------------------------------------

  -------------------------- ASSOCIATIVE SEARCH --------------------------
  signal busy_as                         : std_logic;
  signal busy_as_wire                    : std_logic;
  signal as_en                           : std_logic_vector(accl_range);
  signal as_en_wire                      : std_logic_vector(accl_range);
  signal as_en_pending                   : std_logic_vector(accl_range);
  signal as_en_pending_wire              : std_logic_vector(accl_range);
  
  signal as_stage_1_en                   : std_logic_vector(accl_range);
  signal as_stage_2_en                   : std_logic_vector(accl_range);
  signal as_stage_3_en                   : std_logic_vector(accl_range);

  signal sim_class                       : array_2d_int_sim(accl_range); -- Similarity per classe
  signal temp_best_sim_class             : array_2d_int_sim(accl_range);                  -- Migliore similarità per classe temporanea

  signal class_index                     : array_2d_shift_int(accl_range); -- Indice della classe che sto processando
  signal temp_best_class_index           : array_2d_shift_int(accl_range); -- Indice della classe con migliore similarità

  signal fake_HVSIZE_READ_MCR            : array_2d(accl_range)(Addr_Width downto 0);  
  signal fake_HVSIZE_READ_2_MCR          : array_2d(accl_range)(Addr_Width downto 0);  
  
  signal head_ptr_encoded_hv             : array_2d(accl_range)(31 downto 0); -- Puntatore alla testa dell'encoded vector. Serve per ricaricare lo stesso encoded vector.
  -------------------------------------------------------------------------

  -------------------------- PERFORMANCE COUNTERS --------------------------
  signal hdcu_performance_counter_int  : array_2d(accl_range)(31 downto 0); 
  signal hdcu_bundle_perf_counter_int  : array_2d(accl_range)(31 downto 0); 
  signal hdcu_bind_perf_counter_int    : array_2d(accl_range)(31 downto 0); 
  signal hdcu_sim_perf_counter_int     : array_2d(accl_range)(31 downto 0); 
  signal hdcu_clip_perf_counter_int    : array_2d(accl_range)(31 downto 0); 
  signal hdcu_perm_perf_counter_int    : array_2d(accl_range)(31 downto 0); 
  signal hdcu_search_perf_counter_int  : array_2d(accl_range)(31 downto 0); 

---------------------------------- MCR ARCHITECTURE BEGIN -------------------------------------------

begin
  busy_hdc <= busy_hdc_internal;

  DSP_replicated : for h in accl_range generate
    harc_f(h) <= 0 when multithreaded_accl_en = 1 else h;
  
  ----------------------------- Sequential Stage of MCR Unit ----------------------------------------

  DSP_Exec_Unit : process(clk_i, rst_ni)  -- single cycle unit, fully synchronous 
   variable idx: integer;
  begin
    if rst_ni = '0' then
      rf_rs2(h)     <= '0';
      recover_state(h) <= '0';
      
    elsif rising_edge(clk_i) then
      
      HVSIZE_READ_lat_MCR(h) <= HVSIZE_READ_MCR(h);
      sim_levels(h)  <= sim_levels_wire(h);
      if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then  

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

            if (decoded_instruction_MCR(KSVADDRF_bit_position  ) = '1' or 
               decoded_instruction_MCR(KSVMULRF_bit_position) = '1' )  then
              rf_rs2(h) <= '1';
            else
              rf_rs2(h) <= '0';  
            end if;

            -- We backup data from decode stage since they will get updated
            -- HVSIZE_READ_MASK(h) <= HVSIZE(harc_EXEC);
            MPSCLFAC_DSP(h) <= MPSCLFAC(harc_EXEC); -- Contiene il numero di classi
            class_index(h) <= 0; -- Indice della classe che sto processando
            
            -- Uso un registro che inizializzo a HVSIZE e lo decremento di SIMD_RD_BYTES_wire(h). Quando arriva a 0, incremento l'indice della classe
            -- lo inizializzo nuoamente a HVSIZE. 
            fake_HVSIZE_READ_MCR(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)), HVSIZE_READ_MCR(h)'length));
            fake_HVSIZE_READ_2_MCR(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) + 4*(NUM_LEVELS), HVSIZE_READ_MCR(h)'length));

            -- When the decoded instruction is a bundle, we need to multiply the HVSIZE_WRITE by (Data_Width/COUNTERS_NUMBER)
             if decoded_instruction_MCR(KADDV_bit_position)    = '1' then
            --   -- Per il bundling devo moltiplicare 2, perché ho parte immagianria e reale
               HVSIZE_WRITE_MCR(h) <= std_logic_vector(resize(unsigned(HVSIZE(h)) * 2, HVSIZE_WRITE_MCR(h)'length));
            elsif (decoded_instruction_MCR(KSUBV_bit_position)    = '1' or
                   decoded_instruction_MCR(KDOTP_bit_position)    = '1') then
              HVSIZE_WRITE_MCR(h) <= std_logic_vector(to_unsigned(4, HVSIZE_WRITE_MCR(h)'length));
            else
              HVSIZE_WRITE_MCR(h) <= HVSIZE(harc_EXEC);
            end if;

            decoded_instruction_lat(h)  <= decoded_instruction_MCR;
            
            vec_write_rd_DSP(h) <= vec_write_rd_ID;
            vec_read_rs1_DSP(h) <= vec_read_rs1_ID;
            vec_read_rs2_DSP(h) <= vec_read_rs2_ID;
          
            dsp_rs1_to_sc_MCR(h) <= rs1_to_sc;
            dsp_rs2_to_sc_MCR(h) <= rs2_to_sc;
            dsp_rd_to_sc_MCR(h)  <= rd_to_sc;

            if (decoded_instruction_MCR(KSVMULRF_bit_position) = '1') then
              RD_Data_IE_lat(h)  <= std_logic_vector(unsigned(RD_Data_IE) + resize((SIMD_RD_BYTES_wire_MCR(h)) * (unsigned(RS2_Data_IE)), RD_Data_IE_lat(h)'length)); -- source 1 address increment
            else
              RD_Data_IE_lat(h) <= RD_Data_IE;
            end if;

            
            -- Salvo l'indirizzo in cui viene salvato il primo risultato
            if (decoded_instruction_MCR(KSVMULRF_bit_position) = '1') then
              first_perm_addr(h) <= RD_Data_IE;
            end if;

            -- Increment the read addresses if there is a data grant
            if dsp_data_gnt_i(h) = '1' then
              
              ------------------ Source Register 1 ------------------
              if vec_read_rs1_ID = '1'  then

                if decoded_instruction_MCR(KDOTP_bit_position) = '1' and to_integer(unsigned(HVSIZE(h))) <= SIMD_RD_BYTES_wire_MCR(h) then
                  RS1_Data_IE_lat(h) <= RS1_Data_IE;
                elsif decoded_instruction_MCR(KADDV_bit_position) = '1' then
                  RS1_Data_IE_lat(h) <= std_logic_vector(unsigned(RS1_Data_IE) + SIMD_RD_BYTES_wire_MCR(h)/2*Data_Width/MODULO_BITS); -- source 1 address increment
                elsif decoded_instruction_MCR(KSVADDRF_bit_position) = '1' then
                  RS1_Data_IE_lat(h)  <= RS1_Data_IE;
                else
                  RS1_Data_IE_lat(h) <= std_logic_vector(unsigned(RS1_Data_IE) + SIMD_RD_BYTES_wire_MCR(h));  -- source 1 address increment
                end if;

              else
                RS1_Data_IE_lat(h) <= RS1_Data_IE;
              end if;
              -------------------------------------------------------

              ------------------ Source Register 2 ------------------
              if vec_read_rs2_ID = '1' and decoded_instruction_MCR(KADDV_bit_position) = '0' then
                if decoded_instruction_MCR(KDOTP_bit_position) = '1' and to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire_MCR(h) then
                  RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE) + unsigned(HVSIZE(h)));
                  
                else
                  RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE) + SIMD_RD_BYTES_wire_MCR(h)); 
                end if;
              else
                RS2_Data_IE_lat(h) <= RS2_Data_IE;
              end if;
              -------------------------------------------------------

              -- Decrement the vector elements that have already been operated on
              --   if  (decoded_instruction(KADDV_bit_position)  = '1' or        
              if decoded_instruction_MCR(KDOTP_bit_position)  = '1' then
                -- Il numero di classi è contenuto nel CSR nel segnale MPSCLFAC, quindi la dimensione della memoria associativa è MPSCLFAC * HVSIZE
                HVSIZE_READ_MCR(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * unsigned(MPSCLFAC(harc_EXEC)), HVSIZE_READ_MCR(h)'length)); 
                -- Salvo l'indirizzo dell'encoded vector perche devo ripresentarlo ogni volta che ho finito di calcolare la similarità con un class vector
                head_ptr_encoded_hv(h) <= RS1_Data_IE;
              elsif (decoded_instruction_MCR(KADDV_bit_position)  = '1' or 
                     decoded_instruction_MCR(KSVADDRF_bit_position)  = '1') then
                  HVSIZE_READ_MCR(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * 2, HVSIZE_READ_MCR(h)'length));
                  HVSIZE_READ_init_MCR(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC))*2, HVSIZE_READ_MCR(h)'length));
              else
                if decoded_instruction_MCR(KSVADDRF_bit_position)  = '1' then
                  HVSIZE_READ_MCR(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * 2, HVSIZE_READ_MCR(h)'length));
                else 
                  if unsigned(HVSIZE(harc_EXEC)) >= SIMD_RD_BYTES_wire_MCR(h) then
                    HVSIZE_READ_MCR(h) <= std_logic_vector(unsigned(HVSIZE(harc_EXEC)) - SIMD_RD_BYTES_wire_MCR(h));       -- decrement by SIMD_BYTE Execution Capability            
                  else
                    HVSIZE_READ_MCR(h) <= (others => '0');                                                             -- decrement the remaining bytes
                  end if;
                end if;
              end if;

            -- If there is no data grant, we keep the addresses the same
            else

              RS1_Data_IE_lat(h) <= RS1_Data_IE;
              RS2_Data_IE_lat(h) <= RS2_Data_IE;

              -- When the decoded instruction is a bundle/clipping, we need to multiply the HVSIZE_READ by Data_Width/COUNTERS_NUMBER
            --   if decoded_instruction(KADDV_bit_position)  = '1' or
              if decoded_instruction_MCR(KDOTP_bit_position)  = '1' then
                -- Il numero di classi è contenuto nel CSR nel segnale MPSCLFAC, quindi la dimensione della memoria associativa è MPSCLFAC * HVSIZE
                HVSIZE_READ_MCR(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * unsigned(MPSCLFAC(harc_EXEC)), HVSIZE_READ_MCR(h)'length)); 
                -- Salvo l'indirizzo dell'encoded vector perche devo ripresentarlo ogni volta che ho finito di calcolare la similarità con un class vector
                head_ptr_encoded_hv(h) <= RS1_Data_IE;
              else
                HVSIZE_READ_MCR(h) <= HVSIZE(harc_EXEC);
              end if;  

            end if;

           ---------------------------------------------------------------------------

          when hdc_exec =>
            recover_state(h) <= recover_state_wires(h);
            if halt_dsp(h) = '1' and halt_dsp_lat(h) = '0' then
              dsp_sc_data_write_int(h) <= hdc_sc_data_write_wire_int(h);
              dsp_sc_acc_data_write_int(h) <= dsp_sc_acc_data_write_wire_int(h);
            end if;

            --------------------------------------------------------------------------
            --  ██╗  ██╗██╗    ██╗      ██╗      ██████╗  ██████╗ ██████╗ ███████╗  --
            --  ██║  ██║██║    ██║      ██║     ██╔═══██╗██╔═══██╗██╔══██╗██╔════╝  --
            --  ███████║██║ █╗ ██║█████╗██║     ██║   ██║██║   ██║██████╔╝███████╗  --
            --  ██╔══██║██║███╗██║╚════╝██║     ██║   ██║██║   ██║██╔═══╝ ╚════██║  --
            --  ██║  ██║╚███╔███╔╝      ███████╗╚██████╔╝╚██████╔╝██║     ███████║  --
            --  ╚═╝  ╚═╝ ╚══╝╚══╝       ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝     ╚══════╝  --            
            --------------------------------------------------------------------------

            if halt_dsp(h) = '0' then

              ------------------------------- Increment the write address when we have a result as a vector -------------------------------

              if vec_write_rd_DSP(h) = '1' and wb_ready(h) = '1' then

                -- Bundling
                if decoded_instruction_MCR(KADDV_bit_position) = '1' then
                  RD_Data_IE_lat(h)  <= std_logic_vector(unsigned(RD_Data_IE_lat(h)) + SIMD_RD_BYTES_wire_MCR(h)/2*Data_Width/MODULO_BITS); -- destination address increment
                
                  -- Permutazione
                elsif (decoded_instruction_MCR(KSVMULRF_bit_position) = '1') then
                  -- When we reach the last hdcu_in_perm_operand_1 chunk, we reset the destination address to the original one  
                  if (HVSIZE_READ_lat_MCR(h) = std_logic_vector(resize((SIMD_RD_BYTES_wire_MCR(h)) * unsigned(hdcu_in_perm_operand_1(h)), HVSIZE_READ_MCR(h)'length))) then
                    RD_Data_IE_lat(h)  <= RD_Data_IE; 
                  else
                    RD_Data_IE_lat(h)  <= std_logic_vector(unsigned(RD_Data_IE_lat(h)) + SIMD_RD_BYTES_wire_MCR(h)); -- destination address increment
                  end if;

                else
                  RD_Data_IE_lat(h)  <= std_logic_vector(unsigned(RD_Data_IE_lat(h)) + SIMD_RD_BYTES_wire_MCR(h)); -- destination address increment
                end if;
                
                
              end if;
              
              ------------------------------- Decrement the number of bytes to write -------------------------------
              if wb_ready(h) = '1' then
                
                if to_integer(unsigned(HVSIZE_WRITE_MCR(h))) >= SIMD_RD_BYTES_wire_MCR(h) then
                  HVSIZE_WRITE_MCR(h) <= std_logic_vector(unsigned(HVSIZE_WRITE_MCR(h)) - SIMD_RD_BYTES_wire_MCR(h));    -- decrement by SIMD_BYTE Execution Capability 
                else
                  HVSIZE_WRITE_MCR(h) <= (others => '0');                                                        -- decrement the remaining bytes
                end if;
                
              end if;
              
              ------------------------------- Increment the read addresses -------------------------------

              if to_integer(unsigned(HVSIZE_READ_MCR(h))) >= SIMD_RD_BYTES_wire_MCR(h) and dsp_data_gnt_i(h) = '1' then -- Increment the addresses untill all the vector elements are operated fetched
                
                ----------------------------------------- Source Register 1 -----------------------------------------
                if vec_read_rs1_DSP(h) = '1' then
                  
                  if decoded_instruction_MCR(KDOTP_bit_position) = '1' then

                 
                    -- Incremento l'indice della classe ogni volta che ho finito di leggere un class vector
                      if to_integer(unsigned(fake_HVSIZE_READ_MCR(h))) = 0 then 
                        class_index(h) <= (class_index(h) + 1);
                      end if;

                      -- Se ho quasi finito di leggere un class vector (fake_HVSIZE_READ = 2*SIMD_RD_BYTES_wire) e 
                      -- ho ancora piu di un class vector da leggere (HVSIZE_READ > HVSIZE) o
                      -- sto leggendo un class vector intero a ciclo di clock (HVSIZE = SIMD_RD_BYTES_wire) allora assegno a RS1 l'indirizzo dell'encoded vector
                      if (to_integer(unsigned(fake_HVSIZE_READ_MCR(h))) = 2*SIMD_RD_BYTES_wire_MCR(h)  and 
                          to_integer(unsigned(HVSIZE_READ_MCR(h))) > to_integer(unsigned(HVSIZE(h)))) or
                          to_integer(unsigned(HVSIZE(h))) = SIMD_RD_BYTES_wire_MCR(h) then 
                        RS1_Data_IE_lat(h) <= head_ptr_encoded_hv(h);
                      
                      -- Se il valore a cui si resetta fake_HVSIZE_READ è minore = SIMD_RD_BYTES_wire(h) allora resetto RS1 a head_ptr_encoded_hv(h)

                      elsif to_integer(unsigned(HVSIZE(h)) - SIMD_RD_BYTES_wire_MCR(h)) = SIMD_RD_BYTES_wire_MCR(h) and
                            to_integer(unsigned(fake_HVSIZE_READ_MCR(h))) = 0 then
                        RS1_Data_IE_lat(h) <= head_ptr_encoded_hv(h);
                      
                      -- La condizione di default è incrementare RS1 di SIMD_RD_BYTES_wire(h)
                      else
                        RS1_Data_IE_lat(h) <= std_logic_vector(unsigned(RS1_Data_IE_lat(h)) + SIMD_RD_BYTES_wire_MCR(h)); -- source 1 address increment
                      end if;

                  -- Bundling
                  elsif decoded_instruction_MCR(KADDV_bit_position) = '1' then
                    RS1_Data_IE_lat(h) <= std_logic_vector(unsigned(RS1_Data_IE_lat(h)) + SIMD_RD_BYTES_wire_MCR(h)/2*Data_Width/MODULO_BITS); -- NOTE! if using fp 32 bit, remove the /2 factor on all the occurrences of SIMD_RD_BYTES_wire(h)*Data_Width/MODULO_BITS in this file
                      
                    
                  -- Clipping
                  elsif decoded_instruction_MCR(KSVADDRF_bit_position) = '1' then
                    if index(h) = (2**MODULO_BITS-1)-1 then -- -2 takes into account the memory access latency
                      RS1_Data_IE_lat(h)  <= std_logic_vector(unsigned(RS1_Data_IE_lat(h)) + SIMD_RD_BYTES_wire_MCR(h)/2*Data_Width/MODULO_BITS); -- destination address increment
                    end if;
                  else -- Se non sto eseguendo un KDOTP incremento RS1 normalmente
                    RS1_Data_IE_lat(h) <= std_logic_vector(unsigned(RS1_Data_IE_lat(h)) + SIMD_RD_BYTES_wire_MCR(h)); -- source 1 address increment
                  end if; -- Chiude KDOTP_bit_position
                  
                end if; -- Chiude vec_read_rs1_DSP(h) = '1'
                
                ----------------------------------------- Source Register 2 -----------------------------------------
                if vec_read_rs2_DSP(h) = '1' then
                  
                  if decoded_instruction_MCR(KDOTP_bit_position) = '1'  and to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire_MCR(h) then
                  -- Se ho una search e HVSIZE_READ è minore di SIMD_RD_BYTES_wire(h) incremento RS2 di HVSIZE e non di SIMD_RD_BYTES_wire(h)
                  -- cosi leggo un class vector ogni ciclo di clock
                    RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + unsigned(HVSIZE(h))); -- source 2 address increment

                  elsif decoded_instruction_MCR(KADDV_bit_position) = '1' then
                    idx := (to_integer(unsigned(HVSIZE_READ_init_MCR(h))) - to_integer(unsigned(HVSIZE_READ_MCR(h)))) / SIMD_RD_BYTES_wire_MCR(h);

                    if (to_unsigned(idx, 1)(0) = '0') and (HVSIZE_READ_MCR(h) /= (Addr_Width downto 0 => 'U') ) and ((to_integer(unsigned(HVSIZE_READ_init_MCR(h))) - to_integer(unsigned(HVSIZE_READ_MCR(h)))) / SIMD_RD_BYTES_wire_MCR(h)) >= 0
                          then
                          RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + SIMD_RD_BYTES_wire_MCR(h)); -- source 2 address increment
                      end if;

                  else

                    RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + SIMD_RD_BYTES_wire_MCR(h)); -- source 2 address increment

                  end if;

                end if;
              
              else -- se HVSIZE_READ è minore di SIMD_RD_BYTES_wire(h) incremento RS2 di HVSIZE e non di SIMD_RD_BYTES_wire(h)

                if decoded_instruction_MCR(KDOTP_bit_position) = '1'  and to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire_MCR(h) then

                  -- Incremento l'indice della classe ogni volta che ho finito di leggere un class vector
                  if to_integer(unsigned(fake_HVSIZE_READ_MCR(h))) = 0 then 
                    class_index(h) <= (class_index(h) + 1);
                  end if;

                  RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + unsigned(HVSIZE(h))); -- source 2 address increment

                end if;

              end if;

              -------------------------- Decrement the vector elements that have already been operated on ---------------------------------------
              if dsp_data_gnt_i(h) = '1' then
                
                if to_integer(unsigned(HVSIZE_READ_MCR(h))) >= SIMD_RD_BYTES_wire_MCR(h)   then
                  
                  if (decoded_instruction_MCR(KSVADDRF_bit_position) = '0' or index(h) = 2**MODULO_BITS-1 ) then
                    HVSIZE_READ_MCR(h) <= std_logic_vector(unsigned(HVSIZE_READ_MCR(h)) - SIMD_RD_BYTES_wire_MCR(h)); -- decrement by SIMD_BYTE Execution Capability
                  end if;

                  if decoded_instruction_MCR(KDOTP_bit_position) = '1' then

                    -- Se la dimensione dell'ipervettore è maggiore della capacità di lettura 
                    if to_integer(unsigned(fake_HVSIZE_READ_MCR(h))) > SIMD_RD_BYTES_wire_MCR(h) then

                      -- Decremento di SIMD_RD_BYTES_wire(h) 
                      fake_HVSIZE_READ_MCR(h) <= std_logic_vector(unsigned(fake_HVSIZE_READ_MCR(h)) - SIMD_RD_BYTES_wire_MCR(h));

                    -- Se ho finito di leggere un class vector allora lo resetto
                    elsif to_integer(unsigned(fake_HVSIZE_READ_MCR(h))) = 0 then
                      fake_HVSIZE_READ_MCR(h) <= std_logic_vector(resize(unsigned(HVSIZE(h)) - SIMD_RD_BYTES_wire_MCR(h), HVSIZE_READ_MCR(h)'length));
                    
                    -- Di default lo lascio a 0
                    else
                      fake_HVSIZE_READ_MCR(h) <= (others => '0');
                      --HVSIZE_READ(h) <= std_logic_vector(unsigned(HVSIZE_READ(h)) - unsigned(HVSIZE(h))); 
                    end if;

                    -- Se la dimensione dell'ipervettore è maggiore della capacità di lettura 
                    if to_integer(unsigned(fake_HVSIZE_READ_2_MCR(h))) > SIMD_RD_BYTES_wire_MCR(h) then

                      -- Decremento di SIMD_RD_BYTES_wire(h) 
                      fake_HVSIZE_READ_2_MCR(h) <= std_logic_vector(unsigned(fake_HVSIZE_READ_2_MCR(h)) - SIMD_RD_BYTES_wire_MCR(h));

                    -- Se ho finito di leggere un class vector allora lo resetto
                    elsif to_integer(unsigned(fake_HVSIZE_READ_2_MCR(h))) = 0 then
                      fake_HVSIZE_READ_2_MCR(h) <= std_logic_vector(resize(unsigned(HVSIZE(h)) - SIMD_RD_BYTES_wire_MCR(h), HVSIZE_READ_MCR(h)'length));
                    
                    -- Di default lo lascio a 0
                    else
                      fake_HVSIZE_READ_2_MCR(h) <= (others => '0');
                      --HVSIZE_READ(h) <= std_logic_vector(unsigned(HVSIZE_READ(h)) - unsigned(HVSIZE(h))); 
                    end if;

                  end if;

                else
                  -- Qui vado a coprire il caso in cui la memoria associativa è minore della capacità di lettura
                  -- Quindi significa che potrei leggere piu di un class vector a ciclo di clock
                  -- Quando questo accade dobbiamo tenere fake_HVSIZE_READ(h) a 0 per leggere sempre l'indirizzo dell'encoded vector
                  -- e decrementare HVSIZE_READ(h) di HVSIZE(h) .
                  if decoded_instruction_MCR(KDOTP_bit_position) = '1' then
                    HVSIZE_READ_MCR(h) <= std_logic_vector(unsigned(HVSIZE_READ_MCR(h)) - unsigned(HVSIZE(h)));
                    fake_HVSIZE_READ_MCR(h) <= (others => '0'); 
                  else
                    HVSIZE_READ_MCR(h) <= (others => '0');                                                    -- decrement the remaining bytes
                  end if;

                end if;

              end if;
              
              dsp_sc_data_read_mask(h) <= (others => '0');
            
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
        hdcu_perm_perf_counter_int(h)    <= std_logic_vector(to_unsigned(0, hdcu_perm_perf_counter_int(h)'length));
        hdcu_search_perf_counter_int(h)  <= std_logic_vector(to_unsigned(0, hdcu_search_perf_counter_int(h)'length));

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
        elsif perm_en_wire(h) = '1' then
          hdcu_perm_perf_counter_int(h)    <= std_logic_vector(to_unsigned(0, hdcu_perm_perf_counter_int(h)'length));
        elsif as_en_wire(h) = '1' then
          hdcu_search_perf_counter_int(h)  <= std_logic_vector(to_unsigned(0, hdcu_search_perf_counter_int(h)'length));
        end if;  
        
        -- Counter for the HDCU
        if busy_hdc(h) = '1' then
          hdcu_performance_counter_int(h) <= std_logic_vector(unsigned(hdcu_performance_counter_int(h)) + 1);
        end if;

        -- Counter for each of the FU
        if bundle_en(h) = '1' then
          hdcu_bundle_perf_counter_int(h) <= std_logic_vector(unsigned(hdcu_bundle_perf_counter_int(h)) + 1);
        elsif mul_en(h) = '1' then
          hdcu_bind_perf_counter_int(h) <= std_logic_vector(unsigned(hdcu_bind_perf_counter_int(h)) + 1);
        elsif sim_en(h) = '1' and as_en(h) = '0' then
          hdcu_sim_perf_counter_int(h) <= std_logic_vector(unsigned(hdcu_sim_perf_counter_int(h)) + 1);
        elsif clip_en(h) = '1' then
          hdcu_clip_perf_counter_int(h) <= std_logic_vector(unsigned(hdcu_clip_perf_counter_int(h)) + 1);
        elsif perm_en(h) = '1' then
          hdcu_perm_perf_counter_int(h) <= std_logic_vector(unsigned(hdcu_perm_perf_counter_int(h)) + 1);
        elsif as_en(h) = '1' then
          hdcu_search_perf_counter_int(h) <= std_logic_vector(unsigned(hdcu_search_perf_counter_int(h)) + 1);
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
        hdcu_search_perf_counter(h)  <= (others => '0');
      else
        hdcu_performance_counter(h)  <= hdcu_performance_counter_int(h);
        hdcu_bundle_perf_counter(h)  <= hdcu_bundle_perf_counter_int(h);
        hdcu_bind_perf_counter(h)    <= hdcu_bind_perf_counter_int(h);
        hdcu_sim_perf_counter(h)     <= hdcu_sim_perf_counter_int(h);
        hdcu_clip_perf_counter(h)    <= hdcu_clip_perf_counter_int(h);
        hdcu_perm_perf_counter(h)    <= hdcu_perm_perf_counter_int(h);
        hdcu_search_perf_counter(h)  <= hdcu_search_perf_counter_int(h);
      end if;

    end process;
  end generate gen_perf_counter;

  ------------ Combinational Stage of MCR Unit ----------------------------------------------------------------------
  DSP_Excpt_Cntrl_Unit_comb : process(all)
  
  variable busy_hdc_internal_wires : std_logic;
  variable hdc_except_condition_wires : std_logic_vector(harc_range);
  variable hdc_taken_branch_MCR_wires : std_logic_vector(harc_range);  
      
  begin

    busy_hdc_internal_wires        := '0';
    hdc_except_condition_wires(h)  := '0';
    hdc_taken_branch_MCR_wires(h)  := '0';
    wb_ready(h)                    <= '0';
    halt_dsp(h)                    <= '0';
    nextstate_DSP(h)               <= hdc_init;
    recover_state_wires(h)         <= recover_state(h);
    hdc_except_data_wire(h)        <= hdc_except_data(h);
    overflow_rs1_sc(h)             <= (others => '0');
    overflow_rs2_sc(h)             <= (others => '0');
    overflow_rd_sc(h)              <= (others => '0');
    hdc_we_word_MCR(h)                 <= (others => '0');
    dsp_acc_we_word_MCR(h)             <= (others => '0');
    hdc_sci_req_MCR(h)                 <= (others => '0');
    hdc_sci_we_MCR(h)                  <= (others => '0');
    hdc_sc_write_addr_MCR(h)           <= (others => '0');
    hdc_sc_read_addr_MCR(h)            <= (others => (others => '0'));
    hdc_to_sc_MCR(h)                   <= (others => (others => '0'));
    sim_levels_wire(h)                 <= 0;
    if HVSIZE_READ_lat_MCR(h) = (0 to Addr_Width => '0') and sim_en(h)='1' then
        sim_levels_wire(h) <= sim_levels(h) + 1;
     end if;

    if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then
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
            hdc_taken_branch_MCR_wires(h) := '1';    
            hdc_except_data_wire(h) <= ILLEGAL_VECTOR_SIZE_EXCEPT_CODE;
          elsif HVSIZE(harc_EXEC)(0) /= '0' and MVTYPE(harc_EXEC)(3 downto 2) = "01" then            -- Set exception if the number of bytes are not divisible by two
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_MCR_wires(h) := '1';
            hdc_except_data_wire(h) <= ILLEGAL_VECTOR_SIZE_EXCEPT_CODE;
          elsif (rs1_to_sc  = "100" and vec_read_rs1_ID = '1') or
            (rs2_to_sc  = "100" and vec_read_rs2_ID = '1') or
             rd_to_sc   = "100" then     -- Set exception for non scratchpad access
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_MCR_wires(h) := '1';    
            hdc_except_data_wire(h) <= ILLEGAL_ADDRESS_EXCEPT_CODE;
          elsif rs1_to_sc = rs2_to_sc and vec_read_rs1_ID = '1' and vec_read_rs2_ID = '1' then               -- Set exception for same read access
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_MCR_wires(h) := '1';    
            hdc_except_data_wire(h) <= READ_SAME_SCARTCHPAD_EXCEPT_CODE;    
          elsif (overflow_rs1_sc(h)(Addr_Width) = '1' and vec_read_rs1_ID = '1') or (overflow_rs2_sc(h)(Addr_Width) = '1' and  vec_read_rs2_ID = '1') then -- Set exception if reading overflows the scratchpad's address
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_MCR_wires(h) := '1';    
            hdc_except_data_wire(h) <= SCRATCHPAD_OVERFLOW_EXCEPT_CODE;
          elsif overflow_rd_sc(h)(Addr_Width) = '1'  and vec_write_rd_ID = '1' then           -- Set exception if reading overflows the scratchpad's address, scalar writes are excluded
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_MCR_wires(h) := '1';    
            hdc_except_data_wire(h) <= SCRATCHPAD_OVERFLOW_EXCEPT_CODE;
          else
            if halt_hart(h) = '0' then
              nextstate_DSP(h) <= hdc_exec;
            else
              nextstate_DSP(h) <= hdc_halt_hart;
            end if;
            busy_hdc_internal_wires := '1';
          end if;

          if rs1_to_sc /= "100" and spm_rs1 = '1' and halt_hart(h) = '0' then
            hdc_sci_req_MCR(h)(to_integer(unsigned(rs1_to_sc))) <= '1';
            hdc_to_sc_MCR(h)(to_integer(unsigned(rs1_to_sc)))(0) <= '1';
            hdc_sc_read_addr_MCR(h)(0) <= RS1_Data_IE(Addr_Width-1 downto 0);
          end if;
          if rs2_to_sc /= "100" and spm_rs2 = '1' and rs1_to_sc /= rs2_to_sc and halt_hart(h) = '0' then   -- Do not send a read request if the second operand accesses the same spm as the first, 
            hdc_sci_req_MCR(h)(to_integer(unsigned(rs2_to_sc))) <= '1';
            hdc_to_sc_MCR(h)(to_integer(unsigned(rs2_to_sc)))(1) <= '1';
            hdc_sc_read_addr_MCR(h)(1) <= RS2_Data_IE(Addr_Width-1 downto 0);
          end if;
        
         when hdc_halt_hart =>

           if halt_hart(h) = '0' then
             nextstate_DSP(h) <= hdc_exec;
           else
             nextstate_DSP(h) <= hdc_halt_hart;
           end if;
           busy_hdc_internal_wires := '1';

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

           if (dsp_sci_wr_gnt(h) = '0' and wb_ready(h) = '1') then
             halt_dsp(h) <= '1';
             recover_state_wires(h) <= '1';
           elsif unsigned(HVSIZE_WRITE_MCR(h)) <= SIMD_RD_BYTES_MCR(h) then
             recover_state_wires(h) <= '0';
           end if;

           if vec_write_rd_DSP(h) = '1' and  hdc_sci_we_MCR(h)(to_integer(unsigned(dsp_rd_to_sc_MCR(h)))) = '1' then
             if ((unsigned(HVSIZE_WRITE_MCR(h)) >= (SIMD)*4+1) or (perm_stage_2_en(h)='1')) then  -- 
               hdc_we_word_MCR(h) <= (others => '1');
             elsif  unsigned(HVSIZE_WRITE_MCR(h)) >= 1 then
               for i in 0 to SIMD-1 loop
                 if i <= to_integer(unsigned(HVSIZE_WRITE_MCR(h))-1)/4 then -- Four because of the number of bytes per word
                   if to_integer(unsigned(hdc_sc_write_addr_MCR(h)(SIMD_BITS+1 downto 0))/4 + i) < SIMD then
                     hdc_we_word_MCR(h)(to_integer(unsigned(hdc_sc_write_addr_MCR(h)(SIMD_BITS+1 downto 0))/4 + i)) <= '1';
                   elsif to_integer(unsigned(hdc_sc_write_addr_MCR(h)(SIMD_BITS+1 downto 0))/4 + i) >= SIMD then
                     hdc_we_word_MCR(h)(to_integer(unsigned(hdc_sc_write_addr_MCR(h)(SIMD_BITS+1 downto 0))/4 + i - SIMD)) <= '1';
                   end if;  
                 end if;
               end loop;
             end if;
           elsif vec_write_rd_DSP(h) = '0' and  hdc_sci_we_MCR(h)(to_integer(unsigned(dsp_rd_to_sc_MCR(h)))) = '1' then
             hdc_we_word_MCR(h)(to_integer(unsigned(hdc_sc_write_addr_MCR(h)(SIMD_BITS+1 downto 0))/4)) <= '1';
           end if;

           if vec_write_rd_DSP(h) = '1' and  hdc_sci_we_MCR(h)(to_integer(unsigned(dsp_rd_to_sc_MCR(h)))) = '1' then
             if (unsigned(HVSIZE_WRITE_MCR(h)) >= ((MCR_Acc_SIMD_Width/32))*4+1) then  -- 
               dsp_acc_we_word_MCR(h) <= (others => '1');
             elsif  unsigned(HVSIZE_WRITE_MCR(h)) >= 1 then
               for i in 0 to (MCR_Acc_SIMD_Width/32)-1 loop
                 if i <= to_integer(unsigned(HVSIZE_WRITE_MCR(h))*MODULO_BITS-1)/4 then -- Four because of the number of bytes per word
                   if to_integer(unsigned(hdc_sc_write_addr_MCR(h)(ACC_SIMD_BITS+1 downto 0))/4 + i) < (MCR_Acc_SIMD_Width/32) then
                     dsp_acc_we_word_MCR(h)(to_integer(unsigned(hdc_sc_write_addr_MCR(h)(ACC_SIMD_BITS+1 downto 0))/4 + i)) <= '1';
                   elsif to_integer(unsigned(hdc_sc_write_addr_MCR(h)(ACC_SIMD_BITS+1 downto 0))/4 + i) >= (MCR_Acc_SIMD_Width/32) then
                     dsp_acc_we_word_MCR(h)(to_integer(unsigned(hdc_sc_write_addr_MCR(h)(ACC_SIMD_BITS+1 downto 0))/4 + i - (MCR_Acc_SIMD_Width/32))) <= '1';
                   end if;
                 end if;
               end loop;
             end if;
           elsif vec_write_rd_DSP(h) = '0' and  hdc_sci_we_MCR(h)(to_integer(unsigned(dsp_rd_to_sc_MCR(h)))) = '1' then
             dsp_acc_we_word_MCR(h)(to_integer(unsigned(hdc_sc_write_addr_MCR(h)(ACC_SIMD_BITS+1 downto 0))/4)) <= '1';
           end if;



           -------------------------------------------------------------------------------------------------------------------------

           --------------------------- BUNDLING ---------------------------------
           if decoded_instruction_lat(h)(KADDV_bit_position)  = '1' then
             if bundle_stage_2_en(h) = '1' then 
               wb_ready(h) <= '1';
             elsif recover_state(h) = '1' then
               wb_ready(h) <= '1';  
             end if;
             if HVSIZE_READ_MCR(h) > (0 to Addr_Width => '0') then
               hdc_to_sc_MCR(h)(to_integer(unsigned(dsp_rs1_to_sc_MCR(h))))(0) <= '1';
               hdc_to_sc_MCR(h)(to_integer(unsigned(dsp_rs2_to_sc_MCR(h))))(1) <= '1';
               hdc_sci_req_MCR(h)(to_integer(unsigned(dsp_rs1_to_sc_MCR(h))))  <= '1';
               hdc_sci_req_MCR(h)(to_integer(unsigned(dsp_rs2_to_sc_MCR(h))))  <= '1';
               hdc_sc_read_addr_MCR(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
               hdc_sc_read_addr_MCR(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0);
             end if;
             if HVSIZE_WRITE_MCR(h) > (0 to Addr_Width => '0') then
               nextstate_DSP(h) <= hdc_exec;
               busy_hdc_internal_wires := '1';
             end if;
             if wb_ready(h) = '1' then
               hdc_sci_we_MCR(h)(to_integer(unsigned(dsp_rd_to_sc_MCR(h))))    <= '1';
               hdc_sc_write_addr_MCR(h) <= RD_Data_IE_lat(h);
             end if;
           end if;
           ----------------------------------------------------------------------

           ----------------------------- BINDING --------------------------------
           if decoded_instruction_lat(h)(KVMUL_bit_position)    = '1' then 
             if mul_stage_2_en(h) = '1' then 
               wb_ready(h) <= '1';
             elsif recover_state(h) = '1' then
               wb_ready(h) <= '1';
             end if;
             if HVSIZE_READ_MCR(h) > (0 to Addr_Width => '0') then
               hdc_to_sc_MCR(h)(to_integer(unsigned(dsp_rs1_to_sc_MCR(h))))(0) <= '1';
               hdc_to_sc_MCR(h)(to_integer(unsigned(dsp_rs2_to_sc_MCR(h))))(1) <= '1';
               hdc_sci_req_MCR(h)(to_integer(unsigned(dsp_rs1_to_sc_MCR(h)))) <= '1';
               hdc_sci_req_MCR(h)(to_integer(unsigned(dsp_rs2_to_sc_MCR(h)))) <= '1';
               hdc_sc_read_addr_MCR(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0); 
               hdc_sc_read_addr_MCR(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              --  nextstate_DSP(h) <= dsp_exec;
              --  busy_hdc_internal_wires := '1';
            end if; 
              if HVSIZE_WRITE_MCR(h) > (0 to Addr_Width => '0') then
              nextstate_DSP(h) <= hdc_exec;
              busy_hdc_internal_wires := '1';
              end if;
             if wb_ready(h) = '1' then
               hdc_sci_we_MCR(h)(to_integer(unsigned(dsp_rd_to_sc_MCR(h)))) <= '1';
               hdc_sc_write_addr_MCR(h) <= RD_Data_IE_lat(h);
             end if;
           end if;
           ----------------------------------------------------------------------------
          
           ----------------------------- SIMILARITY -----------------------------------
           if decoded_instruction_lat(h)(KSUBV_bit_position)    = '1' then 
             if sim_stage_2_en(h) = '1' then 
               wb_ready(h) <= '1';
             elsif recover_state(h) = '1' then
               wb_ready(h) <= '1';
             end if;

             if HVSIZE_READ_MCR(h) > (0 to Addr_Width => '0') then
              hdc_to_sc_MCR(h)(to_integer(unsigned(dsp_rs1_to_sc_MCR(h))))(0) <= '1';
              hdc_to_sc_MCR(h)(to_integer(unsigned(dsp_rs2_to_sc_MCR(h))))(1) <= '1';
              hdc_sci_req_MCR(h)(to_integer(unsigned(dsp_rs1_to_sc_MCR(h)))) <= '1';
              hdc_sci_req_MCR(h)(to_integer(unsigned(dsp_rs2_to_sc_MCR(h)))) <= '1';
              hdc_sc_read_addr_MCR(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              hdc_sc_read_addr_MCR(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0);
             end if;
             if HVSIZE_WRITE_MCR(h) > (0 to Addr_Width => '0') then
              nextstate_DSP(h) <= hdc_exec;
              busy_hdc_internal_wires := '1';
             end if;
             if wb_ready(h) = '1' then
               hdc_sci_we_MCR(h)(to_integer(unsigned(dsp_rd_to_sc_MCR(h)))) <= '1';
               hdc_sc_write_addr_MCR(h) <= RD_Data_IE_lat(h);
             end if;
           end if;
           -------------------------------------------------------------------------

           ---------------------------- CLIPPING -----------------------------------
           if decoded_instruction_lat(h)(KSVADDRF_bit_position)  = '1' then
            if clip_stage_2_en(h) = '1' then 
              wb_ready(h) <= '1';
            elsif recover_state(h) = '1' then
              wb_ready(h) <= '1';  
            end if;
            if HVSIZE_READ_MCR(h) > (0 to Addr_Width => '0') then
              hdc_to_sc_MCR(h)(to_integer(unsigned(dsp_rs1_to_sc_MCR(h))))(0) <= '1';
              hdc_sci_req_MCR(h)(to_integer(unsigned(dsp_rs1_to_sc_MCR(h))))  <= '1';
              hdc_sc_read_addr_MCR(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
            end if;
            if HVSIZE_WRITE_MCR(h) > (0 to Addr_Width => '0') then
              nextstate_DSP(h) <= hdc_exec;
              busy_hdc_internal_wires := '1';
            end if;
            if wb_ready(h) = '1' then
              hdc_sci_we_MCR(h)(to_integer(unsigned(dsp_rd_to_sc_MCR(h))))    <= '1';
              hdc_sc_write_addr_MCR(h) <= RD_Data_IE_lat(h);
            end if;
          end if;
          ------------------------------------------------------------------------

          ------------------------------ PERMUTATION -----------------------------
          if decoded_instruction_lat(h)(KSVMULRF_bit_position)  = '1' then
            if perm_stage_1_en(h) = '1' then
              wb_ready(h) <= '1';
            elsif recover_state(h) = '1' then
              wb_ready(h) <= '1';  
            end if;
            if HVSIZE_READ_MCR(h) > (0 to Addr_Width => '0') then
              hdc_to_sc_MCR(h)(to_integer(unsigned(dsp_rs1_to_sc_MCR(h))))(0) <= '1';
              hdc_sci_req_MCR(h)(to_integer(unsigned(dsp_rs1_to_sc_MCR(h))))  <= '1';
              hdc_sc_read_addr_MCR(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);

            end if;
            if HVSIZE_WRITE_MCR(h) > (0 to Addr_Width => '0') then
              nextstate_DSP(h) <= hdc_exec;
              busy_hdc_internal_wires := '1';
            end if;
            if wb_ready(h) = '1' then
              hdc_sci_we_MCR(h)(to_integer(unsigned(dsp_rd_to_sc_MCR(h))))    <= '1';  
              hdc_sc_write_addr_MCR(h) <= RD_Data_IE_lat(h);
            end if;
          end if;
          -------------------------------------------------------------------------

          ------------------------------ SEARCH -----------------------------------
          if decoded_instruction_lat(h)(KDOTP_bit_position) = '1' then 
             if as_stage_3_en(h) = '1' then 
               wb_ready(h) <= '1';
             elsif recover_state(h) = '1' then
               wb_ready(h) <= '1';
             end if;
             if HVSIZE_READ_MCR(h) > (0 to Addr_Width => '0') then
              hdc_to_sc_MCR(h)(to_integer(unsigned(dsp_rs1_to_sc_MCR(h))))(0) <= '1';
              hdc_to_sc_MCR(h)(to_integer(unsigned(dsp_rs2_to_sc_MCR(h))))(1) <= '1';
              hdc_sci_req_MCR(h)(to_integer(unsigned(dsp_rs1_to_sc_MCR(h)))) <= '1';
              hdc_sci_req_MCR(h)(to_integer(unsigned(dsp_rs2_to_sc_MCR(h)))) <= '1';
              hdc_sc_read_addr_MCR(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              hdc_sc_read_addr_MCR(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0);
             end if;
             if HVSIZE_WRITE_MCR(h) > (0 to Addr_Width => '0') then
              nextstate_DSP(h) <= hdc_exec;
              busy_hdc_internal_wires := '1';
             end if;
             if wb_ready(h) = '1' then
               hdc_sci_we_MCR(h)(to_integer(unsigned(dsp_rd_to_sc_MCR(h)))) <= '1';
               hdc_sc_write_addr_MCR(h) <= RD_Data_IE_lat(h);
             end if;
           end if;
           -------------------------------------------------------------------------

        when others =>
           null;
       end case;
     end if;
      
    busy_hdc_internal(h)    <= busy_hdc_internal_wires;
    hdc_except_condition(h) <= hdc_except_condition_wires(h);
    hdc_taken_branch_MCR(h) <= hdc_taken_branch_MCR_wires(h);
      
  end process;

  ---------------------------------------------------------------------------------------------------------------------------------------------------------
  --  ██████╗ ██╗██████╗ ███████╗██╗     ██╗███╗   ██╗███████╗     ██████╗ ██████╗ ███╗   ██╗████████╗██████╗  ██████╗ ██╗     ██╗     ███████╗██████╗   --
  --  ██╔══██╗██║██╔══██╗██╔════╝██║     ██║████╗  ██║██╔════╝    ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔══██╗██╔═══██╗██║     ██║     ██╔════╝██╔══██╗  --
  --  ██████╔╝██║██████╔╝█████╗  ██║     ██║██╔██╗ ██║█████╗      ██║     ██║   ██║██╔██╗ ██║   ██║   ██████╔╝██║   ██║██║     ██║     █████╗  ██████╔╝  --
  --  ██╔═══╝ ██║██╔═══╝ ██╔══╝  ██║     ██║██║╚██╗██║██╔══╝      ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══██╗██║   ██║██║     ██║     ██╔══╝  ██╔══██╗  --
  --  ██║     ██║██║     ███████╗███████╗██║██║ ╚████║███████╗    ╚██████╗╚██████╔╝██║ ╚████║   ██║   ██║  ██║╚██████╔╝███████╗███████╗███████╗██║  ██║  --
  --  ╚═╝     ╚═╝╚═╝     ╚══════╝╚══════╝╚═╝╚═╝  ╚═══╝╚══════╝     ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚══════╝╚══════╝╚═╝  ╚═╝  --
  ---------------------------------------------------------------------------------------------------------------------------------------------------------

  fsm_DSP_pipeline_controller : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then
      
      dsp_data_gnt_i_lat(h)    <= '0';

      ------------ BUNDLING ---------------
      bundle_stage_1_en(h)     <= '0';
      bundle_stage_2_en(h)     <= '0';
      -------------------------------------

      ------------ BINDING ----------------
      mul_stage_1_en(h)        <= '0';
      mul_stage_2_en(h)        <= '0';
      -------------------------------------

      ------------ SIMILARITY ---------------
      sim_stage_1_en(h)        <= '0';
      sim_stage_2_en(h)        <= '0';
      ---------------------------------------

      ------------ CLIPPING -----------------
      clip_stage_1_en(h)       <= '0';
      clip_stage_2_en(h)       <= '0';
      ---------------------------------------

      ------------ PERMUTATION ---------------
      perm_stage_1_en(h)       <= '0';
      perm_stage_2_en(h)       <= '0';
      ---------------------------------------

      ------------ SEARCH -------------------
      as_stage_1_en(h)         <= '0';
      as_stage_2_en(h)         <= '0';
      as_stage_3_en(h)         <= '0';
      ---------------------------------------


      state_HDC(h)             <= hdc_init;

    elsif rising_edge(clk_i) then

      dsp_data_gnt_i_lat(h)    <= dsp_data_gnt_i(h);

      ------------ BUNDLING ----------------
      bundle_stage_1_en(h)     <= dsp_data_gnt_i_lat(h) and bundle_en(h);
      bundle_stage_2_en(h)     <= bundle_stage_1_en(h);
      --------------------------------------

      ------------ BINDING -----------------
      mul_stage_1_en(h)       <= dsp_data_gnt_i_lat(h) and mul_en(h);
      mul_stage_2_en(h)       <= mul_stage_1_en(h);
      --------------------------------------
      
      ------------- SIMILARITY ----------------
      sim_stage_1_en(h) <= '1' when ((dsp_data_gnt_i_lat(h)='1' and sim_en(h)='1') or (sim_levels_wire(h) > 0)) else '0';


      if sim_levels(h) = NUM_LEVELS then
        sim_stage_2_en(h)      <= sim_stage_1_en(h);
        else
        sim_stage_2_en(h)      <= '0';
      end if;
      -----------------------------------------

      ------------- CLIPPING ------------------
      clip_stage_1_en(h)      <= dsp_data_gnt_i_lat(h) and clip_en(h);
      
      if (first_index(h) = 1 and clip_done_reg(h)='1') then
        clip_stage_2_en(h)      <= clip_stage_1_en(h);
      else
        clip_stage_2_en(h)      <= '0';
      end if;
      ---------------------------------------

      ------------- PERMUTATION ----------------
      perm_stage_1_en(h)      <= dsp_data_gnt_i_lat(h) and perm_en(h);
      perm_stage_2_en(h)      <= perm_stage_1_en(h);
      ------------------------------------------

      -------------- SEARCH --------------------
      as_stage_1_en(h)        <= dsp_data_gnt_i_lat(h) and sim_en(h);
      as_stage_2_en(h)        <= as_stage_1_en(h);

      if (HVSIZE_READ_lat_MCR(h) = (0 to Addr_Width => '0')) then
        as_stage_3_en(h)      <= as_stage_2_en(h);
        else
        as_stage_3_en(h)      <= '0';
      end if;
      -------------------------------------------

      halt_dsp_lat(h)          <= halt_dsp(h);
      state_HDC(h)             <= nextstate_DSP(h);
      busy_hdc_internal_lat(h) <= busy_hdc_internal(h);
      SIMD_RD_BYTES_MCR(h)         <= SIMD_RD_BYTES_wire_MCR(h);
      hdc_except_data(h)       <= hdc_except_data_wire(h);

    end if;
  end process;

  DSP_FU_ENABLER_SYNC : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then

      ------- BUNDLING ----------
      bundle_en(h)         <= '0';
      bundle_en_pending(h) <= '0';
      ---------------------------
      
      ------- BINDING -----------
      mul_en(h)           <= '0';
      bind_en_pending(h)   <= '0';
      ---------------------------
      
      ------- SIMILARITY --------
      sim_en(h)           <= '0';
      sim_en_pending(h)   <= '0';  
      ---------------------------

      ------- CLIPPING ----------
      clip_en(h)          <= '0';
      clip_en_pending(h)  <= '0';
      ---------------------------

      ------- PERMUTATION -------
      perm_en(h)          <= '0';
      perm_en_pending(h)  <= '0';
      ---------------------------

      ------- SEARCH -----------
      as_en(h)            <= '0';
      as_en_pending(h)    <= '0';
      --------------------------

    elsif rising_edge(clk_i) then

      ------------ BUNDLING ------------ 
      bundle_en(h)           <= bundle_en_wire(h);
      bundle_en_pending(h)   <= bundle_en_pending_wire(h);
      ----------------------------------

      ------------ BINDING ---------------
      mul_en(h)           <= bind_en_wire(h); 
      bind_en_pending(h)   <= bind_en_pending_wire(h);
      ------------------------------------

      ------------ SIMILARITY ------------
      sim_en(h)           <= sim_en_wire(h); 
      sim_en_pending(h)   <= sim_en_pending_wire(h);  
      ------------------------------------

      ------------ CLIPPING --------------
      clip_en(h)          <= clip_en_wire(h);
      clip_en_pending(h)  <= clip_en_pending_wire(h);
      ------------------------------------

      ------------ PERMUTATION -----------
      perm_en(h)          <= perm_en_wire(h);
      perm_en_pending(h)  <= perm_en_pending_wire(h);
      ------------------------------------

      ------------ SEARCH ---------------
      as_en(h)            <= as_en_wire(h);
      as_en_pending(h)    <= as_en_pending_wire(h);
      -----------------------------------

    end if;

  end process;

end generate DSP_replicated;

  -------------------------------------------------------------------------------------------------------------------------------------------
  --  ███████╗██╗   ██╗     █████╗  ██████╗ ██████╗███████╗███████╗███████╗    ██╗  ██╗ █████╗ ███╗   ██╗██████╗ ██╗     ███████╗██████╗   --
  --  ██╔════╝██║   ██║    ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝██╔════╝    ██║  ██║██╔══██╗████╗  ██║██╔══██╗██║     ██╔════╝██╔══██╗  --
  --  █████╗  ██║   ██║    ███████║██║     ██║     █████╗  ███████╗███████╗    ███████║███████║██╔██╗ ██║██║  ██║██║     █████╗  ██████╔╝  --
  --  ██╔══╝  ██║   ██║    ██╔══██║██║     ██║     ██╔══╝  ╚════██║╚════██║    ██╔══██║██╔══██║██║╚██╗██║██║  ██║██║     ██╔══╝  ██╔══██╗  --
  --  ██║     ╚██████╔╝    ██║  ██║╚██████╗╚██████╗███████╗███████║███████║    ██║  ██║██║  ██║██║ ╚████║██████╔╝███████╗███████╗██║  ██║  --
  --  ╚═╝      ╚═════╝     ╚═╝  ╚═╝ ╚═════╝ ╚═════╝╚══════╝╚══════╝╚══════╝    ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝  --
  -------------------------------------------------------------------------------------------------------------------------------------------

FU_HANDLER_MC : if multithreaded_accl_en = 0 generate
  DSP_FU_ENABLER_comb : process(all)
  begin
    for h in accl_range loop

      ---------- BUNDLING -------------
      bundle_en_wire(h)<= bundle_en(h);
      ---------------------------------

      ---------- BINDING --------------
      bind_en_wire(h)   <= mul_en(h); 
      ---------------------------------

      ---------- SIMILARITY -----------
      sim_en_wire(h)   <= sim_en(h);       
      ---------------------------------

      ---------- CLIPPING -------------
      clip_en_wire(h)  <= clip_en(h);
      ---------------------------------

      ---------- PERMUTATION ----------
      perm_en_wire(h)  <= perm_en(h);
      ---------------------------------
      
      ---------- SEARCH ---------------
      as_en_wire(h)    <= as_en(h);
      ---------------------------------

      halt_hart(h)     <= '0';
    

      ----------------- BUNDLING ---------------------------
      if bundle_en(h) = '1' and busy_hdc_internal(h) = '0' then
        bundle_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- BINDING ----------------------------
      if mul_en(h) = '1' and busy_hdc_internal(h) = '0' then
        bind_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- SIMILARITY -------------------------
      if sim_en(h) = '1' and busy_hdc_internal(h) = '0' then
        sim_en_wire(h) <= '0';                                  
      end if;
      ------------------------------------------------------

      ----------------- CLIPPING ---------------------------
      if clip_en(h) = '1' and busy_hdc_internal(h) = '0' then
        clip_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------
      
      ----------------- PERMUTATION ------------------------
      if perm_en(h) = '1' and busy_hdc_internal(h) = '0' then
        perm_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      -------------------- SEARCH --------------------------
      if as_en(h) = '1' and busy_hdc_internal(h) = '0' then
        as_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then

        case state_HDC(h) is

          when hdc_init =>

            -- Set signals to enable correct virtual parallelism operation

            ------------------------ BUNDLING ---------------------------------
            if decoded_instruction_MCR(KADDV_bit_position)    = '1' then 
              bundle_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ------------------------ BINDING ----------------------------------
            elsif decoded_instruction_MCR(KVMUL_bit_position) = '1' then
              bind_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ---------------------- SIMILARITY ---------------------------------
            elsif decoded_instruction_MCR(KSUBV_bit_position)    = '1' then
              sim_en_wire(h) <= '1';                                         
            -------------------------------------------------------------------

            ---------------------- CLIPPING -----------------------------------
            elsif decoded_instruction_MCR(KSVADDRF_bit_position  ) = '1' then
              clip_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ---------------------- PERMUTATION --------------------------------
            elsif decoded_instruction_MCR(KSVMULRF_bit_position) = '1' then
              perm_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ---------------------- SEARCH -------------------------------------
            elsif decoded_instruction_MCR(KDOTP_bit_position) = '1' then
              as_en_wire(h)  <= '1';
              sim_en_wire(h) <= '1';
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
  DSP_FU_ENABLER_comb : process(all)
  begin

    for h in accl_range loop

      ------------- BUNDLING -----------------------
      bundle_en_wire(h)               <= bundle_en(h);
      bundle_en_pending_wire(h)       <= bundle_en_pending(h);
      ----------------------------------------------

      ------------- BINDING ------------------------
      bind_en_wire(h)                 <= mul_en(h);
      bind_en_pending_wire(h)         <= bind_en_pending(h);
      ----------------------------------------------

      ------------- SIMILARITY ---------------------
      sim_en_wire(h)                 <= sim_en(h);  
      sim_en_pending_wire(h)         <= sim_en_pending(h);        
      ----------------------------------------------

      ------------- CLIPPING -----------------------
      clip_en_wire(h)                <= clip_en(h);
      clip_en_pending_wire(h)        <= clip_en_pending(h);
      ----------------------------------------------

      ------------- PERMUTATION --------------------
      perm_en_wire(h)                <= perm_en(h);
      perm_en_pending_wire(h)        <= perm_en_pending(h);
      ----------------------------------------------

      ------------- SEARCH --------------------------
      as_en_wire(h)                  <= as_en(h);
      as_en_pending_wire(h)          <= as_en_pending(h);
      -----------------------------------------------

      fu_req(h)                      <= (others => '0');
      halt_hart(h)                   <= '0';
      
      ----------------- BUNDLING ---------------------------
      if bundle_en(h) = '1' and busy_hdc_internal(h) = '0' then
        bundle_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- BINDING ----------------------------
      if mul_en(h) = '1' and busy_hdc_internal(h) = '0' then
        bind_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ---------------- SIMILARITY --------------------------
      if sim_en(h) = '1' and busy_hdc_internal(h) = '0' then
        sim_en_wire(h) <= '0';   
      end if;                                 
      -----------------------------------------------------

      ----------------- CLIPPING --------------------------
      if clip_en(h) = '1' and busy_hdc_internal(h) = '0' then
        clip_en_wire(h) <= '0';
      end if;
      -----------------------------------------------------

      ----------------- PERMUTATION -----------------------
      if perm_en(h) = '1' and busy_hdc_internal(h) = '0' then
        perm_en_wire(h) <= '0';
      end if;
      -----------------------------------------------------

      ----------------- SEARCH ----------------------------
      if as_en(h) = '1' and busy_hdc_internal(h) = '0' then
        as_en_wire(h) <= '0';
      end if;
      -----------------------------------------------------

      if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then

        case state_HDC(h) is

          when hdc_init =>

            -- Set signals to enable correct virtual parallelism operation
            ------------------------ BUNDLING --------------------------------
            if decoded_instruction_MCR(KADDV_bit_position)    = '1' then
              if busy_bundle = '0' and bundle_en_pending = (accl_range => '0') then 
                bundle_en_wire(h) <= '1';
              else
                bundle_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(0) <= '1';
              end if;
            ------------------------------------------------------------------

            ---------------------- BINDING -----------------------------------
            elsif decoded_instruction_MCR(KVMUL_bit_position) = '1' then
              if busy_bind = '0' and bind_en_pending = (accl_range => '0') then 
                bind_en_wire(h) <= '1';
              else
                bind_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(2) <= '1';
              end if;
            ------------------------------------------------------------------

            ----------------------- SIMILARITY -------------------------------
            elsif decoded_instruction_MCR(KSUBV_bit_position)    = '1' then
              if busy_sim = '0' and sim_en_pending = (accl_range => '0') then 
                sim_en_wire(h) <= '1';                                        
              else
                sim_en_pending_wire(h) <= '1';                                  
                halt_hart(h) <= '1';
                fu_req(h)(5) <= '1';
              end if;
            -------------------------------------------------------------------

            ----------------------- CLIPPING -----------------------------------
            elsif decoded_instruction_MCR(KSVADDRF_bit_position  ) = '1' then
              if busy_clip = '0' and clip_en_pending = (accl_range => '0') then 
                clip_en_wire(h) <= '1';
              else
                clip_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(6) <= '1';
              end if;
            -------------------------------------------------------------------

            ----------------------- PERMUTATION --------------------------------
            elsif decoded_instruction_MCR(KSVMULRF_bit_position) = '1' then
              if busy_perm = '0' and perm_en_pending = (accl_range => '0') then 
                perm_en_wire(h) <= '1';
              else
                perm_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(7) <= '1';
              end if;
            -------------------------------------------------------------------

            ----------------------- SEARCH -------------------------------------
            elsif decoded_instruction_MCR(KDOTP_bit_position) = '1' then
              if busy_as = '0' and busy_sim = '0' and as_en_pending = (accl_range => '0') and sim_en_pending = (accl_range => '0') then 
                as_en_wire(h) <= '1';
                sim_en_wire(h) <= '1';
              else
                as_en_pending_wire(h) <= '1';
                sim_en_pending_wire(h) <= '1'; 
                halt_hart(h) <= '1';
                fu_req(h)(8) <= '1';
                fu_req(h)(5) <= '1';
              end if;
            -------------------------------------------------------------------

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
            
            if fu_gnt(h)(7) = '1' then
              perm_en_wire(h) <= '1';
              perm_en_pending_wire(h) <= '0';
            elsif perm_en_pending(h) = '1' and fu_gnt(h)(7) = '0'  then
              halt_hart(h) <= '1';
            end if;

            if fu_gnt(h)(8) = '1' then
              as_en_wire(h) <= '1';
              as_en_pending_wire(h) <= '0';
            elsif as_en_pending(h) = '1' and fu_gnt(h)(8) = '0'  then
              halt_hart(h) <= '1';
            end if;

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

      ------------------- BUNDLING -----------------------
      if bundle_en_pending_wire(h) = '1' and busy_bundle_wire = '0' then
        fu_gnt_en(h)(0) <= '1';
      end if;
      ----------------------------------------------------

      ------------------- BINDING ------------------------
      if bind_en_pending_wire(h) = '1' and busy_bind_wire = '0' then
        fu_gnt_en(h)(2) <= '1';
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

      -------------------- PERMUTATION -------------------
      if perm_en_pending_wire(h) = '1' and busy_perm_wire = '0' then
        fu_gnt_en(h)(7) <= '1';
      end if;
      ----------------------------------------------------

      -------------------- SEARCH -------------------------
      if as_en_pending_wire(h) = '1' and busy_sim_wire = '0' then
        fu_gnt_en(h)(8) <= '1';
      end if;
      -----------------------------------------------------
      
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


  DSP_BUSY_FU_SYNC : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then
      
    elsif rising_edge(clk_i) then

      -------- BUNDLING ----------
      busy_bundle  <= busy_bundle_wire;
      ----------------------------

      -------- BINDING ------------
      busy_bind    <= busy_bind_wire;
      -----------------------------
      
      -------- SIMILARITY --------
      busy_sim    <= busy_sim_wire;
      ----------------------------

      -------- CLIPPING ----------
      busy_clip   <= busy_clip_wire;
      ----------------------------

      -------- PERMUTATION -------
      busy_perm   <= busy_perm_wire;
      ----------------------------

      -------- SEARCH -------------
      busy_as     <= busy_as_wire;
      -----------------------------

    end if;
  end process;

end generate FU_HANDLER_MT;

-------- BUNDLING ----------
busy_bundle_wire <= '1' when multithreaded_accl_en = 1 and bundle_en_wire   /= (accl_range => '0') else '0';
----------------------------

-------- BINDING -----------
busy_bind_wire <= '1' when multithreaded_accl_en = 1 and bind_en_wire   /= (accl_range => '0') else '0';
----------------------------

-------- SIMILARITY --------
busy_sim_wire <= '1' when multithreaded_accl_en = 1 and sim_en_wire   /= (accl_range => '0') else '0';
----------------------------

-------- CLIPPING ----------
busy_clip_wire <= '1' when multithreaded_accl_en = 1 and clip_en_wire /= (accl_range => '0') else '0';
----------------------------

------- PERMUTATION --------
busy_perm_wire <= '1' when multithreaded_accl_en = 1 and perm_en_wire /= (accl_range => '0') else '0';
----------------------------

-------- SEARCH ------------
busy_as_wire <= '1' when multithreaded_accl_en = 1 and as_en_wire /= (accl_range => '0') else '0';
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
      dsp_sc_acc_data_write_wire_int(h) <= (others => '0');
      hdc_sc_data_write_wire_MCR(h)      <= hdc_sc_data_write_wire_int(h);
      dsp_sc_acc_data_write_wire_MCR(h) <= dsp_sc_acc_data_write_wire_int(h);
      -- Questo segnale stabilisce il numero di byte che la DSP è in grado di leggere in un ciclo di clock
      -- in funzione del parallelismo (SIMD). Nel caso della ricerca assorciativa questa  


      SIMD_RD_BYTES_wire_MCR(h)          <= SIMD*(Data_Width/8);

      if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then
        case state_HDC(h) is
          
          when hdc_init =>


          when hdc_exec =>

            --------------------------- BUNDLING ------------------------------
            if decoded_instruction_lat(h)(KADDV_bit_position)    = '1' then
              dsp_sc_acc_data_write_wire_int(h) <= hdcu_out_bundle_results(h);
            end if;
            -------------------------------------------------------------------

            --------------------------- BINDING --------------------------------
            if (decoded_instruction_lat(h)(KVMUL_bit_position)    = '1' ) then
              hdc_sc_data_write_wire_int(h) <= dsp_out_mul_results(h);
            end if;
            --------------------------------------------------------------------

            -------------------------- SIMILARITY -------------------------------
            if decoded_instruction_lat(h)(KSUBV_bit_position)      = '1' then
              hdc_sc_data_write_wire_int(h)(SIMILARITY_BITS -1 downto 0) <= hdcu_out_sim_results(h);              
            end if;
            ---------------------------------------------------------------------

            ------------------------ CLIPPING -----------------------------------
            if decoded_instruction_lat(h)(KSVADDRF_bit_position  )   = '1' then
                hdc_sc_data_write_wire_int(h) <= hdcu_out_clip_results(h);

            end if;
            ----------------------------------------------------------------------

            ------------------------ PERMUTATION --------------------------------
            if decoded_instruction_lat(h)(KSVMULRF_bit_position)   = '1' then
              hdc_sc_data_write_wire_int(h) <= hdcu_in_perm_operand_0(h);
            end if;
            ----------------------------------------------------------------------

            ------------------------ SEARCH -------------------------------------
            if decoded_instruction_lat(h)(KDOTP_bit_position) = '1' then
              hdc_sc_data_write_wire_int(h)(Data_Width - 1 downto 0) <= std_logic_vector(to_unsigned(temp_best_class_index(h), 32));
            end if;
            ----------------------------------------------------------------------

            if halt_dsp(h) = '0' and halt_dsp_lat(h) = '1' then
              hdc_sc_data_write_wire_MCR(h) <= dsp_sc_data_write_int(h);
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
      dsp_sc_acc_data_write_wire_int(h) <= (others => '0');
      hdc_sc_data_write_wire_MCR(h)      <= hdc_sc_data_write_wire_int(h);
      dsp_sc_acc_data_write_wire_MCR(h) <= dsp_sc_acc_data_write_wire_int(h);
      SIMD_RD_BYTES_wire_MCR(h)          <= SIMD*(Data_Width/8);

      if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then
        
        case state_HDC(h) is
          
          when hdc_init =>

         
          when hdc_exec =>
            
            ------------------------- BUNDLING --------------------------------
            if decoded_instruction_lat(h)(KADDV_bit_position)   = '1' then
              dsp_sc_acc_data_write_wire_int(h) <= hdcu_out_bundle_results(0);
            end if;
            -------------------------------------------------------------------

            ------------------------- BINDING --------------------------------
            if (decoded_instruction_lat(h)(KVMUL_bit_position)    = '1' ) then
              hdc_sc_data_write_wire_int(h) <= dsp_out_mul_results(0);
            end if;
            -------------------------------------------------------------------
            
            ------------------------- SIMILARITY ------------------------------
            if decoded_instruction_lat(h)(KSUBV_bit_position)      = '1' then
              hdc_sc_data_write_wire_int(h)(SIMILARITY_BITS -1 downto 0) <= hdcu_out_sim_results(0);              
            end if;
            -------------------------------------------------------------------

            ------------------------- CLIPPING --------------------------------
            if decoded_instruction_lat(h)(KSVADDRF_bit_position  )   = '1' then
              hdc_sc_data_write_wire_int(h) <= hdcu_out_clip_results(h);
            end if;
            -------------------------------------------------------------------

            ------------------------- PERMUTATION -----------------------------
            if decoded_instruction_lat(h)(KSVMULRF_bit_position)   = '1' then
              hdc_sc_data_write_wire_int(h) <= hdcu_in_perm_operand_0(h);
            end if;
            -------------------------------------------------------------------

            ------------------------- SEARCH ----------------------------------
            if decoded_instruction_lat(h)(KDOTP_bit_position) = '1' then
              hdc_sc_data_write_wire_int(h)(Data_Width - 1 downto 0) <= std_logic_vector(to_unsigned(temp_best_class_index(0), 32));
            end if;
            -------------------------------------------------------------------

            if halt_dsp(h) = '0' and halt_dsp_lat(h) = '1' then
              hdc_sc_data_write_wire_MCR(h) <= dsp_sc_data_write_int(h);
            end if;

          when others =>
            null;

        end case;
      end if;
    end loop;
  end process;
end generate;


FU_replicated : for f in fu_range generate

  DSP_MAPPING_IN_UNIT_comb : process(all)
  
  variable h : integer;

  begin
    
    ------------------- BUNDLING ---------------------
    hdcu_in_bundle_operand(f)      <= (others => '0');
    hdcu_in_acc_bundle_operand(f)  <= (others => '0');
    -------------------------------------------------

    ------------------- BINDING --------------------
    hdcu_in_bind_operands(f)        <= (others => (others => '0'));
    --------------------------------------------------
    
    ------------------- SIMILARITY -------------------
    hdcu_in_sim_operands(f)         <= (others => (others => '0'));
    -------------------------------------------------

    ------------------- CLIPPING ---------------------
    hdcu_in_clip_operand_0(f)       <= (others => '0');
    -------------------------------------------------

    ------------------- PERMUTATION ------------------
    hdcu_in_perm_operand_0(f)       <= (others => '0');
    -------------------------------------------------

    for g in 0 to (ACCL_NUM - FU_NUM) loop

      if multithreaded_accl_en = 1 then
        h := g;  -- set the spm rd/wr ports equal to the "for-loop"
      elsif multithreaded_accl_en = 0 then
        h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
      end if;

      if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then
        case state_HDC(h) is
          
          when hdc_exec =>

            ---------------------------- BUNDLING --------------------------------------
            if decoded_instruction_lat(h)(KADDV_bit_position) = '1' then 
              hdcu_in_acc_bundle_operand(f) <= dsp_sc_acc_data_read(h)(0);
              hdcu_in_bundle_operand(f) <= dsp_sc_data_read(h)(1);
            end if;
            -------------------------------------------------------------------------

            ----------------------------- BINDING -----------------------------------
            if decoded_instruction_lat(h)(KVMUL_bit_position)  = '1' then
              hdcu_in_bind_operands(f)(0) <= dsp_sc_data_read(h)(0);
              hdcu_in_bind_operands(f)(1) <= dsp_sc_data_read(h)(1);             
            end if;
            ------------------------------------------------------------------------

            ----------------------------- SIMILARITY -------------------------------
             if (decoded_instruction_lat(h)(KSUBV_bit_position)  = '1') or 
                (decoded_instruction_lat(h)(KDOTP_bit_position)  = '1') then
              
              hdcu_in_sim_operands(f)(0) <= dsp_sc_data_read(h)(0);
              
              if decoded_instruction_lat(h)(KDOTP_bit_position) = '1' and to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire_MCR(h) then
                -- Prendo un class vector alla volta 
                hdcu_in_sim_operands(f)(1)(to_integer(unsigned(HVSIZE(h)))*8 - 1 downto 0) <= dsp_sc_data_read(h)(1)(to_integer(unsigned(HVSIZE(h)))*8 - 1 downto 0); 
              else
                hdcu_in_sim_operands(f)(1) <= dsp_sc_data_read(h)(1);

              end if;

            end if;
            ------------------------------------------------------------------------

             --------------------------- CLIPPING ----------------------------------
             if decoded_instruction_lat(h)(KSVADDRF_bit_position  ) = '1' then
              hdcu_in_clip_operand_0(f) <= dsp_sc_acc_data_read(h)(0);
              -- hdcu_in_clip_operand_1(f) <= RS2_Data_IE_lat(h);  -- unused in the MCR version
            end if;
            ------------------------------------------------------------------------
            
             --------------------------- PERMUTATION --------------------------------
             if decoded_instruction_lat(h)(KSVMULRF_bit_position) = '1' then
              hdcu_in_perm_operand_0(f) <= dsp_sc_data_read(h)(0);
              hdcu_in_perm_operand_1(f) <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0);
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

  fsm_HDCU_bundling : process(clk_i, rst_ni)
  variable h : integer;
  begin
    if rst_ni = '0' then
      count_real_imag         <= (others => 0);
    elsif rising_edge(clk_i) then
      count_real_imag         <= (others => 0);
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;

        -- When we have the enable
        if bundle_en(h) = '1' and halt_dsp_lat(h) = '0' and (bundle_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
        count_real_imag(h)  <= count_real_imag_wire(h);

        for i in 0 to (((SIMD*Data_Width)/MODULO_BITS) / 2) - 1 loop
          -- Assign real part to even index
          hdcu_out_bundle_results(f)(FP_Width_MCR*(2*i+1)-1 downto FP_Width_MCR*(2*i)) <= bundling_sum_real(h)(FP_Width_MCR*(i+1)-1 downto FP_Width_MCR*i);

          -- Assign imag part to odd index
          hdcu_out_bundle_results(f)(FP_Width_MCR*(2*i+2)-1 downto FP_Width_MCR*(2*i+1)) <= bundling_sum_imag(h)(FP_Width_MCR*(i+1)-1 downto FP_Width_MCR*i);
        end loop;


        end if;
      end loop;
    end if;
  end process;

  comb_HDCU_bundling: process(all)
  variable h : integer;
  begin

    for g in 0 to (ACCL_NUM - FU_NUM) loop
    
      if multithreaded_accl_en = 1 then
        h := g;  -- set the spm rd/wr ports equal to the "for-loop"
      elsif multithreaded_accl_en = 0 then
        h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
      end if;
      
      if bundle_en(h) = '1' and halt_dsp_lat(h) = '0' and (bundle_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
        if (count_real_imag(h)  = 0) then
          count_real_imag_wire(h) <= 1;
        else 
          count_real_imag_wire(h) <= 0;
        end if;
      for i in 0 to (((SIMD*Data_Width)/MODULO_BITS) / 2) - 1 loop
          -- Index for input/accumulator, one per pair
          -- Real part (even position)
          cosine_values(h)(FP_Width_MCR*(i+1)-1 downto FP_Width_MCR*i) <= std_logic_vector(resize(signed(COS_LUT(to_integer(unsigned(hdcu_in_bundle_operand(f)(count_real_imag(h) * SIMD*Data_Width/2 + MODULO_BITS*(i+1)-1 downto MODULO_BITS*i + count_real_imag(h) * SIMD*Data_Width/2))))),FP_Width_MCR));
          bundling_sum_real(h)(FP_Width_MCR*(i+1)-1 downto FP_Width_MCR*i) <= 
              std_logic_vector(
                  signed(cosine_values(h)(FP_Width_MCR*(i+1)-1 downto FP_Width_MCR*i)) +
                  signed(hdcu_in_acc_bundle_operand(f)(FP_Width_MCR*(2*i+1)-1 downto FP_Width_MCR*(2*i)))
              );

          -- Imag part (odd position, next slot)
          sine_values(h)(FP_Width_MCR*(i+1)-1 downto FP_Width_MCR*i) <=  std_logic_vector(resize(signed(SIN_LUT(to_integer(unsigned(hdcu_in_bundle_operand(f)(count_real_imag(h) * SIMD*Data_Width/2 + MODULO_BITS*(i+1)-1 downto MODULO_BITS*i + count_real_imag(h) * SIMD*Data_Width/2))))),FP_Width_MCR));
          bundling_sum_imag(h)(FP_Width_MCR*(i+1)-1 downto FP_Width_MCR*i) <= 
              std_logic_vector(
                  signed(sine_values(h)(FP_Width_MCR*(i+1)-1 downto FP_Width_MCR*i)) +
                  signed(hdcu_in_acc_bundle_operand(f)(FP_Width_MCR*(2*i+2)-1 downto FP_Width_MCR*(2*i+1)))
              );
      end loop;


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
    elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;
        if halt_dsp_lat(h) = '0' then
          if mul_en(h) = '1' and (mul_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
            for i in 0 to SIMD-1 loop
                for k in 0 to (Data_Width / MODULO_BITS) - 1 loop
                    dsp_out_mul_results(f)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k) <=
                        std_logic_vector(unsigned(hdcu_in_bind_operands(f)(0)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k)) +
                                         unsigned(hdcu_in_bind_operands(f)(1)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k)));
                end loop;
            end loop;
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

  fsm_HDCU_sim : process(clk_i,rst_ni)
  variable h : integer;
  begin
    if rst_ni = '0' then    
      hdcu_out_sim_results <= (others => (others => '0'));
      distance_accumulator <= (others => (others => '0'));
      sim_measure_reg      <= (others => (others => '0'));

    elsif rising_edge(clk_i) then
      
      hdcu_out_sim_results     <= (others => (others => '0'));
      distance_accumulator     <= (others => (others => '0'));
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then -- Tutti gli acceleratori condividono la stessa FU
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then -- Ho una FU per acceleratore
          h := f;  -- set the spm rd/wr ports equal to the "for-generate",
        end if;

        if sim_en(h) = '1' then 
            valid_pipe(0) <= '1';
            for j in 1 to NUM_LEVELS loop
                valid_pipe(j) <= valid_pipe(j-1);
            end loop;
        end if;

        if halt_dsp_lat(h) = '0' and sim_en(h) = '1' and (sim_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
          distance_accumulator(h) <= distance_accumulator_wire(h); 
          -- Inizializzo il registro quando ho finito di leggere un class Vector
          if to_integer(unsigned(fake_HVSIZE_READ_2_MCR(h))) = 0 then
            sim_measure_reg <= (others => (others => '0'));
          else
            sim_measure_reg(f) <= sim_measure(f);
          end if;
          hdcu_out_sim_results(f) <= distance_accumulator_wire(h);  -- Con 13 bit sono in grado di misurare una similarity fino a 8192 che è la dimensione massima attuale degli ipervettori
        end if;
      end loop;
    end if;
  end process;

  HDCU_sim_comb : process(all)
  variable h : integer;
  variable accumulated_sim : std_logic_vector(SIMILARITY_BITS - 1 downto 0); 
  
  begin
    accumulated_sim       := (others => '0'); -- Reset accumulated_sim for each process activation
    distance_accumulator_wire <= (others => (others => '0')); -- Reset distance_accumulator_wire for each process activation
    sim_measure           <= (others => (others => '0')); -- Reset sim_measure for each process activation
    
    for g in 0 to (ACCL_NUM - FU_NUM) loop
      if multithreaded_accl_en = 1 then 
        h := g;  -- set the spm rd/wr ports equal to the "for-loop"
      elsif multithreaded_accl_en = 0 then 
        h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
      end if;
      
      if halt_dsp_lat(h) = '0' and sim_en(h) = '1' and (sim_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
        
          for i in 0 to SIMD-1 loop
            for k in 0 to (Data_Width / MODULO_BITS) - 1 loop
                difference1(f)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k) <=
                    std_logic_vector(unsigned(hdcu_in_sim_operands(f)(0)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k)) -
                                     unsigned(hdcu_in_sim_operands(f)(1)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k)));

                difference2(f)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k) <=
                std_logic_vector(unsigned(hdcu_in_sim_operands(f)(1)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k)) -
                                unsigned(hdcu_in_sim_operands(f)(0)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k)));

                -- Find the minimum between the two differences
                minimum_difference(f)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k) <= 
                (difference1(f)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k))  when
                                 unsigned(difference1(f)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k)) <
                                 unsigned(difference2(f)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k)) else
                                 (difference2(f)(Data_Width*i + MODULO_BITS*(k+1) - 1 downto Data_Width*i + MODULO_BITS*k));
            end loop;
         end loop;

          sim_measure(f) <= (others => '0'); 
          -- Se ho una KDOTP allora aggiorno distance_accumulator_wire solo se ho finito di leggere un class Vector (fake_HVSIZE_READ = 0)
          if decoded_instruction_lat(h)(KDOTP_bit_position) = '1' then
            if sim_levels_search(h) < NUM_LEVELS then
              sim_levels_search_wire(h) <= sim_levels_search(h)+1;
            end if;
            if valid_pipe(NUM_LEVELS) = '1' then
              distance_accumulator_wire(h) <= std_logic_vector(unsigned(distance_accumulator(h)) + stage_array(NUM_LEVELS-1)(SIMILARITY_BITS - 1 downto 0));
              if to_integer(unsigned(fake_HVSIZE_READ_2_MCR(h))) = 0 then
                distance_accumulator_wire(h) <= std_logic_vector(unsigned(stage_array(NUM_LEVELS-1)(SIMILARITY_BITS - 1 downto 0)));
              end if;
            end if;
          -- Altrimenti aggiorno sempre distance_accumulator_wire
          else
            if valid_pipe(NUM_LEVELS) = '1' then-- or sim_levels_wire(h) > 0 then
                distance_accumulator_wire(h) <= std_logic_vector(unsigned(distance_accumulator(h)) + stage_array(NUM_LEVELS-1)(SIMILARITY_BITS - 1 downto 0));
            end if;
          end if;
          
        end if;
    end loop;
  end process;

  gen_stages: for level in 0 to NUM_LEVELS-1 generate
    constant num_in : integer := Simd_Width/MODULO_BITS / (2 ** level);
    constant in_w   : integer := stage_in_width(level);
    constant out_w  : integer := stage_out_width(level);
    constant current_width : integer := MODULO_BITS + level;  -- width of each input element at this stage
    signal stage_in_sig  : unsigned(in_w-1 downto 0);
    signal stage_out_sig : unsigned(out_w-1 downto 0);
  begin

      gen_first_stage: if level = 0 generate
        stage_in_sig <= unsigned(minimum_difference(0)(in_w-1 downto 0));

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
            DATA_WIDTH => MODULO_BITS + 1*level  -- 1 bit is added for each level to account for the carry
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

  --------------------------------------------------------------------------------------------------
  --  ██████╗██╗     ██╗██████╗ ██████╗ ██╗███╗   ██╗ ██████╗     ██╗   ██╗███╗   ██╗██╗████████╗ --
  -- ██╔════╝██║     ██║██╔══██╗██╔══██╗██║████╗  ██║██╔════╝     ██║   ██║████╗  ██║██║╚══██╔══╝ --
  -- ██║     ██║     ██║██████╔╝██████╔╝██║██╔██╗ ██║██║  ███╗    ██║   ██║██╔██╗ ██║██║   ██║    -- 
  -- ██║     ██║     ██║██╔═══╝ ██╔═══╝ ██║██║╚██╗██║██║   ██║    ██║   ██║██║╚██╗██║██║   ██║    -- 
  -- ╚██████╗███████╗██║██║     ██║     ██║██║ ╚████║╚██████╔╝    ╚██████╔╝██║ ╚████║██║   ██║    -- 
  --  ╚═════╝╚══════╝╚═╝╚═╝     ╚═╝     ╚═╝╚═╝  ╚═══╝ ╚═════╝      ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --  
  --------------------------------------------------------------------------------------------------
  clip_comb : process(all)
  variable h : integer;
  begin
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        
        if multithreaded_accl_en = 1 then -- Assicurarsi che sia std_logic
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then -- Assicurarsi che sia std_logic
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;
        
        -- Initializations:
        real_in(h) <= (others => (others => '0'));
        imag_in(h) <= (others => (others => '0'));
        cos_val(h) <= (others => (others => '0'));
        sin_val(h) <= (others => (others => '0'));
        real_mult(h) <= (others => (others => '0'));
        imag_mult(h) <= (others => (others => '0'));
        real_fixed(h) <= (others => (others => '0'));
        imag_fixed(h) <= (others => (others => '0'));
        buff(h) <= (others => (others => '0'));
        max_val_wire(h) <= max_val(h);
        max_index_wire(h) <= max_index(h);


        if halt_dsp_lat(h) = '0' and clip_en(h) = '1' and (clip_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
          
          
          for i in 0 to (((SIMD*Data_Width)/MODULO_BITS) / 2) - 1 loop
            -- Get real and imag input
            real_in(h)(i) <= (hdcu_in_clip_operand_0(f)(FP_Width_MCR*(2*i+1)-1 downto FP_Width_MCR*(2*i)));
            imag_in(h)(i) <= (hdcu_in_clip_operand_0(f)(FP_Width_MCR*(2*i+2)-1 downto FP_Width_MCR*(2*i+1)));

            -- LUT values
            cos_val(h)(i) <= std_logic_vector(resize(signed(COS_LUT(index(h))),FP_Width_MCR)); -- Q16.16
            sin_val(h)(i) <= std_logic_vector(resize(signed(SIN_LUT(index(h))),FP_Width_MCR)); -- Q16.16
            
            -- Multiply (Q16.16 × Q16.16 → Q32.32, extract bits [47:16] for Q16.16)
            real_mult(h)(i) <= std_logic_vector(signed(real_in(h)(i)) * signed(cos_val_reg(h)(i)));
            imag_mult(h)(i) <= std_logic_vector(signed(imag_in(h)(i)) * signed(sin_val_reg(h)(i)));

            -- Shift right 16 (Q16.16 result)
            real_fixed(h)(i) <= real_mult(h)(i)(FP_frac_MCR+FP_Width_MCR-1 downto FP_frac_MCR);
            imag_fixed(h)(i) <= imag_mult(h)(i)(FP_frac_MCR+FP_Width_MCR-1 downto FP_frac_MCR);

            -- Sum the pair
            buff(h)(i) <= std_logic_vector(signed(real_fixed(h)(i)) + signed(imag_fixed(h)(i)));

            -- Update argmax
            if (signed(buff_reg(h)(i)) > max_val(h)(i)) then
              max_val_wire(h)(i)   <= signed(buff_reg(h)(i));
              max_index_wire(h)(i) <= index_reg_reg(h);
            end if;
          end loop;
        end if;
      end loop;
  end process;

  fsm_DSP_clip : process(clk_i,rst_ni)
  variable h : integer;
  begin
    if rst_ni = '0' then
      max_val              <= (others => (others => (others => '0')));
      max_index            <= (others=>(others => 0));
      first_index          <= (others => 0);
      index                <= (others => 0);
      buff_reg             <= (others => (others => (others => '0')));
      index_reg            <= (others => 0);
      index_reg_reg            <= (others => 0);
      clip_done            <= (others => '0');
      clip_done_reg        <= (others => '0');
    elsif rising_edge(clk_i) then
  
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        
        if multithreaded_accl_en= 1 then -- Assicurarsi che sia std_logic
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then -- Assicurarsi che sia std_logic
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;
        
        clip_done(h) <= '0'; 
        if halt_dsp_lat(h) = '0' and clip_en(h) = '1' and (clip_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
          max_val(h) <= max_val_wire(h);      -- Assign the max_val from the combinational process
          max_index(h) <= max_index_wire(h);  -- Assign the max_index from the combinational process

          buff_reg(h) <= buff(h);   -- Register the buffer for the next cycle
          index_reg(h) <= index(h); -- Register the index for the next cycle
          index_reg_reg(h) <= index_reg(h);
          cos_val_reg(h) <= cos_val(h);
          sin_val_reg(h) <= sin_val(h);
          clip_done_reg(h) <= clip_done(h);

          if (index(h) = 2**MODULO_BITS-1) then
            index(h) <= 0;
            clip_done(h) <= '1'; -- Set clip_done to 1 when index wraps around
          else
            index(h) <= index(h) + 1;
          end if;

          -- Increment index (with wrap)
          if (clip_done_reg(h)) then
            
            for i in 0 to (((SIMD*Data_Width)/MODULO_BITS) / 2) - 1 loop
                if first_index(h) = 0 then  
                  hdcu_out_clip_results(f)(MODULO_BITS*(i+1)-1 downto MODULO_BITS*i) <= std_logic_vector(to_unsigned(max_index_wire(h)(i), MODULO_BITS));
                else
                  hdcu_out_clip_results(f)((32*SIMD/2)+MODULO_BITS*(i+1)-1 downto (32*SIMD/2)+MODULO_BITS*i) <= std_logic_vector(to_unsigned(max_index_wire(h)(i), MODULO_BITS));
                end if;
            end loop;

            if first_index(h) = 0 then
              first_index(h) <= 1; -- Set first_index to 1 when index wraps around
            else
              first_index(h) <= 0; -- Reset first_index when index wraps around
            end if;

            max_val(h) <= (others=>(others => '0')); -- Reset max_val
            max_index(h) <= (others => 0); -- Reset max_index
          end if;
        end if;
      end loop;
    end if;
  end process;

  -------------------------------------------------------------------------------------------------------------------------------------
  -- ██████╗ ███████╗██████╗ ███╗   ███╗██╗   ██╗████████╗ █████╗ ████████╗██╗ ██████╗ ███╗   ██╗    ██╗   ██╗███╗   ██╗██╗████████╗ --
  -- ██╔══██╗██╔════╝██╔══██╗████╗ ████║██║   ██║╚══██╔══╝██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║    ██║   ██║████╗  ██║██║╚══██╔══╝ --
  -- ██████╔╝█████╗  ██████╔╝██╔████╔██║██║   ██║   ██║   ███████║   ██║   ██║██║   ██║██╔██╗ ██║    ██║   ██║██╔██╗ ██║██║   ██║    --
  -- ██╔═══╝ ██╔══╝  ██╔══██╗██║╚██╔╝██║██║   ██║   ██║   ██╔══██║   ██║   ██║██║   ██║██║╚██╗██║    ██║   ██║██║╚██╗██║██║   ██║    --
  -- ██║     ███████╗██║  ██║██║ ╚═╝ ██║╚██████╔╝   ██║   ██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║    ╚██████╔╝██║ ╚████║██║   ██║    --
  -- ╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝ ╚═════╝    ╚═╝   ╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝     ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --
  -------------------------------------------------------------------------------------------------------------------------------------                                                                                                                               
  
  -- -- This process can be removed and substituted by a variable in the sync process, no difference in hardware (actually separated just for debug reasons)
  -- fsm_DSP_permcomb : process(all)
  -- variable h : integer;
  -- begin
  --   for g in 0 to (ACCL_NUM - FU_NUM) loop
        
  --     if multithreaded_accl_en = 1 then -- Assicurarsi che sia std_logic
  --       h := g;  -- set the spm rd/wr ports equal to the "for-loop"
  --     elsif multithreaded_accl_en = 0 then -- Assicurarsi che sia std_logic
  --       h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
  --     end if;
  --     new_perm_offset(h) <= 0;
  --     if (to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire(h)) then
  --       new_perm_offset(h) <= (SIMD_RD_BYTES_wire(h) - to_integer(unsigned(HVSIZE(h))))*8;
  --     elsif (to_integer(unsigned(HVSIZE_READ_lat(h)))/=0 and to_integer(unsigned(HVSIZE_READ_lat(h))) <= SIMD_RD_BYTES_wire(h)) then
  --       new_perm_offset(h) <= (SIMD_RD_BYTES_wire(h) - to_integer(unsigned(HVSIZE_READ_lat(h))))*8;
  --     end if;
    
  --   end loop;
  -- end process;

  -- shift_amount(f) <= to_integer(unsigned(hdcu_in_perm_operand_1(f)));
  -- fsm_DSP_perm : process(clk_i_MCR,rst_ni)
  -- variable h : integer;
  
  -- begin
  --   if rst_ni = '0' then
  --     buffer_reg <= (others=>(others => '0'));
  --     hdcu_out_perm_results(f)    <=  (others => '0');
      
  --   elsif rising_edge(clk_i) then
      
  --     for g in 0 to (ACCL_NUM - FU_NUM) loop
        
  --       if multithreaded_accl_en = 1 then -- Assicurarsi che sia std_logic
  --         h := g;  -- set the spm rd/wr ports equal to the "for-loop"
  --       elsif multithreaded_accl_en = 0 then -- Assicurarsi che sia std_logic
  --         h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
  --       end if;

  --       if halt_dsp_lat(h) = '0' and perm_en(h) = '1' and (perm_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
          
  --         -- Default Assignment for the hdcu_out_perm_results
  --         hdcu_out_perm_results(f)(SIMD_Width-1 downto SIMD_Width  - shift_amount(f))  <=  buffer_reg(f)(Data_Width -1 downto Data_Width - shift_amount(f));  -- Bit più significativi
  --         hdcu_out_perm_results(f)(SIMD_Width - 1 - shift_amount(f) downto 0 )         <= hdcu_in_perm_operand_0(f)(SIMD_Width-1 downto shift_amount(f));    -- Bit meno significativi
          
  --         -- Default Assignment for the buffer_reg
  --         buffer_reg(f)(Data_Width-1 downto Data_Width - shift_amount(f)) <=  hdcu_in_perm_operand_0(f)(shift_amount(f) - 1 downto 0);
          
  --         -- If the number of bytes to permute is less than the SIMD_RD_BYTES_wire, we update the assignment properly
  --         if (to_integer(unsigned(HVSIZE_READ_lat(h)))/=0 and to_integer(unsigned(HVSIZE_READ_lat(h))) <= SIMD_RD_BYTES_wire(h)) or to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire(h) then
                
  --               buffer_reg(f)(Data_Width-1 downto Data_Width - shift_amount(f)) <= 
  --                                           hdcu_in_perm_operand_0(f)(shift_amount(f) + new_perm_offset(h) - 1 downto new_perm_offset(h));      

  --               hdcu_out_perm_results(f)(SIMD_Width-1 downto new_perm_offset(h))  <= buffer_reg(f)(Data_Width -1 downto Data_Width - shift_amount(f)) &
  --                                             hdcu_in_perm_operand_0(f)(SIMD_Width-1 downto shift_amount(f)+new_perm_offset(h));  
  --         end if;

  --       end if;
  --     end loop;
  --   end if;
  -- end process;

 --------------------------------------------------------------------------------------------------------------------------
 --    █████╗ ███████╗███████╗       ███████╗███████╗ █████╗ ██████╗  ██████╗██╗  ██╗    ██╗   ██╗███╗   ██╗██╗████████╗ --
 --   ██╔══██╗██╔════╝██╔════╝       ██╔════╝██╔════╝██╔══██╗██╔══██╗██╔════╝██║  ██║    ██║   ██║████╗  ██║██║╚══██╔══╝ --
 --   ███████║███████╗███████╗       ███████╗█████╗  ███████║██████╔╝██║     ███████║    ██║   ██║██╔██╗ ██║██║   ██║    -- 
 --   ██╔══██║╚════██║╚════██║       ╚════██║██╔══╝  ██╔══██║██╔══██╗██║     ██╔══██║    ██║   ██║██║╚██╗██║██║   ██║    --
 --   ██║  ██║███████║███████║██╗    ███████║███████╗██║  ██║██║  ██║╚██████╗██║  ██║    ╚██████╔╝██║ ╚████║██║   ██║    --
 --   ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝    ╚══════╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝     ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --
 --------------------------------------------------------------------------------------------------------------------------
  
  fsm_HDCU_as : process(clk_i,rst_ni)          
  variable h : integer;

  begin
    if rst_ni = '0' then
      -- sim_class            <= (others => 0);
      temp_best_sim_class   <= (others => 0); -- Worst case scenario
      temp_best_class_index <= (others => 0);
      sim_levels_search<= (others => 0);

    elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then 
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then 
          h := f;  -- set the spm rd/wr ports equal to the "for-generate",
        end if;

        if halt_dsp_lat(h) = '0' and as_en(h) = '1' and (as_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
          
          -- Quando ho finito di leggere un class Vector....
          if to_integer(unsigned(fake_HVSIZE_READ_2_MCR(h))) = 0  and to_integer(unsigned(HVSIZE_READ_lat_MCR(h))) /= 0 then -- distance_accumulator_wire diventa 0 quando HVSIZE_READ(h) diventa 0. Quindi per evitare che 0 venga preso come valore di similarity devo evitare di entrare nell'if quando HVSIZE_READ(h) = 0
            -- -- Quando la dimensione dell'ipervettore è inferiore quella che sono in grado di leggere ad ogni ciclo di clock          
            -- if to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire(h) then
            -- -- Devo sommare le parziali che sono presenti in partial_sim_measure.
            -- -- Il numero di parziali presenti nel segnale è dato da SIMD_Width/(HVSIZE*8)
            -- -- A esempio se ho SIMD = 4, avrò 2 parziali che sono contenute da 0 a 63 (devo sommare 0 31 e 32 63 per ottenere la prima parziale
            -- -- e da 64 a 127 per ottenere la seconda parziale (devo sommare 64 95 e 96 127)

            -- -- In pratica voglio fare cosi:
            -- -- Similarity tra l'encoded vector e la prima classe = partial_sim_measure(h)(32 downto 0) + partial_sim_measure(h)(64 downto 32) 
            -- -- Similarity tra l'encoded vector e la seconda classe =  partial_sim_measure(h)(96 downto 64) + partial_sim_measure(h)(128 downto 96)
            -- -- A questo punto devo stabilire quale tra le due è la migliore (la più bassa) e assegnare a class_index il valore corretto della classe che
            -- -- ha prodotto la similarity migliore
            
            -- for i in 0 to (SIMD/2)-1 loop
            --   if to_integer(unsigned(partial_sim_measure(h)(Data_Width*(i+1)-1 downto Data_Width*i))) < to_integer(unsigned(partial_sim_measure(h)(Data_Width*(i+2)-1 downto Data_Width*(i+1)))) then
            --     distance_accumulator_wire(h)(Data_Width*(i+1)-1 downto Data_Width*i) <= partial_sim_measure(h)(Data_Width*(i+1)-1 downto Data_Width*i);
            --     class_index(h)(Data_Width*(i+1)-1 downto Data_Width*i) <= i;
            --   else
            --     distance_accumulator_wire(h)(Data_Width*(i+1)-1 downto Data_Width*i) <= partial_sim_measure(h)(Data_Width*(i+2)-1 downto Data_Width*(i+1));
            --     class_index(h)(Data_Width*(i+1)-1 downto Data_Width*i) <= i+1;
            --   end if;
            -- end loop;
            
            -- else

            -- sim_class(h) <= distance_accumulator_wire(h); -- Lo tengo per debuggare (controllo se la similarity per ciascuna classe viene uguale a quella software)

            --end if;

            sim_levels_search(h) <= sim_levels_search_wire(h);


            -- Se la similarity è migliore di quella attuale la sostituisco e campiono l'indice della classe che ha prodotto la similarity
            if temp_best_sim_class(h) < to_integer(unsigned(distance_accumulator_wire(h)))then
              temp_best_sim_class(h)   <= to_integer(unsigned(distance_accumulator_wire(h)));
              temp_best_class_index(h) <= class_index(h) - 1 ;
            end if;

          end if;

        end if;
      end loop;
    end if;
  end process;

end generate FU_replicated;
end MCR;
--------------------------------- END of MCR architecture ------------------------------------

