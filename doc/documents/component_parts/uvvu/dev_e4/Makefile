# ��� �������� ����� ��� ����������
document := KCAY.467239.018e4

TOP_DIR = ../../../..

include $(TOP_DIR)/include.mak

$(document).tex: pics

pics:
	cp $(TOP_DIR)/../hard/e4/e4_*.eps ./

semiall: pdf $(document)_crop.pdf 

$(document)_crop.pdf: $(document).pdf
	pdfcrop  $(document).pdf

clean: localclean

localclean:
	-rm -f ./*.pdf 
	-rm -f ./*.eps 

