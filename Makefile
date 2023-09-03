FLAGS := -Iinclude  -Llib -Os -s -O3 -O3 -march=native -ffast-math -funroll-loops -std=c++17 -msse
LDLINKS := -lsfml-graphics -lsfml-window -lsfml-system

all:

CFILES  := main.cpp engine.cpp utility.cpp mesh.cpp meshManager.cpp component.cpp \
					componentManager.cpp transform.cpp camera.cpp
COMPILER := g++
SRCDIR := src
OBJDIR := obj
EXE := ps1_clone.exe
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

all : ${obj.cpp} build2 run
	

${obj.cpp} : % :
	$(COMPILER) $(FLAGS)  -c $^ -o $@

${OBJDIR} :
	mkdir $@

build2: 
	
	$(COMPILER) $(FLAGS) -o $(OUTPUT) $(obj.cpp) $(LDLINKS)

run:
	cmd /C "cd $(BUILDDIR) && $(EXE)"

clean:	
	rmdir /s /q obj
	mkdir obj

