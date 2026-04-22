#!/bin/bash

# Already allocated 2GB Hugepage and You don't need to run this
TARGET_GB=2
# ===========================

if [[ $EUID -ne 0 ]]; then
    echo "Please run as root (use sudo)"
    exit 1
fi

echo "Reading HugePage size..."

HUGESIZE_KB=$(grep Hugepagesize /proc/meminfo | awk '{print $2}')
HUGESIZE_MB=$((HUGESIZE_KB / 1024))

echo "HugePage size: ${HUGESIZE_MB} MB"

TARGET_MB=$((TARGET_GB * 1024))
NUM_PAGES=$((TARGET_MB / HUGESIZE_MB))

echo "Allocating $NUM_PAGES huge pages (~${TARGET_GB}GB)"

sysctl -w vm.nr_hugepages=$NUM_PAGES

echo
echo "Verification:"
grep Huge /proc/meminfo