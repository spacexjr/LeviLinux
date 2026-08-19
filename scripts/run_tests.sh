#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════════
# LeviLinux Test Runner
# Runs unit and integration tests
# ═══════════════════════════════════════════════════════════════════════════════

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_ROOT}/build"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info()  { echo -e "${GREEN}[INFO]${NC} $*"; }
log_warn()  { echo -e "${YELLOW}[WARN]${NC} $*"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*"; }
log_step()  { echo -e "\n${CYAN}═══ $* ═══${NC}"; }

# Parse arguments
VERBOSE=0
FILTER=""
MEMORY_CHECK=0
COVERAGE=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        -v|--verbose)   VERBOSE=1 ;;
        -f|--filter)    FILTER="$2"; shift ;;
        -m|--memory)    MEMORY_CHECK=1 ;;
        -c|--coverage)  COVERAGE=1 ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -v, --verbose     Verbose test output"
            echo "  -f, --filter <P>  Filter tests by pattern"
            echo "  -m, --memory      Run with Valgrind memory check"
            echo "  -c, --coverage    Generate code coverage report"
            echo "  --help            Show this help"
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            exit 1
            ;;
    esac
    shift
done

# Check if build exists
if [[ ! -d "$BUILD_DIR" ]]; then
    log_error "Build directory not found. Run ./build.sh first."
    exit 1
fi

cd "$BUILD_DIR"

# Check if tests were built
if [[ ! -f CTestTestfile.cmake ]]; then
    log_error "Tests not configured. Rebuild with: ./build.sh --tests"
    exit 1
fi

log_step "Running LeviLinux Tests"

# Build test arguments
CTEST_ARGS=("--output-on-failure")
if [[ $VERBOSE -eq 1 ]]; then
    CTEST_ARGS+=("--verbose")
fi
if [[ -n "$FILTER" ]]; then
    CTEST_ARGS+=("-R" "$FILTER")
fi

# Memory check with Valgrind
if [[ $MEMORY_CHECK -eq 1 ]]; then
    if ! command -v valgrind &>/dev/null; then
        log_warn "Valgrind not found, skipping memory check"
    else
        log_step "Running tests with Valgrind"
        export CTEST_MEMORYCHECK_COMMAND="valgrind"
        export CTEST_MEMORYCHECK_COMMAND_OPTIONS="--leak-check=full --show-leak-kinds=all --track-origins=yes"
        ctest -T memcheck "${CTEST_ARGS[@]}"
    fi
fi

# Coverage
if [[ $COVERAGE -eq 1 ]]; then
    log_step "Generating coverage report"
    if command -v gcov &>/dev/null; then
        gcov -r -b "$BUILD_DIR"/**/*.gcno 2>/dev/null || true
    fi
    if command -v lcov &>/dev/null; then
        lcov --capture --directory "$BUILD_DIR" --output-file coverage.info 2>/dev/null || true
        if command -v genhtml &>/dev/null; then
            genhtml coverage.info --output-directory coverage_report 2>/dev/null || true
            log_info "Coverage report: $BUILD_DIR/coverage_report/index.html"
        fi
    fi
fi

# Run tests
log_step "Executing test suite"
ctest "${CTEST_ARGS[@]}"
TEST_EXIT=$?

# Summary
echo ""
if [[ $TEST_EXIT -eq 0 ]]; then
    log_info "All tests passed!"
else
    log_error "Some tests failed (exit code: $TEST_EXIT)"
fi

exit $TEST_EXIT