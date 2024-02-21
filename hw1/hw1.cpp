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
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

int mousePos[2]={-1,-1}; // x,y screen coordinates of the current mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

int screenShotCounter = 0;
int renderType=1;
bool enableCameraMov=false;
float speed[2]={0.0,0.0};
float eyeVec[3];
float focusVec[3];
float upVec[3];

// Rotate the camera
float focusRotate[3] = { 0.0f,0.0f,0.0f };
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
ImageIO * heightmapImage=nullptr;
ImageIO* texturemapImage=nullptr;

// CSCI 420 helper classes.
OpenGLMatrix matrix;
PipelineProgram * pipelineProgram = nullptr;

// Number of Vertices, VBOs, VAOs and EBOs
int numVerticesPoint;
VBO * vboVerticesPoint = nullptr;
VBO * vboColorsPoint = nullptr;
EBO * eboPoint = nullptr;
VAO * vaoPoint = nullptr;

int numVerticesLine;
VBO * vboVerticesLine = nullptr;
VBO * vboColorsLine = nullptr;
EBO * eboLine = nullptr;
VAO * vaoLine = nullptr;

int numVerticesTriangle;
VBO * vboVerticesTriangle = nullptr;
VBO * vboColorsTriangle = nullptr;
EBO * eboTriangle = nullptr;
VAO * vaoTriangle = nullptr;

int numVerticesSmooth;
float smoothScale=1.0;
float smoothExponent=1.0;
VBO * vboVerticesSmoothLeft = nullptr;
VBO * vboVerticesSmoothRight = nullptr;
VBO * vboVerticesSmoothDown = nullptr;
VBO * vboVerticesSmoothUp = nullptr;
VBO * vboVerticesSmoothCenter = nullptr;
VBO * vboColorsSmooth = nullptr;
EBO * eboSmooth = nullptr;
VAO * vaoSmooth = nullptr;

int numVerticesWireframe;
VBO* vboVerticesWireframe = nullptr;
VBO* vboColorsWireframe = nullptr;
EBO* eboWireframe = nullptr;
VAO* vaoWireframe = nullptr;

int numVerticesTexture;
unsigned int textureHandle;
VBO* vboVerticesTexture = nullptr;
VBO* vboTexCoordTexture = nullptr;
EBO* eboTexture = nullptr;
VAO* vaoTexture = nullptr;

int numVerticesColorimage;
VBO* vboVerticesColorimage = nullptr;
VBO* vboColorsColorimage = nullptr;
EBO* eboColorimage = nullptr;
VAO* vaoColorimage = nullptr;

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

// Save screenshot in increasing number 0001,0002,0003....
void autoSave() {
    if (screenShotCounter <= 299) {
        char filename[40];
        sprintf(filename, "../newanime/Heightmap-%04d.jpg", ++screenShotCounter);
        saveScreenshot(filename);
    }
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

    if (timeInterval > 1.0/10.0) {
        //autoSave();
        lastTime = currentTime;
    }

    //if (timeInterval > 1.0) {
    //    fps = frameCount / timeInterval;
    //    cout<<"FPS: "<<fps;

    //    frameCount = 0;

    //     cout<<"\n Translate ";
    //     for(int i=0;i<3;i++) cout<<terrainTranslate[i]<<" ";cout<<" | Rotate ";
    //     for(int i=0;i<3;i++) cout<<terrainRotate[i]<<" ";cout<<" | Scale ";
    //     for(int i=0;i<3;i++) cout<<terrainScale[i]<<" ";cout<<" | Focus Rotate ";
    //     for (int i = 0; i < 3; i++) cout << focusRotate[i] << " "; cout << " | FocuseVec ";
    //     for (int i = 0; i < 3; i++) cout << focusVec[i] << " "; cout << " | EyeVec ";
    //     for(int i=0;i<3;i++) cout<<eyeVec[i]<<" ";cout<<"\n";
    //    lastTime = currentTime;
    //}
    
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
    const float zNear = 0.01f;
    const float zFar = 1000.0f;
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
    // the change in mouse position since the last invocation of this function
    int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

    // In case the first move of mouse will not be display so sharp.
    if(mousePos[0]==-1&&mousePos[1]==-1){
        mousePosDelta[0]=mousePosDelta[1]=0;
    }
    if (!leftMouseButton && !middleMouseButton && !rightMouseButton &&enableCameraMov) {
        focusRotate[0] = mousePosDelta[1]*0.3f;
        focusRotate[1] = mousePosDelta[0]*0.3f;
    }
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
            speed[0]=min(speed[0]+0.002,0.004);
        break;

        case 's': //Going backward
            speed[0]=max(speed[0]-0.002,-0.004);
        break;

        case 'a': //Going Left
            speed[1]=max(speed[1]-0.002,-0.004);
        break;

        case 'd': //Going Right
            speed[1]=min(speed[1]+0.002,0.004);
        break;

        case 'c': //Set to initial view.
            terrainRotate[0]=terrainRotate[1]=terrainRotate[2]=0.0;
            terrainScale[0]=terrainScale[1]=terrainScale[2]=1.0;
            terrainTranslate[0]=terrainTranslate[1]=terrainTranslate[2]=0.0;
        break;

        case 'v': //Enable Moving the camera
            speed[0]=speed[1]=0.0;
            if(enableCameraMov) enableCameraMov=false;
            else enableCameraMov=true;
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
            autoSave();
            //saveScreenshot("screenshot.jpg");
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

        case '5': // Smooth Mode with JetMapXolor
            renderType = 5;
        break;

        case '6': // Render a wireframe
            renderType = 6;
        break;

        case '7': // Render a colorful triangle using texture mapping (When there is a texture map)
            renderType = 70;
        break;

        case '&': // Render a colorful triangle using a image with equal size (When there is a texture map)
            renderType = 71;
        break;

        case '9':
            smoothExponent*=2.0;
        break;

        case '0':
            smoothExponent/=2.0;
        break;
 
        case '+':
            smoothScale*=2.0;
        break;

        case '-':
            smoothScale/=2.0;
        break;
    }
}

void modifyFocusAndCamera(){

    float verticalVec[3]={-focusVec[2],0,focusVec[0]};
    
    // change the camera position
    for(int i=0;i<3;i++){
        eyeVec[i]+=1.0*speed[0]*focusVec[i];
        eyeVec[i]+=1.0*speed[1]*verticalVec[i];
    }
    
    matrix.SetMatrixMode(OpenGLMatrix::ModelView);

    // Modify the focus vector.
    matrix.LoadIdentity();
    matrix.Rotate(focusRotate[0], focusVec[2], 0.0, -focusVec[0]);
    matrix.Rotate(focusRotate[1], 0.0, -1.0, 0.0);

    float tmpMatrix[16];
    matrix.GetMatrix(tmpMatrix);
    glm::mat4 rotateMatrix=glm::make_mat4(tmpMatrix);
    glm::vec4 tmpFocus(focusVec[0],focusVec[1],focusVec[2],1.0f);
    
    // Multiply the rotate matrix with the focus vector to get the rotated vector
    tmpFocus = rotateMatrix * tmpFocus;

    for (int i = 0; i < 3; i++) {
        focusVec[i] = tmpFocus[i];
        focusRotate[i] = 0;
    }
}

void displayFunc()
{
    // This function performs the actual rendering.

    // First, clear the screen.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(enableCameraMov){
        modifyFocusAndCamera();
    }

    matrix.SetMatrixMode(OpenGLMatrix::ModelView);
    // Set up the camera position, focus point, and the up vector.
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
    matrix.Scale(terrainScale[0],terrainScale[1]/3.0,terrainScale[2]);

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

    // Set render type, if is 4 then render the smooth
    pipelineProgram->SetUniformVariablei("renderType",renderType);

    pipelineProgram->SetUniformVariablef("scale",smoothScale);
    pipelineProgram->SetUniformVariablef("exponent",smoothExponent);

    // Execute the rendering.
    // Bind the VAO that we want to render. Remember, one object = one VAO. 

    if(renderType==1){
        vaoPoint->Bind();
        glDrawElements(GL_POINTS,numVerticesPoint,GL_UNSIGNED_INT,0); // Render the VAO, by using element array, size is "numVerticesPoint", starting from vertex 0.
    }
    else if(renderType==2){
        vaoLine->Bind();
        glDrawElements(GL_LINES,numVerticesLine,GL_UNSIGNED_INT,0); // Render the VAO, by using element array, size is "numVerticesLine", starting from vertex 0.
    }
    else if(renderType==3){
        vaoTriangle->Bind();
        glDrawElements(GL_TRIANGLES,numVerticesTriangle,GL_UNSIGNED_INT,0); // Render the VAO, by using element array, size is "numVerticesTriangle", starting from vertex 0.
    }
    else if(renderType==4){
        vaoSmooth->Bind();
        glDrawElements(GL_TRIANGLES,numVerticesSmooth,GL_UNSIGNED_INT,0); // Render the VAO, by using element array, size is "numVerticesSmooth", starting from vertex 0.
    }
    else if (renderType == 5) {
        vaoSmooth->Bind();
        glDrawElements(GL_TRIANGLES, numVerticesSmooth, GL_UNSIGNED_INT, 0); // Render the VAO, by using element array, size is "numVerticesSmooth", starting from vertex 0.
    }
    else if (renderType==6) {
        vaoWireframe->Bind();
        glDrawElements(GL_LINES, numVerticesWireframe, GL_UNSIGNED_INT, 0); // Render the VAO, by using element array, size is "numVerticesLine", starting from vertex 0.
        
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f,1.0f);
        
        // after drawing the line, then set the renderType to normal color
        pipelineProgram->SetUniformVariablei("renderType", 3);
        vaoTriangle->Bind();
        glDrawElements(GL_TRIANGLES, numVerticesTriangle, GL_UNSIGNED_INT, 0); // Render the VAO, by using element array, size is "numVerticesTriangle", starting from vertex 0.
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    else if(renderType==70){
        if(texturemapImage==nullptr){
            cout<<"There is no texture map image\n";
            renderType=0; //Not compatible so render nothing. 
        }
        else{
            vaoTexture->Bind();
            pipelineProgram->SetUniformVariablei("textureImg",0);
            glDrawElements(GL_TRIANGLES, numVerticesTexture, GL_UNSIGNED_INT, 0); // Render the VAO, by using element array, size is "numVerticesColorimage", starting from vertex 0.
        }
    }
    else if(renderType==71){
        // Check if it could render a colorful image
        if(texturemapImage==nullptr||texturemapImage->getHeight()!=heightmapImage->getHeight()||texturemapImage->getWidth()!=heightmapImage->getWidth()){
            cout<<"There is no texture map image or the texture map size is not compatible with heightmap size\n";
            renderType=0; //Not compatible so render nothing.
        }
        else{
            vaoColorimage->Bind();
            glDrawElements(GL_TRIANGLES, numVerticesColorimage, GL_UNSIGNED_INT, 0); // Render the VAO, by using element array, size is "numVerticesColorimage", starting from vertex 0.
        }
    }
    else{
        // If not match with any render type then just render noting.
    }
    
    // Swap the double-buffers.
    glutSwapBuffers();
}

void calcPosColors(float* positions,float* colors){
    // maximum of height and width, for normalizatoin
    float maxHW = max(heightmapImage->getHeight() - 1.0, heightmapImage->getWidth() - 1.0);
    if (heightmapImage->getBytesPerPixel() == 1) {
        //Calculating points positions, the height range is [0.0,1.0] for scale\exponent | x range is [-1.0,1.0] | z range is [-1.0,1.0]
        for (int i = 0; i < heightmapImage->getHeight(); i++) {
            for (int j = 0, pos = i * heightmapImage->getWidth() * 3; j < heightmapImage->getWidth(); j++, pos += 3) {
                positions[pos] = (j * 2.0 - (heightmapImage->getWidth() - 1.0)) / maxHW;
                positions[pos + 1] = 1.0 * heightmapImage->getPixel(j, i, 0) / 255.0f;
                positions[pos + 2] = (-2.0 * i + (heightmapImage->getHeight() - 1.0)) / maxHW;
            }
        }
    }
    else if(heightmapImage->getBytesPerPixel()==3){
        //Calculating points positions, the height range is [0.0,1.0] for scale\exponent | x range is [-1.0,1.0] | z range is [-1.0,1.0]
        for (int i = 0; i < heightmapImage->getHeight(); i++) {
            for (int j = 0, pos = i * heightmapImage->getWidth() * 3; j < heightmapImage->getWidth(); j++, pos += 3) {
                positions[pos] = (j * 2.0 - (heightmapImage->getWidth() - 1.0)) / maxHW;
                // Transform the RGB to Grayscale by using 0.299 R + 0.587 G + 0.114 B same in OpenCV https://docs.opencv.org/4.x/de/d25/imgproc_color_conversions.html
                //cout << pos << " ";
                positions[pos + 1] = (  0.299 * heightmapImage->getPixel(j, i, 0)+
                                        0.587 * heightmapImage->getPixel(j, i, 1)+
                                        0.114 * heightmapImage->getPixel(j, i, 2))/ 255.0f;
                positions[pos + 2] = (-2.0 * i + (heightmapImage->getHeight() - 1.0)) / maxHW;
            }
        }
    }
    //Calculating points colors.
    for (int i = 0; i < heightmapImage->getHeight(); i++) {
        for (int j = 0, pos = i * heightmapImage->getWidth() * 4; j < heightmapImage->getWidth(); j++, pos += 4) {
            colors[pos] = colors[pos + 1] = colors[pos + 2] = 1.0 * positions[pos / 4 * 3 + 1] ;
            colors[pos + 3] = 1.0;
        }
    }
}

void initPoint(float* positions,float* colors){

    unsigned int * elements = (unsigned int*)malloc(numVerticesPoint * sizeof(unsigned int)); // initialize elements array

    //The elements for point is just 0,1,2,...,numVerticesPoint-1.
    for (int i = 0; i < heightmapImage->getHeight(); i++) {
        for (int j = 0; j < heightmapImage->getWidth(); j++) {
            int pos=i*heightmapImage->getWidth()+j;
            elements[pos]=pos;
        }
    }

    vboVerticesPoint = new VBO(numVerticesPoint, 3, positions, GL_STATIC_DRAW); // 3 values per position
    vboColorsPoint = new VBO(numVerticesPoint, 4, colors, GL_STATIC_DRAW); // 4 values per color    

    vaoPoint = new VAO();

    vaoPoint->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesPoint, "position");
    vaoPoint->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboColorsPoint, "color");
    eboPoint = new EBO(numVerticesPoint,elements,GL_STATIC_DRAW); //Bind the EBO
    
    //Free the elements;
    free(elements);
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
    vboVerticesLine = new VBO(numVerticesPoint, 3, positions, GL_STATIC_DRAW); // 3 values per position, usinng number of point
    vboColorsLine = new VBO(numVerticesPoint, 4, colors, GL_STATIC_DRAW); // 4 values per color, using number of point
    vaoLine = new VAO();
    
    vaoLine->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesLine, "position");
    vaoLine->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboColorsLine, "color");
    eboLine = new EBO(numVerticesLine,elements,GL_STATIC_DRAW); //Bind the EBO

    //Free the elements;
    free(elements);
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
    vboVerticesTriangle = new VBO(numVerticesPoint, 3, positions, GL_STATIC_DRAW); // 3 values per position, usinng number of point
    vboColorsTriangle = new VBO(numVerticesPoint, 4, colors, GL_STATIC_DRAW); // 4 values per color, usinng number of point
    vaoTriangle = new VAO();
    
    vaoTriangle->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesTriangle, "position");
    vaoTriangle->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboColorsTriangle, "color");
    eboTriangle = new EBO(numVerticesTriangle,elements,GL_STATIC_DRAW); //Bind the EBO

    //Free the elements;
    free(elements);
}

void initSmooth(float* positions,float* colors){

    numVerticesSmooth = (heightmapImage->getHeight() - 1) * (heightmapImage->getWidth() - 1)*6;

    unsigned int* elements = (unsigned int*)malloc(numVerticesSmooth * sizeof(unsigned int)); // initialize elements array
    
    int pos=0;
    // The offset of two triangles in a square (Starting from the left up corner).
    int dxy[6]={0,1,(int)heightmapImage->getWidth(),
                1,(int)heightmapImage->getWidth()+1,(int)heightmapImage->getWidth()};
    // Filling the triangles elements
    for (int i = 0; i < heightmapImage->getHeight()-1; i++) {
        for (int j = 0; j < heightmapImage->getWidth()-1; j++,pos+=6) {
            int leftUpCorner=i*(heightmapImage->getWidth())+j;
            for(int k1=0;k1<6;k1++){
                elements[pos+k1]=leftUpCorner+dxy[k1];
            }
        }
    }

    vboVerticesSmoothCenter = new VBO(numVerticesPoint, 3, positions, GL_STATIC_DRAW); // For center, we can use the original positions.

    // Creating temperary positions array.
    float* tmpPos = (float*)malloc(numVerticesPoint * 3 * sizeof(float));

    // Create Pleft : tmpPos[i*width+j]=positions[i*width+(j-1)]
    pos=0;
    for(int i=0;i<heightmapImage->getHeight();i++){
        tmpPos[pos]=positions[pos];
        tmpPos[pos+1]=positions[pos+1];
        tmpPos[pos+2]=positions[pos+2];
        pos+=3;
        for(int j=1;j<heightmapImage->getWidth();j++,pos+=3){
            tmpPos[pos]=positions[pos-3];
            tmpPos[pos+1]=positions[pos-2];
            tmpPos[pos+2]=positions[pos-1];
        }
    }
    // Send Pleft to vbo
    vboVerticesSmoothLeft = new VBO(numVerticesPoint,3,tmpPos,GL_STATIC_DRAW);

    // Create Pright : tmpPos[i*width+j]=positions[i*width+(j+1)]
    pos=0;
    for(int i=0;i<heightmapImage->getHeight();i++){
        for(int j=0;j<heightmapImage->getWidth()-1;j++,pos+=3){
            tmpPos[pos]=positions[pos+3];
            tmpPos[pos+1]=positions[pos+4];
            tmpPos[pos+2]=positions[pos+5];
        }
        tmpPos[pos]=positions[pos];
        tmpPos[pos+1]=positions[pos+1];
        tmpPos[pos+2]=positions[pos+2];
        pos+=3;
    }
    // Send Pright to vbo
    vboVerticesSmoothRight = new VBO(numVerticesPoint,3,tmpPos,GL_STATIC_DRAW);

    // Create Pup : tmpPos[i*width+j]=positions[(i+1)*width+j]
    pos=0;
    for(int i=0;i<heightmapImage->getHeight();i++){
        if(i==0){
            for(int j=0;j<heightmapImage->getWidth();j++,pos+=3){
                tmpPos[pos]=positions[pos];
                tmpPos[pos+1]=positions[pos+1];
                tmpPos[pos+2]=positions[pos+2];
            }
        }
        else{
            for(int j=0;j<heightmapImage->getWidth();j++,pos+=3){
                tmpPos[pos]=positions[pos-heightmapImage->getWidth()*3];
                tmpPos[pos+1]=positions[pos+1-heightmapImage->getWidth()*3];
                tmpPos[pos+2]=positions[pos+2-heightmapImage->getWidth()*3];
            }
        }
    }
    // Send Pup to vbo
    vboVerticesSmoothUp = new VBO(numVerticesPoint,3,tmpPos,GL_STATIC_DRAW);

    // Create Pdown : tmpPos[i*width+j]=positions[(i-1)*width+j]
    pos=0;
    for(int i=0;i<heightmapImage->getHeight();i++){
        if(i==heightmapImage->getHeight()-1){
            for(int j=0;j<heightmapImage->getWidth();j++,pos+=3){
                tmpPos[pos]=positions[pos];
                tmpPos[pos+1]=positions[pos+1];
                tmpPos[pos+2]=positions[pos+2];
            }
        }
        else{
            for(int j=0;j<heightmapImage->getWidth();j++,pos+=3){
                tmpPos[pos]=positions[pos+heightmapImage->getWidth()*3];
                tmpPos[pos+1]=positions[pos+1+heightmapImage->getWidth()*3];
                tmpPos[pos+2]=positions[pos+2+heightmapImage->getWidth()*3];
            }
        }
    }
    // Send Pdown to vbo
    vboVerticesSmoothDown = new VBO(numVerticesPoint,3,tmpPos,GL_STATIC_DRAW);

    //We can still use the colors since we use elements array
    vboColorsSmooth = new VBO(numVerticesPoint, 4, colors, GL_STATIC_DRAW); // 4 values per color, usinng number of point

    vaoSmooth = new VAO();

    vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesSmoothLeft, "pleft");
    vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesSmoothRight, "pright");
    vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesSmoothUp, "pup");
    vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesSmoothDown, "pdown");
    vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesSmoothCenter, "pcenter");

    vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboColorsSmooth, "color");

    eboSmooth = new EBO(numVerticesSmooth,elements,GL_STATIC_DRAW); //Bind the EBO

    // Free the pointer
    free(tmpPos);
    free(elements);
}

void initWireframe(float* positions, float* colors) {

    numVerticesWireframe = heightmapImage->getHeight() * (heightmapImage->getWidth() - 1) * 2
        + (heightmapImage->getHeight() - 1) * heightmapImage->getWidth() * 2
        + (heightmapImage->getHeight() - 1) * (heightmapImage->getWidth()-1) * 2;

    unsigned int* elements = (unsigned int*)malloc(numVerticesWireframe * sizeof(unsigned int)); // initialize elements array

    int pos = 0;

    // Drawing horizontal line
    for (int i = 0; i < heightmapImage->getHeight(); i++) {
        for (int j = 0; j < heightmapImage->getWidth() - 1; j++, pos += 2) {
            elements[pos] = i * heightmapImage->getWidth() + j;
            elements[pos + 1] = elements[pos] + 1;
        }
    }

    // Drawing vertical line
    for (int i = 0; i < heightmapImage->getHeight() - 1; i++) {
        for (int j = 0; j < heightmapImage->getWidth(); j++, pos += 2) {
            elements[pos] = i * heightmapImage->getWidth() + j;
            elements[pos + 1] = elements[pos] + heightmapImage->getWidth();
        }
    }

    // Drawing diagnal line
    for (int i = 0; i < heightmapImage->getHeight() - 1; i++) {
        for (int j = 0; j < heightmapImage->getWidth()-1; j++, pos += 2) {
            elements[pos] = i * heightmapImage->getWidth() + j+1;
            elements[pos + 1] = elements[pos] + heightmapImage->getWidth()-1;
        }
    }

    // The color of the wireframe is all white
    float* colorsWireframe = (float*)malloc(numVerticesPoint * 4 * sizeof(float)); // 4 floats per vertex, i.e., r,g,b,a
    pos = 0;
    for (int i = 0; i < heightmapImage->getHeight(); i++) {
        for (int j = 0; j < heightmapImage->getWidth(); j++,pos+=4) {
            colorsWireframe[pos] = 1.0f;
            colorsWireframe[pos + 1] = 1.0f;
            colorsWireframe[pos + 2] = 1.0f;
            colorsWireframe[pos + 3] = 1.0f;
        }
    }

    //We can still use the positions and colors since we use elements array
    vboVerticesWireframe = new VBO(numVerticesPoint, 3, positions, GL_STATIC_DRAW); // 3 values per position, usinng number of point
    vboColorsWireframe = new VBO(numVerticesPoint, 4, colorsWireframe, GL_STATIC_DRAW); // 4 values per color, using number of point
    vaoWireframe = new VAO();

    vaoWireframe->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesWireframe, "position");
    vaoWireframe->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboColorsWireframe, "color");
    eboWireframe = new EBO(numVerticesWireframe, elements, GL_STATIC_DRAW); //Bind the EBO

    //Free the elements;
    free(elements);
    free(colorsWireframe);
}

void initTriangleWithTexture(float* positions){

    numVerticesTexture = (heightmapImage->getHeight() - 1) * (heightmapImage->getWidth() - 1)*6;

    unsigned int* elements = (unsigned int*)malloc(numVerticesTexture * sizeof(unsigned int)); // initialize elements array

    // Initialize the texture coordinate, there are only (x,y);
    float * texCoord=(float *)malloc(numVerticesPoint*sizeof(float)*2);

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

    // Filling the texture coordinate
    pos=0;
    for(int i=0;i<heightmapImage->getHeight();i++){
        for(int j=0;j<heightmapImage->getWidth();j++,pos+=2){
            texCoord[pos]=(1.0*j)/(1.0f*heightmapImage->getWidth()-1.0);
            texCoord[pos+1]=(1.0*i)/(1.0f*heightmapImage->getHeight()-1.0);
        }
    }

    //We can still use the positions and colors since we use elements array
    vboVerticesTexture = new VBO(numVerticesPoint, 3, positions, GL_STATIC_DRAW); // 3 values per position, usinng number of point
    vboTexCoordTexture = new VBO(numVerticesPoint, 2, texCoord, GL_STATIC_DRAW);
    vaoTexture = new VAO();
    
    vaoTexture->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesTexture, "position");
    vaoTexture->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboTexCoordTexture, "texCoord");

    // Bind the texture map to texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1,&textureHandle);
    glBindTexture(GL_TEXTURE_2D,textureHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texturemapImage->getWidth(), texturemapImage->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, texturemapImage->getPixels());
    eboTexture = new EBO(numVerticesTexture,elements,GL_STATIC_DRAW); //Bind the EBO

    //Free the elements;
    free(elements);
}

void initTriangleWithImage(float* positions){

    numVerticesColorimage = (heightmapImage->getHeight() - 1) * (heightmapImage->getWidth() - 1)*6;

    unsigned int* elements = (unsigned int*)malloc(numVerticesColorimage * sizeof(unsigned int)); // initialize elements array

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

    float* colorsImage = (float*)malloc(numVerticesPoint * 4 * sizeof(float)); // 4 floats per vertex, i.e., r,g,b,a

    //Calculating points colors.
    for (int i = 0; i < heightmapImage->getHeight(); i++) {
        for (int j = 0, pos = i * heightmapImage->getWidth() * 4; j < heightmapImage->getWidth(); j++, pos += 4) {
            colorsImage[pos] = texturemapImage->getPixel(j, i, 0) / 255.0f;
            colorsImage[pos+1] = texturemapImage->getPixel(j, i, 1) / 255.0f;
            colorsImage[pos+2] = texturemapImage->getPixel(j, i, 2) / 255.0f;
            colorsImage[pos + 3] = 1.0;
        }
    }

    //We can still use the positions and colors since we use elements array
    vboVerticesColorimage = new VBO(numVerticesPoint, 3, positions, GL_STATIC_DRAW); // 3 values per position, usinng number of point
    vboColorsColorimage = new VBO(numVerticesPoint, 4, colorsImage, GL_STATIC_DRAW); // 4 values per color, usinng number of point
    vaoColorimage = new VAO();
    
    vaoColorimage->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVerticesColorimage, "position");
    vaoColorimage->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboColorsColorimage, "color");
    eboColorimage = new EBO(numVerticesColorimage,elements,GL_STATIC_DRAW); //Bind the EBO

    //Free the elements;
    free(elements);
    free(colorsImage);
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
    // Check the heightmapImage has 3 channels or 1 channel otherwise throws error.
    if (heightmapImage->getBytesPerPixel() != 1 && heightmapImage->getBytesPerPixel() != 3) {
        cout << "Image channel need to be 1 or 3 " << argv[1] << "." << endl;
        exit(EXIT_FAILURE);
    }

    if (argc==3) {
        cout << "loading texture\n";
        texturemapImage = new ImageIO();
        // Load the image from a jpeg disk file into main memory.
        if (texturemapImage->loadJPEG(argv[2]) != ImageIO::OK) {
            cout << "Error reading image " << argv[2] << "." << endl;
            exit(EXIT_FAILURE);
        }
        // Check the htexturemapImage has 3 channels throws error.
        if (texturemapImage->getBytesPerPixel() != 3) {
            cout << "texture map should to be 3 " << argv[2] << "." << endl;
            exit(EXIT_FAILURE);
        }
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

    initSmooth(positions,colors); //Create the Smooth VAO,VBO,EBO

    initWireframe(positions, colors); //Create the Wireframe VAO,VBO,EBO

    if(texturemapImage!=nullptr){
        initTriangleWithTexture(positions); //Create the triangle with texture VAO,VBO,EBO
        if(heightmapImage->getHeight()==texturemapImage->getHeight()&&heightmapImage->getWidth()==texturemapImage->getWidth()){
            initTriangleWithImage(positions); // Create the triangle with image at same size.
        }
    }

    // We don't need this data any more, as we have already uploaded it to the VBO. And so we can destroy it, to avoid a memory leak.
    free(positions);
    free(colors);

    // Initialize variables in LookAt function
    // eyeVec[0]=0.5,eyeVec[1]=0.5,eyeVec[2]=0.5;
    // focusVec[0]=0.0,focusVec[1]=-0.5,focusVec[2]=-1.0;
    // upVec[0]=0.0,upVec[1]=1.0,upVec[2]=0.0;

    // Initialize variables in LookAt function
    eyeVec[0]=0.0,eyeVec[1]=1.3,eyeVec[2]=1.7;
    focusVec[0]=0.0,focusVec[1]=-1.3,focusVec[2]=-1.7;
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

    if (argc != 2&&argc!=3)
    {
        cout << "The arguments are incorrect." << endl;
        cout << "usage: ./hw1 <heightmap file> or ./hw1 <heightmap file> <texturemap file>" << endl;
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
