// CHuck tutorial 10/3

// start vm by 'cmd .'
// /* */ is block command

// THIS IS A patch
//adsr is envelope, lpf 2nd order butterworth, NRev is a reverb convolution

SqrOsc foo => LPF lpf => ADSR e => NRev r => dac;
SqrOsc bar => lpf;
.2 => foo.gain;
.1=> bar.gain;
//set dry/wet mix -> higher is more wet
.2 => r.mix;
// set cutoff freq
500 => lpf.freq;
// set param of env
e.set( 10::ms, 5::ms, .5, 20::ms );   //(a,d,s height % of freq,r)

200::ms => dur Q;

while(true)
{
    // cmin7 in 6/8 time
    spork ~ playNote(bar, 72, Q); playNote( foo, 60, Q ); //spork is saying the
    spork ~ playNote(bar, 70, Q); playNote( foo, 63, Q ); 
    spork ~ playNote(bar, 67, Q); playNote( foo, 67, Q ); 
    spork ~ playNote(bar, 63, Q); playNote( foo, 70, Q ); 
    spork ~ playNote(bar, 60, Q); playNote( foo, 72, Q ); 
    Q => now;
}

// function
fun void playNote( Osc @ so, float pitch, dur T )
{
    //convert pitch to freq, set it
    pitch => Std.mtof => so.freq;   //midi to freq func from standard library (see programmer's guide)
    // press the key
    e.keyOn();
    // play/wait until beginning of release
    T - e.releaseTime() => now;
    //release the key
    e.keyOff();
    // wait until release is done
    e.releaseTime() => now;
}
