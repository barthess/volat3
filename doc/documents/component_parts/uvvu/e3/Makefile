# имя главного файла без расширения
document := KCAY.469555.025e3

TOP_DIR = ../../../..

include $(TOP_DIR)/include.mak

LOCAL_DEPS += ./*.eps

semiall: pdf $(document)_crop.pdf

$(document)_crop.pdf: $(document).pdf
	pdfcrop  $(document).pdf

# пока что автоматически сконвертать не получается
# pe3_table.tex: $(TOP_DIR)/../hard/pcb_main/bom/main_ListByParts.htm
# 	$(HTM2TSV_CMD) $< > uvvu.tsv
# 	$(PE3GEN_CMD) uvvu.tsv

clean: localclean

localclean:
	-rm -f ./*.pdf 
	# -rm -f ./*.eps 




