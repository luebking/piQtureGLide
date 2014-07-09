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

#ifndef QGLIV_H
#define QGLIV_H

#include <QWidget>

#include "qglimageviewer.h"

using namespace QGLImageView;

class QComboBox;
class DirFileCombo;
class QDial;
class QMenu;
class QPanGesture;
class QPinchGesture;
class QSlider;
class QStringList;
class QSwipeGesture;
class QTimer;
class QToolBar;
class QToolBox;

class QGLIV;

class Image
{
public:
    Image(QGLImage *image = 0, int idx = -1);
    QGLImage *image;
    QImage *qGlImage;
    int index;
};

class QGLIV : public QWidget
{
    Q_OBJECT
public:
    enum Feature { Cycle = 1, Autosize = 2, Diashow = 4, ShowMessage = 8  };
    enum Effect { Crossfade = 1, FadeOutAndIn,
                  HorizontalRotation, VerticalRotation,
                  HorizontalSlide, VerticalSlide, ZSlide,
                  KnownEffects
                };
    QGLIV(QWidget* parent = 0, const char* name = 0);
    bool filterMatches(int index) const;

    inline bool autoSize()
    {
        return (features & (Autosize | Diashow));
    }
    inline bool cycle()
    {
        return (features & (Cycle | Diashow));
    }
    inline bool showMessage()
    {
        return (features & (ShowMessage | Diashow)) == ShowMessage;
    }

signals:
    void cacheReady(const QString&) const;
    void imageChanged() const;

protected:
    virtual bool eventFilter(QObject * watched, QEvent * e);

protected:
    friend void loadImage(void *qgliv, Image *client);
    bool filterIsDirty;
    QRegExp filter;
    QStringList files;
    int diaTime;

private:
    // UI elements
    struct UI {
        QMenu *rmbPopup;
        DirFileCombo *dirCombo, *fileCombo;
        QToolBar *toolbar, *viewBar;
        QAction *autoSize;
        QToolBox *tools;
        QSlider *sliderRed, *sliderGreen, *sliderBlue, *sliderAlpha, *sliderGamma;
        QDial *rotateZ;
        QSlider *rotateX, *rotateY;
        QSlider *zoomSlider; QTimer *zoomTimer;
        QComboBox *transitions;
        int level;
    } ui;

    QGLImageViewer* view;
    QString editorCmd, wallpaperCmd;
    // gl image benders
    Image *prev, *current, *next;

    int features;
    int transitionEffect;

    // interslot transporters
    QGLImage *_oldImage;
    int _effect, _lastDirection;
    bool _animationDone;
    QTimer *transitionFinisher;
    bool iAmTouchy, iAmMultiTouchy, dont_dragswitch;

private:
    void addToView(Image *image);
    bool cacheIsReady();
    void changeColor(int key, int v);
    bool changeImage(int direction);
    QString currentImageInfo();
    void load(Image *image, bool block = false);
    void unload(Image *image);
    void maxW(int ms); void resetView(int ms);
    void setupUI(const QString &startDir);

    bool pinch(const QPinchGesture *pg, bool newPinch);
    bool pan(const QPanGesture *pg, bool newPan);
    bool swipe(const QSwipeGesture *sg, bool newSwipe);

private slots:
    void clearFilter();
    void setDirectory(const QString &dir);
    void dirLoaded();
    void finishImageChange();
    void imageLoaded();
    void init();
    void setAnimationDone();
    void tryChangeImage();
    bool setFilter(const QString &);
    void jumpTo(const QString &);
    void setCycle(bool);
    void setDiaShow(bool);
    void setDiaShowTime(double);
    void setAutosize(bool);
    void setTransitionEffect(int);
private slots:
    void saveSettings();
    void editImage();
    void setAsWallpaper();
    void begin(); void bwd(); void fwd(); void end();
    void flipLeft(); void flipRight();
    void flipUp(); void flipDown();

    void moveLeft(); void moveRight();
    void moveUp(); void moveDown();

    void zoomIn(); void zoomOut();
    void zoom(int dir); void resetZoomSlider();
    inline void maxW()
    {
        maxW(520);
    };
    inline void resetView()
    {
        resetView(520);
    }

    void r90cw(); void r90ccw();
    void XRotate(int); void YRotate(int); void ZRotate(int);
    void resetRotation();

    void toggleAnimation();
    void toggleFullscreen();
    void toggleUI();
    void toggleMessage();

    void gammaUp(); void gammaDown();

    void setCanvasValue(int);
    void redUp(); void redDown();
    void greenUp(); void greenDown();
    void blueUp(); void blueDown();
    void alphaUp(); void alphaDown();
    void invert(); void resetColors();
    void setRed(int); void setGreen(int);
    void setBlue(int); void setAlpha(int);
    void setGamma(int);

    void blurLess(); void blurMore();
    void tunnelLess(); void tunnelMore();
};


#endif // QGLIV_H
