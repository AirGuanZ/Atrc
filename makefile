CPPC = clang++
CPPC_INCLUDE_FLAGS = -I ./Source/ -I $(AGZ_UTILS_HOME)
CPPC_FLAGS = $(CPPC_INCLUDE_FLAGS) -stdlib=libc++ -std=c++17 -Werror -Wall -Wextra -O2
LD_FLAGS = -pthread

TARGETS =

CPP_FILES = $(shell find ./Source/Atrc/ -name "*.cpp")
CPP_FILES += $(shell find ./Source/ObjMgr/ -name "*.cpp")
OPP_FILES = $(patsubst %.cpp, %.o, $(CPP_FILES))
DPP_FILES = $(patsubst %.cpp, %.d, $(CPP_FILES))

ATRC = ./Build/atrc.a

CPP_SUFFIX = _CPP_FILES
O_SUFFIX = _O_FILES
D_SUFFIX = _D_FILES

ALL_OPP_FILES = $(OPP_FILES)
ALL_DPP_FILES = $(DPP_FILES)

$(ATRC) : $(OPP_FILES)
	ar -r $@ $^

define add_target

$(2) = ./Build/$(1)

.PHONY : $(1)
$(1) : $$($(2))

.PHONY : run$(1)
run$(1) :
	make $(1)
	$$($(2))

$(2)$(CPP_SUFFIX) = $$(shell find ./Source/$(3)/ -name "*.cpp")
$(2)$(O_SUFFIX) = $$(patsubst %.cpp, %.o, $$($(2)$(CPP_SUFFIX)))
$(2)$(D_SUFFIX) = $$(patsubst %.cpp, %.d, $$($(2)$(CPP_SUFFIX)))

$$($(2)) :$$($(2)$(O_SUFFIX)) $$(ATRC) 
	$$(CPPC) $(CPPC_FLAGS) $(LD_FLAGS) $$^ -o $$@

$$($(2)$(O_SUFFIX)) : %.o : %.cpp
	$$(CPPC) $$(CPPC_FLAGS) -c $$< -o $$@

$$($(2)$(D_SUFFIX)) : %.d : %.cpp
	@set -e; \
	rm -f $$@; \
	$$(CPPC) -MM $$(CPPC_FLAGS) $$< $$(CPPC_INCLUDE_FLAGS) > $$@.$$$$$$$$.dtmp; \
	sed 's,\(.*\)\.o\:,$$*\.o $$*\.d\:,g' < $$@.$$$$$$$$.dtmp > $$@; \
	rm -f $$@.$$$$$$$$.dtmp

-include $$($(2)$(D_SUFFIX))

ALL_OPP_FILES += $$($(2)$(O_SUFFIX))
ALL_DPP_FILES += $$($(2)$(D_SUFFIX))

TARGETS += ./Build/$(1)

endef

$(OPP_FILES) : %.o : %.cpp
	$(CPPC) $(CPPC_FLAGS) -c $< -o $@

$(DPP_FILES) : %.d : %.cpp
	@set -e; \
	rm -f $@; \
	$(CPPC) -MM $(CPPC_FLAGS) $< > $@.$$$$.dtmp; \
	sed 's,\(.*\)\.o\:,$*\.o $*\.d\:,g' < $@.$$$$.dtmp > $@; \
	rm -f $@.$$$$.dtmp

-include $(DPP_FILES)

# The main renderer launcher
$(eval $(call add_target,Launcher,LAUNCHER_TARGET,Tools/Launcher))
# Sphere harmonics projector and reconstructor
$(eval $(call add_target,SH,SH_TARGET,Tools/SH))
# Cube map to sphere map
$(eval $(call add_target,C2S,C2S_TARGET,Tools/C2S))

.PHONT : all
all :
	make Launcher SH C2S

.PHONY : clean
clean :
	rm -f $(ATRC) $(TARGETS)
	rm -f $(ALL_OPP_FILES) $(ALL_DPP_FILES)
	rm -f $(shell find ./Source/ -name "*.dtmp")
