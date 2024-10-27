---
page_ref: "@ARK_PROJECT__VARIANT@/termux/termux-exec-package/docs/@ARK_DOC__VERSION@/developer/build/index.md"
---

# termux-exec-package Build Docs

<!-- @ARK_DOCS__HEADER_PLACEHOLDER@ -->

The [`termux-exec`](https://github.com/termux/termux-exec) build instructions are available below. For install instructions, check [`install`](../../install/index.md) docs.

### Contents

- [Build Methods](#build-methods)

---

&nbsp;





## Build Methods

The `termux-exec` package provided by Termux is built from the [`termux/termux-exec`](https://github.com/termux/termux-exec) repository. It can be built with the following methods.

- [Termux Packages Build Infrastructure](#termux-packages-build-infrastructure)
- [On Device With `make`](#on-device-with-make)

**The [Termux Packages Build Infrastructure](#termux-packages-build-infrastructure) is the recommended way to build `termux-exec`.** If the `termux-exec` package is built with the [Termux Packages Build Infrastructure](#termux-packages-build-infrastructure), then the Termux variable values in the `Makefile` are dynamically set to the values defined in the [`properties.sh`] file of the build infrastructure by passing them to `make` via the `$TERMUX_PKG_EXTRA_MAKE_ARGS` variable set in the [`packages/termux-exec/build.sh`] file. If `termux-exec` is built with `make` instead, then the hardcoded fallback/default Termux variable values in the `Makefile` will get used during build time, which may affect or break `termux-exec` at runtime if current app/environment is different from the Termux default one (`TERMUX_APP__PACKAGE_NAME=com.termux` and `TERMUX__ROOTFS=/data/data/com.termux/files`). However, if `make` must be used for some reason, and building for a different app/environment than the Termux default, like for a Termux fork or alternate package name/rootfs, then manually update the hardcoded values in the `Makefile` or manually pass the alternate values to the `make` command.

## &nbsp;

&nbsp;



### Termux Packages Infrastructure

To build the `termux-exec` package with the [`termux-packages`](https://github.com/termux/termux-packages) build infrastructure, the provided [`build-package.sh`](https://github.com/termux/termux-packages/blob/master/build-package.sh) script can be used. Check the [Build environment](https://github.com/termux/termux-packages/wiki/Build-environment) and [Building packages](https://github.com/termux/termux-packages/wiki/Building-packages) docs for how to build packages.

#### Default Sources

To build the `termux-exec` package from its default repository release tag or git branch sources that are used for building the package provided in Termux repositories, just clone the `termux-packages` repository and build.

```shell
# Clone `termux-packages` repo and switch current working directory to it.
git clone https://github.com/termux/termux-packages.git
cd termux-packages

# (OPTIONAL) Run termux-packages docker container if running off-device.
./scripts/run-docker.sh

# Force build package and download dependencies from Termux packages repositories.
./build-package.sh -f -I termux-exec
```

#### Local Sources

To build the `termux-exec` package from its local sources or a pull request branch, clone the `termux-packages` repository, clone/create the `termux-exec` repository locally, make required changes to the [`packages/termux-exec/build.sh`] file to update the source url and then build.

Check [Build Local Package](https://github.com/termux/termux-packages/wiki/Building-packages#build-local-package) and [Package Build Local Source URLs](https://github.com/termux/termux-packages/wiki/Creating-new-package#package-build-local-source-urls) docs for more info on how to building packages from local sources.*

```shell
# Clone `termux-packages` repo and switch current working directory to it.
git clone https://github.com/termux/termux-packages.git
cd termux-packages

# Update `$TERMUX_PKG_SRCURL` variable in `packages/termux-exec/build.sh`.
# We use `file:///path/to/source/dir` format for the local source URL.
TERMUX_PKG_SRCURL=file:///home/builder/termux-packages/sources/termux-exec
TERMUX_PKG_SHA256=SKIP_CHECKSUM

# Clone/copy `termux-exec` repo at `termux-packages/sources/termux-exec`
# directory. Make sure current working directory is root directory of
# termux-packages repo when cloning.
git clone https://github.com/termux/termux-exec.git sources/termux-exec

# (OPTIONAL) Manually switch to different (pull) branch that exists on
# origin if required, or to the one defined in $TERMUX_PKG_GIT_BRANCH
# variable of build.sh file, as it will not be automatically checked out.
# By default, the repo default/current branch that's cloned
# will get built, which is usually `master` or `main`.
# Whatever is the current state of the source directory will
# be built as is, including any uncommitted changes to current
# branch.
(cd sources/termux-exec; git checkout <branch_name>)

# (OPTIONAL) Run termux-packages docker container if running off-device.
./scripts/run-docker.sh

# Force build package and download dependencies from Termux packages repositories.
./build-package.sh -f -I termux-exec
```

## &nbsp;

&nbsp;



### On Device With `make`

To build `termux-exec` package on the device inside the Termux app with [`make`](https://www.gnu.org/software/make), check below. Do not use a PC to build the package as PC architecture may be different from target device architecture and the `clang` compiler wouldn't have been patched like Termux provided one is so that built packages are compatible with Termux, like patches done for `DT_RUNPATH`.

```shell
# Install dependencies.
pkg install clang git make termux-create-package

# Clone/copy `termux-exec` repo at `termux-exec` directory and switch
# current working directory to it.
git clone https://github.com/termux/termux-exec.git termux-exec
cd termux-exec

# Whatever is the current state of the `termux-exec` directory will be built.
# If required, manually switch to different (pull) branch that exists on origin.
git checkout <branch_name>

# Remove any existing deb files in current directory.
rm -f termux-exec_*.deb

# Build deb file for the architecture of the host device/clang compiler.
make deb

# Install.
# We use shell * glob expansion to automatically select the deb file
# regardless of `_<version>_<arch>.deb` suffix, that's why existing
# deb files were deleted earlier in case any existed with the wrong version.
dpkg -i termux-exec_*.deb
```

---

&nbsp;





[`packages/termux-exec/build.sh`]: https://github.com/termux/termux-packages/blob/master/packages/termux-exec/build.sh
[`properties.sh`]: https://github.com/termux/termux-packages/blob/master/scripts/properties.sh
