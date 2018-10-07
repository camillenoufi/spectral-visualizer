//-----------------------------------------------------------------------------
// name: VisualSine.cpp
// desc: real-time spectrum visualizer with mountain theme
//
// author: Camille Noufi (cnoufi@ccrma.stanford.edu)
//   date: fall 2018
//   uses: RtAudio by Gary Scavone and SndPeek by Ge Wang
//-----------------------------------------------------------------------------
#include "RtAudio/RtAudio.h"
#include "chuck.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

// FFT
#include "chuck_fft.h"

//Marsyas
/*
#include "Centroid.h"
#include "DownSampler.h"
#include "Flux.h"
#include "LPC.h"
#include "MFCC.h"
#include "RMS.h"
#include "Rolloff.h"
*/

#ifdef __MACOSX_CORE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif




//-----------------------------------------------------------------------------
// function prototypes
//-----------------------------------------------------------------------------
void initGfx();
void idleFunc();
void displayFunc();
void reshapeFunc( GLsizei width, GLsizei height );
void keyboardFunc( unsigned char, int, int );
void mouseFunc( int button, int state, int x, int y );

// our datetype
#define SAMPLE float
// corresponding format for RtAudio
#define MY_FORMAT RTAUDIO_FLOAT32
// sample rate
#define MY_SRATE 44100
// number of channels
#define MY_CHANNELS 2
// for convenience
#define MY_PIE 3.14159265358979
// Audio buffer size
#define SND_BUFFER_SIZE 1024
// FFT size
#define SND_FFT_SIZE    ( SND_BUFFER_SIZE * 2 )
// DSP manipulation window size
#define SND_MARSYAS_SIZE    ( 512 )

// width and height
long g_width = 1024;
long g_height = 720;
// global buffer
SAMPLE * g_buffer = NULL;
long g_bufferSize;

// global variables
bool g_draw_dB = false;
ChucK * the_chuck = NULL;

// gain
GLfloat g_gain = 1.0f;
GLfloat g_time_scale = 1.0f;
GLfloat g_freq_scale = 1.0f;

// how much to see
GLint g_time_view = 1;
GLint g_freq_view = 2;

// global audio buffer
SAMPLE g_fft_buffer[SND_FFT_SIZE];


//-----------------------------------------------------------------------------
// name: callme()
// desc: audio callback
//-----------------------------------------------------------------------------
int callme( void * outputBuffer, void * inputBuffer, unsigned int numFrames,
            double streamTime, RtAudioStreamStatus status, void * data )
{
    // cast!
    SAMPLE * input = (SAMPLE *)inputBuffer;
    SAMPLE * output = (SAMPLE *)outputBuffer;

    // compute chuck!
    // (TODO: to fill in)

    // fill
    for( int i = 0; i < numFrames; i++ )
    {
        // copy the input to visualize only the left-most channel
        g_buffer[i] = input[i*MY_CHANNELS];

        // also copy in the output from chuck to our visualizer
        // (TODO: to fill in)

        // mute output -- TODO will need to disable this once ChucK produces output, in order for you to hear it!
        for( int j = 0; j < MY_CHANNELS; j++ ) { output[i*MY_CHANNELS + j] = 0; }
    }



    return 0;
}




//-----------------------------------------------------------------------------
// name: main()
// desc: entry point
//-----------------------------------------------------------------------------
int main( int argc, char ** argv )
{
    // instantiate RtAudio object
    RtAudio audio;
    // variables
    unsigned int bufferBytes = 0;
    // frame size
    unsigned int bufferFrames = SND_BUFFER_SIZE;

    // check for audio devices
    if( audio.getDeviceCount() < 1 )
    {
        // nopes
        cout << "no audio devices found!" << endl;
        exit( 1 );
    }

    // initialize GLUT
    glutInit( &argc, argv );
    // init gfx
    initGfx();

    // let RtAudio print messages to stderr.
    audio.showWarnings( true );

    // set input and output parameters
    RtAudio::StreamParameters iParams, oParams;
    iParams.deviceId = audio.getDefaultInputDevice();
    iParams.nChannels = MY_CHANNELS;
    iParams.firstChannel = 0;
    oParams.deviceId = audio.getDefaultOutputDevice();
    oParams.nChannels = MY_CHANNELS;
    oParams.firstChannel = 0;

    // create stream options
    RtAudio::StreamOptions options;

    // go for it
    try {
        // open a stream
        audio.openStream( &oParams, &iParams, MY_FORMAT, MY_SRATE, &bufferFrames, &callme, (void *)&bufferBytes, &options );
    }
    catch( RtError& e )
    {
        // error!
        cout << e.getMessage() << endl;
        exit( 1 );
    }

    // compute
    bufferBytes = bufferFrames * MY_CHANNELS * sizeof(SAMPLE);
    // allocate global buffer
    g_bufferSize = bufferFrames;
    g_buffer = new SAMPLE[g_bufferSize];
    memset( g_buffer, 0, sizeof(SAMPLE)*g_bufferSize );

    // set up chuck
    the_chuck = new ChucK();
    // TODO: set sample rate and number of in/out channels on our chuck - THIS IS WRONG!!!
    the_chuck->setParam("SAMPLE_RATE", MY_SRATE);
    the_chuck->setParam("INPUT_CHANNELS", MY_CHANNELS);
    the_chuck->setParam("OUTPUT_CHANNELS", MY_CHANNELS);

    // TODO: initialize our chuck

    // TODO: run a chuck program


    // go for it
    try {
        // start stream
        audio.startStream();

        // let GLUT handle the current thread from here
        glutMainLoop();

        // stop the stream.
        audio.stopStream();
    }
    catch( RtError& e )
    {
        // print error message
        cout << e.getMessage() << endl;
        goto cleanup;
    }

cleanup:
    // close if open
    if( audio.isStreamOpen() )
        audio.closeStream();

    // done
    return 0;
}




//-----------------------------------------------------------------------------
// Name: initGfx( )
// Desc: sets initial OpenGL states and initializes any application data
//-----------------------------------------------------------------------------
void initGfx()
{
    // double buffer, use rgb color, enable depth buffer
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    // initialize the window size
    glutInitWindowSize( g_width, g_height );
    // set the window postion
    glutInitWindowPosition( 100, 100 );
    // create the window
    glutCreateWindow( "VisualSine" );

    // set the idle function - called when idle
    glutIdleFunc( idleFunc );
    // set the display function - called when redrawing
    glutDisplayFunc( displayFunc );
    // set the reshape function - called when client area changes
    glutReshapeFunc( reshapeFunc );
    // set the keyboard function - called on keyboard events
    glutKeyboardFunc( keyboardFunc );
    // set the mouse function - called on mouse stuff
    glutMouseFunc( mouseFunc );

    // set clear color
    glClearColor( 0, 0, 0, 1 );
    // enable color material
    glEnable( GL_COLOR_MATERIAL );
    // enable depth test
    glEnable( GL_DEPTH_TEST );
}




//-----------------------------------------------------------------------------
// Name: reshapeFunc( )
// Desc: called when window size changes
//-----------------------------------------------------------------------------
void reshapeFunc( GLsizei w, GLsizei h )
{
    // save the new window size
    g_width = w; g_height = h;
    // map the view port to the client area
    glViewport( 0, 0, w, h );
    // set the matrix mode to project
    glMatrixMode( GL_PROJECTION );
    // load the identity matrix
    glLoadIdentity( );
    // create the viewing frustum
    gluPerspective( 45.0, (GLfloat) w / (GLfloat) h, 1.0, 300.0 );
    // set the matrix mode to modelview
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point
    gluLookAt( 0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );
}




//-----------------------------------------------------------------------------
// Name: keyboardFunc( )
// Desc: key event
//-----------------------------------------------------------------------------
void keyboardFunc( unsigned char key, int x, int y )
{
    switch( key )
    {
        case 'Q':
        case 'q':
            exit(1);
            break;

        case 'd':
            g_draw_dB = !g_draw_dB;
            break;
    }

    glutPostRedisplay( );
}




//-----------------------------------------------------------------------------
// Name: mouseFunc( )
// Desc: handles mouse stuff
//-----------------------------------------------------------------------------
void mouseFunc( int button, int state, int x, int y )
{
    if( button == GLUT_LEFT_BUTTON )
    {
        // when left mouse button is down
        if( state == GLUT_DOWN )
        {
        }
        else
        {
        }
    }
    else if ( button == GLUT_RIGHT_BUTTON )
    {
        // when right mouse button down
        if( state == GLUT_DOWN )
        {
        }
        else
        {
        }
    }
    else
    {
    }

    glutPostRedisplay( );
}




//-----------------------------------------------------------------------------
// Name: idleFunc( )
// Desc: callback from GLUT
//-----------------------------------------------------------------------------
void idleFunc( )
{
    // render the scene
    glutPostRedisplay( );
}



//-----------------------------------------------------------------------------
// Name: displayFunc( )
// Desc: callback function invoked to draw the client area
//-----------------------------------------------------------------------------
void displayFunc( )
{
    // local state
        static GLfloat zrot = 0.0f, c = 0.0f;

        // clear the color and depth buffers
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );



        // line width
        glLineWidth( 0.5 );
        // define a starting point
        GLfloat x = -5;
        // increment
        GLfloat xinc = ::fabs(x*2 / g_bufferSize);

        // color
        glColor3f( 1.0, 1.0, 1.0 );

        // start primitive
        glBegin( GL_LINE_STRIP );

        // loop over buffer
        for( int i = 0; i < g_bufferSize; i++ )
        {
            if (::fabs(g_buffer[i]>=0.8)) {
                glColor3f(1.0,1.0,0);
                //do i need to do this again??
                glBegin( GL_LINE_STRIP );
                //CHANGE BACKGROUND COLOR TO WHITE
            }

            // plot
            glVertex2f( x, 1*g_buffer[i] );
            // increment x
            x += xinc;
        }

        // take forward FFT; result in buffer as FFT_SIZE/2 complex values
        rfft( (float *)buffer, g_fft_size/2, FFT_FORWARD );
        // cast to complex
        complex * cbuf = (complex *)buffer;

        // end primitive
        glEnd();

        // flush!
        glFlush( );
        // swap the double buffer
        glutSwapBuffers( );

}
