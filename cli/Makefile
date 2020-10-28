SRCDIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
JULIAHOME := $(abspath $(SRCDIR)/..)
BUILDDIR ?= .
include $(JULIAHOME)/deps/Versions.make
include $(JULIAHOME)/Make.inc
include $(JULIAHOME)/deps/llvm-ver.make


HEADERS := $(addprefix $(JULIAHOME)/src/support/,platform.h dirpath.h)

LOADER_CFLAGS = $(JCFLAGS) -I$(BUILDROOT)/src -I$(JULIAHOME)/src -I$(JULIAHOME)/src/support -I$(build_includedir) -ffreestanding
LOADER_LDFLAGS = $(JLDFLAGS) -ffreestanding -L$(build_shlibdir)

ifeq ($(OS),WINNT)
LOADER_CFLAGS += -municode -mconsole -nostdlib -fno-stack-check -fno-stack-protector -mno-stack-arg-probe
endif

ifeq ($(OS),WINNT)
LOADER_LDFLAGS += -municode -mconsole -nostdlib --disable-auto-import \
                  --disable-runtime-pseudo-reloc -lntdll -lkernel32 -lpsapi
else ifeq ($(OS),Linux)
LOADER_LDFLAGS += -Wl,--no-as-needed -ldl -lpthread -rdynamic -lc -Wl,--as-needed
else ifeq ($(OS),FreeBSD)
LOADER_LDFLAGS += -Wl,--no-as-needed -ldl -lpthread -rdynamic -lc -Wl,--as-needed
else ifeq ($(OS),Darwin)
LOADER_LDFLAGS += -lSystem
endif

# Build list of dependent libraries that must be opened
SHIPFLAGS  += -DDEP_LIBS="\"$(LOADER_BUILD_DEP_LIBS)\""
DEBUGFLAGS += -DDEP_LIBS="\"$(LOADER_DEBUG_BUILD_DEP_LIBS)\""

EXE_OBJS := $(BUILDDIR)/loader_exe.o
EXE_DOBJS := $(BUILDDIR)/loader_exe.dbg.obj
LIB_OBJS := $(BUILDDIR)/loader_lib.o
LIB_DOBJS := $(BUILDDIR)/loader_lib.dbg.obj

default: release
all: release debug
release debug :  % : julia-% libjulia-%

$(BUILDDIR)/%.o : $(SRCDIR)/%.c $(HEADERS)
	@$(call PRINT_CC, $(CC) $(SHIPFLAGS) $(LOADER_CFLAGS) -c $< -o $@)
$(BUILDDIR)/%.dbg.obj : $(SRCDIR)/%.c $(HEADERS)
	@$(call PRINT_CC, $(CC) $(DEBUGFLAGS) $(LOADER_CFLAGS) -c $< -o $@)

ifeq ($(OS),WINNT)
ifneq ($(USEMSVC), 1)
$(BUILDDIR)/julia_res.o: $(JULIAHOME)/contrib/windows/julia.rc $(JULIAHOME)/VERSION
	JLVER=`cat $(JULIAHOME)/VERSION` && \
	JLVERi=`echo $$JLVER | perl -nle \
		'/^(\d+)\.?(\d*)\.?(\d*)/ && \
		print int $$1,",",int $$2,",0,",int $$3'` && \
	$(CROSS_COMPILE)windres $< -O coff -o $@ -DJLVER=$$JLVERi -DJLVER_STR=\\\"$$JLVER\\\"
EXE_OBJS += $(BUILDDIR)/julia_res.o
EXE_DOBJS += $(BUILDDIR)/julia_res.o
endif
endif

ifneq ($(OS),WINNT)
HIDDEN_SYMBOL := __attribute__((visibility(\"hidden\")))
endif

# Embed an Info.plist in the julia executable
# Create an intermediate target Info.plist for Darwin code signing.
ifeq ($(DARWIN_FRAMEWORK),1)
$(BUILDDIR)/Info.plist: $(JULIAHOME)/VERSION
	/usr/libexec/PlistBuddy -x -c "Clear dict" $@
	/usr/libexec/PlistBuddy -x -c "Add :CFBundleName string julia" $@
	/usr/libexec/PlistBuddy -x -c "Add :CFBundleIdentifier string $(darwin_codesign_id_julia_ui)" $@
	/usr/libexec/PlistBuddy -x -c "Add :CFBundleInfoDictionaryVersion string 6.0" $@
	/usr/libexec/PlistBuddy -x -c "Add :CFBundleVersion string $(JULIA_COMMIT)" $@
	/usr/libexec/PlistBuddy -x -c "Add :CFBundleShortVersionString string $(JULIA_MAJOR_VERSION).$(JULIA_MINOR_VERSION).$(JULIA_PATCH_VERSION)" $@
.INTERMEDIATE: $(BUILDDIR)/Info.plist # cleanup this file after we are done using it
JLDFLAGS += -Wl,-sectcreate,__TEXT,__info_plist,Info.plist
$(build_bindir)/julia$(EXE): $(BUILDDIR)/Info.plist
$(build_bindir)/julia-debug$(EXE): $(BUILDDIR)/Info.plist
endif

julia-release: $(build_bindir)/julia$(EXE)
julia-debug: $(build_bindir)/julia-debug$(EXE)
libjulia-release: $(build_shlibdir)/libjulia.$(JL_MAJOR_MINOR_SHLIB_EXT)
libjulia-debug: $(build_shlibdir)/libjulia-debug.$(JL_MAJOR_MINOR_SHLIB_EXT)

$(build_shlibdir)/libjulia.$(JL_MAJOR_MINOR_SHLIB_EXT): $(LIB_OBJS)
	@$(call PRINT_LINK, $(CC) $(LOADER_CFLAGS) -DLIBRARY_EXPORTS -shared $(SHIPFLAGS) $(LIB_OBJS) -o $@ $(LOADER_LDFLAGS) $(RPATH_LIB)) $(call SONAME_FLAGS,$@)
ifneq ($(OS), WINNT)
	@ln -sf $(notdir $@) $(build_shlibdir)/libjulia.$(JL_MAJOR_SHLIB_EXT)
	@ln -sf $(notdir $@) $(build_shlibdir)/libjulia.$(SHLIB_EXT)
endif


$(build_shlibdir)/libjulia-debug.$(JL_MAJOR_MINOR_SHLIB_EXT): $(LIB_DOBJS)
	@$(call PRINT_LINK, $(CC) $(LOADER_CFLAGS) -DLIBRARY_EXPORTS -shared $(DEBUGFLAGS) $(LIB_DOBJS) -o $@ $(LOADER_LDFLAGS) $(RPATH_LIB)) $(call SONAME_FLAGS,$@)
ifneq ($(OS), WINNT)
	@ln -sf $(notdir $@) $(build_shlibdir)/libjulia-debug.$(JL_MAJOR_SHLIB_EXT)
	@ln -sf $(notdir $@) $(build_shlibdir)/libjulia-debug.$(SHLIB_EXT)
endif

$(build_bindir)/julia$(EXE): $(EXE_OBJS) $(build_shlibdir)/libjulia.$(JL_MAJOR_MINOR_SHLIB_EXT)
	@$(call PRINT_LINK, $(CC) $(LOADER_CFLAGS) $(SHIPFLAGS) $(EXE_OBJS) -o $@ $(LOADER_LDFLAGS) $(RPATH) -ljulia)

$(build_bindir)/julia-debug$(EXE): $(EXE_DOBJS) $(build_shlibdir)/libjulia-debug.$(JL_MAJOR_MINOR_SHLIB_EXT)
	@$(call PRINT_LINK, $(CC) $(LOADER_CFLAGS) $(DEBUGFLAGS) $(EXE_DOBJS) -o $@ $(LOADER_LDFLAGS) $(RPATH) -ljulia-debug)

clean: | $(CLEAN_TARGETS)
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/*.dbg.obj
	rm -f $(build_bindir)/julia*

.PHONY: clean release debug julia-release julia-debug
