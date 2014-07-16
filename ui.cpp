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

#include <QAction>
#include <QDial>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QToolBar>
#include <QToolBox>
#include <QToolButton>
#include <QVBoxLayout>

#include "dirfilecombo.h"
#include "qgliv.h"

class EventKiller : public QObject
{
public:
    EventKiller() : QObject()
    {
        qWarning("new eventkiller");
    }
protected:
    bool eventFilter(QObject*, QEvent*)
    {
        qWarning("kill some event");
        return true;
    }
};

#define ADD_ACTION( _KEY_, _SLOT_, _TIP_ )\
    action = new QAction( this ); addAction( action ); action->setShortcut( Qt::Key_##_KEY_ );\
    action->setToolTip( _TIP_ );\
    view->addAction( action );\
    connect ( action, SIGNAL( triggered(bool) ), this, SLOT( _SLOT_ ) )

#define ADD_COLOR_SLIDER( _KEY_, _Color_, _color_, _RANGE_ )\
    sliderBox = new QHBoxLayout;\
    label = new QLabel( #_Color_ " ", widget );\
    label->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );\
    sliderBox->addWidget( label );\
    ui.slider##_Color_ = new QSlider( Qt::Horizontal, widget ); ui.slider##_Color_->setFixedWidth( 85 );\
    ui.slider##_Color_->setRange( 0, _RANGE_ ); ui.slider##_Color_->setValue( _RANGE_ );\
    ui.slider##_Color_->setToolTip("('Shift' + ) "#_KEY_);\
    sliderBox->addWidget( ui.slider##_Color_ );\
    widget->layout()->addItem( sliderBox );\
    connect ( ui.slider##_Color_, SIGNAL( valueChanged(int) ), this, SLOT( set##_Color_(int) ) );\
    \
    ADD_ACTION( _KEY_ + Qt::SHIFT, _color_##Up(), "Shift + "#_KEY_ );\
    ADD_ACTION( _KEY_, _color_##Down(), #_KEY_ ); //

#define ADD_BUTTON( _LAYOUT_, _LABEL_, _KEY_, _SLOT_, _TIP_ )\
    btn = new QToolButton( widget ); action = new QAction( _LABEL_, btn );\
    action->setShortcut( Qt::Key_##_KEY_ );\
    action->setToolTip( _TIP_ );\
    view->addAction( action );\
    btn->addAction( action ); btn->setDefaultAction( action );\
    connect ( action, SIGNAL( triggered(bool) ), this, SLOT( _SLOT_ ) );\
    _LAYOUT_->addWidget( btn ); //

#ifndef CLAMP
#define CLAMP( x, l, u ) ( x ) < ( l ) ? ( l ) :\
    ( x ) > ( u ) ? ( u ) :\
    ( x )
#endif

#define DO_BLOCKED( _WIDGET_, _ACTION_ )\
    _WIDGET_->blockSignals( true ); _WIDGET_->_ACTION_; _WIDGET_->blockSignals( false )

static QWidget *spacerWidget(Qt::Orientation o, QWidget *parent = 0)
{
    QWidget *spacer = new QWidget(parent);
    if (o == Qt::Horizontal)
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    else
        spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    return spacer;
}

static QString tooltip(const QString &function, const QStringList &shortcuts)
{
    QString tip = "<b>" + function + "</b>";
    foreach(QString shortcut, shortcuts)
    tip += "<br>&nbsp;&nbsp;" + shortcut;
    return tip;
}

void QGLIV::setupUI(const QString &startDir)
{
    QAction *action; QToolButton *btn;

    ui.toolbar = new QToolBar(this);

    ui.toolbar->addWidget(new QLabel("Folder:", ui.toolbar));

    ui.dirCombo = new DirFileCombo(ui.toolbar, QDir::Dirs, startDir);
    ui.dirCombo->setMaximumWidth(200);
    ui.toolbar->addWidget(ui.dirCombo);

    ui.toolbar->addWidget(spacerWidget(Qt::Horizontal, ui.toolbar));

    ui.toolbar->addWidget(new QLabel("Filter:", ui.toolbar));

    ui.fileCombo = new DirFileCombo(ui.toolbar, QDir::Files, "none");
    ui.fileCombo->setEntries(files);
    ui.fileCombo->setMaximumWidth(200);
    ui.fileCombo->setEditText(QString());
    ui.toolbar->addWidget(ui.fileCombo);

    ui.toolbar->addWidget(btn = new QToolButton(ui.toolbar));
    btn->setText("Set wallpaper");
    connect(btn, SIGNAL(clicked()), this, SLOT(setAsWallpaper()));

    connect(ui.dirCombo, SIGNAL(currentIndexChanged(const QString&)), ui.fileCombo, SLOT(setDirectory(const QString&)));
    connect(ui.dirCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setDirectory(const QString&)));

    connect(ui.fileCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(jumpTo(const QString&)));
    connect(ui.fileCombo, SIGNAL(textEntered(const QString&)), this, SLOT(setFilter(const QString&)));


    ui.tools = new QToolBox(this);

    QHBoxLayout *btnBox;

    QWidget *widget = ui.tools->widget(ui.tools->addItem(new QWidget, "Navigation"));
    widget->setLayout(new QVBoxLayout(widget));

    btnBox = new QHBoxLayout;
    btnBox->addWidget(new QLabel("Diashow delay: ", widget));
    QDoubleSpinBox *spin = new QDoubleSpinBox(widget);
    spin->setSuffix(" s");
    spin->setDecimals(2);
    spin->setValue(diaTime / 1000.0);
    connect(spin, SIGNAL(valueChanged(double)), this, SLOT(setDiaShowTime(double)));
    btnBox->addWidget(spin);
    widget->layout()->addItem(btnBox);

    widget->layout()->addWidget(new QLabel("Transition Effect", widget));
    ui.transitions = new QComboBox(widget);
    ui.transitions->addItems(QStringList() <<  "Random" << "Crossfade" << "Fade Out & In" <<
                             "Rotate horizontally" << "Rotate vertically" <<
                             "Slide horizontally" << "Slide vertically" <<
                             "Slide through screen");
    connect(ui.transitions, SIGNAL(currentIndexChanged(int)), this, SLOT(setTransitionEffect(int)));

    widget->layout()->addWidget(ui.transitions);

    btnBox = new QHBoxLayout;
    ADD_BUTTON(btnBox, "<<", PageUp, bwd(), tooltip("Previous Image", QStringList() << "[PageUp]" << "Mousewheel"));
    QAction *bwdAction = action;
    ADD_BUTTON(btnBox, ">>", PageDown, fwd(), tooltip("Next Image", QStringList() << "[PageDown]" << "Mousewheel"));
    QAction *fwdAction = action;
    widget->layout()->addItem(btnBox);
    ADD_ACTION(Home, begin(), "Jump to begin");
    ADD_ACTION(End, end(), "Jump to end");

    btnBox = new QHBoxLayout;
    btnBox->addStretch(); ADD_BUTTON(btnBox, "^", Up, moveUp(), tooltip("Move up", QStringList() << "[Up]" << "Just drag the image"));
    btnBox->addStretch();
    widget->layout()->addItem(btnBox);

    btnBox = new QHBoxLayout;
    ADD_BUTTON(btnBox, "<", Left, moveLeft(), tooltip("Move left", QStringList() << "[Left]" << "Just drag the image"));
    ADD_BUTTON(btnBox, ">", Right, moveRight(), tooltip("Move right", QStringList() << "[Right]" << "Just drag the image"));
    widget->layout()->addItem(btnBox);

    btnBox = new QHBoxLayout;
    btnBox->addStretch(); ADD_BUTTON(btnBox, "v", Down, moveDown(), tooltip("Move down", QStringList() << "[Down]" << "Just drag the image"));
    btnBox->addStretch();
    widget->layout()->addItem(btnBox);


    static_cast<QVBoxLayout*>(widget->layout())->addStretch();

    widget = ui.tools->widget(ui.tools->addItem(new QWidget, "Rotation"));
    widget->setToolTip(tooltip("Rotation", QStringList() << "Ctrl + Shift + LMB Drag" << "Ctrl + Shift + Arrow keys"));
    widget->setLayout(new QVBoxLayout(widget));

    btnBox = new QHBoxLayout;
    btnBox->addStretch();
    ADD_BUTTON(btnBox, "^", Up + Qt::CTRL + Qt::SHIFT, flipUp(), tooltip("Flip vertical", QStringList("[Ctrl + Shift + Up]")));
    btnBox->addStretch();
    widget->layout()->addItem(btnBox);

    btnBox = new QHBoxLayout;
    ADD_BUTTON(btnBox, "<", Left + Qt::CTRL + Qt::SHIFT, flipLeft(), tooltip("Flip horizontal", QStringList("[Ctrl + Shift + Left]")));
    ADD_BUTTON(btnBox, ">", Right + Qt::CTRL + Qt::SHIFT, flipRight(), tooltip("Flip horizontal", QStringList("[Ctrl + Shift + Right]")));
    widget->layout()->addItem(btnBox);

    btnBox = new QHBoxLayout;
    btnBox->addStretch();
    ADD_BUTTON(btnBox, "v", Down + Qt::CTRL + Qt::SHIFT, flipDown(), tooltip("Flip vertical", QStringList("[Ctrl + Shift + Down]")));
    btnBox->addStretch();
    widget->layout()->addItem(btnBox);

    ui.rotateZ = new QDial(widget); ui.rotateZ->setWrapping(true); ui.rotateZ->setRange(0, 360);
    connect(ui.rotateZ, SIGNAL(valueChanged(int)), this, SLOT(ZRotate(int)));

    widget->layout()->addWidget(ui.rotateZ);

    ui.rotateX = new QSlider(widget); ui.rotateX->setRange(-360, 360);
    connect(ui.rotateX, SIGNAL(valueChanged(int)), this, SLOT(XRotate(int)));

    QHBoxLayout *sliderBox = new QHBoxLayout;
    sliderBox->addWidget(ui.rotateZ);
    sliderBox->addWidget(ui.rotateX);
    widget->layout()->addItem(sliderBox);

    ui.rotateY = new QSlider(Qt::Horizontal, widget); ui.rotateY->setRange(-360, 360);
    connect(ui.rotateY, SIGNAL(valueChanged(int)), this, SLOT(YRotate(int)));
    widget->layout()->addWidget(ui.rotateY);

    ADD_ACTION(Left + Qt::CTRL, r90ccw(), "Rotate left");
    ADD_ACTION(Right + Qt::CTRL, r90cw(), "Rotate right");

    ADD_BUTTON(widget->layout(), "Reset", R + Qt::CTRL, resetRotation(), tooltip("Reset rotation", QStringList("[Ctrl + R]")));

    static_cast<QVBoxLayout*>(widget->layout())->addStretch();

    widget = ui.tools->widget(ui.tools->addItem(new QWidget, "Color"));
    widget->setLayout(new QVBoxLayout(widget));

    btnBox = new QHBoxLayout;
    btnBox->addWidget(new QLabel("Canvas value", widget));
    QSlider *slider = new QSlider(widget); slider->setRange(0, 255);
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setCanvasValue(int)));
    btnBox->addWidget(slider);
    widget->layout()->addItem(btnBox);

    btnBox = new QHBoxLayout;
    ADD_BUTTON(btnBox, "Reset", C + Qt::SHIFT, resetColors(), "[Shift + C]");
    btnBox->addStretch();
    ADD_BUTTON(btnBox, "Invert", I, invert(), "[I]");
    widget->layout()->addItem(btnBox);


    QLabel *label;

    ADD_COLOR_SLIDER(R, Red, red, 255);
    ADD_COLOR_SLIDER(G, Green, green, 255);
    ADD_COLOR_SLIDER(B, Blue, blue, 255);
    ADD_COLOR_SLIDER(A, Alpha, alpha, 100);
    ADD_COLOR_SLIDER(L, Gamma, gamma, 200);

    ui.viewBar = new QToolBar(this);

    action = ui.viewBar->addAction("Fullscreen", this, SLOT(toggleFullscreen()));
    view->addAction(action);
    action->setToolTip(tooltip("Toggle Fullscreen", QStringList("[Alt + Enter]")));
    action->setShortcuts(QList<QKeySequence>() << Qt::ALT + Qt::Key_Return << Qt::ALT + Qt::Key_Enter);

    ADD_ACTION(U + Qt::CTRL, toggleUI(), tooltip("Toggle UI",  QStringList("[Ctrl + U]")));
    action->setText("Toggle UI");
    ui.viewBar->addAction(action);

    action = ui.viewBar->addAction("(-)", this, SLOT(setCycle(bool)));
    action->setCheckable(true); action->setShortcut(Qt::CTRL + Qt::Key_C);
    action->setToolTip("Cylcle Images");
    view->addAction(action);

    ui.viewBar->addWidget(spacerWidget(Qt::Horizontal, ui.viewBar));
    ui.viewBar->addAction(bwdAction);

    action = ui.viewBar->addAction(" > ", this, SLOT(setDiaShow(bool)));
    action->setShortcut(Qt::CTRL + Qt::Key_D); action->setCheckable(true);
    action->setToolTip(tooltip("Diashow", QStringList("[Ctrl + D]")));
    view->addAction(action);

    ui.viewBar->addAction(fwdAction);
    ui.viewBar->addWidget(spacerWidget(Qt::Horizontal, ui.viewBar));

    ui.autoSize = action = ui.viewBar->addAction("Fit", this, SLOT(setAutosize(bool)));
    action->setCheckable(true);
    action->setToolTip(tooltip("Maximize", QStringList() << "[*]"  << "Doubleclick"));

    ADD_ACTION(Asterisk, maxW(), tooltip("Maximize", QStringList() << "[*]"  << "Doubleclick"));

    ui.zoomTimer = new QTimer(this);
    ui.zoomSlider = new QSlider(Qt::Horizontal, ui.viewBar);
    ui.zoomSlider->setFixedWidth(80);
    ui.viewBar->addWidget(ui.zoomSlider);
    ui.zoomSlider->setRange(-1, 1);
    ui.zoomSlider->setToolTip(tooltip("Zoom", QStringList() << "[+]/[-]"  << "[Shift] + LMB Drag"));
    connect(ui.zoomSlider, SIGNAL(sliderReleased()), this, SLOT(resetZoomSlider()));
    connect(ui.zoomSlider, SIGNAL(sliderMoved(int)), this, SLOT(zoom(int)));

    ADD_ACTION(Plus, zoomIn(), tooltip("Zoom in", QStringList("[+]")));
    ADD_ACTION(Minus, zoomOut(), tooltip("Zoom in", QStringList("[-]")));

    action = ui.viewBar->addAction("100%", this, SLOT(resetView()));
    action->setShortcut(Qt::Key_Slash);
    action->setToolTip(tooltip("Reset view", QStringList() << "[/]"  << "[Ctrl] + Doubleclick"));
    view->addAction(action);

    static_cast<QVBoxLayout*>(widget->layout())->addStretch();
}

void QGLIV::toggleUI()
{
    if (ui.tools->isVisibleTo(this)) {
        ui.level = 1;
        ui.tools->hide();
    } else if (!m_touchMode && ui.viewBar->isVisibleTo(this)) {
        ui.level = 2;
        ui.viewBar->hide();
    } else if (ui.toolbar->isVisibleTo(this)) {
        ui.level = 3;
        ui.toolbar->hide();
    } else {
        ui.level = 0;
        ui.tools->show();
        ui.toolbar->show();
        ui.viewBar->show();
    }
    if (current && autoSize())
        maxW();
}

void QGLIV::setCanvasValue(int v)
{
    view->setCanvas(QColor(v, v, v));
}

void QGLIV::setRed(int v)
{
    QColor c = current->image->color();
    c.setRed(CLAMP(v, 0, 255));
    current->image->tint(c);
}

void QGLIV::setGreen(int v)
{
    QColor c = current->image->color();
    c.setGreen(CLAMP(v, 0, 255));
    current->image->tint(c);
}

void QGLIV::setBlue(int v)
{
    QColor c = current->image->color();
    c.setBlue(CLAMP(v, 0, 255));
    current->image->tint(c);
}

void QGLIV::setAlpha(int v)
{
    current->image->setAlpha(CLAMP(v, 0, 100));
}

void QGLIV::setGamma(int v)
{
    current->image->setBrightness(v);
}

void QGLIV::ZRotate(int r)
{
    view->rotateTo(QGLImageView::Z, -r);
}

void QGLIV::YRotate(int r)
{
    view->rotateTo(QGLImageView::Y, r);
#if 0
    int x = 0;
    if (r == -1) {
        r = 359;
        x = ui.rotateY->width();
    } else if (r == 360) {
        r = 0;
        x = 1;
    }

    if (x) {
        EventKiller *ek = new EventKiller; ui.rotateY->installEventFilter(ek);
        ui.rotateY->blockSignals(true);
        ui.rotateY->setValue(r);
//         QCursor::setPos( ui.rotateY->mapToGlobal( QPoint( x, ui.rotateY->height()/2 ) ) );
        ui.rotateY->blockSignals(false);
        ui.rotateY->removeEventFilter(ek); delete ek;
        qWarning("eventkiller deleted");
    }
#endif
}

void QGLIV::XRotate(int r)
{
    view->rotateTo(QGLImageView::X, -r);
#if 0
    int y = 0;
    if (r == -1) {
        r = 359;
        y = 1;
    } else if (r == 360) {
        r = 0;
        y = ui.rotateX->height();
    }

    if (y) {
        ui.rotateX->blockSignals(true);
        EventKiller *ek = new EventKiller; ui.rotateX->installEventFilter(ek);
        ui.rotateX->setValue(r);
        QCursor::setPos(ui.rotateX->mapToGlobal(QPoint(ui.rotateX->width() / 2, y)));
        ui.rotateX->removeEventFilter(ek); delete ek;
        ui.rotateX->blockSignals(false);
    }
#endif
}

void QGLIV::resetRotation()
{
    view->rotateTo(QGLImageView::X, 0.0, 520);
    view->rotateTo(QGLImageView::Y, 0.0, 520);
    view->rotateTo(QGLImageView::Z, 0.0, 520);
    current->image->rotateTo(QGLImageView::X, 0.0, 520);
    current->image->rotateTo(QGLImageView::Y, 0.0, 520);
    current->image->rotateTo(QGLImageView::Z, 0.0, 520);

    DO_BLOCKED(ui.rotateX, setValue(0));
    DO_BLOCKED(ui.rotateY, setValue(0));
    DO_BLOCKED(ui.rotateZ, setValue(0));
}

void QGLIV::setDiaShowTime(double d)
{
    diaTime = (int)d * 1000;
}

void QGLIV::setTransitionEffect(int v)
{
    transitionEffect = CLAMP(v, 0, KnownEffects - 1);
}

void QGLIV::setAutosize(bool on)
{
    if (on) {
        if (current) maxW();
        features |= Autosize;
    } else
        features &= ~Autosize;
}

void QGLIV::zoom(int dir)
{
    ui.zoomTimer->stop();
    ui.zoomTimer->disconnect();
    if (dir < 0) {
        zoomOut();
        connect(ui.zoomTimer, SIGNAL(timeout()), SLOT(zoomOut()));
    } else if (dir > 0) {
        zoomIn();
        connect(ui.zoomTimer, SIGNAL(timeout()), SLOT(zoomIn()));
    } else {
        ui.zoomSlider->setValue(0);
        view->scale(QGLImageView::X, 100.0, 0);
        view->scale(QGLImageView::Y, 100.0, 0);
        return;
    }
    ui.zoomTimer->start(520);
}

void QGLIV::resetZoomSlider()
{
    zoom(0);
}

void QGLIV::resetView(int ms)
{
    // view
    view->rotateTo(QGLImageView::X, 0.0, ms);
    view->rotateTo(QGLImageView::Y, 0.0, ms);
    view->rotateTo(QGLImageView::Z, 0.0, ms);

    view->scaleTo(QGLImageView::X, 100.0, ms);
    view->scaleTo(QGLImageView::Y, 100.0, ms);

    if (current) {
        current->image->moveTo(QGLImageView::X, 50.0, ms);
        current->image->moveTo(QGLImageView::Y, 50.0, ms);
        current->image->rotateTo(QGLImageView::X, 0.0, ms);
        current->image->rotateTo(QGLImageView::Y, 0.0, ms);
        current->image->rotateTo(QGLImageView::Z, 0.0, ms);
        current->image->resize(current->image->basicWidth(), current->image->basicHeight(), ms, 100.0, 100.0);
    }

    view->moveTo(QGLImageView::X, 50.0, ms);
    view->moveTo(QGLImageView::Y, 50.0, ms);
    /** NOTICE the Z axis is set to 8.0. what can be interpreted as moving 8 steps backward.
    this value is arbitrary.
    the value should be bigger than 3.0, otherwise nothing placed at 0.0 ( the images ) will be visable
    ( put a peace of paper right in front of your eyes and try to read sth. to understand this )
    the additional 2 steps are made so that the view doesn't get clipped as soon as it's rotated slightly by the X or Y axis ( "into" the screen )
    */
    view->moveTo(QGLImageView::Z, 8.0, ms);

    // ui
    DO_BLOCKED(ui.rotateX, setValue(0));
    DO_BLOCKED(ui.rotateY, setValue(0));
    DO_BLOCKED(ui.rotateZ, setValue(0));
}

void QGLIV::maxW(int ms)
{
    dont_dragswitch = false;
    current->image->moveTo(QGLImageView::X, 50.0, ms);
    current->image->moveTo(QGLImageView::Y, 50.0, ms);

    view->scaleTo(QGLImageView::X, 100.0, ms);
    view->scaleTo(QGLImageView::Y, 100.0, ms);
    view->rotateTo(QGLImageView::X, 0.0, ms);
    view->rotateTo(QGLImageView::Y, 0.0, ms);
    int rz = view->rotation(QGLImageView::Z);
    rz += 45; rz -= rz % 90;
    bool portrait = rz % 180;
    view->rotateTo(QGLImageView::Z, rz, ms);

    float vH = view->height(), vW = view->width(), a = 1.0;
    if (portrait) {
        vH = view->width();
        vW = view->height();
        a = vW / vH;
    }
    if (vH / current->image->basicHeight() > vW / current->image->basicWidth())
        current->image->scaleTo(QGLImageView::Y, current->image->scaleTo(QGLImageView::X, 100.0 * a, ms, true, 100.0), ms);
    else
        current->image->scaleTo(QGLImageView::X, current->image->scaleTo(QGLImageView::Y, 100.0 * (1.0 / a), ms, true, 100.0), ms);
    view->moveTo(QGLImageView::X, 50.0, ms);
    view->moveTo(QGLImageView::Y, 50.0, ms);

    // ui
    DO_BLOCKED(ui.rotateX, setValue(0));
    DO_BLOCKED(ui.rotateY, setValue(0));
    DO_BLOCKED(ui.rotateZ, setValue(0));
}

void QGLIV::begin()
{
    if (features & Diashow) return;
    jumpTo(ui.fileCombo->itemText(0));
}
void QGLIV::bwd()
{
    if (features & Diashow) return;
    changeImage(-1);
}
void QGLIV::fwd()
{
    if (features & Diashow) return;
    changeImage(1);
}
void QGLIV::end()
{
    if (features & Diashow) return;
    jumpTo(ui.fileCombo->itemText(ui.fileCombo->count() - 1));
}



void QGLIV::moveLeft()
{
    view->move(QGLImageView::X, -15.0, 120);
}
void QGLIV::moveRight()
{
    view->move(QGLImageView::X, 15.0, 120);
}
void QGLIV::moveUp()
{
    view->move(QGLImageView::Y, -15.0, 120);
}
void QGLIV::moveDown()
{
    view->move(QGLImageView::Y, 15.0, 120);
}

/** rotate by 90° clockwise*/
void QGLIV::r90cw()
{
    view->rotate(QGLImageView::Z, 90.0, 520);
}
/** rotate by 90° counterclockwise*/
void QGLIV::r90ccw()
{
    view->rotate(Z, -90.0, 520);
}

void QGLIV::flipLeft()
{
    view->rotate(QGLImageView::Y, -180.0, 520);
}
void QGLIV::flipRight()
{
    view->rotate(QGLImageView::Y, 180.0, 520);
}
void QGLIV::flipUp()
{
    view->rotate(QGLImageView::X, -180.0, 520);
}
void QGLIV::flipDown()
{
    view->rotate(QGLImageView::X, 180.0, 520);
}

/** zoom ( in ) in by 125%*/
void QGLIV::zoomIn()
{
    dont_dragswitch = true;
    view->scale(QGLImageView::X, 125.0, 520);
    view->scale(QGLImageView::Y, 125.0, 520);
}

/** zoom ( out ) in by 80% == 1/125%*/
void QGLIV::zoomOut()
{
    dont_dragswitch = true;
    view->scale(QGLImageView::X, 80.0, 520);
    view->scale(QGLImageView::Y, 80.0, 520);
}

void QGLIV::gammaUp()
{
    const float val = current->image->brightness() + 5.0;
    current->image->setBrightness(val);
    view->message(20, 20, QString(currentImageInfo() + "\nBrightness: %1\%").arg(val), 1000);
}

void QGLIV::gammaDown()
{
    const float val = current->image->brightness() - 5.0;
    current->image->setBrightness(val);
    view->message(20, 20, QString(currentImageInfo() + "\nBrightness: %1\%").arg(val), 1000);
}

void QGLIV::resetColors()
{
    current->image->setAlpha(100.0, 520);
    current->image->tint(Qt::white, 520);
}

void QGLIV::invert()
{
    current->image->invert(!current->image->isInverted());
}

void QGLIV::redUp()
{
    changeColor(Qt::Key_R, 10);
}
void QGLIV::redDown()
{
    changeColor(Qt::Key_R, -10);
}
void QGLIV::greenUp()
{
    changeColor(Qt::Key_G, 10);
}
void QGLIV::greenDown()
{
    changeColor(Qt::Key_G, -10);
}
void QGLIV::blueUp()
{
    changeColor(Qt::Key_B, 10);
}
void QGLIV::blueDown()
{
    changeColor(Qt::Key_B, -10);
}
void QGLIV::alphaUp()
{
    changeColor(Qt::Key_A, 5);
}
void QGLIV::alphaDown()
{
    changeColor(Qt::Key_A, -5);
}

void QGLIV::toggleFullscreen()
{
    if (isFullScreen()) {
        setPalette(QPalette());
//         toolbar->show();
        window()->showNormal();
    } else {
        setPalette(QPalette(QColor(200, 200, 200), Qt::black, QColor(200, 200, 200),
                            Qt::black, QColor(200, 200, 200), QColor(200, 200, 200),
                            QColor(200, 200, 200), Qt::black, Qt::black));
//         toolbar->hide();
        window()->showFullScreen();
    }
    if (current && autoSize())
        maxW();
}
