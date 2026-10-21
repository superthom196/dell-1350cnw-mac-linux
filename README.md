# Dell 1350cnw on modern Linux

A working, open-source driver setup for the **Dell 1350cnw** colour laser printer (and its relatives) on current Linux systems, with full colour and A4/Letter printing over the network.

Dell never released a Linux driver for this printer. Xerox's old Phaser 6000/6010 Linux driver can no longer be downloaded, and the standard `foo2zjs` driver doesn't speak this printer's language. This repo packages the open-source **foo2hbpl1** driver, which does.

## Tested

| | |
|---|---|
| Printer | Dell 1350cnw, firmware 201510201256, network firmware 17.12 |
| Connection | Wi-Fi, raw socket (port 9100) |
| OS | Arch Linux ([Omarchy](https://omarchy.org)), x86_64, CUPS 2.4, Ghostscript 10.08 |
| Result | CUPS test page and colour test pages print correctly, A4, colour |
| Date | October 2026 |

Other distributions should work with the manual steps below, but I've only tested Arch.

## Why the usual drivers fail

The 1350cnw is a rebadged **Xerox Phaser 6010N** (Fuji Xerox makes the hardware). It speaks **HBPL version 1**, a host-based printer language. Many users assume it can use these drivers, which fail:

- **Xerox Phaser 6000/6010 Linux driver:** proprietary, 32-bit, from 2011, and Xerox's download links are now dead.
- **foo2zjs `foo2hbpl2`** (Dell 1355 / Xerox WorkCentre 6015 profiles): this speaks HBPL **version 2**. The printer pulls in a sheet, then stops, and logs **`PDL Error. PDL Request Data Violation. 016-720`**. If you see that error, you have the wrong HBPL version.

**foo2hbpl1** was reverse-engineered by Dave Coffin and is maintained in [mikerr/foo2zjs](https://github.com/mikerr/foo2zjs). It hasn't been merged into mainline foo2zjs, so most distributions don't ship it.

## Printers that use HBPLv1

Upstream ([mikerr/foo2zjs](https://github.com/mikerr/foo2zjs)) lists these as working with foo2hbpl1:

- Dell 1250c, C1660w, C1760nw
- Xerox Phaser 6010N
- Fuji Xerox DocuPrint CP105b
- Epson AcuLaser C1700

Added here: **Dell 1350cnw**, tested. The Xerox Phaser 6000 and 6010 belong to the same family and will probably work too, but I haven't confirmed them.

If the printer's own configuration page says "HBPL" and the HBPLv2 driver fails with error 016-720, it's probably HBPLv1.

## Install on Arch Linux (and Arch-based: Omarchy, EndeavourOS, Manjaro…)

The package is self-contained. It doesn't need `foo2zjs` or `foo2zjs-nightly` from the AUR.

```bash
git clone https://github.com/superthom196/dell-1350cnw-linux.git
cd dell-1350cnw-linux
makepkg -si
```

This builds `foo2hbpl1` from source, pinned to a known-good commit, and installs:

- `/usr/bin/foo2hbpl1` and `/usr/bin/foo2hbpl1-wrapper`, the driver
- `/usr/lib/foo2hbpl1/foo2zjs-pstops`, a private copy of a helper script the driver needs
- `/usr/share/ppd/foo2hbpl1/*.ppd`, printer profiles for CUPS

To uninstall:

```bash
sudo pacman -R foo2hbpl1
```

## Install on other distributions (Debian, Ubuntu, Fedora…)

These steps are untested. They mirror what the Arch package does.

```bash
# Debian/Ubuntu dependencies
sudo apt install build-essential git ghostscript cups cups-filters bc
# Fedora dependencies
# sudo dnf install gcc make git ghostscript cups cups-filters bc

git clone https://github.com/mikerr/foo2zjs.git
cd foo2zjs
git checkout 5bf0142d1e3d4363684608ac42933510d3b66e27
rm -f foo2hbpl1 foo2hbpl1.o foo2hbpl1-wrapper
make foo2hbpl1 foo2hbpl1-wrapper
sed -i -E 's@(^|[[:space:];(])foo2zjs-pstops @\1/usr/lib/foo2hbpl1/foo2zjs-pstops @g' foo2hbpl1-wrapper

sudo install -Dm755 foo2hbpl1          /usr/bin/foo2hbpl1
sudo install -Dm755 foo2hbpl1-wrapper  /usr/bin/foo2hbpl1-wrapper
sudo install -Dm755 foo2zjs-pstops.sh  /usr/lib/foo2hbpl1/foo2zjs-pstops
sudo install -Dm644 PPD/Fuji_Xerox-DocuPrint_CP105.ppd \
  /usr/share/ppd/foo2hbpl1/Fuji_Xerox-DocuPrint_CP105-foo2hbpl1.ppd
```

The binaries go in `/usr/bin` because CUPS filters don't search `/usr/local/bin`. These file names are chosen not to overlap with mainline foo2zjs, which doesn't include `foo2hbpl1`. I haven't checked this against every distribution's package.

## Add the printer

### 1. Find the printer's IP address

On the printer's control panel, print a **Printer Settings** report. Its network section shows the IP address. You can also look in your router's list of connected devices for something named Dell or Fuji Xerox. On my unit, the network chip's MAC address starts with `08:00:37` (Fuji Xerox).

Check that the printer accepts raw jobs:

```bash
timeout 3 bash -c '</dev/tcp/192.168.0.201/9100' && echo "printer reachable"
```

The printer can be slow to answer while it's in deep sleep. If the check fails, press a button on the printer and try again.

### 2. Create the print queue

Replace the IP address with your printer's.

```bash
sudo lpadmin -p Dell_1350cnw -E \
  -v socket://192.168.0.201:9100 \
  -P /usr/share/ppd/foo2hbpl1/Fuji_Xerox-DocuPrint_CP105-foo2hbpl1.ppd \
  -D "Dell 1350cnw" -L "Network" \
  -o PageSize=A4 -o ColorMode=Color
sudo lpadmin -d Dell_1350cnw   # optional: make it the default printer
```

Use `PageSize=Letter` in the US. The Dell 1250c, C1660, C1760 and CP105 profiles are identical apart from the model name, so any of them works.

To set it up with a graphical tool instead, open the CUPS web interface at <http://localhost:631> or `system-config-printer`. Add a network printer using **AppSocket/HP JetDirect** at your printer's IP on port 9100, and choose **Provide PPD file**. Browse to the `.ppd` file above.

CUPS prints a warning that printer drivers are deprecated. That's expected; it still works on CUPS 2.4.

### 3. Print a test page

```bash
lp -d Dell_1350cnw /usr/share/cups/data/testprint
```

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| Printer feeds a sheet, then stops; log shows **016-720 PDL Request Data Violation** | Wrong driver (HBPLv2 / foo2hbpl2). Use the foo2hbpl1 profile from this package. |
| Nothing happens at all | Wrong IP, or the printer is asleep. Check with the reachability command above and look at the printer's web page at `http://<ip>/`. Its **Printer Events** page shows error codes. |
| Job fails in colour when calling `foo2hbpl1-wrapper` by hand with only `-c` | Bare `-c` uses a colour-calibration mode whose files this package doesn't include. Pass the same options CUPS does: `-c -C10 -Gnone.icm`. Printing through CUPS isn't affected. |

To test the driver without CUPS, which is handy for debugging:

```bash
foo2hbpl1-wrapper -c -C10 -Gnone.icm -p a4 some-file.ps > job.hbpl
cat job.hbpl > /dev/tcp/192.168.0.201/9100    # bash only
```

## Credits

- **Dave Coffin** reverse-engineered HBPLv1 and wrote `foo2hbpl1`.
- **Rick Richardson** wrote the [foo2zjs](https://foo2zjs.linkevich.net/) driver framework.
- **[mikerr/foo2zjs](https://github.com/mikerr/foo2zjs)** maintains the HBPLv1 fork this package builds from.
- [Dechifro's HBPL page](http://www.dechifro.org/hbpl/) has background on the printer language.

## License

This repository's packaging files are licensed under GPL-2.0-or-later, the same as the foo2zjs driver they build. See [LICENSE](LICENSE).
