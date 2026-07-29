-- ieee packages
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;
use std.textio.all;
use ieee.math_real.all;

-- local packages

package riscv_klessydra is

 -- package parameters is new work.klessydra_parameters
 -- generic (
 -- RV32E                 : natural -- Regfile size, Can be set to 32 for RV32E being 0 else 16 for RV32E being set to 1
 --   );
 -- generic map (
 --             RV32E                 => RV32E
 --             );
 --
 -- constant RF_SIZE : natural := 32-16*RV32E;
 -- constant RF_CEIL : natural := integer(ceil(log2(real(RF_SIZE))));

  -- instruction trace file
  file file_handler : text open write_mode is "execution_trace.txt";

  file file_handler0 : text open write_mode is "execution_0.txt";

  file file_handler1 : text open write_mode is "execution_1.txt";

  file file_handler2 : text open write_mode is "execution_2.txt";

------------------------------------------------------------------------------------------------------------
--   ██████╗██╗   ██╗███████╗████████╗ ██████╗ ███╗   ███╗    ████████╗██╗   ██╗██████╗ ███████╗███████╗  --
--  ██╔════╝██║   ██║██╔════╝╚══██╔══╝██╔═══██╗████╗ ████║    ╚══██╔══╝╚██╗ ██╔╝██╔══██╗██╔════╝██╔════╝  --
--  ██║     ██║   ██║███████╗   ██║   ██║   ██║██╔████╔██║       ██║    ╚████╔╝ ██████╔╝█████╗  ███████╗  --
--  ██║     ██║   ██║╚════██║   ██║   ██║   ██║██║╚██╔╝██║       ██║     ╚██╔╝  ██╔═══╝ ██╔══╝  ╚════██║  --
--  ╚██████╗╚██████╔╝███████║   ██║   ╚██████╔╝██║ ╚═╝ ██║       ██║      ██║   ██║     ███████╗███████║  --
--   ╚═════╝ ╚═════╝ ╚══════╝   ╚═╝    ╚═════╝ ╚═╝     ╚═╝       ╚═╝      ╚═╝   ╚═╝     ╚══════╝╚══════╝  --
------------------------------------------------------------------------------------------------------------
--Shared
  type array_2d           is array (integer range<>) of std_logic_vector;
  type array_2d_int       is array (integer range<>) of integer;
  type array_2d_csnt_int  is array (integer range<>) of integer range 0 to 24;
  type array_2d_shift_int is array (integer range<>) of integer range 0 to 7;
  type array_2d_int_sim   is array (integer range<>) of integer range 0 to 8192;

  type array_3d           is array (integer range<>) of array_2d;
  type array_3d_int       is array (integer range<>) of array_2d_int;
  type array_3d_csnt_int  is array (integer range<>) of array_2d_csnt_int;  

  type CountArray         is array (integer range<>) of std_logic_vector; 
  type CountArray2d       is array (integer range<>) of CountArray;
  type CountArray3d       is array (integer range<>) of CountArray2d;

  type fsm_IE_states      is (sleep, reset, normal, csr_instr_wait_state, debug);
  type mulh_states        is (init, mult, accum);
  type mul_states         is (mult, accum);
  type div_states         is (init, divide);
  type fsm_LS_states      is (normal , data_valid_waiting);
  
--MCR
---------------------------------------------------------------------------
  constant MODULO_BITS          : integer := 4;
  type array_2d_index     is array (integer range <>) of integer range 0 to 2**MODULO_BITS-1;
  type array_2d_signed    is array (integer range<>) of signed;
  type array_2d_bool      is array (integer range<>) of integer range 0 to 1;

  type array_3d_signed    is array (integer range<>) of array_2d_signed;

--FHRR
---------------------------------------------------------------------------
  type array_2d_nat       is array (integer range<>) of natural;
  
  type array_4d           is array (integer range<>) of array_3d;
  type array_4d_int       is array (integer range<>) of array_3d_int;
  -- Custom types for dividers
  type array_2d_int_div_32 is array (integer range<>) of integer range -1 to 31;
  type array_2d_int_div_16 is array (integer range<>) of integer range -1 to 15;
  type array_2d_int_div_8  is array (integer range<>) of integer range -1 to 7;
  type array_3d_int_div_32 is array (integer range<>) of array_2d_int_div_32;
  type array_3d_int_div_16 is array (integer range<>) of array_2d_int_div_16;
  type array_3d_int_div_8  is array (integer range<>) of array_2d_int_div_8;
  --MCR
---------------------------------------------------------------------------

  -- The accelerators states are defined here
  constant hdc_init                : std_logic_vector(1 downto 0) := "00";
  constant hdc_halt_hart           : std_logic_vector(1 downto 0) := "01";
  constant hdc_exec                : std_logic_vector(1 downto 0) := "10";

  constant ACCL_SEL_DSP            : natural := 0;
  constant ACCL_SEL_FHRR           : natural := 1;

  constant THREAD_POOL_BASELINE    : integer := 3;
  constant THREAD_ID_SIZE          : integer := 4;
  constant NOP_POOL_SIZE           : integer := 2;
  constant BRANCHING_DELAY_SLOT    : integer := 3;
  --constant HARC_SIZE               : integer := THREAD_POOL_SIZE;

  constant SLEEP_MODE              : natural := 0;
  constant SINGLE_HART_MODE        : natural := 1;
  constant DUAL_HART_MODE          : natural := 2;
  constant IMT_MODE                : natural := THREAD_POOL_BASELINE;

  constant COUNTER_BITS             : integer := 4;
--MCR
----------------------------------------------------------------------------------------------------------------
  type lut16_t is array(0 to 15) of std_logic_vector(15 downto 0);
  constant COS_LUT : lut16_t := (
    0 => x"0000",
    1 => x"0188",
    2 => x"02D4",
    3 => x"03B2",
    4 => x"0400",
    5 => x"03B2",
    6 => x"02D4",
    7 => x"0188",
    8 => x"0000",
    9 => x"FE78",
    10 => x"FD2C",
    11 => x"FC4E",
    12 => x"FC00",
    13 => x"FC4E",
    14 => x"FD2C",
    15 => x"FE78"
);


constant SIN_LUT : lut16_t := (
    0 => x"0400",
    1 => x"03B2",
    2 => x"02D4",
    3 => x"0188",
    4 => x"0000",
    5 => x"FE78",
    6 => x"FD2C",
    7 => x"FC4E",
    8 => x"FC00",
    9 => x"FC4E",
    10 => x"FD2C",
    11 => x"FE78",
    12 => x"0000",
    13 => x"0188",
    14 => x"02D4",
    15 => x"03B2"
);

--FHRR
----------------------------------------------------------------------------------------------------------------
  type cosine_lut_type is array (0 to 255) of std_logic_vector(31 downto 0);

  constant cosine_lut : cosine_lut_type := (
      x"00000100", x"000000FF", x"000000FF", x"000000FF", x"000000FF", x"000000FF", x"000000FE", x"000000FE",
      x"000000FE", x"000000FD", x"000000FC", x"000000FC", x"000000FB", x"000000FA", x"000000F9", x"000000F9",
      x"000000F8", x"000000F7", x"000000F5", x"000000F4", x"000000F3", x"000000F2", x"000000F1", x"000000EF",
      x"000000EE", x"000000EC", x"000000EB", x"000000E9", x"000000E7", x"000000E6", x"000000E4", x"000000E2",
      x"000000E0", x"000000DE", x"000000DC", x"000000DA", x"000000D8", x"000000D6", x"000000D4", x"000000D1",
      x"000000CF", x"000000CD", x"000000CA", x"000000C8", x"000000C5", x"000000C3", x"000000C0", x"000000BE",
      x"000000BB", x"000000B8", x"000000B5", x"000000B2", x"000000B0", x"000000AD", x"000000AA", x"000000A7",
      x"000000A4", x"000000A1", x"0000009D", x"0000009A", x"00000097", x"00000094", x"00000090", x"0000008D",
      x"0000008A", x"00000086", x"00000083", x"00000080", x"0000007C", x"00000079", x"00000075", x"00000071",
      x"0000006E", x"0000006A", x"00000067", x"00000063", x"0000005F", x"0000005C", x"00000058", x"00000054",
      x"00000050", x"0000004C", x"00000049", x"00000045", x"00000041", x"0000003D", x"00000039", x"00000035",
      x"00000031", x"0000002D", x"00000029", x"00000025", x"00000022", x"0000001E", x"0000001A", x"00000016",
      x"00000012", x"0000000E", x"0000000A", x"00000006", x"00000002", x"FFFFFFFF", x"FFFFFFFB", x"FFFFFFF7",
      x"FFFFFFF3", x"FFFFFFEF", x"FFFFFFEB", x"FFFFFFE7", x"FFFFFFE3", x"FFFFFFDF", x"FFFFFFDB", x"FFFFFFD7",
      x"FFFFFFD3", x"FFFFFFCF", x"FFFFFFCB", x"FFFFFFC7", x"FFFFFFC3", x"FFFFFFBF", x"FFFFFFBB", x"FFFFFFB8",
      x"FFFFFFB4", x"FFFFFFB0", x"FFFFFFAC", x"FFFFFFA8", x"FFFFFFA5", x"FFFFFFA1", x"FFFFFF9D", x"FFFFFF9A",
      x"FFFFFF96", x"FFFFFF92", x"FFFFFF8F", x"FFFFFF8B", x"FFFFFF88", x"FFFFFF84", x"FFFFFF81", x"FFFFFF7D",
      x"FFFFFF7A", x"FFFFFF76", x"FFFFFF73", x"FFFFFF70", x"FFFFFF6C", x"FFFFFF69", x"FFFFFF66", x"FFFFFF63",
      x"FFFFFF60", x"FFFFFF5D", x"FFFFFF5A", x"FFFFFF57", x"FFFFFF54", x"FFFFFF51", x"FFFFFF4E", x"FFFFFF4B",
      x"FFFFFF48", x"FFFFFF45", x"FFFFFF43", x"FFFFFF40", x"FFFFFF3D", x"FFFFFF3B", x"FFFFFF38", x"FFFFFF36",
      x"FFFFFF33", x"FFFFFF31", x"FFFFFF2F", x"FFFFFF2C", x"FFFFFF2A", x"FFFFFF28", x"FFFFFF26", x"FFFFFF24",
      x"FFFFFF22", x"FFFFFF20", x"FFFFFF1E", x"FFFFFF1C", x"FFFFFF1A", x"FFFFFF19", x"FFFFFF17", x"FFFFFF15",
      x"FFFFFF14", x"FFFFFF12", x"FFFFFF11", x"FFFFFF10", x"FFFFFF0E", x"FFFFFF0D", x"FFFFFF0C", x"FFFFFF0B",
      x"FFFFFF0A", x"FFFFFF09", x"FFFFFF08", x"FFFFFF07", x"FFFFFF06", x"FFFFFF05", x"FFFFFF04", x"FFFFFF04",
      x"FFFFFF03", x"FFFFFF03", x"FFFFFF02", x"FFFFFF02", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01",
      x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01",
      x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01",
      x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01",
      x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01",
      x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01",
      x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01",
      x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01", x"FFFFFF01"
  );
  

constant cosine_lut_bun : cosine_lut_type := (
  x"00010000", x"0000FFF8", x"0000FFE0", x"0000FFB8", x"0000FF80", x"0000FF38", x"0000FEE0", x"0000FE78",
  x"0000FE00", x"0000FD79", x"0000FCE1", x"0000FC3A", x"0000FB83", x"0000FABC", x"0000F9E6", x"0000F900",
  x"0000F80A", x"0000F705", x"0000F5F1", x"0000F4CD", x"0000F399", x"0000F257", x"0000F105", x"0000EFA5",
  x"0000EE35", x"0000ECB7", x"0000EB29", x"0000E98D", x"0000E7E3", x"0000E62A", x"0000E462", x"0000E28D",
  x"0000E0A9", x"0000DEB7", x"0000DCB7", x"0000DAA9", x"0000D88E", x"0000D665", x"0000D42F", x"0000D1EB",
  x"0000CF9B", x"0000CD3D", x"0000CAD3", x"0000C85C", x"0000C5D8", x"0000C348", x"0000C0AC", x"0000BE04",
  x"0000BB4F", x"0000B890", x"0000B5C4", x"0000B2EE", x"0000B00C", x"0000AD1F", x"0000AA27", x"0000A725",
  x"0000A418", x"0000A101", x"00009DE0", x"00009AB5", x"00009780", x"00009442", x"000090FB", x"00008DAA",
  x"00008A51", x"000086EF", x"00008384", x"00008012", x"00007C97", x"00007915", x"0000758B", x"000071FA",
  x"00006E61", x"00006AC2", x"0000671C", x"0000636F", x"00005FBD", x"00005C04", x"00005846", x"00005482",
  x"000050B8", x"00004CEA", x"00004917", x"00004540", x"00004164", x"00003D84", x"000039A0", x"000035B8",
  x"000031CD", x"00002DDF", x"000029EF", x"000025FB", x"00002205", x"00001E0D", x"00001A14", x"00001618",
  x"0000121B", x"00000E1D", x"00000A1F", x"0000061F", x"0000021F", x"FFFFFE20", x"FFFFFA20", x"FFFFF621",
  x"FFFFF222", x"FFFFEE24", x"FFFFEA27", x"FFFFE62B", x"FFFFE232", x"FFFFDE39", x"FFFFDA44", x"FFFFD650",
  x"FFFFD25F", x"FFFFCE71", x"FFFFCA86", x"FFFFC69E", x"FFFFC2BA", x"FFFFBEDA", x"FFFFBAFE", x"FFFFB726",
  x"FFFFB352", x"FFFFAF84", x"FFFFABBA", x"FFFFA7F6", x"FFFFA437", x"FFFFA07E", x"FFFF9CCB", x"FFFF991E",
  x"FFFF9578", x"FFFF91D8", x"FFFF8E3F", x"FFFF8AAE", x"FFFF8723", x"FFFF83A0", x"FFFF8025", x"FFFF7CB2",
  x"FFFF7947", x"FFFF75E5", x"FFFF728B", x"FFFF6F3A", x"FFFF6BF2", x"FFFF68B3", x"FFFF657E", x"FFFF6252",
  x"FFFF5F31", x"FFFF5C19", x"FFFF590B", x"FFFF5608", x"FFFF5310", x"FFFF5022", x"FFFF4D40", x"FFFF4A68",
  x"FFFF479C", x"FFFF44DC", x"FFFF4227", x"FFFF3F7E", x"FFFF3CE1", x"FFFF3A50", x"FFFF37CC", x"FFFF3554",
  x"FFFF32E9", x"FFFF308A", x"FFFF2E39", x"FFFF2BF5", x"FFFF29BE", x"FFFF2794", x"FFFF2578", x"FFFF2369",
  x"FFFF2168", x"FFFF1F76", x"FFFF1D91", x"FFFF1BBA", x"FFFF19F2", x"FFFF1838", x"FFFF168D", x"FFFF14F0",
  x"FFFF1361", x"FFFF11E2", x"FFFF1071", x"FFFF0F10", x"FFFF0DBD", x"FFFF0C7A", x"FFFF0B46", x"FFFF0A21",
  x"FFFF090C", x"FFFF0806", x"FFFF070F", x"FFFF0628", x"FFFF0551", x"FFFF0489", x"FFFF03D1", x"FFFF0329",
  x"FFFF0290", x"FFFF0208", x"FFFF018F", x"FFFF0126", x"FFFF00CD", x"FFFF0084", x"FFFF004B", x"FFFF0023",
  x"FFFF000A", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001",
  x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001",
  x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001",
  x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001",
  x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001",
  x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001",
  x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001", x"FFFF0001"
);

  
type sine_lut_type is array (0 to 255) of std_logic_vector(31 downto 0);

constant sine_lut : sine_lut_type := (
  x"00000000", x"000003FF", x"000007FF", x"00000BFE", x"00000FFD", x"000013FA", x"000017F7", x"00001BF1",
  x"00001FEA", x"000023E1", x"000027D6", x"00002BC8", x"00002FB8", x"000033A4", x"0000378D", x"00003B73",
  x"00003F55", x"00004334", x"0000470D", x"00004AE3", x"00004EB4", x"00005280", x"00005646", x"00005A08",
  x"00005DC4", x"00006179", x"00006529", x"000068D3", x"00006C76", x"00007012", x"000073A7", x"00007735",
  x"00007ABB", x"00007E3A", x"000081B1", x"00008520", x"00008886", x"00008BE4", x"00008F39", x"00009285",
  x"000095C8", x"00009902", x"00009C32", x"00009F59", x"0000A275", x"0000A587", x"0000A88F", x"0000AB8D",
  x"0000AE7F", x"0000B167", x"0000B444", x"0000B715", x"0000B9DB", x"0000BC96", x"0000BF44", x"0000C1E7",
  x"0000C47D", x"0000C707", x"0000C985", x"0000CBF6", x"0000CE5B", x"0000D0B2", x"0000D2FD", x"0000D53A",
  x"0000D76A", x"0000D98D", x"0000DBA2", x"0000DDA9", x"0000DFA2", x"0000E18D", x"0000E36B", x"0000E53A",
  x"0000E6FB", x"0000E8AD", x"0000EA51", x"0000EBE6", x"0000ED6C", x"0000EEE4", x"0000F04C", x"0000F1A6",
  x"0000F2F0", x"0000F42B", x"0000F557", x"0000F674", x"0000F781", x"0000F87F", x"0000F96E", x"0000FA4C",
  x"0000FB1B", x"0000FBDB", x"0000FC8A", x"0000FD2A", x"0000FDBA", x"0000FE3A", x"0000FEAB", x"0000FF0B",
  x"0000FF5B", x"0000FF9C", x"0000FFCC", x"0000FFED", x"0000FFFD", x"0000FFFE", x"0000FFEE", x"0000FFCF",
  x"0000FF9F", x"0000FF60", x"0000FF10", x"0000FEB1", x"0000FE42", x"0000FDC3", x"0000FD34", x"0000FC95",
  x"0000FBE6", x"0000FB28", x"0000FA5A", x"0000F97C", x"0000F88E", x"0000F792", x"0000F685", x"0000F56A",
  x"0000F43E", x"0000F304", x"0000F1BB", x"0000F062", x"0000EEFA", x"0000ED84", x"0000EBFE", x"0000EA6A",
  x"0000E8C7", x"0000E716", x"0000E556", x"0000E388", x"0000E1AB", x"0000DFC1", x"0000DDC8", x"0000DBC2",
  x"0000D9AE", x"0000D78C", x"0000D55D", x"0000D321", x"0000D0D7", x"0000CE80", x"0000CC1D", x"0000C9AC",
  x"0000C72F", x"0000C4A6", x"0000C210", x"0000BF6E", x"0000BCC0", x"0000BA07", x"0000B742", x"0000B471",
  x"0000B195", x"0000AEAE", x"0000ABBC", x"0000A8BF", x"0000A5B8", x"0000A2A6", x"00009F8A", x"00009C65",
  x"00009935", x"000095FC", x"000092B9", x"00008F6E", x"00008C19", x"000088BC", x"00008556", x"000081E7",
  x"00007E71", x"00007AF3", x"0000776D", x"000073DF", x"0000704B", x"00006CAF", x"0000690C", x"00006563",
  x"000061B4", x"00005DFF", x"00005A43", x"00005682", x"000052BC", x"00004EF0", x"00004B20", x"0000474A",
  x"00004371", x"00003F93", x"00003BB1", x"000037CB", x"000033E2", x"00002FF6", x"00002C07", x"00002815",
  x"00002420", x"00002029", x"00001C30", x"00001836", x"0000143A", x"0000103C", x"00000C3E", x"0000083F",
  x"0000043F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F",
  x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F",
  x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F",
  x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F",
  x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F",
  x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F",
  x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F"
);

constant sine_lut_bun : sine_lut_type := (
  x"00000000", x"000003FF", x"000007FF", x"00000BFE", x"00000FFD", x"000013FA", x"000017F7", x"00001BF1",
  x"00001FEA", x"000023E1", x"000027D6", x"00002BC8", x"00002FB8", x"000033A4", x"0000378D", x"00003B73",
  x"00003F55", x"00004334", x"0000470D", x"00004AE3", x"00004EB4", x"00005280", x"00005646", x"00005A08",
  x"00005DC4", x"00006179", x"00006529", x"000068D3", x"00006C76", x"00007012", x"000073A7", x"00007735",
  x"00007ABB", x"00007E3A", x"000081B1", x"00008520", x"00008886", x"00008BE4", x"00008F39", x"00009285",
  x"000095C8", x"00009902", x"00009C32", x"00009F59", x"0000A275", x"0000A587", x"0000A88F", x"0000AB8D",
  x"0000AE7F", x"0000B167", x"0000B444", x"0000B715", x"0000B9DB", x"0000BC96", x"0000BF44", x"0000C1E7",
  x"0000C47D", x"0000C707", x"0000C985", x"0000CBF6", x"0000CE5B", x"0000D0B2", x"0000D2FD", x"0000D53A",
  x"0000D76A", x"0000D98D", x"0000DBA2", x"0000DDA9", x"0000DFA2", x"0000E18D", x"0000E36B", x"0000E53A",
  x"0000E6FB", x"0000E8AD", x"0000EA51", x"0000EBE6", x"0000ED6C", x"0000EEE4", x"0000F04C", x"0000F1A6",
  x"0000F2F0", x"0000F42B", x"0000F557", x"0000F674", x"0000F781", x"0000F87F", x"0000F96E", x"0000FA4C",
  x"0000FB1B", x"0000FBDB", x"0000FC8A", x"0000FD2A", x"0000FDBA", x"0000FE3A", x"0000FEAB", x"0000FF0B",
  x"0000FF5B", x"0000FF9C", x"0000FFCC", x"0000FFED", x"0000FFFD", x"0000FFFE", x"0000FFEE", x"0000FFCF",
  x"0000FF9F", x"0000FF60", x"0000FF10", x"0000FEB1", x"0000FE42", x"0000FDC3", x"0000FD34", x"0000FC95",
  x"0000FBE6", x"0000FB28", x"0000FA5A", x"0000F97C", x"0000F88E", x"0000F792", x"0000F685", x"0000F56A",
  x"0000F43E", x"0000F304", x"0000F1BB", x"0000F062", x"0000EEFA", x"0000ED84", x"0000EBFE", x"0000EA6A",
  x"0000E8C7", x"0000E716", x"0000E556", x"0000E388", x"0000E1AB", x"0000DFC1", x"0000DDC8", x"0000DBC2",
  x"0000D9AE", x"0000D78C", x"0000D55D", x"0000D321", x"0000D0D7", x"0000CE80", x"0000CC1D", x"0000C9AC",
  x"0000C72F", x"0000C4A6", x"0000C210", x"0000BF6E", x"0000BCC0", x"0000BA07", x"0000B742", x"0000B471",
  x"0000B195", x"0000AEAE", x"0000ABBC", x"0000A8BF", x"0000A5B8", x"0000A2A6", x"00009F8A", x"00009C65",
  x"00009935", x"000095FC", x"000092B9", x"00008F6E", x"00008C19", x"000088BC", x"00008556", x"000081E7",
  x"00007E71", x"00007AF3", x"0000776D", x"000073DF", x"0000704B", x"00006CAF", x"0000690C", x"00006563",
  x"000061B4", x"00005DFF", x"00005A43", x"00005682", x"000052BC", x"00004EF0", x"00004B20", x"0000474A",
  x"00004371", x"00003F93", x"00003BB1", x"000037CB", x"000033E2", x"00002FF6", x"00002C07", x"00002815",
  x"00002420", x"00002029", x"00001C30", x"00001836", x"0000143A", x"0000103C", x"00000C3E", x"0000083F",
  x"0000043F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F",
  x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F",
  x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F",
  x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F",
  x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F",
  x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F",
  x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F", x"0000003F"
);

type atan_lut_type is array (0 to 255) of std_logic_vector(31 downto 0);
  
constant atan_lut : atan_lut_type := (
  x"00000000", x"00001FD6", x"00003EB7", x"00005BD8", x"000076B2", x"00008F00", x"0000A4BC", x"0000B805", x"0000C910", x"0000D81A",
  x"0000E564", x"0000F127", x"0000FB98", x"000104E6", x"00010D39", x"000114B2", x"00011B6E", x"00012186", x"0001270F", x"00012C1A",
  x"000130B7", x"000134F2", x"000138D7", x"00013C6E", x"00013FC1", x"000142D7", x"000145B5", x"00014862", x"00014AE1", x"00014D38",
  x"00014F69", x"00015178", x"00015369", x"0001553D", x"000156F7", x"00015899", x"00015A25", x"00015B9D", x"00015D01", x"00015E54",
  x"00015F97", x"000160CB", x"000161F0", x"00016309", x"00016415", x"00016515", x"0001660B", x"000166F7", x"000167D9", x"000168B2",
  x"00016982", x"00016A4B", x"00016B0C", x"00016BC6", x"00016C79", x"00016D26", x"00016DCC", x"00016E6D", x"00016F09", x"00016F9F",
  x"00017031", x"000170BE", x"00017146", x"000171CA", x"0001724A", x"000172C6", x"0001733F", x"000173B3", x"00017425", x"00017493",
  x"000174FE", x"00017566", x"000175CC", x"0001762E", x"0001768E", x"000176EC", x"00017746", x"0001779F", x"000177F5", x"0001784A",
  x"0001789C", x"000178EC", x"0001793A", x"00017986", x"000179D1", x"00017A1A", x"00017A61", x"00017AA6", x"00017AEA", x"00017B2D",
  x"00017B6E", x"00017BAD", x"00017BEB", x"00017C28", x"00017C64", x"00017C9E", x"00017CD7", x"00017D0F", x"00017D46", x"00017D7B",
  x"00017DB0", x"00017DE4", x"00017E16", x"00017E48", x"00017E78", x"00017EA8", x"00017ED7", x"00017F05", x"00017F32", x"00017F5E",
  x"00017F8A", x"00017FB5", x"00017FDF", x"00018008", x"00018030", x"00018058", x"0001807F", x"000180A6", x"000180CB", x"000180F1",
  x"00018115", x"00018139", x"0001815C", x"0001817F", x"000181A1", x"000181C3", x"000181E4", x"00018205", x"00018225", x"00018245",
  x"00018264", x"00018282", x"000182A1", x"000182BE", x"000182DC", x"000182F9", x"00018315", x"00018331", x"0001834D", x"00018368",
  x"00018383", x"0001839D", x"000183B7", x"000183D1", x"000183EB", x"00018404", x"0001841C", x"00018435", x"0001844D", x"00018464",
  x"0001847C", x"00018493", x"000184AA", x"000184C0", x"000184D6", x"000184EC", x"00018502", x"00018517", x"0001852C", x"00018541",
  x"00018556", x"0001856A", x"0001857E", x"00018592", x"000185A5", x"000185B9", x"000185CC", x"000185DF", x"000185F1", x"00018604",
  x"00018616", x"00018628", x"0001863A", x"0001864B", x"0001865D", x"0001866E", x"0001867F", x"00018690", x"000186A0", x"000186B1",
  x"000186C1", x"000186D1", x"000186E1", x"000186F1", x"00018700", x"0001870F", x"0001871F", x"0001872E", x"0001873D", x"0001874B",
  x"0001875A", x"00018768", x"00018777", x"00018785", x"00018793", x"000187A1", x"000187AE", x"000187BC", x"000187C9", x"000187D7",
  x"000187E4", x"000187F1", x"000187FE", x"0001880A", x"00018817", x"00018824", x"00018830", x"0001883C", x"00018848", x"00018854",
  x"00018860", x"0001886C", x"00018878", x"00018883", x"0001888F", x"0001889A", x"000188A6", x"000188B1", x"000188BC", x"000188C7",
  x"000188D2", x"000188DC", x"000188E7", x"000188F2", x"000188FC", x"00018907", x"00018911", x"0001891B", x"00018925", x"0001892F",
  x"00018939", x"00018943", x"0001894D", x"00018956", x"00018960", x"0001896A", x"00018973", x"0001897C", x"00018986", x"0001898F",
  x"00018998", x"000189A1", x"000189AA", x"000189B3", x"000189BC", x"000189C5", x"000189CD", x"000189D6", x"000189DE", x"000189E7",
  x"000189EF", x"000189F8", x"00018A00", x"00018A08", x"00018A10", x"00018A18"
);

-------------------------------------------------------------------------------------------------
--  ███████╗██╗  ██╗███████╗ ██████╗    ██████╗ ███████╗███████╗██╗███╗   ██╗███████╗███████╗  --
--  ██╔════╝╚██╗██╔╝██╔════╝██╔════╝    ██╔══██╗██╔════╝██╔════╝██║████╗  ██║██╔════╝██╔════╝  --
--  █████╗   ╚███╔╝ █████╗  ██║         ██║  ██║█████╗  █████╗  ██║██╔██╗ ██║█████╗  ███████╗  --
--  ██╔══╝   ██╔██╗ ██╔══╝  ██║         ██║  ██║██╔══╝  ██╔══╝  ██║██║╚██╗██║██╔══╝  ╚════██║  --
--  ███████╗██╔╝ ██╗███████╗╚██████╗    ██████╔╝███████╗██║     ██║██║ ╚████║███████╗███████║  --
--  ╚══════╝╚═╝  ╚═╝╚══════╝ ╚═════╝    ╚═════╝ ╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝╚══════╝╚══════╝  --
-------------------------------------------------------------------------------------------------
--Shared
  constant EXEC_UNIT_INSTR_SET_SIZE   : natural := 52;  -- total number of instructions in the exec unit
  constant LS_UNIT_INSTR_SET_SIZE     : natural := 14;  -- total number of instructions in the ld_str unit
  constant HDC_UNIT_INSTR_SET_SIZE    : natural := 17;  -- total number of instructions in the HDC unit
  constant BRANCHING_INSTR_SET_SIZE   : natural := 3;   -- total number of instructions in the HDC unit
--MCR
  constant MCR_UNIT_INSTR_SET_SIZE    : natural := 28;  -- total number of instructions in the dsp unit
--FHRR
  constant FHRR_UNIT_INSTR_SET_SIZE   : natural := 35;  -- total number of instructions in the dsp unit
  constant FP_UNIT_INSTR_SET_SIZE     : natural := 6;   -- total number of instructions in the dsp unit


  -- EXEC UNIT INSTR SET --------------------------------------------------------------------------------------------------------------------
  constant ADDI_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000000000000000001";
  constant SLTI_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000000000000000010";
  constant SLTIU_pattern   : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000000000000000100";
  constant ANDI_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000000000000001000";
  constant ORI_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000000000000010000";
  constant XORI_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000000000000100000";
  constant SLLI_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000000000001000000";
  constant SRLI7_pattern   : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000000000010000000";
  constant SRAI7_pattern   : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000000000100000000";
  constant LUI_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000000001000000000";
  constant AUIPC_pattern   : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000000010000000000";
  constant ADD7_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000000100000000000";
  constant SUB7_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000001000000000000";
  constant SLT_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000010000000000000";
  constant SLTU_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000000100000000000000";
  constant ANDD_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000001000000000000000";
  constant ORR_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000010000000000000000";
  constant XORR_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000000100000000000000000";
  constant SLLL_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000001000000000000000000";
  constant SRLL7_pattern   : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000010000000000000000000";
  constant SRAA7_pattern   : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000000100000000000000000000";
  constant JAL_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000001000000000000000000000";
  constant JALR_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000010000000000000000000000";
  constant BEQ_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000000100000000000000000000000";
  constant BNE_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000001000000000000000000000000";
  constant BLT_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000010000000000000000000000000";
  constant BLTU_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000100000000000000000000000000";
  constant BGE_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000001000000000000000000000000000";
  constant BGEU_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000010000000000000000000000000000";
  constant FENCE_pattern   : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000100000000000000000000000000000";
  constant FENCEI_pattern  : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000001000000000000000000000000000000";
  constant ECALL_pattern   : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000010000000000000000000000000000000";
  constant EBREAK_pattern  : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000100000000000000000000000000000000";
  constant MRET_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000001000000000000000000000000000000000";
  constant WFI_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000010000000000000000000000000000000000";
  constant CSRRW_pattern   : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000100000000000000000000000000000000000";
  constant CSRRS_pattern   : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000001000000000000000000000000000000000000";
  constant CSRRC_pattern   : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000010000000000000000000000000000000000000";
  constant CSRRWI_pattern  : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000100000000000000000000000000000000000000";
  constant CSRRSI_pattern  : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000001000000000000000000000000000000000000000";
  constant CSRRCI_pattern  : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000010000000000000000000000000000000000000000";
  constant SW_MIP_pattern  : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000100000000000000000000000000000000000000000";
  constant ILL_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000001000000000000000000000000000000000000000000";
  constant NOP_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000010000000000000000000000000000000000000000000";
  constant MUL_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000100000000000000000000000000000000000000000000";
  constant MULH_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000001000000000000000000000000000000000000000000000";
  constant MULHSU_pattern  : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000010000000000000000000000000000000000000000000000";
  constant MULHU_pattern   : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000100000000000000000000000000000000000000000000000";
  constant DIV_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0001000000000000000000000000000000000000000000000000";
  constant DIVU_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0010000000000000000000000000000000000000000000000000";
  constant REM_pattern     : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "0100000000000000000000000000000000000000000000000000";
  constant REMU_pattern    : std_logic_vector(EXEC_UNIT_INSTR_SET_SIZE-1 downto 0) := "1000000000000000000000000000000000000000000000000000";
  -------------------------------------------------------------------------------------------------------------------------------------------

  -- LOAD STORE UNIT INSTR SET ---------------------------------------------------------------------
  constant LW_pattern        : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000001";
  constant LH_pattern        : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000010";
  constant LHU_pattern       : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000100";
  constant LB_pattern        : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000001000";
  constant LBU_pattern       : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000010000";
  constant SW_pattern        : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000100000";
  constant SH_pattern        : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000001000000";
  constant SB_pattern        : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000010000000";
  constant AMOSWAP_pattern   : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000100000000";
  constant HVMEMLD_pattern   : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00001000000000";
  constant HVMEMSTR_pattern  : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00010000000000";
  constant HVBCASTLD_pattern : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00100000000000";
--FHRR & MCR
  constant FLW_pattern       : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000100000000";
  constant FSW_pattern       : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00001000000000";
  constant KMEMLD_pattern    : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "00100000000000";
  constant KMEMSTR_pattern   : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "01000000000000";
  constant KBCASTLD_pattern  : std_logic_vector(LS_UNIT_INSTR_SET_SIZE-1 downto 0) := "10000000000000";
  --------------------------------------------------------------------------------------------------

  -- HDC UNIT INSTR SET ------------------------------------------------------------------------------------
  constant HVBUNDLE_pattern   : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000001";
  constant HVSIM_pattern      : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000010";
  constant HVBIND_pattern     : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000100";
  constant KVRED_pattern      : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000001000";
  constant HVSEARCH_pattern   : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000010000";
  constant KSVADDSC_pattern   : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000100000";
  constant HVCLIP_pattern     : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000001000000";
  constant KSVMULSC_pattern   : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000010000000";
  constant HVPERM_pattern     : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000100000000";
  constant KSRAV_pattern      : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000001000000000";
  constant KSRLV_pattern      : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000010000000000";
  constant KBCAST_pattern     : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000100000000000";
  constant KRELU_pattern      : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00001000000000000";
  constant KDOTPPS_pattern    : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00010000000000000";
  constant KVSLT_pattern      : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00100000000000000";
  constant KSVSLT_pattern     : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "01000000000000000";
  constant KVCP_pattern       : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "10000000000000000";
  ----------------------------------------------------------------------------------------------------------
 -- MCR UNIT INSTR SET ------------------------------------------------------------------------------------
  constant KADDV_pattern_MCR      : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000001";
  constant KSUBV_pattern_MCR      : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000010";
  constant KVMUL_pattern_MCR      : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000000100";
  constant KVRED_pattern_MCR      : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000001000";
  constant KDOTP_pattern_MCR      : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000010000";
  constant KSVADDSC_pattern_MCR   : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000000100000";
  constant KSVADDRF_pattern_MCR   : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000001000000";
  constant KSVMULSC_pattern_MCR   : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000010000000";
  constant KSVMULRF_pattern_MCR   : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000000100000000";
  constant KSRAV_pattern_MCR      : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000001000000000";
  constant KSRLV_pattern_MCR      : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000010000000000";
  constant KBCAST_pattern_MCR     : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000000100000000000";
  constant KRELU_pattern_MCR      : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000001000000000000";
  constant KDOTPPS_pattern_MCR    : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000010000000000000";
  constant KVSLT_pattern_MCR      : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000000100000000000000";
  constant KSVSLT_pattern_MCR     : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000001000000000000000";
  constant KVCP_pattern_MCR       : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000010000000000000000";
  constant KVDIV_pattern_MCR      : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000000100000000000000000";
  constant KSVDIVSC_pattern_MCR   : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000001000000000000000000";
  constant KSVDIVRF_pattern_MCR   : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000010000000000000000000";
  constant KVREM_pattern_MCR      : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000000100000000000000000000";
  constant KSVREMSC_pattern_MCR   : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000001000000000000000000000";
  constant KSVREMRF_pattern_MCR   : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000010000000000000000000000";
  constant KVMULPS_pattern_MCR    : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0000100000000000000000000000";
  constant KSVMULPSSC_pattern_MCR : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0001000000000000000000000000";
  constant KSVMULPSRF_pattern_MCR : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0010000000000000000000000000";
  constant KDOTPS_pattern_MCR     : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "0100000000000000000000000000";
  constant KADDVCLIP_pattern      : std_logic_vector(MCR_UNIT_INSTR_SET_SIZE-1 downto 0) := "1000000000000000000000000000";
  ----------------------------------------------------------------------------------------------------------  
  -- FHRR UNIT INSTR SET ------------------------------------------------------------------------------------
  constant KADDV_pattern_FHRR      : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000000000000000001";
  constant KSUBV_pattern_FHRR      : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000000000000000010";
  constant KVMUL_pattern_FHRR      : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000000000000000100";
  constant KVRED_pattern_FHRR      : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000000000000001000";
  constant KDOTP_pattern_FHRR      : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000000000000010000";
  constant KSVADDSC_pattern_FHRR   : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000000000000100000";
  constant KSVADDRF_pattern_FHRR   : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000000000001000000";
  constant KSVMULSC_pattern_FHRR   : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000000000010000000";
  constant KSVMULRF_pattern_FHRR   : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000000000100000000";
  constant KSRAV_pattern_FHRR      : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000000001000000000";
  constant KSRLV_pattern_FHRR      : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000000010000000000";
  constant KBCAST_pattern_FHRR     : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000000100000000000";
  constant KRELU_pattern_FHRR      : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000001000000000000";
  constant KDOTPPS_pattern_FHRR    : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000010000000000000";
  constant KVSLT_pattern_FHRR      : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000000100000000000000";
  constant KSVSLT_pattern_FHRR     : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000001000000000000000";
  constant KVCP_pattern_FHRR       : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000010000000000000000";
  constant KVDIV_pattern_FHRR      : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000000100000000000000000";
  constant KSVDIVSC_pattern_FHRR   : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000001000000000000000000";
  constant KSVDIVRF_pattern_FHRR   : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000010000000000000000000";
  constant KVREM_pattern_FHRR      : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000100000000000000000000";
  constant KSVREMSC_pattern_FHRR   : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000001000000000000000000000";
  constant KSVREMRF_pattern_FHRR   : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000010000000000000000000000";
  constant KVMULPS_pattern_FHRR    : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000100000000000000000000000";
  constant KSVMULPSSC_pattern_FHRR : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000001000000000000000000000000";
  constant KSVMULPSRF_pattern_FHRR : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000010000000000000000000000000";
  constant KDOTPS_pattern_FHRR     : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000100000000000000000000000000";
  constant HVBUNDLE_pattern_FHRR   : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000001000000000000000000000000000";
  constant HVCLIP_pattern_FHRR     : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000010000000000000000000000000000";
  constant HVBIND_pattern_FHRR     : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000100000000000000000000000000000";
  constant HVPERM_pattern_FHRR     : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00001000000000000000000000000000000";
  constant HVSIM_pattern_FHRR      : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00010000000000000000000000000000000";
  constant HVSEARCH_pattern_FHRR   : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "00100000000000000000000000000000000";
  constant HVENC_pattern           : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "01000000000000000000000000000000000";
  constant HVDEC_pattern           : std_logic_vector(FHRR_UNIT_INSTR_SET_SIZE-1 downto 0) := "10000000000000000000000000000000000";
  ----------------------------------------------------------------------------------------------------------

  -- BRANCHING INSTRUCTIONS in FETCH unit -------------------------------------------------------
  constant JAL_FETCH_pattern    : std_logic_vector(BRANCHING_INSTR_SET_SIZE-1 downto 0) := "001";
  constant JALR_FETCH_pattern   : std_logic_vector(BRANCHING_INSTR_SET_SIZE-1 downto 0) := "010";
  constant BRANCH_FETCH_pattern : std_logic_vector(BRANCHING_INSTR_SET_SIZE-1 downto 0) := "100";
  -----------------------------------------------------------------------------------------------
  
  -- FLOAT UNIT INSTR SET --------------------------------------------------------------------------
  constant FADD_pattern     : std_logic_vector(FP_UNIT_INSTR_SET_SIZE-1 downto 0) := "000001";
  constant FSUB_pattern     : std_logic_vector(FP_UNIT_INSTR_SET_SIZE-1 downto 0) := "000010";
  constant FMUL_pattern     : std_logic_vector(FP_UNIT_INSTR_SET_SIZE-1 downto 0) := "000100";
  constant FDIV_pattern     : std_logic_vector(FP_UNIT_INSTR_SET_SIZE-1 downto 0) := "001000";
  --------------------------------------------------------------------------------------------------

  -- HDC UNIT INSTR SET ------------------------------------------------------------------------------------
  constant KADDV_pattern      : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000001";
  constant KSUBV_pattern      : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000010";
  constant KVMUL_pattern      : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000000100";
  constant KDOTP_pattern      : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000000010000";
  constant KSVADDRF_pattern   : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000001000000";
  constant KSVMULRF_pattern   : std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0) := "00000000100000000";
  
  constant ADDI_bit_position       : natural := 0;
  constant SLTI_bit_position       : natural := 1;
  constant SLTIU_bit_position      : natural := 2;
  constant ANDI_bit_position       : natural := 3;
  constant ORI_bit_position        : natural := 4;
  constant XORI_bit_position       : natural := 5;
  constant SLLI_bit_position       : natural := 6;
  constant SRLI7_bit_position      : natural := 7;
  constant SRAI7_bit_position      : natural := 8;
  constant LUI_bit_position        : natural := 9;
  constant AUIPC_bit_position      : natural := 10;
  constant ADD7_bit_position       : natural := 11;
  constant SUB7_bit_position       : natural := 12;
  constant SLT_bit_position        : natural := 13;
  constant SLTU_bit_position       : natural := 14;
  constant ANDD_bit_position       : natural := 15;
  constant ORR_bit_position        : natural := 16;
  constant XORR_bit_position       : natural := 17;
  constant SLLL_bit_position       : natural := 18;
  constant SRLL7_bit_position      : natural := 19;
  constant SRAA7_bit_position      : natural := 20;
  constant JAL_bit_position        : natural := 21;
  constant JALR_bit_position       : natural := 22;
  constant BEQ_bit_position        : natural := 23;
  constant BNE_bit_position        : natural := 24;
  constant BLT_bit_position        : natural := 25;
  constant BLTU_bit_position       : natural := 26;
  constant BGE_bit_position        : natural := 27;
  constant BGEU_bit_position       : natural := 28;
  constant FENCE_bit_position      : natural := 29;
  constant FENCEI_bit_position     : natural := 30;
  constant ECALL_bit_position      : natural := 31;
  constant EBREAK_bit_position     : natural := 32;
  constant MRET_bit_position       : natural := 33;
  constant WFI_bit_position        : natural := 34;
  constant CSRRW_bit_position      : natural := 35;
  constant CSRRS_bit_position      : natural := 36;
  constant CSRRC_bit_position      : natural := 37;
  constant CSRRWI_bit_position     : natural := 38;
  constant CSRRSI_bit_position     : natural := 39;
  constant CSRRCI_bit_position     : natural := 40;
  constant SW_MIP_bit_position     : natural := 41;
  constant ILL_bit_position        : natural := 42;
  constant NOP_bit_position        : natural := 43;
  constant MUL_bit_position        : natural := 44;
  constant MULH_bit_position       : natural := 45;
  constant MULHSU_bit_position     : natural := 46;
  constant MULHU_bit_position      : natural := 47;
  constant DIV_bit_position        : natural := 48;
  constant DIVU_bit_position       : natural := 49;
  constant REM_bit_position        : natural := 50;
  constant REMU_bit_position       : natural := 51;

  constant LW_bit_position         : natural := 0;
  constant LH_bit_position         : natural := 1;
  constant LHU_bit_position        : natural := 2;
  constant LB_bit_position         : natural := 3;
  constant LBU_bit_position        : natural := 4;
  constant SW_bit_position         : natural := 5;
  constant SH_bit_position         : natural := 6;
  constant SB_bit_position         : natural := 7;
  constant AMOSWAP_bit_position    : natural := 8;
  constant HVMEMLD_bit_position    : natural := 9;
  constant HVMEMSTR_bit_position   : natural := 10;
  constant HVBCASTLD_bit_position  : natural := 11;
 --FHRR & MCR 
  constant FLW_bit_position              : natural := 8;
  constant FSW_bit_position              : natural := 9;
  constant AMOSWAP_bit_position_FHRR     : natural := 10; 
  constant KMEMLD_bit_position           : natural := 11;
  constant KMEMSTR_bit_position          : natural := 12;
  constant KBCASTLD_bit_position         : natural := 13;
  
  constant KADDV_bit_position            : natural := 0;
  constant KSUBV_bit_position            : natural := 1;
  constant KVMUL_bit_position            : natural := 2;
  constant KDOTP_bit_position            : natural := 4;
  constant KSVADDRF_bit_position         : natural := 6;
  constant KSVMULRF_bit_position         : natural := 8;
  constant KVDIV_bit_position            : natural := 17;
  constant KSVDIVSC_bit_position         : natural := 18;
  constant KSVDIVRF_bit_position         : natural := 19;
  constant KVREM_bit_position            : natural := 20;
  constant KSVREMSC_bit_position         : natural := 21;
  constant KSVREMRF_bit_position         : natural := 22;
  constant KVMULPS_bit_position          : natural := 23;
  constant KSVMULPSSC_bit_position       : natural := 24;
  constant KSVMULPSRF_bit_position       : natural := 25;
  constant KDOTPS_bit_position           : natural := 26;
  constant HVBUNDLE_bit_position_FHRR    : natural := 27;
  constant KADDVCLIP_bit_position        : natural := 27;
  constant HVCLIP_bit_position_FHRR      : natural := 28;
  constant HVBIND_bit_position_FHRR      : natural := 29;
  constant HVPERM_bit_position_FHRR      : natural := 30;
  constant HVSIM_bit_position_FHRR       : natural := 31;
  constant HVSEARCH_bit_position_FHRR    : natural := 32;
  constant HVENC_bit_position            : natural := 33;
  constant HVDEC_bit_position            : natural := 34;


  -- HDC UNIT BIT POSITION -----------------------------------------------------------------------------------
  constant HVBUNDLE_bit_position : natural := 0;
  constant HVSIM_bit_position    : natural := 1;
  constant HVBIND_bit_position   : natural := 2;
  constant KVRED_bit_position    : natural := 3;
  constant HVSEARCH_bit_position : natural := 4;
  constant KSVADDSC_bit_position : natural := 5;
  constant HVCLIP_bit_position   : natural := 6;
  constant KSVMULSC_bit_position : natural := 7;
  constant HVPERM_bit_position   : natural := 8;
  constant KSRAV_bit_position    : natural := 9;  
  constant KSRLV_bit_position    : natural := 10;
  constant KBCAST_bit_position   : natural := 11;
  constant KRELU_bit_position    : natural := 12;
  constant KDOTPPS_bit_position  : natural := 13;
  constant KVSLT_bit_position    : natural := 14;
  constant KSVSLT_bit_position   : natural := 15;
  constant KVCP_bit_position     : natural := 16;

  constant JAL_FETCH_instr       : natural := 0;
  constant JALR_FETCH_instr      : natural := 1;
  constant BRANCH_FETCH_instr    : natural := 2;

-----------------------------------------------------------------------------------------
--   ██████╗███████╗██████╗     ██████╗ ███████╗███████╗██╗███╗   ██╗███████╗███████╗  --
--  ██╔════╝██╔════╝██╔══██╗    ██╔══██╗██╔════╝██╔════╝██║████╗  ██║██╔════╝██╔════╝  --
--  ██║     ███████╗██████╔╝    ██║  ██║█████╗  █████╗  ██║██╔██╗ ██║█████╗  ███████╗  --
--  ██║     ╚════██║██╔══██╗    ██║  ██║██╔══╝  ██╔══╝  ██║██║╚██╗██║██╔══╝  ╚════██║  --
--  ╚██████╗███████║██║  ██║    ██████╔╝███████╗██║     ██║██║ ╚████║███████╗███████║  --
--   ╚═════╝╚══════╝╚═╝  ╚═╝    ╚═════╝ ╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝╚══════╝╚══════╝  --
-----------------------------------------------------------------------------------------

  -- CSRs addresses
  constant MSTATUS_addr       : std_logic_vector (11 downto 0) := x"300";
  constant MEPC_addr          : std_logic_vector (11 downto 0) := x"341";
  constant MCAUSE_addr        : std_logic_vector (11 downto 0) := x"342";
  constant MTVEC_addr         : std_logic_vector (11 downto 0) := x"305";
  constant MIP_addr           : std_logic_vector (11 downto 0) := x"344";
  constant PCER_addr          : std_logic_vector (11 downto 0) := x"7A0";
  constant MESTATUS_addr      : std_logic_vector (11 downto 0) := x"7B8";
  constant MCPUID_addr        : std_logic_vector (11 downto 0) := x"F00";
  constant MIMPID_addr        : std_logic_vector (11 downto 0) := x"F01";
  constant MHARTID_addr       : std_logic_vector (11 downto 0) := x"F14";
  constant BADADDR_addr       : std_logic_vector (11 downto 0) := x"343";
  constant MIRQ_addr          : std_logic_vector (11 downto 0) := x"FC0";
  --Performance Counters CSR addresses
  constant MCYCLE_addr        : std_logic_vector (11 downto 0) := x"B00";
  constant MINSTRET_addr      : std_logic_vector (11 downto 0) := x"B02";
  constant MHPMCOUNTER3_addr  : std_logic_vector (11 downto 0) := x"B03";
  constant MHPMCOUNTER4_addr  : std_logic_vector (11 downto 0) := x"B04";
  constant MHPMCOUNTER5_addr  : std_logic_vector (11 downto 0) := x"B05";
  constant MHPMCOUNTER6_addr  : std_logic_vector (11 downto 0) := x"B06";
  constant MHPMCOUNTER7_addr  : std_logic_vector (11 downto 0) := x"B07";
  constant MHPMCOUNTER8_addr  : std_logic_vector (11 downto 0) := x"B08";
  constant MHPMCOUNTER9_addr  : std_logic_vector (11 downto 0) := x"B09";
  constant MHPMCOUNTER10_addr : std_logic_vector (11 downto 0) := x"B0A";
  constant MHPMCOUNTER11_addr : std_logic_vector (11 downto 0) := x"B0B";
  constant MHPMCOUNTER12_addr : std_logic_vector (11 downto 0) := x"B0C";
  constant MHPMCOUNTER13_addr : std_logic_vector (11 downto 0) := x"B0D";
  constant MHPMCOUNTER14_addr : std_logic_vector (11 downto 0) := x"B0E";
  constant MHPMCOUNTER15_addr : std_logic_vector (11 downto 0) := x"B0F";
  constant MHPMCOUNTER16_addr : std_logic_vector (11 downto 0) := x"B10";
  constant MHPMCOUNTER17_addr : std_logic_vector (11 downto 0) := x"B11";
  constant MHPMCOUNTER18_addr : std_logic_vector (11 downto 0) := x"B12";
  constant MHPMCOUNTER19_addr : std_logic_vector (11 downto 0) := x"B13";
  constant MHPMCOUNTER20_addr : std_logic_vector (11 downto 0) := x"B14";
  constant MHPMCOUNTER21_addr : std_logic_vector (11 downto 0) := x"B15";
  constant MHPMCOUNTER22_addr : std_logic_vector (11 downto 0) := x"B16";
  constant MHPMCOUNTER23_addr : std_logic_vector (11 downto 0) := x"B17";
  constant MHPMCOUNTER24_addr : std_logic_vector (11 downto 0) := x"B18";
  constant MHPMCOUNTER25_addr : std_logic_vector (11 downto 0) := x"B19";
  constant MHPMCOUNTER26_addr : std_logic_vector (11 downto 0) := x"B1A";
  constant MHPMCOUNTER27_addr : std_logic_vector (11 downto 0) := x"B1B";
  constant MHPMCOUNTER28_addr : std_logic_vector (11 downto 0) := x"B1C";
  constant MHPMCOUNTER29_addr : std_logic_vector (11 downto 0) := x"B1D";
  constant MHPMCOUNTER30_addr : std_logic_vector (11 downto 0) := x"B1E";
  constant MHPMCOUNTER31_addr : std_logic_vector (11 downto 0) := x"B1F";
  constant MCYCLEH_addr       : std_logic_vector (11 downto 0) := x"B80";
  constant MINSTRETH_addr     : std_logic_vector (11 downto 0) := x"B82";
  constant MHPMEVENT3_addr    : std_logic_vector (11 downto 0) := x"323";
  constant MHPMEVENT4_addr    : std_logic_vector (11 downto 0) := x"324";
  constant MHPMEVENT5_addr    : std_logic_vector (11 downto 0) := x"325";
  constant MHPMEVENT6_addr    : std_logic_vector (11 downto 0) := x"326";
  constant MHPMEVENT7_addr    : std_logic_vector (11 downto 0) := x"327";
  constant MHPMEVENT8_addr    : std_logic_vector (11 downto 0) := x"328";
  constant MHPMEVENT9_addr    : std_logic_vector (11 downto 0) := x"329";
  constant MHPMEVENT10_addr   : std_logic_vector (11 downto 0) := x"32A";
  constant MHPMEVENT11_addr   : std_logic_vector (11 downto 0) := x"32B";
  constant MHPMEVENT12_addr   : std_logic_vector (11 downto 0) := x"32C";
  constant MHPMEVENT13_addr   : std_logic_vector (11 downto 0) := x"32D";
  constant MHPMEVENT14_addr   : std_logic_vector (11 downto 0) := x"32E";
  constant MHPMEVENT15_addr   : std_logic_vector (11 downto 0) := x"32F";
  constant MHPMEVENT16_addr   : std_logic_vector (11 downto 0) := x"330";
  constant MHPMEVENT17_addr   : std_logic_vector (11 downto 0) := x"331";
  constant MHPMEVENT18_addr   : std_logic_vector (11 downto 0) := x"332";
  constant MHPMEVENT19_addr   : std_logic_vector (11 downto 0) := x"333";
  constant MHPMEVENT20_addr   : std_logic_vector (11 downto 0) := x"334";
  constant MHPMEVENT21_addr   : std_logic_vector (11 downto 0) := x"335";
  constant MHPMEVENT22_addr   : std_logic_vector (11 downto 0) := x"336";
  constant MHPMEVENT23_addr   : std_logic_vector (11 downto 0) := x"337";
  constant MHPMEVENT24_addr   : std_logic_vector (11 downto 0) := x"338";
  constant MHPMEVENT25_addr   : std_logic_vector (11 downto 0) := x"339";
  constant MHPMEVENT26_addr   : std_logic_vector (11 downto 0) := x"33A";
  constant MHPMEVENT27_addr   : std_logic_vector (11 downto 0) := x"33B";
  constant MHPMEVENT28_addr   : std_logic_vector (11 downto 0) := x"33C";
  constant MHPMEVENT29_addr   : std_logic_vector (11 downto 0) := x"33D";
  constant MHPMEVENT30_addr   : std_logic_vector (11 downto 0) := x"33E";
  constant MHPMEVENT31_addr   : std_logic_vector (11 downto 0) := x"33F";

  -- HDCU Performance Counters  (custom, read/optional-write)
  constant HDCU_CYCLES_addr   : std_logic_vector (11 downto 0) := x"BD0"; 
  constant HDCU_BIND_addr     : std_logic_vector (11 downto 0) := x"BD1";
  constant HDCU_BUNDLE_addr   : std_logic_vector (11 downto 0) := x"BD2"; 
  constant HDCU_SIM_addr      : std_logic_vector (11 downto 0) := x"BD3"; 
  constant HDCU_CLIP_addr     : std_logic_vector (11 downto 0) := x"BD4"; 
  constant HDCU_PERM_addr     : std_logic_vector (11 downto 0) := x"BD5"; 
  constant HDCU_SEARCH_addr   : std_logic_vector (11 downto 0) := x"BD6"; 
  constant HDCU_MCYCLE_addr   : std_logic_vector (11 downto 0) := x"BD7";
--FHRR
  constant HDCU_ENC_addr      : std_logic_vector (11 downto 0) := x"BD8";
  -----------------------------------------------------------------------

  -- Custom Klessydra CSR addresses
  constant MPSCLFAC_addr      : std_logic_vector (11 downto 0) := x"BE0";  -- custom CSR registers
  constant HVSIZE_addr        : std_logic_vector (11 downto 0) := x"BF0";  -- custom CSR registers
  constant MVTYPE_addr        : std_logic_vector (11 downto 0) := x"BF8";  -- custom CSR registers
  constant MBHARTID_addr      : std_logic_vector (11 downto 0) := x"FC4";  -- custom CSR registers
  constant MPIP_addr          : std_logic_vector (11 downto 0) := x"FC8";  -- custom CSR registers

  -- reset values of CSR Registers
  constant MTVEC_RESET_VALUE    : std_logic_vector(31 downto 0)          := x"00000094";
  constant PCER_RESET_VALUE     : std_logic_vector(31 downto 0)          := x"00000000";
  constant MSTATUS_RESET_VALUE  : std_logic_vector(1 downto 0)           := "00";
  constant MESTATUS_RESET_VALUE : std_logic_vector(2 downto 0)           := "000";
  constant MEPC_RESET_VALUE     : std_logic_vector(31 downto 0)          := x"00000000";  -- label to decode the kind of misaligned access
  constant MCAUSE_RESET_VALUE   : std_logic_vector(31 downto 0)          := x"00000000";
  constant MIP_RESET_VALUE      : std_logic_vector(31 downto 0)          := x"00000000";
  --constant HVSIZE_RESET_VALUE   : std_logic_vector(Addr_Width downto 0)  := (1 to Addr_Width => '0') & '1';
  constant MVTYPE_RESET_VALUE   : std_logic_vector(3 downto 0)           := "1000";
  constant MPSCLFAC_RESET_VALUE : std_logic_vector(4 downto 0)           := (others => '0');							  

  -- csr bits for instructions SYSTEM -> CSRRS
  --constant RDCYCLE    : std_logic_vector(11 downto 0) := "110000000000";
  --constant RDCYCLEH   : std_logic_vector(11 downto 0) := "110010000000";
  --constant RDTIME     : std_logic_vector(11 downto 0) := "110000000001";
  --constant RDTIMEH    : std_logic_vector(11 downto 0) := "110010000001";
  --constant RDINSTRET  : std_logic_vector(11 downto 0) := "110000000010";
  --constant RDINSTRETH : std_logic_vector(11 downto 0) := "110010000010";

-------------------------------------------------------------------------------------------------------------------------
--  ██████╗ ███████╗ ██████╗ ██████╗ ██████╗ ███████╗██████╗     ██████╗ ███████╗███████╗██╗███╗   ██╗███████╗███████╗ --
--  ██╔══██╗██╔════╝██╔════╝██╔═══██╗██╔══██╗██╔════╝██╔══██╗    ██╔══██╗██╔════╝██╔════╝██║████╗  ██║██╔════╝██╔════╝ --
--  ██║  ██║█████╗  ██║     ██║   ██║██║  ██║█████╗  ██████╔╝    ██║  ██║█████╗  █████╗  ██║██╔██╗ ██║█████╗  ███████╗ --
--  ██║  ██║██╔══╝  ██║     ██║   ██║██║  ██║██╔══╝  ██╔══██╗    ██║  ██║██╔══╝  ██╔══╝  ██║██║╚██╗██║██╔══╝  ╚════██║ --
--  ██████╔╝███████╗╚██████╗╚██████╔╝██████╔╝███████╗██║  ██║    ██████╔╝███████╗██║     ██║██║ ╚████║███████╗███████║ --
--  ╚═════╝ ╚══════╝ ╚═════╝ ╚═════╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝    ╚═════╝ ╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝╚══════╝╚══════╝ --
-------------------------------------------------------------------------------------------------------------------------                          

  -- opcodes
  constant OP_IMM   : std_logic_vector(6 downto 0) := "0010011";
  constant LUI      : std_logic_vector(6 downto 0) := "0110111";
  constant AUIPC    : std_logic_vector(6 downto 0) := "0010111";
  constant OP       : std_logic_vector(6 downto 0) := "0110011";
  constant JAL      : std_logic_vector(6 downto 0) := "1101111";
  constant JALR     : std_logic_vector(6 downto 0) := "1100111";
  constant BRANCH   : std_logic_vector(6 downto 0) := "1100011";
  constant LOAD     : std_logic_vector(6 downto 0) := "0000011";
  constant STORE    : std_logic_vector(6 downto 0) := "0100011";
  constant MISC_MEM : std_logic_vector(6 downto 0) := "0001111";
  constant SYSTEM   : std_logic_vector(6 downto 0) := "1110011";
  constant AMO      : std_logic_vector(6 downto 0) := "0101111";
  constant LOAD_F   : std_logic_vector(6 downto 0) := "0000111";
  constant STORE_F  : std_logic_vector(6 downto 0) := "0100111";
  constant FLOAT    : std_logic_vector(6 downto 0) := "1010011";
  constant HVMEM    : std_logic_vector(6 downto 0) := "0001011";
  constant HDCU     : std_logic_vector(6 downto 0) := "1011011";  -- custom-2 (0x5B), was custom-1 0x2B; BUG4 fix [20260728]

  -- funct3 bits of OP_IMM opcode
  constant ADDI      : std_logic_vector(2 downto 0) := "000";
  constant SLTI      : std_logic_vector(2 downto 0) := "010";
  constant SLTIU     : std_logic_vector(2 downto 0) := "011";
  constant ANDI      : std_logic_vector(2 downto 0) := "111";
  constant ORI       : std_logic_vector(2 downto 0) := "110";
  constant XORI      : std_logic_vector(2 downto 0) := "100";
  constant SLLI      : std_logic_vector(2 downto 0) := "001";
  constant SRLI_SRAI : std_logic_vector(2 downto 0) := "101";

  -- funct3 bits of OP opcode -- chiedere conferma su xorr-orr-andd ecc ecc
  constant ADD     : std_logic_vector(2 downto 0) := "000";
  constant SLT     : std_logic_vector(2 downto 0) := "010";
  constant SLTU    : std_logic_vector(2 downto 0) := "011";
  constant ANDD    : std_logic_vector(2 downto 0) := "111";
  constant ORR     : std_logic_vector(2 downto 0) := "110";
  constant XORR    : std_logic_vector(2 downto 0) := "100";
  constant SLLL    : std_logic_vector(2 downto 0) := "001";
  constant SRLL    : std_logic_vector(2 downto 0) := "101";

  -- funct3 bits of OP opcode -- chiedere conferma su xorr-orr-andd ecc ecc
  constant SUB7  : std_logic_vector(2 downto 0) := "000";
  constant SRAA  : std_logic_vector(2 downto 0) := "101";

  -- funct3 bits of OP opcode -- chiedere conferma su xorr-orr-andd ecc ecc
  constant MUL     : std_logic_vector(2 downto 0) := "000";
  constant MULH    : std_logic_vector(2 downto 0) := "001";
  constant MULHSU  : std_logic_vector(2 downto 0) := "010";
  constant MULHU   : std_logic_vector(2 downto 0) := "011";
  constant DIV     : std_logic_vector(2 downto 0) := "100";
  constant DIVU    : std_logic_vector(2 downto 0) := "101";
  constant REMD    : std_logic_vector(2 downto 0) := "110";
  constant REMDU   : std_logic_vector(2 downto 0) := "111";

  -- funct3 bits of BRANCH opcode
  constant BEQ  : std_logic_vector(2 downto 0) := "000";
  constant BNE  : std_logic_vector(2 downto 0) := "001";
  constant BLT  : std_logic_vector(2 downto 0) := "100";
  constant BGE  : std_logic_vector(2 downto 0) := "101";
  constant BLTU : std_logic_vector(2 downto 0) := "110";
  constant BGEU : std_logic_vector(2 downto 0) := "111";

  -- funct3 bits of LOAD opcode
  constant LW  : std_logic_vector(2 downto 0) := "010";
  constant LH  : std_logic_vector(2 downto 0) := "001";
  constant LHU : std_logic_vector(2 downto 0) := "101";
  constant LB  : std_logic_vector(2 downto 0) := "000";
  constant LBU : std_logic_vector(2 downto 0) := "100";

  -- funct3 bits of STORE opcode
  constant SW : std_logic_vector(2 downto 0) := "010";
  constant SH : std_logic_vector(2 downto 0) := "001";
  constant SB : std_logic_vector(2 downto 0) := "000";

  -- funct3 bits of MISC_MEM opcode
  constant FENCE  : std_logic_vector(2 downto 0) := "000";
  constant FENCEI : std_logic_vector(2 downto 0) := "001";

  -- funct3 bits of AMO opcode
  constant SINGLE : std_logic_vector(2 downto 0) := "010";

  -- funct7 bits of FLOAT opcode
  constant FADD     : std_logic_vector(6 downto 0) := "0000000";
  constant FSUB     : std_logic_vector(6 downto 0) := "0000100";
  constant FMUL     : std_logic_vector(6 downto 0) := "0001000";
  constant FDIV     : std_logic_vector(6 downto 0) := "0001100";

  -- funct3 bits of KLESS opcode
  constant KARITH8     : std_logic_vector(2 downto 0) := "000";
  constant KARITH16    : std_logic_vector(2 downto 0) := "001";
  constant KARITH32    : std_logic_vector(2 downto 0) := "010";

  -- instructions to access CSRs
  -- funct3 bits of SYSTEM opcode:
  constant PRIV   : std_logic_vector(2 downto 0) := "000";
  constant CSRRW  : std_logic_vector(2 downto 0) := "001";
  constant CSRRS  : std_logic_vector(2 downto 0) := "010";
  constant CSRRC  : std_logic_vector(2 downto 0) := "011";
  constant CSRRWI : std_logic_vector(2 downto 0) := "101";
  constant CSRRSI : std_logic_vector(2 downto 0) := "110";
  constant CSRRCI : std_logic_vector(2 downto 0) := "111";

  --funct5 bits of AMO opcode
  constant LRW     : std_logic_vector(4 downto 0) := "00010";
  constant SCW     : std_logic_vector(4 downto 0) := "00011";
  constant AMOSWAP : std_logic_vector(4 downto 0) := "00001";
  constant AMOADD  : std_logic_vector(4 downto 0) := "00000";
  constant AMOXOR  : std_logic_vector(4 downto 0) := "00100";
  constant AMOAND  : std_logic_vector(4 downto 0) := "01100";
  constant AMOOR   : std_logic_vector(4 downto 0) := "01000";
  constant AMOMIN  : std_logic_vector(4 downto 0) := "10000";
  constant AMOMAX  : std_logic_vector(4 downto 0) := "10100";
  constant AMOMINU : std_logic_vector(4 downto 0) := "11000";
  constant AMOMAXU : std_logic_vector(4 downto 0) := "11100";

  -- funct7 bits
  constant SRLI7  : std_logic_vector(6 downto 0) := "0000000";
  constant SRAI7  : std_logic_vector(6 downto 0) := "0100000";
  constant OP_I1  : std_logic_vector(6 downto 0) := "0000000";
  constant OP_I2  : std_logic_vector(6 downto 0) := "0100000";
  constant OP_M   : std_logic_vector(6 downto 0) := "0000001";

  --funct7 bits for KMEM Instr
  constant KMEMLD   : std_logic_vector(6 downto 0) := "0000000";
  constant KMEMSTR  : std_logic_vector(6 downto 0) := "0000001";
  constant KBCASTLD : std_logic_vector(6 downto 0) := "0000010";

  --funct7 bits for HVMEM Instr
  constant HVMEMLD   : std_logic_vector(6 downto 0) := "0000000";
  constant HVMEMSTR  : std_logic_vector(6 downto 0) := "0000001";
  constant HVBCASTLD : std_logic_vector(6 downto 0) := "0000010";

  --funct7 bits for KREG Instr
  constant HVBUNDLE   : std_logic_vector(6 downto 0) := "0101011";  -- 0x2B, BUG4 fix [20260728] (was 0x01)
  constant HVSIM      : std_logic_vector(6 downto 0) := "0101111";  -- 0x2F, BUG4 fix [20260728] (was 0x02)
  constant HVBIND     : std_logic_vector(6 downto 0):=  "0101101";  -- 0x2D, BUG4 fix [20260728] (was 0x04)
  constant HVCLIP     : std_logic_vector(6 downto 0) := "0101100";  -- 0x2C, BUG4 fix [20260728] (was 0x0D)
  constant HVPERM     : std_logic_vector(6 downto 0) := "0101110";  -- 0x2E, BUG4 fix [20260728] (was 0x0F)
  constant KADDV      : std_logic_vector(6 downto 0) := "0000001";
  constant KSUBV      : std_logic_vector(6 downto 0) := "0000010";
  constant KVMUL      : std_logic_vector(6 downto 0) := "0000100";
  constant KSVADDRF   : std_logic_vector(6 downto 0) := "0001101";
  constant KSVMULRF   : std_logic_vector(6 downto 0) := "0001111";
  constant KVDIV      : std_logic_vector(6 downto 0) := "0100000";
  constant KSVDIVSC   : std_logic_vector(6 downto 0) := "0100001";
  constant KSVDIVRF   : std_logic_vector(6 downto 0) := "0100010";
  constant KVREM      : std_logic_vector(6 downto 0) := "0100011";
  constant KSVREMSC   : std_logic_vector(6 downto 0) := "0100100";
  constant KSVREMRF   : std_logic_vector(6 downto 0) := "0100101";
  constant KVMULPS    : std_logic_vector(6 downto 0) := "0100111";
  constant KSVMULPSSC : std_logic_vector(6 downto 0) := "0101000";
  constant KSVMULPSRF : std_logic_vector(6 downto 0) := "0101001";
  constant KDOTPS     : std_logic_vector(6 downto 0) := "0101010";
  constant KADDVCLIP  : std_logic_vector(6 downto 0) := "0101011";
  constant HVSEARCH   : std_logic_vector(6 downto 0) := "0110000";
  constant HVENC      : std_logic_vector(6 downto 0) := "0110001";
  constant HVDEC      : std_logic_vector(6 downto 0) := "0110010";
  constant KVRED      : std_logic_vector(6 downto 0) := "0000110";
  constant KDOTP      : std_logic_vector(6 downto 0) := "0001000";
  constant KSVADDSC   : std_logic_vector(6 downto 0) := "0001100";
  constant KSVMULSC   : std_logic_vector(6 downto 0) := "0001110";
  constant KSRAV      : std_logic_vector(6 downto 0) := "0010000";
  constant KSRLV      : std_logic_vector(6 downto 0) := "0010001";
  constant KRELU      : std_logic_vector(6 downto 0) := "0011000";
  constant KDOTPPS    : std_logic_vector(6 downto 0) := "0011001";
  constant KVSLT      : std_logic_vector(6 downto 0) := "0011010";
  constant KSVSLT     : std_logic_vector(6 downto 0) := "0011100";
  constant KBCAST     : std_logic_vector(6 downto 0) := "0011110";
  constant KVCP       : std_logic_vector(6 downto 0) := "0011111";

  -- instr. to change privilege level & interrupt-management instruction
  -- funct12 bits for instructions SYSTEM -> PRIV:
  constant ECALL  : std_logic_vector(11 downto 0) := "000000000000";
  constant EBREAK : std_logic_vector(11 downto 0) := "000000000001";
  constant MRET   : std_logic_vector(11 downto 0) := "001100000010";
  constant WFI    : std_logic_vector(11 downto 0) := "000100000101";

----------------------------------------------------------------------------------------------------------------------------
--  ███████╗██╗  ██╗ ██████╗███████╗██████╗ ████████╗██╗ ██████╗ ███╗   ██╗     ██████╗ ██████╗ ██████╗ ███████╗███████╗  --
--  ██╔════╝╚██╗██╔╝██╔════╝██╔════╝██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║    ██╔════╝██╔═══██╗██╔══██╗██╔════╝██╔════╝  --
--  █████╗   ╚███╔╝ ██║     █████╗  ██████╔╝   ██║   ██║██║   ██║██╔██╗ ██║    ██║     ██║   ██║██║  ██║█████╗  ███████╗  --
--  ██╔══╝   ██╔██╗ ██║     ██╔══╝  ██╔═══╝    ██║   ██║██║   ██║██║╚██╗██║    ██║     ██║   ██║██║  ██║██╔══╝  ╚════██║  --
--  ███████╗██╔╝ ██╗╚██████╗███████╗██║        ██║   ██║╚██████╔╝██║ ╚████║    ╚██████╗╚██████╔╝██████╔╝███████╗███████║  --
--  ╚══════╝╚═╝  ╚═╝ ╚═════╝╚══════╝╚═╝        ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝     ╚═════╝ ╚═════╝ ╚═════╝ ╚══════╝╚══════╝  --
----------------------------------------------------------------------------------------------------------------------------


  -- exception codes (riscv mcause register priv isa 1.10)
  constant ILLEGAL_INSN_EXCEPT_CODE          : std_logic_vector(31 downto 0) := x"00000002";
  constant LOAD_ERROR_EXCEPT_CODE            : std_logic_vector(31 downto 0) := x"00000005";
  constant STORE_ERROR_EXCEPT_CODE           : std_logic_vector(31 downto 0) := x"00000007";
  constant ECALL_EXCEPT_CODE                 : std_logic_vector(31 downto 0) := x"0000000B";
  constant LOAD_MISALIGNED_EXCEPT_CODE       : std_logic_vector(31 downto 0) := x"00000004";
  constant STORE_MISALIGNED_EXCEPT_CODE      : std_logic_vector(31 downto 0) := x"00000006";
  constant ILLEGAL_VECTOR_SIZE_EXCEPT_CODE   : std_logic_vector(31 downto 0) := x"00000100"; -- Custom codes
  constant ILLEGAL_ADDRESS_EXCEPT_CODE       : std_logic_vector(31 downto 0) := x"00000101"; -- Custom codes
  constant SCRATCHPAD_OVERFLOW_EXCEPT_CODE   : std_logic_vector(31 downto 0) := x"00000102"; -- Custom codes
  constant READ_SAME_SCARTCHPAD_EXCEPT_CODE  : std_logic_vector(31 downto 0) := x"00000103"; -- Custom codes
  constant WRITE_SAME_SCARTCHPAD_EXCEPT_CODE : std_logic_vector(31 downto 0) := x"00000104"; -- Custom codes
  constant CTX_SWITCH_CODE                   : std_logic_vector(31 downto 0) := x"00000110"; -- Custom codes

----------------------------------------------------------------------------------
--  ███████╗██╗   ██╗███╗   ██╗ ██████╗████████╗██╗ ██████╗ ███╗   ██╗███████╗  --
--  ██╔════╝██║   ██║████╗  ██║██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║██╔════╝  --
--  █████╗  ██║   ██║██╔██╗ ██║██║        ██║   ██║██║   ██║██╔██╗ ██║███████╗  --
--  ██╔══╝  ██║   ██║██║╚██╗██║██║        ██║   ██║██║   ██║██║╚██╗██║╚════██║  --
--  ██║     ╚██████╔╝██║ ╚████║╚██████╗   ██║   ██║╚██████╔╝██║ ╚████║███████║  --
--  ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝  --
----------------------------------------------------------------------------------                                                                        

  -- functions --

  --function aq(signal instr : in std_logic_vector(31 downto 0)) return std_logic;
  --function rl(signal instr : in std_logic_vector(31 downto 0)) return std_logic;

--  function rs1(signal instr : in std_logic_vector(31 downto 0)) return integer;
--  function rs2(signal instr : in std_logic_vector(31 downto 0)) return integer;
--  function rd(signal instr  : in std_logic_vector(31 downto 0)) return integer;

  function or_vect_bits(input_vector : in std_logic_vector)              return std_logic;
  function I_immediate(signal instr  : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function S_immediate(signal instr  : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function B_immediate(signal instr  : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function U_immediate(signal instr  : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function UJ_immediate(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function I_imm(signal instr        : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function S_imm(signal instr        : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function B_imm(signal instr        : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function U_imm(signal instr        : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function UJ_imm(signal instr       : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function shamt(signal instr        : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function OPCODE(signal instr       : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function FUNCT3(signal instr       : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function FUNCT7(signal instr       : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function FUNCT12(signal instr      : in std_logic_vector(31 downto 0)) return std_logic_vector;
  function CSR_ADDR(signal instr     : in std_logic_vector(31 downto 0)) return std_logic_vector;

end package;

package body riscv_klessydra is

  --function aq (signal instr : in std_logic_vector(31 downto 0)) return std_logic is
  --begin
  --  return instr(26);
  --end;

  --function rl (signal instr : in std_logic_vector(31 downto 0)) return std_logic is
  --begin
  --  return instr(25);
  --end;

  --function rs1 (signal instr : in std_logic_vector(31 downto 0)) return integer is
  --begin
  --  return to_integer(unsigned(instr(15+(RF_CEIL-1) downto 15)));
  --end;

  --function rs2 (signal instr : in std_logic_vector(31 downto 0)) return integer is
  --begin
  --  return to_integer(unsigned(instr(20+(RF_CEIL-1) downto 20)));
  --end;

  --function rd (signal instr : in std_logic_vector(31 downto 0)) return integer is
  --begin
  --  return to_integer(unsigned(instr(11+(RF_CEIL-1) downto 7)));
  --end;
  
  function or_vect_bits(input_vector : std_logic_vector) return std_logic is
    variable result : std_logic := '0';
  begin
    for i in input_vector'range loop
      result := result or input_vector(i);
    end loop;
    return result;
  end function or_vect_bits;

  function I_immediate(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return std_logic_vector(resize(signed(instr(31) & instr(30 downto 20)), 32));
  end;

  function S_immediate(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return std_logic_vector(resize(signed(instr(31 downto 25) & instr(11 downto 8) & instr(7)), 32));
  end;

  function B_immediate(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return std_logic_vector(resize(signed(instr(31) & instr(7) & instr(30 downto 25)
                                          & instr(11 downto 8) & '0'), 32));
  end;

  function U_immediate(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return instr(31 downto 12) & std_logic_vector(to_unsigned(0, 12));
  end;

  function UJ_immediate(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return std_logic_vector(resize(signed(instr(31) & instr(19 downto 12) & instr(20)
                                          & instr(30 downto 21) & '0'), 32));
  end;

  function I_imm(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return std_logic_vector(signed(instr(31) & instr(30 downto 20)));
  end;

  function S_imm(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return std_logic_vector(signed(instr(31 downto 25) & instr(11 downto 8) & instr(7)));
  end;

  function B_imm(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return std_logic_vector(signed(instr(31) & instr(7) & instr(30 downto 25)
                                          & instr(11 downto 8) & '0'));
  end;

  function U_imm(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return instr(31 downto 12);
  end;

  function UJ_imm(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return std_logic_vector(signed(instr(31) & instr(19 downto 12) & instr(20)
                                          & instr(30 downto 21) & '0'));
  end;

  function shamt(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return instr(24 downto 20);
  end;

  function OPCODE(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return instr(6 downto 0);
  end;

  function FUNCT3(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return instr(14 downto 12);
  end;

  function FUNCT7(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return instr(31 downto 25);
  end;

  function FUNCT12(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return instr(31 downto 20);
  end;

  function CSR_ADDR(signal instr : in std_logic_vector(31 downto 0)) return std_logic_vector is
  begin
    return instr(31 downto 20);
  end;

end package body;
                                    