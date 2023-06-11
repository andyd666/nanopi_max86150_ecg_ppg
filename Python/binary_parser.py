import matplotlib.pyplot as plt
import numpy
from scipy import signal

# These are bits that show us which signal is present in file
ppg1 = 1
ppg2 = 2
ecg = 16

f_name = "ecg_ppg_binary"
#f_name = "ecg_only"
data = []
num_of_signals = 0

with open(f_name, "rb") as bin_file:
    which_data = int.from_bytes(bin_file.read(4), "little")
    if which_data & ppg1:
        num_of_signals = num_of_signals + 1
    if which_data & ppg2:
        num_of_signals = num_of_signals + 1
    if which_data & ecg:
        num_of_signals = num_of_signals + 1
    fs = which_data & 0xffff00
    fs = fs/256
    fs = 200
    if fs == 0:
        fs = 200

    while True:
        tmp_val = int.from_bytes(bin_file.read(4), "little")
        if not tmp_val:
            break
        data.append(tmp_val)

data_len = len(data)
i = 0
signals = []
while i < num_of_signals:
    signals.append([])
    i = i + 1

i = 0
while i < data_len:
    j = 0
    while j < num_of_signals:
        signals[j].append(data[i+j])
        j = j + 1
    i = i + num_of_signals

t = numpy.linspace(0, len(signals[0])/fs - 1/fs, len(signals[0]))
sos_lpf = signal.cheby1(10, 1, 10, "lowpass", fs=fs, output='sos')
#sos_hpf = signal.cheby1(10, 1, 1, "highpass", fs=fs, output='sos')

i = 0
while i < num_of_signals:
    filtered = signal.sosfilt(sos_lpf, signals[i])
    #filtered = signal.sosfilt(sos_hpf, filtered)
    plt.subplot(num_of_signals, 1, i+1)
    plt.plot(t, filtered)
    i = i + 1

