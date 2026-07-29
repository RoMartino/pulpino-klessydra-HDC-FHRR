proc fhrr_core_root {} {
  return "sim:/tb/top_i/core_region_i/CORE/RISCV_CORE"
}

proc fhrr_pipe_root {} {
  return "[fhrr_core_root]/MORPH_inst/Pipe"
}

proc fhrr_csr_root {} {
  return "[fhrr_core_root]/MORPH_inst/CSR"
}

proc fhrr_hdc_root {} {
  return "[fhrr_core_root]/ACCL_generate/VCU_inst/FHRR_ACCEL/HDC"
}

proc fhrr_safe_add_wave {group path} {
  catch {add wave -group $group -radix hexadecimal $path}
}

proc fhrr_safe_examine {path} {
  if {[catch {examine -radix hexadecimal $path} value]} {
    return "NA"
  }
  return $value
}

proc fhrr_env_or_default {name default_value} {
  if {[info exists ::env($name)] && $::env($name) ne ""} {
    return $::env($name)
  }
  return $default_value
}

proc fhrr_value_is_known_hex {value} {
  set trimmed [string trim $value]
  if {$trimmed eq "" || $trimmed eq "NA"} {
    return 0
  }
  set normalized [string tolower $trimmed]
  if {[string match "0x*" $normalized]} {
    set normalized [string range $normalized 2 end]
  }
  if {$normalized eq ""} {
    return 0
  }
  return [expr {![regexp {[^0-9a-f]} $normalized]}]
}

proc fhrr_value_is_clean_nonzero {value} {
  if {![fhrr_value_is_known_hex $value]} {
    return 0
  }

  set parsed 0
  scan $value %x parsed
  return [expr {$parsed != 0}]
}

proc fhrr_debug_add_common_waves {} {
  set pipe [fhrr_pipe_root]
  set csr  [fhrr_csr_root]
  set hdc  [fhrr_hdc_root]

  fhrr_safe_add_wave "FHRR/Common" "$pipe/pc_IE"
  fhrr_safe_add_wave "FHRR/Common" "$pipe/instr_word_IE"
  fhrr_safe_add_wave "FHRR/Common" "$pipe/decoded_instruction_IE"
  fhrr_safe_add_wave "FHRR/Common" "$csr/MCAUSE"
  fhrr_safe_add_wave "FHRR/Common" "$csr/MEPC"
  fhrr_safe_add_wave "FHRR/Common" "$csr/MVSIZE"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/state_HDC(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/nextstate_HDC(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/hdc_instr_req(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/busy_HDC_internal(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/busy_HDC_internal_lat(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/HVSIZE_READ(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/HVSIZE_READ_lat(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/HVSIZE_WRITE(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/hdc_data_gnt_i(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/hdc_sci_req(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/hdc_sci_we(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/hdc_sc_read_addr(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/hdc_sc_write_addr(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/hdc_sc_data_read(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/hdc_sc_data_write_wire(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/wb_ready(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/halt_hdc(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/halt_hdc_lat(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/recover_state(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/recover_state_wires(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/RS1_Data_IE_lat(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/RS2_Data_IE_lat(0)"
  fhrr_safe_add_wave "FHRR/Common" "$hdc/RD_Data_IE_lat(0)"
}

proc fhrr_debug_add_bundle_waves {} {
  set hdc [fhrr_hdc_root]
  fhrr_safe_add_wave "FHRR/Bundle" "$hdc/bundle_en(0)"
  fhrr_safe_add_wave "FHRR/Bundle" "$hdc/bundle_stage_1_en(0)"
  fhrr_safe_add_wave "FHRR/Bundle" "$hdc/bundle_stage_2_en(0)"
  fhrr_safe_add_wave "FHRR/Bundle" "$hdc/hdcu_in_bundled(0)"
  fhrr_safe_add_wave "FHRR/Bundle" "$hdc/hdcu_in_bundle_operand(0)"
  fhrr_safe_add_wave "FHRR/Bundle" "$hdc/hdcu_in_bundle_operand_real(0)"
  fhrr_safe_add_wave "FHRR/Bundle" "$hdc/hdcu_in_bundle_operand_imag(0)"
  fhrr_safe_add_wave "FHRR/Bundle" "$hdc/real_accum(0)"
  fhrr_safe_add_wave "FHRR/Bundle" "$hdc/imag_accum(0)"
}

proc fhrr_debug_add_bind_waves {} {
  set hdc [fhrr_hdc_root]
  fhrr_safe_add_wave "FHRR/Bind" "$hdc/bind_en(0)"
  fhrr_safe_add_wave "FHRR/Bind" "$hdc/bind_stage_1_en(0)"
  fhrr_safe_add_wave "FHRR/Bind" "$hdc/bind_stage_2_en(0)"
  fhrr_safe_add_wave "FHRR/Bind" "$hdc/bind_stage_3_en(0)"
  fhrr_safe_add_wave "FHRR/Bind" "$hdc/hdcu_in_bind_operands(0)"
  fhrr_safe_add_wave "FHRR/Bind" "$hdc/hdcu_in_bind_operands_lat(0)"
  fhrr_safe_add_wave "FHRR/Bind" "$hdc/hdc_out_bind_results_wire(0)"
  fhrr_safe_add_wave "FHRR/Bind" "$hdc/hdc_out_bind_results(0)"
}

proc fhrr_debug_add_encode_waves {} {
  set hdc [fhrr_hdc_root]
  fhrr_safe_add_wave "FHRR/Encode" "$hdc/enc_en(0)"
  fhrr_safe_add_wave "FHRR/Encode" "$hdc/enc_stage_1_en(0)"
  fhrr_safe_add_wave "FHRR/Encode" "$hdc/enc_stage_2_en(0)"
  fhrr_safe_add_wave "FHRR/Encode" "$hdc/enc_stage_3_en(0)"
  fhrr_safe_add_wave "FHRR/Encode" "$hdc/hdcu_in_enc_fpp_operands(0)"
  fhrr_safe_add_wave "FHRR/Encode" "$hdc/mult_result(0)"
  fhrr_safe_add_wave "FHRR/Encode" "$hdc/trunc_mul_results(0)"
  fhrr_safe_add_wave "FHRR/Encode" "$hdc/accumulator_reg(0)"
  fhrr_safe_add_wave "FHRR/Encode" "$hdc/accumulator_wire(0)"
  fhrr_safe_add_wave "FHRR/Encode" "$hdc/hdc_out_enc_result(0)"
}

proc fhrr_debug_add_similarity_waves {} {
  set hdc [fhrr_hdc_root]
  fhrr_safe_add_wave "FHRR/Similarity" "$hdc/sim_en(0)"
  fhrr_safe_add_wave "FHRR/Similarity" "$hdc/sim_stage_1_en(0)"
  fhrr_safe_add_wave "FHRR/Similarity" "$hdc/sim_stage_2_en(0)"
  fhrr_safe_add_wave "FHRR/Similarity" "$hdc/hdcu_in_sim_operands(0)"
  fhrr_safe_add_wave "FHRR/Similarity" "$hdc/delta(0)"
  fhrr_safe_add_wave "FHRR/Similarity" "$hdc/cosine_diff(0)"
  fhrr_safe_add_wave "FHRR/Similarity" "$hdc/cosine_accumulator_reg(0)"
  fhrr_safe_add_wave "FHRR/Similarity" "$hdc/cosine_accumulator_wire(0)"
  fhrr_safe_add_wave "FHRR/Similarity" "$hdc/cosine_accumulator_avg(0)"
}

proc fhrr_debug_add_clip_waves {} {
  set hdc [fhrr_hdc_root]
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/clip_en(0)"
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/clip_stage_1_en(0)"
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/clip_stage_2_en(0)"
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/clip_stage_3_en(0)"
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/hdcu_in_clip_operands(0)"
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/dividend(0)"
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/divisor(0)"
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/div_enable(0)"
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/div_done(0)"
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/div_result(0)"
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/tangent(0)"
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/tang_map(0)"
  fhrr_safe_add_wave "FHRR/Clip" "$hdc/hdcu_out_clip_results(0)"
}

proc fhrr_debug_setup {test_name} {
  set normalized [string tolower $test_name]
  fhrr_debug_add_common_waves

  if {[string match "*bind*" $normalized]} {
    fhrr_debug_add_bind_waves
  }
  if {[string match "*bundle*" $normalized]} {
    fhrr_debug_add_bundle_waves
  }
  if {[string match "*encode*" $normalized]} {
    fhrr_debug_add_encode_waves
  }
  if {[string match "*similarity*" $normalized]} {
    fhrr_debug_add_similarity_waves
  }
  if {[string match "*clip*" $normalized]} {
    fhrr_debug_add_clip_waves
  }

  catch {configure wave -namecolwidth 220}
  catch {configure wave -valuecolwidth 180}
  catch {configure wave -justifyvalue left}
  catch {configure wave -timelineunits ns}
  catch {WaveRestoreZoom {0 ns} {1000 ns}}

  puts "FHRR debug waves ready for $test_name"
}

proc fhrr_debug_setup_from_env {} {
  if {[info exists ::env(FHRR_DEBUG_TEST)]} {
    fhrr_debug_setup $::env(FHRR_DEBUG_TEST)
  } else {
    fhrr_debug_setup "fhrr"
  }
}

proc fhrr_debug_watch {iterations step_ns} {
  set pipe [fhrr_pipe_root]
  set csr  [fhrr_csr_root]
  set hdc  [fhrr_hdc_root]

  puts "FHRR debug watch: iterations=$iterations step_ns=$step_ns"
  for {set iter 0} {$iter < $iterations} {incr iter} {
    run ${step_ns}ns
    set line "step=$iter"
    append line " pc=[fhrr_safe_examine "$pipe/pc_IE"]"
    append line " instr=[fhrr_safe_examine "$pipe/instr_word_IE"]"
    append line " state=[fhrr_safe_examine "$hdc/state_HDC(0)"]"
    append line " hvr=[fhrr_safe_examine "$hdc/HVSIZE_READ(0)"]"
    append line " hvw=[fhrr_safe_examine "$hdc/HVSIZE_WRITE(0)"]"
    append line " wb=[fhrr_safe_examine "$hdc/wb_ready(0)"]"
    append line " halt=[fhrr_safe_examine "$hdc/halt_hdc(0)"]"
    append line " rec=[fhrr_safe_examine "$hdc/recover_state(0)"]"
    append line " mcause=[fhrr_safe_examine "$csr/MCAUSE"]"
    append line " mepc=[fhrr_safe_examine "$csr/MEPC"]"
    puts $line
  }
}

proc fhrr_debug_warmup_and_watch {warmup_ns iterations step_ns} {
  if {$warmup_ns > 0} {
    puts "FHRR debug warmup: warmup_ns=$warmup_ns"
    run ${warmup_ns}ns
  }
  fhrr_debug_watch $iterations $step_ns
}

proc fhrr_debug_watch_progress {iterations step_ns stall_limit} {
  set pipe [fhrr_pipe_root]
  set csr  [fhrr_csr_root]
  set hdc  [fhrr_hdc_root]

  set debug_test [string tolower [fhrr_env_or_default FHRR_DEBUG_TEST fhrr]]
  set stage1_path ""
  set stage2_path ""
  set stage3_path ""

  if {[string match "*bundle*" $debug_test]} {
    set stage1_path "$hdc/bundle_stage_1_en(0)"
    set stage2_path "$hdc/bundle_stage_2_en(0)"
  } elseif {[string match "*bind*" $debug_test]} {
    set stage1_path "$hdc/bind_stage_1_en(0)"
    set stage2_path "$hdc/bind_stage_2_en(0)"
    set stage3_path "$hdc/bind_stage_3_en(0)"
  } elseif {[string match "*encode*" $debug_test]} {
    set stage1_path "$hdc/enc_stage_1_en(0)"
    set stage2_path "$hdc/enc_stage_2_en(0)"
    set stage3_path "$hdc/enc_stage_3_en(0)"
  } elseif {[string match "*similarity*" $debug_test]} {
    set stage1_path "$hdc/sim_stage_1_en(0)"
    set stage2_path "$hdc/sim_stage_2_en(0)"
  } elseif {[string match "*clip*" $debug_test]} {
    set stage1_path "$hdc/clip_stage_1_en(0)"
    set stage2_path "$hdc/clip_stage_2_en(0)"
    set stage3_path "$hdc/clip_stage_3_en(0)"
  }

  set prev_signature ""
  set stall_count 0
  set stage1_seen 0
  set stage2_seen 0
  set stage3_seen 0
  set wb_count 0
  set gnt_count 0

  puts "FHRR debug progress watch: iterations=$iterations step_ns=$step_ns stall_limit=$stall_limit test=$debug_test"
  if {[string match "*bundle*" $debug_test]} {
    puts "FHRR bundle LUT sample: sin1=[fhrr_safe_examine "$hdc/sine_lut_bun(1)"] cos1=[fhrr_safe_examine "$hdc/cosine_lut_bun(1)"] sin127=[fhrr_safe_examine "$hdc/sine_lut_bun(127)"] cos127=[fhrr_safe_examine "$hdc/cosine_lut_bun(127)"]"
  }

  for {set iter 0} {$iter < $iterations} {incr iter} {
    run ${step_ns}ns

    set pc [fhrr_safe_examine "$pipe/pc_IE"]
    set instr [fhrr_safe_examine "$pipe/instr_word_IE"]
    set decoded [fhrr_safe_examine "$pipe/decoded_instruction_IE"]
    set state [fhrr_safe_examine "$hdc/state_HDC(0)"]
    set hvr [fhrr_safe_examine "$hdc/HVSIZE_READ(0)"]
    set hvw [fhrr_safe_examine "$hdc/HVSIZE_WRITE(0)"]
    set wb [fhrr_safe_examine "$hdc/wb_ready(0)"]
    set halt [fhrr_safe_examine "$hdc/halt_hdc(0)"]
    set rec [fhrr_safe_examine "$hdc/recover_state(0)"]
    set gnt [fhrr_safe_examine "$hdc/hdc_data_gnt_i(0)"]
    set gnt_lat [fhrr_safe_examine "$hdc/hdc_data_gnt_i_lat(0)"]
    set hreq [fhrr_safe_examine "$hdc/hdc_instr_req(0)"]
    set busy [fhrr_safe_examine "$hdc/busy_HDC_internal(0)"]
    set busy_lat [fhrr_safe_examine "$hdc/busy_HDC_internal_lat(0)"]
    set req [fhrr_safe_examine "$hdc/hdc_sci_req(0)"]
    set raddr [fhrr_safe_examine "$hdc/hdc_sc_read_addr(0)"]
    set waddr [fhrr_safe_examine "$hdc/hdc_sc_write_addr(0)"]
    set rdata [fhrr_safe_examine "$hdc/hdc_sc_data_read(0)"]
    set wdata [fhrr_safe_examine "$hdc/hdc_sc_data_write_wire(0)"]
    set bundled [fhrr_safe_examine "$hdc/hdcu_in_bundled(0)"]
    set operand [fhrr_safe_examine "$hdc/hdcu_in_bundle_operand(0)"]
    set op_real [fhrr_safe_examine "$hdc/hdcu_in_bundle_operand_real(0)"]
    set op_imag [fhrr_safe_examine "$hdc/hdcu_in_bundle_operand_imag(0)"]
    set real_acc [fhrr_safe_examine "$hdc/real_accum(0)"]
    set imag_acc [fhrr_safe_examine "$hdc/imag_accum(0)"]
    set bundle_en [fhrr_safe_examine "$hdc/bundle_en(0)"]
    set decoded_lat [fhrr_safe_examine "$hdc/decoded_instruction_DSP_lat(0)"]
    set hvr_init [fhrr_safe_examine "$hdc/HVSIZE_READ_init(0)"]
    set hvr_lat [fhrr_safe_examine "$hdc/HVSIZE_READ_lat(0)"]
    set nextstate [fhrr_safe_examine "$hdc/nextstate_HDC(0)"]
    set harc_exec [fhrr_safe_examine "$hdc/harc_EXEC"]
    set hvsize0 [fhrr_safe_examine "$hdc/HVSIZE(0)"]
    set hvsize1 [fhrr_safe_examine "$hdc/HVSIZE(1)"]
    set hvsize2 [fhrr_safe_examine "$hdc/HVSIZE(2)"]
    set rs1_lat [fhrr_safe_examine "$hdc/RS1_Data_IE_lat(0)"]
    set rs2_lat [fhrr_safe_examine "$hdc/RS2_Data_IE_lat(0)"]
    set mcause [fhrr_safe_examine "$csr/MCAUSE"]
    set mepc [fhrr_safe_examine "$csr/MEPC"]

    set stage1 "NA"
    set stage2 "NA"
    set stage3 "NA"
    if {$stage1_path ne ""} {
      set stage1 [fhrr_safe_examine $stage1_path]
      if {$stage1 eq "1"} {
        set stage1_seen 1
      }
    }
    if {$stage2_path ne ""} {
      set stage2 [fhrr_safe_examine $stage2_path]
      if {$stage2 eq "1"} {
        set stage2_seen 1
      }
    }
    if {$stage3_path ne ""} {
      set stage3 [fhrr_safe_examine $stage3_path]
      if {$stage3 eq "1"} {
        set stage3_seen 1
      }
    }

    if {$wb eq "1"} {
      incr wb_count
    }
    if {$gnt eq "1"} {
      incr gnt_count
    }

    set stage_activity 0
    if {[fhrr_value_is_clean_nonzero $hreq] ||
        [fhrr_value_is_clean_nonzero $busy] ||
        [fhrr_value_is_clean_nonzero $busy_lat] ||
        [fhrr_value_is_clean_nonzero $req] ||
        [fhrr_value_is_clean_nonzero $gnt] ||
        $stage1_seen ||
        $stage2_seen ||
        $stage3_seen} {
      set stage_activity 1
    }

    if {$stage_activity} {
      set signature "$state|$nextstate|$hvr|$hvw|$wb|$halt|$rec|$hreq|$busy|$busy_lat|$gnt|$req|$raddr|$waddr|$stage1|$stage2|$stage3|$bundle_en|$decoded_lat|$hvr_init|$hvr_lat"
    } else {
      set signature "$pc|$instr|$state|$hreq|$busy|$busy_lat|$req|$gnt"
    }
    if {$signature eq $prev_signature} {
      incr stall_count
    } else {
      set stall_count 0
      set prev_signature $signature
    }

    set line "step=$iter"
    append line " pc=$pc"
    append line " instr=$instr"
    append line " dec=$decoded"
    append line " state=$state"
    append line " next=$nextstate"
    append line " hvr=$hvr"
    append line " hvri=$hvr_init"
    append line " hvrl=$hvr_lat"
    append line " hvw=$hvw"
    append line " wb=$wb"
    append line " halt=$halt"
    append line " rec=$rec"
    append line " hreq=$hreq"
    append line " busy=$busy"
    append line " busy_lat=$busy_lat"
    append line " gnt=$gnt"
    append line " gntl=$gnt_lat"
    append line " req=$req"
    append line " raddr=$raddr"
    append line " rs1=$rs1_lat"
    append line " rs2=$rs2_lat"
    append line " waddr=$waddr"
    append line " rdata=$rdata"
    append line " wdata=$wdata"
    append line " bundled=$bundled"
    append line " op=$operand"
    append line " opr=$op_real"
    append line " opi=$op_imag"
    append line " racc=$real_acc"
    append line " iacc=$imag_acc"
    append line " ben=$bundle_en"
    append line " dlat=$decoded_lat"
    append line " harc=$harc_exec"
    append line " hv0=$hvsize0"
    append line " hv1=$hvsize1"
    append line " hv2=$hvsize2"
    append line " s1=$stage1"
    append line " s2=$stage2"
    append line " s3=$stage3"
    if {[string match "*similarity*" $debug_test]} {
      append line " simop=[fhrr_safe_examine "$hdc/hdcu_in_sim_operands(0)"]"
      append line " simsum=[fhrr_safe_examine "$hdc/sim_cos_sum_wire(0)"]"
      append line " simacc=[fhrr_safe_examine "$hdc/cosine_accumulator_reg(0)"]"
      append line " simavg=[fhrr_safe_examine "$hdc/cosine_accumulator_avg(0)"]"
    }
    if {[string match "*encode*" $debug_test]} {
      append line " encop=[fhrr_safe_examine "$hdc/hdcu_in_enc_fpp_operands(0)"]"
      append line " mult=[fhrr_safe_examine "$hdc/mult_result(0)"]"
      append line " trunc=[fhrr_safe_examine "$hdc/trunc_mul_results(0)"]"
      append line " encacc=[fhrr_safe_examine "$hdc/accumulator_reg(0)"]"
      append line " encout=[fhrr_safe_examine "$hdc/hdc_out_enc_result(0)"]"
    }
    append line " stall=$stall_count"
    append line " mcause=$mcause"
    append line " mepc=$mepc"
    puts $line

    if {$stall_limit > 0 && $stall_count >= $stall_limit} {
      puts "FHRR debug stall detected: step=$iter stall_count=$stall_count state=$state hvr=$hvr hvw=$hvw req=$req gnt=$gnt"
      break
    }
  }

  puts "FHRR debug summary: stage1_seen=$stage1_seen stage2_seen=$stage2_seen stage3_seen=$stage3_seen grants=$gnt_count wb_pulses=$wb_count"
}

proc fhrr_debug_wait_for_activity {iterations step_ns} {
  set pipe [fhrr_pipe_root]
  set hdc  [fhrr_hdc_root]

  puts "FHRR debug wait-for-activity: iterations=$iterations step_ns=$step_ns"

  for {set iter 0} {$iter < $iterations} {incr iter} {
    run ${step_ns}ns

    set pc [fhrr_safe_examine "$pipe/pc_IE"]
    set instr [fhrr_safe_examine "$pipe/instr_word_IE"]
    set decoded [fhrr_safe_examine "$pipe/decoded_instruction_IE"]
    set hreq [fhrr_safe_examine "$hdc/hdc_instr_req(0)"]
    set busy [fhrr_safe_examine "$hdc/busy_HDC_internal(0)"]
    set busy_lat [fhrr_safe_examine "$hdc/busy_HDC_internal_lat(0)"]
    set gnt [fhrr_safe_examine "$hdc/hdc_data_gnt_i(0)"]
    set req [fhrr_safe_examine "$hdc/hdc_sci_req(0)"]

    if {$iter % 500 == 0} {
      puts "wait_step=$iter pc=$pc instr=$instr dec=$decoded hreq=$hreq busy=$busy busy_lat=$busy_lat gnt=$gnt req=$req"
    }

    if {[fhrr_value_is_clean_nonzero $hreq] ||
        [fhrr_value_is_clean_nonzero $busy] ||
        [fhrr_value_is_clean_nonzero $busy_lat] ||
        [fhrr_value_is_clean_nonzero $gnt] ||
        [fhrr_value_is_clean_nonzero $req]} {
      puts "FHRR activity detected: step=$iter pc=$pc instr=$instr dec=$decoded hreq=$hreq busy=$busy busy_lat=$busy_lat gnt=$gnt req=$req"
      return 1
    }
  }

  puts "FHRR activity not detected within the requested window"
  return 0
}

proc fhrr_debug_run_from_env {} {
  set warmup_ns [fhrr_env_or_default FHRR_DEBUG_WARMUP_NS 200000]
  set iterations [fhrr_env_or_default FHRR_DEBUG_ITERATIONS 200]
  set step_ns [fhrr_env_or_default FHRR_DEBUG_STEP_NS 1000]
  set stall_limit [fhrr_env_or_default FHRR_DEBUG_STALL_LIMIT 16]
  set wait_iters [fhrr_env_or_default FHRR_DEBUG_WAIT_ITERS 0]
  set wait_step_ns [fhrr_env_or_default FHRR_DEBUG_WAIT_STEP_NS $step_ns]

  if {$warmup_ns > 0} {
    puts "FHRR debug warmup: warmup_ns=$warmup_ns"
    run ${warmup_ns}ns
  }

  if {$wait_iters > 0} {
    fhrr_debug_wait_for_activity $wait_iters $wait_step_ns
  }

  fhrr_debug_watch_progress $iterations $step_ns $stall_limit
}
