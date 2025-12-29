#include "GL/freeglut.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
#include "vecmath.h"
#include <fstream>
using namespace std;

// Globals
#define MAX_BUFFER_SIZE 200

int i=0;
// Light position
GLfloat Lt0pos[] = {1.0f, 1.0f, 5.0f, 1.0f};
// This is the list of points (3D vectors)
vector<Vector3f> vecv;

// This is the list of normals (also 3D vectors)
vector<Vector3f> vecn;

// This is the list of faces (indices into vecv and vecn)
vector<vector<unsigned> > vecf;


// You will need more global variables to implement color and position changes


// These are convenience functions which allow us to call OpenGL 
// methods on Vec3d objects
inline void glVertex(const Vector3f &a) 
{ glVertex3fv(a); }

inline void glNormal(const Vector3f &a) 
{ glNormal3fv(a); }

void drawScene(void);
void keyboardFunc( unsigned char key, int x, int y );
void specialFunc( int key, int x, int y );
void reshapeFunc(int w, int h);
void initRendering();
void loadInput();
// This function is called whenever a "Normal" key press is received.
void keyboardFunc( unsigned char key, int x, int y )
{
    switch ( key )
    {
    case 27: // Escape key
        exit(0);
        break;
    case 'c':
        // add code to change color here
        i++;
		cout << "Color change c" << key << "." << endl; 
        break;
    default:
        cout << "Unhandled key press " << key << "." << endl;        
    }

	// this will refresh the screen so that the user sees the color change
    glutPostRedisplay();
}

// This function is called whenever a "Special" key press is received.
// Right now, it's handling the arrow keys.
void specialFunc( int key, int x, int y )
{
    switch ( key )
    {
    case GLUT_KEY_UP:
        // add code to change light position
        Lt0pos[1] += 0.5f;
		cout << "Unhandled key press: up arrow." << endl;
		break;
    case GLUT_KEY_DOWN:
        // add code to change light position
        Lt0pos[1] -= 0.5f;
		cout << "Unhandled key press: down arrow." << endl;
		break;
    case GLUT_KEY_LEFT:
        // add code to change light position
        Lt0pos[0] -= 0.5f;
		cout << "Unhandled key press: left arrow." << endl;
		break;
    case GLUT_KEY_RIGHT:
        // add code to change light position
        Lt0pos[0] += 0.5f;
		cout << "Unhandled key press: right arrow." << endl;
		break;
    }

	// this will refresh the screen so that the user sees the light position
    glutPostRedisplay();
}

// This function is responsible for displaying the object.
void drawScene(void)
{
    

    // Clear the rendering window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Rotate the image
    glMatrixMode( GL_MODELVIEW );  // Current matrix affects objects positions
    glLoadIdentity();              // Initialize to the identity

    // Position the camera at [0,0,5], looking at [0,0,0],
    // with [0,1,0] as the up direction.
    gluLookAt(0.0, 0.0, 5.0,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);

    // Set material properties of object

	// Here are some colors you might use - feel free to add more
    GLfloat diffColors[4][4] = { {0.5, 0.5, 0.9, 1.0},
                                 {0.9, 0.5, 0.5, 1.0},
                                 {0.5, 0.9, 0.3, 1.0},
                                 {0.3, 0.8, 0.9, 1.0} };
    
	// Here we use the first color entry as the diffuse color
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffColors[i % 4]);

	// Define specular color and shininess
    GLfloat specColor[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat shininess[] = {100.0};

	// Note that the specular color and shininess can stay constant
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
  
    // Set light properties

    // Light color (RGBA)
    GLfloat Lt0diff[] = {1.0,1.0,1.0,1.0};
    /*
    // Light position
	GLfloat Lt0pos[] = {1.0f, 1.0f, 5.0f, 1.0f};*/

    glLightfv(GL_LIGHT0, GL_DIFFUSE, Lt0diff);
    glLightfv(GL_LIGHT0, GL_POSITION, Lt0pos);

	// This GLUT method draws a teapot.  You should replace
	// it with code which draws the object you loaded.
	glBegin(GL_TRIANGLES);
    int a,b,c,d,e,f,g,h,i;

 for(unsigned int j=0; j < vecf.size(); j++) {
        
        // Since we stored 6 numbers per face, we access them in pairs
        
        // Point 1
        unsigned int v1 = vecf[j][0];
        unsigned int n1 = vecf[j][1];

        // Point 2
        unsigned int v2 = vecf[j][2];
        unsigned int n2 = vecf[j][3];

        // Point 3
        unsigned int v3 = vecf[j][4];
        unsigned int n3 = vecf[j][5];

        // Draw with Lighting!
        if(v1 < vecv.size() && n1 < vecn.size()) {
            glNormal3fv(vecn[n1]);
            glVertex3fv(vecv[v1]);
        }
        
        if(v2 < vecv.size() && n2 < vecn.size()) {
            glNormal3fv(vecn[n2]);
            glVertex3fv(vecv[v2]);
        }

        if(v3 < vecv.size() && n3 < vecn.size()) {
            glNormal3fv(vecn[n3]);
            glVertex3fv(vecv[v3]);
        }
    }

   


    
    glEnd();

    
    // Dump the image to the screen.
    glutSwapBuffers();


}

// Initialize OpenGL's rendering modes
void initRendering()
{
    glEnable(GL_DEPTH_TEST);   // Depth testing must be turned on
    glEnable(GL_LIGHTING);     // Enable lighting calculations
    glEnable(GL_LIGHT0);       // Turn on light #0.
}

// Called when the window is resized
// w, h - width and height of the window in pixels.
void reshapeFunc(int w, int h)
{
    // Always use the largest square viewport possible
    if (w > h) {
        glViewport((w - h) / 2, 0, h, h);
    } else {
        glViewport(0, (h - w) / 2, w, w);
    }

    // Set up a perspective view, with square aspect ratio
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // 50 degree fov, uniform aspect ratio, near = 1, far = 100
    gluPerspective(50.0, 1.0, 1.0, 100.0);
}

void loadInput()
{
    // Use a hardcoded file name so you don't have to pipe it in
    ifstream infile("sphere.obj"); 

    if(!infile) {
        cerr << "Error: Could not open garg.obj" << endl;
        exit(1);
    }

    char buffer[MAX_BUFFER_SIZE];
    while(infile.getline(buffer, MAX_BUFFER_SIZE))
    {
        stringstream ss(buffer);
        string s;
        ss >> s;
        
        if(s == "v"){
            Vector3f v;
            ss >> v[0] >> v[1] >> v[2];
            vecv.push_back(v);
        }
        else if(s == "vn"){
            Vector3f v;
            ss >> v[0] >> v[1] >> v[2];
            vecn.push_back(v);
        }
       else if(s == "f"){
            vector<unsigned> face;
            string vertexStr;
            
            // Loop 3 times for a triangle
            for(int i=0; i<3; i++) {
                ss >> vertexStr; 
                
                // vertexStr looks like "14//5" or "1/2/3"
                
                // 1. Get Vertex Index (Before the first slash)
                size_t firstSlash = vertexStr.find('/');
                string vIdxStr = vertexStr.substr(0, firstSlash);
                
                // 2. Get Normal Index (After the last slash)
                size_t lastSlash = vertexStr.rfind('/');
                string nIdxStr = vertexStr.substr(lastSlash + 1);

                // 3. Store both (subtract 1 because OBJ starts at 1)
                unsigned int vIndex = stoi(vIdxStr) - 1;
                unsigned int nIndex = stoi(nIdxStr) - 1;
                
                face.push_back(vIndex);
                face.push_back(nIndex);
            }
            // Now 'face' has 6 numbers: [v1, n1, v2, n2, v3, n3]
            vecf.push_back(face); 
        }
    }
    infile.close();
    cout << "Loaded " << vecf.size() << " faces." << endl;
}

// Main routine.
// Set up OpenGL, define the callbacks and start the main loop
int main( int argc, char** argv )
{
    loadInput();

    glutInit(&argc,argv);

    // We're going to animate it, so double buffer 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );

    // Initial parameters for window position and size
    glutInitWindowPosition( 60, 60 );
    glutInitWindowSize( 360, 360 );
    glutCreateWindow("Assignment 0");

    // Initialize OpenGL parameters.
    initRendering();

    // Set up callback functions for key presses
    glutKeyboardFunc(keyboardFunc); // Handles "normal" ascii symbols
    glutSpecialFunc(specialFunc);   // Handles "special" keyboard keys

     // Set up the callback function for resizing windows
    glutReshapeFunc( reshapeFunc );

    // Call this whenever window needs redrawing
    glutDisplayFunc( drawScene );

    // Start the main loop.  glutMainLoop never returns.
    glutMainLoop( );

    return 0;	// This line is never reached.
}
