/* QGLIV.h v0.0 - This file is NOT part of KDE ;)
    Copyright (C) 2006-2009 Thomas Lübking <thomas.luebking@web.de>

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

#ifndef DIR_FILE_COMBO
#define DIR_FILE_COMBO

#include <QComboBox>
#include <QDir>

class DirFileCombo : public QComboBox
{
    Q_OBJECT
public:
    DirFileCombo( QWidget * parent = 0, QDir::Filter type = QDir::Dirs, const QString& startDir = QString() );
    inline const QDir &currentDir() const { return iCurrentDir; }
    void setEntries( const QStringList &list );
signals:
    void directoryChanged( const QString& ) const;
    void textEntered( const QString& ) const;
public slots:
    void setDirectory( const QString& );
private slots:
    void textEntered() const;
private:
    QDir iCurrentDir;
    QDir::Filter iType;
};

#endif // DIR_FILE_COMBO
