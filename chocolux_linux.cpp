#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <ctime>
#include <cmath>

const char *vertShader = "\
varying vec3 s[4];\
void main(){\
gl_Position=gl_Vertex;\
s[0]=vec3(0);\
s[3]=vec3(sin(abs(gl_Vertex.x*.0001)),cos(abs(gl_Vertex.x*.0001)),0);\
s[1]=s[3].zxy;\
s[2]=s[3].zzx;\
}";

const char *fragShader = "\
varying vec3 s[4];\
void main(){\
float t,b,c,h=0;\
vec3 m,n,p=vec3(.2),d=normalize(.001*gl_FragCoord.rgb-p);\
for(int i=0;i<4;i++){\
t=2;\
for(int i=0;i<4;i++){\
b=dot(d,n=s[i]-p);\
c=b*b+.2-dot(n,n);\
if(b-c<t)if(c>0){m=s[i];t=b-c;}\
}\
p+=t*d;\
d=reflect(d,n=normalize(p-m));\
h+=pow(n.x*n.x,44.)+n.x*n.x*.2;\
}\
gl_FragColor=vec4(h,h*h,h*h*h*h,h);\
}";

void compileShader()
{
    GLuint p = ((PFNGLCREATEPROGRAMPROC) glXGetProcAddress((const GLubyte *) "glCreateProgram"))();

    GLuint s = ((PFNGLCREATESHADERPROC) glXGetProcAddress((const GLubyte *) "glCreateShader"))(GL_VERTEX_SHADER);
    ((PFNGLSHADERSOURCEPROC) glXGetProcAddress((const GLubyte *) "glShaderSource"))(s, 1, (const GLchar**) (&vertShader), NULL);
    ((PFNGLCOMPILESHADERPROC) glXGetProcAddress((const GLubyte *) "glCompileShader"))(s);
    ((PFNGLATTACHSHADERPROC) glXGetProcAddress((const GLubyte *) "glAttachShader"))(p, s);

    s = ((PFNGLCREATESHADERPROC) glXGetProcAddress((const GLubyte *) "glCreateShader"))(GL_FRAGMENT_SHADER);
    ((PFNGLSHADERSOURCEPROC) glXGetProcAddress((const GLubyte *) "glShaderSource"))(s, 1, (const GLchar**) (&fragShader), NULL);
    ((PFNGLCOMPILESHADERPROC) glXGetProcAddress((const GLubyte *) "glCompileShader"))(s);
    ((PFNGLATTACHSHADERPROC) glXGetProcAddress((const GLubyte *) "glAttachShader"))(p, s);

    ((PFNGLLINKPROGRAMPROC) glXGetProcAddress((const GLubyte *) "glLinkProgram"))(p);
    ((PFNGLUSEPROGRAMPROC) glXGetProcAddress((const GLubyte *) "glUseProgram"))(p);
}

int main()
{
    Display *dpy = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(dpy);

    GLint att[] = {GLX_RGBA,
                   GLX_DOUBLEBUFFER,
                   None};
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);

    XSetWindowAttributes swa;

    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;

    Window win = XCreateWindow(dpy, root, 0, 0, 1024, 768, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

    XMapWindow(dpy, win);

    GLXContext context = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, context);

    int x11_fd = ConnectionNumber(dpy);
    fd_set in_fds;

    compileShader();

    XEvent xev;
    int t;
    struct timespec ts;
    struct timeval tv;
    while (true)
    {
        FD_ZERO(&in_fds);
        FD_SET(x11_fd, &in_fds);

        clock_gettime(CLOCK_MONOTONIC, &ts);
        t = ts.tv_sec * 1000 + round(ts.tv_nsec / 1.0e6);

        glRecti(t, t, -t, -t);
        glXSwapBuffers(dpy, win);

        tv.tv_usec = 0;
        tv.tv_sec = 0;
        if (select(x11_fd + 1, &in_fds, NULL, NULL, &tv) > 0)
        {
            XNextEvent(dpy, &xev);
            if (xev.type == KeyPress)
            {
                if (xev.xkey.keycode == 9)
                {
                    glXMakeCurrent(dpy, None, NULL);
                    glXDestroyContext(dpy, context);
                    XDestroyWindow(dpy, win);
                    XCloseDisplay(dpy);
                    return 0;
                }
            }
        }
    }
}
