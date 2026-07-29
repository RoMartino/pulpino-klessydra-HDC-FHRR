#!/usr/bin/env python3

import tkinter
import tkinter.messagebox
import customtkinter

import re, os, subprocess
import math

from PIL import Image

def prepare(s):
    return re.sub("[^a-zA-Z0-9_]", "_", s)

class tcolors:
    OK      = '\033[92m'
    WARNING = '\033[93m'
    ERROR   = '\033[91m'
    ENDC    = '\033[0m'
    BLUE    = '\033[94m'

def execute(cmd, silent=False):
    with open(os.devnull, "wb") as devnull:
        if silent:
            stdout = devnull
        else:
            stdout = None

        return subprocess.call(cmd.split(), stdout=stdout)

def execute_out(cmd, silent=False):
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)
    out, err = p.communicate()

    return out

def execute_popen(cmd, silent=False):
    with open(os.devnull, "wb") as devnull:
        if silent:
            return subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=devnull)
        else:
            return subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)

customtkinter.set_appearance_mode("System")  # Modes: "System" (standard), "Dark", "Light"
customtkinter.set_default_color_theme("blue")  # Themes: "blue" (standard), "green", "dark-blue"


class App(customtkinter.CTk):
    def __init__(self):
        super().__init__()

        # configure window
        self.title("KView")
        self.geometry(f"{1180}x{675}")

        # Block the resizing of the window
        self.resizable(False, False)


        # configure grid layout (4x4)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure((2, 2), weight=1)

        def disable_riscy(event):
            if  self.klessydra_switch.get() == 1 or self.riscy_switch.get() == 0:
                self.pulpino_core_optionmenu.configure(state="disabled")
                self.pulpino_rvc_switch.configure(state="disabled", progress_color="gray")
                self.zero_rv32e_switch.configure(state="disabled", progress_color="gray")
                self.zero_rv32m_switch.configure(state="disabled", progress_color="gray")
                self.ri5cy_rv32f_switch.configure(state="disabled", progress_color="gray")

        def enable_klessydra(event):
            if self.klessydra_switch.get() == 1:
                self.riscy_switch.deselect()
                self.klessydra_core_optionmenu.configure(state="normal")
                self.KLESS_RVE_switch.configure(state="normal", progress_color="#1F6AA5")
                self.KLESS_RVM_switch.configure(state="normal", progress_color="#1F6AA5")
                self.THREAD_POOL_SIZE_combobox.configure(state="normal")
                self.Morph_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.Latch_RF_switch.configure(state="normal", progress_color="#1F6AA5")
                self.Lutram_RF_switch.configure(state="normal", progress_color="#1F6AA5")
                self.fetch_stage_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.superscalar_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.branch_predict_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.btb_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.btb_len_combobox.configure(state="normal")
                self.accl_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.Replicated_Accl_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.Multithreaded_Accl_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.SIMD_combobox.configure(state="normal")
                self.SPM_NUM_combobox.configure(state="normal")
                self.SPM_SIZE_combobox.configure(state="normal")
                self.SPM_START_ADDR_textbox.configure(state="normal")
                self.mcycle_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.minstret_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.mhpmcounter_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.count_all_switch.configure(state="normal", progress_color="#1F6AA5")
                #self.debug_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.tracer_en_switch.configure(state="normal", progress_color="#1F6AA5")

        def enable_riscy(event):
            if  self.riscy_switch.get() == 1:
                self.klessydra_switch.deselect()
                self.pulpino_core_optionmenu.configure(state="normal")
                self.pulpino_rvc_switch.configure(state="normal", progress_color="#1F6AA5")
                self.zero_rv32e_switch.configure(state="normal", progress_color="#1F6AA5")
                self.zero_rv32m_switch.configure(state="normal", progress_color="#1F6AA5")
                self.ri5cy_rv32f_switch.configure(state="normal", progress_color="#1F6AA5")

        def disable_klessydra(event):
            if  self.klessydra_switch.get() == 0 or self.riscy_switch.get() == 1:
                self.klessydra_core_optionmenu.configure(state="disabled")
                self.KLESS_RVE_switch.configure(state="disabled", progress_color="gray")
                self.KLESS_RVM_switch.configure(state="disabled", progress_color="gray")
                self.THREAD_POOL_SIZE_combobox.configure(state="disabled")
                self.Morph_en_switch.configure(state="disabled", progress_color="gray")
                self.Latch_RF_switch.configure(state="disabled", progress_color="gray")
                self.Lutram_RF_switch.configure(state="disabled", progress_color="gray")
                self.fetch_stage_en_switch.configure(state="disabled", progress_color="gray")
                self.superscalar_en_switch.configure(state="disabled", progress_color="gray")
                self.branch_predict_en_switch.configure(state="disabled", progress_color="gray")
                self.btb_en_switch.configure(state="disabled", progress_color="gray")
                self.btb_len_combobox.configure(state="disabled")
                self.accl_en_switch.configure(state="disabled", progress_color="gray")
                self.Replicated_Accl_en_switch.configure(state="disabled", progress_color="gray")
                self.Multithreaded_Accl_en_switch.configure(state="disabled", progress_color="gray")
                self.SIMD_combobox.configure(state="disabled")
                self.SPM_NUM_combobox.configure(state="disabled")
                self.SPM_SIZE_combobox.configure(state="disabled")
                self.SPM_START_ADDR_textbox.configure(state="disabled")
                self.mcycle_en_switch.configure(state="disabled", progress_color="gray")
                self.minstret_en_switch.configure(state="disabled", progress_color="gray")
                self.mhpmcounter_en_switch.configure(state="disabled", progress_color="gray")
                self.count_all_switch.configure(state="disabled", progress_color="gray")
                #self.debug_en_switch.configure(state="disabled", progress_color="gray")
                self.tracer_en_switch.configure(state="disabled", progress_color="gray")

        def clear_text(event):
            self.textbox.configure(state="normal")
            self.textbox.delete("0.0", tkinter.END)
            self.textbox.configure(state="disabled")

        def hover_KLESS_RVE_switch(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables the Embedded extension that sets the registerfile size to 16 registers instead of 32. \n\n")
            self.textbox.configure(state="disabled")

        def hover_KLESS_RVM_switch(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables the M-extension of the RISC-V instruction set. \n\n")
            self.textbox.configure(state="disabled")

        def hover_THREAD_POOL_SIZE(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Sets the total number of Harts in the core. \n\n")
            self.textbox.configure(state="disabled")

        def hover_Morph_en(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables hardware metamorphosis, that lets the IMT processor morph into an in-order execution processor to avoid stalling. \n\n")
            self.textbox.configure(state="disabled")

        def hover_Latch_RF(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Lets the registerfile be generated with Latches instead of Flip-Flops (FPGA Implementation recommended). \n\n")
            self.textbox.configure(state="disabled")

        def hover_Lutram_RF(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Lets the registerfile be generated with LUTRAMs instead of Flip-Flops (FPGA Implementation recommended). \n\n")
            self.textbox.configure(state="disabled")

        def hover_superscalar_en(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables superscalar between different instructions, as long as these instructions write to different memories. \n\n")
            self.textbox.configure(state="disabled")

        def hover_fetch_stage_en(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables the an additional fetch stage which handles some of the data dependency logic. \n\n")
            self.textbox.configure(state="disabled")

        def hover_branch_predict_en(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables the generation of a single bit branch predictor, that branches with negative offsets. \n\n")
            self.textbox.configure(state="disabled")

        def hover_btb_en(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables the generation of a branch target buffer. \n\n")
            self.textbox.configure(state="disabled")

        def hover_blb_len(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Sets the size of the branch target buffer. \n\n")
            self.textbox.configure(state="disabled")


        def hover_Accl_en(event):
            clear_text
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables the generation of the Vector Coprocessing Unit (VCU). \n\n")
            self.textbox.configure(state="disabled")

        def hover_Replicated_Accl_en(event):
            clear_text
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Makes a dedicated VCU for each hart, that means each hart will have its own VCE that has its own set of SIMD functional units, and each hart has its own dedicated SPMs. \n\n")
            self.textbox.configure(state="disabled")

        def hover_Multithreaded_Accl_en(event):
            clear_text
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Makes the SIMD functional units in the VCU become shared among all the harts instead of dedicated this requires \"Replicated_Accl_en\" to be set.  However each hart still maintainins its own set of SPMs. Instructions from other harts that need to use a busy functional unit in the VCU will have their access to the unit serialized. \n\n")
            self.textbox.configure(state="disabled")

        def hover_SIMD(event):
            clear_text
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Sets the width of the SIMD execution of the functional units in the VCU, where SIMD = 1 is equal to 32-bits \n\n")
            self.textbox.configure(state="disabled")

        def hover_SPM_SIZE(event):
            clear_text
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Sets the size of a single Scratchpad Memory\n\n")
            self.textbox.configure(state="disabled")

        def hover_SPM_NUM(event):
            clear_text
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Sets the number of Scratchpad Memories, however if \"Replicated_Accl_en\" is set, then this value will represent the number of scratchpad memories per hart instead of the total number of scratchpad memories in the core \n\n")
            self.textbox.configure(state="disabled")

        def hover_SPM_START_ADDR(event):
            clear_text
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Defines the physical memory address in which the Scratchpad memories are mapped, the occupied memory space depends on the combination of settings defined above, and the occupied memory space goes in increasing order using this address as the starting point \n\n")
            self.textbox.configure(state="disabled")

        def hover_mcycle_en(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables the clock cycle counting for performance monitoring. \n\n")
            self.textbox.configure(state="disabled")

        def hover_minstret_en(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables the instruction counting for performance monitoring. \n\n")
            self.textbox.configure(state="disabled")

        def hover_mhpmcounter_en(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables the all the remainder performance monitors . \n\n")
            self.textbox.configure(state="disabled")

        def hover_count_all(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enabling this lets perfomance counters count for all the harts instead of the hart they belong to. \n\n")
            self.textbox.configure(state="disabled")

        def hover_debug_en(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Debug Unit is currently not available, this will be enabled once it is. \n\n")
            self.textbox.configure(state="disabled")

        def hover_tracer_en(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables the instruction tracer (not synthesizable). \n\n")
            self.textbox.configure(state="disabled")

        def hover_klessydra(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables the usage of the Klessydra Cores. \n\n")
            self.textbox.configure(state="disabled")

        def hover_riscy(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Enables the usage of the PULPino RISCY cores. \n\n")
            self.textbox.configure(state="disabled")

        def hover_klessydra_core(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Select which Klessydra Core to use. \n\n")
            self.textbox.configure(state="disabled")

        def hover_pulpino_core(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Select which RISCY Core to use. \n\n")
            self.textbox.configure(state="disabled")


        def hover_pulpino_rvc(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Set this to use compressed instructions. \n\n")
            self.textbox.configure(state="disabled")

        def hover_zero_rv32e(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Zeroriscy with only 16 registers. \n\n")
            self.textbox.configure(state="disabled")

        def hover_zero_rv32m(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Zeroriscy with the M-extension. \n\n")
            self.textbox.configure(state="disabled")

        def hover_ri5cy_rv32f(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Set this if you are using the Floating Point extensions for RI5CY. \n\n")
            self.textbox.configure(state="disabled")

        def hover_vsize(event):
            self.textbox.configure(state="normal")
            self.textbox.insert("0.0", "Sets the Vector Size (number of elements) to use in the VCU Instruction tests. \n\n")
            self.textbox.configure(state="disabled")


        def toggle_core_selection(event):
            if (self.klessydra_core_optionmenu.get() == "Klessydra-Morph" or self.klessydra_core_optionmenu.get() == "Klessydra-HetC"):
                self.KLESS_RVM_switch.configure(state="normal", progress_color="#1F6AA5")
                self.Latch_RF_switch.configure(state="normal", progress_color="#1F6AA5")
                self.Lutram_RF_switch.configure(state="normal", progress_color="#1F6AA5")
                self.Morph_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.superscalar_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.fetch_stage_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.branch_predict_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.btb_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.btb_len_combobox.configure(state="normal")
                self.accl_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.Replicated_Accl_en_switch.configure(state="normal", progress_color="#1F6AA5")
            else:
                self.Latch_RF_switch.configure(state="disabled", progress_color="gray")
                self.Lutram_RF_switch.configure(state="disabled", progress_color="gray")
                self.Morph_en_switch.configure(state="disabled", progress_color="gray")
                self.btb_len_combobox.configure(state="disabled")
                self.fetch_stage_en_switch.configure(state="disabled", progress_color="gray")
                self.branch_predict_en_switch.configure(state="disabled", progress_color="gray")
                self.btb_en_switch.configure(state="disabled", progress_color="gray")

            if (self.klessydra_core_optionmenu.get() == "Klessydra-T03" or self.klessydra_core_optionmenu.get() == "Klessydra-T02"):
                self.KLESS_RVE_switch.configure(state="disabled", progress_color="gray")
                self.KLESS_RVM_switch.configure(state="disabled", progress_color="gray")
                self.accl_en_switch.configure(state="disabled", progress_color="gray")
                self.Replicated_Accl_en_switch.configure(state="disabled", progress_color="gray")
                self.Multithreaded_Accl_en_switch.configure(state="disabled", progress_color="gray")
                self.SIMD_combobox.configure(state="disabled")
                self.SPM_NUM_combobox.configure(state="disabled")
                self.SPM_SIZE_combobox.configure(state="disabled")
                self.SPM_START_ADDR_textbox.configure(state="disabled")
            else:
                self.KLESS_RVE_switch.configure(state="normal", progress_color="#1F6AA5")
                self.KLESS_RVM_switch.configure(state="normal", progress_color="#1F6AA5")
                self.accl_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.Replicated_Accl_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.Multithreaded_Accl_en_switch.configure(state="normal", progress_color="#1F6AA5")
                self.SIMD_combobox.configure(state="normal")
                self.SPM_NUM_combobox.configure(state="normal")
                self.SPM_SIZE_combobox.configure(state="normal")
                self.SPM_START_ADDR_textbox.configure(state="normal")

        # create textbox label
        self.label = customtkinter.CTkLabel(self, text="Description", font=customtkinter.CTkFont(size=14, weight="bold"))
        self.label.grid(row=1, column=3, pady=(20, 0), sticky="nsew")

        # create textbox
        self.textbox = customtkinter.CTkTextbox(self, width=250,  font=customtkinter.CTkFont(size=14, weight="bold"))
        self.textbox.grid(row=2, column=3, padx=(20, 20), pady=(10, 0), sticky="nsew", rowspan=2)

        klessydra_logo = customtkinter.CTkImage(Image.open("assets/Klessydra_logo.png"),
                                          size=(100, 100))
        klessydra_logo_button = customtkinter.CTkButton(self, image=klessydra_logo, text="", fg_color=("lightgray", "#696969"), hover_color=("lightgray", "#696969"))
        klessydra_logo_button.grid(row=0, column=3, padx=(20, 20), pady=(10, 0), sticky="nsew")


        # create sidebar frame with widgets
        self.sidebar_frame = customtkinter.CTkFrame(self, width=140, corner_radius=0)
        self.sidebar_frame.grid(row=0, column=0, rowspan=6, sticky="nsew")
        self.sidebar_frame.grid_rowconfigure(6, weight=1)
        self.logo_label = customtkinter.CTkLabel(self.sidebar_frame, text="Klessydra", font=customtkinter.CTkFont(size=20, weight="bold"))
        self.logo_label.grid(row=0, column=0, padx=20, pady=(20, 10))
        self.sidebar_button_1 = customtkinter.CTkButton(self.sidebar_frame, text="CMake", command=self.cmake_button_event)
        self.sidebar_button_1.grid(row=1, column=0, padx=20, pady=10)
        self.sidebar_button_2 = customtkinter.CTkButton(self.sidebar_frame, text="Compile RTL", command=self.compile_rtl_button_event)
        self.sidebar_button_2.grid(row=2, column=0, padx=20, pady=10)
        self.sidebar_button_3 = customtkinter.CTkButton(self.sidebar_frame, text="Compile Core", command=self.compile_core_button_event)
        self.sidebar_button_3.grid(row=3, column=0, padx=20, pady=10)
        self.sidebar_button_4 = customtkinter.CTkButton(self.sidebar_frame, text="Build Tests", command=self.build_tests_button_event)
        self.sidebar_button_4.grid(row=4, column=0, padx=20, pady=10)
        self.appearance_mode_label = customtkinter.CTkLabel(self.sidebar_frame, text="Appearance Mode:", anchor="w")
        self.appearance_mode_label.grid(row=7, column=0, padx=20, pady=(10, 0))
        self.appearance_mode_optionemenu = customtkinter.CTkOptionMenu(self.sidebar_frame, values=["Light", "Dark", "System"],
                                                                       command=self.change_appearance_mode_event)
        self.appearance_mode_optionemenu.grid(row=8, column=0, padx=20, pady=(10, 10))
        self.appearance_mode_optionemenu.set("Dark")
        customtkinter.set_appearance_mode(self.appearance_mode_optionemenu.get())

        self.scaling_label = customtkinter.CTkLabel(self.sidebar_frame, text="UI Scaling:", anchor="w")
        self.scaling_label.grid(row=9, column=0, padx=20, pady=(10, 0))
        self.scaling_optionemenu = customtkinter.CTkOptionMenu(self.sidebar_frame, values=["80%", "90%", "100%", "110%", "120%"],
                                                               command=self.change_scaling_event)
        self.scaling_optionemenu.grid(row=10, column=0, padx=20, pady=(10, 20))
        self.scaling_optionemenu.set("100%")

        # create main entry and button
        self.command_line = customtkinter.CTkEntry(self, placeholder_text="e.g. make helloworld")
        self.command_line.grid(row=5, column=1, columnspan=2, padx=(20, 0), pady=(20, 20), sticky="nsew")

        self.main_button_1 = customtkinter.CTkButton(master=self, fg_color="transparent", border_width=2, text="Run", text_color=("gray10", "#DCE4EE"), command=self.compile_and_run_button_event)
        self.main_button_1.grid(row=5, column=3, padx=(20, 20), pady=(20, 20), sticky="nsew")

        # create tabview label
        #self.label = customtkinter.CTkLabel(self, text="Configurations", font=customtkinter.CTkFont(size=14, weight="bold"))
        #self.label.grid(row=0, column=1, sticky="nsew")

        # create tabview
        self.tabview = customtkinter.CTkTabview(self, width=250, height=500)
        self.tabview.grid(row=0, column=1, padx=(20, 0), sticky="nsew", rowspan=4)
        self.tabview.add("Klessydra")
        self.tabview.add("PULPino")
        self.tabview.add("Software Defines")
        self.tabview.add("Toolchain")

        self.klessydra_switch = customtkinter.CTkSwitch(self.tabview.tab("Klessydra"), text="Klessydra", font=customtkinter.CTkFont(size=16, weight="bold"))
        self.klessydra_core_label = customtkinter.CTkLabel(self.tabview.tab("Klessydra"), text="Klessydra Core: ", font=customtkinter.CTkFont(size=12, weight="bold"))
        self.klessydra_core_optionmenu = customtkinter.CTkOptionMenu(self.tabview.tab("Klessydra"), width=175, dynamic_resizing=False, command=toggle_core_selection,
                                                        values=["Klessydra-Morph", "Klessydra-HetC", "Klessydra-T13", "Klessydra-T03", "Klessydra-T02", "Klessydra-S1, Klessydra-F03, Klessydra-fT13"])
        self.ISA_frame = customtkinter.CTkScrollableFrame(self.tabview.tab("Klessydra"), label_text="ISA Extensions", width=100, height=50)
        self.KLESS_RVE_switch = customtkinter.CTkSwitch(master=self.ISA_frame, text="RV32E")
        self.KLESS_RVM_switch = customtkinter.CTkSwitch(master=self.ISA_frame, text="RV32M")
        self.Features_frame = customtkinter.CTkScrollableFrame(self.tabview.tab("Klessydra"), label_text="Core Features")
        self.THREAD_POOL_SIZE_label = customtkinter.CTkLabel(master=self.Features_frame, text="Thread Pool Size", font=customtkinter.CTkFont(size=12, weight="bold"))
        self.THREAD_POOL_SIZE_combobox = customtkinter.CTkComboBox(master=self.Features_frame, values=["1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"], width=50)
        self.Morph_en_switch = customtkinter.CTkSwitch(master=self.Features_frame, text="Morph_en")
        self.Latch_RF_switch = customtkinter.CTkSwitch(master=self.Features_frame, text="Latch_RF")
        self.Lutram_RF_switch = customtkinter.CTkSwitch(master=self.Features_frame, text="Lutram_RF")
        self.superscalar_en_switch = customtkinter.CTkSwitch(master=self.Features_frame, text="superscalar_en")
        self.fetch_stage_en_switch = customtkinter.CTkSwitch(master=self.Features_frame, text="fetch_stage_en")
        self.branch_predict_en_switch = customtkinter.CTkSwitch(master=self.Features_frame, text="branch_predict_en")
        self.btb_en_switch = customtkinter.CTkSwitch(master=self.Features_frame, text="btb_en")
        self.blb_len_label = customtkinter.CTkLabel(master=self.Features_frame, text="btb_len", font=customtkinter.CTkFont(size=12, weight="bold"))
        self.btb_len_combobox = customtkinter.CTkComboBox(master=self.Features_frame, values=["1", "2", "4", "8", "16", "32", "64", "128", "256","512","1024"], width=70)
        self.VCU_Frame = customtkinter.CTkScrollableFrame(self.tabview.tab("Klessydra"), label_text="VCU Features")
        self.accl_en_switch = customtkinter.CTkSwitch(master=self.VCU_Frame, text="accl_en")
        self.Replicated_Accl_en_switch = customtkinter.CTkSwitch(master=self.VCU_Frame, text="Replicated_Accl_en")
        self.Multithreaded_Accl_en_switch = customtkinter.CTkSwitch(master=self.VCU_Frame, text="Multithreaded_Accl_en")
        self.SIMD_label = customtkinter.CTkLabel(self.VCU_Frame, text="SIMD", font=customtkinter.CTkFont(size=12, weight="bold"))
        self.SIMD_combobox = customtkinter.CTkComboBox(self.VCU_Frame, values=["1", "2", "4", "8"], width=50)
        self.SPM_NUM_label = customtkinter.CTkLabel(self.VCU_Frame, text="SPM_NUM", font=customtkinter.CTkFont(size=12, weight="bold"))
        self.SPM_NUM_combobox = customtkinter.CTkComboBox(self.VCU_Frame, values=["2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"], width=50)
        self.SPM_SIZE_label = customtkinter.CTkLabel(self.VCU_Frame, text="SPM SIZE", font=customtkinter.CTkFont(size=12, weight="bold"))
        self.SPM_SIZE_combobox = customtkinter.CTkComboBox(self.VCU_Frame, values=["128", "256","512","1024","2048","4096","8192","16384","32768","65536"], width=70)
        self.SPM_START_ADDR_label = customtkinter.CTkLabel(self.VCU_Frame, text="Mem map", font=customtkinter.CTkFont(size=12, weight="bold"))
        self.SPM_START_ADDR_textbox = customtkinter.CTkTextbox(self.VCU_Frame, width=40, height=10, font=customtkinter.CTkFont(size=12, weight="bold"))
        self.SPM_START_ADDR_textbox.insert("0.0", text="0x10000000")
        self.monitoring_label = customtkinter.CTkLabel(self.tabview.tab("Klessydra"), text="Monitoring: ", font=customtkinter.CTkFont(size=12, weight="bold"))
        self.mcycle_en_switch = customtkinter.CTkSwitch(self.tabview.tab("Klessydra"), text="mcycle_en")
        self.minstret_en_switch = customtkinter.CTkSwitch(self.tabview.tab("Klessydra"), text="minstret_en")
        self.mhpmcounter_en_switch = customtkinter.CTkSwitch(self.tabview.tab("Klessydra"), text="mhpcounter_en")
        self.count_all_switch = customtkinter.CTkSwitch(self.tabview.tab("Klessydra"), text="count_all")
        self.misc_label = customtkinter.CTkLabel(self.tabview.tab("Klessydra"), text="Misc: ", font=customtkinter.CTkFont(size=12, weight="bold"))
        self.debug_en_switch = customtkinter.CTkSwitch(self.tabview.tab("Klessydra"), text="debug_en")
        self.debug_en_switch.configure(state="disabled", progress_color="gray")
        self.tracer_en_switch = customtkinter.CTkSwitch(self.tabview.tab("Klessydra"), text="tracer_en")

        self.riscy_switch = customtkinter.CTkSwitch(self.tabview.tab("PULPino"), text="RISCY", font=customtkinter.CTkFont(size=16, weight="bold"))
        self.pulpino_core_label = customtkinter.CTkLabel(self.tabview.tab("PULPino"), text="PULPino Core: ", font=customtkinter.CTkFont(size=12, weight="bold"))
        self.pulpino_core_optionmenu = customtkinter.CTkOptionMenu(self.tabview.tab("PULPino"), dynamic_resizing=False, command=toggle_core_selection,
                                                        values=["RI5CY", "ZERO-RISCY"])
        self.pulpino_rvc_switch = customtkinter.CTkSwitch(self.tabview.tab("PULPino"), text="RVC")
        self.Zeroriscy_frame = customtkinter.CTkScrollableFrame(self.tabview.tab("PULPino"), label_text="Zeroriscy Features", width=100)
        self.zero_rv32e_switch = customtkinter.CTkSwitch(self.Zeroriscy_frame, text="RV32E")
        self.zero_rv32m_switch = customtkinter.CTkSwitch(self.Zeroriscy_frame, text="RV32M")
        self.ri5cy_frame = customtkinter.CTkScrollableFrame(self.tabview.tab("PULPino"), label_text="RI5CY Features", width=100)
        self.ri5cy_rv32f_switch = customtkinter.CTkSwitch(self.ri5cy_frame, text="RV32F")

        self.vsize_label   = customtkinter.CTkLabel(self.tabview.tab("Software Defines"), text="VSIZE", font=customtkinter.CTkFont(size=12, weight="bold"))
        self.vsize_textbox = customtkinter.CTkTextbox(self.tabview.tab("Software Defines"), width=60, height=10, font=customtkinter.CTkFont(size=12, weight="bold"))
        self.vsize_textbox.insert("0.0", text="1")


        self.klessydra_switch.place(x=40, y=18)
        self.klessydra_core_label.place(x=40, y=60)
        self.klessydra_core_optionmenu.place(x=175, y=55)
        self.ISA_frame.grid(row=1, column=0, padx=(20, 0), pady=(110, 0), sticky="nsew")
        self.KLESS_RVE_switch.grid(row=1, column=0, padx=5, pady=5, sticky="W")
        self.KLESS_RVM_switch.grid(row=2, column=0, padx=5, pady=5, sticky="W")
        self.Features_frame.grid(row=1, column=1, padx=(20, 0), pady=(110, 0), sticky="nsew")
        self.THREAD_POOL_SIZE_label.grid(row=0, column=0, padx=5, pady=5, sticky="W")
        self.THREAD_POOL_SIZE_combobox.grid(row=0, column=0, padx=140, sticky="W")
        self.Morph_en_switch.grid(row=1, column=0, padx=5, pady=5, sticky="W")
        self.Latch_RF_switch.grid(row=2, column=0, padx=5, pady=5, sticky="W")
        self.Lutram_RF_switch.grid(row=3, column=0, padx=5, pady=5, sticky="W")
        self.superscalar_en_switch.grid(row=4, column=0, padx=5, pady=5, sticky="W")
        self.fetch_stage_en_switch.grid(row=5, column=0, padx=5, pady=5, sticky="W")
        self.branch_predict_en_switch.grid(row=6, column=0, padx=5, pady=5, sticky="W")
        self.btb_en_switch.grid(row=7, column=0, padx=5, pady=5, sticky="W")
        self.blb_len_label.grid(row=8, column=0, padx=5, pady=5, sticky="W")
        self.btb_len_combobox.grid(row=8, column=0, padx=70, sticky="W")
        self.VCU_Frame.grid(row=1, column=2, padx=(20, 0), pady=(110, 0), sticky="nsew")
        self.accl_en_switch.grid(row=1, column=0, padx=5, pady=5, sticky="W")
        self.Replicated_Accl_en_switch.grid(row=2, column=0, padx=5, pady=5, sticky="W")
        self.Multithreaded_Accl_en_switch.grid(row=3, column=0, padx=5, pady=5, sticky="W")
        self.SIMD_label.grid(row=4, column=0, padx=5, pady=5, sticky="W")
        self.SIMD_combobox.grid(row=4, column=0, padx=100, sticky="W")
        self.SPM_NUM_label.grid(row=5, column=0, padx=5, pady=5, sticky="W")
        self.SPM_NUM_combobox.grid(row=5, column=0, padx=100, sticky="W")
        self.SPM_SIZE_label.grid(row=6, column=0, padx=5, pady=5, sticky="W")
        self.SPM_SIZE_combobox.grid(row=6, column=0, padx=100, sticky="W")
        self.SPM_START_ADDR_label.grid(row=7, column=0, padx=5, pady=5, sticky="W")
        self.SPM_START_ADDR_textbox.grid(row=7, column=0, padx=(90, 0), pady=(0, 0), sticky="nsew", rowspan=4)
        self.monitoring_label.grid(row=2, column=0, padx=25, pady=(10,0), sticky="W")
        self.mcycle_en_switch.grid(row=3, column=0, padx=25, pady=5, sticky="W")
        self.minstret_en_switch.grid(row=4, column=0, padx=25, pady=5, sticky="W")
        self.mhpmcounter_en_switch.grid(row=5, column=0, padx=25, pady=5, sticky="W")
        self.count_all_switch.grid(row=6, column=0, padx=25, pady=5, sticky="W")
        self.misc_label.grid(row=2, column=1, padx=35, pady=(10,0), sticky="W")
        self.debug_en_switch.grid(row=3, column=1, padx=35, pady=5, sticky="W")
        self.tracer_en_switch.grid(row=4, column=1, padx=35, pady=5, sticky="W")

        self.riscy_switch.place(x=40, y=18)
        self.pulpino_core_label.place(x=40, y=60)
        self.pulpino_core_optionmenu.place(x=165, y=60)
        self.pulpino_rvc_switch.place(x=40, y=100)
        self.Zeroriscy_frame.grid(row=2, column=0, padx=(20, 0), pady=(140, 0), sticky="nsew")
        self.zero_rv32e_switch.grid(row=0, column=0, pady=(0,20), sticky="W")
        self.zero_rv32m_switch.grid(row=1, column=0, pady=(0,20), sticky="W")
        self.ri5cy_frame.grid(row=2, column=1, padx=(20, 0), pady=(140, 0), sticky="nsew")
        self.ri5cy_rv32f_switch.grid(row=0, column=0, pady=(0,20), sticky="W")

        self.vsize_label.grid(row=0, column=0, padx=(10, 0), pady=(20, 0), sticky="W")
        self.vsize_textbox.grid(row=0, column=0, padx=(70, 0), pady=(20, 0), sticky="W")

        self.command_line.bind('<Return>', lambda e: self.compile_and_run_button_event() , add='+')
        self.klessydra_core_label.bind('<Enter>', hover_klessydra_core, add='+')
        self.klessydra_core_label.bind('<Leave>', clear_text, add='+')
        self.klessydra_core_optionmenu.bind('<Enter>', hover_klessydra_core, add='+')
        self.klessydra_core_optionmenu.bind('<Leave>', clear_text, add='+')
        self.klessydra_switch.bind('<Button-1>', disable_riscy, add='+')
        self.klessydra_switch.bind('<Button-1>', enable_klessydra, add='+')
        self.klessydra_switch.bind('<Button-1>', disable_klessydra, add='+')
        self.klessydra_switch.bind('<Enter>', hover_klessydra, add='+')
        self.klessydra_switch.bind('<Leave>', clear_text, add='+')
        self.KLESS_RVE_switch.bind('<Enter>', hover_KLESS_RVE_switch, add='+')
        self.KLESS_RVE_switch.bind('<Leave>', clear_text, add='+')
        self.KLESS_RVM_switch.bind('<Enter>', hover_KLESS_RVM_switch, add='+')
        self.KLESS_RVM_switch.bind('<Leave>', clear_text, add='+')
        self.THREAD_POOL_SIZE_label.bind('<Enter>', hover_THREAD_POOL_SIZE, add='+')
        self.THREAD_POOL_SIZE_label.bind('<Leave>', clear_text, add='+')
        self.THREAD_POOL_SIZE_combobox.bind('<Enter>', hover_THREAD_POOL_SIZE, add='+')
        self.THREAD_POOL_SIZE_combobox.bind('<Leave>', clear_text, add='+')
        self.Morph_en_switch.bind('<Enter>', hover_Morph_en, add='+')
        self.Morph_en_switch.bind('<Leave>', clear_text, add='+')
        self.Latch_RF_switch.bind('<Enter>', hover_Latch_RF, add='+')
        self.Latch_RF_switch.bind('<Leave>', clear_text, add='+')
        self.Lutram_RF_switch.bind('<Enter>', hover_Lutram_RF, add='+')
        self.Lutram_RF_switch.bind('<Leave>', clear_text, add='+')
        self.superscalar_en_switch.bind('<Enter>', hover_superscalar_en, add='+')
        self.superscalar_en_switch.bind('<Leave>', clear_text, add='+')
        self.fetch_stage_en_switch.bind('<Enter>', hover_fetch_stage_en, add='+')
        self.fetch_stage_en_switch.bind('<Leave>', clear_text, add='+')
        self.branch_predict_en_switch.bind('<Enter>', hover_branch_predict_en, add='+')
        self.branch_predict_en_switch.bind('<Leave>', clear_text, add='+')
        self.btb_en_switch.bind('<Enter>', hover_btb_en, add='+')
        self.btb_en_switch.bind('<Leave>', clear_text, add='+')
        self.blb_len_label.bind('<Enter>', hover_blb_len, add='+')
        self.blb_len_label.bind('<Leave>', clear_text, add='+')
        self.btb_len_combobox.bind('<Enter>', hover_blb_len, add='+')
        self.btb_len_combobox.bind('<Leave>', clear_text, add='+')
        self.accl_en_switch.bind('<Enter>', hover_Accl_en, add='+')
        self.accl_en_switch.bind('<Leave>', clear_text, add='+')
        self.Replicated_Accl_en_switch.bind('<Enter>', hover_Replicated_Accl_en, add='+')
        self.Replicated_Accl_en_switch.bind('<Leave>', clear_text, add='+')
        self.Multithreaded_Accl_en_switch.bind('<Enter>', hover_Multithreaded_Accl_en, add='+')
        self.Multithreaded_Accl_en_switch.bind('<Leave>', clear_text, add='+')
        self.SIMD_label.bind('<Enter>', hover_SIMD, add='+')
        self.SIMD_label.bind('<Leave>', clear_text, add='+')
        self.SIMD_combobox.bind('<Enter>', hover_SIMD, add='+')
        self.SIMD_combobox.bind('<Leave>', clear_text, add='+')
        self.SPM_NUM_label.bind('<Enter>', hover_SPM_NUM, add='+')
        self.SPM_NUM_label.bind('<Leave>', clear_text, add='+')
        self.SPM_NUM_combobox.bind('<Enter>', hover_SPM_NUM, add='+')
        self.SPM_NUM_combobox.bind('<Leave>', clear_text, add='+')
        self.SPM_SIZE_label.bind('<Enter>', hover_SPM_SIZE, add='+')
        self.SPM_SIZE_label.bind('<Leave>', clear_text, add='+')
        self.SPM_SIZE_combobox.bind('<Enter>', hover_SPM_SIZE, add='+')
        self.SPM_SIZE_combobox.bind('<Leave>', clear_text, add='+')
        self.SPM_START_ADDR_label.bind('<Enter>', hover_SPM_START_ADDR, add='+')
        self.SPM_START_ADDR_label.bind('<Leave>', clear_text, add='+')
        self.SPM_START_ADDR_textbox.bind('<Enter>', hover_SPM_START_ADDR, add='+')
        self.SPM_START_ADDR_textbox.bind('<Leave>', clear_text, add='+')

        self.mcycle_en_switch.bind('<Enter>', hover_mcycle_en)
        self.mcycle_en_switch.bind('<Leave>', clear_text)
        self.minstret_en_switch.bind('<Enter>', hover_minstret_en)
        self.minstret_en_switch.bind('<Leave>', clear_text)
        self.mhpmcounter_en_switch.bind('<Enter>', hover_mhpmcounter_en)
        self.mhpmcounter_en_switch.bind('<Leave>', clear_text)
        self.count_all_switch.bind('<Enter>', hover_count_all)
        self.count_all_switch.bind('<Leave>', clear_text)
        self.debug_en_switch.bind('<Enter>', hover_debug_en)
        self.debug_en_switch.bind('<Leave>', clear_text)
        self.tracer_en_switch.bind('<Enter>', hover_tracer_en)
        self.tracer_en_switch.bind('<Leave>', clear_text)

        self.riscy_switch.bind('<Button-1>', disable_klessydra, add='+')
        self.riscy_switch.bind('<Button-1>', enable_riscy, add='+')
        self.riscy_switch.bind('<Button-1>', disable_riscy, add='+')
        self.riscy_switch.bind('<Enter>', hover_riscy, add='+')
        self.riscy_switch.bind('<Leave>', clear_text, add='+')
        self.pulpino_core_label.bind('<Enter>', hover_pulpino_core, add='+')
        self.pulpino_core_label.bind('<Leave>', clear_text, add='+')
        self.pulpino_core_optionmenu.bind('<Enter>', hover_pulpino_core, add='+')
        self.pulpino_core_optionmenu.bind('<Leave>', clear_text, add='+')
        self.zero_rv32e_switch.bind('<Enter>', hover_zero_rv32e, add='+')
        self.zero_rv32e_switch.bind('<Leave>', clear_text, add='+')
        self.zero_rv32m_switch.bind('<Enter>', hover_zero_rv32m, add='+')
        self.zero_rv32m_switch.bind('<Leave>', clear_text, add='+')
        self.ri5cy_rv32f_switch.bind('<Enter>', hover_ri5cy_rv32f, add='+')
        self.ri5cy_rv32f_switch.bind('<Leave>', clear_text, add='+')
        self.pulpino_rvc_switch.bind('<Enter>', hover_pulpino_rvc, add='+')
        self.pulpino_rvc_switch.bind('<Leave>', clear_text, add='+')

        self.vsize_label.bind('<Enter>', hover_vsize, add='+')
        self.vsize_label.bind('<Leave>', clear_text, add='+')
        self.vsize_textbox.bind('<Enter>', hover_vsize, add='+')
        self.vsize_textbox.bind('<Leave>', clear_text, add='+')

        if (self.klessydra_core_optionmenu.get() == "Klessydra-Morph"):
            self.textbox.configure(state="disabled")
            disable_riscy(self)
            self.klessydra_switch.select()
            self.KLESS_RVM_switch.select()
            self.THREAD_POOL_SIZE_combobox.set(3)
            self.Morph_en_switch.select()
            self.superscalar_en_switch.select()
            self.accl_en_switch.select()
            self.Replicated_Accl_en_switch.select()
            self.btb_len_combobox.set(64)
            self.SIMD_combobox.set(2)
            self.SPM_NUM_combobox.set(4)
            self.SPM_SIZE_combobox.set("8192")

    def open_input_dialog_event(self):
        dialog = customtkinter.CTkInputDialog(text="Type in a number:", title="CTkInputDialog")
        print("CTkInputDialog:", dialog.get_input())

    def change_appearance_mode_event(self, new_appearance_mode: str):
        customtkinter.set_appearance_mode(new_appearance_mode)
        #if (self.appearance_mode_optionemenu.get() == "Light"):
        #    klessydra_logo_button(fg_color="#dedede", hover_color="#dedede")
        #elif (self.appearance_mode_optionemenu.get() == "Dark"):
        #    klessydra_logo_button(fg_color="#343638", hover_color="#343638")

    def change_scaling_event(self, new_scaling: str):
        if (self.scaling_optionemenu.get() == "120%"):
                self.geometry(f"{1380}x{775}")
        elif (self.scaling_optionemenu.get() == "110%"):
                self.geometry(f"{1280}x{725}")
        elif (self.scaling_optionemenu.get() == "100%"):
                self.geometry(f"{1180}x{675}")
        elif (self.scaling_optionemenu.get() == "90%"):
                self.geometry(f"{1080}x{625}")
        elif (self.scaling_optionemenu.get() == "80%"):
                self.geometry(f"{980}x{575}")
        new_scaling_float = int(new_scaling.replace("%", "")) / 100
        customtkinter.set_widget_scaling(new_scaling_float)


    def cmake_button_event(self):
        automate_en = "1"

        USE_RI5CY = "0"
        USE_ZERO_RISCY = "0"
        USE_KLESSYDRA = "0"
        USE_KLESSYDRA_M = "0"
        USE_KLESSYDRA_T0_2TH = "0"
        USE_KLESSYDRA_T0_3TH = "0"
        USE_KLESSYDRA_T1_3TH = "0"
        USE_KLESSYDRA_S1 = "0"
        USE_KLESSYDRA_OoO = "0"
        USE_KLESSYDRA_F0_3TH = "0"
        USE_KLESSYDRA_FT13 = "0"
        USE_KLESSYDRA_NETLIST = "0"
        KLESS_CONTEXT_SWITCH = "0"

        KLESS_RV32E = str(int(self.KLESS_RVE_switch.get()))
        KLESS_RV32M = str(int(self.KLESS_RVM_switch.get()))

        KLESS_THREAD_POOL_SIZE = self.THREAD_POOL_SIZE_combobox.get()
        KLESS_LUTRAM_RF = str(int(self.Lutram_RF_switch.get()))
        KLESS_LATCH_RF = str(int(self.Latch_RF_switch.get()))
        KLESS_superscalar_exec_en = str(int(self.superscalar_en_switch.get()))
        KLESS_morph_en = str(int(self.Morph_en_switch.get()))
        KLESS_fetch_stage_en = str(int(self.fetch_stage_en_switch.get()))
        KLESS_branch_predict_en = str(int(self.branch_predict_en_switch.get()))
        KLESS_btb_en = str(int(self.btb_en_switch.get()))
        KLESS_btb_len = str(int(math.log2(int(self.btb_len_combobox.get()))))

        KLESS_accl_en = str(int(self.accl_en_switch.get()))
        KLESS_replicate_accl_en = str(int(self.Replicated_Accl_en_switch.get()))
        KLESS_multithreaded_accl_en = str(int(self.Multithreaded_Accl_en_switch.get()))
        KLESS_SPM_NUM= self.SPM_NUM_combobox.get()
        KLESS_Addr_Width =  str(int(math.log2(int(self.SPM_SIZE_combobox.get()))))
        ##   KLESS_SPM_STRT_ADDR
        KLESS_SIMD = self.SIMD_combobox.get()

        KLESS_MCYCLE_EN = str(int(self.mcycle_en_switch.get()))
        KLESS_MINSTRET_EN = str(int(self.minstret_en_switch.get()))
        KLESS_MHPMCOUNTER_EN = str(int(self.mhpmcounter_en_switch.get()))
        KLESS_count_all = str(int(self.count_all_switch.get()))

        KLESS_debug_en = str(int(self.debug_en_switch.get()))
        KLESS_tracer_en= str(int(self.tracer_en_switch.get()))

        RVC = str(int(self.pulpino_rvc_switch.get()))
        RISCY_RV32F = str(int(self.ri5cy_rv32f_switch.get()))
        ZERO_RV32E = str(int(self.zero_rv32e_switch.get()))
        ZERO_RV32M = str(int(self.zero_rv32m_switch.get()))

        FM_Size = "21"
        Filter_Size = "3"
        VSIZE = self.vsize_textbox.get("0.0", "end")
        TIME = "0"
        COREMARK_ITR = "1"

        if (self.klessydra_switch.get() == 1):
            USE_KLESSYDRA = "1"
            USE_ZERO_RISCY = "0"
            USE_RISCY = "0"
            USE_RI5CY
            if (self.klessydra_core_optionmenu.get() == "Klessydra-Morph"):
                USE_KLESSYDRA_M = "1"
            elif (self.klessydra_core_optionmenu.get() == "Klessydra-HetC"):
                KLESS_CONTEXT_SWITCH = "1"
                USE_KLESSYDRA_M = "1"
            elif (self.klessydra_core_optionmenu.get() == "Klessydra-T13"):
                USE_KLESSYDRA_T1_3TH = "1"
            elif (self.klessydra_core_optionmenu.get() == "Klessydra-T03"):
                USE_KLESSYDRA_T0_3TH = "1"
            elif (self.klessydra_core_optionmenu.get() == "Klessydra-T02"):
                USE_KLESSYDRA_T0_2TH = "1"
            elif (self.klessydra_core_optionmenu.get() == "Klessydra-S1"):
                USE_KLESSYDRA_S1 = "1"
            elif (self.klessydra_core_optionmenu.get() == "Klessydra-OoO"):
                USE_KLESSYDRA_OoO = "1"
            elif (self.klessydra_core_optionmenu.get() == "Klessydra-F03"):
                USE_KLESSYDRA = "1"
                USE_KLESSYDRA_F0_3TH = "1"
            elif (self.klessydra_core_optionmenu.get() == "Klessydra-FT13"):
                USE_KLESSYDRA_FT13 = "1"

        elif (self.riscy_switch.get() == 1):
            USE_KLESSYDRA = "0"
            USE_KLESSYDRA = "0"
            USE_KLESSYDRA_M = "0"
            USE_KLESSYDRA_T0_2TH = "0"
            USE_KLESSYDRA_T0_3TH = "0"
            USE_KLESSYDRA_T1_3TH = "0"
            USE_KLESSYDRA_S1 = "0"
            USE_KLESSYDRA_OoO = "0"
            USE_KLESSYDRA_F0_3TH = "0"
            USE_KLESSYDRA_FT13 = "0"
            USE_KLESSYDRA_NETLIST = "0"
            if (self.pulpino_core_optionmenu.get() == "RI5CY"):
                USE_RI5CY = "1"
            elif (self.pulpino_core_optionmenu.get() == "ZERO-RISCY"):
                USE_ZERO_RISCY = "1"


        automate_en=os.putenv("automate_en", automate_en)

        USE_RI5CY=os.putenv("USE_RI5CY", USE_RI5CY)
        USE_ZERO_RISCY=os.putenv("USE_ZERO_RISCY", USE_ZERO_RISCY)

        USE_KLESSYDRA=os.putenv("USE_KLESSYDRA", USE_KLESSYDRA)
        USE_KLESSYDRA_M=os.putenv("USE_KLESSYDRA_M", USE_KLESSYDRA_M)
        USE_KLESSYDRA_OoO=os.putenv("USE_KLESSYDRA_OoO", USE_KLESSYDRA_OoO)
        USE_KLESSYDRA_T1_3TH=os.putenv("USE_KLESSYDRA_T1_3TH", USE_KLESSYDRA_T1_3TH)
        USE_KLESSYDRA_T0_2TH=os.putenv("USE_KLESSYDRA_T0_2TH", USE_KLESSYDRA_T0_2TH)
        USE_KLESSYDRA_T0_3TH=os.putenv("USE_KLESSYDRA_T0_3TH", USE_KLESSYDRA_T0_3TH)
        USE_KLESSYDRA_S1=os.putenv("USE_KLESSYDRA_S1", USE_KLESSYDRA_S1)
        USE_KLESSYDRA_F0_3TH=os.putenv("USE_KLESSYDRA_F0_3TH", USE_KLESSYDRA_F0_3TH)
        USE_KLESSYDRA_FT13=os.putenv("USE_KLESSYDRA_FT13", USE_KLESSYDRA_FT13)
        USE_KLESSYDRA_NETLIST=os.putenv("USE_KLESSYDRA_NETLIST", USE_KLESSYDRA_NETLIST)
        KLESS_CONTEXT_SWITCH=os.putenv("KLESS_CONTEXT_SWITCH", KLESS_CONTEXT_SWITCH)

        KLESS_RV32E=os.putenv("KLESS_RV32E", KLESS_RV32E)
        KLESS_RV32M=os.putenv("KLESS_RV32M", KLESS_RV32M)

        KLESS_THREAD_POOL_SIZE=os.putenv("KLESS_THREAD_POOL_SIZE", KLESS_THREAD_POOL_SIZE)
        KLESS_LUTRAM_RF=os.putenv("KLESS_LUTRAM_RF", KLESS_LUTRAM_RF)
        KLESS_LATCH_RF=os.putenv("KLESS_LATCH_RF", KLESS_LATCH_RF)
        KLESS_superscalar_exec_en=os.putenv("KLESS_superscalar_exec_en", KLESS_superscalar_exec_en)
        KLESS_morph_en=os.putenv("KLESS_morph_en", KLESS_morph_en)
        KLESS_fetch_stage_en=os.putenv("KLESS_fetch_stage_en", KLESS_fetch_stage_en)
        KLESS_branch_predict_en=os.putenv("KLESS_branch_predict_en", KLESS_branch_predict_en)
        KLESS_btb_en=os.putenv("KLESS_btb_en", KLESS_btb_en)
        KLESS_btb_len=os.putenv("KLESS_btb_len", KLESS_btb_len)

        KLESS_accl_en=os.putenv("KLESS_accl_en", KLESS_accl_en)
        KLESS_replicate_accl_en=os.putenv("KLESS_replicate_accl_en", KLESS_replicate_accl_en)
        KLESS_multithreaded_accl_en=os.putenv("KLESS_multithreaded_accl_en", KLESS_multithreaded_accl_en)
        KLESS_SPM_NUM=os.putenv("KLESS_SPM_NUM", KLESS_SPM_NUM)
        KLESS_Addr_Width=os.putenv("KLESS_Addr_Width", KLESS_Addr_Width)
        KLESS_SIMD=os.putenv("KLESS_SIMD", KLESS_SIMD)

        KLESS_MCYCLE_EN=os.putenv("KLESS_MCYCLE_EN", KLESS_MCYCLE_EN)
        KLESS_MINSTRET_EN=os.putenv("KLESS_MINSTRET_EN", KLESS_MINSTRET_EN)
        KLESS_MHPMCOUNTER_EN=os.putenv("KLESS_MHPMCOUNTER_EN", KLESS_MHPMCOUNTER_EN)
        KLESS_count_all=os.putenv("KLESS_count_all", KLESS_count_all)

        KLESS_debug_en=os.putenv("KLESS_debug_en", KLESS_debug_en)
        KLESS_tracer_en=os.putenv("KLESS_tracer_en", KLESS_tracer_en)

        RVC=os.putenv("RVC", RVC)
        RISCY_RV32F=os.putenv("RISCY_RV32F", RISCY_RV32F)
        ZERO_RV32E=os.putenv("ZERO_RV32E", ZERO_RV32E)
        ZERO_RV32M=os.putenv("ZERO_RV32M", ZERO_RV32M)

        FM_Size = os.putenv("FM_Size", FM_Size)
        Filter_Size = os.putenv("Filter_Size", Filter_Size)
        VSIZE = os.putenv("VSIZE", VSIZE)
        TIME = os.putenv("TIME", TIME)
        COREMARK_ITR = os.putenv("COREMARK_ITR", COREMARK_ITR)


        os.system("cd ../sw/build; ./cmake_configure.klessydra-m.gcc.sh $automate_en")

    def compile_rtl_button_event(self):
        os.system("cd ../sw/build; make -j`nproc` vcompile &")

    def compile_core_button_event(self):
        os.system("cd ../sw/build; make -j'nproc` vcompile &")

    def build_tests_button_event(self):
        os.system("cd ../sw/build; make -j`nproc` all &")

    def compile_and_run_button_event(self):
        cmd = self.command_line.get()
        self.command_line.delete("0", tkinter.END)
        cmd=os.putenv("cmd", cmd)
        os.system("cd ../sw/build; $cmd")

    def sidebar_button_event(self):
        print("sidebar_button click")


if __name__ == "__main__":
    app = App()
    app.mainloop()