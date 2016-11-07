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
#define GYRO_FILE "gyro.txt"
#define ACCEL_FILE "accel.txt"
#define CLOSE_COUNT_MAX 15 // Close-Open file after this many interrupts to expediate reading on the other side #No_hacks

/* Globals */
FILE * emgFile; 
FILE * gyroFile;
FILE * accelFile;
FILETIME ft_now;
int closeCounterEMG = 1;
int closeCounterGYRO = 1;
int closeCounterACCEL = 1;

class DataCollector : public myo::DeviceListener {
public:
    /* On user disconnect */
    void onUnpair(myo::Myo* myo, uint64_t timestamp) {
		std::cout << "Myo unpaired." << std::endl;
    }

	/* On device connection */
	void onConnect(myo::Myo *myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion) {
		// In case we ever want to do anything
	}

    /* EMG data interrupt - save data and print to text file */
    void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg) {
        unsigned long long temp_now, ll_sec, ll_ms;

        GetSystemTimeAsFileTime(&ft_now); // Time grabbed from windows since myo SDK specifies timestamp as from (unspecified) time 
        temp_now = ((LONGLONG)ft_now.dwLowDateTime + ((LONGLONG)(ft_now.dwHighDateTime) << 32LL) - 116444736000000000LL); // time since UNIX epoch in 100ns resolution
        ll_sec = temp_now / 10000000; // Get seconds (integer division)
        ll_ms = (temp_now / 10000) - (ll_sec * 1000); // Get difference caused by truncation in integer division

        fprintf(emgFile, "%4i,%4i,%4i,%4i,%4i,%4i,%4i,%4i,%10" PRIu64 ".%03" PRIu64 "\n", emg[0], emg[1], emg[2], emg[3], emg[4], emg[5], emg[6], emg[7], ll_sec, ll_ms);

		closeCounterEMG++; // Close-Open file to increase capture speed on MATLAB side
        if (closeCounterEMG >= CLOSE_COUNT_MAX) {
			closeCounterEMG = 1;
            fclose(emgFile);
            emgFile = fopen(EMG_FILE, "a");
        }
    }

	/* Gyro data interrupt - save data and print to text file*/
	void onGyroscopeData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3< float > &gyro) {
		unsigned long long temp_now, ll_sec, ll_ms;

		GetSystemTimeAsFileTime(&ft_now); // Time grabbed from windows since myo SDK specifies timestamp as from (unspecified) time 
		temp_now = ((LONGLONG)ft_now.dwLowDateTime + ((LONGLONG)(ft_now.dwHighDateTime) << 32LL) - 116444736000000000LL); // time since UNIX epoch in 100ns resolution
		ll_sec = temp_now / 10000000; // Get seconds (integer division)
		ll_ms = (temp_now / 10000) - (ll_sec * 1000); // Get difference caused by truncation in integer division

		fprintf(gyroFile, "% 10.4f,% 10.4f,% 10.4f,%10" PRIu64 ".%03" PRIu64 "\n", gyro.x(), gyro.y(), gyro.z(), ll_sec, ll_ms);

		closeCounterGYRO++; // Close-Open file to increase capture speed on MATLAB side
		if (closeCounterGYRO >= CLOSE_COUNT_MAX) {
			closeCounterGYRO = 1;
			fclose(gyroFile);
			gyroFile = fopen(GYRO_FILE, "a");
		}
	}

	/* Accelerometer data interrupt - save data and print to text file*/
	void onAccelerometerData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3< float > &accel) {
		unsigned long long temp_now, ll_sec, ll_ms;

		GetSystemTimeAsFileTime(&ft_now); // Time grabbed from windows since myo SDK specifies timestamp as from (unspecified) time 
		temp_now = ((LONGLONG)ft_now.dwLowDateTime + ((LONGLONG)(ft_now.dwHighDateTime) << 32LL) - 116444736000000000LL); // time since UNIX epoch in 100ns resolution
		ll_sec = temp_now / 10000000; // Get seconds (integer division)
		ll_ms = (temp_now / 10000) - (ll_sec * 1000); // Get difference caused by truncation in integer division

		fprintf(accelFile, "% 10.6f,% 10.6f,% 10.6f,%10" PRIu64 ".%03" PRIu64 "\n", accel.x(), accel.y(), accel.z(), ll_sec, ll_ms);

		closeCounterACCEL++; // Close-Open file to increase capture speed on MATLAB side
		if (closeCounterACCEL >= CLOSE_COUNT_MAX) {
			closeCounterACCEL = 1;
			fclose(accelFile);
			accelFile = fopen(ACCEL_FILE, "a");
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
            throw std::runtime_error("Unable to open EMG output file!");
        }

		gyroFile = fopen(GYRO_FILE, "a");
		if (gyroFile == NULL) {
			throw std::runtime_error("Unable to open GYRO output file!");
		}

		accelFile = fopen(ACCEL_FILE, "a");
		if (accelFile == NULL) {
			throw std::runtime_error("Unable to open ACCEL output file!");
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