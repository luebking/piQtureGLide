/* QGLImageViewer.h v0.2 - This file is NOT part of KDE ;)
   Copyright(C) 2006 Thomas L�bking <thomas.luebking@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or(at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KGLIMAGEVIEWER_H
#define KGLIMAGEVIEWER_H

#define GL_GLEXT_PROTOTYPES

#include <QGLWidget>

class QTimer;
class QStringList;
class QGLPixelBuffer;

namespace QGLImageView {

class QGLImage;

/**
* The 3 axis of space
*/
enum Axis{X = 0, Y, Z};

/**
 * The usage should be pretty simple. You load images and rotate, scale, move tint and fade them by your needs. \n
 * However, there's one thing in 3D in general and OpenGL in special that seems to confuse people, so i'll describe the "problem" and how it's implemented in this class. \n
 * The confusing thing is the difference between model(i.e. the images in our case) and view transformation.\n \n
 * I guess the main reason why people are confused by this when writing OpenGL applications is that OpenGL handles both, model and view transformations on the same Matrix stack, i.e. only the time when you make a call decides whether you influence the model or the view. \n
 * Luckily, you won't face this aspect with the QGLImageViewer class as the transformation values for the images and the view are set independently(as this is an object oriented approach ;) ) \n
 * Now what is this like? \n
 * 1. The class limits the OpenGL calls to what i believe makes most sense for this purpose - if you need anything beyond this, you'll have to reimplement the paintGL() function. \n
 * 2. The QGLImageView implementation of OpenGL transformation calls can be viewn as a solar system: Every action happens around the sun \n
 * In the beginning you're in the center of that system, i.e. you're the sun. \n
 * You can place images like planets around you, so you'll leave the view as it is and call move() and rotate() on the images to position and turn them in space.
 * TODO: finish this up... ;)
 *
 * @short An image viewer on top of OpenGL \n
 * This class provides a 3-dimensional image display area. \n
 * It allows to load multimple images and transform them independently in a 3D space as well as manipulating the space as well \n
 *
 * @author Thomas Lübking <thomas.luebking@web.de>
 *
 */

typedef QList<GLhandleARB> ShaderList;

class QGLImageViewer : public QGLWidget
{
   Q_OBJECT

public:
   /**
   * Creates a new image display area
   *
   * @param parent The parenting QWidget
   * @param name string name for the widget
   * @param fps The framerate for animated transitions(defaults to 25/PAL, NTSC uses 29.4)
   * @param interactive Controls wheter the user may change the view position/roation/scale
   */
   QGLImageViewer( QWidget* parent, const char* name, float fps = 25, bool interactive = true);
   ~QGLImageViewer();
   
   /**
   * Whether the user may influence the view using the mouse cursor
   *
   * @param interactive Yes or No ;)
   */
   inline void setInteractive(bool interactive = true){ _interactive = interactive; }
   
   /**
   * Sets the canvas, i.e. glClear Color(the background where no images appear... defaults to black)
   *
   * @param color The color
   * @param update Whether to update the view right after updating the color
   */
   void setCanvas(const QColor & color, bool update = true);
   
   /**
   * The position of the viewer
   *
   * @param a On which axis
   */
   inline float position(Axis a) const { return _translation[a]; }
   
   /**
   * The rotation of the viewer
   *
   * @param a On which axis
   */
   inline float rotation(Axis a) const { return _rotation[a]; }
   
   /**
   * The scale of the view
   *
   * @param a On which axis
   */
   inline float scaleFactor(Axis a) const { return _scale[a]; }
   
   /**
   * Rotate by some degrees(accumulating)\n
   * Keep in mind that this is a 3D environment. If you rotate around the X xor the Y axis by 180�, the rotation direction(CW/CCW) around the Z axis is swapped
   *
   * @param a On which axis(use Z for a 2D rotation)
   * @param degrees How much. Values may be positive(CW) or negative(CCW). Rotates accumulative(new rotation = current rotation + degrees)
   * @param msecs How long the rotation shall last, the default is 0 - thus the new rotation is reached immediately. NOTICE: that rotation() will allways respond the current value, not the final value, while the rotation is active
   */
   void rotate(Axis a, float degrees, int msecs = 0);
   
   /**
   * Rotate around all three axes(accumulating)\n
   * Keep in mind that this is a 3D environment. If you rotate around the X xor the Y axis by 180�, the rotation direction(CW/CCW) around the Z axis is swapped
   *
   * @param xDegrees Rotation around the X axis(horizontal axis)
   * @param yDegrees Rotation around the Y axis(vertical axis)
   * @param zDegrees Rotation around the Z axis(2D rotation)
   * @param update Whether the screen should be updated after this action(set false in case you want to perform several translations to the view and this is not the last)
   */
   void rotate(float xDegrees, float yDegrees, float zDegrees, bool update = true);
   
   /**
   * Rotate by some degrees(NOT accumulative)\n
   * Keep in mind that this is a 3D environment. If you rotate around the X xor the Y axis by 180�, the rotation direction(CW/CCW) around the Z axis is swapped
   *
   * @param a On which axis(use Z for a 2D rotation)
   * @param degrees How much. Values may be positive(CW) or negative(CCW). Rotates nonaccumulative(new rotation = degrees)
   * @param msecs How long the rotation shall last, the default is 0 - thus the new rotation is reached immediately. NOTICE: that rotation() will allways respond the current value, not the final value, while the rotation is active
   */
   void rotateTo(Axis a, float degrees, int msecs = 0);
   
   /**
   * Rotate around all three axes(NOT accumulative)\n
   * Keep in mind that this is a 3D environment. If you rotate around the X xor the Y axis by 180�, the rotation direction(CW/CCW) around the Z axis is swapped
   *
   * @param xDegrees Rotation around the X axis(horizontal axis)
   * @param yDegrees Rotation around the Y axis(vertical axis)
   * @param zDegrees Rotation around the Z axis(2D rotation)
   * @param update Whether the screen should be updated after this action(set false in case you want to perform several translations to the view and this is not the last)
   */
   void rotateTo(float xDegrees, float yDegrees, float zDegrees, bool update = true);
   
   /**
   * Scale by some percent(accumulating)
   *
   * @param a On which axis(use X or Y, Z is ignored as images don't have a third dimension)
   * @param percent How much. Values should be positive. Scales accumulative, i.e. if your view was previously scaled to 80% scale(X, 50%); will end up with a 40% scaled view
   * @param msecs How long the scaling shall last, the default is 0 - thus the new scale is reached immediately. NOTICE: that scacleFactor() will allways respond the current value, not the final value, while scaling is active
   */
   void scale(Axis a, float percent, int msecs = 0);
   
   /**
   * Scale X and Y axis by some percent(accumulating)
   *
   * @param yPercent 
   * @param xPercent How much. Values should be positive. Scales accumulative, i.e. if your view was previously scaled to 80% scale(X, 50%); will end up with a 40% scaled view
   * @param update Whether the screen should be updated after this action(set false in case you want to perform several translations to the view and this is not the last)
   */
   void scale(float xPercent, float yPercent, bool update = true);
   
   /**
   * Scale by some percent(NOT accumulative)
   *
   * @param a On which axis(use X or Y, Z is ignored as images don't have a third dimension)
   * @param percent How much. Values should be positive. Scales NOT accumulative, i.e. if your view was previously scaled to 80% scale(X, 50%); will end up with a 50% scaled view
   * @param msecs How long the scaling shall last, the default is 0 - thus the new scale is reached immediately. NOTICE: that scacleFactor() will allways respond the current value, not the final value, while scaling is active
   */
   void scaleTo(Axis a, float percent, int msecs = 0);
   /**
   * Scale X and Y axis by some percent(NOT accumulative)
   *
   * @param yPercent 
   * @param xPercent How much. Values should be positive. Scales accumulative, i.e. if your view was previously scaled to 80% scale(X, 50%); will end up with a 50% scaled view
   * @param update Whether the screen should be updated after this action(set false in case you want to perform several translations to the view and this is not the last)
   */
   void scaleTo(float xPercent, float yPercent, bool update = true);
   
   
   void move(Axis a, float percent, int msecs = 0);
   void move(float xPercent, float yPercent, float zPercent, bool update = true);
   void moveTo(Axis a, float percent, int msecs = 0);
   void moveTo(float xPercent, float yPercent, float zPercent, bool update = true);
   
   /**
   * Display a message(string)
   * @param x The X offset in pixels
   * @param y The Y offset in pixels
   * @param message The string to display
   * @param msecs how long the Message shall be displayed
   * @param color The text color. The message will have an inverse colored translucent background.\n If NULL(the default) the Widgets foreground color will be used(i might change this to the textcolor in the future)
   */
   void message( int x, int y, QString message, int msecs, const QColor *color = 0L );
   void hideMessage();
   
#if 0
   /**
   * Draws a rect.(HUD, it's no way translated)
   * @param rect The rectangle. View relative, i.e. if your view is 600x300px, QRect(200,100,200,100) will draw around the center.
   * @param longlines If true, the rect lines will reach up to the view border
   * @param color The frame color
   * @param lineWidth How thick the lines are(pixels) \n NOTICE that different GL implementations allow different value ranges - maybe your value is clamped
   * @param fill Whether the rect shall be filled(default no)
   * @param fillColor Which color shall be used for filling
   * @param fillAlpha If you can still see through a filled rect - alpha == 0 is as unfilled, alpha = 100 is opaque(you cannot see through)
   */
   void mark(const QRect & rect, bool longLines = false, const QColor & color = Qt::white, float lineWidth = 3.0, bool fill = false, const QColor & fillColor = Qt::white, float fillAlpha = 60.0);
#endif
   /**
   * Load an image from a QGLImage \n
   * It can be referred accessed as images().last() as long as no other Image was loaded \n
   * This constructor creates a "Flat" copy of the image, i.e. the /same/ QGLImage::glObject() is used \n
   * You can however set any parameters(transformation, coloring, etc.) on this image without influencing the original
   *
   * @param image The QGLImage to use. May contain alpha level, different from OpenGL textures there are NO restrictions on the size.
   * @param show The Image is not displayed unless this flag is true or it's explicitly shown later(see QGLImage::show())
   * @return the id of the just loaded image
   */
   uint load( const QGLImage& image, bool show = false );
   
   /**
   * Load an image from a QImage \n
   * It can be accessed as images().last() as long as no other Image was loaded
   *
   * @param img The QImage to use. May contain alpha level, different from OpenGL textures there are NO restrictions on the size.
   * @param show The Image is not displayed unless this flag is true or it's explicitly shown later(see QGLImage::show())
   * @return the id of the just loaded image
   */
   uint load( const QImage& img, bool show = false, int numPol = 1, bool isGLFormat = false );
   /**
   * Conveniance function, see below. \n
   * Loads an image from a file.
   *
   * @param imgPath A relative or absolute path to the image
   * @param show The Image is not displayed unless this flag is true or it's explicitly shown later(see QGLImage::show())
   * @return the id of the just loaded image or -1 if unable to open image from path
   */
   int load( const QString& imgPath, bool show = false, int numPol = 1 );
   /**
   * Conveniance function, see below. \n
   * Converts the QPixmap to a QImage and loads that
   *
   * @param pix A QPixmap
   * @param show The Image is not displayed unless this flag is true or it's explicitly shown later(see QGLImage::show())
   */
   uint load( const QPixmap& pix, bool show = false, int numPol = 1);
   
   /**
   * Removes the image with the id(glObject()) id from view \n
   * The function handles the GLlist containing the image \n
   * DON'T DARE TO CALL images().remove() directly unless you REALLY!!! know what you're doing
   *
   * @param id the images glObject()
   */
   void remove(uint id);
   
   /**
   * The FPS delay, i.e. after how many milliseconds the screen is updated in animations(1000/fps)
   */
   int fpsDelay() {return _fpsDelay;}
   
   inline bool providesShaders(){return _providesShaders; }
   
   typedef QList<QGLImage> QGLImageList;

   /**
   * A QValueList of all currently loaded QGLImages
   */
   QGLImageList &images() {return _images;}
   
   /*inline */bool isTimerActive() const;
   
   /**
   * Loads a vertex or fragment shader
   * Returns the shader id(all shaders a stored in the shaders()) or 0 if shader could not be created from file
   *
   *@param file The textfile that contains the shader
   *@param shaderType use GL_VERTEX_SHADER_ARB(vertexshader) or GL_FRAGMENT_SHADER_ARB(pixelshader)
   */
   GLhandleARB loadShader(QString file, GLenum shaderType);
   
   /**
   * Returns a list of all available shader programs
   */
   inline ShaderList &shaders() { return _shaders; };
   inline bool animated() { return _animated; }
   
public slots:
   /**
   * Reimplemented to skip if timer is active, i.e. we have an animation
   */
   virtual void updateGL();
   virtual void animate(bool yes = true);
protected:
   friend class QGLImage;
   void ensureTimerIsActive();
   virtual void mergeCnB(QGLImage & img);
   
   /**
   * Reimplemented from QGLWidget
   */
   virtual void initializeGL();
   
   /**
   * Reimplemented from QGLWidget, you may want to reimplement it yourself to change the scene rendering
   */
   virtual void paintGL();
   
   /**
   * Reimplemented from QGLWidget, you may want to reimplement it yourself to change the behaviour on resizes
   */
   virtual void resizeGL( int w, int h );

   /**
   * Reimplemented from QWidget for the interaction feature
   */
   virtual void mousePressEvent( QMouseEvent *e );
   
   /**
   * Reimplemented from QWidget for the interaction feature
   */
   virtual void mouseReleaseEvent( QMouseEvent *e );
   
   /**
   * Reimplemented from QWidget for the interaction feature
   */
   virtual void mouseMoveEvent( QMouseEvent *e );

   /**
    * 
    */
   virtual void timerEvent( QTimerEvent *e );
   
   /**
   * Not really reimplemented from QWidget yet
   */
   virtual void wheelEvent( QWheelEvent *e );
   
   void blur(QGLImage &img);

private:
   QGLImageList _images;
   typedef QMap<GLuint, uint> ObjectCounter;
   ObjectCounter _objectCounter;
   GLfloat _scale[3];
   GLfloat _desiredScale[3];
   GLfloat _scaleStep[3];
   
   GLfloat _translation[3];
   GLfloat _desiredTranslation[3];
   GLfloat _translationStep[3];
   
   GLfloat _rotation[3];
   GLfloat _desiredRotation[3];
   GLfloat _rotationStep[3];
   
   QStringList _message;
   QPoint _messagePos;
   int _messageTimeOut;
   GLfloat _messageColor[4];
   
   QPoint _lastPos;
   float _scaleTarget[2];
   
   uint _fpsDelay;
   int _activeAnimations;
   int _timer;
   
   float _canvasColor[3];
   
   /* HUD rect */
   int _rectCoords[4];
   float _rectColor[3];
   bool _rectFilled;
   float _rectFillColor[4];
   float _rectThickness;
   bool _rectLongLines;

   QGLPixelBuffer *_pbuffer;
   bool _interactive;
   
   ShaderList _shaders;
   bool _providesShaders;
   bool _animated;
   float _animCounter;
   
   
private:
   void handleAnimationsPrivate(float(*value)[3], float(*desiredValue)[3], float(*valueStep)[3], int *animCounter);
   uint newUniqueId();

};

/**
 * Images for the QGLImageViewer(GLlists and stuff around)
 *
 *
 * @author Thomas Lübking <thomas.luebking@web.de>
 *
 */
class QGLImage
{
public:
   /**
   * An empty constructor to make QValueList happy -- !! DON'T USE IT !!
   */
   QGLImage(){}

#if 0
   /**
   * Destructor
   */
//    ~QGLImage();
#endif
   
   /**
   * Returns the current onscreen pixelwidth \n
   * NOTICE: This value changes as soon as the image or the view are translated on the Z axis, scaled or the view is resized. \n
   * You need to recall it if you need it after such operation.
   */
   int width() const;
   
   /**
   * Returns the current onscreen pixelheight \n
   * NOTICE: This value changes as soon as the image or the view are translated on the Z axis, scaled or the view is resized. \n
   * You need to recall it if you need it after such operation.
   */
   int height() const;
   
   /**
   * Returns the original width of the image in pixles \n
   * different from width() this value stays the same all the time, no matter which operations on the image are performed
   */
   inline int basicWidth() const { return _basicWidth; }
   
   /**
   * Returns the original height of the image in pixles \n
   * different from height() this value stays the same all the time, no matter which operations on the image are performed
   */
   inline int basicHeight() const { return _basicHeight; }
   
   /**
   * Returns the current alpha level of the image \n
   * NOTICE: if the alpha value is set animated, this is NOT the final but allways the CURRENT!!! alpha value
   */
   inline float alpha() const { return _colorI[3]*100.0; }
   
   /**
   * Whether the Image has an alpha channel \n
   * NOTICE: this differs from the above function! hasAlpha() does NOT refer to the alpha value that you can setAlpha(), but if the original image comes with an active alpha channel(like an icon etc.)\n
   * if you want a help on taking a decision about the canvas color, THIS is the proper function
   */
   inline bool hasAlpha() const { return _hasAlpha; }
   
   /**
   * Sets the internal color.\n
   * WANRNING: the internal color is updated as soon as QGLImageViewer::mergeCnB() is called.
   * You probably only wnat to call this one if reimplementing QGLImageViewer::mergeCnB()
   *
   *@param color the color to be set(rgb)
   */
   inline void setInternalColor(const QColor color)
   {
      _colorI[0] =((float)color.red())/255.0;
      _colorI[1] =((float)color.green())/255.0;
      _colorI[2] =((float)color.blue())/255.0;
   }
   
   /**
   * Returns the current tint color of the image \n
   * NOTICE: if the color is set animated, this is NOT the final but allways the CURRENT!!! color
   */
   inline QColor color() const { return QColor((int)(_color[0]*255),(int)(_color[1]*255),(int)(_color[2]*255)); }

   /**
   * Returns the current brightness of the image \n
   * NOTICE: if the brightness is set animated, this is NOT the final but allways the CURRENT!!! color
   */
   inline float brightness() const { return _brightness*100.0; }
   
   /**
   * Returns the current position of the image on axis a \n
   * NOTICE: if the image is moved animated, this is NOT the final but allways the CURRENT!!! position
   */
   inline float position(Axis a) const { return _translation[a]; }
   
   /**
   * Returns the current rotation of the image(degrees, may be negative) \n
   * NOTICE: if the image is rotated animated, this is NOT the final but allways the CURRENT!!! rotation
   */
   inline float rotation(Axis a) const { return _rotation[a]; }
   
   /**
   * Returns the current scale of the image(percent, allways positive) \n
   * NOTICE: if the image is scaled animated, this is NOT the final but allways the CURRENT!!! scale
   */
   inline float scaleFactor(Axis a) const { return _scale[a]; }
   
   /**
   * Returns the OpenGL index of this image \n
   * NOTICE: as long as you don't use flat copies of the image, this is unique - OTHERWISE NOT!
   */
   inline GLuint glObject() const { return _object; }
   inline uint id() const { return _id; }
   
   inline QGLImageViewer *parent() const { return _parent; }
   
   /**
   * Whether the image is shown
   *
   */
   inline bool isShown() const { return _isShown; }
//    int positioni(Axis a);
   
   /**
   * Set the image to be shown
   *
   * @param update Whether the View should be updated right after showing the image, set to false in case you want to perform many operations on the image and present the user only the final state \n
   * Example: you want to show the image, set the alpha value to zero and then fade it in:
   * @code
   * view.load("Some_image.png", false);
   * view.images().last().setAlpha(0.0);
   * view.images().last().show();
   * view.images().last().setAlpha(100.0, 500);
   * @endcode
   */
   void show(bool update = true);
   
   /**
   * Set the image to be hidden
   *
   * @param update Whether the View should be updated right after hiding the image, set to false in case you want to perform many operations on the image and present the user only the final state
   */
   void hide(bool update = true);
   
   /**
   * Rotate the image by some degrees(accumulating) \n
   * Keep in mind that this is a 3D environment. If you rotate around the X xor the Y axis by 180�, the rotation direction(CW/CCW) around the Z axis is swapped \n
   * NOTICE: the rotation happens in ADDITION to the VIEW rotation
   *
   * @param a On which axis(use Z for a 2D rotation)
   * @param degrees How much. Values may be positive(CW) or negative(CCW). Rotates accumulative(new rotation = current rotation + degrees)
   * @param msecs How long the rotation shall last, the default is 0 - thus the new rotation is reached immediately. NOTICE: that rotation() will allways respond the current value, not the final value, while the rotation is active
   */
   void rotate(Axis a, float degrees, int msecs = 0);
   
   /**
   * Rotate the image around all three axes(accumulating) \n
   * Keep in mind that this is a 3D environment. If you rotate around the X xor the Y axis by 180�, the rotation direction(CW/CCW) around the Z axis is swapped \n
   * NOTICE: the rotation happens in ADDITION to the VIEW rotation
   *
   * @param xDegrees Rotation around the X axis(horizontal axis)
   * @param yDegrees Rotation around the Y axis(vertical axis)
   * @param zDegrees Rotation around the Z axis(2D rotation)
   * @param update Whether the screen should be updated after this action(set false in case you want to perform several translations to the view and this is not the last)
   */
   void rotate(float xDegrees, float yDegrees, float zDegrees, bool update = true);
   
   /**
   * Rotate the image to demanded degrees(NOT accumulative) \n
   * Keep in mind that this is a 3D environment. If you rotate around the X xor the Y axis by 180�, the rotation direction(CW/CCW) around the Z axis is swapped \n
   * NOTICE: the rotation happens in ADDITION to the VIEW rotation
   *
   * @param a On which axis(use Z for a 2D rotation)
   * @param degrees How much. Values may be positive(CW) or negative(CCW). Rotates nonaccumulative(new rotation = degrees)
   * @param msecs How long the rotation shall last, the default is 0 - thus the new rotation is reached immediately. NOTICE: that rotation() will allways respond the current value, not the final value, while the rotation is active
   */
   void rotateTo(Axis a, float degrees, int msecs = 0);
   
   /**
   * Rotate the image around all three axes(NOT accumulative) \n
   * Keep in mind that this is a 3D environment. If you rotate around the X xor the Y axis by 180�, the rotation direction(CW/CCW) around the Z axis is swapped \n
   * NOTICE: the rotation happens in ADDITION to the VIEW rotation
   *
   * @param xDegrees Rotation around the X axis(horizontal axis)
   * @param yDegrees Rotation around the Y axis(vertical axis)
   * @param zDegrees Rotation around the Z axis(2D rotation)
   * @param update Whether the screen should be updated after this action(set false in case you want to perform several translations to the view and this is not the last)
   */
   void rotateTo(float xDegrees, float yDegrees, float zDegrees, bool update = true);
   
   /**
   * Scale the image by some percent(accumulating) \n
   * NOTICE: the scaling happens in ADDITION to the VIEW scale
   *
   * @param a On which axis(use X or Y, Z is ignored as images don't have a third dimension)
   * @param percent How much. Values should be positive. Scales accumulative, i.e. if your view was previously scaled to 80% scale(X, 50%); will end up with a 40% scaled view
   * @param msecs How long the scaling shall last, the default is 0 - thus the new scale is reached immediately. NOTICE: that scacleFactor() will allways respond the current value, not the final value, while scaling is active
   */
   float scale(Axis a, float percent, int msecs = 0);
   
   /**
   * Scale the image along the X and Y axis by some percent(accumulating) \n
   * NOTICE: the scaling happens in ADDITION to the VIEW scale
   *
   * @param yPercent 
   * @param xPercent How much. Values should be positive. Scales accumulative, i.e. if your view was previously scaled to 80% scale(X, 50%); will end up with a 40% scaled view
   * @param update Whether the screen should be updated after this action(set false in case you want to perform several translations to the view and this is not the last)
   */
   void scale(float xPercent, float yPercent, bool update = true);
   
   /**
   * Scale the image along given axis to some percent(NOT accumulating) \n
   * NOTICE: the scaling happens in ADDITION to the VIEW scale
   *
   * @param a the Axis. Use X or Y, Z is ignored
   * @param percent The final scale factor.
   * @param msecs How long the scaling shall last, the default is 0 - thus the new scalefactor is reached immediately.\n
   * NOTICE: that scaleFactor() will allways respond the current value - not the final value - while the scaling is active
   * @param viewRelative 
   * @param assumedViewScale Assume you want to scale viewRelative and at the same time scale the view. 
   */
   float scaleTo(Axis a, float percent, int msecs = 0, bool viewRelative = false, float assumedViewScale = -1.0);
   void scaleTo(float xPercent, float yPercent, bool update = true, bool viewRelative = false, float assumedViewScaleX = -1.0, float assumedViewScaleY = -1.0);
   void move(Axis a, float percent, int msecs = 0);
   void move(float xPercent, float yPercent, float zPercent, bool update = true);
   void moveTo(Axis a, float percent, int msecs = 0);
   void moveTo(float xPercent, float yPercent, float zPercent, bool update = true);
   
   void setAlpha(float percent, int msecs = 0);
   void tint(const QColor & color, int msecs = 0);
   void setBrightness(float percent, int msecs = 0);
   
   /**
   * Blur the image
   *
   * @param factor How much((blur kernel - 1)/2, i.e. blur(1) results in a 3x3 kernel)
   * @param msecs How fast(don't use animated blurring on big kernels - stupid idea)
   *
   * (Qt4) Blurring is gained by painting several shifted translucent versions of the image over each other,
   * so the higher the blur factor is, the more does it cost. The result is saved in a texture.
   *(Qt3) The blurring is gained by creating a prefabbed boxblurred miniversion of the image and use the GL lienar upscaler
   * and blending to create blurring effects(different from Qt4, this makes it necessary to blend and an additional gllist at
   * paint time)
   *
   * (Qt4) Blurring is pretty fast(as it's performed spatial), at least for not too big kernels. And accurate "enough"
   *(though it's not like blurring with "The GIMP", but that's not the approach here)
   *
   *(Qt4) if you pass "1" as blur type, a ceter motion blur is performed instead of a boxblur
   *(i.e. the image blurs more to the edges and keeps sharp in the center).
   * This function is O(1/2 * O(glScale) / O(glTranform)) compared to the boxblur(but of course a different effect)
   *
   * Blur functions ARE EXCLUSIVE(i.e. you cannot perform a boxblur on top of the motion blur - at least for the moment)
   */
   inline void boxBlur(float factor, int msecs = 0) {blur(factor, msecs, 0);}
   /**
   * See boxBlur()
   */
   inline void tunnel(float factor, int msecs = 0) {blur(factor, msecs, 1);}
   void blur(float factor, int msecs = 0, int type = 0);
   
   inline float blurrage(){return _blur;}
   
   void setClipRect(int x, int y, int w, int h, bool update = true);
   inline void setClipRect(QRect &r, bool update = true) { setClipRect(r.x(), r.y(), r.width(), r.height(), update); }
   
   inline void setClipping(bool enabled = true) { _hasClipping = enabled; }
   inline bool hasClipping() const { return _hasClipping; }
   
   void invert(bool inverted = true, bool update = true);
   inline bool isInverted() { return _inverted; }
   
   
   void resize(int width, int height, int msecs = 0, float assumedViewScaleX = -1.0, float assumedViewScaleY = -1.0);
   
   /**
   * Adds a shader to the images pipeline
   *
   *@param shader A shader previously loaded with QGLImageViewer::loadShader()
   *@param linkProgram If you want to add many shaders in a row, set this to false for any but the last one(performance issue)
   *@param update Whether to update the display after adding this shader(autoset to false if you're not linking, so addShader(myFancyShader, false, true) will NOT update the display)
   */
   void addShader(GLhandleARB shader, bool linkProgram = true, bool update = true);
   /**
   * Convenience function. Calls addShader(QGLImageViewer::loadShader(file, shaderType), relinkProgram, update);
   */
   inline void addShader(QString file, GLenum shaderType, bool linkProgram = true, bool update = true)
   {
      addShader(_parent->loadShader(file, shaderType), linkProgram, update);
   }
   /**
   * Removes a shader from the images pipeline
   *
   *@param shader A shader to remove(if non existent nothing happens)
   *@param relinkProgram If you want to remove many shaders in a row, set this to false for any but the last one(performance issue)
   *@param update Whether to update the display after removing this shader(autoset to false if you're not linking, so removeShader(myFancyShader, false, true) will NOT update the display)
   */
   void removeShader(GLhandleARB shader, bool relinkProgram = true, bool update = true);
   
   /**
   * The shader program(collection of shaders) for this image(may be zero if no shader was added)
   */
   inline GLhandleARB shaderProgram(){ return _shaderProgram; }
   
   /**
   * Sets the images shader program to the program of another image
   * WARNING: The two images will then share the same shader program and a modification on one will affect the other as well
   *
   *@param img The QGLImage to share the shader program with
   */
   inline void setShaderProgram(QGLImage &img){ _shaderProgram = img._shaderProgram; }
   
   /**
   * A QList containig all shaders that are atached to this image
   */
   inline ShaderList & shaders() {return _shaders;}
   
   /**
   * Set a uniform variable you may need in your shader.\n
   * Attributes cannot be set, as it won't make any sense in this context(attributes are only necessary to set them between glBegin() and glEnd() - what you're usually not gonna do)\n
   *
   *@param var the shader uniform variable ID(can be queried by calling the below overload at the first time)
   *@param varSize [1,4] whether you want to set float, vec2, vec3 or vec4
   *@param val1 the 1st value to be set
   *@param val2 the 2nd value to be set
   *@param val3 the 3rd value to be set
   *@param val4 the 4th value to be set
   */
   void setShaderUniform(const int var, const int varSize, const float val1, const float val2 = 0.0, const float val3 = 0.0, const float val4 = 0.0);
   
   /**
   * Overloaded for convenience - queries the id for the variable string and sets the variable value \n
   * Returns the variable identifier
   */
   int setShaderUniform(QString var, const int varSize, const float val1, const float val2 = 0.0, const float val3 = 0.0, const float val4 = 0.0);
   
   
protected:
   void paint();
   
private:
   QGLImage(QGLImageViewer *parent, const uint id, const GLuint object, GLuint *textures, const int texAmount, int width, int height, bool hasAlpha = false);
//    friend void QGLImageViewer::load( const QImage& img, bool show = false );
   friend class QGLImageViewer;
   GLfloat _scale[3];
   GLfloat _desiredScale[3];
   GLfloat _scaleStep[3];
   
   GLfloat _translation[3];
   GLfloat _desiredTranslation[3];
   GLfloat _translationStep[3];
   
   GLfloat _rotation[3];
   GLfloat _desiredRotation[3];
   GLfloat _rotationStep[3];
   
   GLfloat _color[3];
   GLfloat _colorI[4];
   GLfloat _desiredColor[4];
   GLfloat _colorStep[4];
   
   GLfloat _brightness;
   GLfloat _desiredBrightness;
   GLfloat _brightnessStep;
   
   GLfloat _blur;
   GLfloat _desiredBlur;
   GLfloat _blurStep;
   int _blurType;
   
   GLdouble _clip[4][4];
   
   GLint _combineRGB;
   
   uint _id;
   GLuint _object;
   GLuint _blurObj;
   GLuint _blurTex;
   int _blurW, _blurH;
   GLuint *_textures;
   int _texAmount;
   int _basicWidth, _basicHeight;
   bool _isShown;
   int _activeAnimations;
   bool _hasAlpha, _hasClipping;
   QGLImageViewer *_parent;
   float _ratio;
   bool _inverted;
   
   ShaderList _shaders;
   GLhandleARB _shaderProgram;
   GLint _sColor, _sAlpha, _sBrightness, _sInverted, _sTime;
   int _numPol;
};

} // namespace

#endif //KGLIMAGEVIEWER_H
