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

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFrame>
#include <QFuture>
#include <QFutureWatcher>
#include <QGesture>
#include <QHBoxLayout>
#include <QImageReader>
#include <QInputEvent>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QProcess>
#include <QSettings>
#include <QStringList>
#include <QSysInfo>
#if QT_VERSION < 0x050000
#include <QtConcurrentRun>
#else
#include <QtConcurrent>
#endif
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

#include "dirfilecombo.h"

#include "qgliv.h"

#include <QDebug>

#ifndef CLAMP
#define CLAMP( x, l, u ) ( x ) < ( l ) ? ( l ) :\
    ( x ) > ( u ) ? ( u ) :\
    ( x )
#endif

#ifndef QMAX
#define QMAX( x, y ) ( x ) > ( y ) ? ( x ) : ( y )
#endif

/**
Ok, hello and welcome to al little KGlImageView demo app.
This is not a maintaned app but only a short tutorial for using the QGLImageViewer.
Please don't mourn about functional bugs - THIS DEMO IS NOT MAINTAINED
*******************************************************/

static GLhandleARB shader_vert, shader_frag;

static int numPoly = 1;
static bool supportsAnimation = false;

static void
getNumPoly(QString file)
{
    QFile f(file);

    if (f.open(QIODevice::ReadOnly)) {
        QTextStream ts(&f);
        bool found = false;
        QString line;
        int index;
        while (!found) {
            line = ts.readLine().simplified();
            supportsAnimation = supportsAnimation || line.startsWith("uniform const float time;");
            index = line.indexOf("NUMPOLY=");
            if (index > -1) { // hahaa!
                found = true;
                int indr;
                indr = line.indexOf(' ', index);
                if (indr > -1)
                    line = line.mid(index + 8, indr - (index + 8));
                else
                    line = line.right(line.length() - (index + 8));
                int newPoly = line.toInt();
                if (newPoly > numPoly)
                    numPoly = newPoly;
            }
        }
        f.close();
    } else
        qWarning("No such shader file: %s", file.toLatin1().constData());
}

Image::Image(QGLImage *image, int idx)
{
    this->image = image;
    qGlImage = NULL;
    this->index = idx;
}


static QString _firstFile;

#define xstr(s) str(s)
#define str(s) #s

/** Just building the app framework - just some Qt code, nothing GL special... */
QGLIV::QGLIV(QWidget* parent, const char* name) : QWidget(parent)
{
    setAccessibleName(name);
    filter.setPatternSyntax(QRegExp::Wildcard);
    filter.setCaseSensitivity(Qt::CaseInsensitive);

    current = prev = next = NULL;
    features = ShowMessage;
    diaTime = 10000;
    transitionEffect = 0;
    m_wantedDirection = 0;
    _animationDone = true;
    transitionFinisher = new QTimer(this);
    transitionFinisher->setSingleShot(true);
    connect(transitionFinisher, SIGNAL(timeout()), this, SLOT(finishImageChange()));
    m_oldImage = NULL;
    m_animationTime = 300;

    QStringList args = QCoreApplication::arguments();

    QString startDir;
    if (args.count() == 2) { // one paramenter
        QDir dir(args.at(1));
        if (!dir.exists()) {
            QFileInfo fi(args.at(1));
            dir = fi.absoluteDir();
            _firstFile = fi.absoluteFilePath();
        }
        startDir = dir.absolutePath();
    }

    files = QStringList();

    if (_firstFile.isNull()) {
        for (int i = 1; i < args.count(); ++i) {
            QDir dir(args.at(i));
            if (dir.exists()) {
                QStringList fileList = dir.entryList(QDir::Files | QDir::Readable, QDir::Name);
                for (int i = 0; i < fileList.count(); ++i)
                    fileList[i].prepend(dir.absolutePath() + '/');
                for (int i = 0; i < fileList.count(); ++i) {
                    if (QImageReader(fileList.at(i)).canRead())
                        files << fileList.at(i);
                }
            } else {
                for (int i = 0; i < args.count(); ++i) {
                    if (QImageReader(args.at(i)).canRead())
                        files << args.at(i);
                }
            }
        }
    } else
        files << _firstFile;

    //BEGIN  Create & setup the GL viewport                  .
    view = new QGLImageView::QGLImageViewer(this, "glbox1");

    if (view->providesShaders()) {
#if QT_VERSION < 0x050000

#define xstr(s) str(s)
#define str(s) #s
        QString globalDir(xstr(DATADIR)"/piQtureGLide/");
#undef str
#undef xstr
        QString localDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "piQtureGLide/";
        char *env = getenv("QGLIV_SHADER_VERT");
        QString path = env ? env : "shader.vert";
        if (QFile::exists(globalDir + path))
            path = globalDir + path;
        else if (QFile::exists(localDir + path))
            path = localDir + path;
        else {
            path = QString();
#define CHARS(_S_) _S_.toLocal8Bit().data()
            qWarning("No such shader %s in %s or %s", CHARS(path), CHARS(globalDir), CHARS(localDir));
        }
        shader_vert = 0;
        if (!path.isNull()) {
            getNumPoly(path);
            shader_vert = view->loadShader(path, GL_VERTEX_SHADER_ARB);
        }

        env = getenv("QGLIV_SHADER_FRAG");
        path = env ? env : "shader.frag";
        if (QFile::exists(globalDir + path))
            path = globalDir + path;
        else if (QFile::exists(localDir + path))
            path = localDir + path;
        else {
            path = QString();
            qWarning("No such shader %s in %s or %s", CHARS(path), CHARS(globalDir), CHARS(localDir));
#undef CHARS
        }
        shader_frag = path.isNull() ? 0 : view->loadShader(path, GL_FRAGMENT_SHADER_ARB);

#else // QT_VERSION

        char *env = getenv("QGLIV_SHADER_VERT");
        QString path = env ? env : "shader.vert";
        path = QStandardPaths::locate(QStandardPaths::DataLocation, path);
        shader_vert = 0;
        if (!path.isEmpty()) {
            getNumPoly(path);
            shader_vert = view->loadShader(path, GL_VERTEX_SHADER_ARB);
        }

        env = getenv("QGLIV_SHADER_FRAG");
        path = env ? env : "shader.frag";
        path = QStandardPaths::locate(QStandardPaths::DataLocation, path);
        shader_frag = path.isEmpty() ? 0 : view->loadShader(path, GL_FRAGMENT_SHADER_ARB);
#endif
    }

    QFont fnt = view->font(); fnt.setPointSize(20); view->setFont(fnt);
    view->setFocusPolicy(Qt::WheelFocus);
    view->installEventFilter(this);


    //BEGIN  Create a menu                     .
    QMenu *file = new QMenu("&File", this);
    file->addAction("Set as wallpaper",  this, SLOT(setAsWallpaper()), Qt::Key_W);
    file->addAction("Edit in image editor",  this, SLOT(editImage()), Qt::Key_E);
    file->addAction("Exit",  qApp, SLOT(quit()), Qt::CTRL + Qt::Key_Q);

    QMenu *edit = new QMenu("&View", this);
    edit->addAction("Toggle Filename Display", this, SLOT(toggleMessage()), Qt::Key_M);
    //    edit->addAction( "Toggle Freeze", this, SLOT( toggleFreeze() ), Qt::Key_F );
    edit->addAction("Blur more", this, SLOT(blurMore()), Qt::SHIFT + Qt::Key_S);
    edit->addAction("Blur less", this, SLOT(blurLess()), Qt::Key_S);
    edit->addAction("Tunnel more", this, SLOT(tunnelMore()), Qt::SHIFT + Qt::Key_T);
    edit->addAction("Tunnel less", this, SLOT(tunnelLess()), Qt::Key_T);
    if (supportsAnimation)
        edit->addAction("Toggle animation", this, SLOT(toggleAnimation()), Qt::Key_F);

    // Create a rmb menu
    ui.rmbPopup = new QMenu(this);
    ui.rmbPopup->addMenu(file);
    ui.rmbPopup->addMenu(edit);

    //BEGIN apply actions to viewport       .
    view->addActions(file->actions());
    view->addActions(edit->actions());

    setupUI(startDir);

    resetView(0);

    // Put the GL widget inside the frame
    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setMargin(0);
    vl->setSpacing(0);
    vl->addWidget((QWidget*)ui.toolbar);
    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget((QWidget*)ui.tools);
    hl->addWidget(view, 1);
    vl->addLayout(hl, 1);
    vl->addWidget((QWidget*)ui.viewBar);

    QSettings settings("piQtureGLide");
    m_touchMode = settings.value("TouchMode", 0).toInt();
    view->setCursorVisible(!m_touchMode);
    if (m_touchMode) {
        QFont fnt = QApplication::font();
        fnt.setPointSize(2*fnt.pointSize());
        QApplication::setFont(fnt);
    }
    if (m_touchMode > 1) {
        view->setAttribute(Qt::WA_AcceptTouchEvents);
        view->setAttribute(Qt::WA_TouchPadAcceptSingleTouchEvents);
//         view->grabGesture(Qt::TapGesture);
//         view->grabGesture(Qt::TapAndHoldGesture);
        view->grabGesture(Qt::PanGesture);
        view->grabGesture(Qt::PinchGesture);
        view->grabGesture(Qt::SwipeGesture);
    }
    editorCmd = settings.value("EditorCmd", "gimp \"%f\"").toString();
    wallpaperCmd = settings.value("SetWallpaperCmd", "qdbus org.kde.be.shell /Desktop setWallpaper \"%f\"").toString();
    ui.autoSize->setChecked(settings.value("AutoSize", true).toBool());
    setAutosize(ui.autoSize->isChecked());
    ui.transitions->setCurrentIndex(settings.value("Transition", 0).toUInt());
    int uilevel = settings.value("UILevel", 0).toUInt();
    if (m_touchMode && uilevel == 2) // forbidden
        uilevel = 1;
    if (uilevel < 0)
        uilevel = 0;
    if (uilevel > 3)
        uilevel = 3;
    for (int i = 0; i < uilevel; ++i)
        toggleUI();
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(saveSettings()));
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

static bool firstInit = true;

void
QGLIV::init()
{
    unload(prev);
    // prepare the first image
    prev = new Image;

    bool reloadCurrent = true;
    if (firstInit || _firstFile.isNull()) {
        unload(current);
        current = new Image(0, 0);
    } else {
        reloadCurrent = false;
        if (files.count() > 1)
            current->index = files.indexOf(_firstFile);
    }

    unload(next);
    next = new Image;

    if (reloadCurrent) {
        load(current, true);
        if (!current->image) {
            QImage img(640, 128, QImage::Format_ARGB32);
            img.fill(Qt::transparent);
            QPainter p(&img);
            QFont font; font.setPixelSize(24); font.setBold(true);
            font.setPixelSize(24 * 600 / QFontMetrics(font).width("piQtureGLide"));
            p.setFont(font); p.setPen(Qt::white);
            p.drawText(img.rect(), Qt::AlignCenter, "piQtureGLide");
            p.end();
            current->qGlImage = new QImage(QGLWidget::convertToGLFormat(img));
            current->index = 0;
            addToView(current);
            resetView();
            current->image->rotate(QGLImageView::Y, 100 * 360, 100 * 4000);
        } else
            autoSize() ? maxW() : resetView();
        if (showMessage())
            view->message(20, 20, currentImageInfo(), 2500);
    }

    // ...and cache in case
    if (files.count() > 1) {
        next->index = current->index + 1;
        load(next);
        if (features & Diashow) {
            QTimer::singleShot(1000, this, SLOT(maxW()));
            m_wantedDirection = 1;
            QTimer::singleShot(diaTime, this, SLOT(tryChangeImage()));
        } else if (features & Cycle)
            load(prev);
    }
    if (!_firstFile.isNull()) {
        if (firstInit)
            setDirectory(QFileInfo(_firstFile).absoluteDir().absolutePath());
        else
            _firstFile.clear();
    }
    firstInit = false;
}


void QGLIV::setCycle(bool active)
{
    if (active) {
        features |= Cycle;
        if (!next->image) load(next);
        if (!prev->image) load(prev);
    } else
        features &= ~Cycle;
}

void QGLIV::setDiaShow(bool active)
{
    view->setCursorVisible(!active);
    if (active) {
        features |= Diashow;
        m_wantedDirection = 1;
        QTimer::singleShot(diaTime, this, SLOT(tryChangeImage()));
        m_animationTime = qMin(1500, 15*diaTime/100);
        current->image->scale(QGLImageView::X, 125.0, diaTime);
        current->image->scale(QGLImageView::Y, 125.0, diaTime);
    } else {
        features &= ~Diashow;
        m_animationTime = 300;
        autoSize() ? maxW(m_animationTime) : resetView(m_animationTime);
    }
}

QStringList
loadDir(const QDir &dir)
{
    QThread::currentThread()->setPriority(QThread::LowestPriority);
    QString basePath = dir.absolutePath() + '/';
    QStringList files = dir.entryList(QDir::Files | QDir::Readable, QDir::Name);
    QStringList files2;
    for (int i = 0; i < files.count(); ++i) {
        files[i].prepend(basePath);
        if (QImageReader(files.at(i)).canRead())
            files2 << files.at(i);
    }
    return files2;
}

void
QGLIV::setDirectory(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
        return; // tsts...

    files.clear();

    QFutureWatcher<QStringList>* watcher = new QFutureWatcher<QStringList>;
    connect(watcher, SIGNAL(finished()), this, SLOT(dirLoaded()));
    watcher->setFuture(QtConcurrent::run(loadDir, dir));
}

void
QGLIV::dirLoaded()
{
    if (QFutureWatcher<QStringList>* watcher = dynamic_cast< QFutureWatcher<QStringList>* >(sender())) {
        files = watcher->result();
        watcher->deleteLater(); // has done it's job
    }
    ui.fileCombo->setEntries(files);

    init();
}

bool
QGLIV::filterMatches(int index) const
{
    if (index < 0 || index >= files.count())
        return false;
    return QFileInfo(files.at(index)).fileName().contains(filter);
}

void
QGLIV::clearFilter()
{
    disconnect(this, SIGNAL(imageChanged()), this, SLOT(clearFilter()));
    setFilter(QString());
}

bool
QGLIV::setFilter(const QString &string)
{
    const bool growing = !string.contains(filter);
    filter.setPattern(string.trimmed());

    // get rid of junk
    int newIndex = -1;
    if (!filterMatches(current->index)) {
        int i = 0; bool valid = true;
        while (valid) {
            valid = false;
            if (current->index + i < files.count()) {
                valid = true;
                if (filterMatches(current->index + i)) {
                    newIndex = current->index + i;
                    break;
                }
            }
            if (current->index - i > -1) {
                valid = true;
                if (filterMatches(current->index - i)) {
                    newIndex = current->index - i;
                    break;
                }
            }
            ++i;
        }
    }
    if (newIndex < 0) // not valid or required
        newIndex = current->index;

    if (growing || prev->index > newIndex || !filterMatches(prev->index)) {
        unload(prev);
        prev = new Image(0, newIndex - 1 * (newIndex >= current->index));
        load(prev);
    }
    if (growing || next->index < newIndex || !filterMatches(next->index)) {
        unload(next);
        next = new Image(0, newIndex + 1 * (newIndex <= current->index));
        load(next);
    }
    changeImage(newIndex - current->index);
    return (newIndex != current->index);
}

void
QGLIV::jumpTo(const QString &string)
{
    connect(this, SIGNAL(imageChanged()), this, SLOT(clearFilter()));
    if (!setFilter(string)) {
        disconnect(this, SIGNAL(imageChanged()), this, SLOT(clearFilter()));
        clearFilter();
    }
}

QString
QGLIV::currentImageInfo()
{
    if (files.count() <= current->index)
        return "";
    return "\"" + QFileInfo(files.at(current->index)).fileName() +
           "\" (" + QString::number(current->image->basicWidth()) + "x" +
           QString::number(current->image->basicHeight()) + ")";
}

void
QGLIV::editImage()
{
    if (files.count() <= current->index)
        return;
    view->message(20, 20, "Opening in editor...", 1000);
    QProcess::startDetached(QString(editorCmd).replace("%f", files.at(current->index)));
}

void
QGLIV::setAsWallpaper()
{
    if (files.count() <= current->index)
        return;
    view->message(20, 20, "Setting wallpaper...", 1000);
    QProcess::startDetached(QString(wallpaperCmd).replace("%f", files.at(current->index)));
}

bool
QGLIV::cacheIsReady()
{
    return (prev->image || prev->index < 0) && (next->image || next->index < 0);
}


void
QGLIV::unload(Image *image)
{
    if (!image)
        return;
    if (image->image) {
        view->remove(image->image->id());
        image->image = NULL;
    }
    delete image;
}

void
loadImage(void *ptr, Image *image)
{
    QThread::currentThread()->setPriority(QThread::LowestPriority);
    QGLIV *qgliv = static_cast<QGLIV*>(ptr);
    const bool bwd = (image == qgliv->prev);
    bool wantCycle = qgliv->cycle();

    if (image->index < 0 || image->index > qgliv->files.count() - 1) {
        if (wantCycle) {
            wantCycle = false;
            if (bwd)
                image->index = qgliv->files.count() - 1;
            else
                image->index = 0;
        } else {
            image->index = -1; // invalidate
            return;
        }
    }

    QImage *buffer = new QImage;
    const int inc = bwd ? -1 : 1;
//     int safer = 0;
    while (!qgliv->files.isEmpty()) {
        while (!qgliv->filterMatches(image->index)) {
            // search next valid entry
            image->index += inc;
            if (image->index < 0 || image->index > qgliv->files.count() - 1) {
                if (wantCycle) {
                    wantCycle = false;
                    if (bwd)
                        image->index = qgliv->files.count() - 1;
                    else
                        image->index = 0;
                } else {
                    image->index = -1;
                    delete buffer;
                    return;
                }
            }
        }

        // try to load
        if (buffer->load(qgliv->files.at(image->index)))
            break;
        else {
            // this entry is not a supported Image
            qgliv->files.removeAt(image->index);
            qgliv->filterIsDirty = true;

            // TODO this code is the same as above ==================
            image->index += inc;
            if (image->index < 0 || image->index > qgliv->files.count() - 1) {
                if (wantCycle) {
                    wantCycle = false;
                    if (bwd)
                        image->index = qgliv->files.count() - 1;
                    else
                        image->index = 0;
                } else {
                    image->index = -1;
                    delete buffer;
                    return;
                }
                // ========================
            }
        }
    }

    // TODO pimp qglimageview for use on raw GL char data
//     if ( !buffer->hasAlphaChannel() )
    // pre-convert the image w/o blocking the UI/GLView

    if (buffer->depth() != 32) {
        qWarning("converting non 32bpp imge!");
        QImage copy = buffer->convertToFormat(QImage::Format_ARGB32);
        delete buffer;
        buffer = new QImage(copy);
    }

    // stolen from QGLWidget::convertToGLFormat
    const int width = buffer->width();
    const int height = buffer->height();
    uint *p = (uint*) const_cast<uchar*>(buffer->constScanLine(height - 1));
    uint *q = (uint*) const_cast<uchar*>(buffer->constScanLine(0));
    uint r;

    const int height_2 = (height + 1) / 2;
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        for (int i = 0; i < height_2; ++i) {
            const uint *end = p + width;
            while (p < end) {
                r = (*q << 8) | ((*q >> 24) & 0xff);
                *q = (*p << 8) | ((*p >> 24) & 0xff);
                *p = r;
                p++;
                q++;
            }
            p -= 2 * width;
        }
    } else {
        for (int i = 0; i < height_2; ++i) {
            const uint *end = p + width;
            while (p < end) {
                r = ((*q << 16) & 0xff0000) | ((*q >> 16) & 0xff) | (*q & 0xff00ff00);
                *q = ((*p << 16) & 0xff0000) | ((*p >> 16) & 0xff) | (*p & 0xff00ff00);
                *p = r;
                ++p;
                ++q;
            }
            p -= 2 * width;
        }
    }

    image->qGlImage = buffer;
}

void
QGLIV::load(Image *image, bool block)
{
    if (block) {
        loadImage(this, image);
        imageLoaded();
    } else {
        QFutureWatcher<void>* watcher = new QFutureWatcher<void>;
        connect(watcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
        watcher->setFuture(QtConcurrent::run(loadImage, this, image));
    }
}


void
QGLIV::addToView(Image *image)
{
    if (!image->qGlImage || image->image || image->index < 0)
        return;
    view->load(*image->qGlImage, image == current, numPoly, true);
    image->image = &view->images().last();
    image->image->addShader(shader_vert, false);
    image->image->addShader(shader_frag);
    delete image->qGlImage;
    image->qGlImage = NULL;
}

void
QGLIV::imageLoaded()
{
    if (QFutureWatcher<void>* watcher = dynamic_cast< QFutureWatcher<void>* >(sender()))
        watcher->deleteLater(); // has done it's job

    if (_animationDone) {
        if (filterIsDirty) {
            ui.fileCombo->blockSignals(true);
            const QString text = ui.fileCombo->lineEdit()->text();
            ui.fileCombo->setEntries(files);
            ui.fileCombo->setEditText(text);
            ui.fileCombo->blockSignals(false);
        }
        addToView(prev);
        addToView(current);
        addToView(next);
        if (cacheIsReady())
            emit cacheReady(QString());
    } else
        QTimer::singleShot(50, this, SLOT(imageLoaded()));
}


bool QGLIV::pinch(const QPinchGesture *pg, bool newPinch)
{
    if (!pg)
        return false;

    static int usedAngle = 0;
    if (newPinch) {
        usedAngle = 0;
    }

    float scale = 1.0;
    float angle = 0.0;
    if (pg->changeFlags() & QPinchGesture::ScaleFactorChanged) {
        if (newPinch)
            view->setScaleTarget(pg->centerPoint().toPoint());
        scale = pg->scaleFactor();
        if (scale < 0.95 || scale > 1.05)
            scale = 1.0;
        else if (scale > 0.99 && scale < 1.01)
            scale = 1.0;
    }
    else if (pg->changeFlags() & QPinchGesture::CenterPointChanged) {
        view->setScaleTarget(pg->centerPoint().toPoint());
    }
    if (pg->changeFlags() & QPinchGesture::RotationAngleChanged) {
        angle = pg->totalRotationAngle();
        const int diff = qAbs(angle - pg->lastRotationAngle());
        if (diff > 30.0) { // jump, restart
            usedAngle = 90*(int(angle + 45) / 90);
        } else if (diff > 5.0) {
            scale = 1.0;
        }
        angle -= usedAngle;
        if (angle < -60.0)
            angle = -90.0;
        else if (angle > 60.0)
            angle = 90.0;
        else
            angle = 0.0;
        usedAngle += angle;
    }

    if (scale != 1.0) {
        view->zoom(100*scale);
    }
    if (angle != 0.0) {
        view->rotate(QGLImageView::Z, angle, 250);
    }

    return true;
}

bool QGLIV::pan(const QPanGesture *pg, bool newPan)
{
    if (!pg)
        return false;
    return false;
}

bool QGLIV::swipe(const QSwipeGesture *sg, bool newSwipe)
{
    if (!sg)
        return false;
//     qDebug() << "swipe";
    if (sg->horizontalDirection() == QSwipeGesture::Left)
        changeImage(-1);
    else if (sg->horizontalDirection() == QSwipeGesture::Right)
        changeImage(1);
    else
        return false;
    return true;
}

// view->grabGesture(Qt::TapGesture);
//     view->grabGesture(Qt::TapAndHoldGesture);


bool
QGLIV::eventFilter(QObject *o, QEvent * e)
{
//    if ( e->type() != QEvent::Resize )
//       return false;
//    ( (QGLImageViewer* )o )->images().last().scaleTo( 100.0, 100.0, true, true );

    static QPoint drag_start;
    static bool is_rotation = false;
    static QElapsedTimer swipeTimer;
    if (m_touchMode > 1) {
        if (e->type() == QEvent::Gesture) {
            static bool newPinch = true;
            bool havePinch = false;
            const QPinchGesture *pinchG = NULL;
//             const QSwipeGesture *swipeG = NULL;
//             const QPanGesture *panG = NULL;
            foreach (const QGesture *g, static_cast<QGestureEvent*>(e)->activeGestures()) {
                if (!pinchG)
                    pinchG = qobject_cast<const QPinchGesture*>(g);
//                 if (!swipeG)
//                     swipeG = qobject_cast<const QSwipeGesture*>(g);
//                 if (!panG)
//                     panG = qobject_cast<const QPanGesture*>(g);
            }
            if (pinchG) {
                static_cast<QGestureEvent*>(e)->setAccepted(pinchG);
                havePinch |= pinch(pinchG, newPinch);
            }
//             if (swipeG) {
//                 static_cast<QGestureEvent*>(e)->setAccepted(swipeG);
//                 swipe(swipeG, true);
//             }
            newPinch = !havePinch;
        }
    }

    if (!(features & Diashow) && e->type() == QEvent::Wheel) {
        QWheelEvent *we = static_cast<QWheelEvent*>(e);
        int d = we->delta();
        if (we->modifiers() & Qt::ControlModifier) {
            d > 0 ? zoomIn() : zoomOut();
        } else
            changeImage(-d);
    }

    else if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        if (me->button() == Qt::LeftButton) {
            drag_start = me->pos();
            swipeTimer.start();
            if (m_touchMode == 1) {
                const int tw = view->width() / 8, th = view->height() / 8, x = drag_start.x(), y = drag_start.y();
                is_rotation = ((x < tw && y < th) || (x > view->width() - tw && y < th) ||
                              (x < tw && y > view->height() - th) || (x > view->width() - tw && y > view->height() - th));
            }
        } else if (me->button() == Qt::RightButton)
            ui.rmbPopup->exec(QCursor::pos());
    }

    else if (e->type() == QEvent::MouseMove) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        if (is_rotation && !(me->modifiers() & Qt::ControlModifier)) {
            QMouseEvent me2(QEvent::MouseMove, me->pos(), me->globalPos(), me->button(), me->buttons(), Qt::ControlModifier);
            QCoreApplication::sendEvent(o, &me2);
            return true;
        } else
            return false;
    }

    else if (e->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        if (m_touchMode && !dont_dragswitch && me->button() == Qt::LeftButton) {
            if (is_rotation) {
                maxW(150);
                return false;
            }
            const int dx = drag_start.x() - me->pos().x();
            const int dy = drag_start.y() - me->pos().y();
            int d = 0;
            int xInch = qMax(16, qMin(width()/10, physicalDpiX()));
            const int yInch = qMax(16, qMin(height()/10, physicalDpiY()));
            if (qAbs(dx) < xInch && qAbs(dy) > 3*yInch/2) {
                transitionEffect = VerticalSlide;
                d = dy;
                xInch = yInch;
            } else if (qAbs(dx) > 3*xInch/2 && qAbs(dy) < yInch) {
                transitionEffect = HorizontalSlide;
                d = dx;
            }
            const int t = swipeTimer.elapsed();
            if (d && (!t || qAbs(1000*d/(xInch*t)) < 14)) // >= 14"/s
                d = 0;
            if ((d || m_touchMode < 2) && !changeImage(d))
                maxW(qMax(qAbs(dx), qAbs(dy)) / 4);
        } else if (me->button() == Qt::RightButton)
            ui.rmbPopup->exec(QCursor::pos());
    }

    else if (e->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        if (me->button() != Qt::LeftButton)
            return false;
        if (me->modifiers() & Qt::ControlModifier)
            resetView();
        else
            maxW();
    }

    return false;
}

void
QGLIV::tryChangeImage()
{
    if (!m_wantedDirection)
        return;
    if (cacheIsReady())   // TODO: cacheIsReady( direction )? requires cancelling the thread and ensure the map isn't halfwise borked...
        changeImage(m_wantedDirection);
    else
        QTimer::singleShot(50, this, SLOT(tryChangeImage()));
}

#define BREAK_BECAUSE(_STRING_) { view->message( 20, 20, _STRING_, 2000 ); m_wantedDirection = 0; return false; } //

bool
QGLIV::changeImage(int direction)
{
    if (!direction)   // should not happen
        return false;

    if (direction > 0 && next->index < 0)
        BREAK_BECAUSE("This is the last image");

    if (direction < 0 && prev->index < 0)
        BREAK_BECAUSE("This is the first image");

    if (!cacheIsReady()) {
        if (m_wantedDirection)
            view->message(20, 20, "Please wait...", 0);
        m_wantedDirection = direction;
        tryChangeImage();
        return true;
    }

    if (transitionFinisher->isActive()) {
        transitionFinisher->stop();
        finishImageChange();
    }

    m_wantedDirection = 0;
    m_oldImage = current->image;

    int sign = 1;
    m_transitionType = transitionEffect ? transitionEffect : random() % KnownEffects;

    if (direction > 0) {
        // fwd
        unload(prev);
        prev = current; current = next; next = new Image(0, current->index + 1); load(next);
    } else if (cycle() || current->index > 0) {
        // bwd
        unload(next);
        next = current; current = prev; prev = new Image(0, current->index - 1);  load(prev);
        sign = -1;
    } else
        return false; // no history B.C. ;-P

    // display change
    int delay = m_animationTime + 20;
    int resetTime = m_animationTime;
    _animationDone = false;
    QGLImageView::Axis axis = QGLImageView::X;
    switch (m_transitionType) {
    default:
    case Crossfade:
        delay = m_animationTime;
        current->image->setAlpha(0.0);
        current->image->show(false);
        m_oldImage->setAlpha(0.0, m_animationTime);
        current->image->setAlpha(100.0, m_animationTime);
        break;
    case FadeOutAndIn:
        current->image->setAlpha(0.0);
        m_oldImage->setAlpha(0.0, m_animationTime);
        break;
    case HorizontalRotation:
        axis = QGLImageView::Y;
    case VerticalRotation:
        m_oldImage->rotate(axis, 90.0 * sign, m_animationTime);
        current->image->rotate(axis, -90.0 * sign);
        break;
    case ZSlide:
        axis = QGLImageView::Z;
        current->image->setAlpha(0.0);
        current->image->resize(current->image->basicWidth(), current->image->basicHeight(), 0, 100.0, 100.0);
        m_oldImage->setAlpha(0.0, m_animationTime - sign * 100);
        current->image->setAlpha(100.0, m_animationTime + sign * 100);
    case VerticalSlide:
        if (axis == QGLImageView::X) {
            axis = QGLImageView::Y;
        }
    case HorizontalSlide:
        resetTime = 2*m_animationTime;
        current->image->hide(false);
        if (axis == QGLImageView::Z) {
            m_oldImage->moveTo(axis, (-300.0 * sign)*m_oldImage->scaleFactor(axis), m_animationTime);
        } else {
            m_oldImage->moveTo(axis, (-150.0 * sign)*m_oldImage->scaleFactor(axis), m_animationTime);
        }
        m_oldImage->setAlpha(0.0, m_animationTime);
        delay = m_animationTime/2;
    }

    autoSize() ? maxW(0) : resetView(resetTime);

    if (features & Diashow) {
        current->image->scale(QGLImageView::X, 125.0, diaTime + delay);
        current->image->scale(QGLImageView::Y, 125.0, diaTime + delay);
    }

    m_transitionType *= sign;
    transitionFinisher->start(delay);
    QTimer::singleShot(1000, this, SLOT(setAnimationDone()));
    return true;
}

void QGLIV::finishImageChange()
{
    if (!m_oldImage)
        return;

    m_oldImage->hide();

    //     if ( current->image->hasAlpha() )
    //         view->setCanvas( Qt::gray );
    //     else
    //         view->setCanvas( Qt::black );

    int sign = 1;
    if (m_transitionType < 0) {
        m_transitionType = -m_transitionType;
        sign = -1;
    }


    QGLImageView::Axis axis = QGLImageView::X;
    switch (m_transitionType) {
    default:
    case Crossfade:
        m_oldImage->setAlpha(100.0);
        break;
    case FadeOutAndIn:
        m_oldImage->setAlpha(100.0);
        current->image->show(false);
        current->image->setAlpha(100.0, m_animationTime);
        break;
    case HorizontalRotation:
        axis = QGLImageView::Y;
    case VerticalRotation:
        m_oldImage->rotateTo(0, 0, 0, false);
        current->image->show(false);
        current->image->rotate(axis, sign * 90.0, m_animationTime);
        break;
    case ZSlide:
        axis = QGLImageView::Z;
        // fall through
    case VerticalSlide:
        if (axis == QGLImageView::X)
            axis = QGLImageView::Y;
        // fall through
    case HorizontalSlide:
        m_oldImage->moveTo(axis, 50.0 * m_oldImage->scaleFactor(axis));
        m_oldImage->setAlpha(100.0);
        if (axis == QGLImageView::Z) {
            current->image->moveTo(axis, 300.0 * sign);
        } else {
            current->image->moveTo(axis, 100.0 * sign);
        }
        current->image->show(false);
        current->image->moveTo(axis, 50.0, m_animationTime+20);
    }

    if (showMessage())
        view->message(20, 20, currentImageInfo(), 2500);

    setWindowTitle(files.at(current->index));

    if (prev && m_oldImage == prev->image && !filterMatches(prev->index)) {
        unload(prev);
        prev = new Image(0, current->index - 1);
        load(prev);
    }
    if (next && m_oldImage == next->image && !filterMatches(next->index)) {
        unload(next);
        next = new Image(0, current->index + 1);
        load(next);
    }

    if (features & Diashow) {
        m_oldImage->scale(QGLImageView::X, 80.0);
        m_oldImage->scale(QGLImageView::Y, 80.0);
    }

    m_oldImage = NULL;

    emit imageChanged();

    if (features & Diashow) {
        m_wantedDirection = 1;
        QTimer::singleShot(diaTime, this, SLOT(tryChangeImage()));
    }
}

void QGLIV::saveSettings()
{
    QSettings settings("piQtureGLide");
    settings.setValue("AutoSize", ui.autoSize->isChecked());
    settings.setValue("Transition", ui.transitions->currentIndex());
    settings.setValue("UILevel", ui.level);
    settings.sync();
}

void
QGLIV::setAnimationDone()
{
    _animationDone = true;
}


/** SLOTS for QActions ========================================================================== */


const char* blurstring = "\nBlurrage: %1x%1 kernel";
const char* tunnelstring = "\nTunnel: %1x%1 kernel";

void QGLIV::blurMore()
{
    float fact = current->image->blurrage() + 0.5;
    current->image->boxBlur(fact);
    view->message(20, 20, QString(currentImageInfo() + blurstring).arg(2 * fact + 1), 1000);
}

void QGLIV::blurLess()
{
    float fact = QMAX(0, current->image->blurrage() - 0.5);
    current->image->boxBlur(fact);
    if (fact)
        view->message(20, 20, QString(currentImageInfo() + blurstring).arg(2 * fact + 1), 1000);
    else
        view->message(20, 20, QString(currentImageInfo() + "\nBlurrage: off"), 1000);
}

void QGLIV::tunnelMore()
{
    float fact = current->image->blurrage() + 0.5;
    current->image->tunnel(fact);
    view->message(20, 20, QString(currentImageInfo() + tunnelstring).arg(2 * fact + 1), 1000);
}

void QGLIV::tunnelLess()
{
    float fact = QMAX(0, current->image->blurrage() - 0.5);
    current->image->tunnel(fact);
    if (fact)
        view->message(20, 20, QString(currentImageInfo() + tunnelstring).arg(2 * fact + 1), 1000);
    else
        view->message(20, 20, QString(currentImageInfo() + "\nTunnel: off"), 1000);
}

void QGLIV::toggleAnimation()
{
    if (!supportsAnimation)
        return;
    view->animate(!view->animated());
//     qDebug() << "animating";
}

void QGLIV::changeColor(int key, int v)
{
    QColor c = current->image->color();
    float a = current->image->alpha();
    if (key == Qt::Key_A) { // alpha
        a = CLAMP(a + v, 0.0, 100.0);
        current->image->setAlpha(a, 200);
    } else {
        if (key == Qt::Key_R)
            c = QColor(CLAMP(c.red() + v, 0, 255), c.green(), c.blue());
        else if (key == Qt::Key_G)
            c = QColor(c.red(), CLAMP(c.green() + v, 0, 255), c.blue());
        else if (key == Qt::Key_B)
            c = QColor(c.red(), c.green(), CLAMP(c.blue() + v, 0, 255));
        current->image->tint(c, 200);
    }
    view->message(20, 20, QString("R: %1, G: %2, B: %3, A: %4\%").arg(c.red()).arg(c.green()).arg(c.blue()).arg(a), 1000);
}




void QGLIV::toggleMessage()
{
    if (features & ShowMessage) {
        features &= ~ShowMessage;
        view->hideMessage();
    } else {
        features |= ShowMessage;
        view->message(20, 20, currentImageInfo(), 2500);
    }
}
