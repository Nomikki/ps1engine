# Compiler and flags
CXX := g++
CXXFLAGS := -Iinclude -Llib -Os -s -O3 -march=native -ffast-math -funroll-loops -std=c++17 -msse
LDFLAGS := -lsfml-graphics -lsfml-window -lsfml-system

# Directories and output
SRCDIR := src
OBJDIR := obj
BUILDDIR := build
TARGET := $(BUILDDIR)/ps1_engine

# Detect operating system
ifeq ($(OS),Windows_NT)
    EXE_EXT := .exe
    RM := rmdir /s /q
    MKDIR := mkdir
else
    EXE_EXT :=
    RM := rm -rf
    MKDIR := mkdir -p
endif

TARGET := $(TARGET)$(EXE_EXT)

# Source and object files
SOURCES := main.cpp engine.cpp utility.cpp mesh.cpp meshManager.cpp component.cpp \
           componentManager.cpp transform.cpp camera.cpp
OBJECTS := $(addprefix $(OBJDIR)/, $(SOURCES:.cpp=.o))

# Default target
all: $(TARGET)

# Link target
$(TARGET): $(OBJECTS) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source to object
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure object and build directories exist
$(OBJDIR) $(BUILDDIR):
	$(MKDIR) $@

# Clean up build artifacts
clean:
	$(RM) $(OBJDIR)
	$(RM) $(BUILDDIR)


rebuild: clean all