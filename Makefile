# Project Name
TARGET = Main

# Sources
CPP_SOURCES = src/Main.cpp src/PitchShifter.cpp src/Harmonizer.cpp

# Library Locations
LIBDAISY_DIR = libDaisy
DAISYSP_DIR = DaisySP

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

