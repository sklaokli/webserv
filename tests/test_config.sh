#!/usr/bin/env bash

# Colors
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
CYAN="\033[0;36m"
RESET="\033[0m"

BIN="./webserv"
TOTAL=0
PASSED=0
FAILED=0

if [ ! -f "$BIN" ]; then
    echo -e "${RED}Error: Binary $BIN not found. Run 'make' first.${RESET}"
    exit 1
fi

echo -e "${CYAN}==============================================${RESET}"
echo -e "${CYAN}        WEBSERV CONFIG TEST RUNNER           ${RESET}"
echo -e "${CYAN}==============================================${RESET}\n"

run_valid_test() {
    local file="$1"
    local name=$(basename "$file")
    TOTAL=$((TOTAL + 1))

    output=$("$BIN" "$file" 2>&1)
    exit_code=$?

    if [ $exit_code -eq 0 ]; then
        echo -e "  [${GREEN}PASS${RESET}] (valid)   $name"
        PASSED=$((PASSED + 1))
    else
        echo -e "  [${RED}FAIL${RESET}] (valid)   $name (unexpected exit code $exit_code)"
        echo -e "         ${YELLOW}$output${RESET}"
        FAILED=$((FAILED + 1))
    fi
}

run_invalid_test() {
    local file="$1"
    local name=$(basename "$file")
    TOTAL=$((TOTAL + 1))

    output=$("$BIN" "$file" 2>&1)
    exit_code=$?

    if [ $exit_code -ne 0 ]; then
        err_msg=$(echo "$output" | grep -m1 "\[ERROR\]" | sed 's/.*\[ERROR\] //')
        echo -e "  [${GREEN}PASS${RESET}] (invalid) $name"
        if [ -n "$err_msg" ]; then
            echo -e "         ↳ ${YELLOW}$err_msg${RESET}"
        fi
        PASSED=$((PASSED + 1))
    else
        echo -e "  [${RED}FAIL${RESET}] (invalid) $name (should have failed but passed)"
        FAILED=$((FAILED + 1))
    fi
}

echo -e "${CYAN}--- Testing Valid Configurations ---${RESET}"
for f in conf/*.conf tests/configs/valid/*.conf; do
    [ -f "$f" ] || continue
    run_valid_test "$f"
done

echo -e "\n${CYAN}--- Testing Invalid Configurations ---${RESET}"
for f in tests/configs/invalid/*.conf; do
    [ -f "$f" ] || continue
    run_invalid_test "$f"
done

echo -e "\n${CYAN}==============================================${RESET}"
echo -e "Total: $TOTAL | Passed: ${GREEN}$PASSED${RESET} | Failed: ${RED}$FAILED${RESET}"
echo -e "${CYAN}==============================================${RESET}"

if [ $FAILED -eq 0 ]; then
    exit 0
else
    exit 1
fi

