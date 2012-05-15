#------------------------------------------------------------------------------
# ����������	
#

# ������� ��� ������� ���������
OUTPUT_DIR = $(TOP_DIR)/output

# ������� �� ��������� ���������������� ��������
SCRIPTS_DIR = $(TOP_DIR)/scripts

DEPS = $(TOP_DIR)/tex/*.sty $(TOP_DIR)/tex/*.tex $(SCRIPTS_DIR)/*.sh
DEPS += $(shell find $(TOP_DIR)/templates -name '*.tex')
DEPS += $(LOCAL_DEPS) # � ��� � ������ ��������� ��������� ���, �� �����-�� �������� �� ����������� ������ �����������

# *.aux ����� ����� ���������� � ��������� ����������, �����
# ������������ ������ �� ������ ����-�����
AUX_DIR = aux_files/$@
TEXOPTION = --max-print-line=65000 --time-statistics
# TEXOPTION = --max-print-line=65000 --time-statistics --aux-directory=$(AUX_DIR)

TEX = latex $(TEXOPTION)
PDFTEX = pdflatex $(TEXOPTION)

TEXIFY = mkdir -p $(AUX_DIR) ; texify --tex-option="$(TEXOPTION)" 
PDFTEXIFY = mkdir -p $(AUX_DIR) ; texify --tex-option="$(TEXOPTION)" -p

HYPERREFOPTS = bookmarksopen=true,bookmarksnumbered=true,colorlinks=true,unicode

# ������ ��� ������
# OLD_SHELL := $(SHELL)
# SHELL = $(warning [$@ ($^)($?)])$(OLD_SHELL)

#------------------------------------------------------------------------------
# �������
#

# ����������� ������ ��������. 
# ���� ��� ������������ ���������� ����� _����_ ��������,
# ��� �� ��� �������� ��������� - �� ����
#
# ����������� ��� �� ����� �������� �������� �������, ������� ����� 
# ��������� ����������,
# ���� ������� ���������. ����� ���� ���� ����� ������ ��� �������������
# � pdf
#
figpdf := 	$(shell find $(TOP_DIR) -name '*.svg' | sed -e 's/.svg$$/.pdf/')

figeps := 	$(shell find $(TOP_DIR) -name '*.svg' | sed -e 's/.svg$$/.eps/';\
   			find $(TOP_DIR) -name '*.jpg' | sed -e 's/.jpg$$/.eps/';\
			find $(TOP_DIR) -name '*.png' | sed -e 's/.png$$/.eps/')

figvsdpdf:=	$(shell find $(TOP_DIR) -name '*.vsd' | sed -e 's/.vsd$$/.pdf/')
figvsdeps:=	$(shell find $(TOP_DIR) -name '*.vsd' | sed -e 's/.vsd$$/.eps/')

# � ��������� ������ ������ ������ � ������� ����
texsrc := 	$(shell find -name '*.tex')

# ��� ������� inkscape ��� ������ ���������� ��������� ���������� ���� � ������
# ������ ��� ����� cygpath
ink_opts := --without-gui --export-text-to-path --export-area-drawing --export-dpi=600 

ifeq ($(shell uname -o),Cygwin)
	anytoeps = inkscape $(ink_opts) --export-eps=`cygpath -aw $@` `cygpath -aw $<`
	anytopdf = inkscape $(ink_opts) --export-pdf=`cygpath -aw $@` `cygpath -aw $<`
	vsdtoeps = inkscape $(ink_opts) --export-eps=`cygpath -aw $@` `cygpath -aw $*.pdf`
else
	anytoeps = inkscape $(ink_opts) --export-eps=$@ $<
	anytopdf = inkscape $(ink_opts) --export-pdf=$@ $<
	vsdtoeps = inkscape $(ink_opts) --export-eps=$@ $*.pdf
endif


# ������� � ������������ �������� � ���������
DEPS += $(texsrc) $(figpdf) $(figvsdpdf)
DVIDEPS =  $(figeps) $(figvsdeps)


#------------------------------------------------------------------------------
# ������� ��� ������
#
# ��� ������� ������ ���� ���������� ������, ����� ���� ������� �����
# semiall - ��� ��������� ������� ��� ������� ���������
all: semiall

allbw: semiallbw

reallyall: all allbw

# �������� ������� ������� ������ dvi �� tex
%.dvi: %.tex 

%.pdf: %.svg 
	$(anytopdf)

%.eps: %.svg
	$(anytoeps)

%.eps: %.jpg
	$(anytoeps)

%.eps: %.png
	$(anytoeps)


# ��� ������� ��� ������ eps �� vsd ����� ������� ������������� pdf
%.eps: %.vsd %.pdf
	$(vsdtoeps)

# ������������� �������������� pdf �� vsd �� ���������
# ������� ������ �������� ������
%.pdf: %.vsd
	$(error ERROR!. File $@ is out of date. Manually convert it from $<)


#------------------------------------------------------------------------------
# ����� ����������� ����� ��� ������� � ��������� ���������, ���� ���
# ������������� ������ �� ���� ���������� � �������� � ������� ������ 
# ���������
# �� ��������� ����� ���������� ������ ������� ���������
# ���� *.touch.tmp ������������ � ��� �������, ����� ������� �� ��������,
# ����� ����� ����� ������� � ���������� ��������� �������
#
# ���� ��� �������
reading: pdf hyperref
	cp $(document).pdf $(document)__reading.pdf
	
# �������� ��� ������������ �� ������
pdf: $(document).pdf

# �������� � �������������
hyperref: $(document)__reading-hyperref.pdf

# �������� � ������� dvi. �������� �� �����, �� ����� ��������
dvi: $(document).dvi

# �������� ��� ������ � �������� �����
archive: $(document)__archive.touch.tmp

# �������� ��� ������ � �������� ����� (black'n'white)
archivebw: $(document)__archive-bw.touch.tmp

# �������� ��� ��������
distrib: $(document)__distrib.touch.tmp

# �������� ��� �������� (black'n'white)
distribbw: $(document)__distrib-bw.touch.tmp

# ������
booklet: $(document)__distrib-booklet-1.pdf $(document)__distrib-booklet-2.pdf

# ������ (black'n'white)
bookletbw: $(document)__distrib-bw-booklet-1.pdf $(document)__distrib-bw-booklet-2.pdf



# ������� �����������. ������ �������� ��� �������� �� ���� "pdf"
install: pdf
	-cp -uv *.pdf $(OUTPUT_DIR)



#------------------------------------------------------------------------------
# ��� ������� ����� �������� �� �������� ����� ��������������� ���������
#
$(document).pdf: $(DEPS)
	$(PDFTEXIFY) $(document).tex

reading_index.tmp:
	echo "\href{$(document).pdf}{�������� ��� ������� � ��� �������}" > reading_index.tmp

$(document).dvi: $(DEPS) $(DVIDEPS)
	$(TEXIFY) $(document).tex

# ������ �� �������� ����� hyperref
$(document)__reading-hyperref.pdf: $(DEPS)
	-find -name '*.aux' | xargs rm -f
	sed -e '/^%/d' $(document).tex |\
	sed -e 's/\\begin{document}/\\usepackage[$(HYPERREFOPTS)]{hyperref}\\begin\{document}/' > $(document)__reading-hyperref.tex;\
	$(PDFTEXIFY) $(document)__reading-hyperref.tex;\
	rm -f $(document)__reading-hyperref.tex

# ��� ��������� ������
$(document)__archive.touch.tmp: $(document)__archive.pdf
	$(SCRIPTS_DIR)/a3extract.sh $(document)__archive &&\
	$(SCRIPTS_DIR)/preprint.sh $(document)__archive &&\
	touch $(document)__archive.touch.tmp 

$(document)__archive.pdf: $(DEPS)
	sed -e '/^%/d' $(document).tex |\
	sed -e 's/\\begin{document}/\\setboolean{reading}{false}\\begin\{document}/' > $(document)__archive.tex &&\
	$(PDFTEXIFY) $(document)__archive.tex &&\
	rm -f $(document)__archive.tex

# ��� ��������� ������ (black'n'white)
$(document)__archive-bw.touch.tmp: $(document)__archive-bw.pdf
	$(SCRIPTS_DIR)/a3extract.sh $(document)__archive-bw &&\
	$(SCRIPTS_DIR)/preprint.sh $(document)__archive-bw &&\
	touch $(document)__archive-bw.touch.tmp 

$(document)__archive-bw.pdf: $(DEPS)
	sed -e '/^%/d' $(document).tex |\
	sed -e 's/\\begin{document}/\\setboolean{reading}{false}\\setboolean{blacknwhite}{true}\\begin\{document}/' > $(document)__archive-bw.tex &&\
	$(PDFTEXIFY) $(document)__archive-bw.tex &&\
	rm -f $(document)__archive-bw.tex


# ��� ��������
$(document)__distrib.touch.tmp: $(document)__distrib.pdf
	$(SCRIPTS_DIR)/a3extract.sh $(document)__distrib &&\
	touch $(document)__distrib.touch.tmp 

$(document)__distrib.pdf: $(DEPS)
	$(SCRIPTS_DIR)/distrib.sh $(document) |\
	sed -e 's/\\begin{document}/\\setboolean{reading}{false}\\begin\{document}/' > $(document)__distrib.tex &&\
	$(PDFTEXIFY) $(document)__distrib.tex &&\
	rm -f $(document)__distrib.tex


# ��� �������� (black'n'white)
$(document)__distrib-bw.touch.tmp: $(document)__distrib-bw.pdf
	$(SCRIPTS_DIR)/a3extract.sh $(document)__distrib-bw &&\
	touch $(document)__distrib-bw.touch.tmp 

$(document)__distrib-bw.pdf: $(DEPS)
	$(SCRIPTS_DIR)/distrib.sh $(document) |\
	sed -e 's/\\begin{document}/\\setboolean{reading}{false}\\setboolean{blacknwhite}{true}\\begin\{document}/' > $(document)__distrib-bw.tex &&\
	$(PDFTEXIFY) $(document)__distrib-bw.tex &&\
	rm -f $(document)__distrib-bw.tex

# ������ ���� ������ �������
$(document)__distrib-booklet-1.pdf $(document)__distrib-booklet-2.pdf : $(document)__distrib.pdf
	$(SCRIPTS_DIR)/booklet.sh $(document)__distrib

# ������ ���� ������ ������� (black'n'white)
$(document)__distrib-bw-booklet-1.pdf $(document)__distrib-bw-booklet-2.pdf : $(document)__distrib-bw.pdf
	$(SCRIPTS_DIR)/booklet.sh $(document)__distrib-bw





#------------------------------------------------------------------------------
# �������
clean:
	-rm -f $(document)*.{tex~,tps,aux,lof,bak,lot,dvi,toc,blg,bbl,idx,*.gz*,tmp,out,pdf}
	-rm -f *latexSuiteTemp* *.origin *.ps *.bak *.log _tmp*.* *.fls
	-find -name '*.aux'    | xargs rm -f
	-find -name '*.tmp'    | xargs rm -f
	-find -name '*~'       | xargs rm -f
	-find -name '*.origin' | xargs rm -f

distclean: clean
	-rm -f $(figpdf)
	-rm -f $(figeps)
	-rm -f $(figvsdeps)
	-rm -f $(document)*.pdf

maintainerclean: distclean
	-rm -f $(figvsdpdf)

.PHONY : clean distclean maintainerclean pdf dvi semiall install twoside colorprint bwprint bw booklet
