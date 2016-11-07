%% Pseudo-real-time EMG data extraction from Myo to MATLAB
% Extracts EMG data from a MYO to MATLAB via C++
% Requires MYO Connect to be running
% Runs a C++ executable in a command prompt

clear; clc; close all % House keep

%% Genmeral Setup
lineLengthEMG = 54;
lineLengthGYRO = 47;
lineLengthACCEL = 47;
numChannels = 8;
emgSampMax = 4000;
gyroAccelSampMax = emgSampMax/4;
windowLength = 40;
curPositionEMG = 0;
curPositionGYRO = 0;
curPositionACCEL = 0;

emgData = NaN([emgSampMax numChannels]); % Matrix for EMG data
mavData = NaN([(emgSampMax - windowLength) numChannels]); % Matrix for MAV feature
gyroData = NaN([gyroAccelSampMax 3]); % Matrix for Gyro data
accelData = NaN([gyroAccelSampMax 3]); % Matrix for Accelerometer data

fileNameEMG = 'emg.txt'; % File for data transmission; again stop judging ^^
fileNameGYRO = 'gyro.txt'; % File for data transmission; again stop judging ^^
fileNameACCEL = 'accel.txt'; % File for data transmission; again stop judging ^^
cmdWindowName = 'EMG Gather';

FileEMG = fopen(fileNameEMG,'w'); % Reset file so we don't read previous runs etc
fclose(FileEMG); 
FileGYRO = fopen(fileNameGYRO,'w'); % Reset file so we don't read previous runs etc
fclose(FileGYRO);
FileACCEL = fopen(fileNameACCEL,'w'); % Reset file so we don't read previous runs etc
fclose(FileACCEL);

system(['start /realtime "' cmdWindowName '" getMyoEmg.exe & exit &']) % Start (non-blocking) C thread
figure(1) % Do after cmd call to bring to foreground
set(gcf,'currentchar',']'); % For loop breaking later

%% Hack for forcing data sources
emgData(1,:) = [1 2 3 4 5 6 7 8];
emgData(2,:) = 1;
mavData(1,:) = [1 2 3 4 5 6 7 8]; 
mavData(2,:) = 1;
gyroData(1,:) = [1 2 3]; 
gyroData(2,:) = 1;
accelData(1,:) = [1 2 3];
accelData(2,:) = [4 5 6];

%% Show graphs
subplot(4,1,1)
h1 = plot(emgData);
ylim([-128 127])
xlim([1 emgSampMax])
% title(['Sample frequency: ' num2str(curSampleEMG/(curTime - startTimeEMG))])
title('EMG Signal')
set(gca,'XTickLabel','')
ylabel('Amplitude')


subplot(4,1,2)
h2 = plot(mavData);
ylim([0 127])
xlim([1 emgSampMax])
title([num2str(windowLength) ' Sample Window MAV'])
xlabel('Press any key to quit..')
ylabel('Amplitude')


subplot(4,1,3)
h3 = plot(gyroData);
ylim([-500 500])
xlim([1 gyroAccelSampMax])
title('Gyro Data')
set(gca,'XTickLabel','')
ylabel('Amplitude')

subplot(4,1,4)
h4 = plot(accelData);
ylim([-2 2])
xlim([1 gyroAccelSampMax])
title('Accelerometer Data')
set(gca,'XTickLabel','')
ylabel('Amplitude')

%% Hack continued
linkdata on
linkdata off

%% Pause for handshake with myo connect and for data collection to begin
pause(1);

%% Pseudo-realtime extraction
% Get first timestamp from file (and check data gathering is working)
FileEMG = fopen(fileNameEMG,'r'); 
fseek(FileEMG,curPositionEMG,-1); 
fileDataRaw = fgetl(FileEMG);
if fileDataRaw == -1 
    system(['taskkill /f /fi "WindowTitle eq  ' cmdWindowName '" /T & exit']) 
    close(gcf)
    disp('Data acquisition not active')
    return;
end
fileDataStrArray = strsplit(fileDataRaw,',');
startTimeEMG = str2double(fileDataStrArray(end));
fclose(FileEMG); 

curSampleEMG = 1;
curSampleGYRO = 1;
curSampleACCEL = 1;
lastSampleEMG = 1;
lastSampleGYRO = 1;
lastSampleACCEL = 1;
while get(gcf,'currentchar')==']' % While no button has been pressed
    %% Do EMG processing and graphing
    FileEMG = fopen(fileNameEMG,'r'); 
    fseek(FileEMG,curPositionEMG,-1); 

    fileDataRaw = ' ';
    while ischar(fileDataRaw) % Extract new data from file
        fileDataRaw = fgetl(FileEMG);
        if numel(fileDataRaw) ~= lineLengthEMG % Break if last line incomplete (and seek back to start of that line
            fseek(FileEMG,-numel(fileDataRaw) ,0);
            break;
        end
        
        fileDataStrArray = strsplit(fileDataRaw,',');
        curTime = str2double(fileDataStrArray(end));
        emgData(curSampleEMG,:) = str2double(fileDataStrArray(1:numChannels));
        
        if curSampleEMG >= windowLength % MAV feature extraction
            mavData(curSampleEMG - windowLength + 1,:) = mean(abs(emgData(curSampleEMG - windowLength + 1:curSampleEMG,:)));
        end
        
        curSampleEMG = curSampleEMG + 1;
    end
    curPositionEMG = ftell(FileEMG);
    fclose(FileEMG); 
    
    %% Do Gyro processing and graphing
    FileGYRO = fopen(fileNameGYRO,'r'); 
    fseek(FileGYRO,curPositionGYRO,-1); 

    fileDataRaw = ' ';
    while ischar(fileDataRaw) % Extract new data from file
        fileDataRaw = fgetl(FileGYRO);
        if numel(fileDataRaw) ~= lineLengthGYRO % Break if last line incomplete (and seek back to start of that line
            fseek(FileGYRO,-numel(fileDataRaw) ,0);
            break;
        end
        
        fileDataStrArray = strsplit(fileDataRaw,',');
        gyroData(curSampleGYRO,:) = str2double(fileDataStrArray(1:3));
        
        curSampleGYRO = curSampleGYRO + 1;
    end
    curPositionGYRO = ftell(FileGYRO);
    fclose(FileGYRO); 
    
    %% Do Accelerometer processing and graphing
    FileACCEL = fopen(fileNameACCEL,'r'); 
    fseek(FileACCEL,curPositionACCEL,-1); 

    fileDataRaw = ' ';
    while ischar(fileDataRaw) % Extract new data from file
        fileDataRaw = fgetl(FileACCEL);
        if numel(fileDataRaw) ~= lineLengthACCEL % Break if last line incomplete (and seek back to start of that line
            fseek(FileACCEL,-numel(fileDataRaw) ,0);
            break;
        end
        
        fileDataStrArray = strsplit(fileDataRaw,',');
        accelData(curSampleACCEL,:) = str2double(fileDataStrArray(1:3));
        
        curSampleACCEL = curSampleACCEL + 1;
    end
    curPositionACCEL = ftell(FileACCEL);
    fclose(FileACCEL); 
    
    %% Loop housekeeping
    refreshdata
    drawnow
    if curSampleEMG > emgSampMax % Clear arrays when large
        curSampleEMG = 1;
        curSampleGYRO = 1;
        curSampleACCEL = 1;
        lastSampleEMG = 1;
        lastSampleGYRO = 1;
        lastSampleACCEL = 1;
        emgData = NaN([emgSampMax numChannels]);
        mavData = NaN([(emgSampMax - windowLength) numChannels]);
        gyroData = NaN([gyroAccelSampMax 3]); 
        accelData = NaN([gyroAccelSampMax 3]); 
        startTimeEMG = curTime;
    end
end

%% CLean up - target specific window made for this script
system(['taskkill /f /fi "WindowTitle eq  ' cmdWindowName '" /T & exit']) 
close(gcf)

