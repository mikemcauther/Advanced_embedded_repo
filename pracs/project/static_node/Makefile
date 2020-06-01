##############################################################################
# Application Settings
##############################################################################

PROJ_NAME     		:= staticNode
SUPPORTED_TARGETS 	:= nrf52840dk xenon

##############################################################################
# Application Specific Flags
##############################################################################

APP_CFLAGS			:=

##############################################################################
# Application Specific Library Source Files
##############################################################################

CORE_CSIRO          := $(realpath ./../../core_csiro)
APPLICATION_SRCS 	:=

MY_OS_LIBS   := MY_OS_LIB_CLI MY_OS_LIB_LED MY_OS_LIB_LOG MY_OS_LIB_TIMER MY_OS_LIB_HCI MY_OS_LIB_BLE MY_OS_LIB_ULTRARANGER
##############################################################################
# Main Build Rules
##############################################################################

include ../../../common/proj_common.mk
include $(REPO_ROOT)/core_csiro/m_common.mk
