#!/usr/bin/env bash
# =============================================================
# ci.sh — MyAutoSarCP 一键 CI 脚本
# 用法:
#   bash scripts/ci.sh            # 完整流程（构建+测试+覆盖率）
#   bash scripts/ci.sh build      # 仅构建 Release
#   bash scripts/ci.sh test       # 构建 Debug + 运行测试
#   bash scripts/ci.sh coverage   # 构建 Debug + 测试 + 覆盖率报告
# =============================================================
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_RELEASE="${ROOT}/build"
BUILD_DEBUG="${ROOT}/build_coverage"
REPORT_DIR="${ROOT}/coverage_report"
JOBS="$(sysctl -n hw.logicalcpu 2>/dev/null || nproc)"

CXX_COMPILER="$(xcrun -f clang++ 2>/dev/null || which clang++)"
CC_COMPILER="$(xcrun -f clang 2>/dev/null || which clang)"
GTEST_DIR="/opt/homebrew/lib/cmake/GTest"
LCOV_IGNORE="inconsistent,unused,empty,format,mismatch,category,unsupported"

info()  { echo -e "\033[1;34m[CI]\033[0m $*"; }
ok()    { echo -e "\033[1;32m[OK]\033[0m $*"; }
step()  { echo ""; echo -e "\033[1;33m── $* ──\033[0m"; }

# ── Stages ────────────────────────────────────────────────────

do_build_release() {
    step "Build (Release)"
    cmake -S "${ROOT}" -B "${BUILD_RELEASE}" \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_C_COMPILER="${CC_COMPILER}" \
          -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" \
          -DCMAKE_OSX_ARCHITECTURES=arm64 \
          -DBUILD_TESTS=OFF > /dev/null
    cmake --build "${BUILD_RELEASE}" --parallel "${JOBS}"
    ok "Release build → ${BUILD_RELEASE}/MyAutoSarCP"
}

do_build_debug() {
    step "Build (Debug + Coverage)"
    cmake -S "${ROOT}" -B "${BUILD_DEBUG}" \
          -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_C_COMPILER="${CC_COMPILER}" \
          -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" \
          -DCMAKE_OSX_ARCHITECTURES=arm64 \
          -DGTest_DIR="${GTEST_DIR}" \
          -DBUILD_TESTS=ON \
          -DENABLE_COVERAGE=ON > /dev/null
    cmake --build "${BUILD_DEBUG}" --parallel "${JOBS}"
    ok "Debug build complete"
}

do_test() {
    step "Unit Tests"
    find "${BUILD_DEBUG}" -name "*.gcda" -delete 2>/dev/null || true
    cd "${BUILD_DEBUG}"
    ctest --output-on-failure --parallel "${JOBS}"
    cd "${ROOT}"
    ok "All tests passed"
}

do_coverage() {
    step "Coverage Report (lcov)"
    local RAW="${BUILD_DEBUG}/coverage_raw.info"
    local FILTERED="${BUILD_DEBUG}/coverage.info"

    lcov --capture \
         --directory "${BUILD_DEBUG}" \
         --source-directory "${ROOT}" \
         --output-file "${RAW}" \
         --ignore-errors "${LCOV_IGNORE}" \
         --rc branch_coverage=0 --quiet

    lcov --remove "${RAW}" \
         '*/googletest/*' '*/gtest/*' '/usr/*' '/Library/*' '/opt/*' \
         '*/tests/unit/*' \
         --output-file "${FILTERED}" \
         --ignore-errors "${LCOV_IGNORE}" --quiet

    rm -rf "${REPORT_DIR}"
    genhtml "${FILTERED}" \
            --output-directory "${REPORT_DIR}" \
            --title "MyAutoSarCP Coverage (AUTOSAR CP R25-11)" \
            --legend --show-details \
            --prefix "${ROOT}" \
            --ignore-errors "${LCOV_IGNORE},unsupported" --quiet

    ok "Coverage report → ${REPORT_DIR}/index.html"
    lcov --summary "${FILTERED}" 2>&1 | grep -E 'lines|functions' || true
}

# ── Main ──────────────────────────────────────────────────────
MODE="${1:-all}"
info "MyAutoSarCP CI  |  mode=${MODE}  |  jobs=${JOBS}"

case "${MODE}" in
    build)    do_build_release ;;
    test)     do_build_debug; do_test ;;
    coverage) do_build_debug; do_test; do_coverage ;;
    all|"")   do_build_release; do_build_debug; do_test; do_coverage ;;
    *) echo "Usage: $0 [build|test|coverage|all]"; exit 1 ;;
esac

echo ""; ok "CI finished ✓"
