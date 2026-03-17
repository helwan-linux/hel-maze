# Maintainer: Saeed Badreldin <https://github.com/helwan-linux>
# المستودع: https://github.com/helwan-linux/hel-maze

pkgname=hel-maze
pkgver=1.0
pkgrel=2
pkgdesc="لعبة متاهة تعليمية رسمية لتوزيعة Helwan Linux"
arch=('x86_64')
url="https://github.com/helwan-linux/hel-maze"
license=('GPL')
depends=('gtk3')
makedepends=('gcc' 'pkg-config')
source=("git+https://github.com/helwan-linux/hel-maze.git")
sha256sums=('SKIP')

build() {
  cd "$srcdir/hel-maze/maze"
  # بناء البرنامج مباشرة
  gcc main.c -o hel-maze $(pkg-config --cflags --libs gtk+-3.0)
}

package() {
  cd "$srcdir/hel-maze/maze"

  # 1. تثبيت الملف التنفيذي
  install -Dm755 hel-maze "$pkgdir/usr/bin/hel-maze"

  # 2. تثبيت الأيقونة
  install -Dm644 helwan-maze.png "$pkgdir/usr/share/icons/hicolor/48x48/apps/helwan-maze.png"

  # 3. تثبيت ملف الـ Desktop
  install -Dm644 helwan-maze.desktop "$pkgdir/usr/share/applications/helwan-maze.desktop"
}

# --- الجزء السحري: تحديث النظام تلقائياً بعد التثبيت ---
# الأسطر دي بتخلي pacman يحدّث القوائم والأيقونات فوراً بمجرد انتهاء التثبيت
post_install() {
  update-desktop-database -q
  gtk-update-icon-cache -q -t -f usr/share/icons/hicolor
}

post_upgrade() {
  post_install
}
