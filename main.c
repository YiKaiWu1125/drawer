#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/freeglut.h>

#define    SIZEX   800
#define    SIZEY   800

#define    MY_QUIT -1
#define    MY_CLEAR -2
#define    MY_SAVE  -3
#define    MY_BLEND -4
#define    MY_LOAD  -5
#define    UN_DO    -6
#define    RE_DO    -7

#define    WHITE   1
#define    RED     2
#define    GREEN   3
#define    BLUE    4

#define    POINT   1
#define    LINE    2
#define    POLYGON 3
#define    CIRCLE  4
#define    CURVE   5
#define    FILL_CIRCLE 6
#define    NOT_FILL_CIRCLE 7
#define    SQUARE          8
#define    FILL_SQUARE     9
#define    ERASER          10
#define    TRIANGLE        11
#define    FILL_TRIANGLE   12

typedef    int   menu_t;
menu_t     top_m, color_m, file_m, type_m;

int        height=512, width=512;
unsigned char  image[SIZEX*SIZEY][4];  /* Image data in main memory */

unsigned char  buffer[500][SIZEX * SIZEY][4];
unsigned char  redobuffer[50][SIZEX * SIZEY][4];
int cnt = 0;
int redocnt = 0;

int        pos_x=-1, pos_y=-1;
float      myColor[3]={0.0,0.0,0.0};
int        obj_type = -1;
int        first=0;      /* flag of initial points for lines and curve,..*/
int        vertex[128][2]; /*coords of vertices */
int        side=0;         /*num of sides of polygon */
float      pnt_size=1.0;
float      cycle_size = 10.0;
int        trianglearr[3][2];

/*------------------------------------------------------------
 * Callback function for display, redisplay, expose events
 * Just clear the window again
 */

void buffersave(int no) {
    int i, j;
    if (no == -1) {
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
        for (i = 0; i < width; i++)   /* Assign 0 opacity to black pixels */
            for (j = 0; j < height; j++)
                if (image[i * width + j][0] == 0 &&
                    image[i * width + j][1] == 0 &&
                    image[i * width + j][2] == 0) image[i * width + j][3] = 0;
                else image[i * width + j][3] = 127; /* Other pixels have A=127*/
    }
    else if(no == 1){
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer[cnt]);
        for (i = 0; i < width; i++)   /* Assign 0 opacity to black pixels */
            for (j = 0; j < height; j++)
                if (buffer[cnt][i * width + j][0] == 0 &&
                    buffer[cnt][i * width + j][1] == 0 &&
                    buffer[cnt][i * width + j][2] == 0) buffer[cnt][i * width + j][3] = 0;
                else buffer[cnt][i * width + j][3] = 127; /* Other pixels have A=127*/
        cnt++;
    }
    else if (no == 2) {
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, redobuffer[redocnt]);
        for (i = 0; i < width; i++)   // Assign 0 opacity to black pixels //
            for (j = 0; j < height; j++)
                if (redobuffer[redocnt][i * width + j][0] == 0 &&
                    redobuffer[redocnt][i * width + j][1] == 0 &&
                    redobuffer[redocnt][i * width + j][2] == 0) redobuffer[redocnt][i * width + j][3] = 0;
                else redobuffer[redocnt][i * width + j][3] = 127;
        redocnt++;
    }
}
void bufferload(int no) {
    if (no == -1) {
        glRasterPos2i(0, 0);
        glDrawPixels(width, height,
            GL_RGBA, GL_UNSIGNED_BYTE,
            image);
    }
    else if(no == 1){
        if (cnt > 1) {
            cnt--;
        }
        glRasterPos2i(0, 0);
        glDrawPixels(width, height,
            GL_RGBA, GL_UNSIGNED_BYTE,
            buffer[cnt-1]);
    }
    else if(no == 2){
        if (redocnt == 0)return ;
        if (redocnt > 1)redocnt--;
            glRasterPos2i(0, 0);
            glDrawPixels(width, height,
                GL_RGBA, GL_UNSIGNED_BYTE,
                redobuffer[redocnt - 1]);
    }
}



void display_func(void)
{
  //printf("go in display\n");
    
  /* define window background color */
  //glClear(GL_COLOR_BUFFER_BIT);
  glFlush();
}


/*-------------------------------------------------------------
 * reshape callback function for window.
 */
void my_reshape(int new_w, int new_h)
{
  height = new_h;
  width = new_w;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, (double) width, 0.0, (double) height);
  glViewport(0,0,width,height);
  glMatrixMode(GL_MODELVIEW);

  glutPostRedisplay();   /*---Trigger Display event for redisplay window*/
}


/*--------------------------------------------------------------
 * Callback function for keyboard event.
 * key = the key pressed,
 * (x,y)= position in the window, where the key is pressed.
 */
int keyboardlastx = -1;
int keyboardlasty = -1;
int nowkeyboardlastx = -1;
int nowkeyboardlasty = -1;
void keyboard(unsigned char key, int x, int y)
{
  if (key == (char)(25)) {
      bufferload(2);
      buffersave(1);
  }
  else if (key == (char)(26)) {
      buffersave(2);
      bufferload(1);
  }
  else if(key=='Q'||key=='q') exit(0);
  else {
      if (x == keyboardlastx && y == keyboardlasty) {
          glRasterPos2i(nowkeyboardlastx, height - nowkeyboardlasty);
          glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, key);
          nowkeyboardlastx += 15;
      }
      else {
          glRasterPos2i(x, height - y);
          glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, key);
          keyboardlastx = x;
          keyboardlasty = y;
          nowkeyboardlastx = x + 15;
          nowkeyboardlasty = y ;
      }
      
  }
  glFlush();
}


/*---------------------------------------------------------
 * Procedure to draw a polygon
 */
void draw_polygon()
{
  int  i;
  //glBegin((1) ? GL_POLYGON : GL_LINE_LOOP);
  glBegin(GL_POLYGON);
  for (i = 0; i < side; i++)
      glVertex2f(vertex[i][0], height - vertex[i][1]);
  glEnd();
  glFinish();
  side = 0;    /* set side=0 for next polygon */
}



/*------------------------------------------------------------
 * Procedure to draw a circle
 */
void fill_draw_circle()
{
  static GLUquadricObj *mycircle=NULL;

  if(mycircle==NULL){
    mycircle = gluNewQuadric();
    gluQuadricDrawStyle(mycircle,GLU_FILL);
  }
  glPushMatrix();
  glTranslatef(pos_x, height-pos_y, 0.0);
  gluDisk(mycircle,
       0.0,           /* inner radius=0.0 */
	  cycle_size,          /* outer radius=10.0 */
	  16,            /* 16-side polygon */
	   3);
  glPopMatrix();
}
void not_fill_draw_circle()
{
    static GLUquadricObj* mycirclee = NULL;

    if (mycirclee == NULL) {
        mycirclee = gluNewQuadric();
        gluQuadricDrawStyle(mycirclee, GLU_FILL);
    }
    glPushMatrix();
    glTranslatef(pos_x, height - pos_y, 0.0);
    gluDisk(mycirclee,
        cycle_size-pnt_size,           /* inner radius=0.0 */
        cycle_size,          /* outer radius=10.0 */
        16,            /* 16-side polygon */
        3);
    glPopMatrix();
}



/*------------------------------------------------------------
 * Callback function handling mouse-press events
 */

void mouse_func(int button, int state, int x, int y)
{
  if(button!=GLUT_LEFT_BUTTON||state!=GLUT_DOWN)
    return;
  redocnt = 0;
  switch(obj_type){
  case POINT:
    glPointSize(pnt_size);     /*  Define point size */
    glBegin(GL_POINTS);     /*  Draw a point */
       glVertex2f(x, height-y);
    glEnd();
    break;
  case LINE:
    if(first==0){
      first = 1;
      pos_x = x; pos_y = y;
	  glPointSize(pnt_size);
      glBegin(GL_POINTS);   /*  Draw the 1st point */
	    glVertex3f(x, height-y, 0);
      glEnd();
    }else{
      first=0;
      glLineWidth(pnt_size);     /* Define line width */
      glBegin(GL_LINES);    /* Draw the line */
        glVertex2f(pos_x, height - pos_y);
	    glVertex2f(x, height - y);
      glEnd();
    }
    break;
  case POLYGON:  /* Define vertices of poly */
      if (side == 0) {
          vertex[side][0] = x; vertex[side][1] = y;
          side++;
      }
      else {
          if (fabs(vertex[side - 1][0] - x) + fabs(vertex[side - 1][1] - y) < 2)
              draw_polygon();
          else {
              glBegin(GL_LINES);
              glVertex2f(vertex[side - 1][0], height - vertex[side - 1][1]);
              glVertex2f(x, height - y);
              glEnd();
              vertex[side][0] = x;
              vertex[side][1] = y;
              side++;
          }
      }
      break;
  case FILL_CIRCLE:
    pos_x = x; pos_y = y;
    fill_draw_circle();
    break;
  case NOT_FILL_CIRCLE:
      pos_x = x; pos_y = y;
      not_fill_draw_circle();
      break;
  case SQUARE:
      if (first == 0) {
          first = 1;
          pos_x = x; pos_y = y;
          glPointSize(pnt_size);
          glBegin(GL_POINTS);   /*  Draw the 1st point */
          glVertex3f(x, height - y, 0);
          glEnd();
      }
      else {
          first = 0;
          glLineWidth(pnt_size);     /* Define line width */
          glBegin(GL_LINES);    /* Draw the line */
          glVertex2f(pos_x, height - y);
          glVertex2f(x, height - y);
          glEnd();
          glBegin(GL_LINES);    /* Draw the line */
          glVertex2f(pos_x, height - pos_y);
          glVertex2f(x, height - pos_y);
          glEnd();
          glBegin(GL_LINES);    /* Draw the line */
          glVertex2f(x, height - pos_y);
          glVertex2f(x, height - y);
          glEnd();
          glBegin(GL_LINES);    /* Draw the line */
          glVertex2f(pos_x, height - pos_y);
          glVertex2f(pos_x, height - y);
          glEnd();
      }
      break;
  case FILL_SQUARE:
      if (first == 0) {
          first = 1;
          pos_x = x; pos_y = y;
          glPointSize(pnt_size);
          glBegin(GL_POINTS);   /*  Draw the 1st point */
          glVertex3f(x, height - y, 0);
          glEnd();
      }
      else {
          first = 0;
          glLineWidth(pnt_size);     /* Define line width */
          for (int i = min(pos_x, x); i <= max(pos_x, x); i++) {
              glBegin(GL_LINES);    /* Draw the line */
              glVertex2f(i, height - pos_y);
              glVertex2f(i, height - y);
              glEnd();
          }
      }
      break;
  case FILL_TRIANGLE:
      if (first <2) {
          //pos_x = x; pos_y = y;
          trianglearr[first][0] = x;
          trianglearr[first][1] = y;
          glPointSize(pnt_size);
          glBegin(GL_POINTS);   /*  Draw the 1st point */
          glVertex3f(x, height - y, 0);
          glEnd();
          first++;
      }
      else{
          trianglearr[first][0] = x;
          trianglearr[first][1] = y;
          first = 0;
          glLineWidth(pnt_size);     /* Define line width */
          glBegin(GL_POLYGON);    // Draw the line GL_POLYGON
          glVertex2f(trianglearr[0][0], height - trianglearr[0][1]);
          glVertex2f(trianglearr[1][0], height - trianglearr[1][1]);
          //glEnd();
          //glBegin(GL_LINES);    /* Draw the line */
          glVertex2f(trianglearr[2][0], height - trianglearr[2][1]);
          glVertex2f(trianglearr[1][0], height - trianglearr[1][1]);
          //glEnd();
          //glBegin(GL_LINES);    /* Draw the line */
          glVertex2f(trianglearr[0][0], height - trianglearr[0][1]);
          glVertex2f(trianglearr[2][0], height - trianglearr[2][1]);
          glEnd();
      }
      break;
  case TRIANGLE:
      if (first < 2) {
          //pos_x = x; pos_y = y;
          trianglearr[first][0] = x;
          trianglearr[first][1] = y;
          glPointSize(pnt_size);
          glBegin(GL_POINTS);   /*  Draw the 1st point */
          glVertex3f(x, height - y, 0);
          glEnd();
          first++;
      }
      else {
          trianglearr[first][0] = x;
          trianglearr[first][1] = y;
          first = 0;
          glLineWidth(pnt_size);     /* Define line width */
          glBegin(GL_LINES);    // Draw the line GL_POLYGON
          glVertex2f(trianglearr[0][0], height - trianglearr[0][1]);
          glVertex2f(trianglearr[1][0], height - trianglearr[1][1]);
          //glEnd();
          //glBegin(GL_LINES);    /* Draw the line */
          glVertex2f(trianglearr[2][0], height - trianglearr[2][1]);
          glVertex2f(trianglearr[1][0], height - trianglearr[1][1]);
          //glEnd();
          //glBegin(GL_LINES);    /* Draw the line */
          glVertex2f(trianglearr[0][0], height - trianglearr[0][1]);
          glVertex2f(trianglearr[2][0], height - trianglearr[2][1]);
          glEnd();
      }
      break;
  default:
      break;
  }
  buffersave(1);
  glFinish();
}


    
/*-------------------------------------------------------------
 * motion callback function. The mouse is pressed and moved.
 */

void motion_func(int  x, int y)
{
    if (obj_type != CURVE) return;
    if (first == 0) {
        first = 1;
        pos_x = x; pos_y = y;
    }
    else {
        glBegin(GL_LINES);
        glVertex3f(pos_x, height - pos_y, 0.0);
        glVertex3f(x, height - y, 0.0);
        glEnd();
        pos_x = x; pos_y = y;
    }
    glFinish();
}

/*--------------------------------------------------------
 * procedure to clear window
 */
void init_window(void)
{
  /*Do nothing else but clear window to black*/

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, (double) width, 0.0, (double) height);
  glViewport(0,0,width, height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  buffersave(1);
  glFlush();
}


/*------------------------------------------------------
 * Procedure to initialize data alighment and other stuff
 */
void init_func()
{   glReadBuffer(GL_FRONT);
    glDrawBuffer(GL_FRONT);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

/*-----------------------------------------------------------------
 * Callback function for color menu
 */
void  color_func(int value)
{
  switch(value){
  case WHITE:
    myColor[0] = myColor[1] = myColor[2] = 1.0;
    break;

  case RED:
    myColor[0] = 1.0;
    myColor[1] = myColor[2] = 0.0;
    break;

  case GREEN:
    myColor[0] = myColor[2] = 0.0;
    myColor[1] = 1.0;
    break;
    
  case BLUE:
    myColor[0] = myColor[1] = 0.0;
    myColor[2] = 1.0;
    break;

  default:
    break;
  }
  glColor3f(myColor[0], myColor[1], myColor[2]);
}


/*------------------------------------------------------------
 * Callback function for top menu.
 */
void file_func(int value)
{ 
  int i, j;

  if(value==MY_QUIT) exit(0);
  else if(value==MY_CLEAR) init_window();
  else if(value==MY_SAVE){ /* Save current window */
      buffersave(-1);
  }
  else if(value==MY_LOAD){ /* Restore the saved image */
      bufferload(-1);
  }
  else if(value==MY_BLEND){ /* Blending current image with the saved image */	
	  glEnable(GL_BLEND); 
	  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glRasterPos2i(0, 0);
      glDrawPixels(width, height, 
	       GL_RGBA, GL_UNSIGNED_BYTE, 
	       image);
	  glDisable(GL_BLEND);
  }
  else if (value == RE_DO) {
      buffersave(2);
      bufferload(1);
  }
  else if (value == UN_DO) {
      bufferload(2);
      buffersave(1);
  }
  glFlush();
}

void size_func(int value)
{
	if(value==1){
        if (pnt_size < 5.0) {
            pnt_size += 1.0;
            cycle_size += 4.0;
        }
      
	}else{
        if (pnt_size > 1.0) {
            pnt_size = pnt_size - 1.0;
            cycle_size -= 3.0;
        }
	}
}

/*---------------------------------------------------------------
 * Callback function for top menu. Do nothing.
 */
void top_menu_func(int value)
{
}


/*-------------------------------------------------------------
 * Callback Func for type_m, define drawing object
 */
void draw_type(int value)
{
  obj_type = value;
  if (value == LINE || value == CURVE)
      first = 0; //printf("go in here\n");
  else if (value == POLYGON) side = 0;// printf("go in here\n");
}


/*---------------------------------------------------------------
 * Main procedure sets up the window environment.
 */
int main(int argc, char **argv)
{
  int  size_menu;

  glutInit(&argc, argv);    /*---Make connection with server---*/

  glutInitWindowPosition(0,0);  /*---Specify window position ---*/
  glutInitWindowSize(width, height); /*--Define window's height and width--*/

  glutInitDisplayMode(GLUT_SINGLE|GLUT_RGBA); /*---set display mode---*/
  init_func();

 
   /* Create parent window */
  glutCreateWindow("Menu"); 

  glutDisplayFunc(display_func); /* Associate display event callback func */
  glutReshapeFunc(my_reshape);  /* Associate reshape event callback func */
  glutKeyboardFunc(keyboard); /* Callback func for keyboard event */
  glutMouseFunc(mouse_func);  /* Mouse Button Callback func */
  glutMotionFunc(motion_func);/* Mouse motion event callback func */
  //glutMotionFunc(eraser_motion_func);

  color_m = glutCreateMenu(color_func); /* Create color-menu */
  glutAddMenuEntry("white", WHITE);
  glutAddMenuEntry("red", RED);
  glutAddMenuEntry("green", GREEN);
  glutAddMenuEntry("blue", BLUE);

  file_m = glutCreateMenu(file_func);   /* Create another menu, file-menu */
  glutAddMenuEntry("save", MY_SAVE);
  glutAddMenuEntry("load", MY_LOAD);
  glutAddMenuEntry("blend", MY_BLEND);
  glutAddMenuEntry("clear", MY_CLEAR);
  glutAddMenuEntry("quit", MY_QUIT);
  glutAddMenuEntry("redo", RE_DO);
  glutAddMenuEntry("undo", UN_DO);

  type_m = glutCreateMenu(draw_type);   /* Create draw-type menu */
  glutAddMenuEntry("Point", POINT);
  glutAddMenuEntry("Line", LINE);
  glutAddMenuEntry("Curve", CURVE);
  glutAddMenuEntry("HollowCircle", NOT_FILL_CIRCLE);
  glutAddMenuEntry("HollowSquare", SQUARE);
  glutAddMenuEntry("HollowTriangle", TRIANGLE);
  glutAddMenuEntry("Poly", POLYGON);
  glutAddMenuEntry("FillCircle", FILL_CIRCLE);
  glutAddMenuEntry("FillSquare", FILL_SQUARE);
  glutAddMenuEntry("FillTriangle", FILL_TRIANGLE);

  //glutAddMenuEntry("Eraser", ERASER);
  //glutAddMenuEntry("Circle", CIRCLE);
  //glutAddMenuEntry("Circle", CIRCLE);


  size_menu = glutCreateMenu(size_func);
  glutAddMenuEntry("Bigger", 1);
  glutAddMenuEntry("Smaller",2);

  top_m = glutCreateMenu(top_menu_func);/* Create top menu */
  glutAddSubMenu("colors", color_m);    /* add color-menu as a sub-menu */
  glutAddSubMenu("type", type_m);
  glutAddSubMenu("Size", size_menu);
  glutAddSubMenu("file", file_m);       /* add file-menu as a sub-menu */
  glutAttachMenu(GLUT_RIGHT_BUTTON);    /* associate top-menu with right but*/

  /*---Test whether overlay support is available --*/
  if(glutLayerGet(GLUT_OVERLAY_POSSIBLE)){
    fprintf(stderr,"Overlay is available\n");
  }else{
    fprintf(stderr,"Overlay is NOT available, May encounter problems for menu\n");
  }
  /*---Enter the event loop ----*/
  glutMainLoop();       
}
