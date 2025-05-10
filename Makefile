FLAGS := -Iinclude  -Llib -Os -s -O3 -O3 -march=native -ffast-math -funroll-loops -std=c++17 -msse
LDLINKS := -lsfml-graphics -lsfml-window -lsfml-system

# Detect operating system
ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows
    EXE_EXT := .exe
    RM_CMD := rmdir /s /q
    MKDIR_CMD := mkdir
    RUN_PREFIX := cmd /C
else
    DETECTED_OS := $(shell uname -s)
    EXE_EXT :=
    RM_CMD := rm -rf
    MKDIR_CMD := mkdir -p
    RUN_PREFIX :=
endif

all:

CFILES  := main.cpp engine.cpp utility.cpp mesh.cpp meshManager.cpp component.cpp \
					componentManager.cpp transform.cpp camera.cpp
COMPILER := g++
SRCDIR := src
OBJDIR := obj
EXE := ps1_engine$(EXE_EXT)
BUILDDIR := build
OUTPUT := $(BUILDDIR)/$(EXE)

target = ${OBJDIR}/$(patsubst %.cpp,%.o,$(notdir ${1}))
obj.cpp :=

define obj
  $(call target,${1}) : ${SRCDIR}/${1} | ${OBJDIR}
  obj$(suffix ${1}) += $(call target,${1})
endef

define SOURCES
	$(foreach src,${src}${1},$(eval $(call obj,${src})))
endef

$(eval $(call SOURCES,${CFILES}))

all : ${obj.cpp} build2

${obj.cpp} : % :
	$(COMPILER) $(FLAGS)  -c $^ -o $@

${OBJDIR} :
	$(MKDIR_CMD) $@

build2: 
	$(COMPILER) $(FLAGS) -o $(OUTPUT) $(obj.cpp) $(LDLINKS)

clean:	
	$(RM_CMD) obj
	$(MKDIR_CMD) obj

