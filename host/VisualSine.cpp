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
#include <string>
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

//-----------------------------------------------------------------------------
// global variables and definitions
//-----------------------------------------------------------------------------
// our datetype
#define MIC_FLAG true //true if using microphone input, false if using Chuck input

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
//rbg color max
#define CMAX 255

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

// global audio buffer
SAMPLE g_fft_buffer[SND_FFT_SIZE];
GLint g_fft_size = SND_FFT_SIZE;
GLfloat g_window[SND_BUFFER_SIZE]; // DFT transform window

// for waterfall
struct Pt2D { float y;};
Pt2D ** g_spectrums = NULL;
GLuint g_depth = 64; //
GLfloat g_wf_prc = 0.7;
int g_len_hist = (int)round(g_depth*g_wf_prc);
// the index associated with the waterfall



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
    the_chuck->run(input, output, numFrames);

    // fill
    for( int i = 0; i < numFrames; i++ )
    {
        if(MIC_FLAG) {
            // copy the input to visualize only the left-most channel
            g_buffer[i] = input[i*MY_CHANNELS];
        }
        else if(!MIC_FLAG) {
            // copy in the output from chuck to our visualizer
            g_buffer[i] = output[i*MY_CHANNELS];
        }
        if(MIC_FLAG){
            // un-mute output
            for( int j = 0; j < MY_CHANNELS; j++ ) { output[i*MY_CHANNELS + j] = 0; }
        }
    }

    return 0;
}


//-----------------------------------------------------------------------------
// name: getChuckCode()
// desc: return chuck program formatted as a string
//-----------------------------------------------------------------------------
string getChuckCode() {

    string ck = ""
        // unit gen declarations
        "SinOsc foo => LPF lpf => ADSR e => NRev r_t => dac;"

        // time durations
        "800::ms => dur Q;"
        "3*Q => now;"

        // properties
        "0.1 => foo.gain;"
        //set dry/wet mix -> higher is more wet
        ".15 => r_t.mix;"
        // set cutoff freq
        "600 => lpf.freq;"
        // set param of env
        "e.set( 5::ms, 20::ms, .5, 30::ms );"   //(a,d,s height % of freq,r)

        // function
        "fun void playNote( Osc @ so, float pitch, dur T )"
        "{"
            //convert pitch to freq, set it
            "pitch => Std.mtof => so.freq;"   //midi to freq func from standard library (see programmer's guide)
            // press the key
            "e.keyOn();"
            // play/wait until beginning of release
            "T - e.releaseTime() => now;"
            //release the key
            "e.keyOff();"
            // wait until release is done
            "e.releaseTime() => now;"
        "}"
        // Shire Theme Song
        "playNote(foo, 60+12, Q/2); " //spork is saying the
        "playNote(foo, 62+12, Q/2); "
        "playNote(foo, 64+12, Q);"
        "playNote(foo, 67+12, Q);"
        "playNote(foo, 64+12, Q); "
        "playNote(foo, 62+12, Q/2);"
        "playNote(foo, 64+12, Q/4);"
        "playNote(foo, 62+12, Q/4);"
        "playNote(foo, 60+12, Q*2);" //spork is saying the
        "playNote(foo, 36, Q/2); "
        "playNote(foo, 64+12, Q/4); " //spork is saying the
        "playNote(foo, 67+12, Q/4); "
        "playNote(foo, 69+12, Q);"
        "playNote(foo, 72+12, Q);"
        "playNote(foo, 71+12, Q);"
        "playNote(foo, 67+12, Q);"
        "playNote(foo, 64+12, Q*1.5);"
        "playNote(foo, 65+12, Q/4);"
        "playNote(foo, 64+12, Q/4);" //spork is saying the
        "playNote(foo, 62+12, Q*2);" //spork is saying the
        "playNote(foo, 60+12, Q*2); " //spork is saying the
        "playNote(foo, 24, Q*2); " //spork is saying the
    "";

    return ck;
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
    the_chuck->setParam(CHUCK_PARAM_SAMPLE_RATE, MY_SRATE);
    the_chuck->setParam(CHUCK_PARAM_INPUT_CHANNELS, MY_CHANNELS);
    the_chuck->setParam(CHUCK_PARAM_OUTPUT_CHANNELS, MY_CHANNELS);
    the_chuck->init();
    the_chuck->compileCode(getChuckCode(), "");


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

    // initialize
    g_spectrums = new Pt2D*[g_len_hist];
    for( int i = 0; i < g_len_hist; i++ )
    {
        g_spectrums[i] = new Pt2D[SND_FFT_SIZE];
        memset( g_spectrums[i], 0, sizeof(Pt2D)*SND_FFT_SIZE );
    }
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
// Name: getIntersect( )
// Desc: draw time domain wave form and attach to mountain peak
//-----------------------------------------------------------------------------
GLfloat getIntersect(GLfloat y, GLfloat x1,GLfloat y1,GLfloat x2,GLfloat y2){
    GLfloat m = (y2-y1)/(x2-x1); //slope
    return((y-y1+m*x1)/m); //rearranged point slope form
}

//-----------------------------------------------------------------------------
// Name: drawSnowCap( )
// Desc: draw time domain wave form and attach to mountain peak
//-----------------------------------------------------------------------------
GLfloat bolt_rot = 1;
void drawSnowCap(GLfloat y_snow_min, GLfloat x1,GLfloat y1,GLfloat x2,GLfloat y2,GLfloat x3,GLfloat y3,GLfloat lw) {

    GLfloat xL = -1;
    GLfloat xR = -1;
    GLfloat x = -1;
    GLfloat inc = 1/g_bufferSize;
    GLfloat y_snow = -1;
    GLint nWaves = 6;

    for(int j=0; j<nWaves; j++) {
        y_snow = y_snow_min + ((GLfloat)j/nWaves)*(y2-y_snow);
        xL = getIntersect( y_snow , x1 , y1 , x2 , y2);     // starting point
        xR = getIntersect( y_snow , x2 , y2 , x3 , y3);      //ending point
        inc = ::fabs( (xR-xL) / g_bufferSize);      // increment
        x = xL;
        glLineWidth( lw );  // time domain line width
        glColor3f( 1.0, 1.0, 1.0 ); // color
        // apply the transform window
        //apply_window( (float*)g_buffer, g_window, g_bufferSize );
        // start primitive
        glBegin( GL_LINE_STRIP );
            // loop over buffer
            for(int i = 0; i < g_bufferSize; i++ )
            {
                if (::fabs(g_buffer[i])>=2.0) {

                    glColor3f(1.0,1.0,i/g_bufferSize); //yellow
                    //do i need to do this again??
                    glRotatef(bolt_rot,0,1,0); bolt_rot += 1;
                    glBegin( GL_LINE_STRIP );
                    glVertex2f( -2, -0.5*::fabs(g_buffer[i]+y_snow));
                    //CHANGE BACKGROUND COLOR TO WHITE
                }
                else {
                    glVertex2f( x, 0.2*g_buffer[i]+y_snow);
                }

                // plot
                // increment x
                x += inc;
            }
            // end primitive
        glEnd();
    }
}

//-----------------------------------------------------------------------------
// Name: drawMountain( )
// Desc: draw static mountain with given vertices
//-----------------------------------------------------------------------------
void drawMountain(GLfloat lw, GLfloat mx, GLfloat mb, GLfloat mh, GLfloat w, GLfloat y_off) {

    GLfloat x1 = mx-w, y1 = mb;
    GLfloat x2 = mx, y2 = mh+y_off;
    GLfloat x3 = mx+w, y3 = mb;
    GLfloat y_snow = (mh+y_off) - (mh+y_off-mb)/4;

    glLineWidth( lw ); //line width

    glBegin(GL_LINE_STRIP);
    {
        glColor3f( 0.0 , 0.05, 0.1); //aged blue green
        glVertex3f( x1,y1,0.1 );
        glColor3f( 1.0 , 1.0, 1.0); //white
        glVertex3f( x2,y2,0 );
        glColor3f( 0.0 , 0.05, 0.1); //aged blue green
        glVertex3f( x3,y3,-0.1 );
    }
    glEnd();

    drawSnowCap(y_snow,x1,y1,x2,y2,x3,y3,lw/16);
}

//-----------------------------------------------------------------------------
// Name: drawMoon( )
// Desc: draw static and time-domain affected moon
//-----------------------------------------------------------------------------
void drawMoon(float radius)
{
    //time domain moon rays
    glLineWidth(0.5);
    glBegin(GL_LINE_LOOP);
    for (int i=0; i<360; i++)
    {
      float degInRad = i*MY_PIE/180;
      glVertex2f(cos(degInRad)*radius+0.3*log(1+g_buffer[i])+4.4,0.3*log(1+g_buffer[i])+sin(degInRad)*radius+3.3);
    }
    glEnd();
    //static moon
    glLineWidth(1.0);
    glBegin(GL_LINE_LOOP);
    for (int i=0; i<360; i++)
    {
      float degInRad = i*MY_PIE/180;
      glVertex2f(cos(degInRad)*radius+4.4,sin(degInRad)*radius+3.3);
    }
    glEnd();
}

//-----------------------------------------------------------------------------
// Name: drawWaterfall( )
// Desc: draw fft history waterfall
//-----------------------------------------------------------------------------
void drawWaterfall() {

    //initialized local variables
    GLfloat x_anchor = -1, xinc = -1, xinc_scale = -1;
    GLfloat j_fl = -1;
    GLfloat x = -1, y = -1;
    GLfloat y_scale = -1, y_shift = -1;
    SAMPLE * buffer[g_bufferSize];
    // get the latest (possibly preview) window
    memset( buffer, 0, SND_FFT_SIZE * sizeof(SAMPLE) );
    // copy currently playing audio into buffer
    memcpy( buffer, g_buffer, g_bufferSize * sizeof(SAMPLE) );


    // take forward FFT; result in buffer as FFT_SIZE/2 complex values
    rfft( (float *)buffer, g_fft_size/2, FFT_FORWARD );
    // cast to complex
    complex * cbuf = (complex *)buffer;

    //shift memory down through spectrum array
    for (int k=g_len_hist-1; k>0; k--) {
        memcpy(g_spectrums[k], g_spectrums[k-1], g_bufferSize * sizeof(SAMPLE) );
    }

    //copy current buffer into spectrum mememory
    for (int i = 0; i < g_bufferSize; i++ ) {
        g_spectrums[0][i].y = cmp_abs(cbuf[i]);
    }

    // reset render starting point
    x_anchor = -7.0f;
    xinc = ::fabs(4*x_anchor / g_bufferSize); //set increment size

    //loop through waterfall depth
    for(int j=0; j<g_depth; j++){
        //set level-specific parameters
        j_fl = (float)j;
        x = x_anchor;
        y_scale = 5*(1+5*j_fl/g_depth);
        y_shift = 4*j_fl/g_depth;
        xinc_scale = 2.5*log(1+j_fl/g_depth);
        glLineWidth( 0.5+1.5*j_fl/g_depth);
        //begin line rendering
        glBegin( GL_LINE_STRIP );
        // loop through buffer
        for(int i = 0; i < g_bufferSize/2; i++ )
        {
            glColor3f( 0.0, (j_fl/g_depth)*(CMAX/g_depth)*(j_fl/CMAX), j_fl/g_depth*(1-(float)i/2/g_bufferSize) );
            if(j<g_len_hist) {
                //get history of fft y-values
                glVertex2f( x, y_scale * g_spectrums[g_len_hist-j-1][i].y - y_shift);
            }
            else if (j>=g_len_hist) {
                //get current buffer
                glVertex2f( x, y_scale * g_spectrums[0][i].y - y_shift);
            }

            // draw current spectrum

            x += xinc_scale*xinc;
        }
        glEnd();
    }
}

//-----------------------------------------------------------------------------
// Name: displayFunc( )
// Desc: callback function invoked to draw the client area
//-----------------------------------------------------------------------------
void displayFunc( )
{
        // local state
        static GLfloat zrot = 0.0f, c = 0.0f;
        static GLfloat mb = 0.0, mh = 3.0; //mountain base y, mountain height y
        static GLfloat mx1 = -4.0, mx2 = -2.5, mx3=-0.2, mx4=1.75, mx5 = 4.0, mx6 = 5.7;//mountain x vertices

        // clear the color and depth buffers
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // save current matrix state
                /******* DRAW STATIC MOUNTAINS & TIME DOMAIN *****************/
        glPushMatrix();
            drawMountain(8.0, mx1,mb,mh,2.5,0.0);
            drawMountain(12.0, mx2,mb,mh,2.5,-0.3);
            drawMountain(18.0, mx3,mb-0.1,mh,3.0,0.8);
            drawMountain(9.0, mx4,mb-0.2,mh,3.5,0.3);
            drawMountain(15.0, mx5,mb-0.5,mh,3.0,-0.6);
            drawMountain(30.0, mx6,mb-1.1,mh,2.0,-0.2);
            drawMoon(0.4);
            drawWaterfall();
        glPopMatrix();
        // flush!
        glFlush( );
        // swap the double buffer
        glutSwapBuffers( );

}
