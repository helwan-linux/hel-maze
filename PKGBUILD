# Maintainer: Saeed Badreldin <https://github.com/helwan-linux>
# Project: Helwan Linux Maze (SMA)

pkgname=hel-maze
pkgver=1.0
pkgrel=1
pkgdesc="A professional educational maze game for Helwan Linux students"
arch=('x86_64')
url="https://github.com/helwan-linux/hel-maze"
license=('GPL')
depends=('gtk3')
makedepends=('gcc' 'pkg-config')
source=("git+https://github.com/helwan-linux/hel-maze.git")
sha256sums=('SKIP')

build() {
  cd "$srcdir/hel-maze/maze"
  # بناء اللعبة باستخدام المترجم وربط مكتبات GTK3
  gcc main.c -o hel-maze $(pkg-config --cflags --libs gtk+-3.0)
}

package() {
  cd "$srcdir/hel-maze/maze"

  # 1. تثبيت الملف التنفيذي في المجلد الرئيسي للبرامج
  install -Dm755 hel-maze "$pkgdir/usr/bin/hel-maze"

  # 2. تثبيت الأيقونة في مسار أيقونات النظام (48x48)
  install -Dm644 helwan-maze.png "$pkgdir/usr/share/icons/hicolor/48x48/apps/helwan-maze.png"

  # 3. تثبيت ملف الـ Desktop ليظهر في قائمة التطبيقات
  install -Dm644 helwan-maze.desktop "$pkgdir/usr/share/applications/helwan-maze.desktop"
}
