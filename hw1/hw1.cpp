/*
    CSCI 420 Computer Graphics, Computer Science, USC
    Assignment 1: Height Fields with Shaders.
    C/C++ starter code

    Student username: Shidong Zhang
*/

#include "openGLHeader.h"
#include "glutHeader.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "pipelineProgram.h"
#include "vbo.h"
#include "vao.h"
#include "ebo.h"

#include <iostream>
#include <cstring>

#if defined(WIN32) || defined(_WIN32)
    #ifdef _DEBUG
        #pragma comment(lib, "glew32d.lib")
    #else
        #pragma comment(lib, "glew32.lib")
    #endif
#endif

#if defined(WIN32) || defined(_WIN32)
    char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
    char shaderBasePath[1024] = "../openGLHelper";
#endif

using namespace std;

int mousePos[2]; // x,y screen coordinates of the current mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

int renderType=1;

bool enableCameraMov=false;
float speed[2]={0.0,0.0};
float eyeVec[3];
float focusVec[3];
float upVec[3];

// Transformations of the terrain.
float terrainRotate[3] = { 0.0f, 0.0f, 0.0f }; 
// terrainRotate[0] gives the rotation around x-axis (in degrees)
// terrainRotate[1] gives the rotation around y-axis (in degrees)
// terrainRotate[2] gives the rotation around z-axis (in degrees)
float terrainTranslate[3] = { 0.0f, 0.0f, 0.0f };
float terrainScale[3] = { 1.0f, 1.0f, 1.0f };

// Width and height of the OpenGL window, in pixels.
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 Homework 1";

// Stores the image loaded from disk.
ImageIO * heightmapImage;

// Number of vertices in the single triangle (starter code).
int numVerticesPoint;
int numVerticesLine;
int numVerticesTriangle;

// CSCI 420 helper classes.
OpenGLMatrix matrix;
PipelineProgram * pipelineProgram = nullptr;
VBO * vboVerticesPoint = nullptr;
VBO * vboColorsPoint = nullptr;
EBO * eboPoint = nullptr;
VAO * vaoPoint = nullptr;

VBO * vboVerticesLine = nullptr;
VBO * vboColorsLine = nullptr;
EBO * eboLine = nullptr;
VAO * vaoLine = nullptr;

VBO * vboVerticesTriangle = nullptr;
VBO * vboColorsTriangle = nullptr;
EBO * eboTriangle = nullptr;
VAO * vaoTriangle = nullptr;

// Write a screenshot to the specified filename.
void saveScreenshot(const char * filename)
{
    unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
    glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

    ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

    if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
        cout << "File " << filename << " saved successfully." << endl;
    else cout << "Failed to save file " << filename << '.' << endl;

    delete [] screenshotData;
}

int frameCount = 0;
double lastTime = 0;
double fps = 0;

void idleFunc()
{
    // Do some stuff... 
    // For example, here, you can save the screenshots to disk (to make the animation).

    // Notify GLUT that it should call displayFunc.
    double currentTime = glutGet(GLUT_ELAPSED_TIME) * 0.001;
    ++frameCount;

    double timeInterval = currentTime - lastTime;

    if (timeInterval > 1.0) {
        fps = frameCount / timeInterval;
        cout<<"FPS: "<<fps;

        frameCount = 0;

        cout<<"\nTranslate ";
        for(int i=0;i<3;i++) cout<<terrainTranslate[i]<<" ";cout<<" ｜ Rotate ";
        for(int i=0;i<3;i++) cout<<terrainRotate[i]<<" ";cout<<" | Scale ";
        for(int i=0;i<3;i++) cout<<terrainScale[i]<<" ";cout<<"\n";

        lastTime = currentTime;
    }
    
    glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
    glViewport(0, 0, w, h);

    // When the window has been resized, we need to re-set our projection matrix.
    matrix.SetMatrixMode(OpenGLMatrix::Projection);
    matrix.LoadIdentity();
    // You need to be careful about setting the zNear and zFar. 
    // Anything closer than zNear, or further than zFar, will be culled.
    const float zNear = 0.1f;
    const float zFar = 10000.0f;
    const float humanFieldOfView = 60.0f;
    matrix.Perspective(humanFieldOfView, 1.0f * w / h, zNear, zFar);
}

void mouseMotionDragFunc(int x, int y)
{
    // Mouse has moved, and one of the mouse buttons is pressed (dragging).

    // the change in mouse position since the last invocation of this function
    int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

    switch (controlState)
    {
        // translate the terrain
        case TRANSLATE:
            if (leftMouseButton)
            {
                // control x,y translation via the left mouse button
                terrainTranslate[0] += mousePosDelta[0] * 0.01f;
                terrainTranslate[1] -= mousePosDelta[1] * 0.01f;
            }
            if (middleMouseButton)
            {
                // control z translation via the middle mouse button
                terrainTranslate[2] += mousePosDelta[1] * 0.01f;
            }
            break;

        // rotate the terrain
        case ROTATE:
            if (leftMouseButton)
            {
                // control x,y rotation via the left mouse button
                terrainRotate[0] += mousePosDelta[1];
                terrainRotate[1] += mousePosDelta[0];
            }
            if (middleMouseButton)
            {
                // control z rotation via the middle mouse button
                terrainRotate[2] += mousePosDelta[1];
            }
            break;

        // scale the terrain
        case SCALE:
            if (leftMouseButton)
            {
                // control x,y scaling via the left mouse button
                terrainScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
                terrainScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
            }
            if (middleMouseButton)
            {
                // control z scaling via the middle mouse button
                terrainScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
            }
            break;
    }

    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
    // Mouse has moved.
    // Store the new mouse position.
    mousePos[0] = x;
    mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
    // A mouse button has has been pressed or depressed.

    // Keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables.
    switch (button)
    {
        case GLUT_LEFT_BUTTON:
            leftMouseButton = (state == GLUT_DOWN);
        break;

        case GLUT_MIDDLE_BUTTON:
            middleMouseButton = (state == GLUT_DOWN);
        break;

        case GLUT_RIGHT_BUTTON:
            rightMouseButton = (state == GLUT_DOWN);
        break;
    }

    // Keep track of whether CTRL and SHIFT keys are pressed.
    switch (glutGetModifiers())
    {
        case GLUT_ACTIVE_CTRL:
            controlState = TRANSLATE;
        break;

        case GLUT_ACTIVE_SHIFT:
            controlState = SCALE;
        break;

        // If CTRL and SHIFT are not pressed, we are in rotate mode.
        default:
            #ifdef __APPLE__
                // nothing is needed on Apple
                if(controlState==SCALE) controlState=ROTATE;
            #else
                // Windows, Linux
                controlState = ROTATE;
            #endif
            
        break;
    }

    // Store the new mouse position.
    mousePos[0] = x;
    mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27: // ESC key
            exit(0); // exit the program
        break;

        case 'w': //Going forward
            speed[0]=min(speed[0]+1.0,1.0);
        break;

        case 's': //Going backward
            speed[0]=max(speed[0]-1.0,-1.0);
        break;

        case 'a': //Going Left
            speed[1]=max(speed[1]-1.0,-1.0);
        break;

        case 'd': //Going Right
            speed[1]=min(speed[1]+1.0,1.0);
        break;

        case 't': //Translate on MacOS
            if(controlState==TRANSLATE) controlState=ROTATE;
            else controlState=TRANSLATE;
            cout<<controlState<<"\n";
        break;
        case ' ':
            cout << "You pressed the spacebar." << endl;
        break;

        case 'x':
            // Take a screenshot.
            saveScreenshot("screenshot.jpg");
        break;

        case '1': // Point Mode
            renderType=1;
        break;

        case '2': // Line Mode
            renderType=2;
        break;

        case '3': // Triangle Mode
            renderType=3;
        break;

        case '4': // Smooth Mode
            renderType=4;
        break;

        case '0': //Clear the transformation matrix
            terrainRotate[0]=terrainRotate[1]=terrainRotate[2]=0.0;
            terrainScale[0]=terrainScale[1]=terrainScale[2]=1.0;
            terrainTranslate[0]=terrainTranslate[1]=terrainTranslate[2]=0.0;
        break;

        case '9': //Enable Moving the camera
            speed[0]=speed[1]=0.0;
            if(enableCameraMov) enableCameraMov=false;
            else enableCameraMov=true;
        break;
    }
}

void displayFunc()
{
    // This function performs the actual rendering.

    // First, clear the screen.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(enableCameraMov){
        float verticalVec[3]={-focusVec[2],0,focusVec[0]};
        for(int i=0;i<3;i++){
            eyeVec[i]+=1.0*speed[0]*focusVec[i];
            eyeVec[i]+=1.0*speed[1]*verticalVec[i];
        }
    }

    // Set up the camera position, focus point, and the up vector.
    matrix.SetMatrixMode(OpenGLMatrix::ModelView);
    
    matrix.LoadIdentity();
    
    // View
    matrix.LookAt(eyeVec[0],eyeVec[1],eyeVec[2],
                  eyeVec[0]+focusVec[0],eyeVec[1]+focusVec[1],eyeVec[2]+focusVec[2],
                  upVec[0],upVec[1],upVec[2]);

    // Model
    matrix.Translate(terrainTranslate[0],terrainTranslate[1],terrainTranslate[2]);
    matrix.Rotate(terrainRotate[0],1.0,0.0,0.0);
    matrix.Rotate(terrainRotate[1],0.0,1.0,0.0);
    matrix.Rotate(terrainRotate[2],0.0,0.0,1.0);
    matrix.Scale(terrainScale[0],terrainScale[1],terrainScale[2]);

    // In here, you can do additional modeling on the object, such as performing translations, rotations and scales.
    // ...

    // Read the current modelview and projection matrices from our helper class.
    // The matrices are only read here; nothing is actually communicated to OpenGL yet.
    float modelViewMatrix[16];
    matrix.SetMatrixMode(OpenGLMatrix::ModelView);
    matrix.GetMatrix(modelViewMatrix);

    float projectionMatrix[16];
    matrix.SetMatrixMode(OpenGLMatrix::Projection);
    matrix.GetMatrix(projectionMatrix);

    // Upload the modelview and projection matrices to the GPU. Note that these are "uniform" variables.
    // Important: these matrices must be uploaded to *all* pipeline programs used.
    // In hw1, there is only one pipeline program, but in hw2 there will be several of them.
    // In such a case, you must separately upload to *each* pipeline program.
    // Important: do not make a typo in the variable name below; otherwise, the program will malfunction.
    pipelineProgram->SetUniformVariableMatrix4fv("modelViewMatrix", GL_FALSE, modelViewMatrix);
    pipelineProgram->SetUniformVariableMatrix4fv("projectionMatrix", GL_FALSE, projectionMatrix);

    // Execute the rendering.
    // Bind the VAO that we want to render. Remember, one object = one VAO. 

    if(renderType==1){
        vaoPoint->Bind();
        glDrawElements(GL_POINTS,numVerticesPoint,GL_UNSIGNED_INT,0); // Render the VAO, by using element array, size is "numVerticesPoint", starting from vertex 0.
        // glDrawArrays(GL_POINTS, 0, numVerticesPoint); // Render the VAO, by rendering "numVertices", starting from vertex 0.
    }
    else if(renderType==2){
        vaoLine->Bind();
        glDrawElements(GL_LINES,numVerticesLine,GL_UNSIGNED_INT,0); // Render the VAO, by using element array, size is "numVerticesLine", starting from vertex 0.
        //glDrawArrays(GL_LINES, 0, numVerticesLine); // Render the VAO, by rendering "numVertices", starting from vertex 0.
    }
    else if(renderType==3){
        vaoTriangle->Bind();
        glDrawElements(GL_TRIANGLES,numVerticesTriangle,GL_UNSIGNED_INT,0); // Render the VAO, by using element array, size is "numVerticesTriangle", starting from vertex 0.
        // glDrawArrays(GL_TRIANGLES, 0, numVerticesTriangle);
    }

    // Swap the double-buffers.
    glutSwapBuffers();
}

void calcPosColors(float* positions,float* colors){

    //Calculating points positions, the height is from -25.0 to 25.0
    for (int i = 0; i < heightmapImage->getHeight(); i++) {
        for (int j = 0,pos=i*heightmapImage->getWidth()*3; j < heightmapImage->getWidth(); j++,pos+=3) {
            positions[pos] = i-1.0*(heightmapImage->getHeight() - 1.0)/2.0;
            positions[pos + 1] = 1.0*heightmapImage->getPixel(i,j,0)/255.0*50.0-25.0;
            positions[pos + 2] = -j+1.0*(heightmapImage->getWidth() - 1.0)/2.0;
        }
    }

    //Calculating points colors.
    for (int i = 0; i < heightmapImage->getHeight(); i++) {
        for (int j = 0, pos = i * heightmapImage->getWidth()*4; j < heightmapImage->getWidth(); j++, pos += 4) {
            colors[pos] = 1.0*heightmapImage->getPixel(i,j,0)/255.0;
            colors[pos + 1] = 1.0*heightmapImage->getPixel(i,j,0)/255.0;
            colors[pos + 2] = 1.0*heightmapImage->getPixel(i,j,0)/255.0;
            colors[pos + 3] = 1.0;
        }
    }
}

void initPoint(float* positions,float* colors){

    unsigned int * elements = (unsigned int*)malloc(numVerticesPoint * sizeof(unsigned int)); // initialize elements array

    //The elements for point is just 0,1,2,...,numVerticesPoint-1.
    for (int i = 0; i < heightmapImage->getHeight(); i++) {
        for (int j = 0; j < heightmapImage->getWidth(); j++) {
            int pos=i*heightmapImage->getHeight()+j;
            elements[pos]=pos;
        }
    }

    vboVerticesPoint = new VBO(numVerticesPoint, 3, positions, GL_STATIC_DRAW); // 3 values per position
    vboColorsPoint = new VBO(numVerticesPoint, 4, colors, GL_STATIC_DRAW); // 4 values per color    

    vaoPoint = new VAO();

    vaoPoint->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesPoint, "position");
    vaoPoint->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboColorsPoint, "color");
    eboPoint = new EBO(numVerticesPoint,elements,GL_STATIC_DRAW); //Bind the EBO
    
}

void initLine(float* positions,float* colors){

    numVerticesLine = heightmapImage->getHeight() * (heightmapImage->getWidth() - 1)*2
                    + (heightmapImage->getHeight() - 1) * heightmapImage->getWidth()*2;

    unsigned int* elements = (unsigned int*)malloc(numVerticesLine * sizeof(unsigned int)); // initialize elements array

    int pos=0;

    // Drawing horizontal line
    for (int i = 0; i < heightmapImage->getHeight(); i++) {
        for (int j = 0; j < heightmapImage->getWidth()-1; j++,pos+=2) {
            elements[pos]=i*heightmapImage->getWidth()+j;
            elements[pos+1]=elements[pos]+1;
        }
    }

    // Drawing vertical line
    for (int i = 0; i < heightmapImage->getHeight()-1; i++) {
        for (int j = 0; j < heightmapImage->getWidth(); j++,pos+=2) {
            elements[pos]=i*heightmapImage->getWidth()+j;
            elements[pos+1]=elements[pos]+heightmapImage->getWidth();
        }
    }

    //We can still use the positions and colors since we use elements array
    vboVerticesLine = new VBO(numVerticesLine, 3, positions, GL_STATIC_DRAW); // 3 values per position
    vboColorsLine = new VBO(numVerticesLine, 4, colors, GL_STATIC_DRAW); // 4 values per color
    vaoLine = new VAO();
    
    vaoLine->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesLine, "position");
    vaoLine->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboColorsLine, "color");
    eboLine = new EBO(numVerticesLine,elements,GL_STATIC_DRAW); //Bind the EBO
}

void initTriangle(float* positions,float* colors){

    numVerticesTriangle = (heightmapImage->getHeight() - 1) * (heightmapImage->getWidth() - 1)*6;

    unsigned int* elements = (unsigned int*)malloc(numVerticesTriangle * sizeof(unsigned int)); // initialize elements array

    // The offset of two triangles in a square (Starting from the left up corner).
    int dxy[6]={0,1,(int)heightmapImage->getWidth(),
                1,(int)heightmapImage->getWidth()+1,(int)heightmapImage->getWidth()};

    // Filling the triangles elements
    int pos=0;
    for (int i = 0; i < heightmapImage->getHeight()-1; i++) {
        for (int j = 0; j < heightmapImage->getWidth()-1; j++,pos+=6) {
            int leftUpCorner=i*(heightmapImage->getWidth())+j;
            for(int k1=0;k1<6;k1++){
                elements[pos+k1]=leftUpCorner+dxy[k1];
            }
        }
    }

    //We can still use the positions and colors since we use elements array
    vboVerticesTriangle = new VBO(numVerticesTriangle, 3, positions, GL_STATIC_DRAW); // 3 values per position
    vboColorsTriangle = new VBO(numVerticesTriangle, 4, colors, GL_STATIC_DRAW); // 4 values per color
    vaoTriangle = new VAO();
    
    vaoTriangle->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesTriangle, "position");
    vaoTriangle->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboColorsTriangle, "color");
    eboTriangle = new EBO(numVerticesTriangle,elements,GL_STATIC_DRAW); //Bind the EBO
}

void initScene(int argc, char *argv[])
{
    // Load the image from a jpeg disk file into main memory.
    heightmapImage = new ImageIO();
    if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
    {
        cout << "Error reading image " << argv[1] << "." << endl;
        exit(EXIT_FAILURE);
    }

    // Set the background color.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black color.

    // Enable z-buffering (i.e., hidden surface removal using the z-buffer algorithm).
    glEnable(GL_DEPTH_TEST);

    // Create a pipeline program. This operation must be performed BEFORE we initialize any VAOs.
    // A pipeline program contains our shaders. Different pipeline programs may contain different shaders.
    // In this homework, we only have one set of shaders, and therefore, there is only one pipeline program.
    // In hw2, we will need to shade different objects with different shaders, and therefore, we will have
    // several pipeline programs (e.g., one for the rails, one for the ground/sky, etc.).
    pipelineProgram = new PipelineProgram(); // Load and set up the pipeline program, including its shaders.
    // Load and set up the pipeline program, including its shaders.
    if (pipelineProgram->BuildShadersFromFiles(shaderBasePath, "vertexShader.glsl", "fragmentShader.glsl") != 0)
    {
        cout << "Failed to build the pipeline program." << endl;
        throw 1;
    } 
    cout << "Successfully built the pipeline program." << endl;
        
    // Bind the pipeline program that we just created. 
    // The purpose of binding a pipeline program is to activate the shaders that it contains, i.e.,
    // any object rendered from that point on, will use those shaders.
    // When the application starts, no pipeline program is bound, which means that rendering is not set up.
    // So, at some point (such as below), we need to bind a pipeline program.
    // From that point on, exactly one pipeline program is bound at any moment of time.
    pipelineProgram->Bind();

    numVerticesPoint = heightmapImage->getHeight()* heightmapImage->getWidth(); // This must be a global variable, so that we know how many vertices to render in glDrawArrays.
    float* positions = (float*)malloc(numVerticesPoint * 3 * sizeof(float)); // 3 floats per vertex, i.e., x,y,z
    float* colors = (float*)malloc(numVerticesPoint * 4 * sizeof(float)); // 4 floats per vertex, i.e., r,g,b,a

    calcPosColors(positions,colors); //Transform image to positions and colors

    initPoint(positions,colors); //Create the point VAO,VBO,EBO

    initLine(positions,colors); //Creat the line VAO,VBO,EBO

    initTriangle(positions,colors); //Create the triangle VAO,VBO,EBO

    // We don't need this data any more, as we have already uploaded it to the VBO. And so we can destroy it, to avoid a memory leak.
    free(positions);
    free(colors);

    // Initialize variables in LookAt function
    eyeVec[0]=0.0,eyeVec[1]=0.7* heightmapImage->getWidth(),eyeVec[2]=1.0 * heightmapImage->getWidth();
    focusVec[0]=0.0,focusVec[1]=-0.7* heightmapImage->getWidth(),focusVec[2]=-1.0 * heightmapImage->getWidth();
    upVec[0]=0.0,upVec[1]=1.0,upVec[2]=0.0;

    // Normalize the focusVec
    float sum=0.0;
    for(int i=0;i<3;i++) sum+=focusVec[i]*focusVec[i];
    sum=sqrt(sum);
    for(int i=0;i<3;i++) focusVec[i]/=sum;
    // Check for any OpenGL errors.
    std::cout << "GL error status is: " << glGetError() << std::endl;
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        cout << "The arguments are incorrect." << endl;
        cout << "usage: ./hw1 <heightmap file>" << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Initializing GLUT..." << endl;
    glutInit(&argc,argv);

    cout << "Initializing OpenGL..." << endl;

    #ifdef __APPLE__
        glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    #else
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    #endif

    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(0, 0);    
    glutCreateWindow(windowTitle);

    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    #ifdef __APPLE__
        // This is needed on recent Mac OS X versions to correctly display the window.
        glutReshapeWindow(windowWidth - 1, windowHeight - 1);
    #endif

    // Tells GLUT to use a particular display function to redraw.
    glutDisplayFunc(displayFunc);
    // Perform animation inside idleFunc.
    glutIdleFunc(idleFunc);
    // callback for mouse drags
    glutMotionFunc(mouseMotionDragFunc);
    // callback for idle mouse movement
    glutPassiveMotionFunc(mouseMotionFunc);
    // callback for mouse button changes
    glutMouseFunc(mouseButtonFunc);
    // callback for resizing the window
    glutReshapeFunc(reshapeFunc);
    // callback for pressing the keys on the keyboard
    glutKeyboardFunc(keyboardFunc);

    // init glew
    #ifdef __APPLE__
        // nothing is needed on Apple
    #else
        // Windows, Linux
        GLint result = glewInit();
        if (result != GLEW_OK)
        {
            cout << "error: " << glewGetErrorString(result) << endl;
            exit(EXIT_FAILURE);
        }
    #endif

    // Perform the initialization.
    initScene(argc, argv);

    // Sink forever into the GLUT loop.
    glutMainLoop();
}

