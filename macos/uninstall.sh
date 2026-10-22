#!/bin/sh
# Removes the printer queue and the driver files.
# Usage: sudo sh uninstall.sh [queue-name]
QUEUE="${1:-Dell_1350cnw}"

if [ "$(id -u)" -ne 0 ]; then
    echo "Please run with sudo: sudo sh uninstall.sh $*" >&2
    exit 1
fi

lpadmin -x "$QUEUE" 2>/dev/null && echo "Removed printer '$QUEUE'"
rm -rf /Library/Printers/foo2hbpl1 && echo "Removed /Library/Printers/foo2hbpl1"
