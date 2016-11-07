# Myo-MATLAB Interface

----
## Intro
Simple interface demonstration for taking Myo EMG data in near real time and piping to MATLAB; script shows the data on a graph as it comes in along with the Mean Absolute Value Feature.

* Distributed under a modified MIT Licence
	* For academic works please reference with the following key information: "A, Hartwell (2016) Myo-MATLAB Interface"

* Requires Windows (OS based high res timestamps).

* Build using Myo SDK 0.9 and Myo Connect 1.01 

----
## Usage
1. Get Myo Connect and ensure running correctly 
2. Run realTimeFeatureExtractOptimised.m

----
## Notes on Visual Studio Project
Uses a C++ executable built using Visual Studio 2015 Community Version which is included for reference. This will be run in a command prompt by the MATLAB script.

If recompiling the project you'll have to do some additional set up rather than just opening the project; see [Myo SDK Website](https://developer.thalmic.com/docs/api_reference/platform/the-sdk.html) for further details of the SDK and how to use it as this will depend on operating system/compiler etc.

----
## Potential Limitations
Like any true hack (MATLAB and real time aren't things that generally go together) there are potential issues :

* Currently only works on windows due to taking millisecond precision time stamps from Windows.h 

* The use of a text file (emg.txt) as a bridge. The C++ code keeps it open in append mode for 10 samples at a time (CLOSE\_COUNT\_MAX in the C++ source); the file is then closed and reopened. Opening and closing on each write is possible but intermittently and inconsistently causes problems in the I/O as it likely hits some race hazard which will vary depending on your system/load/etc. Other potential issues include file size I/O problems and locking by other programs. File size is easily mitigated by clearing the text file between runs (as in the script) since the Myo battery will go before this becomes an issue.

* Longest continuous gathering period tested thus far: ~2.5 hours

* Modifying CLOSE\_COUNT\_MAX will allow MATLAB to read in bigger/smaller batches of data at a time but thorough testing on each system will need to be done to ensure stability with any use of the variable. Stripping out lines 47-52 will remove the close-open behaviour. keeping the file open permanently resulted in 73-74 new data point batches on my system which implies the file is updated (for external access) once every ~0.37s (Windows 10).

# Have Fun ^^
