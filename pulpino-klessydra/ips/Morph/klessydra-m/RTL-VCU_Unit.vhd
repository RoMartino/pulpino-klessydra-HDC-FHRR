--------------------------------------------------------------------------------------------------------------
--  VCU Unit --                                                                                             --
--  Author(s): Abdallah Cheikh abdallah.cheikh@uniroma1.it (abdallah93.as@gmail.com)                        --
--                                                                                                          --
--  Date Modified: 07-04-2020                                                                               --
--------------------------------------------------------------------------------------------------------------
--  The processing pipeline encapsulates all the componenets containing the datapath of the instruction     --
--  Also in this entity there is a non-synthesizable instruction tracer that displays the trace of all the  --
--  the instructions entering the pipe.                                                                     --
--------------------------------------------------------------------------------------------------------------

-- ieee packages ------------
library ieee;
use ieee.math_real.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;
use std.textio.all;

-- local packages ------------
use work.riscv_klessydra.all;
--use work.klessydra_parameters.all;

entity VCU is
  generic(
    THREAD_POOL_SIZE       : natural;
    accl_en                : natural;
    replicate_accl_en      : natural;
    multithreaded_accl_en  : natural;
    SPM_NUM                : natural;
    Addr_Width             : natural;
    SIMD                   : natural;
    --------------------------------
    ACCL_NUM               : natural;
    FU_NUM                 : natural;
    TPS_CEIL               : natural;
    TPS_BUF_CEIL           : natural;
    SPM_ADDR_WID           : natural;
    SIMD_BITS              : natural;
    Data_Width             : natural;
    SIMD_Width             : natural;
    HDCU_PERF_EN           : natural;
    --------------------------------  
    -- MCR
    ACC_SIMD_BITS          : natural;
    MCR_Acc_SIMD_Width     : natural;
    FP_Width_MCR           : natural; -- Fixed Point Width
    FP_frac_MCR            : natural;  -- Fixed Point Fractional Bits
    --Accl enable
    -- [R3-2 BUG2 fix v3] changed from std_logic to natural (0/1): VHDL
    -- std_logic-typed generics were found NOT to bind reliably when driven
    -- by name from a SystemVerilog instantiation in this Questa
    -- mixed-language flow -- an interactive vsim probe confirmed
    -- BSC_en/FHRR_en/MCR_en/DSP_en resolved to 'U'/'X' at elaboration even
    -- with explicit `.name(1'b0)`/`.name(1'b1)` associations from
    -- core_region.sv (v1/v2 candidate fixes), while natural-typed generics
    -- in the very same generic map (e.g. accl_en, SIMD, THREAD_POOL_SIZE)
    -- bound correctly. See revision_artifacts/R3-2_systemlevel/BUG2_diagnosis.md.
    BSC_en                : natural := 0;
    FHRR_en               : natural := 0;
    MCR_en                : natural := 0;
    DSP_en                : natural := 0
  );
  
  port (
  -- HDC
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
    -- HDCU Performance counter signals
    hdcu_performance_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_bind_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_bundle_perf_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_clip_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_sim_perf_counter      : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_perm_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_search_perf_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
  -- Program Counter Signals
    hdc_taken_branch_BSC       : out  std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_taken_branch_MCR       : out std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_taken_branch_FHRR      : out  std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_taken_branch_DSP       : out  std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_except_condition       : out std_logic_vector(ACCL_NUM-1 downto 0);
    -- ID_Stage Signals
    decoded_instruction        : in  std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0);
    decoded_instruction_FHRR   : in  std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0);
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
    -- tracer signals
    state_HDC                  : out array_2d(ACCL_NUM-1 downto 0)(1 downto 0);
    -- SPMI specific
    data_rvalid_i              : in  std_logic;
    state_LS                   : in  fsm_LS_states;
    sc_word_count_wire         : in  integer;
    spm_bcast                  : in  std_logic;
    harc_LS_wire               : in  integer range ACCL_NUM-1 downto 0;
    ls_sc_data_write_wire      : in  std_logic_vector(Data_Width-1 downto 0);
    ls_sc_read_addr            : in  std_logic_vector(Addr_Width-(SIMD_BITS+3) downto 0);
    ls_sc_write_addr           : in  std_logic_vector(Addr_Width-(SIMD_BITS+3) downto 0);
    ls_sci_req                 : in  std_logic_vector(SPM_NUM-1 downto 0);
    ls_sci_we                  : in  std_logic_vector(SPM_NUM-1 downto 0);
    kmemld_inflight            : in  std_logic_vector(SPM_NUM-1 downto 0);
    kmemstr_inflight           : in  std_logic_vector(SPM_NUM-1 downto 0);
    ls_sc_data_read_wire       : out std_logic_vector(Data_Width-1 downto 0);
    ls_sci_wr_gnt              : out std_logic;
    ls_data_gnt_i              : out std_logic_vector(SPM_NUM-1 downto 0);
    --------------------------------------------------------------------------------------
    -- FHRR
    -- counter signal declarations:
    hdcu_enc_perf_counter           : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);  
    -- Scratchpad Interface Signals
    hdc_data_gnt_i                  : in  std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_sci_wr_gnt                  : in  std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_sc_data_read_FHRR           : in  array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(SIMD_Width-1 downto 0);
    hdc_we_word                     : out array_2d(ACCL_NUM-1 downto 0)(SIMD-1 downto 0);
    hdc_sc_read_addr                : out array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(Addr_Width-1 downto 0);
    hdc_to_sc                       : out array_3d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0)(1 downto 0);
    hdc_sc_data_write_wire          : out array_2d(ACCL_NUM-1 downto 0)(SIMD_Width-1 downto 0);
    hdc_sc_write_addr_FHRR          : out array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
    hdc_sci_we                      : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    hdc_sci_req_FHRR                : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    --------------------------------------------------------------------------------------
    -- MCR
  -- Scratchpad Interface Signals
    hdc_we_word_MCR                 : out array_2d(ACCL_NUM-1 downto 0)(SIMD-1 downto 0);
    dsp_acc_we_word_MCR             : out array_2d(ACCL_NUM-1 downto 0)((MCR_Acc_SIMD_Width/32)-1 downto 0);
    hdc_sc_read_addr_MCR            : out array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(Addr_Width-1 downto 0);
    hdc_to_sc_MCR                   : out array_3d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0)(1 downto 0);
    hdc_sc_data_write_wire_MCR      : out array_2d(ACCL_NUM-1 downto 0)(SIMD_Width-1 downto 0);
    dsp_sc_acc_data_write_wire_MCR  : out array_2d(ACCL_NUM-1 downto 0)(MCR_Acc_SIMD_Width-1 downto 0);
    hdc_sc_write_addr_MCR           : out array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
    hdc_sci_we_MCR                  : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    hdc_sci_req_MCR                 : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    -- tracer signals
    sc_acc_word_count_wire_MCR      : in  integer;
    ls_sc_acc_read_addr_MCR         : in  std_logic_vector(Addr_Width-(ACC_SIMD_BITS+3) downto 0);
    ls_sc_acc_write_addr_MCR        : in  std_logic_vector(Addr_Width-(ACC_SIMD_BITS+3) downto 0)
  );
  end entity;

  architecture Coprocesso of VCU is

  subtype harc_range     is natural range THREAD_POOL_SIZE - 1 downto 0;
  subtype accl_range     is integer range ACCL_NUM-1 downto 0;
  subtype fu_range       is integer range FU_NUM - 1 downto 0;

  signal hdc_data_gnt_int           : std_logic_vector(accl_range);
  signal hdc_sci_wr_gnt_int         : std_logic_vector(accl_range);
  signal hdc_sc_data_read           : array_3d(accl_range)(1 downto 0)(SIMD_Width-1 downto 0);
  signal hdc_sc_write_addr          : array_2d(accl_range)(Addr_Width-1 downto 0);
  signal hdc_sci_req                : array_2d(accl_range)(SPM_NUM-1 downto 0);
  signal dsp_data_gnt_i             : std_logic_vector(accl_range);
  signal dsp_sci_wr_gnt             : std_logic_vector(accl_range);
  signal dsp_sc_data_read           : array_3d(accl_range)(1 downto 0)(SIMD_Width-1 downto 0);
  signal dsp_sc_acc_data_read       : array_3d(accl_range)(1 downto 0)(MCR_Acc_SIMD_Width-1 downto 0);

  component HDC_Unit is
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
  );
  port (
    -- Core Signals
    clk_i, rst_ni              : in std_logic;
    -- Processing Pipeline Signals
    rs1_to_sc                  : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
    rs2_to_sc                  : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
    rd_to_sc                   : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
    -- CSR Signals
    HVSIZE                     : in  array_2d(harc_range)(Addr_Width downto 0);
    MVTYPE                     : in  array_2d(harc_range)(3 downto 0);
    MPSCLFAC                   : in  array_2d(harc_range)(4 downto 0);
    hdc_except_data            : out array_2d(accl_range)(31 downto 0);
    -- HDCU performance counters
    hdcu_performance_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0); 
    hdcu_bind_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_bundle_perf_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_clip_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_sim_perf_counter      : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_perm_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_search_perf_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    ----------------------------------------------------------------------------
    -- Program Counter Signals
    hdc_taken_branch_BSC       : out std_logic_vector(accl_range);
    hdc_except_condition       : out std_logic_vector(accl_range);
      -- ID_Stage Signals
    decoded_instruction        : in  std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0);
    harc_EXEC                  : in  natural range THREAD_POOL_SIZE-1 downto 0;
    pc_IE                      : in  std_logic_vector(31 downto 0);
    RS1_Data_IE                : in  std_logic_vector(31 downto 0);
    RS2_Data_IE                : in  std_logic_vector(31 downto 0);
    RD_Data_IE                 : in  std_logic_vector(Addr_Width -1 downto 0);
    hdc_instr_req              : in  std_logic_vector(accl_range);
    spm_rs1                    : in  std_logic;
    spm_rs2                    : in  std_logic;
    vec_read_rs1_ID            : in  std_logic;
    vec_read_rs2_ID            : in  std_logic;
    vec_write_rd_ID            : in  std_logic;
    busy_HDC                   : out std_logic_vector(accl_range);
  -- Scratchpad Interface Signals
    hdc_data_gnt_i             : in  std_logic_vector(accl_range);
    hdc_sci_wr_gnt             : in  std_logic_vector(accl_range);
    hdc_sc_data_read           : in  array_3d(accl_range)(1 downto 0)(SIMD_Width-1 downto 0);
    hdc_we_word                : out array_2d(accl_range)(SIMD-1 downto 0);
    hdc_sc_read_addr           : out array_3d(accl_range)(1 downto 0)(Addr_Width-1 downto 0);
    hdc_to_sc                  : out array_3d(accl_range)(SPM_NUM-1 downto 0)(1 downto 0);
    hdc_sc_data_write_wire     : out array_2d(accl_range)(SIMD_Width-1 downto 0);
    hdc_sc_write_addr          : out array_2d(accl_range)(Addr_Width-1 downto 0);
    hdc_sci_we                 : out array_2d(accl_range)(SPM_NUM-1 downto 0);
    hdc_sci_req                : out array_2d(accl_range)(SPM_NUM-1 downto 0);
    -- tracer signals
    state_HDC                  : out array_2d(accl_range)(1 downto 0)
  );
  end component;  ------------------------------------------

  component FHRR_Unit is
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
    hdc_taken_branch_FHRR      : out std_logic_vector(accl_range);
    hdc_except_condition       : out std_logic_vector(accl_range);
    -- ID_Stage Signals
    decoded_instruction_FHRR   : in  std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0);
    harc_EXEC                  : in  natural range THREAD_POOL_SIZE-1 downto 0;
    pc_IE                      : in  std_logic_vector(31 downto 0);
    RS1_Data_IE                : in  std_logic_vector(31 downto 0);
    RS2_Data_IE                : in  std_logic_vector(31 downto 0);
    RD_Data_IE                 : in  std_logic_vector(Addr_Width -1 downto 0);
    hdc_instr_req              : in  std_logic_vector(accl_range);
    spm_rs1                    : in  std_logic;
    spm_rs2                    : in  std_logic;
    vec_read_rs1_ID            : in  std_logic;
    vec_read_rs2_ID            : in  std_logic;
    vec_write_rd_ID            : in  std_logic;
    busy_HDC                   : out std_logic_vector(accl_range);

    -- counter signal declarations:
    hdcu_performance_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0); 
    hdcu_bind_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_bundle_perf_counter   : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_clip_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_sim_perf_counter      : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_perm_perf_counter     : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_enc_perf_counter      : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
  
    -- Scratchpad Interface Signals
    hdc_data_gnt_i                  : in  std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_sci_wr_gnt                  : in  std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_sc_data_read                : in  array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(SIMD_Width-1 downto 0);
    hdc_we_word                     : out array_2d(ACCL_NUM-1 downto 0)(SIMD-1 downto 0);
    hdc_sc_read_addr                : out array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(Addr_Width-1 downto 0);
    hdc_to_sc                       : out array_3d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0)(1 downto 0);
    hdc_sc_data_write_wire          : out array_2d(ACCL_NUM-1 downto 0)(SIMD_Width-1 downto 0);
    hdc_sc_write_addr               : out array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
    hdc_sci_we                      : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    hdc_sci_req                     : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    -- tracer signals
    state_HDC                  : out array_2d(accl_range)(1 downto 0)
  );
  end component;
  
  component MCR_Unit is
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
    FP_frac_MCR           : natural
   );
   
   port (
    -- Core Signals  
    clk_i, rst_ni                 : in std_logic;
  -- Processing Pipeline Signals
    rs1_to_sc                     : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
    rs2_to_sc                     : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
    rd_to_sc                      : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
  -- CSR Signals
    HVSIZE                        : in  array_2d(THREAD_POOL_SIZE-1 downto 0)(Addr_Width downto 0);
    MVTYPE                        : in  array_2d(THREAD_POOL_SIZE-1 downto 0)(3 downto 0);
    MPSCLFAC                      : in  array_2d(THREAD_POOL_SIZE-1 downto 0)(4 downto 0);
    hdc_except_data               : out array_2d(ACCL_NUM-1 downto 0)(31 downto 0);

    hdcu_performance_counter      : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0); 
    hdcu_bind_perf_counter        : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_bundle_perf_counter      : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_clip_perf_counter        : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_sim_perf_counter         : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_perm_perf_counter        : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    hdcu_search_perf_counter      : out array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);
    

  -- Program Counter Signals
    hdc_taken_branch_MCR           : out std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_except_condition           : out std_logic_vector(ACCL_NUM-1 downto 0);
  -- ID_Stage Signals
    decoded_instruction_MCR        : in  std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0);
    harc_EXEC                      : in  natural range THREAD_POOL_SIZE-1 downto 0;
    pc_IE                          : in  std_logic_vector(31 downto 0);
    RS1_Data_IE                    : in  std_logic_vector(31 downto 0);
    RS2_Data_IE                    : in  std_logic_vector(31 downto 0);
    RD_Data_IE                     : in  std_logic_vector(Addr_Width -1 downto 0);
    hdc_instr_req                  : in  std_logic_vector(ACCL_NUM-1 downto 0);
    spm_rs1                        : in  std_logic;
    spm_rs2                        : in  std_logic;
    vec_read_rs1_ID                : in  std_logic;
    vec_read_rs2_ID                : in  std_logic;
    vec_write_rd_ID                : in  std_logic;
    busy_hdc                       : out std_logic_vector(ACCL_NUM-1 downto 0);
  -- Scratchpad Interface Signals
    dsp_data_gnt_i                 : in  std_logic_vector(ACCL_NUM-1 downto 0);
    dsp_sci_wr_gnt                 : in  std_logic_vector(ACCL_NUM-1 downto 0);
    dsp_sc_data_read               : in  array_3d(accl_range)(1 downto 0)(SIMD_Width-1 downto 0);
    dsp_sc_acc_data_read           : in  array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(MCR_Acc_SIMD_Width-1 downto 0);
    hdc_we_word_MCR                : out array_2d(ACCL_NUM-1 downto 0)(SIMD-1 downto 0);
    dsp_acc_we_word_MCR            : out array_2d(ACCL_NUM-1 downto 0)((MCR_Acc_SIMD_Width/32)-1 downto 0);

    hdc_sc_read_addr_MCR           : out array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(Addr_Width-1 downto 0);
    hdc_to_sc_MCR                  : out array_3d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0)(1 downto 0);
    hdc_sc_data_write_wire_MCR     : out array_2d(ACCL_NUM-1 downto 0)(SIMD_Width-1 downto 0);
    dsp_sc_acc_data_write_wire_MCR : out array_2d(ACCL_NUM-1 downto 0)(MCR_Acc_SIMD_Width-1 downto 0);
    hdc_sc_write_addr_MCR          : out array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
    hdc_sci_we_MCR                 : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    hdc_sci_req_MCR                : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    -- tracer signals
    state_HDC                      : out array_2d(ACCL_NUM-1 downto 0)(1 downto 0)
   );
   end component;

  component DSP_Unit is
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
    SIMD_Width            : natural  -- Fixed Point Fractional Bits
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
    hdc_taken_branch_DSP       : out std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_except_condition       : out std_logic_vector(ACCL_NUM-1 downto 0);
    -- ID_Stage Signals
    decoded_instruction    : in  std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0);
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
    hdc_we_word                : out array_2d(ACCL_NUM-1 downto 0)(SIMD-1 downto 0);
    hdc_sc_read_addr           : out array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(Addr_Width-1 downto 0);
    hdc_to_sc                  : out array_3d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0)(1 downto 0);
    hdc_sc_data_write_wire     : out array_2d(ACCL_NUM-1 downto 0)(SIMD_Width-1 downto 0);
    hdc_sc_write_addr          : out array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
    hdc_sci_we                 : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    hdc_sci_req                : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    -- tracer signals
    state_DSP                  : out array_2d(ACCL_NUM-1 downto 0)(1 downto 0)
  );
  end component;  
  
  component Scratchpad_memory_interface is
  generic(
    accl_en                    : natural;
    SPM_NUM                    : natural; 
    Addr_Width                 : natural;
    SIMD                       : natural;
    -------------------------------------
    ACCL_NUM                   : natural;
    SIMD_BITS                  : natural;
    Data_Width                 : natural;
    SIMD_Width                 : natural
  );
  port (
    clk_i, rst_ni              : in  std_logic;
    data_rvalid_i              : in  std_logic;
    state_LS                   : in  fsm_LS_states;
    sc_word_count_wire         : in  integer;
    spm_bcast                  : in  std_logic;
    harc_LS_wire               : in  integer range ACCL_NUM-1 downto 0;
    hdc_we_word                : in  array_2d(accl_range)(SIMD-1 downto 0);
    ls_sc_data_write_wire      : in  std_logic_vector(Data_Width-1 downto 0);
    hdc_sc_data_write_wire     : in  array_2d(accl_range)(SIMD_Width-1 downto 0);
    ls_sc_read_addr            : in  std_logic_vector(Addr_Width-(SIMD_BITS+3) downto 0);
    ls_sc_write_addr           : in  std_logic_vector(Addr_Width-(SIMD_BITS+3) downto 0);
    hdc_sc_write_addr          : in  array_2d(accl_range)(Addr_Width-1 downto 0);
    ls_sci_req                 : in  std_logic_vector(SPM_NUM-1 downto 0);
    ls_sci_we                  : in  std_logic_vector(SPM_NUM-1 downto 0);
    hdc_sci_req                : in  array_2d(accl_range)(SPM_NUM-1 downto 0);
    hdc_sci_we                 : in  array_2d(accl_range)(SPM_NUM-1 downto 0);
    kmemld_inflight            : in  std_logic_vector(SPM_NUM-1 downto 0);
    kmemstr_inflight           : in  std_logic_vector(SPM_NUM-1 downto 0);
    hdc_to_sc                  : in  array_3d(accl_range)(SPM_NUM-1 downto 0)(1 downto 0);
    hdc_sc_read_addr           : in  array_3d(accl_range)(1 downto 0)(Addr_Width-1 downto 0);
    hdc_sc_data_read           : out array_3d(accl_range)(1 downto 0)(SIMD_Width-1 downto 0);
    ls_sc_data_read_wire       : out std_logic_vector(Data_Width-1 downto 0);
    ls_sci_wr_gnt              : out std_logic;
    hdc_sci_wr_gnt             : out std_logic_vector(accl_range);
    ls_data_gnt_i              : out std_logic_vector(SPM_NUM-1 downto 0);
    hdc_data_gnt_i             : out std_logic_vector(accl_range)
  );
  end component; 
  
  component Scratchpad_memory_interface_MCR is 
  generic(
    accl_en               : natural;
    SPM_NUM		            : natural; 
    Addr_Width            : natural;
    SIMD                  : natural;
    --------------------------------
    ACCL_NUM              : natural;
    SIMD_BITS             : natural;
    Data_Width            : natural;
    SIMD_Width            : natural;
    MCR_Acc_SIMD_Width    : natural;
    ACC_SIMD_BITS         : natural
  );
  
  port ( 
    clk_i, rst_ni                   : in  std_logic;
    data_rvalid_i                   : in  std_logic;
    state_LS                        : in  fsm_LS_states;
    sc_word_count_wire              : in  integer;
    sc_acc_word_count_wire_MCR      : in  integer;
    spm_bcast                       : in  std_logic;
    harc_LS_wire                    : in  integer range ACCL_NUM-1 downto 0;
    hdc_we_word_MCR                 : in  array_2d(ACCL_NUM-1 downto 0)(SIMD-1 downto 0);
    dsp_acc_we_word_MCR             : in  array_2d(ACCL_NUM-1 downto 0)((MCR_Acc_SIMD_Width/32)-1 downto 0);
    ls_sc_data_write_wire           : in  std_logic_vector(Data_Width-1 downto 0);
    hdc_sc_data_write_wire_MCR      : in  array_2d(ACCL_NUM-1 downto 0)(SIMD_Width-1 downto 0);
    dsp_sc_acc_data_write_wire_MCR  : in  array_2d(ACCL_NUM-1 downto 0)(MCR_Acc_SIMD_Width-1 downto 0);
    ls_sc_read_addr                 : in  std_logic_vector(Addr_Width-(SIMD_BITS+3) downto 0);
    ls_sc_acc_read_addr_MCR         : in  std_logic_vector(Addr_Width-(ACC_SIMD_BITS+3) downto 0);
    ls_sc_write_addr                : in  std_logic_vector(Addr_Width-(SIMD_BITS+3) downto 0);
    ls_sc_acc_write_addr_MCR        : in  std_logic_vector(Addr_Width-(ACC_SIMD_BITS+3) downto 0);
    hdc_sc_write_addr_MCR           : in  array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
    ls_sci_req                      : in  std_logic_vector(SPM_NUM-1 downto 0);
    ls_sci_we                       : in  std_logic_vector(SPM_NUM-1 downto 0);
    hdc_sci_req_MCR                 : in  array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    hdc_sci_we_MCR                  : in  array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    kmemld_inflight                 : in  std_logic_vector(SPM_NUM-1 downto 0);
    kmemstr_inflight                : in  std_logic_vector(SPM_NUM-1 downto 0);
    hdc_to_sc_MCR                   : in  array_3d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0)(1 downto 0);
    hdc_sc_read_addr_MCR            : in  array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(Addr_Width-1 downto 0);
    dsp_sc_data_read                : out array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(SIMD_Width-1 downto 0);
    dsp_sc_acc_data_read            : out array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(MCR_Acc_SIMD_Width-1 downto 0);
    ls_sc_data_read_wire            : out std_logic_vector(Data_Width-1 downto 0);
    ls_sci_wr_gnt                   : out std_logic;
    dsp_sci_wr_gnt                  : out std_logic_vector(ACCL_NUM-1 downto 0);
    ls_data_gnt_i                   : out std_logic_vector(SPM_NUM-1 downto 0);
    dsp_data_gnt_i                  : out std_logic_vector(ACCL_NUM-1 downto 0)
	);
  end component; ------------------------------------------

  begin

-- Default drivers for the hdc_taken_branch_* outputs that are not driven by the
-- selected accelerator sub-unit. Without these, the unused outputs stay undriven
-- ('U' in simulation), and "unsigned(...) /= 0" in the Processing Pipeline evaluates
-- to true, forcing taken_branch permanently to '1'. Each "off" generate is the exact
-- complement of the matching sub-unit generate, so every output always has one driver.
gen_HDC_off: if not(BSC_en=1 and not(FHRR_en=1 or MCR_en=1 or DSP_en=1)) generate
  hdc_taken_branch_BSC  <= (others => '0');
end generate;

gen_FHRR_off: if not(FHRR_en=1 and not(BSC_en=1 or MCR_en=1 or DSP_en=1)) generate
  hdc_taken_branch_FHRR <= (others => '0');
end generate;

gen_MCRC_off: if not(MCR_en=1 and not(FHRR_en=1 or BSC_en=1 or DSP_en=1)) generate
  hdc_taken_branch_MCR  <= (others => '0');
end generate;

gen_DSP_off: if not(DSP_en=1 and not(FHRR_en=1 or MCR_en=1 or BSC_en=1)) generate
  hdc_taken_branch_DSP  <= (others => '0');
end generate;

gen_HDC: if BSC_en=1 and not(FHRR_en=1 or MCR_en=1 or DSP_en=1) generate
  HDC : HDC_Unit
  generic map(
    THREAD_POOL_SIZE           => THREAD_POOL_SIZE, 
    accl_en                    => accl_en, 
    replicate_accl_en          => replicate_accl_en, 
    multithreaded_accl_en      => multithreaded_accl_en, 
    SPM_NUM                    => SPM_NUM,  
    Addr_Width                 => Addr_Width, 
    SIMD                       => SIMD, 
    HDCU_PERF_EN               => HDCU_PERF_EN,
    --------------------------------
    ACCL_NUM                   => ACCL_NUM, 
    FU_NUM                     => FU_NUM, 
    TPS_CEIL                   => TPS_CEIL, 
    TPS_BUF_CEIL               => TPS_BUF_CEIL, 
    SPM_ADDR_WID               => SPM_ADDR_WID, 
    SIMD_BITS                  => SIMD_BITS, 
    Data_Width                 => Data_Width, 
    SIMD_Width                 => SIMD_Width
  )
  port map(
    clk_i                      => clk_i,
    rst_ni                     => rst_ni,
    rs1_to_sc                  => rs1_to_sc,
    rs2_to_sc                  => rs2_to_sc,
    rd_to_sc                   => rd_to_sc,
    HVSIZE                     => HVSIZE,
    MVTYPE                     => MVTYPE,
    MPSCLFAC                   => MPSCLFAC,
    hdc_except_data            => hdc_except_data,

    -- Performance counter
    hdcu_performance_counter   => hdcu_performance_counter,
    hdcu_bind_perf_counter     => hdcu_bind_perf_counter,
    hdcu_bundle_perf_counter   => hdcu_bundle_perf_counter,
    hdcu_clip_perf_counter     => hdcu_clip_perf_counter,
    hdcu_sim_perf_counter      => hdcu_sim_perf_counter,
    hdcu_perm_perf_counter     => hdcu_perm_perf_counter,
    hdcu_search_perf_counter   => hdcu_search_perf_counter,
    hdc_taken_branch_BSC       => hdc_taken_branch_BSC,
    hdc_except_condition       => hdc_except_condition,
    decoded_instruction        => decoded_instruction,
    harc_EXEC                  => harc_EXEC,
    pc_IE                      => pc_IE,
    RS1_Data_IE                => RS1_Data_IE,
    RS2_Data_IE                => RS2_Data_IE,
    RD_Data_IE                 => RD_Data_IE,
    hdc_instr_req              => hdc_instr_req,
    spm_rs1                    => spm_rs1,
    spm_rs2                    => spm_rs2,
    vec_read_rs1_ID            => vec_read_rs1_ID,
    vec_read_rs2_ID            => vec_read_rs2_ID,
    vec_write_rd_ID            => vec_write_rd_ID,
    busy_HDC                   => busy_HDC,
    hdc_data_gnt_i             => hdc_data_gnt_i,
    hdc_sci_wr_gnt             => hdc_sci_wr_gnt,
    hdc_sc_data_read           => hdc_sc_data_read,
    hdc_sc_read_addr           => hdc_sc_read_addr,
    hdc_to_sc                  => hdc_to_sc,
    hdc_sc_data_write_wire     => hdc_sc_data_write_wire,
    hdc_we_word                => hdc_we_word,
    hdc_sc_write_addr          => hdc_sc_write_addr,
    hdc_sci_we                 => hdc_sci_we,
    hdc_sci_req                => hdc_sci_req
  );
end generate;

gen_FHRR: if FHRR_en=1 and not(BSC_en=1 or MCR_en=1 or DSP_en=1) generate
  FHRR : FHRR_Unit
  generic map(
    THREAD_POOL_SIZE               => THREAD_POOL_SIZE,
    accl_en                        => accl_en,
    replicate_accl_en              => replicate_accl_en,
    multithreaded_accl_en          => multithreaded_accl_en,
    SPM_NUM                        => SPM_NUM,
    Addr_Width                     => Addr_Width,
    SIMD                           => SIMD,  
    --------------------------------
    ACCL_NUM                       => ACCL_NUM,
    FU_NUM                         => FU_NUM,
    TPS_CEIL                       => TPS_CEIL,
    TPS_BUF_CEIL                   => TPS_BUF_CEIL,
    SPM_ADDR_WID                   => SPM_ADDR_WID,
    SIMD_BITS                      => SIMD_BITS,
    Data_Width                     => Data_Width,
    SIMD_Width                     => SIMD_Width,
    HDCU_PERF_EN                   => HDCU_PERF_EN
  )
  
  port map(
    clk_i                          => clk_i,
    rst_ni                         => rst_ni,
  -- Processing Pipeline Signals        
    rs1_to_sc                      => rs1_to_sc,
    rs2_to_sc                      => rs2_to_sc,
    rd_to_sc                       => rd_to_sc,
  -- CSR Signals
    HVSIZE                         => HVSIZE,                  
    MVTYPE                         => MVTYPE,                 
    MPSCLFAC                       => MPSCLFAC,
    hdc_except_data                => hdc_except_data,               
  -- Program Counter Signals
    hdc_taken_branch_FHRR          => hdc_taken_branch_FHRR,
    hdc_except_condition           => hdc_except_condition,     
  -- ID_Stage Signals
    decoded_instruction_FHRR       => decoded_instruction_FHRR,
    harc_EXEC                      => harc_EXEC,
    pc_IE                          => pc_IE,
    RS1_Data_IE                    => RS1_Data_IE,
    RS2_Data_IE                    => RS2_Data_IE,
    RD_Data_IE                     => RD_Data_IE,
    hdc_instr_req                  => hdc_instr_req,
    spm_rs1                        => spm_rs1,
    spm_rs2                        => spm_rs2,
    vec_read_rs1_ID                => vec_read_rs1_ID,
    vec_read_rs2_ID                => vec_read_rs2_ID,
    vec_write_rd_ID                => vec_write_rd_ID,
    busy_HDC                       => busy_HDC,
  --counter signal declarations:
    hdcu_performance_counter       => hdcu_performance_counter,
    hdcu_bind_perf_counter         => hdcu_bind_perf_counter,
    hdcu_bundle_perf_counter       => hdcu_bundle_perf_counter,
    hdcu_clip_perf_counter         => hdcu_clip_perf_counter,
    hdcu_sim_perf_counter          => hdcu_sim_perf_counter,
    hdcu_perm_perf_counter         => hdcu_perm_perf_counter,
    hdcu_enc_perf_counter          => hdcu_enc_perf_counter,
    -- Scratchpad Interface Signals
    hdc_data_gnt_i                 => hdc_data_gnt_i,
    hdc_sci_wr_gnt                 => hdc_sci_wr_gnt,
    hdc_sc_data_read               => hdc_sc_data_read,
    hdc_we_word                    => hdc_we_word,
    hdc_sc_read_addr               => hdc_sc_read_addr,
    hdc_to_sc                      => hdc_to_sc,
    hdc_sc_data_write_wire         => hdc_sc_data_write_wire,
    hdc_sc_write_addr              => hdc_sc_write_addr,
    hdc_sci_we                     => hdc_sci_we,
    hdc_sci_req                    => hdc_sci_req
   );
end generate;
   
gen_MCRC: if MCR_en=1 and not(FHRR_en=1 or BSC_en=1 or DSP_en=1) generate
  MCR : MCR_Unit
  generic map(  
    THREAD_POOL_SIZE                => THREAD_POOL_SIZE,
    accl_en                         => accl_en,
    replicate_accl_en               => replicate_accl_en,
    multithreaded_accl_en           => multithreaded_accl_en,
    SPM_NUM                         => SPM_NUM,
    Addr_Width                      => Addr_Width,
    SIMD                            => SIMD,
    --------------------------------
    ACCL_NUM                        => ACCL_NUM, 
    FU_NUM                          => FU_NUM,
    TPS_CEIL                        => TPS_CEIL,
    TPS_BUF_CEIL                    => TPS_BUF_CEIL,
    SPM_ADDR_WID                    => SPM_ADDR_WID,
    SIMD_BITS                       => SIMD_BITS,
    ACC_SIMD_BITS                   => ACC_SIMD_BITS,
    Data_Width                      => Data_Width,
    SIMD_Width                      => SIMD_Width,
    MCR_Acc_SIMD_Width              => MCR_Acc_SIMD_Width,
    HDCU_PERF_EN                    => HDCU_PERF_EN,
    FP_Width_MCR                    => FP_Width_MCR,
    FP_frac_MCR                     => FP_frac_MCR
    )
    
   port map(
    clk_i                           => clk_i,
    rst_ni                          => rst_ni,
    rs1_to_sc                       => rs1_to_sc,
    rs2_to_sc                       => rs2_to_sc,
    rd_to_sc                        => rd_to_sc, 
    HVSIZE                          => HVSIZE,
    MVTYPE                          => MVTYPE,
    MPSCLFAC                        => MPSCLFAC,
    hdc_except_data                 => hdc_except_data,
    hdcu_performance_counter        => hdcu_performance_counter,
    hdcu_bind_perf_counter          => hdcu_bind_perf_counter,
    hdcu_bundle_perf_counter        => hdcu_bundle_perf_counter,
    hdcu_clip_perf_counter          => hdcu_clip_perf_counter,
    hdcu_sim_perf_counter           => hdcu_sim_perf_counter,
    hdcu_perm_perf_counter          => hdcu_perm_perf_counter,
    hdcu_search_perf_counter        => hdcu_search_perf_counter,
    hdc_taken_branch_MCR            => hdc_taken_branch_MCR,
    hdc_except_condition            => hdc_except_condition,
    decoded_instruction_MCR         => decoded_instruction_MCR,
    harc_EXEC                       => harc_EXEC,
    pc_IE                           => pc_IE,
    RS1_Data_IE                     => RS1_Data_IE,
    RS2_Data_IE                     => RS2_Data_IE,
    RD_Data_IE                      => RD_Data_IE,
    hdc_instr_req                   => hdc_instr_req,
    spm_rs1                         => spm_rs1,
    spm_rs2                         => spm_rs2,
    vec_read_rs1_ID                 => vec_read_rs1_ID,
    vec_read_rs2_ID                 => vec_read_rs2_ID,
    vec_write_rd_ID                 => vec_write_rd_ID,
    busy_hdc                        => busy_hdc,
    dsp_data_gnt_i                  => dsp_data_gnt_i,
    dsp_sci_wr_gnt                  => dsp_sci_wr_gnt,
    dsp_sc_data_read                => dsp_sc_data_read,
    dsp_sc_acc_data_read            => dsp_sc_acc_data_read,
    hdc_we_word_MCR                 => hdc_we_word_MCR,
    dsp_acc_we_word_MCR             => dsp_acc_we_word_MCR,
    hdc_sc_read_addr_MCR            => hdc_sc_read_addr_MCR, 
    hdc_to_sc_MCR                   => hdc_to_sc_MCR,
    hdc_sc_data_write_wire_MCR      => hdc_sc_data_write_wire_MCR,
    dsp_sc_acc_data_write_wire_MCR  => dsp_sc_acc_data_write_wire_MCR,
    hdc_sc_write_addr_MCR           => hdc_sc_write_addr_MCR,
    hdc_sci_we_MCR                  => hdc_sci_we_MCR,
    hdc_sci_req_MCR                 => hdc_sci_req_MCR
   );
end generate;

gen_DSP: if DSP_en=1 and not(FHRR_en=1 or MCR_en=1 or BSC_en=1) generate
  DSP : DSP_Unit
  generic map(
    THREAD_POOL_SIZE      => THREAD_POOL_SIZE,
    accl_en               => accl_en,
    replicate_accl_en     => replicate_accl_en,
    multithreaded_accl_en => multithreaded_accl_en,
    SPM_NUM               => SPM_NUM,
    Addr_Width            => Addr_Width,
    SIMD                  => SIMD,
    --------------------------------
    ACCL_NUM              => ACCL_NUM,
    FU_NUM                => FU_NUM,
    TPS_CEIL              => TPS_CEIL,
    TPS_BUF_CEIL          => TPS_BUF_CEIL,
    SPM_ADDR_WID          => SPM_ADDR_WID,
    SIMD_BITS             => SIMD_BITS,
    Data_Width            => Data_Width,
    SIMD_Width            => SIMD_Width
  )

  port map(
    clk_i                       => clk_i,
    rst_ni                      => rst_ni,
    rs1_to_sc                   => rs1_to_sc,
    rs2_to_sc                   => rs2_to_sc,
    rd_to_sc                    => rd_to_sc,
    HVSIZE                      => HVSIZE,
    MVTYPE                      => MVTYPE,
    MPSCLFAC                    => MPSCLFAC,
    hdc_except_data             => hdc_except_data,
    hdc_taken_branch_DSP        => hdc_taken_branch_DSP,
    hdc_except_condition        => hdc_except_condition,
    decoded_instruction         => decoded_instruction,
    harc_EXEC                   => harc_EXEC,
    pc_IE                       => pc_IE,
    RS1_Data_IE                 => RS1_Data_IE,
    RS2_Data_IE                 => RS2_Data_IE,
    RD_Data_IE                  => RD_Data_IE,
    hdc_instr_req               => hdc_instr_req,
    spm_rs1                     => spm_rs1,
    spm_rs2                     => spm_rs2,
    vec_read_rs1_ID             => vec_read_rs1_ID,
    vec_read_rs2_ID             => vec_read_rs2_ID,
    vec_write_rd_ID             => vec_write_rd_ID,
    busy_hdc                    => busy_hdc,
    dsp_data_gnt_i              => dsp_data_gnt_i,
    dsp_sci_wr_gnt              => dsp_sci_wr_gnt,
    dsp_sc_data_read            => dsp_sc_data_read,
    hdc_we_word                 => hdc_we_word,
    hdc_sc_read_addr            => hdc_sc_read_addr,
    hdc_to_sc                   => hdc_to_sc,
    hdc_sc_data_write_wire      => hdc_sc_data_write_wire,
    hdc_sc_write_addr           => hdc_sc_write_addr,
    hdc_sci_we                  => hdc_sci_we,
    hdc_sci_req                 => hdc_sci_req
  );
end generate;
 
gen_SCI: if not(MCR_en=1) generate
  SCI : Scratchpad_memory_interface
  generic map(
    accl_en                       => accl_en, 
    SPM_NUM                       => SPM_NUM,  
    Addr_Width                    => Addr_Width, 
    SIMD                          => SIMD, 
    ACCL_NUM                      => ACCL_NUM, 
    SIMD_BITS                     => SIMD_BITS, 
    Data_Width                    => Data_Width, 
    SIMD_Width                    => SIMD_Width
  )
  port map(  
    clk_i                         => clk_i,
    rst_ni                        => rst_ni,
    data_rvalid_i                 => data_rvalid_i,
    state_LS                      => state_LS,
    sc_word_count_wire            => sc_word_count_wire,
    spm_bcast                     => spm_bcast,
    harc_LS_wire                  => harc_LS_wire,
    hdc_we_word                   => hdc_we_word,
    ls_sc_data_write_wire         => ls_sc_data_write_wire,
    hdc_sc_data_write_wire        => hdc_sc_data_write_wire,
    ls_sc_read_addr               => ls_sc_read_addr,
    ls_sc_write_addr              => ls_sc_write_addr,
    hdc_sc_write_addr             => hdc_sc_write_addr,
    ls_sci_req                    => ls_sci_req,
    ls_sci_we                     => ls_sci_we,
    hdc_sci_req                   => hdc_sci_req,
    hdc_sci_we                    => hdc_sci_we,
    kmemld_inflight               => kmemld_inflight,
    kmemstr_inflight              => kmemstr_inflight,
    hdc_to_sc                     => hdc_to_sc,
    hdc_sc_read_addr              => hdc_sc_read_addr,    
    hdc_sc_data_read              => hdc_sc_data_read,
    ls_sc_data_read_wire          => ls_sc_data_read_wire,
    ls_sci_wr_gnt                 => ls_sci_wr_gnt,
    hdc_sci_wr_gnt                => hdc_sci_wr_gnt_int,
    hdc_data_gnt_i                => hdc_data_gnt_int
  ); 
end generate;

gen_SCI_MCR: if MCR_en=1 generate 
  SCI_MCR : Scratchpad_memory_interface_MCR
  generic map(
    accl_en                 => accl_en,
    SPM_NUM		            => SPM_NUM, 
    Addr_Width              => Addr_Width,
    SIMD                    => SIMD,
    --------------------------------
    ACCL_NUM                => ACCL_NUM,
    SIMD_BITS               => SIMD_BITS,
    Data_Width              => Data_Width,
    SIMD_Width              => SIMD_Width,
    MCR_Acc_SIMD_Width      => MCR_Acc_SIMD_Width,
    ACC_SIMD_BITS           => ACC_SIMD_BITS
  )
  port map(
    clk_i                           => clk_i,
    rst_ni                          => rst_ni, 
    data_rvalid_i                   => data_rvalid_i,
    state_LS                        => state_LS,
    sc_word_count_wire              => sc_word_count_wire,
    sc_acc_word_count_wire_MCR      => sc_acc_word_count_wire_MCR,
    spm_bcast                       => spm_bcast,
    harc_LS_wire                    => harc_LS_wire,
    hdc_we_word_MCR                 => hdc_we_word_MCR,   
    dsp_acc_we_word_MCR             => dsp_acc_we_word_MCR, 
    ls_sc_data_write_wire           => ls_sc_data_write_wire,
    hdc_sc_data_write_wire_MCR      => hdc_sc_data_write_wire_MCR,   
    dsp_sc_acc_data_write_wire_MCR  => dsp_sc_acc_data_write_wire_MCR,
    ls_sc_read_addr                 =>  ls_sc_read_addr,
    ls_sc_acc_read_addr_MCR         => ls_sc_acc_read_addr_MCR,
    ls_sc_write_addr                => ls_sc_write_addr,
    ls_sc_acc_write_addr_MCR        => ls_sc_acc_write_addr_MCR,
    hdc_sc_write_addr_MCR           => hdc_sc_write_addr_MCR,
    ls_sci_req                      => ls_sci_req,
    ls_sci_we                       => ls_sci_we,
    hdc_sci_req_MCR                 => hdc_sci_req_MCR,
    hdc_sci_we_MCR                  => hdc_sci_we_MCR,
    kmemld_inflight                 => kmemld_inflight,
    kmemstr_inflight                => kmemstr_inflight,
    hdc_to_sc_MCR                   => hdc_to_sc_MCR,
    hdc_sc_read_addr_MCR            => hdc_sc_read_addr_MCR,
    dsp_sc_data_read                => dsp_sc_data_read,
    dsp_sc_acc_data_read            => dsp_sc_acc_data_read,
    ls_sc_data_read_wire            => ls_sc_data_read_wire,
    ls_sci_wr_gnt                   => ls_sci_wr_gnt,
    dsp_sci_wr_gnt                  => dsp_sci_wr_gnt,
    ls_data_gnt_i                   => ls_data_gnt_i,
    dsp_data_gnt_i                  => dsp_data_gnt_i 
    );
  end generate;

  end architecture;