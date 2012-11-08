#!/usr/bin/env python

from scipy.signal import kaiserord, lfilter, firwin, freqz

# output file
out = open('dsp_generated.c', 'w')

# cut off in mHz
fmin = 1000 / 16
fmax = 1000

# numtaps
flen_min = 4
flen_max = 256

pclk2 = 4000000
adcpre = 8
adcclk = pclk2 / adcpre
sample_time = 241 # cycles
sample_count = 16
sample_row_count = 4
sample_rate = adcclk / (sample_time * sample_count)
print "sample rate =", sample_rate, "Hz"

# The Nyquist rate of the signal.
nyq_rate = sample_rate / 4.0

def gen_taps(N, cutoff_hz):
    """
    Calculate taps for filter with specified parameters.

    N -- number of taps
    cutoff_hz -- cutoff frequency of the filter
    """

    # Use firwin with fiexed number of points and Hamming window
    taps = firwin(N, cutoff_hz/(1000.0*nyq_rate), window="blackman")
    q31_taps = []
    for i in taps:
        q31_taps.append(int(round(2**31 * i)))

    st = ""
    for i in q31_taps:
        st = st + str(i) + ", "
    st = st[0:-2]

    out.write(
    "const q31_t taps_"+str(cutoff_hz)+"_"+str(N)+"["+str(N)+"] = {"+st+"};\n"
    )
    return "taps_"+str(cutoff_hz)+"_"+str(N)

# create navigation array
arr = []
f = fmax
while f >= fmin:
    nt = flen_min
    line = []
    while nt <= flen_max:
        line.append(gen_taps(nt+1, f))
        nt = nt * 2
    f = f / 2
    arr.append(line)

print arr

out.write(
"\n"
"/* navigation array */\n"
"const q31_t *taps[" + str(len(arr)) + "][" + str(len(arr[0])) + "] = {\n"
)

for row in arr:
    out.write("    {")
    line = ""
    for i in row:
        line = line + i + ", "
    out.write(line[0:-2])
    out.write("},\n")

out.write(
"};\n"
)







