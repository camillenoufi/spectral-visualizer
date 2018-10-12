Clarinet clair => JCRev r => dac;
Clarinet c => r => dac;
.75 => r.gain;
.1 => r.mix;

// the score: midi notes
[ 61, 63, 65, 66, 68, 66, 65, 63, 61 ] @=> int treb[];
[ 61, 63, 65, 66, 68, 66, 65, 63, 61 ] @=> int bass[];

//beat
300::ms => dur Q;

// infinite time-loop
while( true )
{
    Math.random2f( 64, 128 ) => float stiffness;
    Math.random2f( 0, 128 ) => float noisegain;
    Math.random2f( 0, 128 ) => float vibratofreq;
    Math.random2f( 0, 128 ) => float vibratogain;
    Math.random2f( 64, 128 ) => float pressure;

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

    for( int i; i < treb.cap(); i++ )
    {
        spork ~ play(clair, bass[i], .8, Q); play(c, 5+treb[i], Math.random2f(.6,.9),Q);
    }
    3::second => now;
}

// basic play function (add more arguments as needed)
fun void play( Clarinet @ cl, float note, float velocity, dur T )
{
    // start the note
    Std.mtof( note ) => cl.freq;
    velocity => cl.noteOn;
    T => now;
}
