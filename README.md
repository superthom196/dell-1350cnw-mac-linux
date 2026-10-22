# Dell 1350cnw driver for macOS and Linux

A working, open-source driver for the **Dell 1350cnw** colour laser printer (and its relatives) on current **macOS** and **Linux**, with full colour and A4/Letter printing over the network.

Dell never released a Linux driver for this printer, and its Mac driver is 32-bit, so it hasn't run since macOS Catalina. Xerox's old Phaser 6000/6010 Linux driver can no longer be downloaded, and the standard `foo2zjs` driver doesn't speak this printer's language. This repo packages the open-source **foo2hbpl1** driver, which does, for both platforms.

| Platform | Folder | Install guide |
|---|---|---|
| macOS (Intel and Apple Silicon) | [`macos/`](macos) | [macOS](#macos) |
| Linux (Arch package; manual steps for other distributions) | [`linux/`](linux) | [Linux](#linux) |

## Tested

| | |
|---|---|
| Printer | Dell 1350cnw, firmware 201510201256, network firmware 17.12 |
| Connection | Wi-Fi, raw socket (port 9100) |
| Linux | Arch Linux ([Omarchy](https://omarchy.org)), x86_64, CUPS 2.4, Ghostscript 10.08 |
| macOS | macOS 26.5 (Tahoe), Intel x86_64. No Ghostscript needed |
| Result | CUPS test page and colour test pages print correctly, A4, colour. On macOS, 4pt text is legible |
| Date | October 2026 |

Other Linux distributions should work with the manual steps below, but I've only tested Arch. The macOS binaries are universal (Intel and Apple Silicon); Apple Silicon is untested.

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

## Find the printer's IP address

On the printer's control panel, print a **Printer Settings** report. Its network section shows the IP address. You can also look in your router's list of connected devices for something named Dell or Fuji Xerox. On my unit, the network chip's MAC address starts with `08:00:37` (Fuji Xerox).

Check that the printer accepts raw jobs:

```bash
timeout 3 bash -c '</dev/tcp/192.168.0.201/9100' && echo "printer reachable"
```

The printer can be slow to answer while it's in deep sleep. If the check fails, press a button on the printer and try again.

On a Mac, use `nc -z -G 3 192.168.0.201 9100 && echo "printer reachable"` instead of the `timeout` command.

## macOS

Everything for the Mac is in the [`macos/`](macos) folder. It doesn't need Homebrew or Ghostscript. macOS renders each page with its own built-in PDF renderer, a small converter turns the result into what foo2hbpl1 expects, and foo2hbpl1 sends it to the printer.

**Easiest:** download `dell-1350cnw-macos.zip` from the [latest release](https://github.com/superthom196/dell-1350cnw-mac-linux/releases/latest). It includes prebuilt universal binaries. Unzip it, then in Terminal:

```bash
cd ~/Downloads/dell-1350cnw-macos
sudo sh install.sh 192.168.0.201
```

Replace the IP address with your printer's (see [Find the printer's IP address](#find-the-printers-ip-address)). The printer then appears as **Dell_1350cnw** in System Settings → Printers & Scanners and in every Print dialog. It defaults to A4 colour; change the paper size or switch to Monochrome in the Print dialog.

**From source:** you need the Xcode Command Line Tools (`xcode-select --install`). The installer builds the binaries if `bin/` is missing.

```bash
git clone https://github.com/superthom196/dell-1350cnw-mac-linux.git
cd dell-1350cnw-mac-linux/macos
sudo sh install.sh 192.168.0.201
```

An optional second argument sets the queue name: `sudo sh install.sh 192.168.0.201 Office_Printer`.

To uninstall:

```bash
sudo sh uninstall.sh
```

### What gets installed

Everything goes in `/Library/Printers/foo2hbpl1/`:

- `foo2hbpl1`, the HBPLv1 driver (same source as the Linux package)
- `rastertoppm`, which converts macOS's CUPS raster to the CMYK image foo2hbpl1 reads. It also pads each page to the full paper size, because foo2hbpl1 picks the paper size from the image dimensions
- `hbpl1-filter`, the CUPS filter that connects the two
- `Dell-1350cnw.ppd`, the printer profile

### Why the Mac version does its own colour conversion

foo2hbpl1's built-in RGB-to-CMYK conversion prints pure black as 100% cyan + magenta + yellow + black. The four toner layers never line up perfectly, so text comes out fuzzy. `rastertoppm` does the conversion instead, so black and greys use black toner only. Text is crisp down to 4pt. (On Linux this doesn't arise, because Ghostscript does the CMYK conversion.)

## Linux

Everything for Linux is in the [`linux/`](linux) folder.

### Install on Arch Linux (and Arch-based: Omarchy, EndeavourOS, Manjaro…)

The package is self-contained. It doesn't need `foo2zjs` or `foo2zjs-nightly` from the AUR.

**Easiest (x86_64):** download `foo2hbpl1-r5bf0142-2-x86_64.pkg.tar.zst` from the [latest release](https://github.com/superthom196/dell-1350cnw-mac-linux/releases/latest) and install it:

```bash
sudo pacman -U foo2hbpl1-r5bf0142-2-x86_64.pkg.tar.zst
```

**From source** (any architecture):

```bash
git clone https://github.com/superthom196/dell-1350cnw-mac-linux.git
cd dell-1350cnw-mac-linux/linux
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

### Install on other distributions (Debian, Ubuntu, Fedora…)

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

### Add the printer

#### 1. Create the print queue

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

#### 2. Print a test page

```bash
lp -d Dell_1350cnw /usr/share/cups/data/testprint
```

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| Printer feeds a sheet, then stops; log shows **016-720 PDL Request Data Violation** | Wrong driver (HBPLv2 / foo2hbpl2). Use the foo2hbpl1 profile from this package. |
| Nothing happens at all | Wrong IP, or the printer is asleep. Check with the reachability command above and look at the printer's web page at `http://<ip>/`. Its **Printer Events** page shows error codes. |
| macOS: job stops with "Filter failed" | Check `/var/log/cups/error_log`. If you installed from a downloaded zip and macOS blocked the binaries, run `sudo xattr -dr com.apple.quarantine /Library/Printers/foo2hbpl1`. `install.sh` already does this. |
| macOS: text looks fuzzy | You're probably on an old copy that used foo2hbpl1's own colour conversion. Reinstall from this repo. |
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
- **Markus Kuhn** wrote [JBIG-KIT](https://www.cl.cam.ac.uk/~mgk25/jbigkit/); `macos/src/jbig/` is a copy of its library.
- The macOS printer profile is adapted from [macos2/HBPLv1.0-Printer-driver](https://github.com/macos2/HBPLv1.0-Printer-driver).
- [Dechifro's HBPL page](http://www.dechifro.org/hbpl/) has background on the printer language.

## License

This repository's packaging files and macOS code are licensed under GPL-2.0-or-later, the same as the foo2zjs driver they build. See [LICENSE](LICENSE).
