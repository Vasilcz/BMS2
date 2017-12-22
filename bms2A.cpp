/*
 * File:   bms1A.cpp
 */

#include <iostream>
#include <cmath>

#include "sndfile.hh"

#define SAMPLE_RATE 18000
#define CHANELS 1
#define FORMAT (SF_FORMAT_WAV | SF_FORMAT_PCM_24)
#define AMPLITUDE (1.0 * 0x7F000000)
#define FREQ (1000.0 / SAMPLE_RATE)
#define SYN_SEQ "00110011"
#define SYN_SEQ_LEN 8
#define SAMPLES 30

using namespace std;

int discrete_time = 0;

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

/**
 * QPSK modulace
 * @param sym1 1.symbol
 * @param sym2 2.symbol
 * @param buffer
 */
void modulation(int sym1, int sym2, int *buffer) {

    double shiftPhase = get_phase_shift(sym1, sym2);

    for (int i = 0; i < SAMPLES; i++) {
        buffer[i] = AMPLITUDE * sin(2 * M_PI * FREQ * discrete_time + shiftPhase);
        discrete_time++; // increment diskretniho casu
    }

}


int main(int argc, char** argv) {

    if(argc != 2) {
        cerr << "Chyba! Zadejte nazev souboru!" << endl;
        exit(1);
    }

    SndfileHandle outputFile;
    int *buffer = new int[SAMPLES];
    string outputFileName, inputFileName;

    inputFileName = argv[1];

    /* cteni nazvu souboru dokud se nenarazi na . */
    for(auto &c: inputFileName) {
        if(c == '.')
            break;
        outputFileName.push_back(c);
    }

    /* Pridani nove pripony */
    outputFileName += ".wav";
    outputFile = SndfileHandle(outputFileName.data(), SFM_WRITE, FORMAT, CHANELS, SAMPLE_RATE);


    /* modulace a zapis synchronizacni sekvence */
    for(int i = 0; i < SYN_SEQ_LEN; i += 2) {
        modulation(SYN_SEQ[i] - '0', SYN_SEQ[i+1] - '0', buffer);
        outputFile.write(buffer, SAMPLES);
    }

    int ch1=0, ch2=0;
    FILE *file;
    file = fopen(inputFileName.data(), "r");

    if(file == NULL)
        cerr << "Nepodarilo se otevrit vstupni soubor!" << endl;
    else {
        /* nacitani ze vstupniho souboru*/
        while(ch1 != EOF || ch2 != EOF) {
            ch1 = fgetc(file);
            ch2 = fgetc(file);

            if((ch1 == '1' || ch1 == '0') && (ch2 == '1' || ch2 == '0')) {
                /* modulace a zapis sekvenci ze souboru*/
                modulation(ch1 - '0', ch2 - '0', buffer);
                outputFile.write(buffer, SAMPLES);
            }
        }
    }
    fclose(file);

    delete [] buffer;
    return EXIT_SUCCESS;
}