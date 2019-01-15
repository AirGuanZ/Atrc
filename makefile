CPPC = clang++
CPPC_INCLUDE_FLAGS = -I ./Source/ -I $(AGZ_UTILS_HOME)
CPPC_FLAGS = $(CPPC_INCLUDE_FLAGS) -std=c++17 -Wall -Wextra -O2
ifeq ($(CPPC),clang++)
	CPPC_FLAGS += -stdlib=libc++
endif
LD_FLAGS = -pthread

EMPTY =
TARGETS =

CPP_FILES = $(shell find ./Source/Atrc/Lib/ -name "*.cpp")
CPP_FILES += $(shell find ./Source/Atrc/Mgr/ -name "*.cpp")
OPP_FILES = $(patsubst %.cpp, %.o, $(CPP_FILES))
DPP_FILES = $(patsubst %.cpp, %.d, $(CPP_FILES))

LIB = ./Build/Lib.a

CPP_SUFFIX = _CPP_FILES
O_SUFFIX = _O_FILES
D_SUFFIX = _D_FILES

ALL_OPP_FILES = $(OPP_FILES)
ALL_DPP_FILES = $(DPP_FILES)

$(LIB) : $(OPP_FILES)
	ar -r $@ $^

# 1: target name
# 2: target filename
# 3: source directory
# 4: additional cc flags
# 5: additional link flags
# 6: additional link deps
define add_target

$(2) = ./Build/$(1)

.PHONY : $(1)
$(1) : $$($(2))

.PHONY : run$(1)
run$(1) :
	make $(1)
	$$($(2))

$(2)$(CPP_SUFFIX) = $$(shell find ./Source/Atrc/$(3)/ -name "*.cpp")
$(2)$(O_SUFFIX) = $$(patsubst %.cpp, %.o, $$($(2)$(CPP_SUFFIX)))
$(2)$(D_SUFFIX) = $$(patsubst %.cpp, %.d, $$($(2)$(CPP_SUFFIX)))

$$($(2)) : $$($(2)$(O_SUFFIX)) $$($(6))
	$$(CPPC) $(CPPC_FLAGS) $(LD_FLAGS) $$($(2)$(O_SUFFIX)) $$($(5)) -o $$@

$$($(2)$(O_SUFFIX)) : %.o : %.cpp
	$$(CPPC) $$(CPPC_FLAGS) $$($(4)) -c $$< -o $$@

$$($(2)$(D_SUFFIX)) : %.d : %.cpp
	@set -e; \
	rm -f $$@; \
	$$(CPPC) -MM $$(CPPC_FLAGS) $$($(4)) $$< $$(CPPC_INCLUDE_FLAGS) > $$@.$$$$$$$$.dtmp; \
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
$(eval $(call add_target,Launcher,LAUNCHER_TARGET,Launcher,EMPTY,LIB,LIB))

# SH projection and reconstruction
$(eval $(call add_target,SH2D,SH2D_TARGET,SH2D,EMPTY,LIB,LIB))

# Sample model viewer
MODEL_VIEWER_CC_FLAGS = -I ./Library/Include
MODEL_VIEWER_LD_FLAGS = $(shell pkg-config --static --libs glew glfw3)
$(eval $(call add_target,ModelViewer,MODEL_VIEWER,ModelViewer,MODEL_VIEWER_CC_FLAGS,MODEL_VIEWER_LD_FLAGS))

.PHONT : all
all :
	make $(TARGETS)

.PHONY : clean
clean :
	rm -f $(LIB) $(TARGETS)
	rm -f $(ALL_OPP_FILES) $(ALL_DPP_FILES)
	rm -f $(shell find ./Source/ -name "*.dtmp")
