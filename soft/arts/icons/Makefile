# Process this file with GNU make

COLOR=e69100

all: $(patsubst %.svg,%.png,$(wildcard *.svg))

%.png: %.svg
	./svg2png.sh $< 70 ${COLOR}

clean:
	rm -f *.png

