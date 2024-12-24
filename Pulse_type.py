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
    
	return time, smoothed_voltages
import numpy as np
from scipy.signal import find_peaks

def find_waveform_peaks(voltages, times):
    # Parameters
    threshold = 0.001
    more = 0
    sum = 0
    valid_ratio = 0 
    ratio =0 
    add_it = 0
    

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
    waveform_peak_values = []
    for start, end in waveform_indices:
        segment = voltages[start:end + 1]  # Get the waveform segment
        peaks, true_voltage = find_peaks(segment, height=0.1)  # Adjust height as needed
        real_voltage = true_voltage['peak_heights']
        
        waveform_peak_values.append(real_voltage)
        waveform_peak_counts.append(len(peaks))  # Count peaks

    # Output results
    for i, (indices, count, values) in enumerate(zip(waveform_indices, waveform_peak_counts,waveform_peak_values)):
        print(f"Waveform {i + 1}: Start = {times[indices[0]]}, End = {times[indices[1]]}, Peaks = {count} , Voltage at peaks = {values}" ) # make edits 
        if count == 2:
            more += 1
            ratio = float (values[0] / values[1])
            sum += ratio
    valid_ratio = float (sum / more) if more != 0 else 0 
    
                
    
    for i, (indices, count, values) in enumerate(zip(waveform_indices, waveform_peak_counts, waveform_peak_values)):
        if count == 2:
            ratio = values[0] / values[1]
            lower_bound = valid_ratio - valid_ratio * 0.15
            upper_bound = valid_ratio + valid_ratio * 0.15
            if lower_bound <= ratio <= upper_bound:  # Check if ratio is within the range
                add_it += 1
                print(
                    f"Waveform {i + 1} (Healthy): Start = {times[indices[0]]}, "
                    f"End = {times[indices[1]]}, "
                    f"Peaks = {count}, "
                    f"Voltage at peaks = {[f'{v}' for v in values]}"
                )

    # Final summary
    print(
        f"There are {more} verified pulses, "
        f"the average ratio of their voltages is {valid_ratio}, "
        f"and {add_it} are within the healthy range."
    )        
            

 

def main():
	times, voltages = parse_and_plot("/Users/soumabrady/Documents/VIP Pulse projects /Pulse waves/output2.csv")
	find_waveform_peaks(voltages, times)
	# peaks = find_peaks_per_pulse(voltages)
	# print_peaks(peaks, times, voltages)

main()
