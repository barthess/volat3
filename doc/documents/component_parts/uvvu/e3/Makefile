# имя главного файла без расширения
document := KCAY.469555.025pe3

TOP_DIR = ../../../..

include $(TOP_DIR)/include.mak

semiall: pdf archive 

semiallbw: archivebw 

$(document).tex: pe3_table.tex 

# пока что автоматически сконвертать не получается
pe3_table.tex: $(TOP_DIR)/../hard/pcb_main/bom/volat3-PARTSDB.tsv
	$(error ERROR!. File $@ is out of date. Generate new table from $< manually)

# clean: localclean
# 
# localclean:
# 	-rm -f content_single.tmp




