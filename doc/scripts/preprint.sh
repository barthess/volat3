#!/bin/bash
# скрипт для подготовки документа к печати
# принимает имя pdf-ки без расширения 

# для pdfcrop входной и выходной файл имеет одно имя
# это не ошибка, так задумано

DOCUMENT=$1

# Ghostscript's calculation of the bounding box is faster,
# if --resolution 72 is used instead of ghostscript's implicite
# default setting of 4000 DPI
CROP_OPTS="--verbose --resolution 72"

# есть ли нестандартные листы
test -f ${DOCUMENT}-A3.pdf
if [ "$?" == 0 ] 
	then 
		pdfcrop ${DOCUMENT}-A3.pdf ${DOCUMENT}-A3.pdf $CROP_OPTS
	   	pdfcrop ${DOCUMENT}-A4.pdf ${DOCUMENT}-A4.pdf $CROP_OPTS
	else 
		pdfcrop ${DOCUMENT}.pdf ${DOCUMENT}.pdf $CROP_OPTS
fi


 
