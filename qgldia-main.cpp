/* This file is NOT part of KDE ;)
   Copyright (C) 2006 Thomas L�bking <thomas.luebking@web.de>

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

#include "kgldiashow.h"
// #include <qgl.h>

int main( int argc, char **argv )
{
   if (argc != 2)
   {
   qWarning( "Usage: %s <diashow>",argv[0] );
      return 1;
   }
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication a(argc,argv);

   if ( !QGLFormat::hasOpenGL() )
   {
      qWarning( "This system has no OpenGL support. Exiting." );
      return -1;
   }
   KGLDiaShow* w = new KGLDiaShow();
   w->showFullScreen();
   w->run(QString(argv[1]));
   int result = a.exec();
   delete w;
   return result;
}
