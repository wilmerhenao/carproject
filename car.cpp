#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>       /* for cos(), sin(), and sqrt() */
#include <GL/glut.h>    /* OpenGL Utility Toolkit header */
 
const int BUS_WHEEL_SPOKE_N = 36;
 
// Max and min definitions
template <class T>
inline T max(const T& a, const T& b) {return((((a) > (b)) ? (a) : (b)));}
template <class T>
inline T min(const T& a, const T& b) {return((((a) > (b)) ? (b) : (a)));}
 
// Cotangent function
template <class T>
T CoTan(const T& alpha){
  assert(fabs(cos(alpha)) < 1);  // I had a bug with strange values of sine earlier
  assert(fabs(sin(alpha)) < 1);
  return(cos(alpha)/sin(alpha));
}

/* Time varying or user-controled variables. */
static double espaciox = 0.0, espacioy = 0.0, velocidad = 0.0, dirx = 1.0, diry = 0.0, anglewheel = 0.0, theatan2 = 0.0;
long double anglewheelb = 0.0;
static float lightHeight = 20;
static unsigned long oldtime = 0; 
static GLdouble bodyWidth = 6.0;
/* Parts of the car */
static GLfloat body[][2] = { {0, 2}, {1, 1}, {2, 1}, {3, 3.2}, {5, 3.2}, {6.0, 1}, {12.0, 1}, {12.87, 3.2}, {15.1, 3.2}, {16.0, 1},
        {19, 1}, {20, 2}, {20, 4}, {19, 5},{17, 5.5}, {16, 8}, {15, 9}, {15, 10}, {14, 10}, {13, 9}, {1, 8.5}, {0, 8} };
static GLfloat antenna[][2] = { {15, 9}, {15, 12}, {14.5, 12}, {14.5, 9} };
static GLfloat turboexhaust[][2] = { {0, 2.5}, {0, 3.5}, {-1, 3.5}, {-1, 2.5} };
static GLfloat frontallight[][2] = { {19, 2.5}, {20.3, 2}, {20.3, 3} };
static GLfloat frontwindow[][2] = { {16, 5.5}, {17, 5.8}, {16.2, 8} };
static GLfloat lightPosition[4] = {0};
static GLfloat bodyColor[] = {1.0, 1.0, 0.1, 1.0}, frontColor[] = {0.0, 0.1, 1.0, 1.0}, ceilColor[] = {1.0, 0.0, 0.0, 1.0},
        wheelColor[] = {0.0, 0.0, 0.0, 1.0};
/* dims */
enum {
  X, Y, Z, W
};
enum {
  A, B, C, D
};
static GLfloat spoke_pts[BUS_WHEEL_SPOKE_N][2] = {{0.0}};
/* Enumerants for refering to display lists. */
typedef enum {
  RESERVED, BODY_SIDE, BODY_EDGE, BODY_WHOLE, ANTENNA_SIDE, ANTENNA_EDGE, ANTENNA_WHOLE,       
  TURBO_EXHAUST_SIDE, TURBO_EXHAUST_EDGE, TURBO_EXHAUST_WHOLE, EYE_SIDE, EYE_EDGE, EYE_WHOLE,   
  WHEEL_A_SIDE, WHEEL_A_EDGE, WHEEL_A_WHOLE, WHEEL_B_SIDE, WHEEL_B_EDGE, WHEEL_B_WHOLE,
  F_WINDOW_SIDE, F_WINDOW_EDGE, F_WINDOW_WHOLE} displayLists;
 
/* Floor texture tiling pattern. */ const static char *zebras[] = {
  "................",
  "......xxxx......",
  "......xxxx......",
  "......xxxx......",
  "......xxxx......",
  "......xxxx......",
  "......xxxx......",
  "......xxxx......",
  "......xxxx......",
  "......xxxx......",
  "......xxxx......",
  "......xxxx......",
  "......xxxx......",
  "................",
  "................",
  "................",
};
 
static void makeFloorTexture(void){
  GLubyte floorTexture[16][16][3];
  GLubyte *loc = (GLubyte*) floorTexture;
  int s = 0, t = 0;
 
  /* Setup RGB image for the texture. */
  assert(sizeof(zebras)/sizeof(char) <= 16 * 16); //assert that the size of the zebras is smaller
  for (t = 0; t < 16; t++) {
    for (s = 0; s < 16; s++) {
      if (zebras[t][s] == 'x') {
        /* bar */
        loc[0] = 0xff;
        loc[1] = 0xff;
        loc[2] = 0xff;
      } else {
        loc[0] = 0x8f;
        loc[1] = 0x8f;
        loc[2] = 0x8f;
      }
      loc += 3;
    }
  }
 
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, 16, 16, 0,
      GL_RGB, GL_UNSIGNED_BYTE, floorTexture);
 
  //assertions of this part
  assert(GL_INVALID_ENUM);
  assert(GL_INVALID_VALUE);
  assert(GL_INVALID_OPERATION);
}

void myownRotateFunction(GLfloat angle, GLfloat x, GLfloat y, GLfloat z){
  angle = (M_PI / 180) * angle;
  float c = cos(angle), s = sin(angle);
  GLfloat m[] = { x * x * (1-c) + c,     y * x * (1 - c) + z * s, x * z * ( 1 - c) - y * s, 0,
		  x * y * (1-c) - z * s, y * y * (1 - c) + c,     y * z * (1 - c) + x * s,  0,
		  x * z * (1-c) + y * s, y * z * (1 - c) - x * s, z * z * (1 - c) + c,      0,
                   0,                     0,                       0,                        1};
  glMultMatrixf(m);
}

void myownTranslateFunction(GLfloat x, GLfloat y, GLfloat z){
  GLfloat m[] = { 1, 0, 0, 0,
		  0, 1, 0, 0,
                  0, 0, 1, 0,
                  x, y, z, 1};
  glMultMatrixf(m);
}

void myownScaleFunction(GLfloat x, GLfloat y, GLfloat z){
  GLfloat m[] = { x, 0, 0, 0,
		  0, y, 0, 0,
                  0, 0, z, 0,
                  0, 0, 0, 1};
  glMultMatrixf(m);
}
 
// Creates an array of 2*n doubles representing n points evenly spaced around a circle of radius 1.5. and shifted 1.5 upwards (this is the wheels)
void createCircumferencePoints(int n) {
    int i = 0;
    double theta = 0.0, sintheta = 0.0, costheta = 0.0;
    //Again.  Double check sin and cos bec. I don't trust them
 
    for(i = 0; i < n; i++) {
        theta = 2 * M_PI * i / (double)n;
        sintheta = sin(theta);
        costheta = cos(theta);
        assert((1.05 > sintheta * sintheta + costheta * costheta ) && (0.95 < sintheta * sintheta + costheta * costheta) );
 
        spoke_pts[i][0] = 1.5 * costheta;
        spoke_pts[i][1] = 1.5 + 1.5 * sintheta;
    }
}
 
// Initializes data necessary for drawing the car wheel.
void wheelInit() {
  createCircumferencePoints(BUS_WHEEL_SPOKE_N);
}
 
void extrudeSolidFromPolygon(GLfloat data[][2], unsigned int dataSize,
  GLdouble thickness, GLuint side, GLuint edge, GLuint whole) {
 
  static GLUtriangulatorObj *tobj = NULL;
  GLdouble vertex[3] = {0.0}, dx = 0.0, dy = 0.0, len = 0.0;
  int i = 0;
  int count = (int) (dataSize / (2 * sizeof(GLfloat)));
  assert(count);
 
  if (tobj == NULL) {
    tobj = gluNewTess();  // create and initialize a GLU polygon tesselation object
    assert(tobj);
    gluTessCallback(tobj, GLU_TESS_BEGIN, (_GLUfuncptr)glBegin);
    gluTessCallback(tobj, GLU_TESS_VERTEX, (_GLUfuncptr)glVertex2fv);
    gluTessCallback(tobj, GLU_TESS_END, glEnd);
  }
  glNewList(side, GL_COMPILE);
  gluBeginPolygon(tobj);
  for (i = 0; i < count; i++) {
    vertex[0] = data[i][0];
    vertex[1] = data[i][1];
    vertex[2] = 0;
    gluTessVertex(tobj, vertex, data[i]);
  }
  gluEndPolygon(tobj);
  glEndList();
  glNewList(edge, GL_COMPILE);
  glShadeModel(GL_SMOOTH); //soft edges
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= count; i++) {
    // mod function handles closing the edge -> NOTICE THAT the condition here is: "<=" instead of "<"
    glVertex3f(data[i % count][0], data[i % count][1], 0.0);
    glVertex3f(data[i % count][0], data[i % count][1], thickness);
    dx = data[(i + 1) % count][1] - data[i % count][1];
    dy = data[i % count][0] - data[(i + 1) % count][0];
    len = sqrt(dx * dx + dy * dy);
    glNormal3f(dx / len, dy / len, 0.0);
  }
  glEnd();
  glEndList();
  glNewList(whole, GL_COMPILE);
  glFrontFace(GL_CW);
  glCallList(edge);
  glNormal3f(0.0, 0.0, -1.0);  /* constant normal for side */
  glCallList(side);
  glPushMatrix();
  myownTranslateFunction(0.0, 0.0, thickness);
  glFrontFace(GL_CCW);
  glNormal3f(0.0, 0.0, 1.0);  /* opposite normal for other side */
  glCallList(side);
  glPopMatrix();
  glEndList();
}
 
static void makeBus(void){
  extrudeSolidFromPolygon(body, sizeof(body), bodyWidth*1,
    BODY_SIDE, BODY_EDGE, BODY_WHOLE);
  extrudeSolidFromPolygon(antenna, sizeof(antenna), bodyWidth / 10,
    ANTENNA_SIDE, ANTENNA_EDGE, ANTENNA_WHOLE);
  extrudeSolidFromPolygon(turboexhaust, sizeof(turboexhaust), bodyWidth / 10,
    TURBO_EXHAUST_SIDE, TURBO_EXHAUST_EDGE, TURBO_EXHAUST_WHOLE);
  extrudeSolidFromPolygon(frontallight, sizeof(frontallight), bodyWidth + 0.2,
    EYE_SIDE, EYE_EDGE, EYE_WHOLE);
  extrudeSolidFromPolygon(spoke_pts, sizeof(spoke_pts), bodyWidth /6,
                          WHEEL_A_SIDE, WHEEL_A_EDGE, WHEEL_A_WHOLE);
  extrudeSolidFromPolygon(spoke_pts, sizeof(spoke_pts), -bodyWidth /6,
                          WHEEL_B_SIDE, WHEEL_B_EDGE, WHEEL_B_WHOLE);
  extrudeSolidFromPolygon(frontwindow, sizeof(frontwindow), bodyWidth - 1.0,
                          F_WINDOW_SIDE, F_WINDOW_EDGE, F_WINDOW_WHOLE);
}
 
static void drawBus(void){
  glPushMatrix();
  myownTranslateFunction(-9.0, 0.0, - bodyWidth/2); //Move the truck to the "center p"
  theatan2 = atan2(diry, dirx);
  myownRotateFunction((180 / M_PI) * theatan2, 0.0, -1.0, 0.0);
  myownTranslateFunction(espaciox, 0.0, espacioy); 
  glMaterialfv(GL_FRONT, GL_DIFFUSE, bodyColor);
  glCallList(BODY_WHOLE);

  glPushMatrix();
  myownTranslateFunction(0.0, 0.0, + 0.5 );
  glMaterialfv(GL_FRONT, GL_DIFFUSE, frontColor);
  glCallList(F_WINDOW_WHOLE); 
  glPopMatrix();

  myownTranslateFunction(0.0, 0.0, bodyWidth);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, ceilColor);
  glCallList(ANTENNA_WHOLE);
  glCallList(TURBO_EXHAUST_WHOLE);
  myownTranslateFunction(0.0, 0.0, -bodyWidth );
  glCallList(TURBO_EXHAUST_WHOLE);
  myownTranslateFunction(0.0, 0.0, 0.1);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, frontColor);
  glCallList(EYE_WHOLE);
  // this is the rear left wheel
  myownTranslateFunction(4.0, 0.0, -0.4);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, wheelColor);
  glCallList(WHEEL_A_WHOLE);
  // rear right wheel
  glPushMatrix();
  myownTranslateFunction(0.0, 0.0, bodyWidth - 0.4);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, wheelColor);
  glCallList(WHEEL_A_WHOLE);
  glPopMatrix();
  // The next is the alpha front wheel (to the right)
  glPushMatrix();
  myownTranslateFunction(10.0, 0.0, bodyWidth - 0.4);
  myownRotateFunction(anglewheel, 0.0, 1.0, 0.0);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, wheelColor);
  glCallList(WHEEL_A_WHOLE);
 
  // The next one is the beta front wheel (to the left)
  glPopMatrix();
  myownTranslateFunction(10.0, 0.0, bodyWidth / 6);
  myownRotateFunction(anglewheelb, 0.0, 1.0, 0.0);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, wheelColor);
  glCallList(WHEEL_B_WHOLE);
  glPopMatrix();
}
 
// Extension of the floor
static GLfloat floorVertices[4][3] = {
  { -200.0, 0.0, 200.0 },
  { 200.0, 0.0, 200.0 },
  { 200.0, 0.0, -200.0 },
  { -200.0, 0.0, -200.0 },
};
 
// Draw the floor
static void
drawFloor(void)
{
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D); // the floor will be 2D
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glVertex3fv(floorVertices[0]);
  glTexCoord2f(0.0, 16.0);
  glVertex3fv(floorVertices[1]);
  glTexCoord2f(16.0, 16.0);
  glVertex3fv(floorVertices[2]);
  glTexCoord2f(16.0, 0.0);
  glVertex3fv(floorVertices[3]);
  glEnd();
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
}
 
/* Advance time varying state when idle callback registered. */
static void idle(void)
{
  static unsigned long time = 0.0;
  GLdouble angularspeed = 0.0, len = 0.0;
  double dt = 0.0;

  time = glutGet(GLUT_ELAPSED_TIME);//Time since the start in miliseconds
  dt = (double)time - (double)oldtime;
  if(0 != anglewheel){
    angularspeed = (1 / 10.1) * velocidad / (10 * CoTan(anglewheel));
  }
  dirx = cos(angularspeed * dt) * dirx - sin(angularspeed * dt) * diry;
  diry = sin(angularspeed * dt) * dirx + cos(angularspeed * dt) * diry;
  len = sqrt(dirx * dirx + diry * diry);
  dirx = dirx / len;
  diry = diry / len;
  espaciox += dirx * velocidad * dt;
  espacioy += diry * velocidad * dt;
  assert(time >= oldtime);
  oldtime = time;
  glutPostRedisplay();
}

static void redraw(void){
  idle(); // do the recalculation once (to reduce flickering in my case)
  /* Clear; default stencil clears to zero. */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
 
  /* Position the light source.*/
  lightPosition[0] = 12;
  lightPosition[1] = lightHeight;
  lightPosition[2] = -15;
  lightPosition[3] = 0.0;
  glPushMatrix();
  // A rotation to make it look good
  myownRotateFunction(30, 1.0, 0.0, 0.0);
  myownRotateFunction(-150, 0.0, 1.0, 0.0);
    
   /* We can eliminate the visual "artifact" of seeing the "flipped"
            bus underneath the floor by using stencil.  The idea is
           draw the floor without color or depth update but so that
           a stencil value of one is where the floor will be.  Later when
           rendering the bus reflection, we will only update pixels
           with a stencil value of 1 to make sure the reflection only
           lives on the floor, not below the floor. */
        /* Don't update color or depth. */
  glDisable(GL_DEPTH_TEST);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
 
        /* Re-enable update of color and depth. */
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glEnable(GL_DEPTH_TEST);
 
        /* Now, only render where stencil is set to 1. */
  glStencilFunc(GL_EQUAL, 1, 0xffffffff);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
 
  glPushMatrix();
 
        /* The critical reflection step: Reflect the bus through the floor
           (the Y=0 plane) to make a relection. */
  myownScaleFunction(1.0, -1.0, 1.0);
 
        /* Reflect the light position. */
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
 
        /* To avoid our normals getting reversed and hence botched lighting
           on the reflection, turn on normalize.  */
  glEnable(GL_NORMALIZE);
  glCullFace(GL_FRONT);
 
        /* Draw the reflected bus (before the real bus). */
  drawBus();
 
        /* Disable noramlize again and re-enable back face culling. */
  glDisable(GL_NORMALIZE);
  glCullFace(GL_BACK);
 
  glPopMatrix();
 
      /* Switch back to the unreflected light position. */
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
 
  glDisable(GL_STENCIL_TEST);
 
    /* Draw "top" of floor.  Use blending to blend in reflection. */
  glEnable(GL_BLEND);
  glColor4f(1.0, 1.0, 1.0, 0.3);
  drawFloor();
  glDisable(GL_BLEND);
 
    /* Draw "actual" bus, not its reflection. */
  drawBus();
 
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_STENCIL_TEST);
 
  glPushMatrix();
  glDisable(GL_LIGHTING);
  glColor3f(1.0, 1.0, 0.0);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glPopMatrix();
  glPopMatrix();
  // do the swapping
  glutSwapBuffers();
}
 

/* When not visible, stop animating.  Restart when visible again. */
static void visible(int vis) {
  if (vis == GLUT_VISIBLE) {
      glutIdleFunc(idle);
  }
}
// When you press a special key you enter here
static void key(int c, int x, int y)
{
  switch(c){
    case GLUT_KEY_LEFT :
      anglewheel += 3.0;
      anglewheel = min(21.0, anglewheel);  //limit the torque
      break;
    case GLUT_KEY_UP :
      velocidad += 0.001;
      velocidad = min(0.01, velocidad);
      break;
    case GLUT_KEY_DOWN :   //go back
      velocidad += -0.001;
      velocidad = max(-0.01, velocidad);
      break;
    case GLUT_KEY_RIGHT :  //go forward
      anglewheel -= 3.0;
      anglewheel = max(-24.0, anglewheel);
      break;
    case GLUT_KEY_F5 :
      x = x + y; // this is for debugger only.
      break;
  }
  if(0 != anglewheel){
    anglewheelb = (180 / M_PI) * atan(10/(10 * CoTan(anglewheel * M_PI / 180) - bodyWidth));
  }
  glutPostRedisplay();

}
// When you press space you go back to original state
static void key2(unsigned char c, int x, int y)
{
  switch(c){
    case 32 :
     espaciox = 0;
     espacioy = 0;
     velocidad = 0.0;
     anglewheel = 0.0;
     anglewheelb = 0.0;
     dirx = 1.0;
     diry = 0.0;
     break;
  }
  glutPostRedisplay();
  x = x + y;
}
/*
// Mouse function
static void
mouse(int button, int state, int x, int y){
    if (button == GLUT_LEFT_BUTTON) {
	if (state == GLUT_DOWN) {
	    moving = 1;
	    startx = x;
	    starty = y;
	}
	if (state == GLUT_UP) {
	    moving = 0;
	}
    }
    if (button == GLUT_MIDDLE_BUTTON) {
	if (state == GLUT_DOWN) {
	    lightMoving = 1;
	    lightStartX = x;
	    lightStartY = y;
	}
	if (state == GLUT_UP) {
	    lightMoving = 0;
	}
    }
}
*/
int main(int argc, char **argv){
 
  glutInit(&argc, argv);
  wheelInit(); 
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
  glutInitWindowSize(900, 750);
  glutCreateWindow("Driving the bus game");
  /* Register GLUT callbacks. */
  glutDisplayFunc(redraw);
  glutVisibilityFunc(visible);
  glutSpecialFunc(key);
  glutKeyboardFunc(key2);
  makeBus();
  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* field of view in degree */ 60.0,
  /* aspect ratio */ 1.0,
    /* Z near */ 20.0, /* Z far */ 260.0);
  gluLookAt(0.0, 5.0, 55.0,  /* eye is at */
    0.0, 8.0, 0.0,      /* center is at */
    0.0, 1.0, 0.);      /* up is in postivie Y direction */
 
  glEnable(GL_LIGHT0);
  makeFloorTexture();
 
  /* Setup floor plane for projected shadow calculations. */
 
  glutMainLoop();
  return 0;
}
