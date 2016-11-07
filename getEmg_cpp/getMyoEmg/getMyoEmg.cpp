// Modified version of emg-data-sample.cpp: Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.
#include "stdafx.h"
#include <array>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <stdio.h>
#include <Windows.h>
#include <inttypes.h>

#include <myo/myo.hpp>

#define EMG_FILE "emg.txt"
#define CLOSE_COUNT_MAX 20 // Close-Open file after this many interrupts to expediate reading on the other side #No_hacks

FILE * emgFile; // Globals is cleanest solution for speed
FILETIME ft_now;
int closeCounter = 1;

class DataCollector : public myo::DeviceListener {
public:
    DataCollector() : emgSamples() {
    }

    /* On user disconnect */
    void onUnpair(myo::Myo* myo, uint64_t timestamp) {
        emgSamples.fill(0);
    }

    /* EMG data interrupt - save data and print to text file */
    void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg) {
        unsigned long long temp_now, ll_sec, ll_ms;
        for (int i = 0; i < 8; i++) {
            emgSamples[i] = emg[i];
        }

        GetSystemTimeAsFileTime(&ft_now); // Time grabbed from windows since myo SDK specifies timestamp as from (unspecified) time 
        temp_now = ((LONGLONG)ft_now.dwLowDateTime + ((LONGLONG)(ft_now.dwHighDateTime) << 32LL) - 116444736000000000LL); // time since UNIX epoch in 100ns resolution
        ll_sec = temp_now / 10000000; // Get seconds (integer division)
        ll_ms = (temp_now / 10000) - (ll_sec * 1000); // Get difference caused by truncation in integer division

        fprintf(emgFile, "%4i,%4i,%4i,%4i,%4i,%4i,%4i,%4i,%10" PRIu64 ".%03" PRIu64 "\n", emg[0], emg[1], emg[2], emg[3], emg[4], emg[5], emg[6], emg[7], ll_sec, ll_ms);

        closeCounter++; // Close-Open file to increase capture speed on MATLAB side
        if (closeCounter >= CLOSE_COUNT_MAX) {
            closeCounter = 1;
            fclose(emgFile);
            emgFile = fopen(EMG_FILE, "a");
        }
    }

    /* Overwrite print() - print saved EMG data */
    void print() {
        std::cout << '\r';
        for (size_t i = 0; i < emgSamples.size(); i++) {
            std::ostringstream oss;
            oss << static_cast<int>(emgSamples[i]);
            std::string emgString = oss.str();
            std::cout << '[' << emgString << std::string(4 - emgString.size(), ' ') << ']';
        }
        std::cout << std::flush;
    }
    std::array<int8_t, 8> emgSamples;
};

/* Connect to Myo and set up loop */
int main(int argc, char** argv) {
    try {
        emgFile = fopen(EMG_FILE, "a");
        if (emgFile == NULL) {
            throw std::runtime_error("Unable to open output file!");
        }

        myo::Hub hub("com.Myo-MATLAB-Interface.emg-sample");
        std::cout << "Attempting to find a Myo..." << std::endl;
        
        myo::Myo* myo = hub.waitForMyo(10000);
        if (!myo) {
            throw std::runtime_error("Unable to find a Myo!");
        }

        std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
        myo->setStreamEmg(myo::Myo::streamEmgEnabled); // Set up EMG stream
        DataCollector collector;
        hub.addListener(&collector);

        std::cout << "Gathering EMG.." << std::endl;
        while (1) { // Display EMG data in a loop - loop sets display update
            hub.run(1000 / 20);
            //collector.print();
        }
        
        system("PAUSE"); // Debug
    } catch (const std::exception& e) { // Errors
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
}