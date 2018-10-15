// == "VisualSine generated sound narrative"
// == Camille Noufi 
// == Last modified: 10-14-2018


//============ SETUP ======================= 

//beat
500::ms => dur Q;

//bass line clarinet ugen
Clarinet cb => JCRev rb => dac;
.2 => rb.gain;
.2 => rb.mix;
.5 => float vel_bass;
Math.random2f( 63, 128 ) => float stiffness;
Math.random2f( 0, 128 ) => float noisegain;
Math.random2f( 0, 128 ) => float vibratofreq;
Math.random2f( 0, 128 ) => float vibratogain;
Math.random2f( 63, 128 ) => float pressure;
// clear
cb.clear( 1.0 );
// reed stiffness
cb.controlChange( 2, 90 );
// noise gain
cb.controlChange( 4, 40 );
// vibrato freq
cb.controlChange( 11, 65 );
// vibrato gain
cb.controlChange( 1, 40 );
// breath pressure
cb.controlChange( 128, 80 );


//melody line clarinet ugen
Clarinet ct => JCRev rt => dac;
.5 => rt.gain;
.1 => rt.mix;
.4 => float vel_treb;

Math.random2f( 63, 128 ) => stiffness;
Math.random2f( 0, 128 ) => noisegain;
Math.random2f( 0, 128 ) => vibratofreq;
Math.random2f( 0, 128 ) => vibratogain;
Math.random2f( 63, 128 ) => pressure;
// clear
ct.clear( 1.0 );
// reed stiffness
ct.controlChange( 2, 63 );
// noise gain
ct.controlChange( 4, 80 );
// vibrato freq
ct.controlChange( 11, 120 );
// vibrato gain
ct.controlChange( 1, 70 );
// breath pressure
ct.controlChange( 128, 63 );

// new class to manage envelopes
class Env
{
    Step s => Envelope e => blackhole; // feed constant into env
    200::ms => e.duration; // set ramp time
    fun void target (float val) { 
        e.target(val); 
    }
}
Env amp;

//============ THE MIDI SCORE ======================= 

// the score: midi notes
//Verse 1
[ 60, 62, 63, 67, 63, 62, 63, 62, 60, 60, 63, 67, 68, 72, 71, 67, 63, 65, 63, 62, 60, 60 ] @=> int treb_v1[];
[ 60, 62, 63, 67, 63, 62, 63, 62, 60, 60, 63, 67, 68, 72, 71, 67, 63, 65, 63, 62, 60, 60 ] @=> int bass_v1[];
[ .5,.5,1,1,1,.5,.25,.25,2,.5,.25,.25,1,1,1,1,1.5,.25,.25,2,2,2 ] @=> float dura_v1[];

//Verse 2
[ 60, 62, 63, 62, 60 ] @=> int treb_v2[];
[ 63, 64, 65, 64, 63 ] @=> int bass_v2[];
[ 1.0,1.0,1.0,1.0,2.0 ] @=> float dura_v2[];

//Part 1
//C minor triad melody
[ 60+12, 62+12, 63+12, 62+12, 65+12, 62+12, 63+12, 62+12 ] @=> int treb1[]; // with f top
[ 60+12, 62+12, 63+12, 62+12, 67+12, 62+12, 63+12, 62+12 ] @=> int treb2[]; //with g top
[ 60+12, 62+12, 63+12, 62+12, 68+12, 62+12, 63+12, 62+12 ] @=> int treb21[]; //with ab top
[ 60+12, 62+12, 63+12, 62+12, 71+12, 68+12, 63+12, 62+12 ] @=> int treb22[]; //with b top
//bass harmonies
[ 48, 48, 48, 48, 48, 48, 48, 48 ] @=> int bass_c48[];
[ 43, 43, 43, 43, 43, 43, 43, 43 ] @=> int bass_g43[];
[ 51, 51, 51, 51, 51, 51, 51, 51 ] @=> int bass_eb51[];
[ 50, 50, 50, 50, 50, 50, 50, 50 ] @=> int bass_d50[];
[ 55, 55, 55, 55, 55, 55, 55, 55 ] @=> int bass_g55[];
[ 56, 56, 56, 56, 56, 56, 56, 56 ] @=> int bass_ab56[];
[ 59, 59, 59, 59, 59, 59, 59, 59 ] @=> int bass_b59[];

[ 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5 ] @=> float dur_8th[];
[ 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25 ] @=> float dur_16th[];

//Part 2
[72, 74, 75, 77, 79, 80, 83, 84] @=> int treb3[]; //cm ascending scale
[60, 60, 60, 60, 55, 51, 50, 48 ] @=> int bass3[];
[83, 84, 83, 84, 83, 84, 83, 84, 83, 84, 83, 84, 83, 84, 83, 84 ] @=> int treb4[]; //b-c trill
[60, 60, 60, 60, 51, 51, 51, 51, 53, 53, 53, 53, 50, 50, 50, 50 ] @=> int bass4[];
[74, 75, 77, 79, 80, 83, 84, 86] @=> int treb5[]; // d dorian ascending scale
[55, 55, 50, 50, 53, 53, 47, 47 ] @=> int bass5[];
[84, 86, 84, 86, 84, 86, 84, 86, 84, 86, 84, 86, 84, 86, 84, 86] @=> int treb6[]; // c-d trill
[87, 86, 87, 86, 87, 86, 87, 86, 87, 86, 87, 86, 87, 86, 87, 86] @=> int treb7[]; // d-eb trill
[89, 87, 89, 87, 89, 87, 89, 87, 89, 87, 89, 87, 89, 87, 89, 87] @=> int treb8[]; // eb-f trill
[60, 60, 60, 60, 59, 59, 59, 59, 62, 62, 62, 62, 59, 59, 59, 59] @=> int bass6[]; // bass for trills
[86, 86, 86, 86, 86, 86, 86, 86] @=> int treb_g90[];
[79, 79, 79, 79, 55, 55, 55, 55] @=> int bass_g[];
//[2.0,2.0] @=> float dur_half[];

[90, 87, 86, 84, 80, 77, 74, 71] @=> int treb_desc[];
[72, 74, 76, 78, 80, 82, 84, 84] @=> int treb_whole[];
[72, 72, 72, 72, 72, 72, 72, 72 ] @=> int treb_c72[];
[0.5, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0] @=> float dur_end[];






//============ FUNCTIONS TO CALL ======================= 
fun void construct_phrase(int bass_line[], int treb_line[], float vb, float vt, float dur_line[], float init_gain)
{
    0 => int note;
    init_gain => cb.gain;
    0.05 => ct.gain;
    
    for( int i; i < treb1.cap(); i++ )
    {
        spork ~ play(cb, bass_line[i], vb, dur_line[i]*Q); play(ct, treb_line[i], vt, dur_line[i]*Q);
        if(treb_line[i] >= note) {
            treb_line[i] => note;
            init_gain + 0.05 => init_gain;
        }
        else if(treb_line[i] < note) {
            init_gain - 0.05 => init_gain;
        }
        init_gain => cb.gain;
        init_gain => ct.gain;
    }
}

// basic play function (add more arguments as needed)
fun void play( Clarinet @ cl, float note, float velocity, dur T )
{
    // start the note
    Std.mtof( note ) => cl.freq;
    velocity => cl.noteOn;
    T => now;
}


//============ MAIN ROUTINE ======================= 
while (true) {
    // mm1-4
    construct_phrase(bass_c48, treb1, vel_bass, vel_treb, dur_8th, 0.05);
    construct_phrase(bass_g43, treb1, vel_bass, vel_treb, dur_8th, 0.1);
    construct_phrase(bass_eb51, treb1, vel_bass, vel_treb, dur_8th, 0.2);    
    construct_phrase(bass_d50, treb1, vel_bass, vel_treb, dur_8th, 0.05);
    // mm5-8
    construct_phrase(bass_c48, treb2, vel_bass, vel_treb, dur_8th, 0.01);
    construct_phrase(bass_g55, treb2, vel_bass, vel_treb, dur_8th, 0.1);
    construct_phrase(bass_ab56, treb21, vel_bass, vel_treb, dur_8th, 0.3);
    construct_phrase(bass_b59, treb22, vel_bass, vel_treb, dur_8th, 0.4);
    // mm9-12
    construct_phrase(bass3, treb3, vel_bass, vel_treb, dur_8th, 0.01);
    construct_phrase(bass4, treb4, vel_bass, vel_treb, dur_16th, 0.2); construct_phrase(bass4, treb4, vel_bass, vel_treb, dur_16th, 0.3);
    construct_phrase(bass5, treb5, vel_bass, vel_treb, dur_8th, 0.05);
    construct_phrase(bass6, treb6, vel_bass, vel_treb, dur_16th, 0.02); construct_phrase(bass6, treb6, vel_bass, vel_treb, dur_16th, 0.03);
    // mm13-16
    construct_phrase(bass3, treb3, vel_bass, vel_treb, dur_8th, 0.01);
    construct_phrase(bass4, treb4, vel_bass, vel_treb, dur_16th, 0.2); construct_phrase(bass4, treb4, vel_bass, vel_treb, dur_16th, 0.3);
    construct_phrase(bass6, treb6, vel_bass, vel_treb, dur_16th, 0.35); construct_phrase(bass6, treb7, vel_bass, vel_treb, dur_16th, 0.40);
    construct_phrase(bass6, treb8, vel_bass, vel_treb, dur_16th, 0.45); 
    //mm17-20
    construct_phrase(bass_g, treb_g90, vel_bass, vel_treb, dur_8th, 0.4);
    construct_phrase(treb_desc, treb_desc, vel_bass, vel_treb, dur_8th, 0.3);
    //mm21-24
    construct_phrase(bass_c48, treb1, vel_bass, vel_treb, dur_8th, 0.05);
    construct_phrase(bass_g43, treb2, vel_bass, vel_treb, dur_8th, 0.1);
    construct_phrase(bass_ab56, treb21, vel_bass, vel_treb, dur_8th, 0.2);
    construct_phrase(bass_eb51, treb1, vel_bass, vel_treb, dur_8th, 0.15);
    //mm25-28
    construct_phrase(bass_c48, treb1, vel_bass, vel_treb, dur_8th, 0.1);
    construct_phrase(bass_g43, treb2, vel_bass, vel_treb, dur_8th, 0.05);
    construct_phrase(bass_c48, treb_whole, vel_bass, vel_treb, dur_end, 0.01);
    
    //fade out
    0.4 => float init_gain;
    for( int i; i < 8; i++ )
    {
        spork ~ play(cb, bass_c48[i], 0.2, dur_8th[i]*Q); play(ct, treb_c72[i], 0.2, dur_8th[i]*Q);
        init_gain - 0.05 => init_gain;
        init_gain => cb.gain;
        init_gain => ct.gain;
    }
 
        
    Q => now;
   
}    

