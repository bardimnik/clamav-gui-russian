#!/usr/bin/env bash
# Build a .pkg.tar.zst package for Arch Linux from local source using makepkg
# Usage: ./arch/build-local.sh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
PKGNAME="clamav-gui"
PKGVER="1.4.6"
PKGREL="1"
OUTDIR="$SCRIPT_DIR"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT

echo "==> Building $PKGNAME-$PKGVER-$PKGREL-x86_64.pkg.tar.zst"
echo "    Source:  $SRC_DIR"
echo "    Output:  $OUTDIR"
echo ""

# Create a PKGBUILD that references the local source directory
cat > "$WORKDIR/PKGBUILD" <<PKGBUILD
pkgname=${PKGNAME}
pkgver=${PKGVER}
pkgrel=${PKGREL}
pkgdesc="Graphical Interface for ClamAV Antivirus"
arch=(x86_64)
url="https://github.com/wusel1007/clamav-gui"
license=(GPL-3.0-or-later)
depends=(clamav qt6-base)
makedepends=(cmake)
source=()
sha256sums=()

build() {
    cmake -S "${SRC_DIR}" \\
        -B "\${srcdir}/build" \\
        -DCMAKE_BUILD_TYPE=Release \\
        -DCMAKE_INSTALL_PREFIX=/ \\
        -DBUILD_TESTING=OFF

    cmake --build "\${srcdir}/build" -j\$(nproc)
}

package() {
    cmake --install "\${srcdir}/build" --prefix "\${pkgdir}"
}
PKGBUILD

# Run makepkg (skip checksums since source is empty/local)
echo "==> Running makepkg..."
cd "$WORKDIR"
makepkg --skippgpcheck --nocolor 2>&1 | grep -E "^==>|error|Error" || true

# Find the generated package (main, not debug) and move it to output dir
PKGFILE="$WORKDIR/${PKGNAME}-${PKGVER}-${PKGREL}-x86_64.pkg.tar.zst"
if [[ -f "$PKGFILE" ]]; then
    cp "$PKGFILE" "$OUTDIR/"
    echo ""
    echo "==> Package created:"
    ls -lh "$OUTDIR/$(basename "$PKGFILE")"
    echo ""
    echo "==> Verify with:"
    echo "    pacman -Qip $OUTDIR/$(basename "$PKGFILE")"
    echo "==> Install with:"
    echo "    sudo pacman -U $OUTDIR/$(basename "$PKGFILE")"
else
    echo "ERROR: Package file not found: $PKGFILE"
    ls -la "$WORKDIR"
    exit 1
fi
