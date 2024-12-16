export TERMUX_EXEC_PKG__VERSION ?= 1.0
export TERMUX_EXEC_PKG__ARCH
export TERMUX_EXEC__INSTALL_PREFIX

export TERMUX__NAME ?= Termux# Default value: `Termux`
export TERMUX__LNAME ?= termux# Default value: `termux`

export TERMUX_APP__PACKAGE_NAME ?= com.termux# Default value: `com.termux`
export TERMUX_APP__DATA_DIR ?= /data/data/$(TERMUX_APP__PACKAGE_NAME)# Default value: `/data/data/com.termux`

export TERMUX__ROOTFS ?= $(TERMUX_APP__DATA_DIR)/files# Default value: `/data/data/com.termux/files`
export TERMUX__PREFIX ?= $(TERMUX__ROOTFS)/usr# Default value: `/data/data/com.termux/files/usr`

export TERMUX_ENV__S_ROOT ?= TERMUX_# Default value: `TERMUX_`
export TERMUX_ENV__SS_TERMUX ?= _# Default value: `_`
export TERMUX_ENV__S_TERMUX ?= $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX)# Default value: `TERMUX__`
export TERMUX_ENV__SS_TERMUX_APP ?= APP__# Default value: `APP__`
export TERMUX_ENV__S_TERMUX_APP ?= $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX_APP)# Default value: `TERMUX_APP__`
export TERMUX_ENV__SS_TERMUX_API_APP ?= API_APP__# Default value: `API_APP__`
export TERMUX_ENV__S_TERMUX_API_APP ?= $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX_API_APP)# Default value: `TERMUX_API_APP__`
export TERMUX_ENV__SS_TERMUX_ROOTFS ?= ROOTFS__# Default value: `ROOTFS__`
export TERMUX_ENV__S_TERMUX_ROOTFS ?= $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX_ROOTFS)# Default value: `TERMUX_ROOTFS__`
export TERMUX_ENV__SS_TERMUX_CORE ?= CORE__# Default value: `CORE__`
export TERMUX_ENV__S_TERMUX_CORE ?= $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX_CORE)# Default value: `TERMUX_CORE__`
export TERMUX_ENV__SS_TERMUX_CORE__TESTS ?= CORE__TESTS__# Default value: `CORE__TESTS__`
export TERMUX_ENV__S_TERMUX_CORE__TESTS ?= $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX_CORE__TESTS)# Default value: `TERMUX_CORE__TESTS__`
export TERMUX_ENV__SS_TERMUX_EXEC ?= EXEC__# Default value: `EXEC__`
export TERMUX_ENV__S_TERMUX_EXEC ?= $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX_EXEC)# Default value: `TERMUX_EXEC__`
export TERMUX_ENV__SS_TERMUX_EXEC__TESTS ?= EXEC__TESTS__# Default value: `EXEC__TESTS__`
export TERMUX_ENV__S_TERMUX_EXEC__TESTS ?= $(TERMUX_ENV__S_ROOT)$(TERMUX_ENV__SS_TERMUX_EXEC__TESTS)# Default value: `TERMUX_EXEC__TESTS__`

export TERMUX_APP__NAMESPACE ?= $(TERMUX_APP__PACKAGE_NAME)# Default value: `com.termux`
export TERMUX_APP__SHELL_ACTIVITY__COMPONENT_NAME ?= $(TERMUX_APP__PACKAGE_NAME)/$(TERMUX_APP__NAMESPACE).app.TermuxActivity# Default value: `com.termux/com.termux.app.TermuxActivity`
export TERMUX_APP__SHELL_SERVICE__COMPONENT_NAME ?= $(TERMUX_APP__PACKAGE_NAME)/$(TERMUX_APP__NAMESPACE).app.TermuxService# Default value: `com.termux/com.termux.app.TermuxService`

TERMUX_EXEC__TESTS__API_LEVEL ?=



ifeq ($(TERMUX__ROOTFS), /)
ifeq ($(TERMUX__ROOTFS), $(TERMUX__PREFIX))
	TERMUX_BIN_DIR := /bin# Default value: `/bin`
endif
endif
TERMUX_BIN_DIR ?= $(TERMUX__PREFIX)/bin# Default value: `/data/data/com.termux/files/usr/bin`


# If architecture not set, find it for the compiler based on which
# predefined architecture macro is defined. The `shell` function
# replaces newlines with a space and a literal space cannot be entered
# in a makefile as its used as a splitter, hence $(SPACE) variable is
# created and used for matching.
ifeq ($(TERMUX_EXEC_PKG__ARCH),)
	export PREDEFINED_MACROS := $(shell $(CC) -x c /dev/null -dM -E)
	EMPTY :=
	SPACE := $(EMPTY) $(EMPTY)
	ifneq (,$(findstring $(SPACE)#define __i686__ 1$(SPACE),$(SPACE)$(PREDEFINED_MACROS)$(SPACE)))
		TERMUX_EXEC_PKG__ARCH := i686
	else ifneq (,$(findstring $(SPACE)#define __x86_64__ 1$(SPACE),$(SPACE)$(PREDEFINED_MACROS)$(SPACE)))
		TERMUX_EXEC_PKG__ARCH := x86_64
	else ifneq (,$(findstring $(SPACE)#define __aarch64__ 1$(SPACE),$(SPACE)$(PREDEFINED_MACROS)$(SPACE)))
		TERMUX_EXEC_PKG__ARCH := aarch64
	else ifneq (,$(findstring $(SPACE)#define __arm__ 1$(SPACE),$(SPACE)$(PREDEFINED_MACROS)$(SPACE)))
		TERMUX_EXEC_PKG__ARCH := arm
	else
        $(error Unsupported package arch)
	endif
endif


ifeq ($(DESTDIR)$(PREFIX),)
	TERMUX_EXEC__INSTALL_PREFIX := $(TERMUX__PREFIX)
else
	TERMUX_EXEC__INSTALL_PREFIX := $(DESTDIR)$(PREFIX)
endif



BUILD_DIR := build
PREFIX_BUILD_DIR := $(BUILD_DIR)/usr
LIB_BUILD_DIR := $(PREFIX_BUILD_DIR)/lib
TESTS_BUILD_DIR := $(PREFIX_BUILD_DIR)/libexec/installed-tests/termux-exec


CFLAGS += -Wall -Wextra -Werror -Wshadow -O2 -D_FORTIFY_SOURCE=2 -fstack-protector-strong
FSANTIZE_FLAGS += -fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer
CLANG_FORMAT := clang-format --sort-includes --style="{ColumnLimit: 120}" $(C_SOURCE)
CLANG_TIDY ?= clang-tidy
C_SOURCE := \
	src/data/assert_utils.c \
	src/data/data_utils.c \
	src/exec/exec.c \
	src/exec/exec_variants.c \
	src/file/file_utils.c \
	src/logger/logger.c \
	src/os/safe_strerror.c \
	src/os/selinux_utils.c \
	src/termux/termux_env.c \
	src/termux/termux_files.c

TERMUX_CONSTANTS_MACRO_FLAGS := \
	-DTERMUX_EXEC_PKG__VERSION=\"$(TERMUX_EXEC_PKG__VERSION)\" \
	-DTERMUX__NAME=\"$(TERMUX__NAME)\" \
	-DTERMUX__LNAME=\"$(TERMUX__LNAME)\" \
	-DTERMUX_APP__DATA_DIR=\"$(TERMUX_APP__DATA_DIR)\" \
	-DTERMUX__ROOTFS=\"$(TERMUX__ROOTFS)\" \
	-DTERMUX__PREFIX=\"$(TERMUX__PREFIX)\" \
	-DTERMUX_BIN_DIR=\"$(TERMUX_BIN_DIR)\" \
	-DTERMUX_ENV__S_TERMUX=\"$(TERMUX_ENV__S_TERMUX)\" \
	-DTERMUX_ENV__S_TERMUX_APP=\"$(TERMUX_ENV__S_TERMUX_APP)\" \
	-DTERMUX_ENV__S_TERMUX_ROOTFS=\"$(TERMUX_ENV__S_TERMUX_ROOTFS)\" \
	-DTERMUX_ENV__S_TERMUX_EXEC=\"$(TERMUX_ENV__S_TERMUX_EXEC)\" \
	-DTERMUX_ENV__S_TERMUX_EXEC__TESTS=\"$(TERMUX_ENV__S_TERMUX_EXEC__TESTS)\"

TERMUX_CONSTANTS_SED_ARGS := \
	-e "s%[@]TERMUX_EXEC_PKG__VERSION[@]%$(TERMUX_EXEC_PKG__VERSION)%g" \
	-e "s%[@]TERMUX_EXEC_PKG__ARCH[@]%$(TERMUX_EXEC_PKG__ARCH)%g" \
	-e "s%[@]TERMUX__LNAME[@]%$(TERMUX__LNAME)%g" \
	-e "s%[@]TERMUX_APP__PACKAGE_NAME[@]%$(TERMUX_APP__PACKAGE_NAME)%g" \
	-e "s%[@]TERMUX__PREFIX[@]%$(TERMUX__PREFIX)%g" \
	-e "s%[@]TERMUX_ENV__S_TERMUX[@]%$(TERMUX_ENV__S_TERMUX)%g" \
	-e "s%[@]TERMUX_ENV__S_TERMUX_APP[@]%$(TERMUX_ENV__S_TERMUX_APP)%g" \
	-e "s%[@]TERMUX_ENV__S_TERMUX_API_APP[@]%$(TERMUX_ENV__S_TERMUX_API_APP)%g" \
	-e "s%[@]TERMUX_ENV__S_TERMUX_ROOTFS[@]%$(TERMUX_ENV__S_TERMUX_ROOTFS)%g" \
	-e "s%[@]TERMUX_ENV__S_TERMUX_CORE[@]%$(TERMUX_ENV__S_TERMUX_CORE)%g" \
	-e "s%[@]TERMUX_ENV__S_TERMUX_CORE__TESTS[@]%$(TERMUX_ENV__S_TERMUX_CORE__TESTS)%g" \
	-e "s%[@]TERMUX_ENV__S_TERMUX_EXEC__TESTS[@]%$(TERMUX_ENV__S_TERMUX_EXEC__TESTS)%g" \
	-e "s%[@]TERMUX_APP__NAMESPACE[@]%$(TERMUX_APP__NAMESPACE)%g" \
	-e "s%[@]TERMUX_APP__SHELL_ACTIVITY__COMPONENT_NAME[@]%$(TERMUX_APP__SHELL_ACTIVITY__COMPONENT_NAME)%g" \
	-e "s%[@]TERMUX_APP__SHELL_SERVICE__COMPONENT_NAME[@]%$(TERMUX_APP__SHELL_SERVICE__COMPONENT_NAME)%g"

define replace-termux-constants
	sed $(TERMUX_CONSTANTS_SED_ARGS) "$1.in" > "$2/$$(basename "$1" | sed "s/\.in$$//")"
endef



all: build-runtime-binary-tests
	@echo "Building lib/libtermux-exec.so"
	@mkdir -p $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) src/termux-exec.c $(C_SOURCE) -shared -fPIC \
		-fvisibility=hidden \
		$(TERMUX_CONSTANTS_MACRO_FLAGS) \
		-o $(LIB_BUILD_DIR)/libtermux-exec.so


	@mkdir -p $(TESTS_BUILD_DIR)

	@echo "Building tests/termux-exec-tests"
	$(call replace-termux-constants,tests/termux-exec-tests,$(TESTS_BUILD_DIR))
	chmod u+x $(TESTS_BUILD_DIR)/termux-exec-tests

	@echo "Building tests/runtime-script-tests"
	$(call replace-termux-constants,tests/runtime-script-tests,$(TESTS_BUILD_DIR))
	chmod u+x $(TESTS_BUILD_DIR)/runtime-script-tests


	@echo "Building tests/unit-tests"
	@mkdir -p $(TESTS_BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -g tests/unit-tests.c $(C_SOURCE) \
		$(FSANTIZE_FLAGS) \
		$(TERMUX_CONSTANTS_MACRO_FLAGS) \
		-DTERMUX_EXEC__RUNNING_TESTS=1 \
		-o $(TESTS_BUILD_DIR)/unit-tests-fsanitize
	$(CC) $(CFLAGS) $(LDFLAGS) -g tests/unit-tests.c $(C_SOURCE) \
		$(TERMUX_CONSTANTS_MACRO_FLAGS) \
		-DTERMUX_EXEC__RUNNING_TESTS=1 \
		-o $(TESTS_BUILD_DIR)/unit-tests-nofsanitize


	@mkdir -p $(TESTS_BUILD_DIR)/files


	@echo "Building tests/files/exec/*"
	@mkdir -p $(TESTS_BUILD_DIR)/files/exec
	find tests/files/exec -maxdepth 1 -name '*.c' -exec sh -c '$(CC) $(CFLAGS) $(LDFLAGS) "$$1" -o $(TESTS_BUILD_DIR)/files/exec/"$$(basename "$$1" | sed "s/\.c$$//")"' sh "{}" \;
	find tests/files/exec -maxdepth 1 \( -name '*.sh' -o -name '*.sym' \) -exec cp -a "{}" $(TESTS_BUILD_DIR)/files/exec/ \;
	find tests/files/exec -maxdepth 1 -type f -name "*.in" -exec sh -c \
		'sed $(TERMUX_CONSTANTS_SED_ARGS) "$$1" > $(TESTS_BUILD_DIR)/files/exec/"$$(basename "$$1" | sed "s/\.in$$//")"' sh "{}" \;
	find $(TESTS_BUILD_DIR)/files/exec -maxdepth 1 -name '*.sh' -exec chmod u+x "{}" \;


	@echo "Building termux-exec-package.json"
	$(call replace-termux-constants,termux-exec-package.json,$(BUILD_DIR))

build-runtime-binary-tests:
	@echo "Building tests/runtime-binary-tests$(TERMUX_EXEC__TESTS__API_LEVEL)"
	@mkdir -p $(TESTS_BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -g tests/runtime-binary-tests.c $(C_SOURCE) \
		$(FSANTIZE_FLAGS) \
		$(TERMUX_CONSTANTS_MACRO_FLAGS) \
		-o $(TESTS_BUILD_DIR)/runtime-binary-tests-fsanitize$(TERMUX_EXEC__TESTS__API_LEVEL)
	$(CC) $(CFLAGS) $(LDFLAGS) -g tests/runtime-binary-tests.c $(C_SOURCE) \
		$(TERMUX_CONSTANTS_MACRO_FLAGS) \
		-o $(TESTS_BUILD_DIR)/runtime-binary-tests-nofsanitize$(TERMUX_EXEC__TESTS__API_LEVEL)



clean:
	rm -rf $(BUILD_DIR)

install:
	@echo "Installing in $(TERMUX_EXEC__INSTALL_PREFIX)"

	install -d $(TERMUX_EXEC__INSTALL_PREFIX)/lib
	install $(LIB_BUILD_DIR)/libtermux-exec.so $(TERMUX_EXEC__INSTALL_PREFIX)/lib/libtermux-exec.so


	install -d $(TERMUX_EXEC__INSTALL_PREFIX)/libexec/installed-tests/termux-exec
	find $(TESTS_BUILD_DIR) -maxdepth 1 -type f -exec install "{}" -t $(TERMUX_EXEC__INSTALL_PREFIX)/libexec/installed-tests/termux-exec/ \;


	install -d $(TERMUX_EXEC__INSTALL_PREFIX)/libexec/installed-tests/termux-exec/files
	install -d $(TERMUX_EXEC__INSTALL_PREFIX)/libexec/installed-tests/termux-exec/files/exec
	find $(TESTS_BUILD_DIR)/files/exec -maxdepth 1 \( -type f -o -type l \) -exec cp -a "{}" $(TERMUX_EXEC__INSTALL_PREFIX)/libexec/installed-tests/termux-exec/files/exec/ \;

uninstall:
	@echo "Uninstalling from $(TERMUX_EXEC__INSTALL_PREFIX)"

	rm -f $(TERMUX_EXEC__INSTALL_PREFIX)/lib/libtermux-exec.so
	rm -rf $(TERMUX_EXEC__INSTALL_PREFIX)/libexec/installed-tests/termux-exec



deb: all
	termux-create-package $(BUILD_DIR)/termux-exec-package.json



test: all
	$(MAKE) TERMUX_EXEC__INSTALL_PREFIX=install install

	@echo "Running tests"
	install/libexec/installed-tests/termux-exec/termux-exec-tests --ld-preload="install/lib/libtermux-exec.so" --tests-path="install/libexec/installed-tests/termux-exec" -vvv all

test-unit: all
	$(MAKE) TERMUX_EXEC__INSTALL_PREFIX=install install

	@echo "Running unit tests"
	bash install/libexec/installed-tests/termux-exec/termux-exec-tests --tests-path="install/libexec/installed-tests/termux-exec" -vvv unit

test-runtime: all
	$(MAKE) TERMUX_EXEC__INSTALL_PREFIX=install install

	@echo "Running runtime tests"
	install/libexec/installed-tests/termux-exec/termux-exec-tests --ld-preload="install/lib/libtermux-exec.so" --tests-path="install/libexec/installed-tests/termux-exec" -vvv runtime



format:
	$(CLANG_FORMAT) -i src/termux-exec.c $(C_SOURCE)
check:
	$(CLANG_FORMAT) --dry-run src/termux-exec.c $(C_SOURCE)
	$(CLANG_TIDY) -warnings-as-errors='*' src/termux-exec.c $(C_SOURCE) -- \
		$(TERMUX_CONSTANTS_MACRO_FLAGS)



.PHONY: all build-runtime-binary-tests clean install uninstall deb test test-unit test-runtime format check
