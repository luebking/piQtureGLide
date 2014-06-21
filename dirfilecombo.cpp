/*  This file is NOT part of KDE ;)
 *  Copyright (C) 2006-2014 Thomas Lübking <thomas.luebking@gmail.com>
 *
 *  This application is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or ( at your option ) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "dirfilecombo.h"

#include <QCompleter>
#include <QLineEdit>
#include <QtDebug>

DirFileCombo::DirFileCombo( QWidget * parent, QDir::Filter type, const QString& startDir ) :
QComboBox( parent )
{
    if ( type == QDir::Files )
        iType = QDir::Files;
    else
        iType = QDir::Dirs;

    setEditable( true );
    setInsertPolicy( NoInsert );

    if (startDir != "none") {
        if ( !startDir.isEmpty() )
            iCurrentDir = QDir(startDir);

        if ( !iCurrentDir.exists() )
            iCurrentDir = QDir::current();

        iCurrentDir.makeAbsolute();

        QString path = iCurrentDir.path() + QDir::separator();
        iCurrentDir = QDir(" ");
        setDirectory( path );
    }

    if ( type == QDir::Dirs )
        connect ( this, SIGNAL( editTextChanged(const QString &) ), this, SLOT( setDirectory(const QString &) ) );

    connect ( lineEdit(), SIGNAL( returnPressed() ), this, SLOT( textEntered() ) );
}

void
DirFileCombo::setEntries( const QStringList &newList )
{
    if (iType == QDir::Dirs)
        return; // ah-ahh...

    clear();
    QStringList list = newList;
    for ( int i = 0; i < list.count(); ++i )
        list[i] = QFileInfo( list.at(i) ).fileName();
    addItems( list );
    delete completer();
    QCompleter *comp = new QCompleter( list, this );
    comp->setCaseSensitivity( Qt::CaseInsensitive );
    comp->setCompletionMode( QCompleter::InlineCompletion );
    setCompleter(comp);
}

void
DirFileCombo::setDirectory( const QString &path )
{
    QDir::Filters filter = iType | QDir::Readable | QDir::NoDotAndDotDot;
    QStringList list;
    blockSignals(true);

    if (iType == QDir::Dirs)
    {
        QString fullPath = QFileInfo(path).absolutePath();
        if ( !path.endsWith( QDir::separator() ) && fullPath == QFileInfo( iCurrentDir.path() ).absolutePath() )
            { blockSignals(false); return; }

        QDir dir( fullPath );
        if ( dir.exists() )
        {
            if ( dir == iCurrentDir )
                { blockSignals(false); return; }

            clear();
            iCurrentDir = dir;
            list = iCurrentDir.entryList( filter | QDir::Executable, QDir::Name );
            fullPath = iCurrentDir.path() + QDir::separator();
            for ( int i = 0; i < list.count(); ++i )
                list[i].prepend(fullPath);
            list.prepend( iCurrentDir.path() );
            addItems( list );
            blockSignals(false);
            emit currentIndexChanged( fullPath );
            blockSignals(true);
            setEditText ( path );
        }
        else
            setEditText ( iCurrentDir.path() + QDir::separator() );
    }

    else if ( iType == QDir::Files )
    {
        clear();
        iCurrentDir = QDir( path );
        if ( !iCurrentDir.exists() )
        {
            iCurrentDir = QDir(" ");
            blockSignals(false);
            return;
        }
        addItems( iCurrentDir.entryList( filter, QDir::Name ) );
        setEditText ( QString() );
    }
    else // should not happen at all
        { blockSignals(false); return; }

    delete completer();
    QCompleter *comp = new QCompleter( list, this );
    comp->setCaseSensitivity( Qt::CaseInsensitive );
    comp->setCompletionMode( QCompleter::InlineCompletion );
    setCompleter(comp);

    blockSignals(false);
}

void
DirFileCombo::textEntered() const
{
    emit textEntered( lineEdit()->text() );
}

