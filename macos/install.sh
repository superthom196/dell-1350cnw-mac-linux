#!/bin/sh
# Installs the Dell 1350cnw driver and adds a network printer queue.
# Usage: sudo sh install.sh <printer-ip> [queue-name]
set -e
cd "$(dirname "$0")"

IP="$1"
QUEUE="${2:-Dell_1350cnw}"
D=/Library/Printers/foo2hbpl1

if [ -z "$IP" ]; then
    echo "Usage: sudo sh install.sh <printer-ip> [queue-name]" >&2
    echo "  e.g. sudo sh install.sh 192.168.1.50" >&2
    exit 1
fi
if [ "$(id -u)" -ne 0 ]; then
    echo "Please run with sudo: sudo sh install.sh $*" >&2
    exit 1
fi

if [ ! -x bin/foo2hbpl1 ] || [ ! -x bin/rastertoppm ]; then
    echo "No prebuilt binaries in bin/, building them (needs Xcode Command Line Tools)..."
    make
fi

if ! nc -z -G 3 "$IP" 9100 2>/dev/null; then
    echo "Warning: nothing answering on $IP port 9100. Installing anyway." >&2
fi

mkdir -p "$D"
cp bin/foo2hbpl1 bin/rastertoppm hbpl1-filter Dell-1350cnw.ppd "$D/"
xattr -dr com.apple.quarantine "$D" 2>/dev/null || true
chown -R root:wheel "$D"
chmod 755 "$D" "$D/foo2hbpl1" "$D/rastertoppm" "$D/hbpl1-filter"
chmod 644 "$D/Dell-1350cnw.ppd"

lpadmin -p "$QUEUE" -E -v "socket://$IP:9100" -P "$D/Dell-1350cnw.ppd" \
    -D "Dell 1350cnw" -L "$IP" \
    -o printer-is-shared=false -o PageSize=A4 2>/dev/null
cupsenable "$QUEUE"
cupsaccept "$QUEUE"

echo "Installed printer '$QUEUE' -> socket://$IP:9100"
echo "It is now in System Settings > Printers & Scanners and every Print dialog."
