/*

Document    : FFT.h
Language    : C++
Last modify : 24/09/20
@author     : Andrea Di Mauro
Description : This is my very first project: I wrote an FFT algorithm
              using only my university knowledge.

*/

#ifndef FFT_H
#define FFT_H

#include <iostream>
#include <vector>
#include <fstream>
#include <thread>
#include <cmath>
#include <complex>

const double PI = acos(-1);
using namespace std;

void print_complex(complex<double> *value){
    cout << "(" << real(*value) << ")+i(" << imag(*value) << ")" << endl;
}

/* --------- FFT Calc functions --------- */

complex<double> w_N(double k, int N){
    return exp((-1i)*2.0*PI*(k/N));
}

void sum1(complex<double> *value, double k,complex<double> *samples, const int *N){
    for(int c =0; c<(*N/4);c++){
        *value = *value + samples[(4*c)]*exp((-1i)*2.0*PI*((k*c)/((*N)/4)));
    }
}

void sum2(complex<double> *value, double k,complex<double> *samples, const int *N){
    for(int c =0; c<(*N/4);c++){
        *value = *value + samples[4*c+2]*exp((-1i)*2.0*PI*((k*c)/((*N)/4)));
    }
}

void sum3(complex<double> *value, double k,complex<double> *samples, const int *N){
    for(int c =0; c<(*N/4);c++){
        *value = *value + samples[4*c+1]*exp((-1i)*2.0*PI*((k*c)/((*N)/4)));
    }
}

void sum4(complex<double> *value, double k,complex<double> *samples, const int *N){
    for(int c =0; c<(*N/4);c++){
        *value = *value + samples[4*c+3]*exp((-1i)*2.0*PI*((k*c)/((*N)/4)));
    }
}

/* --------- Threads --------- */

void thread_1(complex<double> *Xpp, complex<double> *samples, complex<double> *result, int *N){
    int a = 0;
    double aa=1;
    for(int i = 0; i<*N;i++, a++){

        if(i<(*N/4)){
            sum1(&Xpp[i], i, samples, N);
        }
        if(i % (*N/4) == 0){ a=0;};
        result[i] = aa*Xpp[a];
    }    
}

void thread_2(complex<double> *Xpd, complex<double> *wn2, complex<double> *samples, complex<double> *result, int *N){
    double bb=-1; 
    int a=0,b=0;

    for(int i = 0; i<*N;i++, a++, b++){
        
        if(i<(*N/4))
        {
            wn2[i] = w_N(i*2,*N);
            sum2(&Xpd[i], i, samples, N);
        }

        if(i % (*N/4) == 0){ a=0; b=0, bb=bb*-1;};
        
        result[i] = bb*wn2[b]*Xpd[a];
    }
}

void thread_3(complex<double> *Xdp, complex<double> *wn1, complex<double> *samples, complex<double> *result, int *N){
    double cc=-1; 
    int a=0,c=0;

    for(int i = 0; i<*N;i++, a++, c++){
        
        if(i<(*N/2)){
            wn1[i] = w_N(i,*N);
        }
        if(i<(*N/4)){
            sum3(&Xdp[i], i, samples, N);
        }

        if(i % (*N/4) == 0){ a=0;};
        if(i % (*N/2) == 0){ c=0, cc=cc*-1;};
        result[i] = cc*wn1[c]*Xdp[a];
    }
}

void thread_4(complex<double> *Xdd, complex<double> *wn3, complex<double> *samples, complex<double> *result, int *N){
    double dd=-1; 
    int a=0,d=0;

    for(int i = 0; i<*N;i++, a++, d++){
        
        if(i<(*N/6)){
            wn3[i] = w_N(i*3,*N);
        }
        if(i<(*N/4)){
            sum4(&Xdd[i], i, samples, N);
        }

        if(i % (*N/4) == 0){ a=0;};
        if(i % (*N/6) == 0){ d=0, dd=dd*-1;};
        result[i] = dd*wn3[d]*Xdd[a];
    }

}

/* --------- FFT --------- */

void FFT(complex<double> *samples, complex<double> *result, int N){

    if((N % 2 != 0) || (N % 4 != 0) || (N % 6 != 0)){
        cerr << "Error: total number of samples must be divisible per 2,4 and 6" << endl;
    }

    //complex<double> *result = new complex<double>[N];
    complex<double> *result1 = new complex<double>[N];
    complex<double> *result2 = new complex<double>[N];
    complex<double> *result3 = new complex<double>[N];
    complex<double> *result4 = new complex<double>[N];
    complex<double> *wn2 = new complex<double>[N/4];
    complex<double> *wn1 = new complex<double>[N/2];
    complex<double> *wn3 = new complex<double>[N/6];
    complex<double> *Xpp = new complex<double>[N/4];
    complex<double> *Xpd = new complex<double>[N/4];
    complex<double> *Xdp = new complex<double>[N/4];
    complex<double> *Xdd = new complex<double>[N/4];

    thread T1(thread_1, Xpp, samples, result1, &N);
    thread T2(thread_2, Xpd, wn2, samples, result2, &N);
    thread T3(thread_3, Xdp, wn1, samples, result3, &N);
    thread T4(thread_4, Xdd, wn3, samples, result4, &N);
    T1.join();
    T2.join();
    T3.join();
    T4.join();

    for(int i = 0; i < N; i++){
        result[i] = result1[i] + result2[i] + result3[i] + result4[i];
    }

    /* IN CASE YOU WANT TO PRINT THE RESULT UNCOMMENT HERE*/
    // for(int count = 0; count < N; count++){
    //     cout<<to_string(count+1) + " - ";
    //     print_complex(&result[count]);
    // }

    delete result1;
    delete result2;
    delete result3;
    delete result4;
    delete wn2;
    delete wn1;
    delete wn3;
    delete Xpp;
    delete Xpd;
    delete Xdp;
    delete Xdd;
   
    return;
}

#endif