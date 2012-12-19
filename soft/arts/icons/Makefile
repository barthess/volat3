# Process this file with GNU make

figpng: $(patsubst %.svg,%.png,$(wildcard *.svg))

%.png: %.svg
	./svg2png.sh $< 70

clean:
	rm -f *.png

