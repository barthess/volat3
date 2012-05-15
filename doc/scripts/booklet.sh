#!/bin/sh
# Usage: 
# booklet filename_without_extension [resolution]

DOCUMENT=$1
n=0

if [[ $2 != '' ]] 
	then DPI=$2	 # 9600 dpi работает
	else DPI=600 # default
fi



# сделаем вертикальными все страницы
pdftk $DOCUMENT.pdf cat 1-endN output _tmp_plain_rotated.pdf verbose dont_ask
# превратим pdf в ps
pdf2ps -r${DPI} _tmp_plain_rotated.pdf _tmp_plain.ps

# sort pages
psbook _tmp_plain.ps book.ps

# put two pages on one a4 sheet
psnup -2 book.ps nup.ps

# take even pages in reverse order
pstops -pa4 '2:-1' nup.ps even_reverse.ps
ps2pdf -sPAPERSIZE#a4 even_reverse.ps _tmp_${DOCUMENT}_booklet_1.pdf 
pdftk _tmp_${DOCUMENT}_booklet_1.pdf cat 1-endE output ${DOCUMENT}-booklet-1.pdf verbose dont_ask

# take odd pages in direct order
pstops -pa4 '2:0' nup.ps odd_direct.ps
ps2pdf -sPAPERSIZE#a4 odd_direct.ps _tmp_${DOCUMENT}_booklet_2.pdf 
pdftk _tmp_${DOCUMENT}_booklet_2.pdf cat 1-endE output ${DOCUMENT}-booklet-2.pdf verbose dont_ask


# clean 
rm _tmp*.*
rm *.ps 
