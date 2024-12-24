import pandas as pd
import matplotlib.pyplot as plt
from scipy.signal import savgol_filter


def parse_and_plot(file):
	# Load data from CSV file
	data = pd.read_csv(file)

	# Extract Time and Voltage columns
	time = data['times']
	voltage = data['voltages']

	smoothed_voltages = savgol_filter(voltage, window_length=10, polyorder=3)

	#Plot Voltage vs Time
	plt.figure(figsize=(10,6))
	plt.plot(time, smoothed_voltages, label='Voltage vs Time')

	# Adding labels and title
	plt.xlabel('Time (s)')
	plt.ylabel('Voltage (V)')
	plt.title('Voltage vs Time')
	plt.legend()

	# Display the plot
	plt.grid(True)
	plt.show()
 
    



def find_peaks_per_pulse(voltages):
    change_width = 2
    n = len(voltages)
    lower_bound = 0.1
    i = 0
    peaks = {} # of the form {pulse_number: {local_idex: [peak_idx], start_idx: peak_idx, end_idx: peak_idx}}
    pulse_number = 0
    i = change_width  # Start after the first `change_width` elements to allow room for checchange_widthing
    while i < n - change_width:
        pulse_number += 1
        peaks[pulse_number] = {}
        start_idx = i
        peaks[pulse_number]['start_idx'] = start_idx
        local_maxima = []
        cur_global_max = 0
        while i < n - change_width and voltages[i] > lower_bound:
            # Checchange_width if the `change_width` elements before are increasing and `change_width` elements after are decreasing
            is_increasing = all(voltages[i - j - 1] < voltages[i - j] for j in range(change_width))
            is_decreasing = all(voltages[i + j] > voltages[i + j + 1] for j in range(change_width))
            if is_increasing and is_decreasing:
                if voltages[i] > cur_global_max:
                    cur_global_max = i
                local_maxima.append(i)

                # Move the index forward to avoid redundant checchange_widths within the decreasing sequence
                i += change_width
            else:
                i += 1
        
        while i < n - change_width and voltages[i] <= lower_bound:
            i += 1
        # Find the global index of the local maxima
        peaks[pulse_number]['local_peaks'] = local_maxima
        peaks[pulse_number]['global_peak'] = cur_global_max
        peaks[pulse_number]['end_idx'] = i
        
    return peaks
    
  
def print_peaks(peaks, times, voltages):
	for peak in peaks:
		peak_info = peaks[peak]
		print('Global:', times[peak_info['global_peak']], voltages[peak_info['global_peak']])
		for local in peak_info['local_peaks']:
			print('Local:', times[local], voltages[local])
		print('Start time:', times[peak_info['start_idx']], 'End time:', times[peak_info['end_idx']])
		


import numpy as np
from scipy.signal import find_peaks

def find_waveform_peaks(voltages, times):
    # Parameters
    threshold = 0.001

    # Find start and end indices of each waveform
    waveform_indices = []
    start = None

    for i, v in enumerate(voltages):
        if start is None and v >= threshold:  # Start of a waveform
            start = i
        elif start is not None and v < threshold:  # Possible end of a waveform
            # Look ahead to see if the next point starts a new waveform
            if i + 1 < len(voltages) and voltages[i + 1] >= threshold:
                waveform_indices.append((start, i))
                start = i + 1

    # Handle the case where the last waveform does not drop below the threshold
    if start is not None:
        waveform_indices.append((start, len(voltages) - 1))

    # Analyze each waveform and count peaks
    waveform_peak_counts = []
    for start, end in waveform_indices:
        segment = voltages[start:end + 1]  # Get the waveform segment
        peaks, _ = find_peaks(segment, height=0.1)  # Adjust height as needed
        waveform_peak_counts.append(len(peaks))  # Count peaks

    # Output results
    for i, (indices, count) in enumerate(zip(waveform_indices, waveform_peak_counts)):
        print(f"Waveform {i + 1}: Start = {times[indices[0]]}, End = {times[indices[1]]}, Peaks = {count}")  

def main():
	times, voltages = parse_and_plot("/Users/soumabrady/Documents/VIP Pulse projects /Pulse waves/output2.csv")
	find_waveform_peaks(voltages, times)
	# peaks = find_peaks_per_pulse(voltages)
	# print_peaks(peaks, times, voltages)

main()
