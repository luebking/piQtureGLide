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

// #include <QtGui>
// #include <QtOpenGL>
#include <QHBoxLayout>
#include <QApplication>
#include <QInputEvent>
#include <QMessageBox>
#include <QCursor>
#include <QStringList>
#include <QTimer>
// #include <QTime>
#include <QTextStream>
#include <QFile>


#include "kgldiashow.h"

// QTime timer;

#ifndef CLAMP
#define CLAMP(x,l,u) (x) < (l) ? (l) :\
(x) > (u) ? (u) :\
(x)
#endif

#ifndef QMAX
#define QMAX(x,y) (x) > (y) ? (x) : (y)
#endif

#ifndef QMIN
#define QMIN(x,y) (x) < (y) ? (x) : (y)
#endif

#define ASCII(_str_) _str_.toLatin1().data()

#if 0
//! libkscreensaver interface
extern "C"
{
   const char *kss_applicationName = "kgldiashow.kss";
   const char *kss_description = "KGLDiashow";
   const char *kss_version = "0.9.8";
   
   KGLDiaShow *kss_create( WId id )
   {
      return new KGLDiaShow( "test", id );
   }
   
   QDialog *kss_setup()
   {
      return new QDialog();
   }
}
#endif

KGLDiaShow::KGLDiaShow( QWidget* parent, const char* name )
: QWidget( parent )
{
   setAccessibleName( name );
// Create an OpenGL widget
   view = new QGLImageView::QGLImageViewer( this, "glbox1", 25, false);
//    
   QPalette pal = palette();
   pal.setColor(QPalette::Background, Qt::black);
   setPalette(pal);
   // Put the GL widget inside the frame
   QHBoxLayout* flayout1 = new QHBoxLayout( this );
   flayout1->setMargin(0);
   flayout1->setSpacing(0);
   flayout1->addWidget( view );
   
   Type oneInt[1] = {Int};
   Type oneUInt[1] = {UInt};
   Type oneFloat[1] = {Float};
   Type stringUInt[2] = {String, UInt};
   Type uintString[2] = {UInt, String};
   Type oneString[1] = {String};
   Type stringString[2] = {String,String};
   Type axFlInt[3] = {Axis, Float, Int};
   Type uintAxFlInt[4] = {UInt, Axis, Float, Int};
   Type flFlInt[3] = {Float, Float, Int};
   Type uintFlFlInt[4] = {UInt, Float, Float, Int};
   Type uintFlInt[3] = {UInt, Float, Int};
   Type intIntInt[3] = {Int, Int, Int};
   Type uintIntIntIntInt[5] = {UInt, Int, Int, Int, Int};
   
   functionList["wait"] = Function(wait, 1, oneInt);
   functionList["quit"] = Function(quit, 0);
   functionList["debug"] = Function(_debug_, 1, oneInt);
   functionList["requireVersion"] = Function(requireVersion, 1, oneFloat);
//    functionList["assign"] = Function(assign, 2, stringUInt, 1);
   functionList["loop"] = Function(loop, 1, oneUInt, 0);
   functionList["endloop"] = Function(endloop, 0);
   
   functionList["for"] = Function(_for, 1, oneString, 1);
   functionList["endfor"] = Function(endfor, 0);
   
   functionList["showImage"] = Function(showImage, 1, oneUInt);
   functionList["hideImage"] = Function(hideImage, 1, oneUInt);
   functionList["loadImage"] = Function(loadImage, 2, stringString, 1);
   functionList["copyImage"] = Function(copyImage, 2, uintString, 2);
   functionList["removeImage"] = Function(removeImage, 1, oneUInt);
   
   functionList["tintImage"] = Function(tintImage, 5, uintIntIntIntInt, 4);
   functionList["setImageBrightness"] = Function(setImageBrightness, 3, uintFlInt, 2);
   functionList["clipImage"] =  Function(clipImage, 5, uintIntIntIntInt);
   functionList["blurImage"] =  Function(blurImage, 3, uintFlInt, 2);
   
   functionList["rotateBy"] = Function(rotateBy, 3, axFlInt, 2);
   functionList["rotateTo"] = Function(rotateTo, 3, axFlInt, 2);
   functionList["rotateImageBy"] = Function(rotateImageBy, 4, uintAxFlInt, 3);
   functionList["rotateImageTo"] = Function(rotateImageTo, 4, uintAxFlInt, 3);
   
   functionList["moveBy"] = Function(moveBy, 3, axFlInt, 2);
   functionList["moveTo"] = Function(moveTo, 3, axFlInt, 2);
   functionList["moveImageBy"] = Function(moveImageBy, 4, uintAxFlInt, 3);
   functionList["moveImageTo"] = Function(moveImageTo, 4, uintAxFlInt, 3);
   
   functionList["scaleBy"] = Function(scaleBy, 3, flFlInt, 1);
   functionList["scaleTo"] = Function(scaleTo, 3, flFlInt, 1);
   functionList["scaleImageBy"] = Function(scaleImageBy, 4, uintFlFlInt, 2);
   functionList["scaleImageTo"] = Function(scaleImageTo, 4, uintFlFlInt, 2);
   
   functionList["setImageAlpha"] = Function(setImageAlpha, 3, uintFlInt, 2);
   functionList["setCanvasColor"] = Function(setCanvasColor, 3, intIntInt);
   currentLine = 0;
   _debug = false;
}

void KGLDiaShow::run(QString file)
{
   setCursor(Qt::BlankCursor);
   // open the script
   QFile data(file);
   if (data.open(QFile::ReadOnly))
   {
      QTextStream script( &data );
      currentLine = 0;
      QStringList lineCommands;
      Command cmd;
      while (!script.atEnd())
      {
         ++currentLine; // we'll use that for the lines for the moment
         // "1024" limit the script line length to avoid high mem load by mal scripts
         // can be left or increased in case... and is disabled for qt3 support ;)
         lineCommands = script.readLine(1024).section('#',0,0).simplified().remove(' ').split(';', QString::SkipEmptyParts);
         for (uint i = 0; i < lineCommands.count(); ++i)
         {
            cmd = command(lineCommands[i]);
            if (cmd.function != INVALID)
               commands += cmd;
         }
      }
      commandCounter = -1;
      nextCommand();
   }
}

Command KGLDiaShow::command(QString & string)
{
   QString function = string.section('(',0,0);
   
   FunctionList::const_iterator funcIt = functionList.find(function);
   if (funcIt == functionList.end())
   {
      qWarning("ERROR #%d: Unknown function \"%s\"", currentLine, ASCII(function));
      return Command(currentLine, INVALID);
   }
   
   Function func = funcIt.value();
   
   QStringList parameter; bool noParameters = false;
   parameter = string.section('(',1).remove(')').remove(';').split(',');
   if (parameter.count() == 1 && parameter[0] == "") noParameters = true;
   
   if ((noParameters && func.min > 0) || (parameter.count() < func.min))
   {
      QString parameters;
      
      qWarning("ERROR #%d: You need to pass at least the first %d parameter(s) to function\n%s ( %s )", currentLine, func.min, ASCII(function), "parameters?!");
      return Command(currentLine, INVALID);
   }
   if (!noParameters && parameter.count() > func.max)
   {
      qWarning("WARNING #%d: Function %s ( %s ) takes only %d parameter(s), but you sent %d\n%s ( %s )", currentLine,
            ASCII(function), "parameters?!",
            func.max, parameter.count(),
            ASCII(function), ASCII(parameter.join(", ")));
   }
   
   if (noParameters)
      return Command(currentLine, func.id);
   
   int numP = QMIN(parameter.count(), func.max);
   void *pList[numP];
   
#define VALUEERROR(_tp_) if (!ok)\
   {\
      qWarning("ERROR #%d: Parameter %d of \"%s\" must be of type \"%s\"", currentLine, i+1, functionString(func.id), typeString(func.type[i]));\
      return Command(currentLine, INVALID);\
   }
   
   bool *vars = 0L;
   
   for (int i = 0; i < numP; ++i)
   {
      bool ok = false;
      switch (func.type[i])
      {
      case Int:
      {
         int v = parameter[i].toInt(&ok);
         VALUEERROR(int);
         pList[i] = new int(v);
         break;
      }
      case UInt:
      {
         uint v = parameter[i].toUInt(&ok);
         if (!ok && parameter[i][0] == '$') // this might be a variable
         {
            if (!vars)
            {
               vars = new bool[numP];
               for (int j = 0; j < numP; ++j)
                  vars[j] = false;
            }
            vars[i] = true;
            pList[i] = new QString(parameter[i].right(parameter[i].length()-1));
            break;
         }
         VALUEERROR(uint);
         pList[i] = new uint(v);
         break;
      }
      case Axis:
      {
         QGLImageView::Axis v = axis(parameter[i], (&ok));
         VALUEERROR(Axis);
         pList[i] = new QGLImageView::Axis(v);
         break;
      }
      case Float:
      {
         float v = parameter[i].toFloat(&ok);
         VALUEERROR(float);
         pList[i] = new float(v);
         break;
      }
      case String:
         pList[i] = new QString(parameter[i]);
      }
   }
   
   return Command(currentLine, func.id, pList, numP, vars);
}

void KGLDiaShow::nextCommand()
{
   ++commandCounter;
   if (commandCounter > commands.size())
      return; // we're done
   exec(commands[commandCounter]);
}

uint KGLDiaShow::variableValue(const QString &string)
{
   VariableList::const_iterator it = variableList.find(string);
   if (it != variableList.end())
      return it.value();
   bool ok;
   uint ret = string.toUInt(&ok); //maybe this is a simple number... ;)
   if (ok) return ret;
      qWarning("ERROR %d: No such variable \"%s\"", currentLine, ASCII(string));
   return 0xFFFFFFFF;
}

#define INT(_p_) (*(int*)command.parameter[_p_])
#define UINT(_p_) (command.isVariable && command.isVariable[_p_]) ?\
                  variableValue(*(QString*)command.parameter[_p_]) :\
                  (*(uint*)command.parameter[_p_])
#define FLOAT(_p_) (*(float*)command.parameter[_p_])
#define AXIS(_p_) (*(QGLImageView::Axis*)command.parameter[_p_])
#define STRING(_p_) (*(QString*)command.parameter[_p_])

void KGLDiaShow::exec(const Command &command)
{
   if (_debug)
      qWarning("#%u: command \"%s\"", command.line, functionString(command.function));
   currentLine = command.line;
   switch (command.function)
   {
   case wait:
      QTimer::singleShot ( INT(0), this, SLOT(nextCommand()) );
      return;
   case quit:
      qApp->quit();
      return;
   case requireVersion:
      if ((float)VERSION < FLOAT(0))
      {
         QMessageBox::critical ( 0L, "KGLDiaShow: versionconflict",
                                 QString("<qt><center>Sorry, the script %1 requires<br>\
                                         KGLDiaShow <b>v%2</b>,<br>\
                                         but this is<br>\
                                         KGLDiaShow <b>v%3</center></qt> ").arg(qApp->argv()[1]).arg(FLOAT(0)).arg(VERSION),
                                 QMessageBox::Abort, 0);
         qApp->quit();
      }
      break;
   case _debug_:
      _debug = (INT(0));
      break;
//    case assign:
//       if (command.numParameters == 1)
//          variableList[STRING(0)] = lastLoadedImage;
//       else
//          variableList[STRING(0)] = UINT(2);
//       break;
   case loop:
      if (INT(0) < 1)
      {
         while (commands[commandCounter].function != endloop)
            ++commandCounter;
         break;
      }
      loopBegin.push(commandCounter);
      if (command.numParameters == 1)
         loopCounter.push(INT(0)-1);
      else
         loopCounter.push(-1);
      break;
   case endloop:
   {
      if (loopCounter.isEmpty() || loopBegin.isEmpty()) // invalid loopend without a begin
      {
         qWarning("WARNING %d: \"endloop\" without matchig \"loop\", ignoring", command.line);
         break;
      }
      int c = loopCounter.pop();
      if (c == 0)
      {
         loopBegin.pop();
         break; // just go on ;)
      }
      if (c < 0) // infinite
         loopCounter.push(-1);
      else // just count down
         loopCounter.push(c-1);
      commandCounter = loopBegin.top();
      break;
   }
   case _for:
   {
      if (!STRING(0).contains('='))
      {
         qWarning("ERROR %d: \"for\" needs an assignment", command.line);
         break;
      }
      QString easyString = STRING(0).simplified().remove(' ');
      QString var = easyString.section('=',0,0);
      forList.push(easyString);
      QString item = easyString.section('=',1,1).section(':', 0,0);
      loopBegin.push(commandCounter);
      variableList[var] = variableValue(item[0] == '$' ? item.right(item.length()-1) : item);
      loopCounter.push(0);
      break;
   }
   case endfor:
   {
      if (loopCounter.isEmpty() || forList.isEmpty()) // invalid loopend without a begin
      {
         qWarning("WARNING %d: \"endfor\" without matchig \"for\", ignoring", command.line);
         break;
      }
      QStringList items = forList.top().section('=',1,1).split(':', QString::SkipEmptyParts);
      uint c = loopCounter.pop()+1;
      if (c < items.count()) // still sth. to do
      {
         loopCounter.push(c);
         variableList[forList.top().section('=',0,0)]
            = variableValue(items[c][0] == '$' ? items[c].right(items[c].length()-1) : items[c]);
         commandCounter = loopBegin.top();
      }
      else // just go on
      {
         forList.pop();
         loopBegin.pop();
      }
      break;
   }
   case setCanvasColor:
      view->setCanvas(QColor(INT(0),INT(1),INT(2)));
      break;
   case loadImage:
      variableList[STRING(1)] = view->load(STRING(0));
      break;
   case copyImage:
   {
      QGLImage *img = image(UINT(0));
      if (img)
         variableList[STRING(1)] = view->load(*img);
      else
         qWarning("WARNING %d: cannot copy nonexistent image", command.line);
      break;
   }
   case showImage:
   {
      QGLImage *img = image(UINT(0));
      if (img) img->show();
      break;
   }
   case hideImage:
   {
      QGLImage *img = image(UINT(0));
      if (img) img->hide();
      break;
   }
   case removeImage:
   {
      view->remove(UINT(0));
      break;
   }
   case tintImage:
   {
      QGLImage *img = image(UINT(0));
      if (!img) break;
      if (command.numParameters == 5)
         img->tint(QColor(INT(1),INT(2),INT(3)), INT(4));
      else
         img->tint(QColor(INT(1),INT(2),INT(3)));
      break;
   }
   case setImageBrightness:
   {
      QGLImage *img = image(UINT(0));
      if (!img) break;
      if (command.numParameters == 3)
         img->setBrightness(FLOAT(1), INT(2));
      else
         img->setBrightness(FLOAT(1));
      break;
   }
   case blurImage:
      {
         QGLImage *img = image(UINT(0));
         if (!img) break;
         if (command.numParameters == 3)
            img->blur(FLOAT(1), INT(2));
         else
            img->blur(FLOAT(1));
         break;
      }
   case clipImage:
   {
      QGLImage *img = image(UINT(0));
      if (img) img->setClipRect(INT(1), INT(2), INT(3), INT(4));
      break;
   }
   case rotateBy:
   {
      if (command.numParameters == 3)
         view->rotate(AXIS(0), FLOAT(1), INT(2));
      else
         view->rotate(AXIS(0), FLOAT(1));
      break;
   }
   case rotateTo:
   {
      if (command.numParameters == 3)
         view->rotateTo(AXIS(0), FLOAT(1), INT(2));
      else
         view->rotateTo(AXIS(0), FLOAT(1));
      break;
   }
   case rotateImageTo:
   {
      QGLImage *img = image(UINT(0));
      if (!img) break;
      if (command.numParameters == 4)
         view->rotateTo(AXIS(1), FLOAT(2), INT(3));
      else
         view->rotateTo(AXIS(1), FLOAT(2));
      break;
   }
   case rotateImageBy:
   {
      QGLImage *img = image(UINT(0));
      if (!img) break;
      if (command.numParameters == 4)
         view->rotate(AXIS(1), FLOAT(2), INT(3));
      else
         view->rotate(AXIS(1), FLOAT(2));
      break;
   }
   case moveBy:
   {
      if (view->scaleFactor(AXIS(0)) == 0.0) // we've trouble with our moving model
         return;
      if (command.numParameters == 3)
         view->move(AXIS(0), FLOAT(1)/view->scaleFactor(AXIS(0)), INT(2));
      else
         view->move(AXIS(0), FLOAT(1)/view->scaleFactor(AXIS(0)));
      break;
   }
   case moveTo:
   {
      if (view->scaleFactor(AXIS(0)) == 0.0) // we've trouble with our moving model
         return;
      if (command.numParameters == 3)
         view->moveTo(AXIS(0), FLOAT(1)/view->scaleFactor(AXIS(0)), INT(2));
      else
         view->moveTo(AXIS(0), FLOAT(1)/view->scaleFactor(AXIS(0)));
      break;
   }
   case moveImageBy:
   {
      QGLImage *img = image(UINT(0));
      if (!img) break;
      if (command.numParameters == 4)
         img->move(AXIS(1), FLOAT(2), INT(3));
      else
         img->move(AXIS(1), FLOAT(2));
      break;
   }
   case moveImageTo:
   {
      QGLImage *img = image(UINT(0));
      if (!img) break;
      if (command.numParameters == 4)
         img->moveTo(AXIS(1), FLOAT(2), INT(3));
      else
         img->moveTo(AXIS(1), FLOAT(2));
      break;
   }
   case scaleBy:
   case scaleTo:
   {
      float sx = FLOAT(0);
      float sy = -1.0;
      int msecs = 0;
      if (command.numParameters > 1 )
      {
         sy = FLOAT(1);
         if (sy < 0.0) sy = sx;
         if (command.numParameters > 2 )
            msecs = INT(2);
      }
      else
         sy = sx;
      if (command.function == scaleTo)
      {
         view->scaleTo(X, sx, msecs);
         view->scaleTo(Y, sy, msecs);
      }
      else
      {
         view->scale(X, sx, msecs);
         view->scale(Y, sy, msecs);
      }
      break;
   }
   case scaleImageBy:
   case scaleImageTo:
   {
      QGLImage *img = image(UINT(0));
      if (!img) break;
      float sx = FLOAT(1);
      float sy = -1.0;
      int msecs = 0;
      if (command.numParameters > 2 )
      {
         sy = FLOAT(2);
         if (command.numParameters > 3 )
            msecs = INT(3);
      }
      else
         sy = sx;
      if (command.function == scaleImageTo)
      {
         if (sy < 0.0)
            img->scaleTo(Y, img->scaleTo(X, sx, msecs, true, 100), msecs);
         else
         {
            img->scaleTo(X, sx, msecs, true, 100);
            img->scaleTo(Y, sy, msecs, true, 100);
         }
      }
      else
      {
         img->scale(X, sx, msecs);
         img->scale(Y, (sy < 0.0) ? sx : sy, msecs);
      }
      break;
   }
   case setImageAlpha:
   {
      QGLImage *img = image(UINT(0));
      if (!img) break;
      if (command.numParameters > 2 )
         img->setAlpha(FLOAT(1), INT(2));
      else
         img->setAlpha(FLOAT(1));
      break;
   }
   default:
      qWarning("ERROR %d: cannot execute invalid function", command.line);
   }
   nextCommand();
//    qWarning("%s ( %s )", function.toLatin1().data(), parameter ? parameter->join(", ").toLatin1().data() : "");
}

const char *KGLDiaShow::functionString(FuncId id) const
{
   switch (id)
   {
   case wait: return "wait";
   case quit: return "quit";
   case requireVersion: return "requireVersion";
   case _debug_: return "debug";
//    case assign: return "assign";
   case loop: return "loop";
   case endloop: return "endloop";
   case _for: return "for";
   case endfor: return "endfor";
   case setCanvasColor: return "setCanvasColor";
   case loadImage: return "loadImage";
   case copyImage: return "copyImage";
   case showImage: return "showImage";
   case hideImage: return "hideImage";
   case removeImage: return "removeImage";
   case tintImage: return "tintImage";
   case setImageBrightness: return "setImageBrightness";
   case blurImage: return "blurImage";
   case clipImage: return "clipImage";
   case rotateBy: return "rotateBy";
   case rotateImageBy: return "rotateImageBy";
   case rotateTo: return "rotateTo";
   case rotateImageTo: return "rotateImageTo";
   case moveBy: return "moveBy";
   case moveImageBy: return "moveImageBy";
   case moveTo: return "moveTo";
   case moveImageTo: return "moveImageTo";
   case scaleBy: return "scaleBy";
   case scaleImageBy: return "scaleImageBy";
   case scaleTo: return "scaleTo";
   case scaleImageTo: return "scaleImageTo";
   case setImageAlpha: return "setImageAlpha";
   default: return "INVALID";
   }
}

const char *KGLDiaShow::typeString(Type tp) const
{
   switch (tp)
   {
   case Int: return "int";
   case UInt: return "uint";
   case Axis: return "Axis";
   case Float: return "float";
   case String: return "String";
   default:
      return "INVALID TYPE";
   }
}


QGLImage *KGLDiaShow::image(uint image) const
{
   for (int i = 0; i < view->images().size(); ++i)
   {
      if (view->images()[i].id() == image)
         return &view->images()[i];
   }
   qWarning("WARNING #%d: No image with id %u found", currentLine, image);
   return 0L;
}

QGLImageView::Axis KGLDiaShow::axis(const QString &string, bool *ok) const
{
   if (ok) *ok = true;
   if (string == "X") return QGLImageView::X;
   if (string == "Y") return QGLImageView::Y;
   if (string == "Z") return QGLImageView::Z;
   if (ok) *ok = false;
   qWarning("ERROR #%d: Axis must be in (X,Y,Z)", currentLine);
   return QGLImageView::Z;
}
