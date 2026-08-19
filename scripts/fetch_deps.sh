#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════════
# LeviLinux Dependency Fetch Script
# Automatically installs required and optional dependencies
# ═══════════════════════════════════════════════════════════════════════════════

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info()  { echo -e "${GREEN}[INFO]${NC} $*"; }
log_warn()  { echo -e "${YELLOW}[WARN]${NC} $*"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*"; }
log_step()  { echo -e "\n${CYAN}═══ $* ═══${NC}"; }

# ─────────────────────────────────────────────────────────────────────────────
# DETECT DISTRO
# ─────────────────────────────────────────────────────────────────────────────

detect_distro() {
    if [[ -f /etc/os-release ]]; then
        . /etc/os-release
        DISTRO_ID="${ID:-unknown}"
        DISTRO_VERSION="${VERSION_ID:-}"
        DISTRO_CODENAME="${VERSION_CODENAME:-}"
    else
        DISTRO_ID="unknown"
    fi

    if [[ -f /etc/debian_version ]]; then
        DISTRO_FAMILY="debian"
    elif [[ -f /etc/redhat-release ]]; then
        DISTRO_FAMILY="redhat"
    elif [[ -f /etc/arch-release ]]; then
        DISTRO_FAMILY="arch"
    else
        DISTRO_FAMILY="unknown"
    fi

    log_info "Detected: ${DISTRO_ID} ${DISTRO_VERSION} (${DISTRO_FAMILY})"
}

# ─────────────────────────────────────────────────────────────────────────────
# DEBIAN/UBUNTU
# ─────────────────────────────────────────────────────────────────────────────

install_debian() {
    log_step "Installing dependencies (Debian/Ubuntu)"

    local REQUIRED=(
        build-essential
        cmake
        ninja-build
        pkg-config
        git
        g++
        gcc
        libgl-dev
        libcurl4-openssl-dev
        libmbedtls-dev
        zlib1g-dev
        qt6-base-dev
        qt6-tools-dev
        qt6-tools-dev-tools
        qt6-l10n-tools
        libqt6svg6-dev
        libqt6opengl6-dev
        libqt6network6
        libqt6concurrent6
    )

    local OPTIONAL=(
        libglfw3-dev
        libvulkan-dev
        libsdl2-dev
        libpipewire-0.3-dev
        libpulse-dev
        libcapstone-dev
        libfmt-dev
        libdwarf-dev
        libunwind-dev
        libfmod-dev
        ccache
    )

    log_info "Updating package lists..."
    sudo apt update -qq

    log_info "Installing required packages..."
    sudo apt install -y "${REQUIRED[@]}"

    log_info "Installing optional packages..."
    sudo apt install -y "${OPTIONAL[@]}" || log_warn "Some optional packages may not be available"

    # Qt6 additional components
    log_info "Installing Qt6 additional components..."
    sudo apt install -y \
        libqt6core6 \
        libqt6widgets6 \
        libqt6gui6 \
        libqt6network6 \
        qt6-base-dev-tools \
        2>/dev/null || true

    # FMOD (may need manual install)
    if ! dpkg -l | grep -q fmod; then
        log_warn "FMOD not in repositories. Audio will use PipeWire/PulseAudio fallback."
        log_info "To install FMOD: download from https://fmod.com/download"
    fi
}

# ─────────────────────────────────────────────────────────────────────────────
# FEDORA/RHEL
# ─────────────────────────────────────────────────────────────────────────────

install_redhat() {
    log_step "Installing dependencies (Fedora/RHEL)"

    local REQUIRED=(
        gcc
        gcc-c++
        cmake
        ninja-build
        pkgconf
        git
        mesa-libGL-devel
        libcurl-devel
        mbedtls-devel
        zlib-devel
        qt6-qtbase-devel
        qt6-qtsvg-devel
        qt6-qttools-devel
        qt6-qtnetworkauth-devel
    )

    local OPTIONAL=(
        glfw-devel
        vulkan-devel
        SDL2-devel
        pipewire-devel
        pulseaudio-libs-devel
        capstone-devel
        fmt-devel
        libdwarf-devel
        libunwind-devel
        ccache
    )

    log_info "Installing required packages..."
    sudo dnf install -y "${REQUIRED[@]}"

    log_info "Installing optional packages..."
    sudo dnf install -y "${OPTIONAL[@]}" || log_warn "Some optional packages may not be available"
}

# ─────────────────────────────────────────────────────────────────────────────
# ARCH LINUX
# ─────────────────────────────────────────────────────────────────────────────

install_arch() {
    log_step "Installing dependencies (Arch Linux)"

    local REQUIRED=(
        base-devel
        cmake
        ninja
        pkgconf
        git
        mesa
        curl
        mbedtls
        zlib
        qt6-base
        qt6-svg
        qt6-tools
    )

    local OPTIONAL=(
        glfw
        vulkan-headers
        sdl2
        pipewire
        libpulse
        capstone
        fmt
        libdwarf
        libunwind
        ccache
    )

    log_info "Syncing package database..."
    sudo pacman -Sy --noconfirm

    log_info "Installing required packages..."
    sudo pacman -S --needed --noconfirm "${REQUIRED[@]}"

    log_info "Installing optional packages..."
    sudo pacman -S --needed --noconfirm "${OPTIONAL[@]}" || log_warn "Some optional packages may not be available"
}

# ─────────────────────────────────────────────────────────────────────────────
# INSTALL FMOD (Cross-distro)
# ─────────────────────────────────────────────────────────────────────────────

install_fmod() {
    log_step "Installing FMOD"

    local FMOD_VERSION="2.03.07"
    local FMOD_URL="https://www.fmod.com/download/12081/fmodstudio-linux-${FMOD_VERSION//./}.tar.gz"
    local FMOD_DIR="/opt/fmod"

    if [[ -d "$FMOD_DIR" ]]; then
        log_info "FMOD already installed at $FMOD_DIR"
        return 0
    fi

    log_info "Downloading FMOD ${FMOD_VERSION}..."
    local tmp_dir
    tmp_dir=$(mktemp -d)
    cd "$tmp_dir"

    if curl -L -o fmod.tar.gz "$FMOD_URL"; then
        tar -xzf fmod.tar.gz
        sudo mkdir -p "$FMOD_DIR"
        sudo cp -r fmodstudio/api/lowlevel/lib/* "$FMOD_DIR/" 2>/dev/null || true
        sudo cp -r fmodstudio/api/lowlevel/inc/* /usr/local/include/ 2>/dev/null || true
        sudo ldconfig
        rm -rf "$tmp_dir"
        log_success "FMOD installed to $FMOD_DIR"
    else
        log_warn "Could not download FMOD (may require manual download)"
        log_info "Download manually from: https://fmod.com/download"
        rm -rf "$tmp_dir"
    fi
}

# ─────────────────────────────────────────────────────────────────────────────
# INSTALL SDL3 (If not available in repos)
# ─────────────────────────────────────────────────────────────────────────────

install_sdl3() {
    log_step "Checking SDL3"

    if pkg-config --exists sdl3 2>/dev/null; then
        log_success "SDL3 already installed"
        return 0
    fi

    mkdir -p "${SCRIPT_DIR}/third_party"

    # Try latest SDL3 main first (has PipeWire fixes), fallback to SDL2
    local sdl_branch=""
    local sdl_repo="https://github.com/libsdl-org/SDL.git"
    local sdl_dir="${SCRIPT_DIR}/third_party/sdl3"

    # Try to find a compatible SDL3 version
    for branch in "main" "release-3.2.1" "release-3.2.0"; do
        if git ls-remote --tags "$sdl_repo" "refs/tags/$branch" 2>/dev/null | head -1 | grep -q .; then
            sdl_branch="$branch"
            break
        fi
    done

    if [[ -n "$sdl_branch" ]]; then
        log_info "Building SDL3 from branch ${sdl_branch}..."
        rm -rf "$sdl_dir"
        git clone --depth 1 --branch "$sdl_branch" "$sdl_repo" "$sdl_dir"

        cd "$sdl_dir"
        mkdir -p build
        cd build
        cmake .. -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DSDL_SHARED=ON \
            -DSDL_STATIC=OFF \
            -DSDL_PIPEWIRE=OFF \
            -DSDL_PIPEWIRE_SHARED=OFF
        ninja
        sudo ninja install
        sudo ldconfig
        log_success "SDL3 installed from source (${sdl_branch})"
        return 0
    fi

    # Fallback: install SDL2 if SDL3 is not available
    log_warn "SDL3 not available. Falling back to SDL2..."
    install_sdl2
}

install_sdl2() {
    log_step "Checking SDL2"

    if pkg-config --exists sdl2 2>/dev/null; then
        log_success "SDL2 already installed"
        return 0
    fi

    mkdir -p "${SCRIPT_DIR}/third_party"
    local sdl2_dir="${SCRIPT_DIR}/third_party/sdl2"

    if [[ ! -d "$sdl2_dir" ]]; then
        log_info "Building SDL2 from source..."
        git clone --depth 1 --branch release-2.30.0 \
            https://github.com/libsdl-org/SDL.git \
            "$sdl2_dir"

        cd "$sdl2_dir"
        mkdir -p build
        cd build
        cmake .. -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DSDL_SHARED=ON \
            -DSDL_STATIC=OFF
        ninja
        sudo ninja install
        sudo ldconfig
        log_success "SDL2 installed from source"
    fi
}

# ─────────────────────────────────────────────────────────────────────────────
# INSTALL CAPSTONE (If not available in repos)
# ─────────────────────────────────────────────────────────────────────────────

install_capstone() {
    log_step "Checking Capstone"

    if pkg-config --exists capstone 2>/dev/null; then
        log_success "Capstone already installed"
        return 0
    fi

    if [[ ! -d "${SCRIPT_DIR}/third_party/capstone" ]]; then
        log_info "Capstone not found. Building from source..."
        mkdir -p "${SCRIPT_DIR}/third_party"

        git clone --depth 1 --branch 5.0.1 \
            https://github.com/capstone-engine/capstone.git \
            "${SCRIPT_DIR}/third_party/capstone"

        cd "${SCRIPT_DIR}/third_party/capstone"
        mkdir -p build
        cd build
        cmake .. -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DCAPSTONE_BUILD_TESTS=OFF \
            -DCAPSTONE_BUILD_CSTOOL=OFF
        ninja
        sudo ninja install
        sudo ldconfig
        log_success "Capstone installed from source"
    fi
}

# ─────────────────────────────────────────────────────────────────────────────
# INSTALL FMT (If not available in repos)
# ─────────────────────────────────────────────────────────────────────────────

install_fmt() {
    log_step "Checking fmt"

    if pkg-config --exists fmt 2>/dev/null; then
        log_success "fmt already installed"
        return 0
    fi

    if [[ ! -d "${SCRIPT_DIR}/third_party/fmt" ]]; then
        log_info "fmt not found. Building from source..."
        mkdir -p "${SCRIPT_DIR}/third_party"

        git clone --depth 1 --branch 10.2.1 \
            https://github.com/fmtlib/fmt.git \
            "${SCRIPT_DIR}/third_party/fmt"

        cd "${SCRIPT_DIR}/third_party/fmt"
        mkdir -p build
        cd build
        cmake .. -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DFMT_TEST=OFF \
            -DFMT_DOC=OFF
        ninja
        sudo ninja install
        sudo ldconfig
        log_success "fmt installed from source"
    fi
}

# ─────────────────────────────────────────────────────────────────────────────
# INSTALL NLOHMANN_JSON (Header-only, required for mod manifest parsing)
# ─────────────────────────────────────────────────────────────────────────────

install_nlohmann_json() {
    log_step "Checking nlohmann/json"

    if [[ -f /usr/include/nlohmann/json.hpp ]] || \
       [[ -f /usr/local/include/nlohmann/json.hpp ]]; then
        log_success "nlohmann/json already installed"
        return 0
    fi

    log_info "Installing nlohmann/json (header-only library)..."
    sudo apt install -y nlohmann-json3-dev 2>/dev/null || \
    sudo dnf install -y nlohmann-json-devel 2>/dev/null || \
    sudo pacman -S --needed --noconfirm nlohmann-json 2>/dev/null || \
    {
        log_info "Installing manually..."
        local tmp_dir
        tmp_dir=$(mktemp -d)
        cd "$tmp_dir"
        curl -L -o json.hpp https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
        sudo mkdir -p /usr/local/include/nlohmann
        sudo cp json.hpp /usr/local/include/nlohmann/
        rm -rf "$tmp_dir"
        log_success "nlohmann/json installed manually"
    }
}

# ─────────────────────────────────────────────────────────────────────────────
# MAIN
# ─────────────────────────────────────────────────────────────────────────────

main() {
    local install_fmod_flag=0
    local install_sdl3_flag=0
    local install_capstone_flag=0
    local install_fmt_flag=0
    local install_json_flag=0
    local install_all=0

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --fmod)         install_fmod_flag=1 ;;
            --sdl3)         install_sdl3_flag=1 ;;
            --capstone)     install_capstone_flag=1 ;;
            --fmt)          install_fmt_flag=1 ;;
            --json)         install_json_flag=1 ;;
            --all)          install_all=1 ;;
            --help|-h)
                echo "Usage: $0 [OPTIONS]"
                echo ""
                echo "Options:"
                echo "  --all         Install all dependencies (default)"
                echo "  --fmod        Install FMOD"
                echo "  --sdl3        Install SDL3 from source"
                echo "  --capstone    Install Capstone from source"
                echo "  --fmt         Install fmt from source"
                echo "  --json        Install nlohmann/json"
                echo "  --help        Show this help"
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                exit 1
                ;;
        esac
        shift
    done

    # Default to all if no specific options given
    if [[ $install_fmod_flag -eq 0 && $install_sdl3_flag -eq 0 && \
          $install_capstone_flag -eq 0 && $install_fmt_flag -eq 0 && \
          $install_json_flag -eq 0 && $install_all -eq 0 ]]; then
        install_all=1
    fi

    echo -e "\033[1;37m"
    echo "╔═══════════════════════════════════════════════════════════════╗"
    echo "║       LeviLinux Dependency Installer                        ║"
    echo "╚═══════════════════════════════════════════════════════════════╝"
    echo -e "\033[0m"

    detect_distro

    # Install system packages based on distro
    case "$DISTRO_FAMILY" in
        debian)   install_debian ;;
        redhat)   install_redhat ;;
        arch)     install_arch ;;
        *)
            log_error "Unsupported distribution: ${DISTRO_ID}"
            log_info "Please install dependencies manually:"
            echo "  - cmake, ninja, pkg-config, git, g++"
            echo "  - Qt6 (qt6-base-dev)"
            echo "  - OpenGL (libgl-dev)"
            echo "  - libcurl, mbedtls, zlib"
            echo "  - Optional: glfw3, vulkan, sdl3, pipewire, pulseaudio, capstone, fmt"
            exit 1
            ;;
    esac

    # Install additional dependencies that may not be in repos
    if [[ $install_all -eq 1 || $install_json_flag -eq 1 ]]; then
        install_nlohmann_json
    fi

    if [[ $install_all -eq 1 || $install_sdl3_flag -eq 1 ]]; then
        install_sdl3
    fi

    if [[ $install_all -eq 1 || $install_capstone_flag -eq 1 ]]; then
        install_capstone
    fi

    if [[ $install_all -eq 1 || $install_fmt_flag -eq 1 ]]; then
        install_fmt
    fi

    if [[ $install_all -eq 1 || $install_fmod_flag -eq 1 ]]; then
        install_fmod
    fi

    # Summary
    echo ""
    log_step "Installation Complete"
    echo ""
    echo -e "${GREEN}All dependencies installed successfully!${NC}"
    echo ""
    echo -e "Run ${CYAN}./build.sh${NC} to build LeviLinux"
    echo ""
}

main "$@"