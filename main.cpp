/* This file is NOT part of KDE ;)
   Copyright (C) 2006 Thomas Lübking <thomas.luebking@web.de>

   This application is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QApplication>
#include <QSettings>

#include "qgliv.h"
// #include <qgl.h>

int main( int argc, char **argv )
{
//     setenv("__GL_SYNC_TO_VBLANK", "1", 1);

    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication a(argc,argv);

    if ( !QGLFormat::hasOpenGL() )
    {
        qWarning( "This system has no OpenGL support. Exiting." );
        return -1;
    }

    QSettings settings("piQtureGLide");
    QGLFormat f;
    f.setDirectRendering( settings.value("GLDirect", true).toBool() );
    f.setSwapInterval( settings.value("GLSync", 1).toUInt() );
    QGLFormat::setDefaultFormat(f);
   
    QGLIV* gliv = new QGLIV;
    gliv->resize( 1000, 620 );
    gliv->show();

    int result = a.exec();
    delete gliv;
    return result;
}
