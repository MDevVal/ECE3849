################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
libraries/display/Crystalfontz128x128_ST7735.obj: C:/Users/Leo\ Riesenbach/Downloads/Lab_0/libraries/display/Crystalfontz128x128_ST7735.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O1 --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/lab_0_Workspace/lab0" --include_path="C:/ti/TivaWare_C_Series-2.2.0.295" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/HAL_TM4C1294" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/buttonsDriver" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/display" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/elapsedTime" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/joystickDriver" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/pll" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/timerLib" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA1 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="libraries/display/Crystalfontz128x128_ST7735.d_raw" --obj_directory="libraries/display" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

libraries/display/HAL_EK_TM4C1294XL_Crystalfontz128x128_ST7735.obj: C:/Users/Leo\ Riesenbach/Downloads/Lab_0/libraries/display/HAL_EK_TM4C1294XL_Crystalfontz128x128_ST7735.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O1 --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/lab_0_Workspace/lab0" --include_path="C:/ti/TivaWare_C_Series-2.2.0.295" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/HAL_TM4C1294" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/buttonsDriver" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/display" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/elapsedTime" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/joystickDriver" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/pll" --include_path="C:/Users/Leo Riesenbach/Downloads/Lab_0/libraries/timerLib" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA1 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="libraries/display/HAL_EK_TM4C1294XL_Crystalfontz128x128_ST7735.d_raw" --obj_directory="libraries/display" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


