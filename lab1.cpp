//
//modified by: Antonio Solorio
//date: January 25, 2018
//
//3350 Spring 2018 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
// .general animation framework
// .animation loop
// .object definition and movement
// .collision detection
// .mouse/keyboard interaction
// .object constructor
// .coding style
// .defined constants
// .use of static variables
// .dynamic memory allocation
// .simple opengl components
// .git
//
//elements we will add to program...
//   .Game constructor
//   .multiple particles
//   .gravity
//   .collision detection
//   .more objects
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/Xdbe.h>
#include <GL/glx.h>
#include "fonts.h"   // Give up using this library

const int MAX_PARTICLES = 9999;

const float GRAVITY = 0.1;

//some structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
	float color[3];
};

class Global {
public:
	int xres, yres;
	Shape box[5];
	Shape circle;
	char waterfall[5][15];
	int shift[5];
	Particle particle[MAX_PARTICLES];
	int n;
	Global() {
		xres = 500;
		yres = 360;
		//define a box shape
		for(int i = 0; i < 5; i++) {
			box[i].width = 75;
			box[i].height = 10;
			box[i].center.x = -200 + 5*65 + 35*i;
			box[i].center.y = 600 - 5*60 - 35*i;
		}
		//define circle
		circle.radius = 130;
		circle.center.x = 375;
		circle.center.y = 0;
		n = 0;
		
		strcpy(waterfall[0], "Requirements");
		strcpy(waterfall[1], "Design");
		strcpy(waterfall[2], "Coding");
		strcpy(waterfall[3], "Testing");
		strcpy(waterfall[4], "Maintenance");
		shift[0] = 38;
		shift[1] = 18;
		shift[2] = 17;
		shift[3] = 19;
		shift[4] = 32;

	}
} g;


class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	~X11_wrapper() {
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
	}
	X11_wrapper() {
		GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
		int w = g.xres, h = g.yres;
		dpy = XOpenDisplay(NULL);
		if (dpy == NULL) {
			cout << "\n\tcannot connect to X server\n" << endl;
			exit(EXIT_FAILURE);
		}
		Window root = DefaultRootWindow(dpy);
		XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
		if (vi == NULL) {
			cout << "\n\tno appropriate visual found\n" << endl;
			exit(EXIT_FAILURE);
		} 
		Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		XSetWindowAttributes swa;
		swa.colormap = cmap;
		swa.event_mask =
			ExposureMask | KeyPressMask | KeyReleaseMask |
			ButtonPress | ButtonReleaseMask |
			PointerMotionMask |
			StructureNotifyMask | SubstructureNotifyMask;
		win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
			InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
		set_title();
		glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
		glXMakeCurrent(dpy, win, glc);
	}
	void set_title() {
		//Set the window title bar.
		XMapWindow(dpy, win);
		XStoreName(dpy, win, "3350 Lab1");
	}
	bool getXPending() {
		//See if there are pending events.
		return XPending(dpy);
	}
	XEvent getXNextEvent() {
		//Get a pending event.
		XEvent e;
		XNextEvent(dpy, &e);
		return e;
	}
	void swapBuffers() {
		glXSwapBuffers(dpy, win);
	}

} x11;

//Function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void movement();
void render();

void makeParticle(int x, int y);


//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main()
{
	srand(time(NULL));
	init_opengl();
	//Main animation loop
	int done = 0;
	while (!done) {
		//Process external events.
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			check_mouse(&e);
			done = check_keys(&e);
		}
		for(int i = 0; i < 10; i++) {
		    makeParticle(150, 340);
		}
		movement();
		render();
		x11.swapBuffers();
	}
	return 0;
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, g.xres, g.yres);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void makeParticle(int x, int y)
{
	if (g.n >= MAX_PARTICLES)
		return;
	cout << "makeParticle() " << x << " " << y << endl;
	//position of particle
	Particle *p = &g.particle[g.n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = ((float)rand() / (float)RAND_MAX) * 1;
	p->velocity.x = ((float)rand() / (float)RAND_MAX) * 1 - 0.5;
	p->color[0] = (((float)rand() / (3*(float)RAND_MAX)) + 0.15); 
	p->color[1] = (((float)rand() / (8*(float)RAND_MAX)) + 0.5);
	p->color[2] = (((float)rand() / (3*(float)RAND_MAX)) + 0.67);
	
//		re *= ((float)rand() / (float)RAND_MAX) * 1;
//		gr *= ((float)rand() / (float)RAND_MAX) * 1;
//		bl *= ((float)rand() / (float)RAND_MAX) * 1;
//		if(re < 50 && gr < 50 && bl < 50) {
//			re = 255;
//			gr = 255;
//			bl = 255;

	++g.n;
}

void check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;

	if (e->type != ButtonRelease &&
		e->type != ButtonPress &&
		e->type != MotionNotify) {
		//This is not a mouse event that we care about.
		return;
	}
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = g.yres - e->xbutton.y;
			for(int i = 0; i < 10; i++) {
				makeParticle(e->xbutton.x, y);
			}
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	if (e->type == MotionNotify) {
		//The mouse moved!
		if (savex != e->xbutton.x || savey != e->xbutton.y) {
			savex = e->xbutton.x;
			savey = e->xbutton.y;
			int y = g.yres - e->xbutton.y;
			for(int i = 0; i < 10; i++) {
				makeParticle(e->xbutton.x, y);
			}


		}
	}
}

int check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
			case XK_1:
				//Key 1 was pressed
				break;
			case XK_a:
				//Key A was pressed
				break;
			case XK_Escape:
				//Escape key was pressed
				return 1;
		}
	}
	return 0;
}

void movement()
{
	if (g.n <= 0)
		return;
	for(int i = 0; i < g.n; i++) {
		Particle *p = &g.particle[i];
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
		p->velocity.y -= GRAVITY;

		//check for collision with shapes...
		//Shape *s;
		for (int j=0; j<5; j++){
		    Shape *s = &g.box[j];
			if (p->s.center.y < s->center.y + s->height 
			    && p->s.center.x > s->center.x - s->width 
			    && p->s.center.x < s->center.x + s->width
			    && p->s.center.y > s->center.y - s->height ) {
		   		p->velocity.y = -p->velocity.y;
				p->velocity.y *= (((float)rand() / (4*(float)RAND_MAX)) + 0.3);
				p->velocity.x  = (((float)rand() / (4*(float)RAND_MAX)) + 0.5);
			}
		}

		//check for collision with circle

		float squared_x = (p->s.center.x - g.circle.center.x)*(p->s.center.x - g.circle.center.x);
		float squared_y = (p->s.center.y - g.circle.center.y)*(p->s.center.y - g.circle.center.y);

		if (sqrt(squared_x +squared_y) < g.circle.radius)
		{
		    if (p->s.center.x < g.circle.center.x)
		    {
			p->velocity.y = -p->velocity.y;
			p->velocity.y *= (((float)rand() / (4*(float)RAND_MAX)) + 0.2);
			p->velocity.x -= 0.1;
		    }
		    else {
			p->velocity.y = -p->velocity.y;
			p->velocity.y *= (((float)rand() / (4*(float)RAND_MAX)) + 0.2);
			p->velocity.x += 0.1;
		    }
		}


		//check for off-screen
		if (p->s.center.y < 0.0 || p->s.center.y > 600) {
			cout << "off screen" << endl;
			g.particle[i] = g.particle[--g.n];
		}
	}
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT);


//		Rect r;
//		glClear(GL_COLOR_BUFFER_BIT);

//		r.bot =g.yres - 20 ;
//		r.left = 10;
//		r.center = 0;
//		ggprint8b(&r, 16, 0x00ff0000, "Box");
	//Draw shapes...
	//
	//draw a box
	Shape *s;
         

	glColor3ub(90,140,90);
	s = &g.box[0];
	float w, h;
	w = s->width;
	h = s->height;
	for(int i = 0; i < 5; i++) {
		glColor3ub(90,140,90);
		s = &g.box[i];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		glBegin(GL_QUADS);
			glVertex2i(-w, -h);
			glVertex2i(-w,  h);
			glVertex2i( w,  h);
			glVertex2i( w, -h);
		glEnd();
		glPopMatrix();
		Rect r;
//		glClear(GL_COLOR_BUFFER_BIT);

		r.bot = s->center.y - 5;
		r.left = s->center.x - g.shift[i];
		r.center = 0;
		ggprint8b(&r, 16, 0x00ffffff, g.waterfall[i]);
                
	}

//		Rect r;
//		glClear(GL_COLOR_BUFFER_BIT);
//
//		r.bot =  s->center.y ;
//		r.left = s->center.x;
//		r.centerx = g.xres/2;
//		r.centery = g.yres/2;
//		ggprint8b(&r, 10, 0x00ffffff, "Box");
	



//	glColor3f(


	//draw circle
	
	float  x , y  ; 

	glColor3ub(90, 140, 90); 
	glBegin(GL_TRIANGLE_FAN);
	for (int i=0; i<180; i++){
	    x = g.circle.radius * cos(i) + g.circle.center.x;
	    y = g.circle.radius * sin(i) + g.circle.center.y;
	    glVertex3f (x, y, 0);

	    x = g.circle.radius * cos(i+0.1) + g.circle.center.x;
	    y = g.circle.radius * sin(i+0.1) + g.circle.center.y;
	    glVertex3f (x, y, 0);
	}



	glEnd();


	//
	//Draw the particle here
	
//	float re, gr, bl;
//	re = 255;
//	gr = 255;
//	bl = 255;
	for(int i = 0; i < g.n; i++) {
		glPushMatrix();
		glColor3fv(g.particle[i].color);
//		re *= ((float)rand() / (float)RAND_MAX) * 1;
//		gr *= ((float)rand() / (float)RAND_MAX) * 1;
//		bl *= ((float)rand() / (float)RAND_MAX) * 1;
//		if(re < 50 && gr < 50 && bl < 50) {
//			re = 255;
//			gr = 255;
//			bl = 255;
//		}
		Vec *c = &g.particle[i].s.center;
		w =
		h = 2;
		glBegin(GL_QUADS);
			glVertex2i(c->x-w, c->y-h);
			glVertex2i(c->x-w, c->y+h);
			glVertex2i(c->x+w, c->y+h);
			glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
	//
	//Draw your 2D text here




}






