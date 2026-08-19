#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════════
# LeviLinux Build Script
# Unified build system for Minecraft Bedrock on Linux
# ═══════════════════════════════════════════════════════════════════════════════

set -euo pipefail

# ─────────────────────────────────────────────────────────────────────────────
# CONFIGURATION
# ─────────────────────────────────────────────────────────────────────────────

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_NAME="LeviLinux"
BUILD_DIR="${SCRIPT_DIR}/build"
INSTALL_PREFIX="/usr/local"
CMAKE_GENERATOR="Ninja"
JOBS="$(nproc 2>/dev/null || echo 4)"

# Build options
BUILD_TYPE="RelWithDebInfo"
BUILD_UI="ON"
BUILD_CLI="OFF"
BUILD_TESTS="OFF"
BUILD_DOCS="OFF"
BUILD_PACKAGE="OFF"
ENABLE_ASAN="OFF"
ENABLE_TSAN="OFF"
ENABLE_UBSAN="OFF"

# Architecture
HOST_ARCH="$(uname -m)"
TARGET_ARCH="${HOST_ARCH}"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m' # No Color

# ─────────────────────────────────────────────────────────────────────────────
# UTILITIES
# ─────────────────────────────────────────────────────────────────────────────

log_info()    { echo -e "${GREEN}[INFO]${NC} $*"; }
log_warn()    { echo -e "${YELLOW}[WARN]${NC} $*"; }
log_error()   { echo -e "${RED}[ERROR]${NC} $*"; }
log_step()    { echo -e "\n${CYAN}═══════════════════════════════════════════════════════════════${NC}"; \
                echo -e "${CYAN}  $*${NC}"; \
                echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"; }
log_header()  { echo -e "\n${MAGENTA}┌─────────────────────────────────────────────────────────────┐${NC}"; \
                echo -e "${MAGENTA}│${NC} ${WHITE}$*${NC}"; \
                echo -e "${MAGENTA}└─────────────────────────────────────────────────────────────┘${NC}"; }
log_success() { echo -e "${GREEN}✓${NC} $*"; }
log_fail()    { echo -e "${RED}✗${NC} $*"; }

check_command() {
    command -v "$1" &>/dev/null
}

confirm() {
    local prompt="${1:-Continue?}"
    local default="${2:-n}"
    local yn

    if [[ "${default}" == "y" ]]; then
        read -rp "$(echo -e "${YELLOW}${prompt} [Y/n]:${NC} ")" yn
        [[ -z "$yn" ]] && yn="y"
    else
        read -rp "$(echo -e "${YELLOW}${prompt} [y/N]:${NC} ")" yn
        [[ -z "$yn" ]] && yn="n"
    fi

    [[ "$yn" == "y" || "$yn" == "Y" ]]
}

# ─────────────────────────────────────────────────────────────────────────────
# DEPENDENCY CHECKING
# ─────────────────────────────────────────────────────────────────────────────

check_dependencies() {
    log_step "Checking Dependencies"

    local missing=()
    local optional_missing=()

    # Required tools
    for cmd in cmake g++ gcc pkg-config git; do
        if check_command "$cmd"; then
            log_success "$cmd $(command -v "$cmd")"
        else
            log_fail "$cmd NOT FOUND"
            missing+=("$cmd")
        fi
    done

    # Ninja
    if check_command ninja; then
        log_success "ninja $(command -v ninja)"
    elif check_command ninja-build; then
        log_warn "ninja-build found but not as 'ninja'"
        CMAKE_GENERATOR="Unix Makefiles"
    else
        log_warn "ninja not found, falling back to make"
        CMAKE_GENERATOR="Unix Makefiles"
    fi

    # Qt6
    if check_command qmake6; then
        log_success "Qt6 $(qmake6 --version 2>/dev/null | head -1 || echo 'found')"
    elif check_command qmake; then
        log_success "Qt $(qmake --version 2>/dev/null | head -1 || echo 'found')"
    else
        log_warn "Qt6 not found in PATH"
        optional_missing+=("qt6-base-dev")
    fi

    # Required libraries (pkg-config-name:debian-pkg:fedora-pkg:arch-pkg)
    local required_libs=(
        "gl:libgl-dev:mesa-libGL-devel:mesa"
        "egl:libegl-dev:mesa-libEGL-devel:libglvnd"
        "libcurl:libcurl4-openssl-dev:libcurl-devel:curl"
        "zlib:zlib1g-dev:zlib-devel:zlib"
        "mbedtls:libmbedtls-dev:mbedtls-devel:mbedtls"
    )

    # Check for libpng (required by mcpelauncher/eglut, found via cmake)
    if ! pkg-config --exists libpng 2>/dev/null; then
        # On Fedora, libpng-devel doesn't always provide a .pc file
        # Check for the library + header directly
        local has_libpng=0
        if [[ -f /usr/lib64/libpng16.so ]] || [[ -f /usr/lib/x86_64-linux-gnu/libpng16.so ]]; then
            if [[ -f /usr/include/png.h ]] || [[ -f /usr/include/libpng16/png.h ]]; then
                has_libpng=1
                log_success "libpng (found via cmake FindPNG, no pkg-config)"
            fi
        fi
        if [[ $has_libpng -eq 0 ]]; then
            log_warn "libpng not found (mcpelauncher will be skipped)"
            optional_missing+=("libpng-dev:libpng-devel:libpng-apng")
        fi
    else
        log_success "libpng $(pkg-config --modversion libpng 2>/dev/null || echo 'found')"
    fi
    for entry in "${required_libs[@]}"; do
        IFS=':' read -r pkg_name debian_name fedora_name arch_name <<< "$entry"
        if pkg-config --exists "$pkg_name" 2>/dev/null; then
            log_success "$pkg_name $(pkg-config --modversion "$pkg_name" 2>/dev/null || echo 'found')"
        else
            log_fail "$pkg_name NOT FOUND"
            missing+=("${debian_name}:${fedora_name}:${arch_name}")
        fi
    done

    # Optional libraries
    local optional_libs=(
        "glfw3:libglfw3-dev:glfw-devel:glfw"
        "vulkan:libvulkan-dev:vulkan-devel:vulkan-headers"
        "sdl3:libsdl2-dev:SDL2-devel:sdl2"
        "libpipewire-0.3:libpipewire-0.3-dev:pipewire-devel:pipewire"
        "libpulse:libpulse-dev:pulseaudio-libs-devel:libpulse"
        "capstone:libcapstone-dev:capstone-devel:capstone"
        "fmt:libfmt-dev:fmt-devel:fmt"
    )
    for entry in "${optional_libs[@]}"; do
        IFS=':' read -r pkg_name debian_name fedora_name arch_name <<< "$entry"
        if pkg-config --exists "$pkg_name" 2>/dev/null; then
            log_success "$pkg_name (optional) $(pkg-config --modversion "$pkg_name" 2>/dev/null || echo 'found')"
        else
            log_warn "$pkg_name not found (optional)"
            optional_missing+=("${debian_name}:${fedora_name}:${arch_name}")
        fi
    done

    # FMOD (proprietary)
    if check_command fmod-config 2>/dev/null || pkg-config --exists fmod 2>/dev/null; then
        log_success "FMOD found"
    elif [[ -d "/opt/fmod" ]] || [[ -d "/usr/local/lib/fmod" ]]; then
        log_success "FMOD found in custom path"
    else
        log_warn "FMOD not found (audio will use PipeWire/PulseAudio fallback)"
        optional_missing+=("fmod")
    fi

    # Detect distro for package suggestions
    local distro_id="unknown"
    if [[ -f /etc/os-release ]]; then
        . /etc/os-release
        distro_id="${ID:-unknown}"
    fi

    # Report
    echo ""
    if [[ ${#missing[@]} -gt 0 ]]; then
        log_error "Missing required dependencies:"

        local debian_pkgs=()
        local fedora_pkgs=()
        local arch_pkgs=()

        for dep in "${missing[@]}"; do
            IFS=':' read -r debian_name fedora_name arch_name <<< "$dep"
            echo "  - ${debian_name}"
            debian_pkgs+=("$debian_name")
            fedora_pkgs+=("$fedora_name")
            arch_pkgs+=("$arch_name")
        done
        echo ""
        echo -e "${YELLOW}Install with:${NC}"
        echo "  Debian/Ubuntu:  sudo apt install ${debian_pkgs[*]}"
        echo "  Fedora:         sudo dnf install ${fedora_pkgs[*]}"
        echo "  Arch:           sudo pacman -S --needed ${arch_pkgs[*]}"
        echo ""
        echo -e "${YELLOW}Or run:${NC} ./scripts/fetch_deps.sh"
        echo ""
        return 1
    fi

    if [[ ${#optional_missing[@]} -gt 0 ]]; then
        log_warn "Optional dependencies missing:"

        local debian_pkgs=()
        local fedora_pkgs=()
        local arch_pkgs=()

        for dep in "${optional_missing[@]}"; do
            IFS=':' read -r debian_name fedora_name arch_name <<< "$dep"
            debian_pkgs+=("$debian_name")
            fedora_pkgs+=("$fedora_name")
            arch_pkgs+=("$arch_name")
        done

        echo -e "${YELLOW}Install for full features:${NC}"
        echo "  Debian/Ubuntu:  sudo apt install ${debian_pkgs[*]}"
        echo "  Fedora:         sudo dnf install ${fedora_pkgs[*]}"
        echo "  Arch:           sudo pacman -S --needed ${arch_pkgs[*]}"
        echo ""
    fi

    log_success "All required dependencies found"
    return 0
}

# ─────────────────────────────────────────────────────────────────────────────
# GIT SUBMODULES
# ─────────────────────────────────────────────────────────────────────────────

init_submodules() {
    log_step "Initializing Git Submodules"

    if [[ ! -d "${SCRIPT_DIR}/.git" ]]; then
        log_warn "Not a git repository, skipping submodule init"
        return 0
    fi

    cd "$SCRIPT_DIR"

    # mcpelauncher-manifest
    if [[ -d "mcpelauncher-manifest" ]]; then
        if [[ -f "mcpelauncher-manifest/.gitmodules" ]]; then
            log_info "Updating mcpelauncher-manifest submodules..."
            (cd mcpelauncher-manifest && git submodule update --init --recursive --jobs 4) || \
                log_warn "Failed to update some mcpelauncher-manifest submodules"
        else
            log_warn "mcpelauncher-manifest/.gitmodules not found"
        fi
    fi

    # pl-mod, pl-config, pl-modmenu
    for submodule in "third_party/pl-mod" "third_party/pl-config" "third_party/pl-modmenu"; do
        if [[ -d "$submodule" ]]; then
            if [[ -f "$submodule/.gitmodules" ]] || [[ -d "$submodule/.git" ]]; then
                log_info "Updating $submodule..."
                (cd "$submodule" && git submodule update --init --recursive) || \
                    log_warn "Failed to update $submodule"
            fi
        fi
    done

    log_success "Submodules initialized"
}

# ─────────────────────────────────────────────────────────────────────────────
# FETCH DEPENDENCIES
# ─────────────────────────────────────────────────────────────────────────────

fetch_deps() {
    log_step "Fetching External Dependencies"

    local third_party="${SCRIPT_DIR}/third_party"
    mkdir -p "$third_party"

    # mcpelauncher-manifest
    if [[ ! -d "${third_party}/mcpelauncher-manifest" ]]; then
        log_info "Cloning mcpelauncher-manifest..."
        git clone --depth 1 https://github.com/minecraft-linux/mcpelauncher-manifest.git \
            "${third_party}/mcpelauncher-manifest"
    else
        log_success "mcpelauncher-manifest already present"
    fi

    # pl-mod (if exists in LeviLaunchroid)
    if [[ -d "${SCRIPT_DIR}/LeviLaunchroid/minecraft" ]] && [[ ! -d "${third_party}/pl-mod" ]]; then
        log_info "Copying pl-mod from LeviLaunchroid..."
        cp -r "${SCRIPT_DIR}/LeviLaunchroid/minecraft" "${third_party}/pl-mod" 2>/dev/null || \
            log_warn "pl-mod not found in LeviLaunchroid"
    fi

    # pl-config
    if [[ ! -d "${third_party}/pl-config" ]]; then
        log_info "Copying pl-config from LeviLaunchroid..."
        if [[ -d "${SCRIPT_DIR}/LeviLaunchroid/minecraft" ]]; then
            find "${SCRIPT_DIR}/LeviLaunchroid/minecraft" -name "Config.hpp" -path "*/pl/*" \
                -exec cp -r "$(dirname "$(dirname "{}")")" "${third_party}/pl-config" \; 2>/dev/null || \
                log_warn "pl-config not found"
        fi
    fi

    log_success "Dependencies fetched"
}

# ─────────────────────────────────────────────────────────────────────────────
# CMAKE CONFIGURATION
# ─────────────────────────────────────────────────────────────────────────────

configure_cmake() {
    log_step "Configuring CMake"

    local build_dir="${BUILD_DIR}"

    # Architecture-specific build dir
    if [[ "$HOST_ARCH" != "$TARGET_ARCH" ]]; then
        build_dir="${BUILD_DIR}/${TARGET_ARCH}"
    fi

    mkdir -p "$build_dir"

    local cmake_args=(
        -G "${CMAKE_GENERATOR}"
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
        -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}"
        -DBUILD_UI="${BUILD_UI}"
        -DBUILD_CLI="${BUILD_CLI}"
        -DBUILD_TESTS="${BUILD_TESTS}"
        -DBUILD_DOCS="${BUILD_DOCS}"
        -DENABLE_ASAN="${ENABLE_ASAN}"
        -DENABLE_TSAN="${ENABLE_TSAN}"
        -DENABLE_UBSAN="${ENABLE_UBSAN}"
    )

    # Architecture-specific flags
    if [[ "$TARGET_ARCH" == "aarch64" ]]; then
        cmake_args+=(
            -DCMAKE_SYSTEM_NAME=Linux
            -DCMAKE_SYSTEM_PROCESSOR=aarch64
            -DCMAKE_C_FLAGS="-march=armv8-a"
            -DCMAKE_CXX_FLAGS="-march=armv8-a"
        )
        log_info "Cross-compiling for aarch64"
    elif [[ "$TARGET_ARCH" == "x86_64" ]]; then
        cmake_args+=(
            -DCMAKE_SYSTEM_NAME=Linux
            -DCMAKE_SYSTEM_PROCESSOR=x86_64
        )
    fi

    # Compiler selection
    if [[ -n "${CC:-}" ]]; then
        cmake_args+=(-DCMAKE_C_COMPILER="${CC}")
    fi
    if [[ -n "${CXX:-}" ]]; then
        cmake_args+=(-DCMAKE_CXX_COMPILER="${CXX}")
    fi

    # Use ccache if available
    if check_command ccache; then
        cmake_args+=(
            -DCMAKE_C_COMPILER_LAUNCHER=ccache
            -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
        )
        log_info "Using ccache for faster builds"
    fi

    # Compatibility: allow older cmake_minimum_required in subprojects (eglut, etc.)
    cmake_args+=(-DCMAKE_POLICY_VERSION_MINIMUM=3.5)

    log_info "Build directory: $build_dir"
    log_info "Generator: $CMAKE_GENERATOR"
    log_info "Build type: $BUILD_TYPE"
    log_info "Jobs: $JOBS"

    cd "$build_dir"

    echo ""
    log_info "CMake command:"
    echo "  cmake ${SCRIPT_DIR} ${cmake_args[*]}"
    echo ""

    cmake "${SCRIPT_DIR}" "${cmake_args[@]}"

    log_success "CMake configured successfully"
}

# ─────────────────────────────────────────────────────────────────────────────
# BUILD
# ─────────────────────────────────────────────────────────────────────────────

build_project() {
    log_step "Building ${PROJECT_NAME}"

    local build_dir="${BUILD_DIR}"
    if [[ "$HOST_ARCH" != "$TARGET_ARCH" ]]; then
        build_dir="${BUILD_DIR}/${TARGET_ARCH}"
    fi

    cd "$build_dir"

    local cmake_args=()
    if [[ "$CMAKE_GENERATOR" == "Ninja" ]]; then
        cmake_args+=(-j "$JOBS")
    fi

    cmake --build . --config "${BUILD_TYPE}" "${cmake_args[@]}"

    log_success "Build completed successfully"
}

# ─────────────────────────────────────────────────────────────────────────────
# TESTS
# ─────────────────────────────────────────────────────────────────────────────

run_tests() {
    log_step "Running Tests"

    local build_dir="${BUILD_DIR}"
    if [[ "$HOST_ARCH" != "$TARGET_ARCH" ]]; then
        build_dir="${BUILD_DIR}/${TARGET_ARCH}"
    fi

    cd "$build_dir"

    if [[ "$BUILD_TESTS" != "ON" ]]; then
        log_warn "Tests not enabled. Re-run with --tests to enable."
        return 0
    fi

    ctest --output-on-failure -j "$JOBS"

    log_success "All tests passed"
}

# ─────────────────────────────────────────────────────────────────────────────
# INSTALL
# ─────────────────────────────────────────────────────────────────────────────

install_project() {
    log_step "Installing ${PROJECT_NAME}"

    local build_dir="${BUILD_DIR}"
    if [[ "$HOST_ARCH" != "$TARGET_ARCH" ]]; then
        build_dir="${BUILD_DIR}/${TARGET_ARCH}"
    fi

    cd "$build_dir"

    sudo cmake --install . --config "${BUILD_TYPE}"

    log_success "Installed to ${INSTALL_PREFIX}"
}

# ─────────────────────────────────────────────────────────────────────────────
# PACKAGE
# ─────────────────────────────────────────────────────────────────────────────

create_package() {
    log_step "Creating Package"

    local build_dir="${BUILD_DIR}"
    if [[ "$HOST_ARCH" != "$TARGET_ARCH" ]]; then
        build_dir="${BUILD_DIR}/${TARGET_ARCH}"
    fi

    cd "$build_dir"

    # Generate CPack configuration
    cmake --build . --target package

    log_success "Package created in ${build_dir}"
    ls -lh "${build_dir}"/${PROJECT_NAME}*.{deb,rpm,tar.gz} 2>/dev/null || true
}

# ─────────────────────────────────────────────────────────────────────────────
# CLEAN
# ─────────────────────────────────────────────────────────────────────────────

clean_build() {
    log_step "Cleaning Build Directory"

    if [[ -d "$BUILD_DIR" ]]; then
        rm -rf "$BUILD_DIR"
        log_success "Build directory removed"
    else
        log_info "Build directory doesn't exist, nothing to clean"
    fi
}

# ─────────────────────────────────────────────────────────────────────────────
# HELP
# ─────────────────────────────────────────────────────────────────────────────

show_help() {
    cat << EOF
${WHITE}${PROJECT_NAME} Build Script${NC}
Usage: $(basename "$0") [OPTIONS]

${CYAN}BUILD OPTIONS:${NC}
  --debug           Build with Debug configuration
  --release         Build with Release configuration
  --relwithdebinfo  Build with RelWithDebInfo configuration (default)
  --clean           Clean build directory before building
  --reconfigure     Force CMake reconfiguration
  --jobs <N>        Number of parallel jobs (default: $(nproc))

${CYAN}COMPONENTS:${NC}
  --no-ui           Disable Qt6 UI build
  --cli             Enable CLI tool build
  --tests           Enable test build and run
  --docs            Enable documentation build

${CYAN}SANITIZERS:${NC}
  --asan            Enable AddressSanitizer
  --tsan            Enable ThreadSanitizer
  --ubsan           Enable UndefinedBehaviorSanitizer

${CYAN}PACKAGE:${NC}
  --package         Create DEB/RPM/tar.gz package after build
  --install         Install to ${INSTALL_PREFIX} after build

${CYAN}ARCHITECTURE:${NC}
  --arch <ARCH>     Target architecture (x86_64, aarch64)

${CYAN}ACTIONS:${NC}
  --check           Check dependencies only
  --fetch           Fetch external dependencies
  --submodules      Initialize git submodules
  --help            Show this help message

${CYAN}EXAMPLES:${NC}
  $(basename "$0")                     # Build with defaults
  $(basename "$0") --debug --tests     # Debug build with tests
  $(basename "$0") --clean --release   # Clean release build
  $(basename "$0") --package           # Build and create package
  $(basename "$0") --arch aarch64      # Cross-compile for aarch64

${CYAN}ENVIRONMENT:${NC}
  CC=<compiler>     C compiler override
  CXX=<compiler>    C++ compiler override
  BUILD_DIR=<path>  Build directory override
  INSTALL_PREFIX=<path>  Install prefix override
EOF
}

# ─────────────────────────────────────────────────────────────────────────────
# MAIN
# ─────────────────────────────────────────────────────────────────────────────

main() {
    local do_clean=0
    local do_reconfigure=0
    local do_check=0
    local do_fetch=0
    local do_submodules=0
    local do_install=0
    local do_package=0
    local do_run_tests=0

    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --debug)
                BUILD_TYPE="Debug"
                ;;
            --release)
                BUILD_TYPE="Release"
                ;;
            --relwithdebinfo)
                BUILD_TYPE="RelWithDebInfo"
                ;;
            --clean)
                do_clean=1
                ;;
            --reconfigure)
                do_reconfigure=1
                ;;
            --jobs|-j)
                JOBS="$2"
                shift
                ;;
            --no-ui)
                BUILD_UI="OFF"
                ;;
            --cli)
                BUILD_CLI="ON"
                ;;
            --tests)
                BUILD_TESTS="ON"
                do_run_tests=1
                ;;
            --docs)
                BUILD_DOCS="ON"
                ;;
            --asan)
                ENABLE_ASAN="ON"
                ;;
            --tsan)
                ENABLE_TSAN="ON"
                ;;
            --ubsan)
                ENABLE_UBSAN="ON"
                ;;
            --package)
                do_package=1
                ;;
            --install)
                do_install=1
                ;;
            --arch)
                TARGET_ARCH="$2"
                shift
                ;;
            --check)
                do_check=1
                ;;
            --fetch)
                do_fetch=1
                ;;
            --submodules)
                do_submodules=1
                ;;
            --help|-h)
                show_help
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                echo "Use --help for usage information"
                exit 1
                ;;
        esac
        shift
    done

    # Banner
    echo -e "${WHITE}"
    cat << 'EOF'
 ╔═══════════════════════════════════════════════════════════════╗
 ║                                                               ║
 ║   ██╗     ██╗████████╗████████╗███████╗██╗          ██████╗   ║
 ║   ██║     ██║╚══██╔══╝╚══██╔══╝██╔════╝██║          ██╔══██╗  ║
 ║   ██║     ██║   ██║      ██║   █████╗  ██║          ██████╔╝  ║
 ║   ██║     ██║   ██║      ██║   ██╔══╝  ██║          ██╔══██╗  ║
 ║   ███████╗██║   ██║      ██║   ███████╗███████╗     ██████╔╝  ║
 ║   ╚══════╝╚═╝   ╚═╝      ╚═╝   ╚══════╝╚══════╝     ╚═════╝   ║
 ║                                                               ║
 ║   Native Minecraft Bedrock Launcher for Linux                ║
 ║                                                               ║
 ╚═══════════════════════════════════════════════════════════════╝
EOF
    echo -e "${NC}"

    log_info "Build type: ${BUILD_TYPE}"
    log_info "Architecture: ${TARGET_ARCH}"
    log_info "Jobs: ${JOBS}"
    log_info "UI: ${BUILD_UI}"
    log_info "Tests: ${BUILD_TESTS}"
    echo ""

    # Handle BUILD_DIR override
    if [[ -n "${BUILD_DIR:-}" ]]; then
        BUILD_DIR="${BUILD_DIR:-${SCRIPT_DIR}/build}"
    fi

    # Execute build steps
    local steps=()

    if [[ $do_check -eq 1 ]]; then
        check_dependencies
        exit $?
    fi

    if [[ $do_fetch -eq 1 ]]; then
        fetch_deps
        exit 0
    fi

    if [[ $do_submodules -eq 1 ]]; then
        init_submodules
        exit 0
    fi

    # Full build pipeline
    check_dependencies || exit 1
    init_submodules
    fetch_deps

    if [[ $do_clean -eq 1 ]]; then
        clean_build
    fi

    # Determine if cmake needs to run: no build.ninja, forced reconfigure, or clean
    local build_dir="${BUILD_DIR}"
    if [[ "$HOST_ARCH" != "$TARGET_ARCH" ]]; then
        build_dir="${BUILD_DIR}/${TARGET_ARCH}"
    fi

    if [[ $do_reconfigure -eq 1 ]] || [[ ! -f "${build_dir}/build.ninja" ]] && [[ ! -f "${build_dir}/Makefile" ]]; then
        if [[ -d "${build_dir}" ]]; then
            log_warn "Build directory exists but is incomplete, reconfiguring..."
        fi
        configure_cmake
    else
        log_info "Using existing CMake configuration"
        log_info "Run with --reconfigure to regenerate"
    fi

    build_project

    if [[ $do_run_tests -eq 1 ]]; then
        run_tests
    fi

    if [[ $do_package -eq 1 ]]; then
        create_package
    fi

    if [[ $do_install -eq 1 ]]; then
        install_project
    fi

    # Summary
    echo ""
    log_step "Build Complete"
    log_success "${PROJECT_NAME} built successfully!"
    echo ""
    echo -e "${CYAN}Build directory:${NC} ${BUILD_DIR}"
    echo -e "${CYAN}Build type:${NC} ${BUILD_TYPE}"
    echo -e "${CYAN}Architecture:${NC} ${TARGET_ARCH}"
    echo ""
    echo -e "${YELLOW}To run:${NC}"
    echo "  ${BUILD_DIR}/src/ui/LeviLinuxUI"
    echo ""
    echo -e "${YELLOW}To install:${NC}"
    echo "  sudo cmake --install ${BUILD_DIR}"
    echo ""
    echo -e "${YELLOW}To create package:${NC}"
    echo "  $(basename "$0") --package"
    echo ""
}

# Run main
main "$@"