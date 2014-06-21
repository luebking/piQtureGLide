piQtureGLide
==============
A simple Imageviewer on top of OpenGL.
This doe only require Qt4. All image formats supported by Qt4 are supported as well (i.e. at least jpeg & png, maybe gif. 
If Qt4 was linked against Graphics/Image-Magick: MANY :-)

This was first release more then two years ago as KGLImageViewer, i hoped someone would pick up the code and 
integrate it into a real image viewer, this did not happen and i thought, i'd only need some improvements to get a 
usable and slim viewer with a lot of iCandy to show off.

So there's now multithreaded background image loading, a pimped UI and some new effects. I also started cleaning up
the code and sanely implement some features of the backing gl widget.

The target audience is less Gwenview but more ImageMagick::display users
You can pass directories and or files as arguments - nothing else (atm)

Have fun.


For completeness: the README of the original release

KGLImageViewer

A simple Image viewer on top of OpenGL
The main aspect of this package is the sublying class that is meant to be reused in real
image viewers.

NOTES ON PIXEL/VERTEX SHADERS:
1. not every system may support shaders (kgliv mentions that it's trying to use shaders) - get a new GPU if you want them (sorry)
2. kgliv tries to load the vertex and pixelshaders from two environment variables, or "color.vert" & "color.frag" in the current dir, if environment variables are missed
3. That is, start kgliv like:
      
   KGLIV_VSHADER=path/to/the/vertex.shader KGLIV_PSHADER=path/to/the/pixel.shader kgliv image.png
      
4. yes. you can edit the shaders as ever you want (and can) and use your own manipulations
5. kgliv will post shader compiler errors - if any
6. the uniforms
- float brightness
- float inverted
- vec3 color
- float alpha
- float time
are exported by kgliv as expected. please don't rely on gl_Color (it's set to strange values for the non shader coloring/brightness algorithm, i'm gonna change that and try to use a fixed internal shader coloring system if available by default - LATER ;)
7. why? well, shaders make really a lot of sense on GL imaging. the color/brightness system via the shaders is by far better (and hardly slower, depending on your GPU) than the before hackaround (especially on handling the alpha chanel etc.)
8. why else? shaders are really cool stuff ;)

THIS IS A PREVIEW RELEASE!
The library API is not completely fixed yet
(i guess i'll change sth. about the "move" parameter spectrum)
The lib doc is incomplete and so is the tutorial (i.e. the application doc)
Code needs clean up

NOTICE:
The image viewer is only thought as demo app
to give developers sth. to read and to show off
It's NOT gonna be a really great image viewer.
Instead viewers like Gwenview and Kuickshow may include the GLView Widget
Don't ask for more features and don't mourn about app bugs ;)


If you need however features in the lib or find a bug here please feel free to drop me
a mail

I will also probably write a simple script interpreter for some kind of diashow.


To INSTALL:
untar the archive, call "qmake qmake.pro && make"
you may then copy the binary "kgliv" to some directory in your $PATH environment,
e.g. /usr/bin to call it from anywhere.

Usage:
call "kgliv [imagepath] [another imagepath]"
or e.g. "kgliv *" to show any image in the current dir

KEYS: (supported by the viewer widget)
Ctrl+LMB: rotate around the Z axis
Ctrl+Shift+LMB: rotate around the X axis
Ctrl+Alt+LMB: rotate around the Y axis
Shift+LMB: scale to marked point
LMB to drag around

Some keycombinations from the app - see the menubar

Have Fun
