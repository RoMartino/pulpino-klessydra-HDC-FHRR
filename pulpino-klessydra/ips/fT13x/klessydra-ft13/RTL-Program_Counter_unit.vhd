--------------------------------------------------------------------------------------------------------------
--  PC -- (Program Counters and hart interleavers)                                                          --
--  Author(s): Abdallah Cheikh abdallah.cheikh@uniroma1.it (abdallah93.as@gmail.com)                        --
--                                                                                                          --
--  Date Modified: 17-11-2019                                                                               --
--------------------------------------------------------------------------------------------------------------
--  Program Counter Managing Units -- synchronous process, sinle cycle.                                     --
--  Note: in the present version, gives priority to branching over trapping, except LSU and DSP traps       -- 
--  i.e. branch instructions are not interruptible. This can be changed but may be unsafe.                  --
--  Implements as many PC units as the  number of harts supported                                           --
--  This entity also implements the hardware context counters that interleve the harts in the core.         --
--------------------------------------------------------------------------------------------------------------


-- ieee packages ------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;
use std.textio.all;

-- local packages ------------
use work.riscv_klessydra.all;

entity Program_Counter is
  generic (
    THREAD_POOL_SIZE      : integer;
    ACCL_NUM              : natural
  );
  port (
    absolute_jump                     : in  std_logic;
    data_we_o_lat                     : in  std_logic;
    PC_offset                         : in  array_2D(THREAD_POOL_SIZE - 1 downto 0)(31 downto 0);
    taken_branch                      : in  std_logic;
    ie_taken_branch                   : in  std_logic;
    ls_taken_branch                   : in  std_logic;
    dsp_taken_branch                  : in  std_logic_vector(ACCL_NUM - 1 downto 0);
    set_branch_condition              : in  std_logic;
    ie_except_condition               : in  std_logic;
    ls_except_condition               : in  std_logic;
    dsp_except_condition              : in  std_logic_vector(ACCL_NUM - 1 downto 0);
    set_except_condition              : in  std_logic;
    set_mret_condition                : in  std_logic;
    set_wfi_condition                 : in  std_logic;

    --TTMR signals
--  fault_LSU                         : in  std_logic;
    WB_wrong_lat                      : in std_logic;
    LS_WB_wrong_lat                   : in std_logic; 
    fault_PC                          : out std_logic;
    fault_PC_lat                      : out std_logic;
    harc_fault_wire                        : out std_logic_vector(THREAD_POOL_SIZE-1 downto 0);
    harc_fault                        : out std_logic_vector(THREAD_POOL_SIZE-1 downto 0);
    harc_fault_wire_lat                    : out std_logic_vector(1 downto 0);
    harc_loss                         : out integer range THREAD_POOL_SIZE - 1 downto 0;
    harc_loss_lat_wire                : out integer range THREAD_POOL_SIZE - 1 downto 0;
    harc_fault_previous               : out std_logic;

    fault_harc                        : out std_logic;
    fault_harc_lat                    : out std_logic;
    restore_fault_PC                  : out std_logic;
--    fault_LSU_lat                     : out std_logic;
    pc_voted_wire                          : out std_logic_vector(31 downto 0); 

--  restore_fault_LSU                 : in  std_logic;
    harc_ID                           : in  integer range THREAD_POOL_SIZE - 1 downto 0;
    harc_EXEC                         : in  integer range THREAD_POOL_SIZE - 1 downto 0;
    instr_rvalid_IE                   : in  std_logic;
    pc_ID                             : in  std_logic_vector(31 downto 0);
    pc_IE                             : in  std_logic_vector(31 downto 0);
    MSTATUS                           : in  array_2d(THREAD_POOL_SIZE - 1 downto 0)(1 downto 0);
    MIP, MEPC, MCAUSE, MTVEC          : in  array_2D(THREAD_POOL_SIZE - 1 downto 0)(31 downto 0);
    instr_word_IE                     : in  std_logic_vector(31 downto 0);
    reset_state                       : in  std_logic;
    pc_IF                             : out std_logic_vector(31 downto 0);
    harc_IF                           : out integer range THREAD_POOL_SIZE - 1 downto 0;
    served_ie_except_condition        : out std_logic_vector(THREAD_POOL_SIZE - 1 downto 0);
    served_ls_except_condition        : out std_logic_vector(THREAD_POOL_SIZE - 1 downto 0);
    served_dsp_except_condition       : out std_logic_vector(THREAD_POOL_SIZE - 1 downto 0);
    served_except_condition           : out std_logic_vector(THREAD_POOL_SIZE - 1 downto 0);
    served_mret_condition             : out std_logic_vector(THREAD_POOL_SIZE - 1 downto 0);
    served_irq                        : in  std_logic_vector(THREAD_POOL_SIZE - 1 downto 0);
    taken_branch_pending              : out std_logic_vector(THREAD_POOL_SIZE - 1 downto 0);
    taken_branch_pc_lat               : out array_2D(THREAD_POOL_SIZE - 1 downto 0)(31 downto 0);
    incremented_pc                    : out array_2D(THREAD_POOL_SIZE - 1 downto 0)(31 downto 0);
    mepc_incremented_pc               : out array_2D(THREAD_POOL_SIZE - 1 downto 0)(31 downto 0);
    mepc_interrupt_pc                 : out array_2D(THREAD_POOL_SIZE - 1 downto 0)(31 downto 0);
    irq_pending                       : out std_logic_vector(THREAD_POOL_SIZE - 1 downto 0);
    clk_i                             : in  std_logic;
    rst_ni                            : in  std_logic;
    irq_i                             : in  std_logic;
    fetch_enable_i                    : in  std_logic;
    boot_addr_i                       : in  std_logic_vector(31 downto 0);
    instr_gnt_i                       : in  std_logic
    );
end entity;


architecture PC of Program_counter is

  subtype harc_range is integer range THREAD_POOL_SIZE - 1 downto 0;
  subtype accl_range is integer range ACCL_NUM - 1 downto 0;

  -- signals for TTMR voting
--  signal fault_PC      : std_logic;
--  signal fault_PC_lat  : std_logic;
--  signal harc_fault    : std_logic_vector(harc_range);
--  signal harc_fault_wire_lat    : std_logic_vector(1 downto 0);
--  signal harc_loss     : integer ;
--  signal harc_loss_lat     : integer ;
--  signal fault_harc    : std_logic;
--  signal fault_harc_lat  : std_logic;
--  signal restore_fault_PC : std_logic;
--  signal fault_LSU_lat : std_logic;
  signal pc_voted          : std_logic_vector(31 downto 0); 
  signal harc_loss_lat     : integer range THREAD_POOL_SIZE - 1 downto 0;
  signal no_exec_wire      : std_logic;
  signal no_exec           : std_logic;

--  signal fault_PC_wire     : std_logic;
--  signal prova : std_logic;

--  signal incremented_pc_voted_wire : std_logic_vector (31 downto 0);
  signal restore_fault_wire        : std_logic_vector(2 downto 0);
  signal restore_fault             : std_logic_vector(2 downto 0);

  -- pc updater signals
  signal pc_update_enable                  : std_logic_vector(harc_range);
  signal taken_branch_replicated           : std_logic_vector(harc_range);
  signal set_branch_condition_replicated   : std_logic_vector(harc_range);
  signal set_wfi_condition_replicated      : std_logic_vector(harc_range);
  signal ls_except_condition_replicated    : std_logic_vector(harc_range);
  signal ie_except_condition_replicated    : std_logic_vector(harc_range);
  signal dsp_except_condition_replicated   : std_logic_vector(harc_range);
  signal set_except_condition_replicated   : std_logic_vector(harc_range);
  signal set_mret_condition_replicated     : std_logic_vector(harc_range);
  signal relative_to_PC                    : array_2D(harc_range)(31 downto 0);
--  signal pc                                : array_3D(harc_range)(harc_range)(31 downto 0);
  signal pc                                   : array_2D(harc_range)(31 downto 0);
  signal pc_wire                              :  array_2D(harc_range)(31 downto 0);
  signal boot_pc                              : std_logic_vector(31 downto 0);
  signal harc_IF_internal                     : harc_range;
  signal mret_condition_pending_internal      : std_logic_vector(harc_range);
  signal mepc_incremented_pc_internal         : array_2D(harc_range)(31 downto 0);
  signal incremented_pc_internal              : array_2D(harc_range)(31 downto 0);
  signal mepc_interrupt_pc_internal           : array_2D(harc_range)(31 downto 0);
  signal taken_branch_pc_lat_internal         : array_2D(harc_range)(31 downto 0);
  signal taken_branch_pc_pending_internal     : array_2D(harc_range)(31 downto 0);
  signal taken_branch_pending_internal        : std_logic_vector(harc_range);
  signal irq_pending_internal                 : std_logic_vector(harc_range);

  signal taken_branch_pc_pending_internal_lat : array_2D(harc_range)(31 downto 0);
  signal taken_branch_pending_internal_lat    : std_logic_vector(harc_range);
  signal served_ie_except_condition_lat       : std_logic_vector(harc_range);
  signal served_ls_except_condition_lat       : std_logic_vector(harc_range);
  signal served_dsp_except_condition_lat      : std_logic_vector(harc_range);
  signal served_except_condition_lat          : std_logic_vector(harc_range);
  signal served_mret_condition_lat            : std_logic_vector(harc_range);

  -- This function return the harc lost , caused by fault if the harc range is 0 to THREAD_POOL_SIZE -1 else
  -- it returns the THREAD_POOL_SIZE 
  function harc_lost(IF_harc, ID_harc, IE_harc : integer) return integer is
    type array_integer is array (0 to THREAD_POOL_SIZE-1) of integer range 0 to THREAD_POOL_SIZE; 
    variable h_count : array_integer;
  begin
    for i in h_count'range loop
      h_count(i) := 0;
    end loop;  
    for i in h_count'range loop
      if IF_harc = i then 
        h_count(i) := h_count(i) +1;
      end if;
      if ID_harc = i then 
        h_count(i) := h_count(i) +1;
      end if;
      if IE_harc = i then 
        h_count(i) := h_count(i) +1;
      end if;
    end loop;

    for i in h_count'range loop
      if h_count(i) = 0 then
        return i;
      end if;
    end loop;  
    return IE_harc;       
  end function harc_lost;
  ------------------------------------------------------------------------------------------------------------
  -- Subroutine implementing pc updating combinational logic, that is replicated for the threads supported  --
  ------------------------------------------------------------------------------------------------------------
  procedure pc_update(
    constant h                           : in  integer range THREAD_POOL_SIZE-1 downto 0;
    signal pc_voted_wire                 : in    std_logic_vector(31 downto 0); 
    signal harc_fault_wire               : in std_logic_vector(2 downto 0);
    signal fetch_enable_i                : in    std_logic;
    signal MTVEC                         : in    std_logic_vector(31 downto 0);
    signal instr_gnt_i, taken_branch     : in    std_logic;
    signal set_wfi_condition             : in    std_logic;
    signal taken_branch_pending          : inout std_logic;
    signal taken_branch_pending_lat      : in    std_logic; --
    signal irq_pending                   : in    std_logic;
    signal ie_except_condition           : in    std_logic;
    signal ls_except_condition           : in    std_logic;
    signal dsp_except_condition          : in    std_logic;
    signal set_except_condition          : in    std_logic;
    signal set_mret_condition            : in    std_logic;
    signal pc                            : inout std_logic_vector(31 downto 0);
    signal taken_branch_pc_lat           : in    std_logic_vector(31 downto 0);
    signal taken_branch_pc_pending       : inout std_logic_vector(31 downto 0);
    signal taken_branch_pc_pending_lat   : in    std_logic_vector(31 downto 0); --
    signal incremented_pc                : in    std_logic_vector(31 downto 0);
    signal boot_pc                       : in    std_logic_vector(31 downto 0);
    signal pc_update_enable              : in    std_logic;
    signal served_ie_except_condition    : out   std_logic;
    signal served_ls_except_condition    : out   std_logic;
    signal served_dsp_except_condition   : out   std_logic;
    signal served_except_condition       : out   std_logic;
    signal served_mret_condition         : out   std_logic) is

  begin
--      if harc_fault = "000" then
--        pc <= pc;
--      elsif harc_fault(0) = '1' and (h /= 0) then
--        pc <= (std_logic_vector(unsigned(pc_voted_wire)+4));
--      elsif harc_fault(1) = '1' and (h = 2) then 
--        pc <= (std_logic_vector(unsigned(pc_voted_wire)+4));
--      elsif harc_fault(2) = '1'  then
--        pc <= pc_voted_wire;
--      else
--        pc <= pc_voted_wire;    
--      end if;  
      if harc_fault_wire = "000" then
        pc <= pc;
      elsif harc_fault_wire(0) = '1' and (h /= 0) then
        pc <= (std_logic_vector(unsigned(pc_voted_wire)+4));
      elsif harc_fault_wire(1) = '1' and (h = 2) then 
        pc <= (std_logic_vector(unsigned(pc_voted_wire)+4));
      elsif harc_fault_wire(2) = '1'  then
        pc <= pc_voted_wire;
      else
        pc <= pc_voted_wire;    
      end if;        
    if pc_update_enable = '1' then

--      pc(h) <= (std_logic_vector(unsigned(pc_voted_wire)+4)) when harc_fault_wire(0) = '1' and (h /= 0)  else --fault su harc 0 vuol dire che 1 e 2 hanno già eseguito pc_voted_wire e vogliono pc_voted_wire +4
--               (std_logic_vector(unsigned(pc_voted_wire)+4)) when harc_fault_wire(1) = '1' and (h = 2)   else --fault su harc 1 vuol dire che 2 ha già eseguito pc_voted_wire e vuole pc_voted_wire +4
--               pc_voted_wire                                 when harc_fault_wire(2) = '1' else --fault su harc 2 vuol dire che nessuno ha eseguito pc_voted_wire e tutti vogliono pc_voted_wire 
--               pc_voted_wire ;


      -- interrupt service launched in the previous instr. cycle
      -- this is done for a second instr. cycle for proper synchronization of flushing
      -- nothing pending    
      if not taken_branch = '1' and not taken_branch_pending = '1' then
        pc                      <= incremented_pc;
        served_except_condition <= '0';
        served_ie_except_condition  <= '0';
        served_ls_except_condition  <= '0';
        served_dsp_except_condition <= '0';
        served_mret_condition   <= '0';
      -- taken_branch pending 
      elsif taken_branch = '1' then
        pc                      <= taken_branch_pc_lat;
        taken_branch_pending    <= '0';
        served_ie_except_condition  <= '1' when ie_except_condition  = '1' else '0'; -- for CS units;
        served_ls_except_condition  <= '1' when ls_except_condition  = '1' else '0'; -- for CS units;
        served_dsp_except_condition <= '1' when dsp_except_condition = '1' else '0'; -- for CS units;
        served_except_condition     <= '1' when set_except_condition = '1' else '0'; -- for CS units;
        served_mret_condition       <= '1' when set_mret_condition   = '1' else '0'; -- for CS units;
      elsif  taken_branch_pending = '1' then
        pc                      <= taken_branch_pc_pending;
        taken_branch_pending    <= '0';
        served_ie_except_condition  <= '1' when ie_except_condition  = '1' else '0'; -- for CS units;
        served_ls_except_condition  <= '1' when ls_except_condition  = '1' else '0'; -- for CS units;
        served_dsp_except_condition <= '1' when dsp_except_condition = '1' else '0'; -- for CS units;
        served_except_condition     <= '1' when set_except_condition = '1' else '0'; -- for CS units;
        served_mret_condition       <= '1' when set_mret_condition   = '1' else '0'; -- for CS units;
      else
        pc <= boot_pc;                  -- default, should never occur
      end if;
      -- end of pc value update ---    
    else                                -- sets registers to record pending requests
      served_except_condition <= '0';
      served_mret_condition   <= '0';
      if taken_branch = '1' then
        taken_branch_pending <= '1';
        taken_branch_pc_pending <= taken_branch_pc_lat;
      end if;
      if set_except_condition = '1' then
        served_except_condition <= '1';
      end if;
      if dsp_except_condition = '1' then
        served_dsp_except_condition <= '1';
      elsif ls_except_condition = '1' then
        served_ls_except_condition <= '1';
      elsif ie_except_condition = '1' then
        served_ie_except_condition <= '1';
      end if;
      if set_mret_condition = '1'  and fetch_enable_i = '1' then
        served_mret_condition <= '1';
      end if;
    end if;
  end pc_update;
  --------------------------------------------------------------------------------------

begin

  harc_IF                  <= harc_IF_internal;
  mepc_incremented_pc      <= mepc_incremented_pc_internal;
  mepc_interrupt_pc        <= mepc_interrupt_pc_internal;
  taken_branch_pc_lat      <= taken_branch_pc_lat_internal;
  incremented_pc           <= incremented_pc_internal;
  taken_branch_pending     <= taken_branch_pending_internal;
  irq_pending              <= irq_pending_internal;


  hardware_context_counter : process(all)
  begin
    if rst_ni = '0' then
      harc_IF_internal <= THREAD_POOL_SIZE -1;

    elsif rising_edge(clk_i) then
      if instr_gnt_i = '1' and fault_PC = '0'  then
        harc_IF_internal <= harc_IF_internal - 1 when harc_IF_internal > 0 else THREAD_POOL_SIZE -1;
      elsif harc_fault_wire(2) = '1'  then
        harc_IF_internal <= THREAD_POOL_SIZE -1;
      elsif harc_fault_wire(1) = '1' then
        harc_IF_internal <= 1;
      end if;
    end if;
  end process hardware_context_counter;

  -- this is the multiplexer on the PC_IF
  -- prelevo il wire risultato del voting
  pc_IF <= pc(harc_IF_internal);

  -- fixed connections, not replicated 
  boot_pc  <= boot_addr_i(31 downto 8) & std_logic_vector(to_unsigned(128, 8));
  ----------------------------------------------------------------------------------------------
  -- this part of logic and registers is replicated as many times as the supported threads:   --
  pc_update_logic : for h in harc_range generate

    mepc_incremented_pc_internal(h) <= MEPC(h);
    mepc_interrupt_pc_internal(h)   <= MEPC(h) when MCAUSE(h)(30) = '0' else std_logic_vector(unsigned(MEPC(h)) + 4);  -- MCAUSE(30) = '0' indicates that we weren't executing a WFI instruction

    relative_to_PC(h) <= std_logic_vector(to_unsigned(0, 32)) when (absolute_jump = '1')
                         else pc_IE when set_branch_condition = '1' or set_wfi_condition = '1' else pc_ID;

--    incremented_pc_internal(h) <= std_logic_vector(unsigned(pc(0)(h))+4);    
    incremented_pc_internal(h) <= std_logic_vector(unsigned(pc(h))+4); 

  -- prelevo il risultato del voting e lo incremento non per harc 0 che dovrà essere ancora incrementato
  --  incremented_pc_voted_wire <= (std_logic_vector(unsigned(pc_voted_wire)+4)) when h /= 0 else pc_voted_wire_wire ;

    irq_pending_internal(h)    <= ((MIP(h)(11) or MIP(h)(7) or MIP(h)(3)) and MSTATUS(h)(0));

    set_wfi_condition_replicated(h) <= '1' when set_wfi_condition = '1' and (harc_EXEC = h)
                                  else '0';
    taken_branch_replicated(h) <=      '1' when dsp_taken_branch /= (accl_range => '0') and (harc_EXEC = h)
	                              else '1' when ls_taken_branch  = '1' and (harc_EXEC = h)
	                              else '1' when ie_taken_branch  = '1' and ((harc_EXEC = h) or served_irq(THREAD_POOL_SIZE -1 ) = '1' )  --served the irq routine when only harc(THREAD_POOL_SIZE - 1) is the one that 
                                  else '0';                                                                                            --updates the irq routine address
    set_branch_condition_replicated(h) <= '1' when set_branch_condition = '1' and (harc_EXEC = h)
                                     else '0';
    dsp_except_condition_replicated(h) <= '1' when dsp_except_condition  /= (accl_range => '0') and (harc_EXEC  = h)
                                     else '0';
    ls_except_condition_replicated(h)  <= '1' when ls_except_condition   = '1' and (harc_EXEC   = h)
                                     else '0';
    ie_except_condition_replicated(h)  <= '1' when ie_except_condition  = '1' and (harc_EXEC = h)
                                     else '0';
    set_except_condition_replicated(h) <= '1' when dsp_except_condition_replicated(h)  = '1' or ls_except_condition_replicated(h) = '1' or ie_except_condition_replicated(h) = '1'
                                     else '0';
    set_mret_condition_replicated(h)   <= '1' when set_mret_condition = '1' and (harc_EXEC = h)
                                     else '0';

    -- latch on the branch address, possibly useless but may be needed in future situations, served_irq has the highest priority, interrupt request are checked before executing any instructions in the IE_Stage

    taken_branch_pc_lat_internal(h) <=
      MTVEC(h)                                                         when dsp_except_condition_replicated(h) = '1'                         else  -- sets MTVEC address for exception trap
      MTVEC(h)                                                         when ls_except_condition_replicated(h)  = '1'                         else  -- sets MTVEC address for exception trap
      std_logic_vector(signed(relative_to_PC(h))+signed(PC_offset(h))) when set_branch_condition_replicated(h) = '1'                         else  -- sets a jump or a branch address
      std_logic_vector(signed(relative_to_PC(h)))                      when set_wfi_condition_replicated(h)    = '1'                         else  -- sets a wfi address (spin lock)
      MTVEC(h)                                                         when ie_except_condition_replicated(h)  = '1'                         else  -- sets MTVEC address for exception trap
      mepc_incremented_pc_internal(h)                                  when set_mret_condition_replicated(h)   = '1' and MCAUSE(h)(31) = '0' else  -- sets return address from exception subroutine
      mepc_interrupt_pc_internal(h)                                    when set_mret_condition_replicated(h)   = '1' and MCAUSE(h)(31) = '1' else  -- sets return address from interrupt subroutine
      MTVEC(h)                                                         when served_irq(THREAD_POOL_SIZE -1 )   = '1'                         else  -- sets MTVEC address for exception trap, when the served harc is 2 
     (others => '0');


    pc_update_enable(h) <= '1' when instr_gnt_i = '1'
                           and (harc_IF_internal = h
                                or taken_branch_replicated(h)       = '1'
                                or set_wfi_condition_replicated(h)  = '1'
                                or taken_branch_pending_internal(h) = '1'
                                or served_irq(THREAD_POOL_SIZE -1 ) = '1' ) -- update the pc only if the serving harc is 2
                           else '0';

    pc_update_sync : process (clk_i, rst_ni)
    begin
      if rst_ni = '0' then 
        pc(h)    <= (31 downto 8 => '0' ) & std_logic_vector( to_unsigned(128,8));
        taken_branch_pc_pending_internal_lat(h)  <= (others => '0');
        taken_branch_pending_internal_lat(h)     <= '0';
        taken_branch_pending_internal(h)         <= '0';
        served_ie_except_condition_lat(h)        <= '0';
        served_ls_except_condition_lat(h)        <= '0';
        served_dsp_except_condition_lat(h)       <= '0';
        served_except_condition_lat(h)           <= '0';
        served_mret_condition_lat(h)             <= '0';

      elsif rising_edge(clk_i) then
        if reset_state = '1' then
          pc_wire(h) <= boot_pc;
        else
          pc_update(
            h,
            pc_voted_wire,
            harc_fault_wire,
            fetch_enable_i,
            MTVEC(h),
            instr_gnt_i,
            taken_branch_replicated(h),
--            set_branch_condition_ID_replicated(h),
            set_wfi_condition_replicated(h),
            taken_branch_pending_internal(h), 
            taken_branch_pending_internal_lat(h),
            irq_pending_internal(h),
            ie_except_condition_replicated(h),
            ls_except_condition_replicated(h), 
            dsp_except_condition_replicated(h),
            set_except_condition_replicated(h), 
            set_mret_condition_replicated(h), 
            pc(h), 
            taken_branch_pc_lat_internal(h), 
            taken_branch_pc_pending_internal(h),
            taken_branch_pc_pending_internal_lat(h), 
            incremented_pc_internal(h), 
            boot_pc, 
            pc_update_enable(h), 
            served_ie_except_condition(h), 
            served_ls_except_condition(h),
            served_dsp_except_condition(h), 
            served_except_condition(h), 
            served_mret_condition(h)
          );


        end if;
         end if;
    end process;
  end generate pc_update_logic;
  -- end of replicated logic --   



  TTMR_PC_SYNC :  process(clk_i, rst_ni) -- the sync process for ttmr
  begin
    if rst_ni = '0' then 
      restore_fault   <= (others => '0');
      pc_voted        <= (others => '0');
      harc_fault      <= (others => '0');
      harc_loss_lat   <= 0;
      fault_PC_lat <= '0';
      fault_harc_lat <= '0';
    elsif rising_edge(clk_i) then
--      harc_loss_lat <= harc_loss_lat_wire;
      harc_fault <= harc_fault_wire;
      fault_harc_lat <= fault_harc;
      pc_voted <= pc_voted_wire;


      no_exec <= no_exec_wire;
      fault_PC_lat <= fault_PC;
      harc_fault_wire_lat(1) <= fault_PC;
      harc_fault_wire_lat(0) <= harc_fault_wire_lat(1);
      restore_fault <= restore_fault_wire;

    end if;

  end process;

  pc_voted_wire <= (( pc(harc_IF_internal) and pc_IE ) or ( pc(harc_IF_internal) and pc_ID ) or ( pc_ID and pc_IE ) );

  TTMR_PC_COMB : process(all)  -- the combinational voting for TTMR
  begin
  fault_PC <= '0';
  no_exec_wire <= '0';

--  pc_voted_wire <= pc_voted;
  restore_fault_wire <= restore_fault;
--  harc_loss_lat_wire <= harc_loss_lat;

  harc_fault_wire(0) <= '0';
  harc_fault_wire(1) <= harc_fault(1);
  harc_fault_wire(2) <= harc_fault(2);
-------------------------------------------------------------

--  --logica per fault_harc un segnale che controlla la corretta gestione degli harc
--  if restore_fault_PC = '1' or WB_wrong_lat = '1' then
--    fault_harc <= '0';
--  else
--   fault_harc <= '0' when  ((harc_EXEC = 2 and harc_ID = 1 and harc_IF_internal = 0) or
--                            (harc_EXEC = 1 and harc_ID = 0 and harc_IF_internal = 2) or
--                            (harc_EXEC = 0 and harc_ID = 2 and harc_IF_internal = 1)) else '1';
--  end if;


----------------------------------------------------

  -- logica per harc_fault, il segnale che controlla se ci sono stati fault su pc_IF pc_ID pc_IE ed avvia le routine di recupero da fault

  if harc_IF_internal = 0 then
      harc_fault_wire(1) <= '0';


      if ( pc(harc_IF_internal) /= pc_ID ) then 
        if pc_ID = pc_IE then --the fault is on PC_IF thread 0
          fault_PC <= '1';
          harc_fault_wire(0) <= '1';
        elsif ( pc(harc_IF_internal) = pc_IE ) then  --the fault is on PC_ID thread 1
          fault_PC <= '1';
          harc_fault_wire(1) <= '1';
        end if;
      elsif (( pc_IE /= pc(harc_IF_internal) ) and ( pc(harc_IF_internal) = pc_ID ))  then --the fault is on PC_IE thread 2
        fault_PC <= '1';
        harc_fault_wire(2) <= '1';
      end if;

  elsif harc_IF_internal = 1 then
    harc_fault_wire(2) <= '0';     
    if ( pc(harc_IF_internal) /= pc_ID ) and (harc_fault = "000") then --se non ci sono già attivi altri fault
      no_exec_wire <= '1';
    end if;  
  end if;          

  harc_fault_previous <= '1' when no_exec = '1' and no_exec_wire = '0' else '0';

--  if harc_IF_internal = 0 then
--    harc_fault_wire(1) <= '0';
--  elsif harc_IF_internal = 1 then
--    harc_fault_wire(2) <= '0';
--  end if;    
--  if harc_IF_internal = 0 then
--      --TMR logic
--      pc_voted_wire <= (( pc(harc_IF_internal) and pc_IE ) or ( pc(harc_IF_internal) and pc_ID ) or ( pc_ID and pc_IE ) );
--
--      if (( pc(harc_IF_internal) /= pc_ID ) and ( pc_ID = pc_IE )) then --the fault is on PC_IF thread 0
--        fault_PC <= '1';
--        harc_fault_wire(0) <= '1';
--
--      elsif (( pc_ID /= pc(harc_IF_internal) ) and ( pc(harc_IF_internal) = pc_IE ))  then --the fault is on PC_ID thread 1
--        fault_PC <= '1';
--        harc_fault_wire(1) <= '1';
--
--      elsif (( pc_IE /= pc(harc_IF_internal) ) and ( pc(harc_IF_internal) = pc_ID ))  then --the fault is on PC_IE thread 2
--        fault_PC <= '1';
--        harc_fault_wire(2) <= '1';
--      end if;
--  end if;

-----------------------------------------------------


--se ho un fault su PC alzo il segnale di controllo che mi indica che è attiva la procedura di restore dal fault e che dura in base al harc faultato
  if ( harc_fault(2) = '1' ) then
    restore_fault_wire(2) <= '1';
  else
    if harc_ID = 1 then
      restore_fault_wire(2) <= '0';
    end if;        
  end if;        

  if ( harc_fault(1) = '1' ) then
    restore_fault_wire(1) <= '1';
  else
    if harc_ID = 0 then
      restore_fault_wire(1) <= '0';
    end if;        
  end if;        

  if ( harc_fault(0) = '1' ) then
    restore_fault_wire(0) <= '1';
  else
    if harc_ID = 2 then
      restore_fault_wire(0) <= '0';
    end if;        
  end if;        

------------------------------------------------------------


-- if ( fault_harc_lat = '1' and fault_harc = '1') then
--   harc_loss_lat_wire <= harc_loss;
-- end if;

  end process;


-- controllo per gli harc, specialmente harc exec
--  harc_loss <= harc_lost(harc_IF_internal, harc_ID, harc_EXEC) when fault_harc = '1' else 0;
--  harc_loss <= harc_lost(harc_IF_internal, harc_ID, harc_EXEC) when restore_fault_PC = '0' and WB_wrong_lat = '0' else harc_EXEC;
  harc_loss <= harc_EXEC;
--  harc_loss <= harc_lost(harc_IF_internal, harc_ID, harc_EXEC) when  WB_wrong_lat = '0' else harc_EXEC;

-- logica per fault_harc un segnale che controlla la corretta gestione degli harc solo quando non sto facedo restore
--  fault_harc <= '1' when harc_lost(harc_IF_internal, harc_ID, harc_EXEC) /= harc_EXEC and restore_fault_PC = '0' and WB_wrong_lat = '0' else '0';
  fault_harc <= '1';
--  fault_harc <= '1' when harc_lost(harc_IF_internal, harc_ID, harc_EXEC) /= harc_EXEC and WB_wrong_lat = '0' else '0';


-- controllo di avvio della procedura di restore fault degli harc  
  restore_fault_PC <= restore_fault_wire(2) or restore_fault_wire(1) or restore_fault_wire(0);

--------------------------------------------------------------------- end of PC Managing Units ---
--------------------------------------------------------------------------------------------------  
 
end PC;
--------------------------------------------------------------------------------------------------
-- END of Program Counter architecture -----------------------------------------------------------
--------------------------------------------------------------------------------------------------
