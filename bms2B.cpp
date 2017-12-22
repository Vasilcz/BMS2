/*
 * #TODO Funkcni pouze synchronizace
 */

#include <cstdlib>
#include <cmath>

#include "sndfile.hh"
#include <iostream>

#define CARRIER_FREQ 1000.0
#define AMPLITUDE (1.0 * 0x7F000000)


using namespace std;

enum synchronizationState {
    state00,
    state11,
};

int discrete_time = 0, samples = 0;

double get_phase_shift(int sym1, int sym2) {
    if(!sym1 && sym2)          //01 - 45°
        return (M_PI / 4);
    else if(!sym1 && !sym2)      //00 - 135°
        return ((3 * M_PI) / 4);
    else if(sym1 && sym2)      //11 - 315°
        return ((7 * M_PI) / 4);
    else if(sym1 && !sym2)       //10 - 225°
        return ((5 * M_PI) / 4);
}

double f1(int x, int *buff, double freq) {
    return (buff[x] * cos(2*M_PI*freq*x));
}

double f2(int x, int *buff, double freq) {
    return (buff[x] * -1.0 * sin(2*M_PI*freq*x));
}

double integrate(int a, int b, int steps, int *buff, double freq, int even)
{
    double s = 0;
    double h = (b-a)/steps;
    for (int i = 0; i < steps; ++i)
        if(even)
            s += f1(a + h*i, buff, freq);
        else
            s += f2(a + h*i, buff, freq);
    return h*s;
}

int odd_bits(int *buffer, double frequency) {
    double bit;

    bit = integrate(0, samples, samples, buffer, frequency, false);

    if(bit < 0)
        return 0;
    else
        return 1;
}

int even_bits(int *buffer, double frequency) {
    double bit;

    bit = integrate(0, samples, samples, buffer, frequency, true);

    if(bit < 0)
        return 0;
    else
        return 1;
}

void demodulation(const int *buffer, double frequency, int frames) {
    int pom[samples];
    int oddBit, evenBit;

    for(; discrete_time < frames; discrete_time+=samples) {
        for(int i = 0; i < samples; i++) {
            pom[i] = buffer[discrete_time + i];
            cout << "POM: " << pom[i]  << endl;
        }

        oddBit = odd_bits(pom, frequency);
        evenBit = even_bits(pom, frequency);
        cout << "\tDISCR: " << discrete_time << "\tODD: " << oddBit << "\tEVEN: " << evenBit << endl;

    }

}

/**
 * Synchronizacni cast, synchronizacni sekvence 00110011
 * @param buffer
 * @param frequency
 */
void synchronization(int *buffer, double frequency) {
    double referenceCosWave, receivedCosWave;
    int count00 = 0, count11 = 0, first00 = true, first11 = true, second00 = false;
    synchronizationState synState = state00;
    int cond = 1;

    while(cond) {
        switch(synState) {
            case state00:
                referenceCosWave = sin(2 * M_PI * frequency * discrete_time + get_phase_shift(0, 0));
                receivedCosWave = buffer[discrete_time] / AMPLITUDE;

                if(fabs(referenceCosWave - receivedCosWave) < 0.1) {
                    if (first00)
                        samples++;
                    else
                        count00++;

                    discrete_time++;
                }
                else {
                    if(!first00 && (samples != count00)) {
                        cerr << "Spatna synchronizace!" << endl;
                        exit(1);
                    }

                    synState = state11;

                    if(first00)
                        first00 = false;
                    else
                        second00 = true;
                }


                break;

            case state11:
                referenceCosWave = sin(2 * M_PI * frequency * discrete_time + get_phase_shift(1, 1));
                receivedCosWave = buffer[discrete_time] / AMPLITUDE;

                if(fabs(referenceCosWave - receivedCosWave) < 0.1) {
                    count11++;
                    discrete_time++;
                }
                else {
                    if(samples != count11) {
                        cerr << "Spatna synchronizace!" << endl;
                        exit(1);
                    }
                    synState = state00;
                    count11 = 0;

                    if(first11)
                        first11 = false;

                    if(second00)
                        cond = 0;
                }

                break;
        }
    }
}

int main(int argc, char** argv) {

    SndfileHandle inputFile;
    int sampleRate, frames;
    double frequency;
    int *buffer;

    if(argc != 2) {
        cerr << "Neplatny pocet parametru! Pouziti: ./bms2B file.wav" << endl;
        exit(1);
    }

    inputFile = SndfileHandle(argv[1]);

    sampleRate = inputFile.samplerate();
    frequency = CARRIER_FREQ / sampleRate; // vypocet frekvence
    frames = inputFile.frames();

    buffer = new int[frames];

    inputFile.read(buffer, frames);

    synchronization(buffer, frequency);
    demodulation(buffer, frequency, frames);


    delete [] buffer;
    return EXIT_SUCCESS;
}

