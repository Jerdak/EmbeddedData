
#include "oglManager.h"
#include <math.h>
#include <algorithm>
#include <functional>     
#include <EventTimer/EventTimer.h>

GLfloat	rtri = 0.0;				// Angle For The Triangle 
GLuint	texture[1];				// Storage For One Texture ( NEW )
int wiggle_count = 0;		// Counter Used To Control How Fast Flag Waves

const float piover180 = 0.0174532925f;
float timeDiff = 0.0;
int timeDir = -1;
NSC::Common::EventTimer timer;

GLfloat speed			= 10.0f;			// Movement speed
GLfloat	yrot			= 0.0f;				// Y Rotation
GLfloat	z				= 0.0f;				// Depth Into The Screen

std::vector<vTrack> points, base_points;
float flag[45][45][3];    // The Array For The Points On The Grid Of Our "Wave"

AUX_RGBImageRec *LoadBMP(char *Filename)				// Loads A Bitmap Image
{
	FILE *file=NULL;									// File Handle

	if (!Filename)										// Make Sure A Filename Was Given
	{
		printf("Couldn't find bitmap file %s\n",Filename);
		return NULL;									// If Not Return NULL
	}

	fopen_s(&file,Filename,"r");							// Check To See If The File Exists

	if (file)											// Does The File Exist?
	{
		fclose(file);									// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}

	return NULL;										// If Load Failed Return NULL
}
int LoadGLTextures()									// Load Bitmaps And Convert To Textures
{
	int Status=FALSE;									// Status Indicator

	AUX_RGBImageRec *TextureImage[1];					// Create Storage Space For The Texture

	memset(TextureImage,0,sizeof(void *)*1);           	// Set The Pointer To NULL

	// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
	if (TextureImage[0]=LoadBMP("test.bmp"))
	{
		Status=TRUE;									// Set The Status To TRUE

		glGenTextures(1, &texture[0]);					// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

		printf("Texture loaded from test.bmp\n");
	} else {
		printf("Failed to load texture\n");
	}

	if (TextureImage[0])									// If Texture Exists
	{
		if (TextureImage[0]->data)							// If Texture Image Exists
		{
			free(TextureImage[0]->data);					// Free The Texture Image Memory
		}

		free(TextureImage[0]);								// Free The Image Structure
	}

	return Status;										// Return The Status
}

void add_vertex(float x,float y,float z){
	points.push_back(vTrack(vertex(x,y,z)));
	base_points.push_back(vTrack(vertex(x,y,z)));
}
void add_color(int idx, float x,float y,float z){
	points[idx].color = vertex(x,y,z);
	base_points[idx].color = vertex(x,y,z);
}

///Hit sort predicate for downward falling points
bool height_predicate_down ( vTrack v1, vTrack v2 )
{
   return v1.base.y < v2.base.y;
}

///Hit sort predicate for upward rising points
bool height_predicate_up( vTrack v1, vTrack v2 )
{
   return v1.base.y > v2.base.y;
}

void get_range(vertex &max,vertex &min){
	///Find data range
	for(size_t i = 0; i < points.size(); i++){
		if(i==0){
			max = points[i].base;
			min = points[i].base;
		} else {
			if(points[i].base.x > max.x)max.x = points[i].base.x;
			if(points[i].base.y > max.y)max.y = points[i].base.y;
			if(points[i].base.z > max.z)max.z = points[i].base.z;

			if(points[i].base.x < min.x)min.x = points[i].base.x;
			if(points[i].base.y < min.y)min.y = points[i].base.y;
			if(points[i].base.z < min.z)min.z = points[i].base.z;
		}
	}
}
void normalize_vertices(){
	vertex max, min, center;
	get_range(max,min);

	center = (max + min)/2.0f;
	
	///Translate object to origin
	for(size_t i = 0; i < points.size(); i++){
		points[i].base = points[i].base - center;
	}

	max = max - center;
	min = min - center;
	
	///Normalize by max height value
	float maxVal = 0.0f;
	if(max.x > max.y && max.y > max.z)maxVal = max.x;
	else if(max.y > max.z)maxVal = max.y;
	else maxVal = max.z;

	///Scale
	for(size_t i = 0; i < points.size(); i++){
		points[i].base = points[i].base / maxVal;
		points[i].original = points[i].base;
	}
	
}
#include <time.h>


void sort_by_height(int dir){
	
	
	srand( (unsigned)::time( NULL ) );

	if(dir==1)std::sort(points.begin(),points.end(),height_predicate_up);
	else if(dir==-1)std::sort(points.begin(),points.end(),height_predicate_down);

	
	vertex max, min;
	get_range(max,min);

	for(size_t i = 0; i < points.size(); i++){
		float offset = (float)(rand() % 100) / 100;//(float)i/points.size()*2;
		float time_range = 3.0;
		if(dir==1){	///points going up
			points[i].dir = vertex(0,1,0);
			points[i].time_base = 0;
			points[i].delay = (offset*3);
			points[i].time_max = time_range;
		} else {	///Points going down
			points[i].dir = vertex(0,-1,0);
			points[i].time_base = -time_range;
			points[i].delay = (offset*3);
			points[i].time_max = 0.0f;
		}
	}
}

///Frustum from Mesa3D
void frustum(GLdouble left, GLdouble right,
        GLdouble bottom, GLdouble top, 
        GLdouble nearval, GLdouble farval)
{
   GLdouble x, y, a, b, c, d;
   GLdouble m[16];

   x = (2.0 * nearval) / (right - left);
   y = (2.0 * nearval) / (top - bottom);
   a = (right + left) / (right - left);
   b = (top + bottom) / (top - bottom);
   c = -(farval + nearval) / ( farval - nearval);
   d = -(2.0 * farval * nearval) / (farval - nearval);

#define M(row,col)  m[col*4+row]
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M

   glMultMatrixd(m);
}

///Perspective from Mesa3D
void myPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
   GLdouble xmin, xmax, ymin, ymax;

   ymax = zNear * tan(fovy * 3.141592654 / 360.0);
   ymin = -ymax;
   xmin = ymin * aspect;
   xmax = ymax * aspect;

   /* don't call glFrustum() because of error semantics (covglu) */
   frustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	myPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);


	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	return TRUE;										// Initialization Went OK
}

int Update(HDC hDC)									// Here's Where We Do All The Drawing
{ 
	timer.AfterEvent();
	float dur = timer.GetTimeSinceLastEvent();
	float dt = 0.01f;//speed * dur; //0.01f;
	//printf("Dur: %f  Dt: %f\n",dur,dt);
	//printf("Time: %f  Pts: %d\n",time,points.size());
	bool flip = true;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();									// Reset The Current Modelview Matrix
	glPushMatrix();
	{	///Draw mesh points
		glTranslatef(0, 0, -3.0f);
		glRotatef(rtri,0.0f,1.0f,0.0f);
		glPointSize(8.0);
		glBegin(GL_POINTS);
		for(size_t i = 0; i < points.size(); i++){
			vertex pt =  points[i].get_vertex(dt);
			vertex c = points[i].color;
			
			glColor3f(c.x,c.y,c.z);
			glVertex3f( pt.x, pt.y, pt.z);	

			if(points[i].time_base != points[i].time_max)flip = false;
		}
		glEnd();
		glPopMatrix();
	}
	
	{	///Show origin as a red point
		glPointSize(15.0);
			glBegin(GL_POINTS);
			glColor3f(1,0,0);
			glVertex3f( 0,0,0);	
		glEnd();
		
	}
	glPopMatrix();
	glPushMatrix();

	{	///Draw texture
		glLoadIdentity();	
		glEnable(GL_TEXTURE_2D); 
		glColor3f(1,1,1);
		glTranslatef(0.0f,0.0f,-12.0f);
		glBindTexture(GL_TEXTURE_2D, texture[0]);

		glBegin(GL_QUADS);
		for(int x = 0; x < 44; x++ )
		{
			for(int  y = 0; y < 44; y++ )
			{
				float float_x, float_y, float_xb, float_yb;
				float_x = float(x)/44.0f;
				float_y = float(y)/44.0f;
				float_xb = float(x+1)/44.0f;
				float_yb = float(y+1)/44.0f;

				glTexCoord2f( float_x, float_y);
				glVertex3f( flag[x][y][0], flag[x][y][1], flag[x][y][2] );

				glTexCoord2f( float_x, float_yb );
				glVertex3f( flag[x][y+1][0], flag[x][y+1][1], flag[x][y+1][2] );

				glTexCoord2f( float_xb, float_yb );
				glVertex3f( flag[x+1][y+1][0], flag[x+1][y+1][1], flag[x+1][y+1][2] );

				glTexCoord2f( float_xb, float_y );
				glVertex3f( flag[x+1][y][0], flag[x+1][y][1], flag[x+1][y][2] );
			}
		}
		glEnd();

		if( wiggle_count == 2 )
		{
			float tmp;
			for(int y = 0; y < 45; y++ )
			{
				tmp=flag[0][y][2];
				for(int x = 0; x < 44; x++)
				{
					flag[x][y][2] = flag[x+1][y][2];
				}
				flag[44][y][2]=tmp;
			}
			wiggle_count = 0;
		}

		wiggle_count++;
		glDisable(GL_TEXTURE_2D);
	
	}	///end draw texture
	glPopMatrix();

	///Rotate scene
	rtri+=0.3f;	

	//Flush and swap
	glFlush();
	if(!SwapBuffers(hDC)){
		DWORD err = GetLastError();
		printf("  - failed to swap buffers: %d\n",err);
	}

	///Flip data fall direction if maximum range met on all points and we exceed the wait duration
	if(flip){
		timeDiff += dt;
		///Wait a small bit to allow object to rotate before resetting falling effect
		if(timeDiff > 1.0f) {
			timeDir *= -1;
			sort_by_height(timeDir);
			timeDiff = 0;
		}
	}
	timer.BeforeEvent();
	return TRUE;										// Keep Going
}
GLvoid Kill(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hRC );
	ReleaseDC( hWnd, hDC );
}

BOOL Init(HWND hWnd, HDC &hDC, HGLRC &hRC){
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory( &pfd, sizeof pfd );
	pfd.nSize = sizeof pfd;
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;

	hDC = GetDC( hWnd );

	int i = ChoosePixelFormat( hDC, &pfd );  
	SetPixelFormat( hDC, i, &pfd );

	hRC = wglCreateContext( hDC );
	wglMakeCurrent( hDC, hRC );

	InitGL();
	
	RECT rect;
	GetClientRect( hWnd, &rect );
    int width = rect.right;         
    int height = rect.bottom;

	ReSizeGLScene(width,height);

	for(int x=0; x<45; x++)
	{
		for(int y=0; y<45; y++)
		{
			flag[x][y][0]=float((x/5.0f)-4.5f);
			flag[x][y][1]=float((y/5.0f)-4.5f);
			flag[x][y][2]=float(sin((((x/5.0f)*40.0f)/360.0f)*3.141592654*2.0f));
		}
	}
	LoadGLTextures();

	timer.BeforeEvent();
	return TRUE;	
}