#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <csignal>
#include <complex>
#include <cmath>
#include <unistd.h>
#include <thread>
#include <portaudio.h>
#include "../fft/FFT.h"

#define DEFAULT_SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 1536
#define NUM_CHANNELS 2
#define SAMPLE_SILENCE 0.0f

using namespace std;

static float *samples;
static PaStream *stream;
int do_exit = 0;


static int callback_function( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    if(!do_exit){
        float *data = (float*)userData;
        const float *rptr = (const float*)inputBuffer;
        float *wptr = &data[0];
        long framesToCalc;
        long i;
        int finished;

        (void) outputBuffer;
        (void) timeInfo;
        (void) statusFlags;


        if( inputBuffer == NULL ){
            for( i=0; i<framesPerBuffer; i++ ){
                *wptr++ = SAMPLE_SILENCE;
                if( NUM_CHANNELS == 2 ) *wptr++ = SAMPLE_SILENCE;
            }
        }
        else        {
            for( i=0; i<framesPerBuffer; i++ ){
                *wptr++ = *rptr++;
                if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;
            }
        }
        return paContinue;
    }else{
        return paComplete;
    }
}

void normalize(complex<double> *result,double *final, int N, ofstream *file_out){
    /* Normalize FFT results between 0 and 256 */

    int i;
    double max = 0;

    for(i=0; i< N; i++){
        final[i] = abs(result[i]);
        if(final[i] > max){
            max = final[i];
        }
    }

    double segment = max/256;

    for(i=0;i<N;i++){
        final[i] = final[i] / segment;
        *file_out << final[i] << endl;;
    }
}

void plot(int N){
    int i;
    complex<double> *samps = new complex<double>[N];
    complex<double> *result = new complex<double>[N];
    ofstream file_out("audio_data.dat", ios_base::out | ios_base::trunc);
    double *final = new double[N];
    string gnuplot("gnuplot -persist");
    bool plotted = false;

    //Setting gnuplot
    FILE* gp1 = popen(gnuplot.c_str(), "w");
    fprintf(gp1, "set term qt\nset xrange [0:400]\nset yrange [0:256]\nset grid\nset title 'Power Spectral Density'\nset xlabel 'Frequency [Hz]'\nset ylabel 'Magnitude'\nset style fill solid\n");
    fflush(gp1);
    
    while(true){
        if(!do_exit){


            for(i=0;i<N;i++){
                samps[i] = complex<double>(samples[i],0);
            }

            result = FFT(samps,N);

            file_out.clear();
            file_out.seekp(0);

            normalize(result, final, N/2, &file_out);


            if(!plotted){
                fprintf(gp1, "plot [0:400] 'audio_data.dat' w filledcurve x1 lw 2 lc 'red'\n");
                fflush(gp1);
                plotted = true;
            }
            else{
                fprintf(gp1, "replot\n");
                fflush(gp1);
            }
        }else{
            file_out.close();
            pclose(gp1);
            break;  
        }
    }
    return;

}

int error(PaError err){
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return -1;
}

void signal_callback_handler(int signal){
    PaError err;

    cout<< "Interrupt received..." <<endl;
    do_exit = 1;
    sleep(0.05);
    
    err = Pa_CloseStream( stream );
    if( err != paNoError ) error(err);

    Pa_Terminate();

    cout << "Terminated!" <<endl;
    return;
}


int main(){

    int i;
    PaError err;
    PaStreamParameters inputParameters;
    int numSamples = FRAMES_PER_BUFFER * NUM_CHANNELS;
    int numBytes = numSamples * sizeof(float);
    samples = new float[numBytes];

    //Ctrl + C Handler
    signal(SIGINT, signal_callback_handler);

    if(samples == NULL){
        cerr << "Error in allocating memory" << endl;
        exit(1);
    }

    for( i=0; i<numBytes; i++ ) samples[i] = 0;

    thread T1(plot, FRAMES_PER_BUFFER);

    /* ------------ PortAudio ------------ */
    err = Pa_Initialize();
    if( err != paNoError ) error(err);

    /* Initializing input device */

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
      fprintf(stderr,"Error: No default input device.\n");
      error(err);
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              NULL,             
              DEFAULT_SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,
              callback_function,
              samples);

    if( err != paNoError ) error(err);

    err = Pa_StartStream( stream );
    if( err != paNoError ) error(err);
    cout << "Now recording... !" << endl;

    T1.join();
    return 0;
}