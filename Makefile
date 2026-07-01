export TERMUX_EXEC_PKG__VERSION := 2.5.0
export TERMUX_EXEC_PKG__ARCH
export TERMUX_EXEC_PKG__INSTALL_PREFIX
export TERMUX_EXEC_PKG__TESTS__API_LEVEL :=

export TERMUX__NAME := Termux# Default value: `Termux`
export TERMUX__LNAME := termux# Default value: `termux`

export TERMUX__REPOS_HOST_ORG_NAME := termux# Default value: `termux`

export TERMUX_APP__NAME := Termux# Default value: `Termux`
export TERMUX_APP__PACKAGE_NAME := com.termux# Default value: `com.termux`
export TERMUX_APP__DATA_DIR := /data/data/$(TERMUX_APP__PACKAGE_NAME)# Default value: `/data/data/com.termux`

export TERMUX__ROOTFS := $(TERMUX_APP__DATA_DIR)/files# Default value: `/data/data/com.termux/files`
export TERMUX__PREFIX := $(TERMUX__ROOTFS)/usr# Default value: `/data/data/com.termux/files/usr`
export TERMUX__PREFIX__BIN_DIR := $(TERMUX__PREFIX)/bin# Default value: `/data/data/com.termux/files/usr/bin`
export TERMUX__PREFIX__INCLUDE_DIR := $(TERMUX__PREFIX)/include# Default value: `/data/data/com.termux/files/usr/include`
export TERMUX__PREFIX__LIB_DIR := $(TERMUX__PREFIX)/lib# Default value: `/data/data/com.termux/files/usr/lib`

export TERMUX_ENV__S_ROOT := TERMUX_# Default value: `TERMUX_`
export TERMUX_ENV__SS_TERMUX := _# Default value: `_`
export TERMUX_ENV__S_TERMUX := $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX)# Default value: `TERMUX__`
export TERMUX_ENV__SS_TERMUX_APP := APP__# Default value: `APP__`
export TERMUX_ENV__S_TERMUX_APP := $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX_APP)# Default value: `TERMUX_APP__`
export TERMUX_ENV__SS_TERMUX_ROOTFS := ROOTFS__# Default value: `ROOTFS__`
export TERMUX_ENV__S_TERMUX_ROOTFS := $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX_ROOTFS)# Default value: `TERMUX_ROOTFS__`
export TERMUX_ENV__SS_TERMUX_EXEC := EXEC__# Default value: `EXEC__`
export TERMUX_ENV__S_TERMUX_EXEC := $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX_EXEC)# Default value: `TERMUX_EXEC__`
export TERMUX_ENV__SS_TERMUX_EXEC__TESTS := EXEC__TESTS__# Default value: `EXEC__TESTS__`
export TERMUX_ENV__S_TERMUX_EXEC__TESTS := $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX_EXEC__TESTS)# Default value: `TERMUX_EXEC__TESTS__`


# If architecture not set, find it for the compiler based on which
# predefined architecture macro is defined. The `shell` function
# replaces newlines with a space and a literal space cannot be entered
# in a makefile as its used as a splitter, hence $(SPACE) variable is
# created and used for matching.
ifeq ($(TERMUX_EXEC_PKG__ARCH),)
	export override PREDEFINED_MACROS := $(shell $(CC) -x c /dev/null -dM -E)
	override EMPTY :=
	override SPACE := $(EMPTY) $(EMPTY)
	ifneq (,$(findstring $(SPACE)#define __i686__ 1$(SPACE),$(SPACE)$(PREDEFINED_MACROS)$(SPACE)))
		override TERMUX_EXEC_PKG__ARCH := i686
	else ifneq (,$(findstring $(SPACE)#define __x86_64__ 1$(SPACE),$(SPACE)$(PREDEFINED_MACROS)$(SPACE)))
		override TERMUX_EXEC_PKG__ARCH := x86_64
	else ifneq (,$(findstring $(SPACE)#define __aarch64__ 1$(SPACE),$(SPACE)$(PREDEFINED_MACROS)$(SPACE)))
		override TERMUX_EXEC_PKG__ARCH := aarch64
	else ifneq (,$(findstring $(SPACE)#define __arm__ 1$(SPACE),$(SPACE)$(PREDEFINED_MACROS)$(SPACE)))
		override TERMUX_EXEC_PKG__ARCH := arm
	else ifneq (,$(findstring $(SPACE)#define __riscv 1$(SPACE),$(SPACE)$(PREDEFINED_MACROS)$(SPACE)))
		override TERMUX_EXEC_PKG__ARCH := riscv64
	else
        $(error Unsupported package arch)
	endif
endif


export override IS_ON_DEVICE_BUILD := $(shell test -f "/system/bin/app_process" && echo 1 || echo 0)



export override BUILD_DIR := build# Default value: `build`

export override BUILD_OUTPUT_DIR := $(BUILD_DIR)/output# Default value: `build/output`

export override TMP_BUILD_OUTPUT_DIR := $(BUILD_OUTPUT_DIR)/tmp# Default value: `build/output/tmp`

export override PREFIX_BUILD_OUTPUT_DIR := $(BUILD_OUTPUT_DIR)/usr# Default value: `build/output/usr`
export override BIN_BUILD_OUTPUT_DIR := $(PREFIX_BUILD_OUTPUT_DIR)/bin# Default value: `build/output/usr/bin`
export override LIB_BUILD_OUTPUT_DIR := $(PREFIX_BUILD_OUTPUT_DIR)/lib# Default value: `build/output/usr/lib`
export override LIBEXEC_BUILD_OUTPUT_DIR := $(PREFIX_BUILD_OUTPUT_DIR)/libexec# Default value: `build/output/usr/libexec`
export override TESTS_BUILD_OUTPUT_DIR := $(LIBEXEC_BUILD_OUTPUT_DIR)/installed-tests/termux-exec# Default value: `build/output/usr/libexec/installed-tests/termux-exec`

export override PACKAGING_BUILD_OUTPUT_DIR := $(BUILD_OUTPUT_DIR)/packaging# Default value: `build/output/packaging`
export override DEBIAN_PACKAGING_BUILD_OUTPUT_DIR := $(PACKAGING_BUILD_OUTPUT_DIR)/debian# Default value: `build/output/packaging/debian`



export override BUILD_INSTALL_DIR := $(BUILD_DIR)/install# Default value: `build/install`
export override PREFIX_BUILD_INSTALL_DIR := $(BUILD_INSTALL_DIR)/usr# Default value: `build/install/usr`

ifeq ($(TERMUX_EXEC_PKG__INSTALL_PREFIX),)
	ifeq ($(DESTDIR)$(PREFIX),)
		override TERMUX_EXEC_PKG__INSTALL_PREFIX := $(TERMUX__PREFIX)
	else
		override TERMUX_EXEC_PKG__INSTALL_PREFIX := $(DESTDIR)$(PREFIX)
	endif
endif
export TERMUX_EXEC_PKG__INSTALL_PREFIX



export override TERMUX__CONSTANTS__MACRO_FLAGS := \
	-DTERMUX_EXEC_PKG__VERSION=\"$(TERMUX_EXEC_PKG__VERSION)\" \
	-DTERMUX__NAME=\"$(TERMUX__NAME)\" \
	-DTERMUX__LNAME=\"$(TERMUX__LNAME)\" \
	-DTERMUX__REPOS_HOST_ORG_NAME=\"$(TERMUX__REPOS_HOST_ORG_NAME)\" \
	-DTERMUX_APP__DATA_DIR=\"$(TERMUX_APP__DATA_DIR)\" \
	-DTERMUX__ROOTFS=\"$(TERMUX__ROOTFS)\" \
	-DTERMUX__PREFIX=\"$(TERMUX__PREFIX)\" \
	-DTERMUX__PREFIX__BIN_DIR=\"$(TERMUX__PREFIX__BIN_DIR)\" \
	-DTERMUX_ENV__S_TERMUX=\"$(TERMUX_ENV__S_TERMUX)\" \
	-DTERMUX_ENV__S_TERMUX_APP=\"$(TERMUX_ENV__S_TERMUX_APP)\" \
	-DTERMUX_ENV__S_TERMUX_ROOTFS=\"$(TERMUX_ENV__S_TERMUX_ROOTFS)\" \
	-DTERMUX_ENV__S_TERMUX_EXEC=\"$(TERMUX_ENV__S_TERMUX_EXEC)\" \
	-DTERMUX_ENV__S_TERMUX_EXEC__TESTS=\"$(TERMUX_ENV__S_TERMUX_EXEC__TESTS)\"

export override TERMUX__CONSTANTS__SED_ARGS := \
	-e "s%[@]TERMUX_EXEC_PKG__VERSION[@]%$(TERMUX_EXEC_PKG__VERSION)%g" \
	-e "s%[@]TERMUX_EXEC_PKG__ARCH[@]%$(TERMUX_EXEC_PKG__ARCH)%g" \
	-e "s%[@]TERMUX__LNAME[@]%$(TERMUX__LNAME)%g" \
	-e "s%[@]TERMUX__REPOS_HOST_ORG_NAME[@]%$(TERMUX__REPOS_HOST_ORG_NAME)%g" \
	-e "s%[@]TERMUX_APP__NAME[@]%$(TERMUX_APP__NAME)%g" \
	-e "s%[@]TERMUX_APP__PACKAGE_NAME[@]%$(TERMUX_APP__PACKAGE_NAME)%g" \
	-e "s%[@]TERMUX_APP__DATA_DIR[@]%$(TERMUX_APP__DATA_DIR)%g" \
	-e "s%[@]TERMUX__ROOTFS[@]%$(TERMUX__ROOTFS)%g" \
	-e "s%[@]TERMUX__PREFIX[@]%$(TERMUX__PREFIX)%g" \
	-e "s%[@]TERMUX_ENV__S_TERMUX[@]%$(TERMUX_ENV__S_TERMUX)%g" \
	-e "s%[@]TERMUX_ENV__S_TERMUX_APP[@]%$(TERMUX_ENV__S_TERMUX_APP)%g" \
	-e "s%[@]TERMUX_ENV__S_TERMUX_EXEC[@]%$(TERMUX_ENV__S_TERMUX_EXEC)%g" \
	-e "s%[@]TERMUX_ENV__S_TERMUX_EXEC__TESTS[@]%$(TERMUX_ENV__S_TERMUX_EXEC__TESTS)%g"

define replace-termux-constants
	sed $(TERMUX__CONSTANTS__SED_ARGS) "$1.in" > "$2/$$(basename "$1")"
endef



export override CFLAGS_DEFAULT :=
export override CPPFLAGS_DEFAULT :=
export override LDFLAGS_DEFAULT :=

# If building with make directly without termux-pacakges build infrastructure,
# then allow custom path for `libtermux-core_*.a` as they may not be
# installed in `TERMUX__PREFIX__LIB_DIR`.
ifeq ($(LDFLAGS),)
	ifneq ($(TERMUX_CORE_PKG__INSTALL_PREFIX),)
		ifneq ($(shell test -d "$(TERMUX_CORE_PKG__INSTALL_PREFIX)" && echo 1 || echo 0), 1)
            $(error The termux-core package install prefix directory does not exist at TERMUX_CORE_PKG__INSTALL_PREFIX '$(TERMUX_CORE_PKG__INSTALL_PREFIX)' path)
		endif
		override CPPFLAGS_DEFAULT += -I$(TERMUX_CORE_PKG__INSTALL_PREFIX)/include/termux-core
		override LDFLAGS_DEFAULT += -L$(TERMUX_CORE_PKG__INSTALL_PREFIX)/lib
	endif
endif

override CPPFLAGS_DEFAULT += -isystem$(TERMUX__PREFIX__INCLUDE_DIR)
override LDFLAGS_DEFAULT += -L$(TERMUX__PREFIX__LIB_DIR)

ifeq ($(TERMUX_EXEC_PKG__ARCH),arm)
	# "We recommend using the -mthumb compiler flag to force the generation of 16-bit Thumb-2 instructions".
	# - https://developer.android.com/ndk/guides/standalone_toolchain.html#abi_compatibility
	override CFLAGS_DEFAULT += -march=armv7-a -mfpu=neon -mfloat-abi=softfp -mthumb
	override LDFLAGS_DEFAULT += -march=armv7-a
else ifeq ($(TERMUX_EXEC_PKG__ARCH),i686)
	# From $NDK/docs/CPU-ARCH-ABIS.html:
	override CFLAGS_DEFAULT += -march=i686 -msse3 -mstackrealign -mfpmath=sse
	# i686 seem to explicitly require '-fPIC'.
	# - https://github.com/termux/termux-packages/issues/7215#issuecomment-906154438
	override CFLAGS_DEFAULT += -fPIC
endif

# - https://github.com/termux/termux-packages/commit/b997c4ea
ifeq ($(IS_ON_DEVICE_BUILD), 0)
	override LDFLAGS_DEFAULT += -Wl,-rpath=$(TERMUX__PREFIX__LIB_DIR)
endif

# Android 7 started to support DT_RUNPATH (but not DT_RPATH).
override LDFLAGS_DEFAULT += -Wl,--enable-new-dtags

# Avoid linking extra (unneeded) libraries.
override LDFLAGS_DEFAULT += -Wl,--as-needed

# Basic hardening.
override LDFLAGS_DEFAULT += -Wl,-z,relro,-z,now


# Set default flags if building with make directly without termux-pacakges build infrastructure.
CFLAGS ?= $(CFLAGS_DEFAULT)
CXXFLAGS ?= $(CFLAGS_DEFAULT)
CPPFLAGS ?= $(CPPFLAGS_DEFAULT)
LDFLAGS ?= $(LDFLAGS_DEFAULT)

# Force optimize for speed and do basic hardening.
export override CFLAGS_FORCE := -Wall -Wextra -Werror -Wshadow -O2 -D_FORTIFY_SOURCE=2 -fstack-protector-strong

CFLAGS += $(CFLAGS_FORCE)
CXXFLAGS += $(CFLAGS_FORCE)

FSANTIZE_FLAGS += -fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer



override LIBTERMUX_EXEC__NOS__C__SOURCE_FILES := \
	lib/termux-exec_nos_c/tre/src/TermuxExecLibraryConfig.c \
	lib/termux-exec_nos_c/tre/src/termux/api/termux_exec/service/ld_preload/TermuxExecLDPreload.c \
	lib/termux-exec_nos_c/tre/src/termux/api/termux_exec/service/ld_preload/direct/exec/ExecIntercept.c \
	lib/termux-exec_nos_c/tre/src/termux/api/termux_exec/service/ld_preload/direct/exec/ExecVariantsIntercept.c \
	lib/termux-exec_nos_c/tre/src/termux/os/process/termux_exec/TermuxExecProcess.c \
	lib/termux-exec_nos_c/tre/src/termux/shell/command/environment/termux_exec/TermuxExecShellEnvironment.c

override LIBTERMUX_EXEC__NOS__C__OBJECT_FILES := $(patsubst lib/%.c,$(TMP_BUILD_OUTPUT_DIR)/lib/%.o,$(LIBTERMUX_EXEC__NOS__C__SOURCE_FILES))

override LIBTERMUX_EXEC__NOS__C__CPPFLAGS := \
	$(CPPFLAGS) -I "$(TERMUX__PREFIX)/include/termux-core" -I "lib/termux-exec_nos_c/tre/include"

override LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR := $(TESTS_BUILD_OUTPUT_DIR)/lib/termux-exec_nos_c/tre


ifneq ($(LIBTERMUX_EXEC__NOS__C__EXECVE_CALL__CHECK_ARGV0_BUFFER_OVERFLOW),1)
	override LIBTERMUX_EXEC__NOS__C__EXECVE_CALL__CHECK_ARGV0_BUFFER_OVERFLOW := 0
endif



override TERMUX_EXEC__MAIN_APP__TESTS_BUILD_OUTPUT_DIR := $(TESTS_BUILD_OUTPUT_DIR)/app/main



# The `-L` flag must come before `$LDFLAGS`, otherwise old library
# installed in system library directory from previous builds
# will get used instead of newly built one in `$LIB_BUILD_OUTPUT_DIR`.
# The `-fvisibility=hidden` flag is passed so that no internal
# functions are exported. All exported functions must explicitly enable
# `default` visibility with `__attribute__((visibility("default")))`,
# like for the `main()` function.
# The `-Wl,--exclude-libs=ALL` flag is passed so that symbols from
# the `libtermux-core_nos_c_tre.a` static library linked are not exported.
# Run `nm --demangle --defined-only --extern-only <executable>` to
# find exported symbols.
override TERMUX_EXEC_EXECUTABLE__C__BUILD_COMMAND := \
	$(CC) $(CFLAGS) $(LIBTERMUX_EXEC__NOS__C__CPPFLAGS) \
	-L$(LIB_BUILD_OUTPUT_DIR) $(LDFLAGS) -Wl,--exclude-libs=ALL \
	$(TERMUX__CONSTANTS__MACRO_FLAGS) \
	-fPIE -pie -fvisibility=hidden

# The `-l` flags must be passed after object files for proper linking.
# The order of libraries matters too and any dependencies of a library
# must come after it.
override TERMUX_EXEC_EXECUTABLE__C__POST_LDFLAGS := -l:libtermux-exec_nos_c_tre.a -l:libtermux-core_nos_c_tre.a


CLANG_FORMAT := clang-format --sort-includes --style="{ColumnLimit: 120}"
CLANG_TIDY ?= clang-tidy



# - https://www.gnu.org/software/make/manual/html_node/Parallel-Disable.html
.NOTPARALLEL:

all: | pre-build build-libtermux-exec_nos_c_tre build-libtermux-exec-direct-ld-preload build-libtermux-exec-linker-ld-preload build-termux-exec-main-app
	@printf "\ntermux-exec-package: %s\n" "Building packaging/debian/*"
	@mkdir -p $(DEBIAN_PACKAGING_BUILD_OUTPUT_DIR)
	find packaging/debian -mindepth 1 -maxdepth 1 -type f -name "*.in" -exec sh -c \
		'sed $(TERMUX__CONSTANTS__SED_ARGS) "$$1" > $(DEBIAN_PACKAGING_BUILD_OUTPUT_DIR)/"$$(basename "$$1" | sed "s/\.in$$//")"' sh "{}" \;
	find $(DEBIAN_PACKAGING_BUILD_OUTPUT_DIR) -mindepth 1 -maxdepth 1 -type f \
		-regextype posix-extended -regex "^.*/(postinst|postrm|preinst|prerm)$$" \
		-exec chmod 700 {} \;
	find $(DEBIAN_PACKAGING_BUILD_OUTPUT_DIR) -mindepth 1 -maxdepth 1 -type f \
		-regextype posix-extended -regex "^.*/(config|conffiles|templates|triggers|clilibs|fortran_mod|runit|shlibs|starlibs|symbols)$$" \
		-exec chmod 600 {} \;


	@printf "\ntermux-exec-package: %s\n\n" "Build termux-exec-package successful"

pre-build: | clean
	@printf "termux-exec-package: %s\n" "Building termux-exec-package"
	@mkdir -p $(BUILD_OUTPUT_DIR)
	@mkdir -p $(TMP_BUILD_OUTPUT_DIR)

build-termux-exec-main-app:
	@printf "\ntermux-exec-package: %s\n" "Building app/main"
	@mkdir -p $(BIN_BUILD_OUTPUT_DIR)


	@printf "\ntermux-exec-package: %s\n" "Building app/main/scripts/*"
	find app/main/scripts -type f -name "*.in" -exec sh -c \
		'sed $(TERMUX__CONSTANTS__SED_ARGS) "$$1" > $(BIN_BUILD_OUTPUT_DIR)/"$$(basename "$$1" | sed "s/\.in$$//")"' sh "{}" \;
	find $(BIN_BUILD_OUTPUT_DIR) -maxdepth 1 -exec chmod 700 "{}" \;
	find app/main/scripts -type l -exec cp -a "{}" $(BIN_BUILD_OUTPUT_DIR)/ \;


	@printf "\ntermux-exec-package: %s\n" "Building app/main/tests/*"
	@mkdir -p $(TERMUX_EXEC__MAIN_APP__TESTS_BUILD_OUTPUT_DIR)
	find app/main/tests -maxdepth 1 -type f -name "*.in" -print0 | xargs -0 -n1 sh -c \
		'output_file="$(TERMUX_EXEC__MAIN_APP__TESTS_BUILD_OUTPUT_DIR)/$$(printf "%s" "$$0" | sed -e "s|^app/main/tests/||" -e "s/\.in$$//")" && mkdir -p "$$(dirname "$$output_file")" && sed $(TERMUX__CONSTANTS__SED_ARGS) "$$0" > "$$output_file"'
	find $(TERMUX_EXEC__MAIN_APP__TESTS_BUILD_OUTPUT_DIR) -maxdepth 1 -type f -exec chmod 700 "{}" \;

build-libtermux-exec_nos_c_tre:
	@printf "\ntermux-exec-package: %s\n" "Building lib/termux-exec_nos_c_tre"
	@mkdir -p $(LIB_BUILD_OUTPUT_DIR)

	@printf "\ntermux-exec-package: %s\n" "Building lib/termux-exec_nos_c/tre/lib/*.o"
	for source_file in $(LIBTERMUX_EXEC__NOS__C__SOURCE_FILES); do \
		mkdir -p "$$(dirname "$(TMP_BUILD_OUTPUT_DIR)/$$source_file")" || exit $$?; \
		$(CC) -c $(CFLAGS) $(LIBTERMUX_EXEC__NOS__C__CPPFLAGS) \
			$(TERMUX__CONSTANTS__MACRO_FLAGS) \
			-DLIBTERMUX_EXEC__NOS__C__EXECVE_CALL__CHECK_ARGV0_BUFFER_OVERFLOW=$(LIBTERMUX_EXEC__NOS__C__EXECVE_CALL__CHECK_ARGV0_BUFFER_OVERFLOW) \
			-fPIC -fvisibility=default \
			-o $(TMP_BUILD_OUTPUT_DIR)/"$$(echo "$$source_file" | sed -E "s/(.*)\.c$$/\1.o/")" \
			"$$source_file" || exit $$?; \
	done

	@# `nm --demangle --dynamic --defined-only --extern-only /home/builder/.termux-build/termux-exec/src/build/output/usr/lib/libtermux-exec_nos_c_tre.so`
	@printf "\ntermux-exec-package: %s\n" "Building lib/libtermux-exec_nos_c_tre.so"
	$(CC) $(CFLAGS) $(LIBTERMUX_EXEC__NOS__C__CPPFLAGS) \
		-L$(LIB_BUILD_OUTPUT_DIR) $(LDFLAGS) -Wl,--exclude-libs=ALL \
		$(TERMUX__CONSTANTS__MACRO_FLAGS) \
		-fPIC -shared -fvisibility=default \
		-o $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec_nos_c_tre.so \
		$(LIBTERMUX_EXEC__NOS__C__OBJECT_FILES) \
		-l:libtermux-core_nos_c_tre.a

	@printf "\ntermux-exec-package: %s\n" "Building lib/libtermux-exec_nos_c_tre.a"
	$(AR) rcs $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec_nos_c_tre.a $(LIBTERMUX_EXEC__NOS__C__OBJECT_FILES)



	@printf "\ntermux-exec-package: %s\n" "Building lib/termux-exec_nos_c/tre/tests/*"
	@mkdir -p $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)


	@printf "\ntermux-exec-package: %s\n" "Building lib/termux-exec_nos_c/tre/tests/libtermux-exec_nos_c_tre_tests"
	$(call replace-termux-constants,lib/termux-exec_nos_c/tre/tests/libtermux-exec_nos_c_tre_tests,$(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR))
	chmod 700 $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/libtermux-exec_nos_c_tre_tests


	@printf "\ntermux-exec-package: %s\n" "Building lib/termux-exec_nos_c/tre/tests/bin/libtermux-exec_nos_c_tre_unit-binary-tests"
	@mkdir -p $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/bin

	@# `nm --demangle --defined-only --extern-only /home/builder/.termux-build/termux-exec/src/build/output/usr/libexec/installed-tests/termux-exec/lib/termux-exec_nos_c/tre/bin/libtermux-exec_nos_c_tre_unit-binary-tests-fsanitize`
	$(TERMUX_EXEC_EXECUTABLE__C__BUILD_COMMAND) -O0 -g \
		$(FSANTIZE_FLAGS) \
		-o $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/bin/libtermux-exec_nos_c_tre_unit-binary-tests-fsanitize \
		lib/termux-exec_nos_c/tre/tests/src/libtermux-exec_nos_c_tre_unit-binary-tests.c \
		$(TERMUX_EXEC_EXECUTABLE__C__POST_LDFLAGS)

	@# `nm --demangle --defined-only --extern-only /home/builder/.termux-build/termux-exec/src/build/output/usr/libexec/installed-tests/termux-exec/lib/termux-exec_nos_c/tre/bin/libtermux-exec_nos_c_tre_unit-binary-tests-nofsanitize`
	$(TERMUX_EXEC_EXECUTABLE__C__BUILD_COMMAND) -O0 -g \
		-o $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/bin/libtermux-exec_nos_c_tre_unit-binary-tests-nofsanitize \
		lib/termux-exec_nos_c/tre/tests/src/libtermux-exec_nos_c_tre_unit-binary-tests.c \
		$(TERMUX_EXEC_EXECUTABLE__C__POST_LDFLAGS)


	@printf "\ntermux-exec-package: %s\n" "Building lib/termux-exec_nos_c/tre/tests/bin/libtermux-exec_nos_c_tre_runtime-binary-tests$(TERMUX_EXEC_PKG__TESTS__API_LEVEL)"
	@mkdir -p $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/bin

	$(TERMUX_EXEC_EXECUTABLE__C__BUILD_COMMAND) -O0 -g \
		$(FSANTIZE_FLAGS) \
		-o $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/bin/libtermux-exec_nos_c_tre_runtime-binary-tests-fsanitize$(TERMUX_EXEC_PKG__TESTS__API_LEVEL) \
		lib/termux-exec_nos_c/tre/tests/src/libtermux-exec_nos_c_tre_runtime-binary-tests.c \
		$(TERMUX_EXEC_EXECUTABLE__C__POST_LDFLAGS)
	$(TERMUX_EXEC_EXECUTABLE__C__BUILD_COMMAND) -O0 -g \
		-o $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/bin/libtermux-exec_nos_c_tre_runtime-binary-tests-nofsanitize$(TERMUX_EXEC_PKG__TESTS__API_LEVEL) \
		lib/termux-exec_nos_c/tre/tests/src/libtermux-exec_nos_c_tre_runtime-binary-tests.c \
		$(TERMUX_EXEC_EXECUTABLE__C__POST_LDFLAGS)


	@printf "\ntermux-exec-package: %s\n" "Building lib/termux-exec_nos_c/tre/tests/scripts/*"
	@mkdir -p $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/scripts
	find lib/termux-exec_nos_c/tre/tests/scripts -type f -name '*.c' -print0 | xargs -0 -n1 sh -c \
		'output_file="$(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/scripts/$$(printf "%s" "$$0" | sed -e "s|^lib/termux-exec_nos_c/tre/tests/scripts/||" -e "s/\.c$$//")" && mkdir -p "$$(dirname "$$output_file")" && $(CC) $(CFLAGS) -O0 -fPIE -pie $(LDFLAGS) -g "$$0" -o "$$output_file"'
	find lib/termux-exec_nos_c/tre/tests/scripts -type f -name '*.sh' -print0 | xargs -0 -n1 sh -c \
		'output_file="$(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/scripts/$$(printf "%s" "$$0" | sed -e "s|^lib/termux-exec_nos_c/tre/tests/scripts/||")" && mkdir -p "$$(dirname "$$output_file")" && cp -a "$$0" "$$output_file"'
	find lib/termux-exec_nos_c/tre/tests/scripts -type f -name "*.in" -print0 | xargs -0 -n1 sh -c \
		'output_file="$(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/scripts/$$(printf "%s" "$$0" | sed -e "s|^lib/termux-exec_nos_c/tre/tests/scripts/||" -e "s/\.in$$//")" && mkdir -p "$$(dirname "$$output_file")" && sed $(TERMUX__CONSTANTS__SED_ARGS) "$$0" > "$$output_file"'
	find lib/termux-exec_nos_c/tre/tests/scripts -type l -print0 | xargs -0 -n1 sh -c \
		'output_file="$(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/scripts/$$(printf "%s" "$$0" | sed -e "s|^lib/termux-exec_nos_c/tre/tests/scripts/||")" && mkdir -p "$$(dirname "$$output_file")" && cp -a "$$0" "$$output_file"'
	find $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/scripts -type f -exec chmod 700 "{}" \;


build-libtermux-exec_nos_c_tre_runtime-binary-tests:
	@printf "termux-exec-package: %s\n" "Building lib/termux-exec_nos_c/tre/tests/bin/libtermux-exec_nos_c_tre_runtime-binary-tests$(TERMUX_EXEC_PKG__TESTS__API_LEVEL)"
	@mkdir -p $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/bin

	$(TERMUX_EXEC_EXECUTABLE__C__BUILD_COMMAND) -O0 -g \
		$(FSANTIZE_FLAGS) \
		-o $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/bin/libtermux-exec_nos_c_tre_runtime-binary-tests-fsanitize$(TERMUX_EXEC_PKG__TESTS__API_LEVEL) \
		lib/termux-exec_nos_c/tre/tests/src/libtermux-exec_nos_c_tre_runtime-binary-tests.c \
		$(TERMUX_EXEC_EXECUTABLE__C__POST_LDFLAGS)
	$(TERMUX_EXEC_EXECUTABLE__C__BUILD_COMMAND) -O0 -g \
		-o $(LIBTERMUX_EXEC__NOS__C__TESTS_BUILD_OUTPUT_DIR)/bin/libtermux-exec_nos_c_tre_runtime-binary-tests-nofsanitize$(TERMUX_EXEC_PKG__TESTS__API_LEVEL) \
		lib/termux-exec_nos_c/tre/tests/src/libtermux-exec_nos_c_tre_runtime-binary-tests.c \
		$(TERMUX_EXEC_EXECUTABLE__C__POST_LDFLAGS)

build-libtermux-exec-direct-ld-preload:
	@mkdir -p $(LIB_BUILD_OUTPUT_DIR)

	@# Unlike `libtermux-exec_nos_c_tre.so` and `libtermux-exec_nos_c_tre.a`, all
	@# symbols are hidden, except the exported functions with
	@# `default` visibility with `__attribute__((visibility("default")))`,
	@# defined in the `TermuxExecDirectLDPreloadEntryPoint.c` file meant to
	@# be intercepted by `$LD_PRELOAD`.
	@# `nm --demangle --dynamic --defined-only --extern-only /home/builder/.termux-build/termux-exec/src/build/output/usr/lib/libtermux-exec-direct-ld-preload.so`
	@printf "\ntermux-exec-package: %s\n" "Building lib/libtermux-exec-direct-ld-preload"
	$(CC) $(CFLAGS) $(LIBTERMUX_EXEC__NOS__C__CPPFLAGS) \
		-L$(LIB_BUILD_OUTPUT_DIR) $(LDFLAGS) -Wl,--exclude-libs=ALL \
		$(TERMUX__CONSTANTS__MACRO_FLAGS) \
		-fPIC -shared -fvisibility=hidden \
		-o $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec-direct-ld-preload.so \
		app/termux-exec-direct-ld-preload/src/termux/api/termux_exec/service/ld_preload/direct/TermuxExecDirectLDPreloadEntryPoint.c \
		-l:libtermux-exec_nos_c_tre.a -l:libtermux-core_nos_c_tre.a

	@# By default, set `libtermux-exec-direct-ld-preload.so` as the
	@# primary library variant exported in `$LD_PRELOAD` by copying it
	@# to `libtermux-exec-ld-preload.so`.
	@# Creating a symlink may have performance impacts.
	@# The `postinst` script run during package installation runs
	@# `termux-exec-ld-preload-lib setup` to set the correct variant
	@# as per the execution type required for the Termux environment
	@# of the host device by running.
	cp -a $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec-direct-ld-preload.so $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec-ld-preload.so

	@# For backward compatibility, symlink `libtermux-exec.so` to
	@# `libtermux-exec-ld-preload.so` so that older clients do not
	@# break which have exported path to `libtermux-exec.so` in
	@# `$LD_PRELOAD` via `login` script of older versions of
	@# `termux-tools `package.
	rm -f $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec.so
	ln -s libtermux-exec-ld-preload.so $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec.so

build-libtermux-exec-linker-ld-preload:
	@mkdir -p $(LIB_BUILD_OUTPUT_DIR)

	@# Unlike `libtermux-exec_nos_c_tre.so` and `libtermux-exec_nos_c_tre.a`, all
	@# symbols are hidden, except the exported functions with
	@# `default` visibility with `__attribute__((visibility("default")))`,
	@# defined in the `TermuxExecLinkerLDPreloadEntryPoint.c` file meant to
	@# be intercepted by `$LD_PRELOAD`.
	@# `nm --demangle --dynamic --defined-only --extern-only /home/builder/.termux-build/termux-exec/src/build/output/usr/lib/libtermux-exec-linker-ld-preload.so`
	@printf "\ntermux-exec-package: %s\n" "Building lib/libtermux-exec-linker-ld-preload"
	$(CC) $(CFLAGS) $(LIBTERMUX_EXEC__NOS__C__CPPFLAGS) \
		-L$(LIB_BUILD_OUTPUT_DIR) $(LDFLAGS) -Wl,--exclude-libs=ALL \
		$(TERMUX__CONSTANTS__MACRO_FLAGS) \
		-fPIC -shared -fvisibility=hidden \
		-o $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec-linker-ld-preload.so \
		app/termux-exec-linker-ld-preload/src/termux/api/termux_exec/service/ld_preload/linker/TermuxExecLinkerLDPreloadEntryPoint.c \
		-l:libtermux-exec_nos_c_tre.a -l:libtermux-core_nos_c_tre.a



clean:
	rm -rf $(BUILD_OUTPUT_DIR)

install:
	@printf "termux-exec-package: %s\n" "Installing termux-exec-package in $(TERMUX_EXEC_PKG__INSTALL_PREFIX)"
	install -d $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/bin
	install -d $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/include
	install -d $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib
	install -d $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/libexec


	find $(BIN_BUILD_OUTPUT_DIR) -maxdepth 1 \( -type f -o -type l \) -exec cp -a "{}" $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/bin/ \;

	rm -rf $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/include/termux-exec
	install -d $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/include/termux-exec/termux

	cp -a lib/termux-exec_nos_c/tre/include/termux/termux_exec__nos__c $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/include/termux-exec/termux/termux_exec__nos__c
	install $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec_nos_c_tre.so $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib/libtermux-exec_nos_c_tre.so
	install $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec_nos_c_tre.a $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib/libtermux-exec_nos_c_tre.a

	install $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec-ld-preload.so $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib/libtermux-exec-ld-preload.so
	install $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec-direct-ld-preload.so $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib/libtermux-exec-direct-ld-preload.so
	install $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec-linker-ld-preload.so $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib/libtermux-exec-linker-ld-preload.so
	@# Use `cp` for symlink as `install` will copy the target regular file instead.
	cp -a $(LIB_BUILD_OUTPUT_DIR)/libtermux-exec.so $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib/libtermux-exec.so

	find $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/include/termux-exec -type d -exec chmod 700 {} \;
	find $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/include/termux-exec -type f -exec chmod 600 {} \;


	rm -rf $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/libexec/installed-tests/termux-exec
	install -d $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/libexec/installed-tests
	cp -a $(TESTS_BUILD_OUTPUT_DIR) $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/libexec/installed-tests/termux-exec

	@printf "\ntermux-exec-package: %s\n\n" "Install termux-exec-package successful"

uninstall:
	@printf "termux-exec-package: %s\n" "Uninstalling termux-exec-package from $(TERMUX_EXEC_PKG__INSTALL_PREFIX)"

	find app/main/scripts \( -type f -o -type l \) -exec sh -c \
		'rm -f $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/bin/"$$(basename "$$1" | sed "s/\.in$$//")"' sh "{}" \;

	rm -rf $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/include/termux-exec


	rm -f $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib/libtermux-exec_nos_c_tre.so
	rm -f $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib/libtermux-exec_nos_c_tre.a

	rm -f $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib/libtermux-exec-ld-preload.so
	rm -f $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib/libtermux-exec-direct-ld-preload.so
	rm -f $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib/libtermux-exec-linker-ld-preload.so
	rm -f $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/lib/libtermux-exec.so


	rm -rf $(TERMUX_EXEC_PKG__INSTALL_PREFIX)/libexec/installed-tests/termux-exec

	@printf "\ntermux-exec-package: %s\n\n" "Uninstall termux-exec-package successful"



packaging-debian-build: all
	termux-create-package $(DEBIAN_PACKAGING_BUILD_OUTPUT_DIR)/termux-exec-package.json



test: all
	$(MAKE) TERMUX_EXEC_PKG__INSTALL_PREFIX=$(PREFIX_BUILD_INSTALL_DIR) install

	@printf "\ntermux-exec-package: %s\n" "Executing termux-exec-package tests"
	bash $(PREFIX_BUILD_INSTALL_DIR)/libexec/installed-tests/termux-exec/app/main/termux-exec-tests \
		--tests-path="$(PREFIX_BUILD_INSTALL_DIR)/libexec/installed-tests/termux-exec" \
		--ld-preload-dir="$(PREFIX_BUILD_INSTALL_DIR)/lib" \
		-vvv all

test-unit: all
	$(MAKE) TERMUX_EXEC_PKG__INSTALL_PREFIX=$(PREFIX_BUILD_INSTALL_DIR) install

	@printf "\ntermux-exec-package: %s\n" "Executing termux-exec-package unit tests"
	bash $(PREFIX_BUILD_INSTALL_DIR)/libexec/installed-tests/termux-exec/app/main/termux-exec-tests \
		--tests-path="$(PREFIX_BUILD_INSTALL_DIR)/libexec/installed-tests/termux-exec" \
		-vvv unit

test-runtime: all
	$(MAKE) TERMUX_EXEC_PKG__INSTALL_PREFIX=$(PREFIX_BUILD_INSTALL_DIR) install

	@printf "\ntermux-exec-package: %s\n" "Executing termux-exec-package runtime tests"
	bash $(PREFIX_BUILD_INSTALL_DIR)/libexec/installed-tests/termux-exec/app/main/termux-exec-tests \
		--tests-path="$(PREFIX_BUILD_INSTALL_DIR)/libexec/installed-tests/termux-exec" \
		--ld-preload-dir="$(PREFIX_BUILD_INSTALL_DIR)/lib" \
		-vvv runtime



format:
	$(CLANG_FORMAT) -i app/termux-exec-direct-ld-preload/src/termux/api/termux_exec/service/ld_preload/direct/TermuxExecDirectLDPreloadEntryPoint.c $(LIBTERMUX_EXEC__NOS__C__SOURCE_FILES)
check:
	$(CLANG_FORMAT) --dry-run app/termux-exec-direct-ld-preload/src/termux/api/termux_exec/service/ld_preload/direct/TermuxExecDirectLDPreloadEntryPoint.c $(LIBTERMUX_EXEC__NOS__C__SOURCE_FILES)
	$(CLANG_TIDY) -warnings-as-errors='*' \
		app/termux-exec-direct-ld-preload/src/termux/api/termux_exec/service/ld_preload/direct/TermuxExecDirectLDPreloadEntryPoint.c $(LIBTERMUX_EXEC__NOS__C__SOURCE_FILES) -- \
		$(LIBTERMUX_EXEC__NOS__C__CPPFLAGS) \
		$(TERMUX__CONSTANTS__MACRO_FLAGS)



.PHONY: all pre-build build-termux-exec-main-app build-libtermux-exec_nos_c_tre build-libtermux-exec_nos_c_tre_runtime-binary-tests build-libtermux-exec-linker-ld-preload build-libtermux-exec-direct-ld-preload clean install uninstall packaging-debian-build test test-unit test-runtime format check
