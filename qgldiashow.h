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

#ifndef QGLIV_H
#define QGLIV_H

#include <QWidget>
#include <QStack>

#include "qglimageviewer.h"

class QStringList;
// template<class T> class QStack;

using namespace QGLImageView;

enum FuncId
{
      wait = 0,/* assign,*/
      loop, endloop, _for, endfor,
      setCanvasColor, tintImage, setImageBrightness,
      clipImage, blurImage,
      loadImage, removeImage, copyImage,
      showImage, hideImage,
      rotateBy, rotateImageBy,
      rotateTo, rotateImageTo,
      moveBy, moveImageBy,
      moveTo, moveImageTo,
      scaleBy, scaleImageBy,
      scaleTo, scaleImageTo,
      setImageAlpha,
      quit, requireVersion, _debug_,
      INVALID
};

class Command
{
public:
   Command(){}
   Command(int _line, FuncId _function, void **_parameter = 0L, int _numParameters = 0, bool *varlist = 0L)
   {
      line = _line;
      function = _function;
      numParameters = _numParameters;
      if (numParameters)
      {
         parameter = new void*[numParameters];
         for (int i = 0; i < numParameters; ++i)
            parameter[i] = _parameter[i];
      }
      else
         parameter = 0L;
      isVariable = varlist;
   }
   ~Command()
   {
//       if (isVariable) delete isVariable;
//       if (parameter)
//       {
//          for (int i = 0; i < numParameters; ++i)
//             delete parameter[i];
//          delete parameter;
//       }
   }
   FuncId function;
   void **parameter;
   bool *isVariable;
   int numParameters, line;
};

class KGLDiaShow : public QWidget
{
   Q_OBJECT
public:
   KGLDiaShow( QWidget* parent = 0, const char* name = 0 );
   void run(QString file);
   
private:
   enum Type { Int = 0, UInt, Axis, Float, String };
   class Function
   {
   public:
      Function(){}
      Function(FuncId _id, uint _max, Type *_type = 0L, int _min = -1)
      {
         id = _id;
         max = _max;
         min = (_min > -1) ? _min : max;
         if (max)
         {
            type = new Type[max];
            for (uint i = 0; i < max; ++i)
               type[i] = _type[i];
         }
         else
            type = 0L;
      }
      ~Function()
      {
//          if (type) delete type;
      }
      FuncId id;
      int min;
      uint max;
      Type *type;
   };
   QGLImageViewer* view;
   typedef QMap<QString, uint> VariableList;
   VariableList variableList;
   typedef QMap<QString, Function> FunctionList;
   FunctionList functionList;
   typedef QList<Command> CommandList;
   CommandList commands;

   int commandCounter;
   int currentLine;
   uint lastLoadedImage;
   bool _debug;
   
private:
   Command command(QString & string);
   void exec(const Command &command);
   QGLImageView::Axis axis(const QString &string, bool *ok) const;
   QGLImage *image(uint image) const;
   uint variableValue(const QString &string);
   const char *functionString(FuncId id) const;
   const char *typeString(Type tp) const;

   QStack<int> loopBegin;
   QStack<int> loopCounter;
   QStack<QString> forList;

private slots:
   void nextCommand();
};


#endif // QGLIV_H
