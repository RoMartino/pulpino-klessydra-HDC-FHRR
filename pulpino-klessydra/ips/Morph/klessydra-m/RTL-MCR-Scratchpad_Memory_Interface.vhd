-----------------------------------------------------------------------------------------------------------------
--  SPI -- (Scratchpad Memory Interface)                                                                       --
--  Author(s): Abdallah Cheikh abdallah.cheikh@uniroma1.it (abdallah93.as@gmail.com)                           --
--                                                                                                             --
--  Date Modified: 02-04-2019                                                                                  --
-----------------------------------------------------------------------------------------------------------------
--  The SPI connects the SPM to the LSU and DSP units. The LSU reads and wirtes one word (32-bits) at a time   --
--  and a bank interleaver switches between the internal banks of the SPM. The DSP reads and writes one SPM    -- 
--  line which is "32*SIMD" bits wide. Hence there is no need for the bank interleaver. However, the DSP       --
--  can perform misaligned reads and writes. So it needs read and write data rotators to allign the source     --
--  operands to the appropriate functional units, and memory banks. The SPI provides 2 rd and 1 wr port to     --
--  the DSP with a two cycle read latency (one for reading the banks, the other is for rotating)               --
-----------------------------------------------------------------------------------------------------------------

-- ieee packages ------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;
use std.textio.all;

-- local packages ------------
use work.riscv_klessydra.all;
--use work.klessydra_parameters.all;

-- SCI pinout --------------------
entity Scratchpad_memory_interface_MCR is
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
    clk_i, rst_ni                  : in  std_logic;
    data_rvalid_i                  : in  std_logic;
    state_LS                       : in  fsm_LS_states;
    sc_word_count_wire             : in  integer;
    sc_acc_word_count_wire_MCR     : in  integer;
    spm_bcast                      : in  std_logic;
    harc_LS_wire                   : in  integer range ACCL_NUM-1 downto 0;
    hdc_we_word_MCR                : in  array_2d(ACCL_NUM-1 downto 0)(SIMD-1 downto 0);
    dsp_acc_we_word_MCR            : in  array_2d(ACCL_NUM-1 downto 0)((MCR_Acc_SIMD_Width/32)-1 downto 0);
    ls_sc_data_write_wire          : in  std_logic_vector(Data_Width-1 downto 0);
    hdc_sc_data_write_wire_MCR     : in  array_2d(ACCL_NUM-1 downto 0)(SIMD_Width-1 downto 0);
    dsp_sc_acc_data_write_wire_MCR : in  array_2d(ACCL_NUM-1 downto 0)(MCR_Acc_SIMD_Width-1 downto 0);
    ls_sc_read_addr                : in  std_logic_vector(Addr_Width-(SIMD_BITS+3) downto 0);
    ls_sc_acc_read_addr_MCR        : in  std_logic_vector(Addr_Width-(ACC_SIMD_BITS+3) downto 0);
    ls_sc_write_addr               : in  std_logic_vector(Addr_Width-(SIMD_BITS+3) downto 0);
    ls_sc_acc_write_addr_MCR       : in  std_logic_vector(Addr_Width-(ACC_SIMD_BITS+3) downto 0);
    hdc_sc_write_addr_MCR          : in  array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
    ls_sci_req                     : in  std_logic_vector(SPM_NUM-1 downto 0);
    ls_sci_we                      : in  std_logic_vector(SPM_NUM-1 downto 0);
    hdc_sci_req_MCR                : in  array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    hdc_sci_we_MCR                 : in  array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    kmemld_inflight                : in  std_logic_vector(SPM_NUM-1 downto 0);
    kmemstr_inflight               : in  std_logic_vector(SPM_NUM-1 downto 0);
    hdc_to_sc_MCR                  : in  array_3d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0)(1 downto 0);
    hdc_sc_read_addr_MCR           : in  array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(Addr_Width-1 downto 0);
    dsp_sc_data_read               : out array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(SIMD_Width-1 downto 0);
    dsp_sc_acc_data_read           : out array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(MCR_Acc_SIMD_Width-1 downto 0);
    ls_sc_data_read_wire           : out std_logic_vector(Data_Width-1 downto 0);
    ls_sci_wr_gnt                  : out std_logic;
    dsp_sci_wr_gnt                 : out std_logic_vector(ACCL_NUM-1 downto 0);
    ls_data_gnt_i                  : out std_logic_vector(SPM_NUM-1 downto 0);
    dsp_data_gnt_i                 : out std_logic_vector(ACCL_NUM-1 downto 0)
	);
end entity;  ------------------------------------------


architecture SCI_MCR of Scratchpad_memory_interface_MCR is

subtype accl_range is integer range ACCL_NUM - 1 downto 0; 

signal dsp_sc_data_write_int_wire      : array_2d(accl_range)(SIMD_Width-1 downto 0);
signal dsp_sc_acc_data_write_int_wire  : array_2d(accl_range)(MCR_Acc_SIMD_Width-1 downto 0);

signal ls_sc_data_read_int_wire        : array_2d(accl_range)(Data_Width-1 downto 0);
signal ls_data_gnt_internal            : array_2d(accl_range)(SPM_NUM-1 downto 0);
signal rd_offset                       : array_3d(accl_range)(1 downto 0)(SIMD-1 downto 0);
signal acc_rd_offset                   : array_3d(accl_range)(1 downto 0)(MCR_Acc_SIMD_Width/32-1 downto 0);
signal wr_offset                       : array_2d(accl_range)(SIMD-1 downto 0);
signal acc_wr_offset                   : array_2d(accl_range)(MCR_Acc_SIMD_Width/32-1 downto 0);
signal dsp_sc_data_read_int_wire       : array_3d(accl_range)(1 downto 0)(SIMD_Width-1 downto 0);
signal dsp_sc_acc_data_read_int_wire   : array_3d(accl_range)(1 downto 0)(MCR_Acc_SIMD_Width-1 downto 0);
signal hdc_sc_read_addr_lat            : array_3d(accl_range)(1 downto 0)(SIMD_BITS+1 downto 0);  --  Only need the lower part to check for the word access
signal dsp_sc_acc_read_addr_lat        : array_3d(accl_range)(1 downto 0)(ACC_SIMD_BITS+1 downto 0);  --  Only need the lower part to check for the word access
signal hdc_sci_req_lat                 : array_2d(accl_range)(SPM_NUM-1 downto 0);
signal hdc_to_sc_lat                   : array_3d(accl_range)(SPM_NUM-1 downto 0)(1 downto 0);
signal dsp_sc_acc_data_read_wire       : array_3d(accl_range)(1 downto 0)(MCR_Acc_SIMD_Width-1 downto 0);
signal dsp_sc_data_read_wire           : array_3d(accl_range)(1 downto 0)(SIMD_Width-1 downto 0);
signal ls_sc_data_read_replicated      : array_2d(accl_range)(Data_Width-1 downto 0);
signal ls_sc_data_read_wire_replicated : array_2d(accl_range)(Data_Width-1 downto 0);
signal dsp_sci_wr_gnt_lat              : std_logic_vector(accl_range);
signal ls_sci_wr_gnt_replicated        : std_logic_vector(accl_range);
signal ls_sci_wr_gnt_lat_replicated    : std_logic_vector(accl_range);
signal halt_dsp                        : std_logic_vector(accl_range);
signal block_dsp_rd                    : std_logic_vector(accl_range);
signal sc_word_count                   : array_2d_int(accl_range);
signal sc_acc_word_count               : array_2d_int(accl_range);
signal sc_acc_we                       : array_2d(accl_range)(MCR_Acc_SIMD_Width/32-1  downto 0); -- Duplica we e capisci replicated
signal sc_we                           : array_2d(accl_range)(SIMD*(SPM_NUM-1)-1 downto 0); -- Duplica we e capisci replicated
signal sc_addr_wr                      : array_3d(accl_range)(SIMD*(SPM_NUM-1)-1 downto 0)(Addr_Width-(SIMD_BITS+3) downto 0);
signal sc_addr_rd                      : array_3d(accl_range)(SIMD*(SPM_NUM-1)-1 downto 0)(Addr_Width-(SIMD_BITS+3) downto 0);
signal sc_data_wr                      : array_3d(accl_range)(SIMD*(SPM_NUM-1)-1 downto 0)(Data_Width-1 downto 0);
signal sc_data_rd                      : array_3d(accl_range)(SIMD*(SPM_NUM-1)-1 downto 0)(Data_Width-1 downto 0);
signal sc_acc_addr_wr                  : array_3d(accl_range)(MCR_Acc_SIMD_Width/32-1 downto 0)(Addr_Width-(ACC_SIMD_BITS+3) downto 0);
signal sc_acc_addr_rd                  : array_3d(accl_range)(MCR_Acc_SIMD_Width/32-1 downto 0)(Addr_Width-(ACC_SIMD_BITS+3) downto 0);
signal sc_acc_data_wr                  : array_3d(accl_range)(MCR_Acc_SIMD_Width/32-1 downto 0)(Data_Width-1 downto 0);
signal sc_acc_data_rd                  : array_3d(accl_range)(MCR_Acc_SIMD_Width/32-1 downto 0)(Data_Width-1 downto 0); 

component Scratchpad_memory_MCR
  generic(
    SPM_NUM               : natural; 
    Addr_Width            : natural;
    SIMD                  : natural;
    --------------------------------
    ACCL_NUM              : natural;
    SIMD_BITS             : natural;
    Data_Width            : natural
  );
  port(
    clk_i                 : in  std_logic;
    sc_we                 : in  array_2d(accl_range)(SIMD*SPM_NUM-1 downto 0);
    sc_addr_wr            : in  array_3d(accl_range)(SIMD*SPM_NUM-1 downto 0)(Addr_Width-(SIMD_BITS+3) downto 0);
    sc_addr_rd            : in  array_3d(accl_range)(SIMD*SPM_NUM-1 downto 0)(Addr_Width-(SIMD_BITS+3) downto 0);
    sc_data_wr            : in  array_3d(accl_range)(SIMD*SPM_NUM-1 downto 0)(Data_Width-1 downto 0);
    sc_data_rd            : out array_3d(accl_range)(SIMD*SPM_NUM-1 downto 0)(Data_Width-1 downto 0)
    );
end component;
--------------------------------------------------------------------------------------------------
-------------------------------- SCI BEGIN -------------------------------------------------------
begin


  SC : Scratchpad_memory_MCR
    generic map(
      SPM_NUM		        => SPM_NUM-1, 
      Addr_Width        => Addr_Width,
      SIMD              => SIMD,
      --------------------------------
      ACCL_NUM          => ACCL_NUM,
      SIMD_BITS         => SIMD_BITS,
      Data_Width        => Data_Width
    )
    port map(
       sc_we            => sc_we,
       clk_i            => clk_i,
       sc_addr_rd       => sc_addr_rd,
       sc_addr_wr       => sc_addr_wr,
       sc_data_wr       => sc_data_wr,
       sc_data_rd       => sc_data_rd
      );

  SC_acc : Scratchpad_memory_MCR
    generic map(
      SPM_NUM           => 1, 
      Addr_Width        => Addr_Width,
      SIMD              => MCR_Acc_SIMD_Width/32,
      --------------------------------
      ACCL_NUM          => ACCL_NUM,
      SIMD_BITS         => ACC_SIMD_BITS,
      Data_Width        => Data_Width
    )
    port map(
       sc_we            => sc_acc_we,
       clk_i            => clk_i,
       sc_addr_rd       => sc_acc_addr_rd,
       sc_addr_wr       => sc_acc_addr_wr,
       sc_data_wr       => sc_acc_data_wr,
       sc_data_rd       => sc_acc_data_rd
      );


  process(all)
  begin
    for h in accl_range loop
      for i in 0 to SPM_NUM-1 loop
        ls_data_gnt_i(i) <=  ls_data_gnt_internal(h)(0) or ls_data_gnt_internal(h)(i);
      end loop;
    end loop;
  end process;

  SPM_replicated : for h in accl_range generate
  
  SCI_Exec_Unit : process(clk_i, rst_ni)  -- single cycle unit, fully synchronous 
  begin
    if rst_ni = '0' then
      hdc_sc_read_addr_lat(h)        <= (others => (others => '0'));
      dsp_sc_acc_read_addr_lat(h)    <= (others => (others => '0'));
      hdc_to_sc_lat(h)               <= (others => (others => '0'));
      hdc_sci_req_lat(h)             <= (others => '0');
      ls_data_gnt_internal(h)        <= (others => '0');
      halt_dsp(h)                    <= '0';
      sc_word_count(h)               <= 0;
      sc_acc_word_count(h)           <= 0;
    elsif rising_edge(clk_i) then
      halt_dsp(h)                     <= '0';
      dsp_sci_wr_gnt_lat(h)           <= dsp_sci_wr_gnt(h);
      ls_sci_wr_gnt_lat_replicated(h) <= ls_sci_wr_gnt_replicated(h);
      hdc_sci_req_lat(h)              <= hdc_sci_req_MCR(h);
      hdc_to_sc_lat(h)                <= hdc_to_sc_MCR(h);
      if harc_LS_wire = h or spm_bcast = '1' then
        sc_word_count(h)              <= sc_word_count_wire;
        sc_acc_word_count(h)          <= sc_acc_word_count_wire_MCR;
      end if;
      if unsigned(ls_data_gnt_internal(h)) /= 0 then
        ls_sc_data_read_replicated(h) <= ls_sc_data_read_wire_replicated(h);
      end if;
      if (dsp_sci_wr_gnt(h) = '0' and hdc_sci_we_MCR(h) /= (0 to SPM_NUM-1 => '0')) then
        halt_dsp(h) <= '1';
      end if;
      if halt_dsp(h) = '0' then
        dsp_sc_data_read(h) <= dsp_sc_data_read_wire(h);
        dsp_sc_acc_data_read(h) <= dsp_sc_acc_data_read_wire(h);
      end if;

      for i in 0 to SPM_NUM-1 loop
        if ls_sci_req(i) = '1' then  -- AAA most probably useless
          ls_data_gnt_internal(h)(i) <= '1';
        elsif ls_sci_req(i) = '0' then
          ls_data_gnt_internal(h)(i) <= '0';
        end if;
        if hdc_sci_req_MCR(h)(i) = '1' then 
          for k in 0 to 1 loop
            hdc_sc_read_addr_lat(h)(k) <= hdc_sc_read_addr_MCR(h)(k)(SIMD_BITS+1 downto 0);
            dsp_sc_acc_read_addr_lat(h)(k) <= hdc_sc_read_addr_MCR(h)(k)(ACC_SIMD_BITS+1 downto 0);
          end loop;
        end if;
      end loop;
    end if;
  end process;

  ls_sc_data_read_wire <= ls_sc_data_read_wire_replicated(harc_LS_wire);
  ls_sci_wr_gnt        <= ls_sci_wr_gnt_replicated(harc_LS_wire);

  SCI_Exec_Unit_comb : process(all)
  begin
    dsp_data_gnt_i(h)  <= '0';
    block_dsp_rd(h)    <= '0';
    -- Initialize standard SPMs
    for l in 0 to (SIMD*(SPM_NUM-1))-1 loop
      sc_we(h)(l)      <= '0';
      sc_addr_rd(h)(l) <= (others => '0');
      sc_addr_wr(h)(l) <= (others => '0');
      sc_data_wr(h)(l) <= (others => '0');
    end loop;

    -- Initialize accumulator SPM
    for l in 0 to (MCR_Acc_SIMD_Width/32)-1 loop
      sc_acc_we(h)(l)      <= '0';
      sc_acc_addr_rd(h)(l) <= (others => '0');
      sc_acc_addr_wr(h)(l) <= (others => '0');
      sc_acc_data_wr(h)(l) <= (others => '0');
    end loop;

    rd_offset(h)                       <= (others => (others => '0'));
    acc_rd_offset(h)                   <= (others => (others => '0'));
    dsp_sc_data_read_int_wire(h)       <= (others => (others => '0'));
    dsp_sc_acc_data_read_int_wire(h)   <= (others => (others => '0'));
    wr_offset(h)                       <= (others => '0');
    acc_wr_offset(h)                   <= (others => '0');
    ls_sci_wr_gnt_replicated(h)        <= ls_sci_wr_gnt_lat_replicated(h);
    dsp_sci_wr_gnt(h)                  <= dsp_sci_wr_gnt_lat(h);
    ls_sc_data_read_wire_replicated(h) <= ls_sc_data_read_replicated(h);
    dsp_sc_data_write_int_wire(h)      <= (others => '0');
    dsp_sc_acc_data_write_int_wire(h)  <= (others => '0');
    dsp_sc_data_read_wire(h)           <= dsp_sc_data_read(h);
    dsp_sc_acc_data_read_wire(h)       <= dsp_sc_acc_data_read(h);

    for i in 0 to SPM_NUM-1 loop	-- Loop through scratchpads A,B,C,D

      if data_rvalid_i = '1' then        -- LS write port
        if ls_sci_req(i) = '1' and ls_sci_we(i) = '1' and ls_sci_wr_gnt = '1' then
          if harc_LS_wire = h or spm_bcast = '1' then
            
            if (i = SPM_NUM-1) then
              -- Accumulator SPM
              sc_acc_we(h)(sc_acc_word_count(h)) <= '1';
              sc_acc_data_wr(h)(sc_acc_word_count(h)) <= ls_sc_data_write_wire(31 downto 0);
              sc_acc_addr_wr(h)(sc_acc_word_count(h)) <= ls_sc_acc_write_addr_MCR;
            else
              sc_we(h)((SIMD)*i + sc_word_count(h)) <= '1';
              -- Standard SPMs
              sc_data_wr(h)(sc_word_count(h) + (SIMD)*i) <= ls_sc_data_write_wire(31 downto 0);
              sc_addr_wr(h)(sc_word_count(h) + (SIMD)*i) <= ls_sc_write_addr;
            end if;
          end if;
        end if;   
      end if;

      if ls_data_gnt_internal(h)(i) = '1' then
        if harc_LS_wire = h then
          if i = SPM_NUM-1 then
            -- Accumulator SPM
            ls_sc_data_read_wire_replicated(h) <= sc_acc_data_rd(h)(sc_acc_word_count(h));  
          else
            -- Standard SPMs
            ls_sc_data_read_wire_replicated(h) <= sc_data_rd(h)((SIMD)*i + sc_word_count(h));  
          end if;
        end if;
      end if;

      if ls_sci_req(i) = '1' then         -- LS read port
        if harc_LS_wire = h then
          if (i = SPM_NUM-1) then
            -- Accumulator SPM
            sc_acc_addr_rd(h)(sc_acc_word_count_wire_MCR) <= ls_sc_acc_read_addr_MCR;
          else 
            -- Standard SPMs
            sc_addr_rd(h)(sc_word_count_wire + (SIMD)*i) <= ls_sc_read_addr;
          end if;

        end if;
      end if;

      if hdc_sci_we_MCR(h)(i) = '1' and dsp_sci_wr_gnt(h) = '1' then
        if (i = SPM_NUM-1) then  -- Accumulator SPM (wide)
          for j in 0 to (MCR_Acc_SIMD_Width/32)-1 loop
            sc_acc_we(h)(j)           <= dsp_acc_we_word_MCR(h)(j);  -- if hdc_we_word is wide enough, otherwise use a wider version!
            sc_acc_addr_wr(h)(j)      <= std_logic_vector(unsigned(hdc_sc_write_addr_MCR(h)(Addr_Width-1 downto ACC_SIMD_BITS+2)) + acc_wr_offset(h)(j));
            sc_acc_data_wr(h)(j)      <= dsp_sc_acc_data_write_int_wire(h)(31+32*j downto 32*j);
          end loop;
        else  -- Standard SPMs
          for j in 0 to SIMD-1 loop
            sc_we(h)((SIMD)*i+j)       <= hdc_we_word_MCR(h)(j);
            sc_addr_wr(h)((SIMD)*i+j)  <= std_logic_vector(unsigned(hdc_sc_write_addr_MCR(h)(Addr_Width-1 downto SIMD_BITS+2)) + wr_offset(h)(j));
            sc_data_wr(h)((SIMD)*i+j)  <= dsp_sc_data_write_int_wire(h)(31+32*j downto 32*j);
          end loop;
        end if;
      end if;


      if hdc_sci_req_MCR(h)(i) = '1' and hdc_to_sc_MCR(h)(i)(0) = '1' and dsp_data_gnt_i(h) = '1' then
        if (i = SPM_NUM-1) then  -- Accumulator SPM (wide)
          for j in 0 to (MCR_Acc_SIMD_Width/32)-1 loop
            sc_acc_addr_rd(h)(j) <= std_logic_vector(unsigned(hdc_sc_read_addr_MCR(h)(0)(Addr_Width-1 downto ACC_SIMD_BITS+2)) + acc_rd_offset(h)(0)(j));
          end loop;
        else  -- Standard SPMs
          for j in 0 to SIMD-1 loop
            sc_addr_rd(h)((SIMD)*i+j) <= std_logic_vector(unsigned(hdc_sc_read_addr_MCR(h)(0)(Addr_Width-1 downto SIMD_BITS+2)) + rd_offset(h)(0)(j));
          end loop;
        end if;
      end if;


      if i = SPM_NUM-1 then  -- Accumulator SPM (wide port) NEW
        for j in 0 to (MCR_Acc_SIMD_Width/32)-1 loop        -- Loop through the sub-scratchpad banks
           if hdc_sci_req_lat(h)(i) = '1' and hdc_to_sc_lat(h)(i)(0) = '1' then         -- DSP read port 1
              dsp_sc_acc_data_read_int_wire(h)(0)(31+32*j downto 32*j) <= sc_acc_data_rd(h)(j);
           end if;
         end loop;
      else
        for j in 0 to SIMD-1 loop        -- Loop through the sub-scratchpad banks
          if hdc_sci_req_lat(h)(i) = '1' and hdc_to_sc_lat(h)(i)(0) = '1' then         -- DSP read port 1
              dsp_sc_data_read_int_wire(h)(0)(31+32*j downto 32*j) <= sc_data_rd(h)((SIMD)*i+j);
          end if;
        end loop;
      end if;
		
      if hdc_sci_req_MCR(h)(i) = '1' and hdc_to_sc_MCR(h)(i)(1) = '1' and dsp_data_gnt_i(h) = '1' then  -- DSP read port 2
        if (i = SPM_NUM-1) then  -- Accumulator SPM (wide)
          for j in 0 to (MCR_Acc_SIMD_Width/32)-1 loop
            sc_acc_addr_rd(h)(j) <= std_logic_vector(unsigned(hdc_sc_read_addr_MCR(h)(1)(Addr_Width-1 downto ACC_SIMD_BITS+2)) + acc_rd_offset(h)(1)(j));
          end loop;
        else  -- Standard SPMs
          for j in 0 to SIMD-1 loop
            sc_addr_rd(h)((SIMD)*i+j) <= std_logic_vector(unsigned(hdc_sc_read_addr_MCR(h)(1)(Addr_Width-1 downto SIMD_BITS+2)) + rd_offset(h)(1)(j));
          end loop;
        end if;
      end if;

      if i = SPM_NUM -1 then  -- Accumulator SPM (wide port)
        for j in 0 to (MCR_Acc_SIMD_Width/32)-1 loop
          if hdc_sci_req_lat(h)(i) = '1' and hdc_to_sc_lat(h)(i)(1) = '1' then
            dsp_sc_acc_data_read_int_wire(h)(1)(31+32*j downto 32*j) <= sc_acc_data_rd(h)(j);
          end if;
        end loop;
      else  -- Standard SPMs
        for j in 0 to SIMD-1 loop
          if hdc_sci_req_lat(h)(i) = '1' and hdc_to_sc_lat(h)(i)(1) = '1' then
            dsp_sc_data_read_int_wire(h)(1)(31+32*j downto 32*j) <= sc_data_rd(h)((SIMD)*i+j);
          end if;
        end loop;
      end if;


      if (kmemld_inflight(i) = '1' or kmemstr_inflight(i) = '1') and hdc_sci_req_MCR(h)(i) = '1' then
        block_dsp_rd(h) <= '1';
      end if;

      -- Allow a DSP read only if the SPM(i) being loaded belongs to another thread and the instruction is not a broadcast load (data hazard)
      if kmemld_inflight(i) = '1' and hdc_sci_req_MCR(h)(i) = '1' and h /= harc_LS_wire and spm_bcast = '0' then
        dsp_data_gnt_i(h) <= '1';
      -- Allow a dsp read only when it is not currently being read by a kmemstr becuase we only have one read port (structural hazard)
      elsif kmemstr_inflight(i) = '1' and hdc_sci_req_MCR(h)(i) = '1' and h /= harc_LS_wire then
        dsp_data_gnt_i(h) <= '1';
      -- Allow a DSP read if there are no current LSU accesses to SPM(i)
      elsif kmemld_inflight(i) = '0' and kmemstr_inflight(i) = '0' and hdc_sci_req_MCR(h)(i) = '1' and block_dsp_rd(h) = '0' then
        dsp_data_gnt_i(h) <= '1';
      end if;

      if hdc_sci_we_MCR(h) = (0 to SPM_NUM-1 => '0') then
        dsp_sci_wr_gnt(h) <= '0';
      -- Allow the DSP to write only if the kmemld is filling the SPM(i) of another thread
      elsif kmemld_inflight(i) = '1' and hdc_sci_we_MCR(h)(i) = '1' and h /= harc_LS_wire and spm_bcast = '0' then
        dsp_sci_wr_gnt(h) <= '1';
      -- Allow the DSP to write only when the kmemstr is reading SPM(i) of another thread
      elsif kmemstr_inflight(i) = '1' and hdc_sci_we_MCR(h)(i) = '1' and  h /= harc_LS_wire then
        dsp_sci_wr_gnt(h) <= '1';
      -- Allow the DSP to write if there are no current LSU accesses to SPM(i)
      elsif kmemld_inflight(i) = '0' and kmemstr_inflight(i) = '0' and hdc_sci_we_MCR(h)(i) = '1' then
        dsp_sci_wr_gnt(h) <= '1';
      end if;

      if kmemld_inflight(i) = '1' and hdc_sci_we_MCR(h)(i) = '0' then -- One LSU write enable request will put the ls_sci_wr_gnt to '1' if there are no ongoing DSP writes to the same scratchpad
        ls_sci_wr_gnt_replicated(h) <= '1';
      elsif kmemld_inflight(i) = '1' and hdc_sci_we_MCR(h)(i) = '1' and (h /= harc_LS_wire) and spm_bcast = '0' then
        ls_sci_wr_gnt_replicated(h) <= '1';
      --elsif unsigned(kmemld_inflight) = 0 then   -- All the ls_sci_we must be zero in-order to switch the ls_sci_wr_gnt back to '0'
      elsif kmemld_inflight = (SPM_NUM-1 downto 0 => '0') then
        ls_sci_wr_gnt_replicated(h) <= '0';
      end if;
    end loop;

 -----------------------------------------------------------------------------------------------
--  ██████╗  █████╗ ████████╗ █████╗     ██████╗  ██████╗ ████████╗ █████╗ ████████╗███████╗  --
--  ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗    ██╔══██╗██╔═══██╗╚══██╔══╝██╔══██╗╚══██╔══╝██╔════╝  --
--  ██║  ██║███████║   ██║   ███████║    ██████╔╝██║   ██║   ██║   ███████║   ██║   █████╗    --
--  ██║  ██║██╔══██║   ██║   ██╔══██║    ██╔══██╗██║   ██║   ██║   ██╔══██║   ██║   ██╔══╝    --
--  ██████╔╝██║  ██║   ██║   ██║  ██║    ██║  ██║╚██████╔╝   ██║   ██║  ██║   ██║   ███████╗  --
--  ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝    ╚═╝  ╚═╝ ╚═════╝    ╚═╝   ╚═╝  ╚═╝   ╚═╝   ╚══════╝  --
------------------------------------------------------------------------------------------------ 

    for i in 0 to SIMD-1 loop
      --if (to_integer(unsigned(hdc_sc_write_addr(h)(SIMD_BITS+1 downto 0))) = 4*i) and (i /= 0) then
      if (hdc_sc_write_addr_MCR(h)(SIMD_BITS+1 downto 0) = std_logic_vector(to_unsigned(4*i, SIMD_BITS+2))) and (i /= 0) then
        wr_offset(h)(i-1 downto 0) <= (others => '1');
      end if;
    end loop;

    -- For accumulator SPM (wide)
    for i in 0 to (MCR_Acc_SIMD_Width/32)-1 loop
      if (hdc_sc_write_addr_MCR(h)(ACC_SIMD_BITS+1 downto 0) = std_logic_vector(to_unsigned(4*i, ACC_SIMD_BITS+2))) and (i /= 0) then
        acc_wr_offset(h)(i-1 downto 0) <= (others => '1');
      end if;
    end loop;
    for i in 0 to SIMD-1 loop		  
      --if (to_integer(unsigned(hdc_sc_write_addr(h)(SIMD_BITS+1 downto 0))) = 4*i) then
      if hdc_sc_write_addr_MCR(h)(SIMD_BITS+1 downto 0) = std_logic_vector(to_unsigned(4*i, SIMD_BITS+2)) then
        for j in 0 to SIMD-1 loop
          if j <= (SIMD-1)-i then
            dsp_sc_data_write_int_wire(h)(31+32*(j+i) downto 32*(j+i)) <= hdc_sc_data_write_wire_MCR(h)(31+32*j downto 32*j);
          elsif j > (SIMD-1)-i then
            dsp_sc_data_write_int_wire(h)(31+32*(j-(SIMD-1)+(i-1)) downto 32*(j-(SIMD-1)+(i-1))) <= hdc_sc_data_write_wire_MCR(h)(31+32*j downto 32*j);
          end if;
        end loop;
      end if;
    end loop;

    -- Data rotator for ACCUMULATOR SPM
    for i in 0 to (MCR_Acc_SIMD_Width/32)-1 loop
      if hdc_sc_write_addr_MCR(h)(ACC_SIMD_BITS+1 downto 0) = std_logic_vector(to_unsigned(4*i, ACC_SIMD_BITS+2)) then
        for j in 0 to (MCR_Acc_SIMD_Width/32)-1 loop
          if j <= ((MCR_Acc_SIMD_Width/32)-1) - i then
            dsp_sc_acc_data_write_int_wire(h)(31+32*(j+i) downto 32*(j+i)) <= dsp_sc_acc_data_write_wire_MCR(h)(31+32*j downto 32*j);
          elsif j > ((MCR_Acc_SIMD_Width/32)-1) - i then
            dsp_sc_acc_data_write_int_wire(h)(31+32*(j-((MCR_Acc_SIMD_Width/32)-1)+(i-1)) downto 32*(j-((MCR_Acc_SIMD_Width/32)-1)+(i-1))) <= dsp_sc_acc_data_write_wire_MCR(h)(31+32*j downto 32*j);
          end if;
        end loop;
      end if;
    end loop;


    --for i in 0 to 4*SIMD-1 loop     
    --  for j in 0 to 4*SIMD-1 loop
    --    if j <= (4*SIMD-1)-i then
    --      dsp_sc_data_write_int_wire(h)(7+8*(j+i) downto 8*(j+i)) <= hdc_sc_data_write_wire(h)(7+8*j downto 8*j);
    --    elsif j > (4*SIMD-1)-i then
    --      dsp_sc_data_write_int_wire(h)(7+8*(j-(SIMD-1)+(i-1)) downto 8*(j-(SIMD-1)+(i-1))) <= hdc_sc_data_write_wire(h)(7+8*j downto 8*j);
    --    end if;
    --  end loop;
    --end loop;
	  
    for k in 0 to 1 loop  -- index for the rs1 and rs2 read addresses
      for i in 0 to SIMD-1 loop -- index points to the 
        --if (to_integer(unsigned(hdc_sc_read_addr(h)(k)(SIMD_BITS+1 downto 0))) = 4*i) and (i /= 0) then -- 4*i instead of i is because the address is word aligned and not byte aligned
        if (hdc_sc_read_addr_MCR(h)(k)(SIMD_BITS+1 downto 0) = std_logic_vector(to_unsigned(4*i, SIMD_BITS+2))) and (i /= 0) then
          rd_offset(h)(k)(i-1 downto 0) <= (others => '1'); -- sets the bank offset withing the scratchpad that will be used for the correct bank access
        end if;
      end loop;

      -- Data rotator for ACCUMULATOR SPM
      for k in 0 to 1 loop
        for i in 0 to (MCR_Acc_SIMD_Width/32)-1 loop
          if (hdc_sc_read_addr_MCR(h)(k)(ACC_SIMD_BITS+1 downto 0) = std_logic_vector(to_unsigned(4*i, ACC_SIMD_BITS+2))) and (i /= 0) then
            acc_rd_offset(h)(k)(i-1 downto 0) <= (others => '1');
          end if;
        end loop;
      end loop;

      for i in 0 to SIMD-1 loop
        --if (to_integer(unsigned(hdc_sc_read_addr_lat(h)(k))) = 4*i) then
        if hdc_sc_read_addr_lat(h)(k) = std_logic_vector(to_unsigned(4*i, SIMD_BITS+2)) then
          for j in 0 to SIMD-1 loop
            if j >= i then
              dsp_sc_data_read_wire(h)(k)(31+32*(j-i) downto 32*(j-i)) <= dsp_sc_data_read_int_wire(h)(k)(31+32*j downto 32*j);
            elsif j < i then
              dsp_sc_data_read_wire(h)(k)(31+32*((SIMD-1)-i+(j+1)) downto 32*((SIMD-1)-i+(j+1))) <= dsp_sc_data_read_int_wire(h)(k)(31+32*j downto 32*j);
            end if;
          end loop;
        end if;
      end loop;

    for i in 0 to (MCR_Acc_SIMD_Width/32)-1 loop
      if dsp_sc_acc_read_addr_lat(h)(k) = std_logic_vector(to_unsigned(4*i, ACC_SIMD_BITS+2)) then
        for j in 0 to (MCR_Acc_SIMD_Width/32)-1 loop
          if j >= i then
            dsp_sc_acc_data_read_wire(h)(k)(31+32*(j-i) downto 32*(j-i)) <= dsp_sc_acc_data_read_int_wire(h)(k)(31+32*j downto 32*j);
          elsif j < i then
            dsp_sc_acc_data_read_wire(h)(k)(31+32*((MCR_Acc_SIMD_Width/32)-1-i+(j+1)) downto 32*((MCR_Acc_SIMD_Width/32)-1-i+(j+1))) <= dsp_sc_acc_data_read_int_wire(h)(k)(31+32*j downto 32*j);
          end if;
        end loop;
      end if;
    end loop;


      --for i in 0 to 4*SIMD-1 loop
      --  for j in 0 to 4*SIMD-1 loop
      --    if j >= i then
      --      dsp_sc_data_read_wire(h)(k)(7+8*(j-i) downto 8*(j-i)) <= dsp_sc_data_read_int_wire(h)(k)(7+8*j downto 8*j);
      --    elsif j < i then
      --      dsp_sc_data_read_wire(h)(k)(7+8*((SIMD-1)-i+(j+1)) downto 8*((SIMD-1)-i+(j+1))) <= dsp_sc_data_read_int_wire(h)(k)(7+8*j downto 8*j);
      --    end if;
      --  end loop;
      --end loop;
    end loop;
  end process;

  end generate SPM_replicated;

--------------------------------------------------------------------- end of SCI Logic -----------
--------------------------------------------------------------------------------------------------  

end SCI_MCR;
--------------------------------------------------------------------------------------------------
-- END of SCI architecture -----------------------------------------------------------------------
--------------------------------------------------------------------------------------------------