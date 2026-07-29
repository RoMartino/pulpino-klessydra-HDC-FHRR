--------------------------------------------------------------------------------------------------------------
--  LSU -- (Load-Store Unit)                                                                                --
--  Author(s): Abdallah Cheikh abdallah.cheikh@uniroma1.it (abdallah93.as@gmail.com)                        --
--                                                                                                          --
--  Date Modified: 8-12-2019                                                                                --
--------------------------------------------------------------------------------------------------------------
--  The LSU performs all the operations that access the external memories. Including the amoswap, and the   --
--  custom burst load and store instructions. The LSU can allow superscalar execution with other execution  --
--  units if a store operation is executing, Loaded instructions write either to the regfile or the SPMs    --
--------------------------------------------------------------------------------------------------------------

-- ieee packages ------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;
use std.textio.all;

-- local packages ------------
use work.riscv_klessydra.all;
--use work.klessydra_parameters.all;

-- LD-STR pinout --------------------
entity Load_Store_Unit is
  generic(
    THREAD_POOL_SIZE           : integer;
    accl_en                    : natural;
    replicate_accl_en          : natural;
    SIMD                       : natural;
    SPM_NUM		                 : natural;  
    Addr_Width                 : natural;
    Data_Width                 : natural;
    SIMD_BITS                  : natural;
    ACCL_NUM                   : natural;
    SPM_ADDR_WID               : natural
    );
  port (
    -- clock, and reset active low
    clk_i, rst_ni              : in std_logic;
    -- Program Counter Signals
    irq_pending                : in std_logic_vector(THREAD_POOL_SIZE-1 downto 0);
    -- ID_Stage Signals
    RS1_Data_IE                : in  std_logic_vector(31 downto 0);
    RS2_Data_IE                : in  std_logic_vector(31 downto 0);
    RD_Data_IE                 : in  std_logic_vector(31 downto 0);
    instr_word_IE              : in  std_logic_vector(31 downto 0);
    pc_IE                      : in  std_logic_vector(31 downto 0);
    decoded_instruction_LS     : in  std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0);
    data_be_ID                 : in  std_logic_vector(3 downto 0);
    data_width_ID              : in  std_logic_vector(1 downto 0);
    harc_EXEC                  : in  integer range THREAD_POOL_SIZE-1 downto 0;
    LS_instr_req               : in  std_logic;
    load_op                    : in  std_logic;
    store_op                   : in  std_logic;
    --sw_mip                     : in  std_logic;
    core_busy_LS               : out std_logic;
    core_busy_WB               : in std_logic;
    busy_LS                    : out std_logic;

    -- TTMR Processing Pipeline Signals
--    fault_LSU                  : out std_logic;                      --serve per ripetere l'istruzione di store quando un fault arriva durante l'ultima voted store'
--    restore_fault_LSU          : out std_logic;
    LS_WB_wrong_EXEC           : in  std_logic;
    load_op_buf_wire           : out std_logic_vector(2 downto 0);   
    load_op_buf_lat            : out std_logic_vector(2 downto 0);   
    ls_instr_req_buf_wire      : out std_logic_vector(2 downto 0);   
    ls_instr_req_buf           : out std_logic_vector(2 downto 0);   
    LS_peripheral              : out std_logic; 
    harc_ID                    : in  integer range THREAD_POOL_SIZE-1 downto 0;
    -- TTMR from PC
    fault_PC                   : in  std_logic;
    fault_PC_lat               : in  std_logic;
    harc_fault_wire            : in  std_logic_vector (2 downto 0);
    harc_fault_wire_lat        : in  std_logic_vector (1 downto 0);
    harc_loss                  : in  integer range THREAD_POOL_SIZE-1 downto 0;
    fault_harc                 : in  std_logic;


    -- Processing Pipeline Signals
    rs1_to_sc                  : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
    rs2_to_sc                  : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
    rd_to_sc                   : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
    halt_LSU                   : in  std_logic;
    data_addr_internal         : out std_logic_vector(31 downto 0);
    ls_except_data             : out std_logic_vector(31 downto 0);
    ls_except_condition        : out std_logic;
    ls_taken_branch            : out std_logic;
    amo_load                   : in  std_logic;
    amo_load_skip              : in  std_logic;
    amo_store                  : out std_logic;
    -- CSR Signals
    misaligned_err             : out std_logic;
    -- Scratchpad Interface Signals
    ls_data_gnt_i              : in  std_logic_vector(SPM_NUM-1 downto 0);
    ls_sci_wr_gnt              : in  std_logic;
    ls_sc_data_read_wire       : in  std_logic_vector(Data_Width-1 downto 0);
    state_LS                   : out fsm_LS_states;
    harc_LS_wire               : out integer range ACCL_NUM-1 downto 0;
    sc_word_count_wire         : out integer;
    spm_bcast                  : out std_logic;
    kmemld_inflight            : out std_logic_vector(SPM_NUM-1 downto 0);
    kmemstr_inflight           : out std_logic_vector(SPM_NUM-1 downto 0);
    ls_sci_req                 : out std_logic_vector(SPM_NUM-1 downto 0);
    ls_sci_we                  : out std_logic_vector(SPM_NUM-1 downto 0);
    ls_sc_read_addr            : out std_logic_vector(Addr_Width-(SIMD_BITS+3) downto 0);
    ls_sc_write_addr           : out std_logic_vector(Addr_Width-(SIMD_BITS+3)downto 0);
    ls_sc_data_write_wire      : out std_logic_vector(Data_Width-1 downto 0);
    -- WB_Stage Signals
    LS_WB_EN                   : out std_logic;
    LS_PE_WB_EN                : out std_logic;
    harc_LS_WB                 : out integer range THREAD_POOL_SIZE-1 downto 0;
    harc_WB                 : in integer range THREAD_POOL_SIZE-1 downto 0;

    instr_word_LS_WB           : out std_logic_vector(31 downto 0);
    LS_WB                      : out std_logic_vector(31 downto 0);
    -- Data memory interface
    data_req_o                 : out std_logic;
    data_gnt_i                 : in  std_logic;
    data_rvalid_i              : in  std_logic;
    data_we_o                  : out std_logic;
    data_be_o                  : out std_logic_vector(3 downto 0);
    data_addr_o                : out std_logic_vector(31 downto 0);
    data_wdata_o               : out std_logic_vector(31 downto 0);
    data_rdata_i               : in  std_logic_vector(31 downto 0);
    data_err_i                 : in  std_logic
	);
end entity;  ------------------------------------------

architecture LSU of Load_Store_Unit is

  subtype harc_range is integer range THREAD_POOL_SIZE - 1 downto 0;
  subtype accl_range is integer range ACCL_NUM - 1 downto 0; 

  --TTMR signals
--  alias fault_PC_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.fault_PC : std_logic >>;
--  alias fault_PC_lat_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.fault_PC_lat : std_logic >>;
--  alias harc_fault_wire_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.harc_fault_wire  : std_logic_vector (2 downto 0) >>;
--  alias harc_fault_wire_lat_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.harc_fault_wire_lat  : std_logic_vector (1 downto 0) >>;
--  alias harc_loss_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.harc_loss  : integer >>;
--  alias fault_harc_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.fault_harc  : std_logic >>;      

--  alias harc_ID_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.harc_ID : harc_range >>;

  signal data_addr_internal_buf_wire       : array_2D(harc_range)(31 downto 0);
  signal data_addr_internal_buf            : array_2D(harc_range)(31 downto 0);
  signal data_wdata_o_buf_wire             : array_2D(harc_range)(31 downto 0);
  signal data_wdata_o_buf                  : array_2D(harc_range)(31 downto 0);
  signal data_we_o_buf_wire                : std_logic_vector (2 downto 0);
  signal data_we_o_buf                     : std_logic_vector (2 downto 0);
  signal data_req_o_buf_wire               : std_logic_vector (2 downto 0);
  signal data_req_o_buf                    : std_logic_vector (2 downto 0);
  signal data_be_internal_buf_wire         : array_2D(harc_range)(3 downto 0);
  signal data_be_internal_buf              : array_2D(harc_range)(3 downto 0);
  signal store_op_buf_wire                 : std_logic_vector(2 downto 0);   --serve per evitare che all'ultimo non venga effettuata la singola store
  signal store_op_buf                      : std_logic_vector(2 downto 0);   --serve per evitare che all'ultimo non venga effettuata la singola store
  signal store_valid                       : std_logic;
  signal load_valid                        : std_logic;
  signal load_op_buf                       : std_logic_vector(2 downto 0);   --serve per evitare che all'ultimo non venga effettuata la singola load
--  signal load_op_buf_lat              : std_logic_vector(2 downto 0);   --serve per evitare che all'ultimo non venga effettuata la singola load
--  signal ls_instr_req_buf             : std_logic_vector(2 downto 0);   --serve per evitare che all'ultimo non venga effettuata la singola LS
--  signal LS_peripheral                : std_logic; 
--  signal LS_peripheral_counter        : integer;

  signal data_width_ID_buf_wire            : array_2D(harc_range)(1 downto 0);
  signal data_width_ID_buf                 : array_2D(harc_range)(1 downto 0);
  signal data_width_ID_voted_wire          : std_logic_vector(1 downto 0);
  signal data_width_ID_voted               : std_logic_vector(1 downto 0);
  signal decoded_instruction_LS_buf_wire   : array_2D(harc_range)(LS_UNIT_INSTR_SET_SIZE-1 downto 0);
  signal decoded_instruction_LS_buf        : array_2D(harc_range)(LS_UNIT_INSTR_SET_SIZE-1 downto 0);
  signal decoded_instruction_LS_voted_wire : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0); -- wire per mantenere il risultato del voting
  signal decoded_instruction_LS_voted      : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0); -- wire per mantenere il risultato del voting
  signal data_addr_internal_voted          : std_logic_vector (31 downto 0);

  -- signals for LSU Comb
  signal data_addr_internal_wires         : std_logic_vector (31 downto 0);
  signal data_wdata_o_wires               : std_logic_vector (31 downto 0);
  signal data_be_internal_wires           : std_logic_vector (3 downto 0);
  signal data_we_o_wires                  : std_logic;
  signal data_req_o_wires                 : std_logic;
  signal ls_except_condition_wires        : std_logic;
  signal ls_taken_branch_wires            : std_logic;
  signal core_busy_LS_wires               : std_logic;
  signal busy_LS_wires                    : std_logic; 


  signal nextstate_LS : fsm_LS_states;
  -- Memory fault signals
  signal data_addr_internal_lat     : std_logic_vector(31 downto 0);
  signal load_err                   : std_logic;
  signal store_err                  : std_logic;
  signal amo_store_lat              : std_logic;
  signal overflow_rs1_sc            : std_logic_vector(Addr_Width downto 0);
  signal overflow_rd_sc             : std_logic_vector(Addr_Width downto 0);
  signal busy_LS_lat                : std_logic;
  signal sc_word_count              : integer;
  signal harc_LS                    : accl_range;
  signal harc_LOAD                  : harc_range;
  signal ls_rs1_to_sc               : std_logic_vector(SPM_ADDR_WID-1 downto 0);
  signal ls_rd_to_sc                : std_logic_vector(SPM_ADDR_WID-1 downto 0);
  signal ls_sc_data_write           : std_logic_vector(Data_Width-1 downto 0);
  signal data_be_internal           : std_logic_vector(3 downto 0);
  signal RS1_Data_IE_wire_lat       : std_logic_vector(31 downto 0);  -- Wire to used to directly send the result of the address increment
  signal RS2_Data_IE_wire_lat       : std_logic_vector(31 downto 0);  -- Wire to used to directly send the result of the address increment
  signal RD_Data_IE_wire_lat        : std_logic_vector(31 downto 0);  -- Wire to used to directly send the result of the address increment
  signal RS1_Data_IE_lat            : std_logic_vector(31 downto 0);  -- Used to preserve the old data in case we start executing in parallel
  signal RS2_Data_IE_lat            : std_logic_vector(31 downto 0);  -- Used to preserve the old data in case we start executing in parallel
  signal RD_Data_IE_lat             : std_logic_vector(31 downto 0);  -- Used to preserve the old data in case we start executing in parallel

  signal add_op_A                   : std_logic_vector(31 downto 0);
  signal add_op_B                   : std_logic_vector(31 downto 0);
  signal add_out                    : std_logic_vector(31 downto 0);


  -- This function increments all the bits in a std_logic_vector
  function add_vect_bits(v: std_logic_vector) return natural is
    variable h: natural;
  begin
    h := 0;
    for i in v'range loop
      if v(i) = '1' then
        h := h + 1;
      end if;
    end loop;
    return h;
  end function add_vect_bits;


begin

  -- Memory fault signals
  load_err  <= data_gnt_i and data_err_i and not(data_we_o);
  store_err <= data_gnt_i and data_err_i and data_we_o;

  -- Memory address signal
  data_addr_o <= ( data_addr_internal_lat(31 downto 2) & "00" ) when fault_PC = '1' else ( data_addr_internal(31 downto 2 ) & "00" ); --se ho fault mantengo il precedente indirizzo cosi da evitare che l'ultima operazione di LSU venga interrotta
  data_be_o <= to_stdlogicvector(to_bitvector(data_be_internal) sll
                                 to_integer(unsigned(data_addr_internal(1 downto 0))));


------------------------------------------------------------------------
--  ██╗     ███████╗██╗   ██╗    ███████╗██╗   ██╗███╗   ██╗ ██████╗  --
--  ██║     ██╔════╝██║   ██║    ██╔════╝╚██╗ ██╔╝████╗  ██║██╔════╝  --
--  ██║     ███████╗██║   ██║    ███████╗ ╚████╔╝ ██╔██╗ ██║██║       --
--  ██║     ╚════██║██║   ██║    ╚════██║  ╚██╔╝  ██║╚██╗██║██║       --
--  ███████╗███████║╚██████╔╝    ███████║   ██║   ██║ ╚████║╚██████╗  --
--  ╚══════╝╚══════╝ ╚═════╝     ╚══════╝   ╚═╝   ╚═╝  ╚═══╝ ╚═════╝  --
------------------------------------------------------------------------

  LSU_sync : process(clk_i, rst_ni)
  begin
	  
    if rst_ni = '0' then
	  amo_store  <= '0';
	  amo_store_lat  <= '0';
	  LS_WB_EN <= '0';
    LS_PE_WB_EN <= '0';     
	  busy_LS_lat <= '0';
	  LS_WB<= (others => '0');
      harc_LS_WB <= THREAD_POOL_SIZE-1;
      misaligned_err <= '0';
      instr_word_LS_WB <= (others => '0');
      if accl_en = 1 then
        ls_rs1_to_sc             <= (others => '0');
        ls_rd_to_sc              <= (others => '0');
      end if;
	elsif rising_edge(clk_i) then
      amo_store  <= '0';
      misaligned_err <= '0';
      LS_WB      <= (others => '0');
      busy_LS_lat <= busy_LS or core_busy_LS;
      if accl_en = 1 then
        RS1_Data_IE_lat <= RS1_Data_IE_wire_lat;
        RS2_Data_IE_lat <= RS2_Data_IE_wire_lat;
        RD_Data_IE_lat  <= RD_Data_IE_wire_lat;
      end if;
      if ( add_vect_bits(ls_instr_req_buf_wire) = 0 and busy_LS_lat = '0' ) or ( harc_fault_wire(2) = '1' or ( harc_fault_wire(1) = '1' and harc_fault_wire_lat = "10" ) or harc_fault_wire_lat = "01" ) or LS_WB_wrong_EXEC = '1' then
        LS_WB_EN <= '0';
        LS_PE_WB_EN <= '0';     
      elsif ( add_vect_bits(ls_instr_req_buf_wire) > 0 or busy_LS_lat = '1' ) then
        if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
          ls_sc_data_write <= data_rdata_i;
        end if;
        LS_WB_EN <= '0';
        LS_PE_WB_EN <= '0';     
        case state_LS is	
          when normal =>

          --------------------------------------------------------------
          --  ██╗      ██████╗  █████╗ ██████╗      ██████╗ ██████╗   --
          --  ██║     ██╔═══██╗██╔══██╗██╔══██╗    ██╔═══██╗██╔══██╗  --
          --  ██║     ██║   ██║███████║██║  ██║    ██║   ██║██████╔╝  --
          --  ██║     ██║   ██║██╔══██║██║  ██║    ██║   ██║██╔═══╝   --
          --  ███████╗╚██████╔╝██║  ██║██████╔╝    ╚██████╔╝██║       --
          --  ╚══════╝ ╚═════╝ ╚═╝  ╚═╝╚═════╝      ╚═════╝ ╚═╝       --
          --------------------------------------------------------------
          -- devo efffettuare una singola voted load quindi quando harc_Exec è zero non posso usare wires ma i risultati del voting sui buffer.
          if ( harc_EXEC = 0 or harc_loss = 0 ) then  
            if load_op = '1' or add_vect_bits(load_op_buf_wire) > 1  then  -- Load Instructions
              if ((data_addr_internal(1 downto 0) = "00" and data_width_ID_voted_wire = "10") or 
                  (data_addr_internal(0)          = '0'  and data_width_ID_voted_wire = "01") or
                                                             data_width_ID_voted_wire = "00") then
		            instr_word_LS_WB <= instr_word_IE;
              else
                ls_except_data <= LOAD_MISALIGNED_EXCEPT_CODE;
                misaligned_err <= '1';
              end if;
            end if;
          else
            if load_op = '1' or add_vect_bits(load_op_buf_wire) > 1 then  -- Load Instructions
              if ((data_addr_internal(1 downto 0) = "00" and data_width_ID = "10") or 
                  (data_addr_internal(0)          = '0'  and data_width_ID = "01") or
                                                             data_width_ID = "00") then
                instr_word_LS_WB <= instr_word_IE;
              else
                ls_except_data <= LOAD_MISALIGNED_EXCEPT_CODE;
                misaligned_err <= '1';
              end if;
            end if;
          end if;
  
          -----------------------------------------------------------------------
          --  ███████╗████████╗ ██████╗ ██████╗ ███████╗     ██████╗ ██████╗   --
          --  ██╔════╝╚══██╔══╝██╔═══██╗██╔══██╗██╔════╝    ██╔═══██╗██╔══██╗  --
          --  ███████╗   ██║   ██║   ██║██████╔╝█████╗      ██║   ██║██████╔╝  --
          --  ╚════██║   ██║   ██║   ██║██╔══██╗██╔══╝      ██║   ██║██╔═══╝   --
          --  ███████║   ██║   ╚██████╔╝██║  ██║███████╗    ╚██████╔╝██║       --
          --  ╚══════╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚══════╝     ╚═════╝ ╚═╝       --
          -----------------------------------------------------------------------
          -- devo efffettuare una singola voted store quindi quando harc_Exec è zero non posso usare wires ma i risultati del voting sui buffer.
          if ( harc_EXEC = 0 or harc_loss = 0 ) then  
            if store_op = '1' or add_vect_bits(store_op_buf_wire) > 1 then	
              if ((data_addr_internal(1 downto 0) = "00" and data_width_ID_voted_wire = "10") or 
                  (data_addr_internal(0)          = '0'  and data_width_ID_voted_wire = "01") or
                                                             data_width_ID_voted_wire = "00") then
				        RS2_Data_IE_lat <= RS2_Data_IE;	
              else
                ls_except_data <= STORE_MISALIGNED_EXCEPT_CODE;
                misaligned_err <= '1';
              end if;
            end if;

            if amo_store = '1' or amo_load_skip = '1' then
              amo_store_lat <= amo_store;
			        amo_store <= '0';
            end if;
          else
            if store_op = '1' or add_vect_bits(store_op_buf_wire) > 1 then  
              if ((data_addr_internal(1 downto 0) = "00" and data_width_ID = "10") or 
                  (data_addr_internal(0)          = '0'  and data_width_ID = "01") or
                                                             data_width_ID = "00") then
                RS2_Data_IE_lat <= RS2_Data_IE; 
              else
                ls_except_data <= STORE_MISALIGNED_EXCEPT_CODE;
                misaligned_err <= '1';
              end if;
            end if;

            if amo_store = '1' or amo_load_skip = '1' then
              amo_store_lat <= amo_store;
              amo_store <= '0';
            end if;          
          end if;      
          ------------------------------------------------------------------------------------------------------
          --  ██████╗ ██╗   ██╗██████╗ ███████╗████████╗    ██╗     ██████╗     ██╗███████╗████████╗██████╗   --
          --  ██╔══██╗██║   ██║██╔══██╗██╔════╝╚══██╔══╝    ██║     ██╔══██╗   ██╔╝██╔════╝╚══██╔══╝██╔══██╗  --
          --  ██████╔╝██║   ██║██████╔╝███████╗   ██║       ██║     ██║  ██║  ██╔╝ ███████╗   ██║   ██████╔╝  --
          --  ██╔══██╗██║   ██║██╔══██╗╚════██║   ██║       ██║     ██║  ██║ ██╔╝  ╚════██║   ██║   ██╔══██╗  --
          --  ██████╔╝╚██████╔╝██║  ██║███████║   ██║       ███████╗██████╔╝██╔╝   ███████║   ██║   ██║  ██║  --
          --  ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚══════╝   ╚═╝       ╚══════╝╚═════╝ ╚═╝    ╚══════╝   ╚═╝   ╚═╝  ╚═╝  --
          ------------------------------------------------------------------------------------------------------

            if accl_en = 1 then
              if decoded_instruction_LS(KMEMLD_bit_position)   = '1' or
                 decoded_instruction_LS(KBCASTLD_bit_position) = '1' then
                -- Illegal byte transfer handler, and illegal writeback address handler
                if RS2_Data_IE(Addr_Width downto 0) = (0 to Addr_Width => '0') then
                  null;
                elsif rd_to_sc = "100" then --  AAA change "100" to make it parametrizable -- Not a scratchpad destination address
                  ls_except_data              <= ILLEGAL_ADDRESS_EXCEPT_CODE;
                elsif RS1_Data_IE(1 downto 0) /= "00" then
                  ls_except_data              <= LOAD_MISALIGNED_EXCEPT_CODE;
                  misaligned_err              <= '1';
                elsif load_err = '1' then  -- AAA move to data_valid_waiting stage
                  ls_except_data                <= LOAD_ERROR_EXCEPT_CODE;
                elsif overflow_rd_sc(Addr_Width) = '1' then
                  ls_except_data                <= SCRATCHPAD_OVERFLOW_EXCEPT_CODE;
                else
                  RS1_Data_IE_lat <= RS1_Data_IE;
                  RS2_Data_IE_lat <= RS2_Data_IE;
                  RD_Data_IE_lat  <= RD_Data_IE;
                  ls_rd_to_sc <= rd_to_sc;
                end if;
              end if;

              if decoded_instruction_LS(KMEMSTR_bit_position) = '1' then
                -- Illegal byte transfer handler, and illegal writeback address handler
                if RS2_Data_IE(Addr_Width downto 0) = (0 to Addr_Width => '0') then
                  null;
		            elsif rs1_to_sc = "100" then                              --  Not a scratchpad source address
                  ls_except_data              <= ILLEGAL_ADDRESS_EXCEPT_CODE;
		            elsif RD_Data_IE(1 downto 0) /= "00" then
                  ls_except_data              <= STORE_MISALIGNED_EXCEPT_CODE;
                  misaligned_err              <= '1';  
                elsif store_err = '1' then
                  ls_except_data              <= STORE_ERROR_EXCEPT_CODE;
                elsif overflow_rs1_sc(Addr_Width) = '1' then
                  ls_except_data              <= SCRATCHPAD_OVERFLOW_EXCEPT_CODE;				
                else
                  RS1_Data_IE_lat <= RS1_Data_IE;
                  RS2_Data_IE_lat <= RS2_Data_IE;
                  RD_Data_IE_lat <= RD_Data_IE;
                  ls_rs1_to_sc <= rs1_to_sc;
                end if;
              end if;
            end if;
			  
          when data_valid_waiting =>
            harc_LS_WB <= harc_loss when fault_harc = '1' else harc_EXEC;

          ----------------------------------------------------------------------------------------------------
          --  ██╗   ██╗ █████╗ ██╗     ██╗██████╗     ██╗    ██╗ █████╗ ██╗████████╗██╗███╗   ██╗ ██████╗   --
          --  ██║   ██║██╔══██╗██║     ██║██╔══██╗    ██║    ██║██╔══██╗██║╚══██╔══╝██║████╗  ██║██╔════╝   --
          --  ██║   ██║███████║██║     ██║██║  ██║    ██║ █╗ ██║███████║██║   ██║   ██║██╔██╗ ██║██║  ███╗  --
          --  ╚██╗ ██╔╝██╔══██║██║     ██║██║  ██║    ██║███╗██║██╔══██║██║   ██║   ██║██║╚██╗██║██║   ██║  --
          --   ╚████╔╝ ██║  ██║███████╗██║██████╔╝    ╚███╔███╔╝██║  ██║██║   ██║   ██║██║ ╚████║╚██████╔╝  --
          --    ╚═══╝  ╚═╝  ╚═╝╚══════╝╚═╝╚═════╝      ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝   ╚═╝   ╚═╝╚═╝  ╚═══╝ ╚═════╝   --
          ----------------------------------------------------------------------------------------------------
          -- devo efffettuare una singola voted LS quindi quando harc_Exec è zero non posso usare wires ma i risultati del voting sui buffer 8 decoded_istr_LS_voted).
          if ( harc_EXEC = 0 or harc_loss = 0 ) then  
            if amo_store_lat = '1' or amo_load_skip = '1' then
              if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1'  then
                amo_store_lat <= '0';
              end if;
            end if; 
    
            if decoded_instruction_LS_voted_wire(LW_bit_position) = '1' then
              if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
                LS_WB<= data_rdata_i;
                LS_WB_EN <= '1';
                LS_PE_WB_EN <= '0';     
                if decoded_instruction_LS_voted_wire(AMOSWAP_bit_position) = '1' then
                 amo_store <= '1';
                end if;
              end if;
            end if;
    
            if decoded_instruction_LS_voted_wire(LH_bit_position) = '1' or decoded_instruction_LS_voted_wire(LHU_bit_position) = '1' then 
              if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
                case data_addr_internal(1) is
                when '0' =>
                          LS_WB_EN <= '1';
                if decoded_instruction_LS_voted_wire(LH_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(signed(data_rdata_i(15 downto 0)), 32));
                elsif decoded_instruction_LS_voted_wire(LHU_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(unsigned(data_rdata_i(15 downto 0)), 32));
                end if;
                when '1' =>
                          LS_WB_EN <= '1';
                if decoded_instruction_LS_voted_wire(LH_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(signed(data_rdata_i(31 downto 16)), 32));
                elsif decoded_instruction_LS_voted_wire(LHU_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(unsigned(data_rdata_i(31 downto 16)), 32));
                end if;
                when others =>
                        null;
                end case;
              end if;
            end if;
    
            if decoded_instruction_LS_voted_wire(LB_bit_position) = '1' or decoded_instruction_LS_voted_wire(LBU_bit_position) = '1' then 
              if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then   
                LS_WB_EN <= '1';
                LS_PE_WB_EN <= '0';     
                case data_addr_internal(1 downto 0) is
                when "00" =>
                  if decoded_instruction_LS_voted_wire(LB_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(signed(data_rdata_i(7 downto 0)), 32));
                  elsif decoded_instruction_LS_voted_wire(LBU_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(unsigned(data_rdata_i(7 downto 0)), 32));
                  end if;
                when "01" =>
                  if decoded_instruction_LS_voted_wire(LB_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(signed(data_rdata_i(15 downto 8)), 32));
                  elsif decoded_instruction_LS_voted_wire(LBU_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(unsigned(data_rdata_i(15 downto 8)), 32));            
                  end if;
                when "10" =>
                  if decoded_instruction_LS_voted_wire(LB_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(signed(data_rdata_i(23 downto 16)), 32));
                  elsif decoded_instruction_LS_voted_wire(LBU_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(unsigned(data_rdata_i(23 downto 16)), 32));
                  end if;
                when "11" =>
                  if decoded_instruction_LS_voted_wire(LB_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(signed(data_rdata_i(31 downto 24)), 32));
                  elsif decoded_instruction_LS_voted_wire(LBU_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(unsigned(data_rdata_i(31 downto 24)), 32));
                  end if;
                when others =>
                  null;               
                end case;
              end if;
            end if;
          else
            if amo_store_lat = '1' or amo_load_skip = '1' then
              if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1'  then
                amo_store_lat <= '0';
              end if;
            end if; 
    
            if decoded_instruction_LS(LW_bit_position) = '1'  then
              if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
                LS_WB<= data_rdata_i;
                LS_WB_EN <= '1';
                LS_PE_WB_EN <= '0';     
                if decoded_instruction_LS(AMOSWAP_bit_position) = '1' then
                 amo_store <= '1';
                end if;
              end if;
            end if;
    
            if decoded_instruction_LS(LH_bit_position) = '1' or decoded_instruction_LS(LHU_bit_position) = '1' then 
              if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
                case data_addr_internal(1) is
                when '0' =>
                          LS_WB_EN <= '1';
                if decoded_instruction_LS(LH_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(signed(data_rdata_i(15 downto 0)), 32));
                elsif decoded_instruction_LS(LHU_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(unsigned(data_rdata_i(15 downto 0)), 32));
                end if;
                when '1' =>
                          LS_WB_EN <= '1';
                if decoded_instruction_LS(LH_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(signed(data_rdata_i(31 downto 16)), 32));
                elsif decoded_instruction_LS(LHU_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(unsigned(data_rdata_i(31 downto 16)), 32));
                end if;
                when others =>
                        null;
                end case;
              end if;
            end if;
    
            if decoded_instruction_LS(LB_bit_position) = '1' or decoded_instruction_LS(LBU_bit_position) = '1' then 
              if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then   
                LS_WB_EN <= '1';
                LS_PE_WB_EN <= '0';     
                case data_addr_internal(1 downto 0) is
                when "00" =>
                  if decoded_instruction_LS(LB_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(signed(data_rdata_i(7 downto 0)), 32));
                  elsif decoded_instruction_LS(LBU_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(unsigned(data_rdata_i(7 downto 0)), 32));
                  end if;
                when "01" =>
                  if decoded_instruction_LS(LB_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(signed(data_rdata_i(15 downto 8)), 32));
                  elsif decoded_instruction_LS(LBU_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(unsigned(data_rdata_i(15 downto 8)), 32));            
                  end if;
                when "10" =>
                  if decoded_instruction_LS(LB_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(signed(data_rdata_i(23 downto 16)), 32));
                  elsif decoded_instruction_LS(LBU_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(unsigned(data_rdata_i(23 downto 16)), 32));
                  end if;
                when "11" =>
                  if decoded_instruction_LS(LB_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(signed(data_rdata_i(31 downto 24)), 32));
                  elsif decoded_instruction_LS(LBU_bit_position) = '1' then
                          LS_WB<= std_logic_vector(resize(unsigned(data_rdata_i(31 downto 24)), 32));
                  end if;
                when others =>
                  null;               
                end case;
              end if;
            end if;     
          end if;
 	      end case;
      end if;
    end if;
  end process;

  spm_bcast <= '1' when  decoded_instruction_LS(KBCASTLD_bit_position) = '1' else '0';


-------------------------------------------------------------------------
--  ██╗     ███████╗██╗   ██╗     ██████╗ ██████╗ ███╗   ███╗██████╗   --
--  ██║     ██╔════╝██║   ██║    ██╔════╝██╔═══██╗████╗ ████║██╔══██╗  --
--  ██║     ███████╗██║   ██║    ██║     ██║   ██║██╔████╔██║██████╔╝  --
--  ██║     ╚════██║██║   ██║    ██║     ██║   ██║██║╚██╔╝██║██╔══██╗  --
--  ███████╗███████║╚██████╔╝    ╚██████╗╚██████╔╝██║ ╚═╝ ██║██████╔╝  --
--  ╚══════╝╚══════╝ ╚═════╝      ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═════╝   --
-------------------------------------------------------------------------

  LSU_comb : process(all)

  begin
    data_addr_internal_wires         <= std_logic_vector(signed(RS1_Data_IE));  -- The reset value was non-zero in order to keep the switching activity minimal
    nextstate_LS                     <= normal;
    data_be_internal_wires           <=(others => '0');
    data_wdata_o_wires               <= (others => '0');
    data_we_o_wires                  <= '0';
    data_req_o_wires                 <= '0';
  	ls_except_condition_wires        <= '0';
	  ls_taken_branch_wires            <= '0';
    core_busy_LS_wires               <= '0';
    busy_LS_wires                    <= '0';

    decoded_instruction_LS_voted_wire <= decoded_instruction_LS_voted;
    data_width_ID_voted_wire          <= data_width_ID_voted;

    if rst_ni = '0' then
      data_width_ID_voted_wire <= (others => '0');
      decoded_instruction_LS_voted_wire  <= (others => '0');
    end if;
    if accl_en = 1 then
      overflow_rs1_sc                  <= (others => '0');
      overflow_rd_sc                   <= (others => '0');
      ls_sc_data_write_wire            <= ls_sc_data_write;
      sc_word_count_wire               <= sc_word_count;
      kmemld_inflight                  <= (others => '0');
      kmemstr_inflight                 <= (others => '0');
      ls_sc_write_addr                 <= (others => '0');
	    ls_sc_read_addr                  <= (others => '0');
      --halt_lsu                         <= '0';
      RS1_Data_IE_wire_lat             <= RS1_Data_IE_lat;
      RS2_Data_IE_wire_lat             <= RS2_Data_IE_lat;
      RD_Data_IE_wire_lat              <= RD_Data_IE_lat;
      harc_LS_wire                     <= harc_LS;
      ls_sci_req <= (others => '0');
      ls_sci_we  <= (others => '0');
    end if;

if ( add_vect_bits(ls_instr_req_buf_wire) > 0 or busy_LS_lat = '1' ) and LS_WB_wrong_EXEC = '0'  then 

      if core_busy_WB = '1' then
            core_busy_LS_wires <= '1';
      end if;   

      case state_LS is
        when normal =>

          --------------------------------------------------------------
          --  ██╗      ██████╗  █████╗ ██████╗      ██████╗ ██████╗   --
          --  ██║     ██╔═══██╗██╔══██╗██╔══██╗    ██╔═══██╗██╔══██╗  --
          --  ██║     ██║   ██║███████║██║  ██║    ██║   ██║██████╔╝  --
          --  ██║     ██║   ██║██╔══██║██║  ██║    ██║   ██║██╔═══╝   --
          --  ███████╗╚██████╔╝██║  ██║██████╔╝    ╚██████╔╝██║       --
          --  ╚══════╝ ╚═════╝ ╚═╝  ╚═╝╚═════╝      ╚═════╝ ╚═╝       --
          --------------------------------------------------------------
          -- devo efffettuare una singola voted load quindi quando harc_Exec è zero non posso usare wires ma i risultati del voting sui buffer.
          if ( harc_EXEC = 0 or harc_loss = 0 ) then  

            if load_op = '1' or add_vect_bits(load_op_buf_wire) > 1 then

                data_addr_internal_wires <= add_out;
  
              -- se ho un indirizzo fuori range accettato, io non voglio eseguire la load, perciò genero exception senza mandare data_req_o
              -- DOVREI METTERE QUESTO CONTROLLO FUORI DA QUESTA CONDIZIONE DI LOAD_OP E RENDERLO GENERALE MA FORSE FACENDO UNA SOLA LOAD NON SERVE PIU
  --            if ((data_addr_internal > ILLEGAL_MEM_REGION_START) and (data_addr_internal < ILLEGAL_MEM_REGION_STOP)) then
  --              ls_except_condition_wires <= '1';
  --              ls_taken_branch_wires     <= '1';
  --              data_req_o_wires       <= '0';
  --              data_be_internal_wires <= data_be_ID;
  --            else      
  
              if ((data_addr_internal_voted(1 downto 0) = "00" and data_width_ID_voted_wire = "10") or 
                  (data_addr_internal_voted(0)          = '0'  and data_width_ID_voted_wire = "01") or
                                                                   data_width_ID_voted_wire = "00") then
                data_be_internal_wires <= data_be_ID;
                data_req_o_wires       <= '1';
                if load_err = '1' then
                  ls_except_condition_wires <= '1';
                  ls_taken_branch_wires     <= '1';
                else  --not a peripheral
                  core_busy_LS_wires <= '1';
                  nextstate_LS <= data_valid_waiting;                
                end if;
              else
                ls_except_condition_wires <= '1';
                ls_taken_branch_wires     <= '1';
              end if;
            end if;
          else
          -- qui effettuo le load fittizie   
            if load_op = '1' or add_vect_bits(load_op_buf_wire) > 1 then

                data_addr_internal_wires <= add_out;
  
              if ((data_addr_internal_wires(1 downto 0) = "00" and data_width_ID = "10") or 
                  (data_addr_internal_wires(0)          = '0'  and data_width_ID = "01") or
                                                                   data_width_ID = "00") then
                data_be_internal_wires <= data_be_ID;
                data_req_o_wires       <= '1';
                if load_err = '1' then
                  ls_except_condition_wires <= '1';
                  ls_taken_branch_wires     <= '1';
                else  --not a peripheral
                  core_busy_LS_wires <= '1';
                  nextstate_LS <= data_valid_waiting;                
                end if;
              else
                ls_except_condition_wires <= '1';
                ls_taken_branch_wires     <= '1';
              end if;
            end if;  
--            end if;
          end if;


          -----------------------------------------------------------------------
          --  ███████╗████████╗ ██████╗ ██████╗ ███████╗     ██████╗ ██████╗   --
          --  ██╔════╝╚══██╔══╝██╔═══██╗██╔══██╗██╔════╝    ██╔═══██╗██╔══██╗  --
          --  ███████╗   ██║   ██║   ██║██████╔╝█████╗      ██║   ██║██████╔╝  --
          --  ╚════██║   ██║   ██║   ██║██╔══██╗██╔══╝      ██║   ██║██╔═══╝   --
          --  ███████║   ██║   ╚██████╔╝██║  ██║███████╗    ╚██████╔╝██║       --
          --  ╚══════╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚══════╝     ╚═════╝ ╚═╝       --
          -----------------------------------------------------------------------
        -- devo efffettuare una singola voted store quindi quando harc_Exec è zero non posso usare wires ma i risultati del voting sui buffer.
        if ( harc_EXEC = 0 or harc_loss = 0 ) then  

          if store_op = '1' or add_vect_bits(store_op_buf_wire) > 1 then

            data_addr_internal_wires <= add_out;
            data_we_o_wires <= '1';

            if ((data_addr_internal_voted(1 downto 0) = "00" and data_width_ID_voted_wire = "10") or 
                (data_addr_internal_voted(0)          = '0'  and data_width_ID_voted_wire = "01") or
                                                                 data_width_ID_voted_wire = "00") then
              data_req_o_wires   <= '1';
              data_wdata_o_wires <= RS2_Data_IE;		
              data_be_internal_wires <= data_be_ID;
              if store_err = '1' then
                ls_except_condition_wires  <= '1';
                ls_taken_branch_wires      <= '1';
              else
                nextstate_LS <= data_valid_waiting;
                busy_LS_wires <= '1';
                if amo_store = '1' or amo_load_skip = '1' then
                  core_busy_LS_wires <= '1';
                end if;
              end if;
            else
              ls_except_condition_wires  <= '1';
              ls_taken_branch_wires      <= '1';
            end if;

            if data_width_ID_voted_wire = "01" then  -- store half word
                case data_addr_internal_voted(1) is
                  when '0' =>
                    data_wdata_o_wires <= RS2_Data_IE(31 downto 0);
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when '1' =>
                    data_wdata_o_wires <= RS2_Data_IE(15 downto 0) & std_logic_vector(to_unsigned(0, 16));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when others =>
                    null;
                end case;
            end if;
            if data_width_ID_voted_wire = "00" then  -- store byte
              case data_addr_internal_voted(1 downto 0) is
                when "00" =>
                  data_wdata_o_wires <= RS2_Data_IE(31 downto 0);
                  data_we_o_wires        <= '1';  -- is a writing
                  data_be_internal_wires <= data_be_ID;
                when "01" =>
                  data_wdata_o_wires <= RS2_Data_IE(23 downto 0) & std_logic_vector(to_unsigned(0, 8));
                  data_we_o_wires        <= '1';  -- is a writing
                  data_be_internal_wires <= data_be_ID;
                when "10" =>
                  data_wdata_o_wires <= RS2_Data_IE(15 downto 0) & std_logic_vector(to_unsigned(0, 16));
                  data_we_o_wires        <= '1';  -- is a writing
                  data_be_internal_wires <= data_be_ID;
                when "11" =>
                  data_wdata_o_wires <= RS2_Data_IE(7 downto 0) & std_logic_vector(to_unsigned(0, 24));
                  data_we_o_wires        <= '1';  -- is a writing
                  data_be_internal_wires <= data_be_ID;
                when others =>
                  null;
              end case;
            end if;
          end if;
        else 
          if store_op = '1' or add_vect_bits(store_op_buf_wire) > 1 then

            data_addr_internal_wires <= add_out;
            data_we_o_wires <= '1';

            if ((data_addr_internal_wires(1 downto 0) = "00" and data_width_ID = "10") or 
                (data_addr_internal_wires(0)          = '0'  and data_width_ID = "01") or
                                                                 data_width_ID = "00") then
              data_req_o_wires   <= '1';
              data_wdata_o_wires <= RS2_Data_IE;    
              data_be_internal_wires <= data_be_ID;
              if store_err = '1' then --or ((data_addr_internal_wires > ILLEGAL_MEM_REGION_START) and (data_addr_internal_wires < ILLEGAL_MEM_REGION_STOP)) then
                ls_except_condition_wires  <= '1';
                ls_taken_branch_wires      <= '1';
              else
                nextstate_LS <= data_valid_waiting;
                busy_LS_wires <= '1';
                if amo_store = '1' or amo_load_skip = '1' then
                  core_busy_LS_wires <= '1';
                end if;
              end if;
            else
              ls_except_condition_wires  <= '1';
              ls_taken_branch_wires      <= '1';
            end if;
            if data_width_ID = "01" then  -- store half word
                case data_addr_internal_wires(1) is
                  when '0' =>
                    data_wdata_o_wires <= RS2_Data_IE(31 downto 0);
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when '1' =>
                    data_wdata_o_wires <= RS2_Data_IE(15 downto 0) & std_logic_vector(to_unsigned(0, 16));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when others =>
                    null;
                end case;
            end if;
            if data_width_ID = "00" then  -- store byte
               case data_addr_internal_wires(1 downto 0) is
                 when "00" =>
                    data_wdata_o_wires <= RS2_Data_IE(31 downto 0);
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                 when "01" =>
                    data_wdata_o_wires <= RS2_Data_IE(23 downto 0) & std_logic_vector(to_unsigned(0, 8));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                 when "10" =>
                    data_wdata_o_wires <= RS2_Data_IE(15 downto 0) & std_logic_vector(to_unsigned(0, 16));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                 when "11" =>
                    data_wdata_o_wires <= RS2_Data_IE(7 downto 0) & std_logic_vector(to_unsigned(0, 24));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                 when others =>
                    null;
               end case;
            end if;
          end if;  
        end if;
        

          ------------------------------------------------------------------------------------------------------
          --  ██████╗ ██╗   ██╗██████╗ ███████╗████████╗    ██╗     ██████╗     ██╗███████╗████████╗██████╗   --
          --  ██╔══██╗██║   ██║██╔══██╗██╔════╝╚══██╔══╝    ██║     ██╔══██╗   ██╔╝██╔════╝╚══██╔══╝██╔══██╗  --
          --  ██████╔╝██║   ██║██████╔╝███████╗   ██║       ██║     ██║  ██║  ██╔╝ ███████╗   ██║   ██████╔╝  --
          --  ██╔══██╗██║   ██║██╔══██╗╚════██║   ██║       ██║     ██║  ██║ ██╔╝  ╚════██║   ██║   ██╔══██╗  --
          --  ██████╔╝╚██████╔╝██║  ██║███████║   ██║       ███████╗██████╔╝██╔╝   ███████║   ██║   ██║  ██║  --
          --  ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚══════╝   ╚═╝       ╚══════╝╚═════╝ ╚═╝    ╚══════╝   ╚═╝   ╚═╝  ╚═╝  --
          ------------------------------------------------------------------------------------------------------

            if accl_en = 1 then
              if decoded_instruction_LS(KMEMLD_bit_position)   = '1' or
                 decoded_instruction_LS(KBCASTLD_bit_position) = '1' then
                -- RS2_Data_IE(Addr_Width downto 0) instead of RS2_Data_IE(Addr_Width -1 downto 0) in order to allow reading sizes = MAX_SC_SIZE and not MAX_SC_SIZE - 1 
                overflow_rd_sc <= add_out(Addr_Width downto 0); -- If storing data to SC overflows it's address space
                if RS2_Data_IE(Addr_Width downto 0) = (0 to Addr_Width => '0') then
                  null;
                elsif rd_to_sc = "100" then
                  ls_except_condition_wires  <= '1';
                  ls_taken_branch_wires      <= '1';
                elsif(RS1_Data_IE(1 downto 0) /= "00") then
                  ls_except_condition_wires  <= '1';
                  ls_taken_branch_wires      <= '1';
                elsif load_err = '1' then
                  ls_except_condition_wires  <= '1';
                  ls_taken_branch_wires      <= '1';
                elsif overflow_rd_sc(Addr_Width) = '1' then
                  ls_except_condition_wires  <= '1';
                  ls_taken_branch_wires      <= '1';
                else
                  nextstate_LS    <= data_valid_waiting;
                  busy_LS_wires   <= '1';
                  sc_word_count_wire <= to_integer(unsigned(RD_Data_IE(SIMD_BITS+1 downto 1))/2);
                  kmemld_inflight(to_integer(unsigned(rd_to_sc))) <= '1';
                  if replicate_accl_en=1 then
                    harc_LS_wire <= harc_loss when fault_harc = '1' else harc_EXEC ;
                  elsif replicate_accl_en=0 then
                    harc_LS_wire <= 0;
                  end if;
                end if;
              end if;

            if decoded_instruction_LS(KMEMSTR_bit_position) = '1' then
              -- RS2_Data_IE(Addr_Width downto 0) instead of RS2_Data_IE(Addr_Width -1 downto 0) in order to allow reading sizes = MAX_SC_SIZE and not MAX_SC_SIZE - 1 
              overflow_rs1_sc <= add_out(Addr_Width downto 0); -- If loading data from SC overflows it's address space
              if RS2_Data_IE(Addr_Width downto 0) = (0 to Addr_Width => '0') then
                null;
              elsif rs1_to_sc = "100" then
                ls_except_condition_wires  <= '1';
                ls_taken_branch_wires      <= '1';
              elsif(RD_Data_IE(1 downto 0) /= "00") then
                ls_except_condition_wires  <= '1';
                ls_taken_branch_wires      <= '1';
              elsif store_err = '1' then
                ls_except_condition_wires  <= '1';
                ls_taken_branch_wires      <= '1';
              elsif overflow_rs1_sc(Addr_Width) = '1' then
                ls_except_condition_wires  <= '1';
                ls_taken_branch_wires      <= '1';
              else
                nextstate_LS    <= data_valid_waiting;
                busy_LS_wires   <= '1';
                ls_sci_req(to_integer(unsigned(rs1_to_sc))) <= '1';
                ls_sc_read_addr <= RS1_Data_IE(Addr_Width - 1 downto SIMD_BITS+2);
                sc_word_count_wire <= to_integer(unsigned(RS1_Data_IE(SIMD_BITS+1 downto 1))/2);
                kmemstr_inflight(to_integer(unsigned(rs1_to_sc))) <= '1';
                if replicate_accl_en=1 then
                  harc_LS_wire    <= harc_loss when fault_harc = '1' else harc_EXEC;
                elsif replicate_accl_en=0 then
                  harc_LS_wire <= 0;
                end if;
              end if;
            end if;
          end if;

        when data_valid_waiting =>  


          ----------------------------------------------------------------------------------------------------
          --  ██╗   ██╗ █████╗ ██╗     ██╗██████╗     ██╗    ██╗ █████╗ ██╗████████╗██╗███╗   ██╗ ██████╗   --
          --  ██║   ██║██╔══██╗██║     ██║██╔══██╗    ██║    ██║██╔══██╗██║╚══██╔══╝██║████╗  ██║██╔════╝   --
          --  ██║   ██║███████║██║     ██║██║  ██║    ██║ █╗ ██║███████║██║   ██║   ██║██╔██╗ ██║██║  ███╗  --
          --  ╚██╗ ██╔╝██╔══██║██║     ██║██║  ██║    ██║███╗██║██╔══██║██║   ██║   ██║██║╚██╗██║██║   ██║  --
          --   ╚████╔╝ ██║  ██║███████╗██║██████╔╝    ╚███╔███╔╝██║  ██║██║   ██║   ██║██║ ╚████║╚██████╔╝  --
          --    ╚═══╝  ╚═╝  ╚═╝╚══════╝╚═╝╚═════╝      ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝   ╚═╝   ╚═╝╚═╝  ╚═══╝ ╚═════╝   --
          ----------------------------------------------------------------------------------------------------


          data_addr_internal_wires <= data_addr_internal_lat;

         -- if ls_sci_wr_gnt = '0' and ls_sci_we /=  (0 to SPM_NUM-1 => '0') then
         --   halt_lsu <= '1';
         -- end if;
        -- devo efffettuare una singola voted store quindi quando harc_Exec è zero non posso usare wires ma i risultati del voting sui buffer.
        if ( harc_EXEC = 0 or harc_loss = 0 ) then  

          if decoded_instruction_LS_voted(KMEMLD_bit_position)   = '1' or
             decoded_instruction_LS_voted(KBCASTLD_bit_position) = '1' then
            if accl_en = 1 then
              if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
                 RS1_Data_IE_wire_lat <= std_logic_vector(unsigned(RS1_Data_IE_lat) + "100");
                 RD_Data_IE_wire_lat  <= std_logic_vector(unsigned(RD_Data_IE_lat)  + "100");
                 if unsigned(RS2_Data_IE_lat(Addr_Width downto 0)) >= 4 then
                   RS2_Data_IE_wire_lat <= std_logic_vector(unsigned(RS2_Data_IE_lat) - "100");
                 else
                   RS2_Data_IE_wire_lat <= (others => '0');
                 end if;
              end if;
              if RS2_Data_IE_lat(Addr_Width downto 0) /= (0 to Addr_Width => '0') then
                busy_LS_wires              <= '1';
                data_be_internal_wires     <= "1111";
                data_req_o_wires           <= '1';
                data_addr_internal_wires <= RS1_Data_IE_wire_lat;
                nextstate_LS <= data_valid_waiting;
                ls_sci_we(to_integer(unsigned(ls_rd_to_sc))) <= '1';
                ls_sc_write_addr <= RD_Data_IE_lat(Addr_Width - 1 downto SIMD_BITS+2);
                kmemld_inflight(to_integer(unsigned(ls_rd_to_sc))) <= '1';
                if data_rvalid_i = '1' or load_valid = '1'  or store_valid = '1'  then
                  if unsigned(RS2_Data_IE_lat(Addr_Width downto 0)) >= 4 then
                    ls_sc_data_write_wire <= data_rdata_i;
                  else
                    ls_sc_data_write_wire(8*to_integer(unsigned(RS2_Data_IE_lat(1 downto 0))) - 1 downto 0) <= data_rdata_i(8*to_integer(unsigned(RS2_Data_IE_lat(1 downto 0))) - 1 downto 0);
                  end if;
                end if;
                if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
                  ls_sci_req(to_integer(unsigned(ls_rd_to_sc))) <= '1';
                  if sc_word_count = SIMD-1 then
                    sc_word_count_wire <= 0;
                  else
                    sc_word_count_wire <= sc_word_count + 1;
                  end if;
                end if;
              end if;
            end if;

          elsif decoded_instruction_LS_voted(KMEMSTR_bit_position) = '1' then
            if accl_en = 1 then
              if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
                 RS1_Data_IE_wire_lat <= std_logic_vector(unsigned(RS1_Data_IE_lat) + "100");
                 RD_Data_IE_wire_lat  <= std_logic_vector(unsigned(RD_Data_IE_lat)  + "100");
                 if unsigned(RS2_Data_IE_lat(Addr_Width downto 0)) >= 4 then
                   RS2_Data_IE_wire_lat <= std_logic_vector(unsigned(RS2_Data_IE_lat) - "100");
                 else
                   RS2_Data_IE_wire_lat <= (others => '0');
                 end if;
              end if;
              if RS2_Data_IE_lat(Addr_Width downto 0) /= (0 to Addr_Width => '0') then
                busy_LS_wires      <= '1';
                nextstate_LS <= data_valid_waiting;
                ls_sc_read_addr <= RS1_Data_IE_wire_lat(Addr_Width - 1 downto SIMD_BITS+2);
                kmemstr_inflight(to_integer(unsigned(ls_rs1_to_sc))) <= '1';
                if to_integer(unsigned(ls_data_gnt_i)) /= 0 then
                  data_be_internal_wires     <= "1111";
                  data_req_o_wires           <= '1';
                  data_we_o_wires            <= '1';
                  data_addr_internal_wires <= RD_Data_IE_lat;
                  data_wdata_o_wires <= ls_sc_data_read_wire;
                end if;
                -- Increments the address of the SC memory every four words for KMEMSTR
                if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
                  ls_sci_req(to_integer(unsigned(ls_rs1_to_sc))) <= '1';
                  if sc_word_count = SIMD-1 then
                    sc_word_count_wire <= 0;
                  else
                    sc_word_count_wire <= sc_word_count + 1;
                  end if;
                end if;
              end if;
            end if;
	  
          elsif data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
            if store_op = '1' or add_vect_bits(store_op_buf_wire) > 1 then -- SW or AMOSWAP data writing
              if decoded_instruction_LS_voted(SW_bit_position) = '1' then  -- SW data writing
                data_wdata_o_wires     <= RS2_Data_IE_lat(31 downto 0);
                data_we_o_wires        <= '1';  -- is a writing
                data_be_internal_wires <= data_be_ID;
              end if;
              if decoded_instruction_LS_voted(SH_bit_position) = '1' then  -- SH data writing
                case data_addr_internal_voted(1) is
                  when '0' =>
                    data_wdata_o_wires <= RS2_Data_IE_lat(31 downto 0);
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when '1' =>
                    data_wdata_o_wires <= RS2_Data_IE_lat(15 downto 0) & std_logic_vector(to_unsigned(0, 16));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when others =>
                    null;
                end case;
              end if;
              if decoded_instruction_LS_voted(SB_bit_position) = '1' then  -- SB data writng
                case data_addr_internal_voted(1 downto 0) is
                  when "00" =>
                    data_wdata_o_wires <= RS2_Data_IE_lat(31 downto 0);
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when "01" =>
                    data_wdata_o_wires <= RS2_Data_IE_lat(23 downto 0) & std_logic_vector(to_unsigned(0, 8));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when "10" =>
                    data_wdata_o_wires <= RS2_Data_IE_lat(15 downto 0) & std_logic_vector(to_unsigned(0, 16));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when "11" =>
                    data_wdata_o_wires <= RS2_Data_IE_lat(7 downto 0) & std_logic_vector(to_unsigned(0, 24));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when others =>
                    null;
                end case;
              end if;
            end if;
		
            if load_op = '1'  or add_vect_bits(load_op_buf_wire) > 1  then
              data_be_internal_wires <= data_be_ID;
              if decoded_instruction_LS_voted(AMOSWAP_bit_position) = '1' then
                core_busy_LS_wires <= '1';				  
		          end if;
	          end if;
          else
            nextstate_LS <= data_valid_waiting;
            busy_LS_wires <= '1';
            -- do not change to "if store_op = '0'" since that will disable store superscalar execution, because store_op resets to 0 on the next cycle
            if decoded_instruction_LS_voted(SW_bit_position) = '0' and  decoded_instruction_LS_voted(SH_bit_position) = '0' and decoded_instruction_LS_voted(SB_bit_position)  = '0' then
              core_busy_LS_wires <= '1';
            end if;
		      end if;
      else

          if decoded_instruction_LS(KMEMLD_bit_position)   = '1' or
             decoded_instruction_LS(KBCASTLD_bit_position) = '1' then
            if accl_en = 1 then
              if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
                 RS1_Data_IE_wire_lat <= std_logic_vector(unsigned(RS1_Data_IE_lat) + "100");
                 RD_Data_IE_wire_lat  <= std_logic_vector(unsigned(RD_Data_IE_lat)  + "100");
                 if unsigned(RS2_Data_IE_lat(Addr_Width downto 0)) >= 4 then
                   RS2_Data_IE_wire_lat <= std_logic_vector(unsigned(RS2_Data_IE_lat) - "100");
                 else
                   RS2_Data_IE_wire_lat <= (others => '0');
                 end if;
              end if;
              if RS2_Data_IE_lat(Addr_Width downto 0) /= (0 to Addr_Width => '0') then
                busy_LS_wires              <= '1';
                data_be_internal_wires     <= "1111";
                data_req_o_wires           <= '1';
                data_addr_internal_wires <= RS1_Data_IE_wire_lat;
                nextstate_LS <= data_valid_waiting;
                ls_sci_we(to_integer(unsigned(ls_rd_to_sc))) <= '1';
                ls_sc_write_addr <= RD_Data_IE_lat(Addr_Width - 1 downto SIMD_BITS+2);
                kmemld_inflight(to_integer(unsigned(ls_rd_to_sc))) <= '1';
                if data_rvalid_i = '1' or load_valid = '1'  or store_valid = '1'  then
                  if unsigned(RS2_Data_IE_lat(Addr_Width downto 0)) >= 4 then
                    ls_sc_data_write_wire <= data_rdata_i;
                  else
                    ls_sc_data_write_wire(8*to_integer(unsigned(RS2_Data_IE_lat(1 downto 0))) - 1 downto 0) <= data_rdata_i(8*to_integer(unsigned(RS2_Data_IE_lat(1 downto 0))) - 1 downto 0);
                  end if;
                end if;
                if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
                  ls_sci_req(to_integer(unsigned(ls_rd_to_sc))) <= '1';
                  if sc_word_count = SIMD-1 then
                    sc_word_count_wire <= 0;
                  else
                    sc_word_count_wire <= sc_word_count + 1;
                  end if;
                end if;
              end if;
            end if;

          elsif decoded_instruction_LS(KMEMSTR_bit_position) = '1' then
            if accl_en = 1 then
              if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
                 RS1_Data_IE_wire_lat <= std_logic_vector(unsigned(RS1_Data_IE_lat) + "100");
                 RD_Data_IE_wire_lat  <= std_logic_vector(unsigned(RD_Data_IE_lat)  + "100");
                 if unsigned(RS2_Data_IE_lat(Addr_Width downto 0)) >= 4 then
                   RS2_Data_IE_wire_lat <= std_logic_vector(unsigned(RS2_Data_IE_lat) - "100");
                 else
                   RS2_Data_IE_wire_lat <= (others => '0');
                 end if;
              end if;
              if RS2_Data_IE_lat(Addr_Width downto 0) /= (0 to Addr_Width => '0') then
                busy_LS_wires      <= '1';
                nextstate_LS <= data_valid_waiting;
                ls_sc_read_addr <= RS1_Data_IE_wire_lat(Addr_Width - 1 downto SIMD_BITS+2);
                kmemstr_inflight(to_integer(unsigned(ls_rs1_to_sc))) <= '1';
                if to_integer(unsigned(ls_data_gnt_i)) /= 0 then
                  data_be_internal_wires     <= "1111";
                  data_req_o_wires           <= '1';
                  data_we_o_wires            <= '1';
                  data_addr_internal_wires <= RD_Data_IE_lat;
                  data_wdata_o_wires <= ls_sc_data_read_wire;
                end if;
                -- Increments the address of the SC memory every four words for KMEMSTR
                if data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
                  ls_sci_req(to_integer(unsigned(ls_rs1_to_sc))) <= '1';
                  if sc_word_count = SIMD-1 then
                    sc_word_count_wire <= 0;
                  else
                    sc_word_count_wire <= sc_word_count + 1;
                  end if;
                end if;
              end if;
            end if;
    
          elsif data_rvalid_i = '1' or load_valid = '1' or store_valid = '1' then
            if store_op = '1' or add_vect_bits(store_op_buf_wire) > 1 then -- SW or AMOSWAP data writing
              if decoded_instruction_LS(SW_bit_position) = '1' then  -- SW data writing
                data_wdata_o_wires     <= RS2_Data_IE_lat(31 downto 0);
                data_we_o_wires        <= '1';  -- is a writing
                data_be_internal_wires <= data_be_ID;
              end if;
              if decoded_instruction_LS(SH_bit_position) = '1' then  -- SH data writing
                case data_addr_internal_wires(1) is
                  when '0' =>
                    data_wdata_o_wires <= RS2_Data_IE_lat(31 downto 0);
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when '1' =>
                    data_wdata_o_wires <= RS2_Data_IE_lat(15 downto 0) & std_logic_vector(to_unsigned(0, 16));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when others =>
                    null;
                end case;
              end if;
              if decoded_instruction_LS(SB_bit_position) = '1' then  -- SB data writng
                case data_addr_internal_wires(1 downto 0) is
                  when "00" =>
                    data_wdata_o_wires <= RS2_Data_IE_lat(31 downto 0);
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when "01" =>
                    data_wdata_o_wires <= RS2_Data_IE_lat(23 downto 0) & std_logic_vector(to_unsigned(0, 8));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when "10" =>
                    data_wdata_o_wires <= RS2_Data_IE_lat(15 downto 0) & std_logic_vector(to_unsigned(0, 16));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when "11" =>
                    data_wdata_o_wires <= RS2_Data_IE_lat(7 downto 0) & std_logic_vector(to_unsigned(0, 24));
                    data_we_o_wires        <= '1';  -- is a writing
                    data_be_internal_wires <= data_be_ID;
                  when others =>
                    null;
                end case;
              end if;
            end if;
    
            if load_op = '1'  or add_vect_bits(load_op_buf_wire) > 1  then
              data_be_internal_wires <= data_be_ID;
              if decoded_instruction_LS(AMOSWAP_bit_position) = '1' then
                core_busy_LS_wires <= '1';          
              end if;
             end if;
          else
            nextstate_LS <= data_valid_waiting;
            busy_LS_wires <= '1';
            -- do not change to "if store_op = '0'" since that will disable store superscalar execution, because store_op resets to 0 on the next cycle
            if decoded_instruction_LS(SW_bit_position) = '0' and  decoded_instruction_LS(SH_bit_position) = '0' and decoded_instruction_LS(SB_bit_position)  = '0' then
              core_busy_LS_wires <= '1';
            end if;
          end if;
        end if;

      end case;
    end if;

  -- meccanismo di voting per LSU quando vengono effettuate store 
  -- if thread is zero and there is a store, send the voted value, else no
  if ( add_vect_bits(store_op_buf_wire) >1  or store_op = '1' or add_vect_bits(load_op_buf_wire) >1  or load_op = '1' ) then --la parte amo non serve le amo non sono supportate
    if ( harc_EXEC = 0 or harc_loss = 0 ) then --make the voting
--      data_addr_internal  <= (data_addr_internal_buf_wire(2) and data_addr_internal_buf_wire(1) ) or (data_addr_internal_buf_wire(2) and data_addr_internal_buf_wire(0) ) or (data_addr_internal_buf_wire(1) and data_addr_internal_buf_wire(0) );
      data_addr_internal  <= data_addr_internal_voted;
      data_wdata_o        <= (data_wdata_o_buf_wire(2) and data_wdata_o_buf_wire(1) ) or (data_wdata_o_buf_wire(2) and data_wdata_o_buf_wire(0) ) or (data_wdata_o_buf_wire(1) and data_wdata_o_buf_wire(0) );
      data_we_o           <= (data_we_o_buf_wire(2) and data_we_o_buf_wire(1) ) or ( data_we_o_buf_wire(2) and data_we_o_buf_wire(0) ) or ( data_we_o_buf_wire(1) and data_we_o_buf_wire(0) );
      data_req_o          <= (data_req_o_buf_wire(2) and data_req_o_buf_wire(1) ) or ( data_req_o_buf_wire(2) and data_req_o_buf_wire(0) ) or ( data_req_o_buf_wire(1) and data_req_o_buf_wire(0) );
      data_be_internal    <= (data_be_internal_buf_wire(2) and data_be_internal_buf_wire(1) ) or ( data_be_internal_buf_wire(2) and data_be_internal_buf_wire(0) ) or ( data_be_internal_buf_wire(1) and data_be_internal_buf_wire(0) );
--data be internal andrebbe ridondato con buffer
      data_width_ID_voted_wire <= (data_width_ID_buf_wire(2) and data_width_ID_buf_wire(1) ) or ( data_width_ID_buf_wire(2) and data_width_ID_buf_wire(0) ) or ( data_width_ID_buf_wire(1) and data_width_ID_buf_wire(0) );
      decoded_instruction_LS_voted_wire  <= (decoded_instruction_LS_buf_wire(2) and decoded_instruction_LS_buf_wire(1) ) or ( decoded_instruction_LS_buf_wire(2) and decoded_instruction_LS_buf_wire(0) ) or ( decoded_instruction_LS_buf_wire(1) and decoded_instruction_LS_buf_wire(0) );           

--    data_be_internal           <= data_be_internal_wires;
      ls_except_condition        <= ls_except_condition_wires;
      ls_taken_branch            <= ls_taken_branch_wires;
      core_busy_LS               <= core_busy_LS_wires;
      busy_LS                    <= busy_LS_wires;

    else
      data_addr_internal         <= (others => '0');
      data_wdata_o               <= (others => '0');
      data_we_o                  <= '0';
      data_req_o                 <= '0';
--      data_be_internal           <= (others => '0');
      data_be_internal           <= data_be_internal_wires;
        ls_except_condition        <= ls_except_condition_wires;
        ls_taken_branch            <= ls_taken_branch_wires;
        core_busy_LS               <= core_busy_LS_wires;
        busy_LS                    <= busy_LS_wires;
    end if;
-- commento perchè sia per load che per store devo fare voting
  else
    data_addr_internal         <= data_addr_internal_wires;
    data_wdata_o               <= data_wdata_o_wires;
    data_we_o                  <= data_we_o_wires;
    data_req_o                 <= data_req_o_wires;
    data_be_internal           <= data_be_internal_wires;
    ls_except_condition        <= ls_except_condition_wires;
    ls_taken_branch            <= ls_taken_branch_wires;
    core_busy_LS               <= core_busy_LS_wires;
    busy_LS                    <= busy_LS_wires;
  end if;
  
  end process;

  fsm_LS_state : process(clk_i, rst_ni) -- also implements some aux signals
  begin
    if rst_ni = '0' then
      state_LS <= normal; 
      if accl_en = 1 then
        sc_word_count <= 0;
	    harc_LS <= ACCL_NUM-1;
      end if;
      harc_LOAD <= THREAD_POOL_SIZE-1;
    elsif rising_edge(clk_i) then
      state_LS <= nextstate_LS;
  	  data_addr_internal_lat   <= data_addr_internal ;
      if accl_en = 1 then
	    --halt_lsu_lat             <= halt_lsu;
        sc_word_count            <= sc_word_count_wire;
        harc_LS                  <= harc_LS_wire;
      end if;
    end if;
  end process;


--TMR SYNC LOGIC CONTROL
  TTMR_LSU_SYNC : process(clk_i, rst_ni)  -- the combinational voting for TTMR
  begin
    if rst_ni = '0' then
        load_op_buf_lat          <= (others => '0');
        decoded_instruction_LS_voted <= (others => '0');
        data_width_ID_voted          <= (others => '0');
        load_op_buf            <= (others => '0');
        store_op_buf           <= (others => '0');
        ls_instr_req_buf       <= (others => '0');
        data_req_o_buf         <= (others => '0');
        data_we_o_buf          <= (others => '0');
    
        for h in harc_range loop
            data_addr_internal_buf(h)       <= (others => '0');
            data_wdata_o_buf(h)             <= (others => '0');
            data_be_internal_buf(h)         <= (others => '0');
            decoded_instruction_LS_buf(h)   <= (others => '0');
            data_width_ID_buf(h)            <= (others => '0');
        end loop;    
    
        store_valid <= '0';
        load_valid <= '0';
    --    LS_peripheral <= '0'; 
    --    LS_peripheral_counter <= 0;
    --    dummy_count <= 0;
    elsif rising_edge(clk_i) then
        load_op_buf_lat              <= load_op_buf_wire;
        decoded_instruction_LS_voted <= decoded_instruction_LS_voted_wire;
        data_width_ID_voted          <= data_width_ID_voted_wire;
        load_op_buf                  <= load_op_buf_wire;
        store_op_buf                 <= store_op_buf_wire;
        ls_instr_req_buf             <= ls_instr_req_buf_wire;
        data_addr_internal_buf       <= data_addr_internal_buf_wire;
        data_wdata_o_buf             <= data_wdata_o_buf_wire;
        data_req_o_buf               <= data_req_o_buf_wire;
        data_we_o_buf                <= data_we_o_buf_wire;
        data_be_internal_buf         <= data_be_internal_buf_wire;
        decoded_instruction_LS_buf   <= decoded_instruction_LS_buf_wire;
        data_width_ID_buf            <= data_width_ID_buf_wire;

        case state_LS is  
          when normal =>
            -- flag signal to replicate the data_rvalid_i signal when the store is redundant
            if add_vect_bits(store_op_buf_wire) > 0 then 
              --la terza LS è quella effettiva e deve funzionare con il segnale data_rvalid_i e non con store_valid(segnale fittizio) o load_valid(segnale fittizio)
              if (harc_EXEC /= 0 or harc_loss /= 0 ) then
                store_valid <= '1';
              else
                store_valid <= '0';                  
              end if;
--              data_addr_internal_buf_wire(harc_EXEC)         <= data_addr_internal_wires;
--              data_wdata_o_buf_wire(harc_EXEC)               <= data_wdata_o_wires;
            elsif add_vect_bits(load_op_buf_wire) > 0 then
              --la terza LS è quella effettiva e deve funzionare con il segnale data_rvalid_i e non con store_valid(segnale fittizio) o load_valid(segnale fittizio)
              if (harc_EXEC /= 0 or harc_loss /= 0 ) then
                load_valid <= '1';
              else
                load_valid <= '0';               
              end if;          
            else
              store_valid <= '0';
              load_valid <= '0';
            end if;
--            LS_peripheral_counter <= 0;

          -- create a counter for load store watchdog. When a fault comes during a peripheral load, is probable that the data_rvalid_i signal will never arrive
          -- so the core will go into stall. Its necessary a signal of watchdog that restore the core after 7 cycles
          when data_valid_waiting =>  
            store_valid <= '0';
            load_valid <= '0';

--            LS_peripheral_counter <= LS_peripheral_counter +1;

          when others =>
            store_valid <= '0';
            load_valid <= '0';
--            LS_peripheral_counter <= 0;
            null;
        end case;

      -- if there is a Load instruction from peripheral, you will do the voting at harc zero , if the voted results are all different you will load zero 
      -- that means that the value will be taken at next iteration loop
--     if (( harc_EXEC = 0 )  or ( harc_loss = 0 and fault_harc = '1' ) ) and ( (add_vect_bits(store_op_buf_wire) = 2 and store_op = '0' ) or ( add_vect_bits(load_op_buf_wire) = 2 and load_op = '0') ) then
--       dummy_count <= dummy_count + 1;
--     else
--       dummy_count <= 0;
--     end if;
--
--      --se ho una load da periferica io devo fare certe cose nel RF quindi serve un segnale che identifichi questo caso
--      if ( add_vect_bits(load_op_buf_wire) > 0  and data_addr_internal(28) = '1'  ) then
--        LS_peripheral <= '1'; 
--      else
--        LS_peripheral <= '0';  
--      end if;

    end if;
  end process;


--TMR LOGIC CONTROL
  TTMR_LSU_COMB : process(all)  -- the combinational voting for TTMR
  begin
  load_op_buf_wire            <= load_op_buf;
  store_op_buf_wire           <= store_op_buf;
  ls_instr_req_buf_wire       <= ls_instr_req_buf;

  data_addr_internal_buf_wire <= data_addr_internal_buf;
  data_wdata_o_buf_wire       <= data_wdata_o_buf;
  data_req_o_buf_wire         <= data_req_o_buf;
  data_we_o_buf_wire          <= data_we_o_buf;
  data_be_internal_buf_wire   <= data_be_internal_buf;  
  decoded_instruction_LS_buf_wire <= decoded_instruction_LS_buf;
  data_width_ID_buf_wire      <= data_width_ID_buf;

  if harc_fault_wire_lat(1) = '1' then
    load_op_buf_wire            <= (others => '0');
    store_op_buf_wire           <= (others => '0');
    ls_instr_req_buf_wire       <= (others => '0');
  end if;

  data_addr_internal_voted  <= (data_addr_internal_buf(2) and data_addr_internal_buf(1) ) or (data_addr_internal_buf(2) and data_addr_internal_buf_wire(0) ) or (data_addr_internal_buf(1) and data_addr_internal_buf_wire(0) );


      case state_LS is  
        when normal =>
          if (load_op = '1' or add_vect_bits(load_op_buf) > 0 ) or ( store_op = '1' or add_vect_bits(store_op_buf) > 0 )   then 
             -- questi sono latch per FPGA, quindi adnranno sostituiti con registri normali, e farò voting tra registro 2 , 1 e tra il wire
            data_addr_internal_buf_wire(harc_EXEC)         <= data_addr_internal_wires;
            data_wdata_o_buf_wire(harc_EXEC)               <= data_wdata_o_wires;
            data_we_o_buf_wire(harc_EXEC)                  <= data_we_o_wires;
            data_req_o_buf_wire(harc_EXEC)                 <= data_req_o_wires;
            data_be_internal_buf_wire(harc_EXEC)           <= data_be_internal_wires;
  
            data_width_ID_buf_wire(harc_EXEC)              <= data_width_ID;
            decoded_instruction_LS_buf_wire(harc_EXEC)     <= decoded_instruction_LS;     
  
            -- abbassa i segnali di controllo se ho due LS vicine 
            if ( harc_EXEC = 2 or harc_loss = 2 ) then 
                 data_we_o_buf_wire(1)                  <= '0';  
                 data_req_o_buf_wire(1)                 <= '0';
                 data_we_o_buf_wire(0)                  <= '0';
                 data_req_o_buf_wire(0)                 <= '0';
            elsif ( harc_EXEC = 1 or harc_loss = 1 )then
                 data_we_o_buf_wire(0)                  <= '0';
                 data_req_o_buf_wire(0)                 <= '0';
            end if;
  
          else
            if add_vect_bits(data_we_o_buf) >2 or add_vect_bits(data_req_o_buf) > 2 then
               for h in harc_range loop  
                 data_we_o_buf_wire(h)                  <= '0';
                 data_req_o_buf_wire(h)                 <= '0';
               end loop;                    
            end if;
          end if;
        when others =>
            if add_vect_bits(data_we_o_buf) > 2 or add_vect_bits(data_req_o_buf) > 2 then
               for h in harc_range loop  
                 data_we_o_buf_wire(h)                  <= '0';
                 data_req_o_buf_wire(h)                 <= '0';
               end loop;                    
            end if;
      end case;

      -- serve per evitare che la singola store nel ultimo thread venga annullata da un fault.
      -- store_op resta alto durante tutte le store mentre ls_instr_req resta alto solo per un ciclo quindi
      -- vanno gestiti in modo diverso.
  
      if LS_WB_wrong_EXEC = '1' then
        store_op_buf_wire(2) <= '0';
        store_op_buf_wire(1) <= '0';
        store_op_buf_wire(0) <= '0';
    
        load_op_buf_wire(2) <= '0';
        load_op_buf_wire(1) <= '0';
        load_op_buf_wire(0) <= '0';
    
        if ls_instr_req = '1' then
          ls_instr_req_buf_wire(2) <= ls_instr_req;
        else
          if add_vect_bits(ls_instr_req_buf) > 1 then
            ls_instr_req_buf_wire(2) <= '0';
          end if;
        end if;
    
        ls_instr_req_buf_wire(1) <= '0';
        ls_instr_req_buf_wire(0) <= '0';    
      end if; 
      
      if ( harc_EXEC = 2 or harc_loss = 2 ) then 
        store_op_buf_wire(2) <= store_op;
        store_op_buf_wire(1) <= '0';
        store_op_buf_wire(0) <= '0';
    
        load_op_buf_wire(2) <= load_op;
        load_op_buf_wire(1) <= '0';
        load_op_buf_wire(0) <= '0';
    
        if ls_instr_req = '1' then
          ls_instr_req_buf_wire(2) <= ls_instr_req;
        else
          if add_vect_bits(ls_instr_req_buf) > 1 then
            ls_instr_req_buf_wire(2) <= '0';
          end if;
        end if;
    
        ls_instr_req_buf_wire(1) <= '0';
        ls_instr_req_buf_wire(0) <= '0';
      elsif ( harc_EXEC = 1 or harc_loss = 1 )then
        store_op_buf_wire(1) <= store_op;
        store_op_buf_wire(0) <= '0';
    
        load_op_buf_wire(1) <= load_op;
        load_op_buf_wire(0) <= '0';
    
        if ls_instr_req = '1' then
          ls_instr_req_buf_wire(1) <= ls_instr_req;
        end if;
        ls_instr_req_buf_wire(0) <= '0';
    
      elsif LS_WB_wrong_EXEC = '0' and ( harc_EXEC = 0 or harc_loss = 0 ) then
    
        store_op_buf_wire(0) <= store_op;
        load_op_buf_wire(0) <= load_op;
    
        if ls_instr_req = '1' then
          ls_instr_req_buf_wire(0) <= ls_instr_req;
        end if;
    
----        --se ho avuto 2 store in precedenza e nell'harc 0 non ho l'ultima significa che ho un fault durante harc 0 e devo ripetere l'istruzione
----        if ((add_vect_bits(store_op_buf_wire) = 2 and store_op = '0') or (add_vect_bits(load_op_buf_wire) = 2 and load_op = '0') ) and dummy_count < 1 then 
----          fault_LSU <= '0';
------          fault_LSU <= '1';
----        else
----          fault_LSU <= '0';
----        end if;        
    
      end if;
----
----       --se ho un fault su LSU alzo il segnale di controllo che mi indica che è attiva la procedura di restore dal fault e che dura finquando harc_EXEC = 1
----     if fault_LSU = '1' then
----       restore_fault_LSU <= '1';
----     elsif ( harc_EXEC = 1 )  or ( harc_loss = 1 and fault_harc = '1' )then
----       restore_fault_LSU <= '0';
----     end if;        

  end process;


  -----------------------------------------------------------------------------------------------
  --   █████╗ ██████╗ ██████╗ ██████╗ ███████╗███████╗███████╗     ██████╗ ███████╗███╗   ██╗  --
  --  ██╔══██╗██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝    ██╔════╝ ██╔════╝████╗  ██║  --
  --  ███████║██║  ██║██║  ██║██████╔╝█████╗  ███████╗███████╗    ██║  ███╗█████╗  ██╔██╗ ██║  --
  --  ██╔══██║██║  ██║██║  ██║██╔══██╗██╔══╝  ╚════██║╚════██║    ██║   ██║██╔══╝  ██║╚██╗██║  --
  --  ██║  ██║██████╔╝██████╔╝██║  ██║███████╗███████║███████║    ╚██████╔╝███████╗██║ ╚████║  --
  --  ╚═╝  ╚═╝╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝     ╚═════╝ ╚══════╝╚═╝  ╚═══╝  --
  -----------------------------------------------------------------------------------------------

  LSU_Mapper_comb : process(all)
  begin
    add_op_A <= (others => '0');
    add_op_B <= (others => '0');

    -- Perform the addition ---------------------------------------------------
    add_out <= std_logic_vector(signed(add_op_A) + signed(add_op_B));
    ---------------------------------------------------------------------------

    if ( harc_EXEC = 0 or harc_loss = 0 ) then --make the voting
      -- MAP input address generator -----------------------------------------------------------------------------------
      if load_op = '1' or add_vect_bits(load_op_buf_wire) > 1  then  -- address building operands
        add_op_A <= RS1_data_IE;
        add_op_B <= I_immediate(instr_word_IE);
      end if;
      if store_op = '1' or add_vect_bits(store_op_buf_wire) > 1 then -- address building operands
        add_op_A <= RS1_data_IE;
        add_op_B <= S_immediate(instr_word_IE);
      end if;
      if accl_en = 1 then
        if decoded_instruction_LS_voted(KMEMLD_bit_position)   = '1' or  -- calculates overflow spm write
           decoded_instruction_LS_voted(KBCASTLD_bit_position) = '1' then
          add_op_A <= (Addr_Width to 31 => '0') & RD_data_IE(Addr_Width -1 downto 0);
          add_op_B <= (Addr_Width to 31 => '0') & std_logic_vector(unsigned(RS2_data_IE(Addr_Width -1 downto 0))-1);
        end if;
        if decoded_instruction_LS_voted(KMEMSTR_bit_position) = '1' then -- calculates overflow spm read
          add_op_A <= (Addr_Width to 31 => '0') & RS1_data_IE(Addr_Width -1 downto 0);
          add_op_B <= (Addr_Width to 31 => '0') & std_logic_vector(unsigned(RS2_data_IE(Addr_Width -1 downto 0))-1);
        end if;
      end if;
    else
      -- MAP input address generator -----------------------------------------------------------------------------------
      if load_op = '1' or add_vect_bits(load_op_buf_wire) > 1 then  -- address building operands
        add_op_A <= RS1_data_IE;
        add_op_B <= I_immediate(instr_word_IE);
      end if;
      if store_op = '1' or add_vect_bits(store_op_buf_wire) > 1 then -- address building operands
        add_op_A <= RS1_data_IE;
        add_op_B <= S_immediate(instr_word_IE);
      end if;
      if accl_en = 1 then
        if decoded_instruction_LS(KMEMLD_bit_position)   = '1' or  -- calculates overflow spm write
           decoded_instruction_LS(KBCASTLD_bit_position) = '1' then
          add_op_A <= (Addr_Width to 31 => '0') & RD_data_IE(Addr_Width -1 downto 0);
          add_op_B <= (Addr_Width to 31 => '0') & std_logic_vector(unsigned(RS2_data_IE(Addr_Width -1 downto 0))-1);
        end if;
        if decoded_instruction_LS(KMEMSTR_bit_position) = '1' then -- calculates overflow spm read
          add_op_A <= (Addr_Width to 31 => '0') & RS1_data_IE(Addr_Width -1 downto 0);
          add_op_B <= (Addr_Width to 31 => '0') & std_logic_vector(unsigned(RS2_data_IE(Addr_Width -1 downto 0))-1);
        end if;
      end if;
    end if;      

    ---------------------------------------------------------------------------------------------------------------
  end process;

--------------------------------------------------------------------- end of LSU -----------------
--------------------------------------------------------------------------------------------------

end LSU;
--------------------------------------------------------------------------------------------------
-- END of Load-Store architecture ----------------------------------------------------------------
--------------------------------------------------------------------------------------------------
