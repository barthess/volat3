#!/bin/bash
# ������ ��� ���������� ��������� � ������
# ��������� ��� pdf-�� ��� ���������� 

# ��� pdfcrop ������� � �������� ���� ����� ���� ���
# ��� �� ������, ��� ��������

DOCUMENT=$1

# Ghostscript's calculation of the bounding box is faster,
# if --resolution 72 is used instead of ghostscript's implicite
# default setting of 4000 DPI
CROP_OPTS="--verbose --resolution 72"

# ���� �� ������������� �����
test -f ${DOCUMENT}-A3.pdf
if [ "$?" == 0 ] 
	then 
		pdfcrop ${DOCUMENT}-A3.pdf ${DOCUMENT}-A3.pdf $CROP_OPTS
	   	pdfcrop ${DOCUMENT}-A4.pdf ${DOCUMENT}-A4.pdf $CROP_OPTS
	else 
		pdfcrop ${DOCUMENT}.pdf ${DOCUMENT}.pdf $CROP_OPTS
fi


 
