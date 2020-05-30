##############################################################################
# Directories
##############################################################################

# Repo Directories
APPS_DIR			:= $(REPO_ROOT)/apps
APPS_TEST_DIR		:= $(REPO_ROOT)/apps_test
CORE_CSIRO_DIR 		:= $(REPO_ROOT)/core_csiro
CORE_EXTERNAL_DIR 	:= $(REPO_ROOT)/core_external
TEST_DIR			:= $(REPO_ROOT)/unit_tests
TOOLS_DIR			:= $(REPO_ROOT)/tools

# Application Directories
SRC_DIR             := $(APP_ROOT)/src
INC_DIR             := $(APP_ROOT)/inc

# Output Directories
BUILD_DIR			:= $(REPO_ROOT)/build/$(BUILD_MODE)
ARCH_LIB_DIR		=  $(BUILD_DIR)/lib_$(CPU_ARCH)
PLATFORM_LIB_DIR	:= $(BUILD_DIR)/$(TARGET)/lib
OBJ_DIR				:= $(BUILD_DIR)/$(TARGET)/obj/$(PROJ_NAME)

# Core CSIRO Directories
CSIRO_ARCH_DIR 		= $(CORE_CSIRO_DIR)/arch/$(CPU_ARCH)
CSIRO_PLATFORM_DIR 	:= $(CORE_CSIRO_DIR)/platform
CSIRO_TARGET_DIR	:= $(CSIRO_PLATFORM_DIR)/$(TARGET)
CSIRO_PERIPH_DIR 	:= $(CORE_CSIRO_DIR)/peripherals
CSIRO_TASK_DIR 		:= $(CORE_CSIRO_DIR)/tasks
CSIRO_SCHEDULER_DIR := $(CORE_CSIRO_DIR)/scheduler
CSIRO_RPC_DIR 		:= $(CORE_CSIRO_DIR)/rpc

# External Core Library Directories
FREERTOS_DIR      	:= $(CORE_EXTERNAL_DIR)/FreeRTOS/Source
FREERTOS_CLI_DIR    := $(CORE_EXTERNAL_DIR)/FreeRTOS-Plus-CLI
PRINTF_DIR			:= $(CORE_EXTERNAL_DIR)/tiny_printf
XTI_DIR             := $(CORE_EXTERNAL_DIR)/pacp-xti/release/c_lib
VM_DIR              := $(CORE_EXTERNAL_DIR)/pacp-vm/release
LOCALISATION_DIR    := $(CORE_EXTERNAL_DIR)/bleat-localisation-api/release/c_lib
SWD_DIR				:= $(CORE_EXTERNAL_DIR)/RTT
UTF_DIR				:= $(CORE_EXTERNAL_DIR)/utf
MBEDTLS_DIR			:= $(CORE_EXTERNAL_DIR)/mbedtls

CMSIS_DIR			:= $(CORE_EXTERNAL_DIR)/CMSIS_5/CMSIS
CMSIS_CORE_DIR		:= $(CMSIS_DIR)/Core
CMSIS_DSP_DIR		:= $(CMSIS_DIR)/DSP

# My OS LIB Directories
MY_OS_LIB_CLI_DIR   := $(MY_OS_LIB_ROOT)/cli
MY_OS_LIB_TIMER_DIR := $(MY_OS_LIB_ROOT)/timer
MY_OS_LIB_LOG_DIR   := $(MY_OS_LIB_ROOT)/log
MY_OS_LIB_LED_DIR   := $(MY_OS_LIB_ROOT)/led
MY_OS_LIB_HCI_DIR   := $(MY_OS_LIB_ROOT)/hci
MY_OS_LIB_BLE_DIR   := $(MY_OS_LIB_ROOT)/ble
MY_OS_LIB_WIFI_DIR  := $(MY_OS_LIB_ROOT)/wifi
MY_OS_LIB_ULTRARANGER_DIR  := $(MY_OS_LIB_ROOT)/ultraRanger
##############################################################################
