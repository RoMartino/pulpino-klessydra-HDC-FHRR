----------------------------------------------------------------------------------------------------------------
--  Stage ID - (Instruction decode and registerfile read)                                                     --
--  Author(s): Abdallah Cheikh abdallah.cheikh@uniroma1.it (abdallah93.as@gmail.com)                          --
--                                                                                                            --
--  Date Modified: 07-04-2020                                                                                  --
----------------------------------------------------------------------------------------------------------------
--  Registerfiles of the incoming hart are read in this stage in parallel with the decoding                   --
--  Two types of registerfiles can be generaated for XILINX FPGAs LUTAM based or FF based dpeneding on the    --
--  setting of the generic variaabble chosen                                                                  --
--  The scratchpad memory mapper also exists in this stage, which maps the address to the corresponding SPM   --
--  This pipeline stage always takes one cycle latency                                                        --
----------------------------------------------------------------------------------------------------------------

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

-- pipeline  pinout --------------------
entity REGISTERFILE is
  generic(
    THREAD_POOL_SIZE           : integer;
    LUTRAM_RF                  : natural;
    accl_en                    : natural;
    SPM_NUM		               : natural;  
    Addr_Width                 : natural;
    SPM_STRT_ADDR              : std_logic_vector(31 downto 0);
    RF_SIZE                    : natural;
    RF_CEIL                    : natural;
    SPM_ADDR_WID               : natural
    );
  port (
    -- clock, reset active low
    clk_i                      : in  std_logic;
    rst_ni                     : in  std_logic;
	-- Branch Control Signals
    set_except_condition         : in std_logic;
	  harc_ID                    : in  integer range THREAD_POOL_SIZE-1 downto 0;
    pc_ID                      : in  std_logic_vector(31 downto 0);  -- pc_ID is PC entering ID stage
  	core_busy_IE               : in  std_logic;
  	core_busy_LS               : in  std_logic;
    core_busy_WB               : out  std_logic;
    ls_parallel_exec           : in  std_logic;
    dsp_parallel_exec          : in  std_logic;
    dsp_to_jump                : in  std_logic;
    instr_rvalid_ID            : in  std_logic;
  	instr_word_ID_lat          : in  std_logic_vector(31 downto 0);
  	LS_WB_EN                   : in  std_logic;
    LS_PE_WB_EN                : in  std_logic;
  	IE_WB_EN                   : in  std_logic;
  	MUL_WB_EN                  : in  std_logic;
  	IE_WB                      : in  std_logic_vector(31 downto 0);
  	MUL_WB                     : in  std_logic_vector(31 downto 0);
  	LS_WB                      : in  std_logic_vector(31 downto 0);
--  LS_WB                      : in array_2d(THREAD_POOL_SIZE-1 downto 0)(31 downto 0);

  --TTMR SIGNALS
    harc_EXEC                  : in integer range THREAD_POOL_SIZE-1 downto 0;
    harc_IF                    : in integer range THREAD_POOL_SIZE-1 downto 0;
    dest_valid                 : in std_logic;
    instr_word_ID              : in  std_logic_vector(31 downto 0);

  --TTMR from PC
    WB_wrong                   : out std_logic;
    LS_WB_wrong                : out std_logic;
    WB_wrong_lat               : out std_logic;
    LS_WB_wrong_lat            : out std_logic; 
    LS_WB_wrong_EXEC           : out std_logic;
    fault_PC                   : in std_logic;
    fault_PC_lat               : in std_logic;
    harc_fault                 : in std_logic_vector(THREAD_POOL_SIZE-1 downto 0);
    harc_fault_wire            : in std_logic_vector(THREAD_POOL_SIZE-1 downto 0);
    harc_fault_wire_lat        : in std_logic_vector(1 downto 0);
    harc_fault_previous        : in std_logic;
    harc_loss                  : in integer range THREAD_POOL_SIZE-1 downto 0;
    harc_loss_lat_wire         : in integer range THREAD_POOL_SIZE-1 downto 0;
    fault_harc                 : in std_logic;
    fault_harc_lat             : in std_logic;
    restore_fault_PC           : in std_logic;

  -- TTMR from LSU
    load_op_buf_wire           : in std_logic_vector(2 downto 0);   
    load_op_buf_lat            : in std_logic_vector(2 downto 0);   
    ls_instr_req_buf_wire      : in std_logic_vector(2 downto 0);   
    ls_instr_req_buf           : in std_logic_vector(2 downto 0);   
    LS_peripheral              : in std_logic; 


  	instr_word_LS_WB           : in  std_logic_vector(31 downto 0);
  	instr_word_IE_WB           : in  std_logic_vector(31 downto 0);
  	harc_LS_WB                 : in  integer range THREAD_POOL_SIZE-1 downto 0;
  	harc_IE_WB                 : in  integer range THREAD_POOL_SIZE-1 downto 0;
    harc_WB                    : out integer range THREAD_POOL_SIZE-1 downto 0;
    RS1_Data_IE                : out std_logic_vector(31 downto 0);
    RS2_Data_IE                : out std_logic_vector(31 downto 0);
    RD_Data_IE                 : out std_logic_vector(31 downto 0);
    rs1_to_sc                  : out std_logic_vector(SPM_ADDR_WID-1 downto 0);
    rs2_to_sc                  : out std_logic_vector(SPM_ADDR_WID-1 downto 0);
    rd_to_sc                   : out std_logic_vector(SPM_ADDR_WID-1 downto 0);
    data_addr_internal_IE      : out std_logic_vector(31 downto 0);
	  regfile                    : out array_3d(THREAD_POOL_SIZE-1 downto 0)(RF_SIZE-1 downto 0)(31 downto 0)
    );
end entity;  ------------------------------------------


-- Klessydra fT13x (4 stages) pipeline implementation -----------------------
architecture RF of REGISTERFILE is

  subtype harc_range is integer range THREAD_POOL_SIZE - 1 downto 0;

  signal regfile_lutram_rs1  : array_2d((THREAD_POOL_SIZE*RF_SIZE)-1 downto 0)(31 downto 0);
  signal regfile_lutram_rs2  : array_2d((THREAD_POOL_SIZE*RF_SIZE)-1 downto 0)(31 downto 0);
  signal regfile_lutram_rd   : array_2d((THREAD_POOL_SIZE*RF_SIZE)-1 downto 0)(31 downto 0);

  -- signals for TTMR voting
  signal regfile_voted_wire     : array_2d(RF_SIZE-1 downto 0)(31 downto 0);
--  signal regfile_voted          : array_2d(RF_SIZE-1 downto 0)(31 downto 0);
  signal WB_RD_voted            : std_logic_vector(31 downto 0);
  signal WB_RD_voted_wire       : std_logic_vector(31 downto 0);
  signal WB_RD_buf_wire         : array_2d(harc_range)(31 downto 0);
  signal WB_RD_buf              : array_2d(harc_range)(31 downto 0);
  signal WB_EN_buf_wire         : std_logic_vector(2 downto 0);
  signal WB_EN_buf              : std_logic_vector(2 downto 0);
  signal RD_dummy_buf           : array_2d_int_fix(harc_range);
  signal RD_dummy_wire          : integer range 31 downto 0;
  signal WB_ready               : std_logic;
  signal rs1_bypass_wire        : std_logic;
  signal rs2_bypass_wire        : std_logic;
  signal rs1_bypass_lat         : std_logic;
  signal rs2_bypass_lat         : std_logic;
  signal rs1_bypass             : std_logic;
  signal rs2_bypass             : std_logic;
  signal harc_WB_lat            : harc_range;
--  signal fault_RF               : std_logic;
  signal LS_WB_wrong_EXEC_wire  : std_logic;

--  alias instr_word_ID_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Pipe.FETCH.instr_word_ID : std_logic_vector(31 downto 0) >>;
--  alias dest_valid_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Pipe.DECODE.dest_valid : std_logic >>;

--  alias fault_LSU_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Pipe.fault_LSU : std_logic >>;
--  alias harc_EXEC_alias is << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.harc_EXEC : harc_range >>; 
--  alias harc_IF_alias is << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.harc_IF : harc_range >>; 
--  alias set_except_condition_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.set_except_condition : std_logic >>;

--  alias fault_PC_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.fault_PC_wire : std_logic >>;
--  alias fault_PC_lat_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.fault_PC_lat : std_logic >>;
--  alias harc_fault_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.harc_fault : std_logic_vector(2 downto 0) >>;
--  alias harc_fault_wire_lat_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.harc_fault_wire_lat : std_logic_vector(1 downto 0) >>;
--  alias harc_loss_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.harc_loss  : integer >>;
--  alias harc_loss_lat_wire_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.harc_loss_lat_wire  : integer >>;
--  alias fault_harc_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.fault_harc  : std_logic >>;
--  alias fault_harc_lat_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.fault_harc_lat  : std_logic >>;
--  alias restore_fault_PC_alias is << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.restore_fault_PC : std_logic >>;
--  alias fault_LSU_lat_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Prg_Ctr.fault_LSU_lat : std_logic >>;

--  alias load_op_buf_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Pipe.LSU.load_op_buf : std_logic_vector(2 downto 0) >>;
--  alias load_op_buf_lat_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Pipe.LSU.load_op_buf_lat : std_logic_vector(2 downto 0) >>;
--  alias ls_instr_req_buf_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Pipe.LSU.ls_instr_req_buf : std_logic_vector(2 downto 0) >>;
--  alias LS_peripheral_alias is  << signal .tb.top_i.core_region_i.CORE.RISCV_CORE.Pipe.LSU.LS_peripheral :  std_logic >>;

  
  attribute ram_style : string;
  attribute ram_style of RS1_Data_IE        : signal is "reg";
  attribute ram_style of RS2_Data_IE        : signal is "reg";
  attribute ram_style of RD_Data_IE         : signal is "reg";
  attribute ram_style of regfile_lutram_rs1 : signal is "distributed";
  attribute ram_style of regfile_lutram_rs2 : signal is "distributed";
  attribute ram_style of regfile_lutram_rd  : signal is "distributed";

  signal ID_rs1_to_sc           : std_logic_vector(SPM_ADDR_WID-1 downto 0);
  signal ID_rs2_to_sc           : std_logic_vector(SPM_ADDR_WID-1 downto 0);
  signal ID_rd_to_sc            : std_logic_vector(SPM_ADDR_WID-1 downto 0);

  -- instruction operands
  signal RS1_Addr_IE            : std_logic_vector(4 downto 0);   -- debugging signals
  signal RS2_Addr_IE            : std_logic_vector(4 downto 0);   -- debugging signals
  signal RD_Addr_IE             : std_logic_vector(4 downto 0);   -- debugging signals
  signal RD_EN                  : std_logic;
  signal WB_RD                  : std_logic_vector(31 downto 0);
  signal WB_EN                  : std_logic;
  signal harc_LS_count          : integer range THREAD_POOL_SIZE-1 downto 0;
  signal instr_word_WB          : std_logic_vector(31 downto 0);
--  signal ls_counter_on             : std_logic;

  function rs1 (signal instr : in std_logic_vector(31 downto 0)) return integer is
  begin
    return to_integer(unsigned(instr(15+(RF_CEIL-1) downto 15)));
  end;

  function rs2 (signal instr : in std_logic_vector(31 downto 0)) return integer is
  begin
    return to_integer(unsigned(instr(20+(RF_CEIL-1) downto 20)));
  end;

  function rd (signal instr : in std_logic_vector(31 downto 0)) return integer is
  begin
    return to_integer(unsigned(instr(7+(RF_CEIL-1) downto 7)));
  end;

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

  RF_FF : if LUTRAM_RF = 0 generate

  RF_ACCESS : process(clk_i, rst_ni, instr_word_ID_lat)  -- synch single state process
  begin
    if rst_ni = '0' then
      for h in harc_range loop
        for i in  RF_SIZE-1 downto 0  loop 
          regfile(h)(i) <= (others => '0');
        end loop;
      end loop;
      RS1_Data_IE <= (others => '0');
      RS2_Data_IE <= (others => '0');
      RD_Data_IE <= (others => '0');
    elsif rising_edge(clk_i) then
      RD_Data_IE <= (others => '0');
      if core_busy_IE = '1' or core_busy_LS = '1' or ls_parallel_exec = '0'  or dsp_parallel_exec = '0' or fault_PC = '1' or WB_wrong = '1' or LS_WB_wrong = '1' then -- the instruction pipeline is halted
      elsif instr_rvalid_ID = '0' then -- wait for a valid instruction
      else  -- process the incoming instruction 

        ------------------------------------------------------------
        --  ██████╗ ███████╗ ██████╗ ███████╗██╗██╗     ███████╗  --
        --  ██╔══██╗██╔════╝██╔════╝ ██╔════╝██║██║     ██╔════╝  --
        --  ██████╔╝█████╗  ██║  ███╗█████╗  ██║██║     █████╗    --
        --  ██╔══██╗██╔══╝  ██║   ██║██╔══╝  ██║██║     ██╔══╝    --
        --  ██║  ██║███████╗╚██████╔╝██║     ██║███████╗███████╗  --
        --  ╚═╝  ╚═╝╚══════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝╚══════╝  --
        ------------------------------------------------------------

        ----- REGISTERFILE READ IS DONE HERE --------------------------------------------------------------------------------------------------------------
----      -- pragma translate_on      

          data_addr_internal_IE <= std_logic_vector(signed(regfile_voted_wire(rs1(instr_word_ID_lat))) + signed(S_immediate(instr_word_ID_lat)));

          if WB_wrong_lat = '0' and LS_WB_wrong_lat = '0' then
            if rs1_bypass_wire = '1' and rs2_bypass_wire = '1' then   
              RS1_Data_IE <= WB_RD_buf_wire(harc_ID);
              RS2_Data_IE <= WB_RD_buf_wire(harc_ID);
            elsif rs1_bypass_wire = '1'  then   
              RS1_Data_IE <= WB_RD_buf_wire(harc_ID);
              RS2_Data_IE <= regfile_voted_wire(rs2(instr_word_ID_lat));  
            elsif rs2_bypass_wire = '1' then
              RS1_Data_IE <= regfile_voted_wire(rs1(instr_word_ID_lat));
              RS2_Data_IE <= WB_RD_buf_wire(harc_ID);
            else            
              RS1_Data_IE <= regfile_voted_wire(rs1(instr_word_ID_lat));
              RS2_Data_IE <= regfile_voted_wire(rs2(instr_word_ID_lat));
            end if;  

            if accl_en = 1 then
              if dsp_to_jump = '0' then
                RD_Data_IE <= regfile_voted_wire(rd(instr_word_ID_lat)); -- only the DSP unit reads the accelerator
              else
                RD_Data_IE <= regfile_voted_wire(0);
              end if;
            end if;          
          else 
            if rs2_bypass_wire = '1' and rs1_bypass_wire = '1' then   
              RS1_Data_IE <= WB_RD_voted_wire;
              RS2_Data_IE <= WB_RD_voted_wire;
            elsif rs1_bypass_wire = '1'  then   
              RS1_Data_IE <= WB_RD_voted_wire;
              RS2_Data_IE <= regfile_voted_wire(rs2(instr_word_ID_lat));  
            elsif rs2_bypass_wire = '1' then
              RS1_Data_IE <= regfile_voted_wire(rs1(instr_word_ID_lat));
              RS2_Data_IE <= WB_RD_voted_wire;
            else            
              RS1_Data_IE <= regfile_voted_wire(rs1(instr_word_ID_lat));
              RS2_Data_IE <= regfile_voted_wire(rs2(instr_word_ID_lat));
            end if;  

            if accl_en = 1 then
              if dsp_to_jump = '0' then
                RD_Data_IE <= regfile_voted_wire(rd(instr_word_ID_lat)); -- only the DSP unit reads the accelerator
              else
                RD_Data_IE <= regfile_voted_wire(0);
              end if;
            end if;          
          end if;

         -- pragma translate_off
          RD_Data_IE  <= regfile_voted_wire(rd(instr_word_ID_lat)); -- reading the 'rd' data here is only for debugging purposes if the acclerator is disabled
          RS1_Addr_IE <= std_logic_vector(to_unsigned(rs1(instr_word_ID_lat), 5)); -- debugging signals
          RS2_Addr_IE <= std_logic_vector(to_unsigned(rs2(instr_word_ID_lat), 5)); -- debugging signals
          RD_Addr_IE  <= std_logic_vector(to_unsigned(rd(instr_word_ID_lat), 5)); -- debugging signals
            -- pragma translate_on

        end if;  
       ---------------------------------------------------------------------------------------------------------------------------------------------------
    
--    -- se c'è un fault, pulisci tutti i registri rimettendoci il valore di regfile_voted_wire.
--        if fault_RF = '1' then
--          for h in harc_range loop
--            for i in RF_SIZE-1 downto 0 loop
--              regfile(h)(i) <= regfile_voted_wire(i); 
--            end loop;
--          end loop;
--        end if;
    
    -- se ho finito di fare wb e wb_en ='1' posso scrivere dentro RF il risultato del voting
    -- tra i tre buffer di write back precedenti.
        if harc_WB = 0  and   add_vect_bits(WB_EN_buf_wire) >= 2  then --  ( WB_EN_lat = '1' or LS_PE_WB_EN = '1' )then
          for h in harc_range loop
            regfile(h)(RD_dummy_wire) <= WB_RD_voted_wire;    
          end loop;
        end if;        

    end if;  -- clk
  end process;
  end generate;



  RF_LUTRAM : if LUTRAM_RF = 1 generate
  RF_RD_EN : process(all)  -- synch single state process
  begin
    RD_EN <= '0';
    if core_busy_IE = '1' or core_busy_LS = '1' or ls_parallel_exec = '0'  or dsp_parallel_exec = '0' then -- the instruction pipeline is halted
    elsif instr_rvalid_ID = '0' then -- wait for a valid instruction
    else  -- process the incoming instruction 
      RD_EN <= '1';
    end if;  -- instr. conditions
  end process;

  RS1_ACCESS : process(clk_i)  -- synch single state process
  begin
    if rising_edge(clk_i) then
      if RD_EN = '1' then 
        data_addr_internal_IE <= std_logic_vector(signed(regfile_lutram_rs1(32*harc_ID+rs1(instr_word_ID_lat))) + signed(S_immediate(instr_word_ID_lat)));
        if rs1(instr_word_ID_lat) /= 0 then
          RS1_Data_IE <= regfile_lutram_rs1(32*harc_ID+rs1(instr_word_ID_lat));
        else
          RS1_Data_IE <= (others => '0');
        end if;
      -- pragma translate_off
       RD_Data_IE  <= regfile_lutram_rs1(32*harc_ID+rd(instr_word_ID_lat)); -- reading the 'rd' data here is only for debugging purposes when the acclerator is disabled
       RS1_Addr_IE <= std_logic_vector(to_unsigned(rs1(instr_word_ID_lat), 5)); -- debugging signals
       RS2_Addr_IE <= std_logic_vector(to_unsigned(rs2(instr_word_ID_lat), 5)); -- debugging signals
       RD_Addr_IE  <= std_logic_vector(to_unsigned(rd(instr_word_ID_lat), 5)); -- debugging signals
      -- pragma translate_on
      end if;  -- instr. conditions
      if WB_EN = '1' then
        regfile_lutram_rs1(32*harc_WB+rd(instr_word_WB)) <= WB_RD;--hoaggiunto 0 ma va tolto
      end if;
    end if;  -- clk
  end process;

  RS2_ACCESS : process(clk_i)  -- synch single state process
  begin
    if rising_edge(clk_i) then
      if RD_EN = '1' then 
        if rs2(instr_word_ID_lat) /= 0 then
          RS2_Data_IE <= regfile_lutram_rs2(32*harc_ID+rs2(instr_word_ID_lat));
        else
          RS2_Data_IE <= (others => '0');
        end if;
      end if;  -- instr. conditions
      if WB_EN = '1' then
        regfile_lutram_rs2(32*harc_WB+rd(instr_word_WB)) <= WB_RD;--hoaggiunto 0 ma va tolto
      end if;
    end if;  -- clk
  end process;

  RD_LUTRAM : if accl_en = 1 generate
  RD_ACCESS : process(clk_i)  -- synch single state process
  begin
    if rising_edge(clk_i) then
      if accl_en = 1 then
        if RD_EN = '1' then
          RD_Data_IE <= regfile_lutram_rd(32*harc_ID+rd(instr_word_ID_lat)); -- only the DSP unit reads the accelerator
        end if;  -- instr. conditions
      end if;
      if WB_EN = '1' then
        regfile_lutram_rd(32*harc_WB+rd(instr_word_WB)) <= WB_RD; --hoaggiunto 0 ma va tolto
      end if;
    end if;  -- clk
  end process;

  end generate;
  end generate;


-- the combinational voting for TTMR
  TTMR_RF_COMB : process(all)  
  begin
--  fault_RF <= '0';
  rs1_bypass_wire <= rs1_bypass_lat;
  rs2_bypass_wire <= rs2_bypass_lat;
--  regfile_voted_wire <= regfile_voted;
  WB_RD_buf_wire     <= WB_RD_buf;
  WB_RD_voted_wire   <= WB_RD_voted;

  if rst_ni = '0' then
    for h in harc_range loop
      WB_RD_buf_wire(h) <= (others => '0');
    end loop;
    WB_RD_voted_wire <= (others => '0');
    rs1_bypass_wire <= '0';
    rs2_bypass_wire <= '0';
  end if; 


  -- faccio voting tra i buffer di writeback 
  if harc_WB = 0 and WB_ready = '1' then
    -- se c'è una load visto che è una sola prendo direttamente il valore senza fare voting, questo può essere un problema quando harc_WB viene messo a zero come default condition o altro
    -- tuttavia anche se sporco WB_RD_voted_wire, questo valore non dovrebbe mai essere scritto nel RF se non ho WB_EN_buf che lo permette
    if add_vect_bits(load_op_buf_lat) > 1 then
      WB_RD_voted_wire    <= WB_RD;
    else
      WB_RD_voted_wire    <= ( WB_RD_buf_wire(2) and WB_RD_buf_wire(1) ) or ( WB_RD_buf_wire(2) and WB_RD ) or ( WB_RD_buf_wire(1) and WB_RD );
    end if;      
  end if;

  -- controllo le dipendenze tra gli operandi di istruzioni adiacenti, se gli rs1 o rs2 sono uguali 
  -- al registro destinazione dell'istruzione precedente (cioè quella in WB) allora non prendo i valori
  -- dal RF (perchè non saranno ancora scritti), ma li prendo dai buffer. 
  if ( harc_ID = 2 or harc_ID = 1 )  and RD_dummy_wire /= 0 and (harc_fault_wire = "000" and harc_fault_wire_lat = "00" ) then
    if ( rs1(instr_word_ID_lat) = RD_dummy_wire ) and ( rs2(instr_word_ID_lat) = RD_dummy_wire ) then
      rs1_bypass_wire <= '1';
      rs2_bypass_wire <= '1';
    elsif rs1(instr_word_ID_lat) = RD_dummy_wire then
      rs1_bypass_wire <= '1';
      rs2_bypass_wire <= '0';
    elsif rs2(instr_word_ID_lat) = RD_dummy_wire then
      rs1_bypass_wire <= '0';
      rs2_bypass_wire <= '1';
    else
      rs1_bypass_wire <= '0';
      rs2_bypass_wire <= '0';
    end if;
  elsif harc_EXEC = 1 or harc_loss = 1 then  
    rs1_bypass_wire <= '0';
    rs2_bypass_wire <= '0';
  end if;

  --  faccio sempre voting tra i RF cosi da avere sempre un risultato giusto sul wire che verrà sempre letto
  for i in  RF_SIZE-1 downto 0  loop  
      regfile_voted_wire(i) <= ( regfile(2)(i) and regfile(1)(i) ) or ( regfile(2)(i) and regfile(0)(i) ) or ( regfile(1)(i) and regfile(0)(i) );  
--   if ( regfile(2)(i) xor regfile(1)(i) ) = ( 0 to 31 => '0' ) then
--     regfile_voted_wire(i) <= regfile(1)(i); 
--   elsif ( regfile(2)(i) xor regfile(1)(i) ) = ( 0 to 31 => '1' ) then
--     regfile_voted_wire(i) <= regfile(0)(i); 
--   end if;        
  end loop;  

--  -- CONTROLLO DEI FAULT
--  if ( regfile(2) /= regfile(1) ) or ( regfile(2) /= regfile(0) ) or ( regfile(1) /= regfile(0) ) then
--    fault_RF <= '1';
--  end if; 

-- riempimento del buffer di WB (e del buffer per destinazione di WB) per voting. se c'è wb_wrong significa che uno tra WB_RD(2) o WB_RD(1) è sbagliato, quindi prendi WB_RD(0)
-- ipotizzando che un fault non cada due volte consecutive a distanza di pochi ns tra loro. Uso wb_wrong_lat per non avere iteration limit
  if WB_EN = '1' and WB_wrong_lat = '0' and LS_WB_wrong_lat ='0' then  -- POSSIBILI LATCH non c'è una condizione di Default
    if add_vect_bits(load_op_buf_wire) > 1 then
      WB_RD_buf_wire(harc_WB) <= (others => '0');      
    else  
      WB_RD_buf_wire(harc_WB) <= WB_RD;    
    end if;
      
  elsif WB_EN = '1' and  ( (WB_wrong_lat ='1' and LS_WB_wrong_lat ='0') or ( WB_wrong_lat = '0' and LS_WB_wrong_lat ='1' ) ) and harc_WB = 0 then
    for h in harc_range loop
      WB_RD_buf_wire(h) <= WB_RD;      
    end loop;

--  elsif WB_EN = '1' and WB_wrong_lat = '0' and LS_WB_wrong_lat ='1'  and harc_WB = 0 then
--    for h in harc_range loop
--      WB_RD_buf_wire(h) <= WB_RD;      
--    end loop;
  end if;
  end process;

  TTMR_RF_SYNCH : process(clk_i, rst_ni)  -- synch single state process
  begin
    if rst_ni = '0' then
      for h in harc_range loop
--        regfile_voted(h) <= (others => '0');
        WB_RD_buf(h) <= (others => '0');
      end loop;
      LS_WB_wrong_EXEC <= '0';
    elsif rising_edge(clk_i) then
--      regfile_voted <= regfile_voted_wire;
      WB_RD_buf <= WB_RD_buf_wire;
      LS_WB_wrong_EXEC <= LS_WB_wrong_EXEC_wire;
    end if;      
  end process;    

-----------------------------------------------------------------------------------------------------
-- Stage WB - (WRITEBACK)
-----------------------------------------------------------------------------------------------------

  -- voglio fare bypass quando non ho in atto procedure di restore da fault su PC o fault su LSU  
  rs1_bypass <= ( rs1_bypass_wire or rs1_bypass_lat ); --when (restore_fault_PC = '0' ) else '0'; --and restore_fault_LSU = '0') else '0' ;
  rs2_bypass <= ( rs2_bypass_wire or rs2_bypass_lat ); --when (restore_fault_PC = '0' ) else '0'; --and restore_fault_LSU = '0') else '0' ;

  harc_WB <= -- harc_loss_lat_wire when (fault_harc = '0' and fault_harc_lat = '1') else
----             harc_LS_count when ls_counter_on = '1' else   
             harc_LS_WB when LS_WB_EN = '1' else 
             0 when (add_vect_bits(WB_EN_buf) >= 1) and ( harc_EXEC = 2 or harc_loss = 2  ) else --serve perchè se fault arriva su istr_word_id durante thread 0 non faccio mai wb di 0 e quindi non faccio voting, devo forzare wb ad essere 0, inoltrre se fault arriva e fa fare un WB_EN io devo poter tornare allo stato di default in harc_WB
--             0 when (add_vect_bits(WB_EN_buf_wire) >= 2) and ( harc_EXEC = 2 or ( harc_loss = 2 and fault_harc = '1' ) ) and add_vect_bits(ls_instr_req_buf) < 2 else --serve perchè se fault arriva su istr_word_id durante thread 0 non faccio mai wb di 0 e quindi non faccio voting, devo forzare wb ad essere 0
--             0 when (WB_EN_buf_wire = "110" or WB_EN_buf_wire = "111" )  and ( harc_EXEC = 2 or ( harc_loss = 2 and fault_harc = '1' ) ) and add_vect_bits(ls_instr_req_buf_wire) < 2 else --serve perchè se fault arriva su istr_word_id durante thread 0 non faccio mai wb di 0 e quindi non faccio voting, devo forzare wb ad essere 0
--             harc_WB_lat when add_vect_bits(ls_instr_req_buf) >= 2  else
             harc_WB_lat when ( LS_WB_EN = '0' and IE_WB_EN = '0' and MUL_WB_EN = '0' and add_vect_bits(harc_fault_wire) >= 0 ) else
--             harc_WB_lat when ( LS_WB_EN = '0' and IE_WB_EN = '0' and MUL_WB_EN = '0' and ( harc_fault = "000" or harc_fault = "010") )  else
--             harc_WB_lat when ( LS_WB_EN = '0' and IE_WB_EN = '0' and MUL_WB_EN = '0' and harc_fault_wire = "010" ) else
               harc_IE_WB  when ( IE_WB_EN = '1' or MUL_WB_EN = '1' ) else 0;


-- harc_WB <= harc_loss_lat_wire when (fault_harc = '0' and fault_harc_lat = '1') else
--             harc_LS_count when ls_counter_on = '1' else   
--             harc_LS_WB when LS_WB_EN = '1' else 
--             0 when (WB_EN_buf = "110" or WB_EN_buf = "111" )  and ( harc_EXEC = 2 or ( harc_loss = 2 and fault_harc = '1' ) ) and add_vect_bits(ls_instr_req_buf_wire) < 2 else --serve perchè se fault arriva su istr_word_id durante thread 0 non faccio mai wb di 0 e quindi non faccio voting, devo forzare wb ad essere 0
--  --           harc_WB_lat when add_vect_bits(ls_instr_req_buf) >= 2  else
--             harc_WB_lat when ( LS_WB_EN = '0' and IE_WB_EN = '0' and MUL_WB_EN = '0' and harc_fault_wire = "000" ) else
--             harc_WB_lat when ( LS_WB_EN = '0' and IE_WB_EN = '0' and MUL_WB_EN = '0' and harc_fault_wire = "010" ) else
--             harc_IE_WB;
--


  instr_word_WB <= instr_word_LS_WB when ( LS_WB_EN = '1') else 
                   instr_word_IE_WB when (IE_WB_EN = '1' or MUL_WB_EN = '1' ) else (others => '0'); 
  
  WB_EN <= '1' when (((LS_WB_EN = '1' or IE_WB_EN = '1' or MUL_WB_EN = '1' ) and fault_PC = '0') or
                    ((LS_WB_EN = '1' or IE_WB_EN = '1' or MUL_WB_EN = '1'  ) and fault_PC = '1' )  ) else '0';


  WB_RD <= LS_WB when LS_WB_EN = '1' else  
           MUL_WB when MUL_WB_EN = '1'  else 
           IE_WB when IE_WB_EN = '1' else
          ( others => '0' );  


  -- WB wrong serve ad evitare di prendere se ho dipendenze, il valore sbagliato calcolato precedentemente e presente nel buffer, per thread 2 e 1 può esserci questa possibilità
  -- mentre per thread 0 non si legge il buffer bensì WB_RD. un bypass tra una istruzione errata e un altra porterebbe il risultato errato a passare all'istruzione nuova, perciò 
  -- se c'è questa possibilità , quando WB_RD_buf_wire (2) / WB_RD_buf_wire (1) bisogna passare il risultato votato e quindi attendere 1 ciclo e poi passarlo.
  WB_wrong <= '1' when ( ( rs1_bypass = '1' or rs2_bypass = '1' ) and ( WB_RD_buf_wire(2) /= WB_RD_buf_wire(1) ) and ( harc_EXEC = 0 or harc_loss = 0 ) and add_vect_bits(WB_EN_buf_wire) > 1 and (add_vect_bits(load_op_buf_wire) < 1 ) and harc_WB /= 0 ) else '0';

--   oppure se il buffer 2 e 1 hanno letto zero ( o comunque sono uguali) e solo il thread 0 legge un valore diverso
  LS_WB_wrong <= '1' when ( rs1_bypass_lat = '1' or rs2_bypass_lat = '1' ) and  ( (WB_RD_buf_wire(2) /= WB_RD_buf_wire(1) and ( harc_EXEC = 0 or harc_loss = 0 )) or ( WB_RD_buf_wire(2) = (0 to 31 => '0') and  WB_RD_buf_wire(1) = (0 to 31 => '0') )) and harc_WB /= 0 and (add_vect_bits(load_op_buf_wire) > 1 ) else '0'; 
--  LS_WB_wrong <= '1' when ( rs1_bypass = '1' or rs2_bypass = '1' ) and ( ( WB_RD_buf_wire(2) /= WB_RD_buf_wire(1) and ( harc_EXEC = 0 or ( harc_loss = 0 and fault_harc = '1' ))  ) or ( WB_RD_buf_wire(2) = (0 to 31 => '0') and  WB_RD_buf_wire(1) = (0 to 31 => '0') )) and harc_WB /= 0 and (add_vect_bits(load_op_buf_wire) > 1 ) else '0'; 

--  LS_WB_wrong_EXEC <= '1' when LS_WB_wrong_lat = '1' and harc_WB = 0 else '0';
  LS_WB_wrong_EXEC_wire <= '1' when LS_WB_wrong_lat = '1' and harc_WB = 0 else '0';


  TTMR_WB_COMB : process(all)  -- wb combinational control
  begin
    WB_EN_buf_wire <= WB_EN_buf; 
    core_busy_WB <= '0';

--    if ls_counter_on = '1' then
--      core_busy_WB <= '1';
--    end if;  -- instr. conditions

--    -- WB_EN_buf_wire serve a collezionare i WB_EN dei diversi thread, e a scrivere nel RF solo quando almeno 2 sono 1, cioè hanno avuto WB_EN.
--    if WB_EN = '1' then
--      if (((harc_EXEC = 1 and fault_harc = '0') or ( harc_loss = 1 and fault_harc = '1' )) and harc_WB = 2) then
--        WB_EN_buf_wire(2) <= '1';          
--      elsif (((harc_EXEC = 1 and fault_harc = '0')  or ( harc_loss = 1 and fault_harc = '1' )) and harc_WB = 0) then
--        WB_EN_buf_wire(2) <= '0';          
--      end if;  
--
--      if (((harc_EXEC = 0 and fault_harc = '0') or ( harc_loss = 0 and fault_harc = '1' )) and harc_WB = 1) then
--        WB_EN_buf_wire(1) <= '1';          
--      elsif (((harc_EXEC = 1 and fault_harc = '0')  or ( harc_loss = 1 and fault_harc = '1' )) and harc_WB = 2) then
--        WB_EN_buf_wire(1) <= '0';          
--      end if;  
--
--      if (((harc_EXEC = 2 and fault_harc = '0')  or ( harc_loss = 2 and fault_harc = '1' )) and harc_WB = 0) or (LS_WB_wrong_lat = '1' and harc_WB = 0) then
--        WB_EN_buf_wire(0) <= '1';          
--      elsif (((harc_EXEC = 1 and fault_harc = '0') or ( harc_loss = 1 and fault_harc = '1' )) and harc_WB = 2) or ((harc_EXEC = 0  or ( harc_loss = 0 and fault_harc = '1' )) and harc_WB = 1) then
--        WB_EN_buf_wire(0) <= '0';          
--      end if;  
--
--    elsif (((harc_EXEC = 1 and fault_harc = '0') or ( harc_loss = 1 and fault_harc = '1' )) and harc_WB = 0 ) or restore_fault_PC = '1' then
--      WB_EN_buf_wire <= (others =>'0');
--    end if;
--


   -- WB_EN_buf_wire serve a collezionare i WB_EN dei diversi thread, e a scrivere nel RF solo quando almeno 2 sono 1, cioè hanno avuto WB_EN.
   if WB_EN = '1' then
     if (harc_EXEC = 1  or harc_loss = 1 )  then
       WB_EN_buf_wire(2) <= '1';          
       WB_EN_buf_wire(1) <= '0';          
       WB_EN_buf_wire(0) <= '0';          
     elsif ( harc_EXEC = 0 or harc_loss = 0 )  then
       WB_EN_buf_wire(1) <= '1';          
       WB_EN_buf_wire(0) <= '0';          
     elsif ( harc_EXEC = 2 or harc_loss = 2 )  then
       WB_EN_buf_wire(0) <= '1';          
     end if;  
     if LS_WB_wrong_lat = '1' and (harc_EXEC = 0  or harc_loss = 0 ) then -- potrei scrivere and harc_EXEC = 0
         WB_EN_buf_wire(0) <= '1';          
     end if;      
   elsif ( (harc_EXEC = 1 or harc_loss = 1 ) and harc_WB = 0) or restore_fault_PC = '1' then
     WB_EN_buf_wire <= (others =>'0');
   end if;


  end process;


  TTMR_WB_SYNCH : process(clk_i, rst_ni)  -- synch single state process
  begin
    if rst_ni = '0' then
      harc_LS_count <=  2;
--      ls_counter_on <= '0';
      WB_ready      <= '0';
      WB_wrong_lat  <= '0';
      LS_WB_wrong_lat  <= '0';
      harc_WB_lat   <=  0;
      WB_EN_buf <= (others => '0');
      rs1_bypass_lat <= '0';
      rs2_bypass_lat <= '0';
      WB_RD_voted <= (others => '0');

      RD_dummy_wire <= 0;
      for h in harc_range loop
        RD_dummy_buf(h) <= 0;
      end loop;

    elsif rising_edge(clk_i) then
      harc_WB_lat  <= harc_WB;
      WB_wrong_lat <= WB_wrong;
      LS_WB_wrong_lat <= LS_WB_wrong;
      WB_RD_voted <= WB_RD_voted_wire;
      WB_ready <= '1' when harc_WB /= 0 else '0';

      rs1_bypass_lat <= rs1_bypass_wire;
      rs2_bypass_lat <= rs2_bypass_wire;

      WB_EN_buf <= WB_EN_buf_wire;

      -- RD_dummy_buf tiene conto dei registri di destinazione delle varie istruzioni decodificate
      if dest_valid = '1' then
        RD_dummy_buf(harc_ID)  <= rd(instr_word_ID_lat); -- is need to take the operand from RF
      else
        RD_dummy_buf(harc_ID)  <= 0; -- is need to take the operand from RF
      end if;              

    -- faccio voting tra il registri che tengono l'indirizzo di dove fare wb durante harc0 solo se almeno 2 su 3 sono uguali
    -- lo faccio in modo asincrono per evitare che durante istruzioni che durano più di 1 ciclo, questi valori vengano cambiati
      if harc_ID = 0  then
        --se sono tutti diversi mantengo il valore precedente
        if ( RD_dummy_buf(2) /= RD_dummy_buf(1) ) and (RD_dummy_buf(2) /= rd(instr_word_ID_lat)) and (RD_dummy_buf(1) /= rd(instr_word_ID_lat) ) then      
        else
          if ( RD_dummy_buf(2) = RD_dummy_buf(1) )  then
            RD_dummy_wire <= RD_dummy_buf(1); 
          elsif ( RD_dummy_buf(2) /= RD_dummy_buf(1) ) then
            RD_dummy_wire <= rd(instr_word_ID_lat); 
          end if;        
        end if;
      end if;

----      if LS_WB_EN = '1' and LS_PE_WB_EN = '1' then
----        ls_counter_on <= '1';
----      end if;  -- instr. conditions    
----      if harc_LS_count > 0 and ( ls_counter_on = '1' or  LS_WB_EN = '1' ) then
----        harc_LS_count <= harc_LS_count -1;
----      else 
----        harc_LS_count <= 2;
----        ls_counter_on <= '0';
----      end if;
----      
    end if;  -- clk
  end process;



--------------------------------------------------------------------- end of WB Stage ----------------
------------------------------------------------------------------------------------------------------


------------------------------------------------------------------------------------------
--  ███████╗██████╗ ███╗   ███╗    ███╗   ███╗ █████╗ ██████╗ ██████╗ ███████╗██████╗   --
--  ██╔════╝██╔══██╗████╗ ████║    ████╗ ████║██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔══██╗  --
--  ███████╗██████╔╝██╔████╔██║    ██╔████╔██║███████║██████╔╝██████╔╝█████╗  ██████╔╝  --
--  ╚════██║██╔═══╝ ██║╚██╔╝██║    ██║╚██╔╝██║██╔══██║██╔═══╝ ██╔═══╝ ██╔══╝  ██╔══██╗  --
--  ███████║██║     ██║ ╚═╝ ██║    ██║ ╚═╝ ██║██║  ██║██║     ██║     ███████╗██║  ██║  --
--  ╚══════╝╚═╝     ╚═╝     ╚═╝    ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝     ╚══════╝╚═╝  ╚═╝  --
------------------------------------------------------------------------------------------                                                           

  spm_mapper : if accl_en = 1 generate 
  RF_FF : if LUTRAM_RF = 0 generate
  Spm_Addr_Mapping : process(all)
  begin
  ID_rs1_to_sc <= std_logic_vector(to_unsigned(SPM_NUM, SPM_ADDR_WID));  -- we assign SPM_NUM to rs1_to_sc as a default case which is out of range (0 to SPM_NUM-1)
  ID_rs2_to_sc <= std_logic_vector(to_unsigned(SPM_NUM, SPM_ADDR_WID));  -- we assign SPM_NUM to rs2_to_sc as a default case which is out of range (0 to SPM_NUM-1)
  ID_rd_to_sc  <= std_logic_vector(to_unsigned(SPM_NUM, SPM_ADDR_WID));  -- we assign SPM_NUM to rd_to_sc  as a default case which is out of range (0 to SPM_NUM-1)
    for i in 0 to SPM_NUM-1 loop -- Decode the address and assign and set the scratchpad number (0 to SPM_NUM-1) to the operand
      if regfile(harc_ID)(rs1(instr_word_ID_lat))(31 downto Addr_Width) >= std_logic_vector(unsigned(SPM_STRT_ADDR(31 downto Addr_Width)) + (i)) and
         regfile(harc_ID)(rs1(instr_word_ID_lat))(31 downto Addr_Width) <  std_logic_vector(unsigned(SPM_STRT_ADDR(31 downto Addr_Width)) + (i+1)) then
        ID_rs1_to_sc <= std_logic_vector(to_unsigned(i, SPM_ADDR_WID));
      end if;
      if regfile(harc_ID)(rs2(instr_word_ID_lat))(31 downto Addr_Width) >= std_logic_vector(unsigned(SPM_STRT_ADDR(31 downto Addr_Width)) + (i)) and
         regfile(harc_ID)(rs2(instr_word_ID_lat))(31 downto Addr_Width) <  std_logic_vector(unsigned(SPM_STRT_ADDR(31 downto Addr_Width)) + (i+1)) then
        ID_rs2_to_sc <= std_logic_vector(to_unsigned(i, SPM_ADDR_WID));
      end if;
      if regfile(harc_ID)(rd(instr_word_ID_lat))(31 downto Addr_Width)  >= std_logic_vector(unsigned(SPM_STRT_ADDR(31 downto Addr_Width)) + (i)) and
         regfile(harc_ID)(rd(instr_word_ID_lat))(31 downto Addr_Width)  <  std_logic_vector(unsigned(SPM_STRT_ADDR(31 downto Addr_Width)) + (i+1)) then
        ID_rd_to_sc <= std_logic_vector(to_unsigned(i, SPM_ADDR_WID));
      end if;
    end loop;
  end process;
  end generate;
  
  RF_LUTRAM : if LUTRAM_RF = 1 generate
  Spm_Addr_Mapping : process(all)
  begin
  ID_rs1_to_sc <= std_logic_vector(to_unsigned(SPM_NUM, SPM_ADDR_WID));  -- we assign SPM_NUM to rs1_to_sc as a default case which is out of range (0 to SPM_NUM-1)
  ID_rs2_to_sc <= std_logic_vector(to_unsigned(SPM_NUM, SPM_ADDR_WID));  -- we assign SPM_NUM to rs2_to_sc as a default case which is out of range (0 to SPM_NUM-1)
  ID_rd_to_sc  <= std_logic_vector(to_unsigned(SPM_NUM, SPM_ADDR_WID));  -- we assign SPM_NUM to rd_to_sc  as a default case which is out of range (0 to SPM_NUM-1)
    for i in 0 to SPM_NUM-1 loop -- Decode the address and assign and set the scratchpad number (0 to SPM_NUM-1) to the operand
      if regfile_lutram_rs1(32*harc_ID+rs1(instr_word_ID_lat))(31 downto Addr_Width) >= std_logic_vector(unsigned(SPM_STRT_ADDR(31 downto Addr_Width)) + (i)) and
         regfile_lutram_rs1(32*harc_ID+rs1(instr_word_ID_lat))(31 downto Addr_Width) <  std_logic_vector(unsigned(SPM_STRT_ADDR(31 downto Addr_Width)) + (i+1)) then
        ID_rs1_to_sc <= std_logic_vector(to_unsigned(i, SPM_ADDR_WID));
      end if;
      if regfile_lutram_rs2(32*harc_ID+rs2(instr_word_ID_lat))(31 downto Addr_Width) >= std_logic_vector(unsigned(SPM_STRT_ADDR(31 downto Addr_Width)) + (i)) and
         regfile_lutram_rs2(32*harc_ID+rs2(instr_word_ID_lat))(31 downto Addr_Width) <  std_logic_vector(unsigned(SPM_STRT_ADDR(31 downto Addr_Width)) + (i+1)) then
        ID_rs2_to_sc <= std_logic_vector(to_unsigned(i, SPM_ADDR_WID));
      end if;
      if regfile_lutram_rd(32*harc_ID+rd(instr_word_ID_lat))(31 downto Addr_Width)  >= std_logic_vector(unsigned(SPM_STRT_ADDR(31 downto Addr_Width)) + (i)) and
         regfile_lutram_rd(32*harc_ID+rd(instr_word_ID_lat))(31 downto Addr_Width)  <  std_logic_vector(unsigned(SPM_STRT_ADDR(31 downto Addr_Width)) + (i+1)) then
        ID_rd_to_sc <= std_logic_vector(to_unsigned(i, SPM_ADDR_WID));
      end if;
    end loop;
  end process;
  end generate;


  Spm_Addr_Mapping_Synch : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then
    elsif rising_edge(clk_i) then
      rs1_to_sc <= ID_rs1_to_sc;
      rs2_to_sc <= ID_rs2_to_sc;
      rd_to_sc  <= ID_rd_to_sc;
    end if;
  end process;
  
  end generate;

---------------------------------------------------------------------- end of ID stage -----------
--------------------------------------------------------------------------------------------------
end RF;
--------------------------------------------------------------------------------------------------
-- END of ID architecture ------------------------------------------------------------------------
-----------