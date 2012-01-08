# Process this file with GNU make
.PHONY: all mirror clean

TOP_DIR = .
generated_svg = autriggers_bg.svg        leg1_red.svg  leg4_red.svg  tiers_bg.svg \
				autriggers_sygn_grey.svg  leg2_red.svg  leg5_red.svg  tiers_sygn_grey.svg \
				autriggers_sygn_red.svg   leg3_red.svg  leg6_red.svg  tiers_sygn_red.svg

# figpng := $(shell /bin/find $(TOP_DIR) -name '*.svg' | sed -e 's/.svg$$/.png/')




all: $(generated_svg) mirror

$(generated_svg): tiers.svg
	genpics.py

%.png: %.svg
	inkscape --without-gui --export-text-to-path --export-area-page --export-dpi=90 --export-png=$@ $<

figpng := $(shell /bin/find $(TOP_DIR) -name '*.svg' | sed -e 's/.svg$$/.png/')

mirror: $(figpng)
	imgconvert -flop thermometer_mask.png thermometer_mask_mirror.png
	imgconvert -flop break_left.png break_right.png
	imgconvert -flop turn_left.png turn_right.png


clean:
	rm -f *.png
	rm -f $(generated_svg)



