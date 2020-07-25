#Ampere control V1.0
#EEforEveryone - 2020
###########################
#
#Written in Python 3.8.3 x64
#
#Dependent on: pySerial (python -m pip install pyserial)
#https://pyserial.readthedocs.io/en/latest/pyserial.html
#
#Dependent on: PySimpleGUI (python -m pip install pysimplegui)
#https://pypi.org/project/PySimpleGUI/
#
###########################
#V1.0 Functionality:
#   Implemented Features:
#
#
#
#   Known Issues:
#
#
#
#Planned for a Future Release:
#
#
#
#
#
###########################

import serial
import serial.threaded
import time
import csv

import re

import matplotlib.pyplot as plt
import PySimpleGUI as sg


def plot_draw(time, voltage, Current, ChannelNum, maxchannels): #Draws voltage and current as subplots!
    #TODO: make sure vectors are the same Length, check for errors!
    fig.suptitle('Live Plot for Channel:'+str(ChannelNum), fontsize=16)
    graphs[0].clear()

    if(ChannelNum == -1 or ChannelNum > maxchannels): #plot all
        i = 1
        #plt.hold(True) #turn on hold plot
        while (i < maxchannels):
            graphs[0].plot(time[i], voltage[i])
            i = i+1
        #plt.hold(False) #turn off hold plot
    else:
        graphs[0].plot(time[controlChannel], voltage[controlChannel]) #plot one
    
    graphs[0].set_ylim([0, 100])
    graphs[0].set_title('Voltage History')
    graphs[0].set_ylabel('Voltage (V)')

    graphs[1].clear()

    if(ChannelNum == -1 or ChannelNum > maxchannels): #plot all
        i = 1
        #plt.hold(True) #turn on hold plot
        while (i < maxchannels):
            graphs[1].plot(time[i], Current[i])
            i = i+1
        #plt.hold(False) #turn off hold plot
    else:
        graphs[1].plot(time[controlChannel], Current[controlChannel]) #plot one

    graphs[1].set_ylim([0, 10])
    graphs[1].set_title('Current History')
    graphs[1].set_ylabel('Current (A)')
    graphs[1].set_xlabel('Time (Seconds)')

    plt.ion() #Interactive mode: non-blocking
    plt.show()
    return True

def reset_plot_vectors(seconds, polling_rate): #generate blank vectors, etc
    #generate time vector
    NUMSAMPLES = int(seconds*1000/polling_rate)
    timevect = list(range(NUMSAMPLES)) #Time vector, raw ints
    timevect = [(x * -1 * polling_rate/1000) for x in timevect] #scale vector from int to float
    timevect.reverse() #flip time vector to put 0 on the right side!

    #reset data vectors
    voltagevect = list(range(NUMSAMPLES))
    voltagevect = [0 for x in voltagevect] #Set voltage vector to all zeros
    currentvect = list(range(NUMSAMPLES))
    currentvect = [0 for x in currentvect] #set current vector to all zeros
    return [timevect, voltagevect, currentvect]

AMPGUI_SEL_I = 1 #define for Current mode from ampgui.h V1.0
AMPGUI_SEL_R = 2 #define for Resistance mode from ampgui.h V1.0
AMPGUI_SEL_P = 3 #define for Power mode from ampgui.h V1.0

lastmode = -1 #remembers the previously sent mode value
polling_interval = 1000 #milliseconds
plot_seconds = 30
PLOT_REFRESH_TIME = 1 #Refresh interval for plot, in seconds #TODO: make better
lastplotrefresh = 0

datainChannel = 1 #channel currently being updated via serial
controlChannel = 1 #channel currently being controlled and plotted via serial

NUMSAMPLES = int(plot_seconds*1000/polling_interval)
MAXCHANNELS = 5 #one more than max
index = 0
timeVector = list() #placeholder... overrite
voltageVector = list() #start with zero vector
CurrentVector =list() #start with zero vector

while index < MAXCHANNELS:
    timeVector.append(list()) #create MAXCHANNELS lists within the list
    voltageVector.append(list()) #create MAXCHANNELS lists within the list
    CurrentVector.append(list()) #create MAXCHANNELS lists within the list
    index += 1

index = 0
while index < MAXCHANNELS:
    [timeVector[index], voltageVector[index], CurrentVector[index]] = reset_plot_vectors(plot_seconds, polling_interval) #set up vectors
    index += 1

doPlotting = False #when true, sample data and plot
doLogging = False
doPWL = False

sg.theme('DarkAmber')	# Add a touch of color
# All the stuff inside your window.
layout = [  [sg.Text('COM Port Setup:')],
            [sg.Text('Com port ID:'), sg.InputText('COM1')],                        #ID0
            [sg.Text('Polling Interval (ms):'), sg.InputText('1000')],              #ID1
            [sg.Button('Connect'), sg.Button('Disconnect') ],
            
            [sg.Text('Single Load Channel Config:')],
            [sg.Text('Set Mode:'),sg.Combo(['Current', 'Resistance', 'Power']) ],   #ID2
            [sg.Text('Set Value:'), sg.InputText('0'), sg.Button('Update Value')],  #ID3
            [sg.Text('OR... Load PWL File #TODO: Enable PWL Playback'), sg.Button('Set PWL File')],
            [sg.Text('Toggle Log (Set File):'),sg.Button('Toggle Log to File')],
            [sg.Button('ON'), sg.Button('OFF')],
            [sg.Text('Voltage:'), sg.InputText('0'), sg.Text('Volts') ],      #ID4 
            [sg.Text('Current:'), sg.InputText('0'), sg.Text('Amps') ],      #ID5 
            [sg.Text('Resistance:'), sg.InputText('0'), sg.Text('Ohms') ],   #ID6 
            [sg.Text('Power:'), sg.InputText('0'), sg.Text('Watts') ],        #ID7 
            [sg.Text('Energy:'), sg.InputText('0'), sg.Text('kJoules') ],   #ID8
            [sg.Text('Seconds to Plot:'), sg.InputText('30'), sg.Button('TOGGLE LIVE PLOT')],       #ID9
            [sg.Text('Channel #:'), sg.InputText('1'), sg.Button('Change Channel')] ] #ID10

fig, graphs  = plt.subplots(2, 1, constrained_layout=True) #create figure made of subplots

BAUDRATE = 2000000
ser = serial.Serial(baudrate = BAUDRATE, bytesize = 8, timeout=2, stopbits=serial.STOPBITS_ONE)

this_V = 0.0
this_I = 0.0
this_R = 0.0
this_P = 0.0
this_KJ = 0.0

# Create the Window
window = sg.Window('Project Ampere Control App V1.0', layout)
# Event Loop to process "events" and get the "values" of the inputs
while True:
    ##PROCESS INCOMING DATA FROM SLAVE!##
    if(ser.isOpen() == True and ser.in_waiting > 0): #port is open and there is data to process!
        newdata = str(ser.readline())

        #print(newdata)
        
        number_milli = re.findall('\d+',newdata) 
        # \d+ is a regular expression which means one or more digit

        #print(number_milli)


        if(newdata.find('J')>0): #Joules update
            this_KJ = int(number_milli[0])/1000.0

            #Energy is reported last... so do this last... #Todo... make logging better
             
            if(doLogging == True):
                try:
                    with open(LOG_OUT_FILE, 'a+', newline='') as csvfile:
                        log_handle = csv.writer(csvfile, delimiter=' ', quotechar='|', quoting=csv.QUOTE_MINIMAL)
                        log_handle.writerow([str(time.time() - LOG_starttime),str(datainChannel),str(this_V),str(this_I), str(this_KJ)])
                except:
                    print('Log File Locked or Does not Exist!!! Can\'t Write!\n');

        elif(newdata.find('V')>0): #Volts update
            this_V = int(number_milli[0])/1000.0
            voltageVector[datainChannel].pop(0) # delete the first element of the vector to maintain length
            voltageVector[datainChannel].append(this_V) #add new element to front of vector with append

        elif(newdata.find('I')>0): #Current update
            this_I = int(number_milli[0])/1000.0
            CurrentVector[datainChannel].pop(0) # delete the first element of the vector to maintain length
            CurrentVector[datainChannel].append(this_I) #add new element to front of vector with append

        elif(newdata.find('L')>0): #Change Channel
            if int(number_milli[0]) < MAXCHANNELS:
                datainChannel = int(number_milli[0])
            else:
                datainChannel = MAXCHANNELS


        if(this_I>0):
            this_R = this_V/this_I;
        else:
            this_R = 999999999999.9;

        this_P = this_V*this_I;

        print("\n\nCh#:",datainChannel,"\nVolts:",this_V,"\nAmps:",this_I,"\nOhms:",this_R,"\nWatts:",this_P,"\nJoules:",this_KJ)
        if (datainChannel == controlChannel): #only update UI if this channel is right
            window[4].update(str(this_V)) #update voltage
            window[5].update(str(this_I)) #update current
            window[6].update(str(this_R)) #update resistance
            window[7].update(str(this_P)) #update power
            window[8].update(str(this_KJ)) #update energy

    ##Update Plot Window!##
    if(doPlotting == True):
        if(((time.time() - PLOT_REFRESH_TIME) > lastplotrefresh) or (lastplotrefresh > time.time()) ):
            #try:
            tempvolt = voltageVector.copy()
            tempcurr = CurrentVector.copy()



            plot_draw(timeVector, tempvolt, tempcurr, controlChannel, MAXCHANNELS)
            #except:
                #print("Plot Error!")
            lastplotrefresh = time.time()


    ##Handle and Process GUI Events!##
    event, values = window.read(timeout = 10)
    if event == sg.WIN_CLOSED:	# if user closes window or clicks cancel
        break
    elif event == 'Connect': #open com port, set up polling rate, and disable output
        print('Connect and Initalize the Ampere Device!')
        ser.port = values[0] #Change the port to the GUI value

        try:            #open the com port!
            ser.open()
        except:
            print('Port Already Open! (',values[0],')')
            ser.close()
            ser.open()

        ser.write(b'<INTm>') #Send header
        ser.write(bytes(str(values[1]), 'ascii')) #send number as a string encoded in bytes
        ser.write(b'<---->') #Send footer
        polling_interval = int(values[1]) #remember the polling interval for graphing
        index = 0
        while index < MAXCHANNELS:
            [timeVector[index], voltageVector[index], CurrentVector[index]] = reset_plot_vectors(plot_seconds, polling_interval) #set up vectors
            index += 1

    elif event == 'Disconnect': #Turn off output, release control, and close the COM port! 
        print('Turn off output, Release Control, and Close the Port!')
        ser.write(b'<STOP><---->') #turn off output
        ser.write(b'<RELS><---->') #take control from GUI#release control
        ser.close() #close com port

    elif (event == 'ON' or event == 'Update Value' or event == 'Change Channel'): #Turn on output and take control! 
        print('Turn on the Output and take Control!')

        #Make sure the Mode Setting is Valid!
        thismode = -1 #default to error
        if values[2] == 'Current': #Current Mode!
            thismode = AMPGUI_SEL_I
        elif values[2] == 'Resistance': #Resistance Mode!
            thismode = AMPGUI_SEL_R
        elif values[2] == 'Power': #Resistance Mode!
            thismode = AMPGUI_SEL_P

        if(ser.isOpen() != True):
            thismode = -1 #set to error state
            print('COM port not open!')

        if(thismode > -1): #A valid mode is selected

            if controlChannel != int(values[10]): #if the current channel doesn't match the last value
                if(int(values[10]) < MAXCHANNELS+10): #todo, proper input validation
                    controlChannel = int(values[10]) #make sure it's an int...
                else:
                    controlChannel = MAXCHANNELS
                ser.write(b'<CHAN>')#Send the Change Channel (HDR)
                ser.write(bytes(str(controlChannel), 'ascii'))#Send the Change Channel (value)
                ser.write(b'<---->')#Send the Change Channel (Footer)

            if(lastmode != thismode):
                #Turn off the output
                ser.write(b'<MODE>')#Send the new mode (HDR)
                ser.write(bytes(str(thismode), 'ascii'))#Send the new mode (value)
                ser.write(b'<---->')#Send the new mode (Footer)
                lastmode = thismode #save the mode

            ser.write(b'<VALm>')#Send the new Value (HDR)
            ser.write(bytes(str((float(values[3])*1000)), 'ascii'))#Send the new Value (value)
            ser.write(b'<---->')#Send the new Value (Footer)

            if event == 'ON':
                ser.write(b'<OVER><---->')#Take Control
                ser.write(b'<STRT><---->') #Turn on the output!
        else: #Need to select Mode
            print('ERROR: No MODE selected! Please select a mode.')

    elif event == 'OFF': #Turn off output! 
        print('Turn off the Output!')
        ser.write(b'<STOP><---->') #turn off output

    elif event == 'TOGGLE LIVE PLOT': #toggle plot live update!
        if(doPlotting == False):
            doPlotting = True
            #start up plotting variables!
            plot_seconds = float(values[9])

            index = 0
            while index < MAXCHANNELS:
                [timeVector[index], voltageVector[index], CurrentVector[index]] = reset_plot_vectors(plot_seconds, polling_interval) #set up vectors
                index += 1
            
            
        else:
            doPlotting = False
    
    elif event == 'Set PWL File':
        PWL_IN_FILE = sg.popup_get_file('Please enter the PWL file name')
        

    elif event == 'Toggle Log to File':

        if(doLogging == False):
            LOG_OUT_FILE = sg.popup_get_file('Please enter the Log file name')
            LOG_starttime = time.time()
            doLogging = True
            try:
                with open(LOG_OUT_FILE, 'a+', newline='') as csvfile:
                    log_handle = csv.writer(csvfile, delimiter=' ', quotechar='|', quoting=csv.QUOTE_MINIMAL)
                    log_handle.writerow(['time_(s)','Channel_Number','voltage_(v)','current_(a)', 'energy_(J)'])
            except:
                print('Log File Locked!!! Can\'t Write!\n');
                doLogging = False
        else:
            doLogging = False



        
        
    
window.close()










