# Maintainer: Thom Powell <thom@fastmail.org>
#
# HBPLv1 printer driver for the Dell 1350cnw and its siblings:
# Xerox Phaser 6000/6010, Dell 1250c / C1660w / C1760nw, Fuji Xerox DocuPrint CP105b, Epson AcuLaser C1700.
# Built from the mikerr/foo2zjs fork (Dave Coffin's foo2hbpl1), pinned to a known-good commit.
# Self-contained: ships a private copy of foo2zjs-pstops, so foo2zjs itself is not needed.

pkgname=foo2hbpl1
pkgver=r5bf0142
pkgrel=2
pkgdesc="CUPS driver for HBPLv1 colour lasers (Dell 1350cnw/1250c/C1660w/C1760nw, Xerox Phaser 6000/6010, Fuji Xerox CP105b)"
arch=('x86_64' 'aarch64' 'armv7h' 'i686')
url="https://github.com/mikerr/foo2zjs"
license=('GPL-2.0-or-later')
depends=('ghostscript' 'cups' 'cups-filters' 'bc')
makedepends=('git' 'gcc' 'make')
options=('!debug')
_commit=5bf0142d1e3d4363684608ac42933510d3b66e27
source=("git+https://github.com/mikerr/foo2zjs.git#commit=${_commit}")
sha256sums=('SKIP')

build() {
  cd foo2zjs
  rm -f foo2hbpl1 foo2hbpl1.o foo2hbpl1-wrapper
  make foo2hbpl1 foo2hbpl1-wrapper
  # Call the bundled pstops helper instead of one from a foo2zjs install.
  sed -i -E 's@(^|[[:space:];(])foo2zjs-pstops @\1/usr/lib/foo2hbpl1/foo2zjs-pstops @g' foo2hbpl1-wrapper
}

package() {
  cd foo2zjs
  install -Dm755 foo2hbpl1 "$pkgdir/usr/bin/foo2hbpl1"
  install -Dm755 foo2hbpl1-wrapper "$pkgdir/usr/bin/foo2hbpl1-wrapper"
  install -Dm755 foo2zjs-pstops.sh "$pkgdir/usr/lib/foo2hbpl1/foo2zjs-pstops"
  install -Dm644 foo2hbpl1.1 "$pkgdir/usr/share/man/man1/foo2hbpl1.1"
  install -Dm644 foo2hbpl1-wrapper.1 "$pkgdir/usr/share/man/man1/foo2hbpl1-wrapper.1"
  for ppd in Dell-1250c Dell-C1660 Dell-C1760 Fuji_Xerox-DocuPrint_CP105; do
    install -Dm644 "PPD/$ppd.ppd" "$pkgdir/usr/share/ppd/foo2hbpl1/$ppd-foo2hbpl1.ppd"
  done
  install -Dm644 COPYING "$pkgdir/usr/share/licenses/$pkgname/COPYING"
}
