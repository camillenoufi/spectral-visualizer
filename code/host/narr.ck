Clarinet ct => JCRev r => dac;
Clarinet clair => r => dac;
.75 => r.gain;
.1 => r.mix;

//beat
400::ms => dur Q;

// the score: midi notes
//Verse 1
[ 60, 62, 63, 67, 63, 62, 63, 62, 60, 60, 63, 67, 68, 72, 71, 67, 63, 65, 63, 62, 60, 60 ] @=> int treb_v1[];
[ 60, 62, 63, 67, 63, 62, 63, 62, 60, 60, 63, 67, 68, 72, 71, 67, 63, 65, 63, 62, 60, 60 ] @=> int bass_v1[];
[ .5,.5,1,1,1,.5,.25,.25,2,.5,.25,.25,1,1,1,1,1.5,.25,.25,2,2,2 ] @=> float dura_v1[];

//Verse 2
[ 60, 62, 63, 62, 60 ] @=> int treb_v2[];
[ 63, 64, 65, 64, 63 ] @=> int bass_v2[];
[ 1.0,1.0,1.0,1.0,2.0 ] @=> float dura_v2[];


// infinite time-loop
while( true )
{
    Math.random2f( 63, 128 ) => float stiffness;
    Math.random2f( 0, 128 ) => float noisegain;
    Math.random2f( 0, 128 ) => float vibratofreq;
    Math.random2f( 0, 128 ) => float vibratogain;
    Math.random2f( 63, 128 ) => float pressure;

    //<<< "---", "" >>>;
    //<<< "reed stiffness:", stiffness >>>;
    //<<< "noise gain:", noisegain >>>;
    //<<< "vibrato freq:", vibratofreq >>>;
    //<<< "vibrato gain:", vibratogain >>>;
    //<<< "breath pressure:", pressure >>>;

    // clear
    clair.clear( 1.0 );

    // reed stiffness
    clair.controlChange( 2, stiffness );
    // noise gain
    clair.controlChange( 4, noisegain );
    // vibrato freq
    clair.controlChange( 11, vibratofreq );
    // vibrato gain
    clair.controlChange( 1, vibratogain );
    // breath pressure
    clair.controlChange( 128, pressure );

    //
    for( int i; i < treb_v1.cap(); i++ )
    {
        spork ~ play(clair, bass_v1[i], .8, dura_v1[i]*Q); play(ct, treb_v1[i], Math.random2f(.6,.9), dura_v1[i]*Q);
    }
    1::second => now;
    
    for( int i; i < treb_v2.cap(); i++ )
    {
        spork ~ play(clair, bass_v2[i], .8, dura_v2[i]*Q); play(ct, treb_v2[i], Math.random2f(.6,.9), dura_v2[i]*Q);
    }
    1::second => now;
    
}

// basic play function (add more arguments as needed)
fun void play( Clarinet @ cl, float note, float velocity, dur T )
{
    // start the note
    Std.mtof( note ) => cl.freq;
    velocity => cl.noteOn;
    T => now;
}
