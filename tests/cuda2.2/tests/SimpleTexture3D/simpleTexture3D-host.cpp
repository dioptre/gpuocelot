/*
 * Copyright 1993-2009 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:   
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and 
 * international Copyright laws.  
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE 
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR 
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH 
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.   
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL, 
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS 
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE 
 * OR PERFORMANCE OF THIS SOURCE CODE.  
 *
 * U.S. Government End Users.  This source code is a "commercial item" as 
 * that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting  of 
 * "commercial computer software" and "commercial computer software 
 * documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995) 
 * and is provided to the U.S. Government only as a commercial end item.  
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through 
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the 
 * source code with only those rights set forth herein.
 */

/*
    3D texture sample

    This sample loads a 3D volume from disk and displays slices through it
    using 3D texture lookups.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#if defined (__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <vector_types.h>
#include <driver_functions.h>
#include <cutil_inline.h>
#include <cutil_gl_inline.h>
#include <cuda_gl_interop.h>
#include <rendercheck_gl.h>

typedef unsigned int  uint;
typedef unsigned char uchar;

#define MAX_EPSILON_ERROR 5.0f

const char *sSDKsample = "simpleTexture3D";

// Define the files that are to be save and the reference images for validation
const char *sOriginal[] =
{
    "simpleTex3D.ppm",
    NULL
};

const char *sReference[] =
{
    "ref_simpleTex3D.ppm",
    NULL
};

const char *volumeFilename = "Bucky.raw";
const cudaExtent volumeSize = make_cudaExtent(32, 32, 32);

const uint width = 512, height = 512;
const dim3 blockSize(16, 16, 1);
const dim3 gridSize(width / blockSize.x, height / blockSize.y);

float w = 0.5;  // texture coordinate in z

GLuint pbo;     // OpenGL pixel buffer object

bool linearFiltering = true;
bool animate = true;

unsigned int timer = 0;

uint *d_output = NULL;

// Auto-Verification Code
const int frameCheckNumber = 4;
int fpsCount = 0;        // FPS count for averaging
int fpsLimit = 1;        // FPS limit for sampling
int g_Index = 0;
unsigned int frameCount = 0;
unsigned int g_TotalErrors = 0;
bool g_Verify = false;
bool g_bQAReadback = false;
bool g_bOpenGLQA   = false;

// CheckFBO/BackBuffer class objects
CheckRender       *g_CheckRender = NULL;

#define MAX(a,b) ((a > b) ? a : b)

extern "C" void setTextureFilterMode(bool bLinearFilter);
extern "C" void initCuda(const uchar *h_volume, cudaExtent volumeSize);
extern "C" void render_kernel(dim3 gridSize, dim3 blockSize, uint *d_output, uint imageW, uint imageH, float w);

void loadVolumeData(char *exec_path);

void AutoQATest()
{
    if (g_CheckRender && g_CheckRender->IsQAReadback()) {
        char temp[256];
        sprintf(temp, "AutoTest: %s", sSDKsample);
	glutSetWindowTitle(temp);
	exit(0);
    }
}

void computeFPS()
{
    frameCount++;
    fpsCount++;
    if (fpsCount == fpsLimit-1) {
        g_Verify = true;
    }
    if (fpsCount == fpsLimit) {
        char fps[256];
        float ifps = 1.f / (cutGetAverageTimerValue(timer) / 1000.f);
        sprintf(fps, "%s %s: %3.1f fps", sSDKsample, 
                ((g_CheckRender && g_CheckRender->IsQAReadback()) ? "AutoTest: " : ""), ifps);  

        glutSetWindowTitle(fps);
        fpsCount = 0; 
        if (g_CheckRender && !g_CheckRender->IsQAReadback()) fpsLimit = (int)MAX(ifps, 1.f);

        cutilCheckError(cutResetTimer(timer));  

        AutoQATest();
    }
}


// render image using CUDA
void render()
{
    // map PBO to get CUDA device pointer
    cutilSafeCall(cudaGLMapBufferObject((void**)&d_output, pbo));

    // call CUDA kernel, writing results to PBO
    render_kernel(gridSize, blockSize, d_output, width, height, w);

    cutilCheckMsg("kernel failed");

    cutilSafeCall(cudaGLUnmapBufferObject(pbo));
}

// display results using OpenGL (called by GLUT)
void display()
{
    cutilCheckError(cutStartTimer(timer));  

    render();

    // display results
    glClear(GL_COLOR_BUFFER_BIT);

    // draw image from PBO
    glDisable(GL_DEPTH_TEST);
    glRasterPos2i(0, 0);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
    glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    if (g_CheckRender && g_CheckRender->IsQAReadback() && g_Verify) {
        // readback for QA testing
        printf("> (Frame %d) Readback BackBuffer\n", frameCount);
        g_CheckRender->readback( width, height );
        g_CheckRender->savePPM(sOriginal[g_Index], true, NULL);
        if (!g_CheckRender->PPMvsPPM(sOriginal[g_Index], sReference[g_Index], MAX_EPSILON_ERROR)) {
            g_TotalErrors++;
        }
        g_Verify = false;
    }

    glutSwapBuffers();
    glutReportErrors();

    cutilCheckError(cutStopTimer(timer));  
    computeFPS();
}

void idle()
{
    if (animate) {
        w += 0.01f;
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
        case 27:
            exit(0);
            break;
        case '=':
        case '+':
            w += 0.01;
            break;
        case '-':
            w -= 0.01;
            break;
        case 'f':
            linearFiltering = !linearFiltering;
            setTextureFilterMode(linearFiltering);
            break;
        case ' ':
            animate = !animate;
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void reshape(int x, int y)
{
    glViewport(0, 0, x, y);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0); 
}

void cleanup()
{
    cutilCheckError( cutDeleteTimer( timer));

    if (!g_bQAReadback) {
        cutilSafeCall(cudaGLUnregisterBufferObject(pbo));    
	    glDeleteBuffersARB(1, &pbo);
    }

    if (g_CheckRender) {
        delete g_CheckRender; g_CheckRender = NULL;
    }
}

void initGLBuffers()
{
    // create pixel buffer object
    glGenBuffersARB(1, &pbo);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, width*height*sizeof(GLubyte)*4, 0, GL_STREAM_DRAW_ARB);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    cutilSafeCall(cudaGLRegisterBufferObject(pbo));
}

// Load raw data from disk
uchar *loadRawFile(const char *filename, size_t size)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error opening file '%s'\n", filename);
        return 0;
    }

    uchar *data = (uchar *) malloc(size);
    size_t read = fread(data, 1, size, fp);
    fclose(fp);

    printf("Read '%s', %d bytes\n", filename, read);

    return data;
}

void initGL( int argc, char **argv )
{
    // initialize GLUT callback functions
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(width, height);
    glutCreateWindow("CUDA 3D texture");
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    glewInit();
    if (!glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object")) {
        fprintf(stderr, "Required OpenGL extensions missing.");
        cudaThreadExit();
        exit(-1);
    }
}

void runAutoTest( int argc, char **argv )
{
    g_CheckRender = new CheckBackBuffer(width, height, 4, false);
    g_CheckRender->setPixelFormat(GL_RGBA);
    g_CheckRender->setExecPath(argv[0]);
    g_CheckRender->EnableQAReadback(true);

    // use command-line specified CUDA device, otherwise use device with highest Gflops/s
    if( cutCheckCmdLineFlag(argc, (const char**)argv, "device") ) {
	    cutilDeviceInit(argc, argv);
    } else {
	    cudaSetDevice( cutGetMaxGflopsDeviceId() );
    }

    loadVolumeData(argv[0]);

    cutilSafeCall( cudaMalloc((void **)&d_output, width*height*sizeof(GLubyte)*4) );

    // render the volumeData
    render_kernel(gridSize, blockSize, d_output, width, height, w);

    cutilSafeCall( cudaThreadSynchronize() );
    cutilCheckMsg("render_kernel failed");

    cutilSafeCall( cudaMemcpy( g_CheckRender->imageData(), d_output, width*height*sizeof(GLubyte)*4, cudaMemcpyDeviceToHost) );
    g_CheckRender->dumpBin((void *)g_CheckRender->imageData(), width*height*sizeof(GLubyte)*4, "simpleTexture3D.bin");
    if (!g_CheckRender->compareBin2BinUint("simpleTexture3D.bin", "ref_texture3D.bin", width*height*sizeof(GLubyte)*4, 0, 0.15f))
       g_TotalErrors++;

    cutilSafeCall( cudaFree(d_output) );
    printf("TEST %s\n", (g_TotalErrors > 0) ? "FAILED!" : "PASSED!");
}


void loadVolumeData(char *exec_path)
{
    // load volume data
        const char* path = cutFindFilePath(volumeFilename, exec_path);
        if (path == 0) {
            fprintf(stderr, "Error finding file '%s'\n", volumeFilename);
            exit(EXIT_FAILURE);
        }

        size_t size = volumeSize.width*volumeSize.height*volumeSize.depth;
        uchar *h_volume = loadRawFile(path, size);

        initCuda(h_volume, volumeSize);
        cutilCheckError( cutCreateTimer( &timer));

        free(h_volume);
}


////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int
main( int argc, char** argv) 
{
    if (!cutCheckCmdLineFlag(argc, (const char **)argv, "noqatest") ||
        cutCheckCmdLineFlag(argc, (const char **)argv, "noprompt")) 
	{
        g_bQAReadback = true;
        fpsLimit = frameCheckNumber;
    }

    if (argc > 1) {
        if (cutCheckCmdLineFlag(argc, (const char **)argv, "glverify")) 
		{
            g_bOpenGLQA = true;
            fpsLimit = frameCheckNumber;
        }
    }

    printf("[%s] ", sSDKsample);
    if (g_bQAReadback) printf("(Automated Testing)\n");
    if (g_bOpenGLQA)   printf("(OpenGL Readback)\n");

    if (g_bQAReadback) {
        runAutoTest(argc, argv);
        cleanup();
        cudaThreadExit();
        cutilExit(argc, argv);
    } 
    else 
    {
        if (g_bOpenGLQA) {
            g_CheckRender = new CheckBackBuffer(width, height, 4);
            g_CheckRender->setPixelFormat(GL_RGBA);
            g_CheckRender->setExecPath(argv[0]);
            g_CheckRender->EnableQAReadback(true);
        }

        // First initialize OpenGL context, so we can properly set the GL for CUDA.
        // This is necessary in order to achieve optimal performance with OpenGL/CUDA interop.
        initGL(argc, argv);

        // use command-line specified CUDA device, otherwise use device with highest Gflops/s
	    if( cutCheckCmdLineFlag(argc, (const char**)argv, "device") ) {
		    cutilGLDeviceInit(argc, argv);
	    } else {
		    cudaGLSetGLDevice( cutGetMaxGflopsDeviceId() );
	    }

        // OpenGL buffers
        initGLBuffers();

        loadVolumeData(argv[0]);
    }

    printf("Press space to toggle animation\n"
           "Press '+' and '-' to change displayed slice\n");

    atexit(cleanup);

    glutMainLoop();

    cudaThreadExit();
    cutilExit(argc, argv);
    return 0;
}
