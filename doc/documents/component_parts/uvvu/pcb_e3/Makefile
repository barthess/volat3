# ��� �������� ����� ��� ����������
document := KCAY.469555.025e3

TOP_DIR = ../../../..

include $(TOP_DIR)/include.mak

$(document).tex: pics

pics:
	cp $(TOP_DIR)/../hard/mpiovd/mpiovd_*.eps ./

semiall: pdf $(document)_crop.pdf 

$(document)_crop.pdf: $(document).pdf
	pdfcrop  $(document).pdf

clean: localclean

localclean:
	-rm -f ./*.pdf 
	-rm -f ./*.eps 

