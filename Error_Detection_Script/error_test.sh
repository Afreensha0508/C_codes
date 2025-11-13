#!/bin/bash
# ==============================================
# auto_error_check.sh
# Purpose: Automate detection of kernel errors and pstore logs
# ==============================================

# Directory to save logs
LOG_DIR="$HOME/kernel_error_logs"
mkdir -p "$LOG_DIR"

# Timestamp for this run
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
OUTPUT_FILE="$LOG_DIR/errors_$TIMESTAMP.txt"

echo "========================================" | tee -a "$OUTPUT_FILE"
echo "Error scan run at: $(date)" | tee -a "$OUTPUT_FILE"
echo "========================================" | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

# -------------------------
# 1. Current boot kernel logs
# -------------------------
echo "=== Current boot kernel logs ===" | tee -a "$OUTPUT_FILE"
journalctl -k -b | grep -iE "error|fail|panic|oops" | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

# -------------------------
# 2. Previous boot kernel logs
# -------------------------
echo "=== Previous boot kernel logs ===" | tee -a "$OUTPUT_FILE"
journalctl -k -b -1 | grep -iE "error|fail|panic|oops" | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

# -------------------------
# 3. pstore logs (if available)
# -------------------------
if [ -d /sys/fs/pstore ]; then
    echo "=== pstore logs ===" | tee -a "$OUTPUT_FILE"
    for f in /sys/fs/pstore/*; do
        echo "--- $f ---" | tee -a "$OUTPUT_FILE"
        cat "$f" | tee -a "$OUTPUT_FILE"
        echo "" | tee -a "$OUTPUT_FILE"
    done
else
    echo "pstore not mounted" | tee -a "$OUTPUT_FILE"
fi

echo "========================================" | tee -a "$OUTPUT_FILE"
echo "Scan completed. Logs saved to $OUTPUT_FILE"

