"Nature at Night" Audio Visualizer
- Visualizes the time and frequency contents of any sound

to run:

after downloading the repo:
1) navigate to "code" folder in the terminal
2) once in folder, type:
2.a) make
2.b) ./VisualSine
3) type Q or q to exit the running program

to utilize microphone input or preset sound narrative:
1) navigate to /code/host/VisualSine.cpp
2) open in editor
3) line 58:  #define MIC_FLAG false     //true if using microphone input, false if using pre-set sound narrative
