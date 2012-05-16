# имя главного файла без расширения
document := KCAY.469555.025pe3

TOP_DIR = ../../../..
HTM2TSV_CMD = $(TOP_DIR)/../../eclgen/htm2tsv.sh
PE3GEN_CMD =  $(TOP_DIR)/../../eclgen/gen.py

include $(TOP_DIR)/include.mak

semiall: pdf archive 

semiallbw: archivebw 

$(document).tex: pe3_table.tex 

# пока что автоматически сконвертать не получается
pe3_table.tex: $(TOP_DIR)/../hard/pcb_main/bom/main_ListByParts.htm
	-rm -f uvvu.tsv
	$(HTM2TSV_CMD) $< > uvvu.tsv
	$(PE3GEN_CMD) uvvu.tsv

clean: localclean

localclean:
	-rm -f pe3_table.tex uvvu.tsv




