#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <gl\gl.h>

HDC			hDC=NULL;		
HGLRC		hRC=NULL;		
HWND		hWnd=NULL;		
HINSTANCE	hInstance;		

bool	keys[256];			


LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int width,height;
char* title;

void init(char *_title,int _width, int _height){
		width = _width;
		height = _height;
		title = _title;
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
	glOrtho(0, width/2, height/2, 0, -1, 1); 

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 255.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	return TRUE;										// Initialization Went OK
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

BOOL CreateGLWindow()
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height


	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	

		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		16,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}


class loadTex{
	unsigned long sizeX,sizeY;
	char *data;
	int LoadImg(char *name){
		FILE *f;
		unsigned long size;
		f = fopen(name,"rb");
		if(f == NULL)
			return 0;
		fread(&sizeX,4,1,f);
		fread(&sizeY,4,1,f);
		fread(&size,4,1,f);
		data = (char*)malloc(size);
		if(data == NULL)
			return 0;
		fread(data,size,1,f);
		return 1;
	}
public:
	void LoadGLTextures(char* name , GLuint *texture) {
		if(!LoadImg(name))
			exit(0);	
	    glGenTextures(1, texture);
	    glBindTexture(GL_TEXTURE_2D, texture[0]); 
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		//free(data);
	}
}insTex;

class hero{
	int x,y; //клетки
	int numframes;
	float drawX,drawY;
	float width;
	float z;
	GLuint texture;
public:
	friend loadTex;
	hero(){
		x=0;			y=0;
		drawX = 0.0f;	drawY = 0.0f;
		width = 0.0f;
	}
	void draw(int state){
		if(state == 0)
			state=1;
		if(state > numframes)
			state=numframes;

		glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_QUADS);
		  glTexCoord2f((1.0f/numframes)*state-1, 1.0f);		glVertex3f(drawX,		drawY, z);		
		  glTexCoord2f((1.0f/numframes)*state,	  1.0f);	glVertex3f(drawX+width, drawY, z);		
		  glTexCoord2f((1.0f/numframes)*state,   0.0f);		glVertex3f(drawX+width,	drawY+width, z);		
		  glTexCoord2f((1.0f/numframes)*state-1, 0.0f);		glVertex3f(drawX,		drawY+width, z);	
		glEnd();
	}
	void init(int _x,int _y,int _numframes,float _width,float _z,char *name,float qWidth,float pX,float pY){
		x = _x;
		y = _y;
		drawX = x*qWidth+pX;
		drawY = y*qWidth+pY;
		insTex.LoadGLTextures(name,&texture);
		numframes = _numframes;
		width = _width;
		z = _z;
	}
}insHero;

class plate{
	float drawX,drawY;
	int numQuadsX,numQuadsY;
	float qWidth;
	float z;
	GLuint texture1;
	GLuint texture2;
public:
	friend loadTex;
	plate(){
		drawX = 0.0f;
		drawY = 0.0f;
	}
	void draw(){
		int i,j;
		//glBindTexture(GL_TEXTURE_2D, texture1);
		for(i=0;i<numQuadsY;i++){
			for(j=0;j<numQuadsX;j++){
				if( (i%2 != 0) && (j%2 != 0)/* && (i>0) && (j>0) */)
					glBindTexture(GL_TEXTURE_2D, texture2);
				else
					glBindTexture(GL_TEXTURE_2D, texture1);
				glBegin(GL_QUADS);
				  glTexCoord2f(0.0f, 1.0f);		glVertex2f(drawX+(qWidth*j),			drawY+qWidth+(qWidth*i)	);		
				  glTexCoord2f(1.0f, 1.0f);		glVertex2f(drawX+qWidth+(qWidth*j),		drawY+qWidth+(qWidth*i)	);		
				  glTexCoord2f(1.0f, 0.0f);		glVertex2f(drawX+qWidth+(qWidth*j),		drawY+(qWidth*i)		);		
				  glTexCoord2f(0.0f, 0.0f);		glVertex2f(drawX+(qWidth*j),			drawY+(qWidth*i)		);	
				glEnd();
			}
		}
	}
	void init(float _drawX,float _drawY,int _numQuadsX,int _numQuadsY,float _qWidth,float _z,char *name1,char *name2){
		drawX = _drawX;
		drawY = _drawY;
		numQuadsX = _numQuadsX;
		numQuadsY = _numQuadsY;
		qWidth = _qWidth;
		z = _z;
		insTex.LoadGLTextures(name1,&texture1);
		insTex.LoadGLTextures(name2,&texture2);
	}
}insPlate;

void elemInit(){
	float x,y;
	int kolY,kolX;
	float w;
	kolY = 9;
	kolX = 9;
	w = 20;
	x = (width/4)-((kolX*w)/2);
	y = (height/4)-((kolY*w)/2);
	insPlate.init(x,y,kolX,kolY,w,0.0f,"tex\\plate.bits","tex\\block.bits");
	insHero.init(3,3,1,20,0.1f,"tex\\hero.bits",20,x,y);
}
int DrawGLScene(GLvoid){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	insPlate.draw();
	insHero.draw(1);
	glLoadIdentity();									
	return TRUE;										
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	MSG		msg;								
	BOOL	done=FALSE;						

	init("window",640,480);

	if (CreateGLWindow() == FALSE){
		return 0;									
	}
	elemInit();
	while(!done){
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
			if (msg.message==WM_QUIT){
				done=TRUE;
			}else{
				TranslateMessage(&msg);				
				DispatchMessage(&msg);				
			}
		}else{
			if (keys[VK_ESCAPE]){
				done=TRUE;						
			}else{
				DrawGLScene();					
				SwapBuffers(hDC);			
			}
		}
	}

	KillGLWindow();									
	return (msg.wParam);							
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	switch (uMsg){
		case WM_CLOSE:								
		{
			PostQuitMessage(0);					
			return 0;								
		}
		case WM_KEYDOWN:							
		{
			keys[wParam] = TRUE;					
			return 0;								
		}

		case WM_KEYUP:								
		{
			keys[wParam] = FALSE;				
			return 0;								
		}

	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}