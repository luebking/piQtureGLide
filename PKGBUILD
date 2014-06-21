#Contributor: <thomas.luebking@gmail.com>

pkgname=piQtureGLide
_realname=piQtureGLide
pkgver=0.9
_upstream_ver=0.9
pkgrel=1
pkgdesc="OpenGL Image viewer for HUUGE images"
arch=('i686' 'x86_64')
url="http://qt-apps.org/content/show.php/piQtureGLide?content=100609"
depends=('qt>=4.6.0')
makedepends=('gcc')
license=('GPL')

build()
{
    cd ..
    for name in *; do if [ ! -d "$name" ]; then ln -f "$name" "src/$name"; fi; done
    cd src
    qmake piQtureGLide.pro
	sed -ie 's/update-desktop-database//g' Makefile
    make || return 1
    make INSTALL_ROOT=$pkgdir install
}

