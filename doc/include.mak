#------------------------------------------------------------------------------
# переменные	
#

# каталог для готовой продукции
OUTPUT_DIR = $(TOP_DIR)/output

# каталог со скриптами вспомогательными бАшевыми
SCRIPTS_DIR = $(TOP_DIR)/scripts

DEPS = $(TOP_DIR)/tex/*.sty $(TOP_DIR)/tex/*.tex $(SCRIPTS_DIR)/*.sh
DEPS += $(shell find $(TOP_DIR)/templates -name '*.tex')
DEPS += $(LOCAL_DEPS) # а это в каждом локальном мейкфайле своё, по каким-то причинам не находящееся общими алгоритмами

# *.aux файлы будем складывать в отдельную директорию, чтобы
# параллельные сборки не мешали друг-другу
AUX_DIR = aux_files/$@
TEXOPTION = --max-print-line=65000 --time-statistics
# TEXOPTION = --max-print-line=65000 --time-statistics --aux-directory=$(AUX_DIR)

TEX = latex $(TEXOPTION)
PDFTEX = pdflatex $(TEXOPTION)

TEXIFY = mkdir -p $(AUX_DIR) ; texify --tex-option="$(TEXOPTION)" 
PDFTEXIFY = mkdir -p $(AUX_DIR) ; texify --tex-option="$(TEXOPTION)" -p

HYPERREFOPTS = bookmarksopen=true,bookmarksnumbered=true,colorlinks=true,unicode

# строки для дебага
# OLD_SHELL := $(SHELL)
# SHELL = $(warning [$@ ($^)($?)])$(OLD_SHELL)

#------------------------------------------------------------------------------
# макросы
#

# Составление списка рисунков. 
# Пока что производится глобальный поиск _всех_ рисунков,
# как из них выделить зависимые - не знаю
#
# Персонально для МС Висио придется написать костыль, который будет 
# прерывать компиляцию,
# если рисунок изменился. После чего надо будет руками его преобразовать
# в pdf
#
figpdf := 	$(shell find $(TOP_DIR) -name '*.svg' | sed -e 's/.svg$$/.pdf/')

figeps := 	$(shell find $(TOP_DIR) -name '*.svg' | sed -e 's/.svg$$/.eps/';\
   			find $(TOP_DIR) -name '*.jpg' | sed -e 's/.jpg$$/.eps/';\
			find $(TOP_DIR) -name '*.png' | sed -e 's/.png$$/.eps/')

figvsdpdf:=	$(shell find $(TOP_DIR) -name '*.vsd' | sed -e 's/.vsd$$/.pdf/')
figvsdeps:=	$(shell find $(TOP_DIR) -name '*.vsd' | sed -e 's/.vsd$$/.eps/')

# а исходники латеха ищутся только в текущей дире
texsrc := 	$(shell find -name '*.tex')

# при запуске inkscape под виндой необходимо указывать абсолютные пути к файлам
# сделал это через cygpath
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


# добавим к зависимостям картинки и исходники
DEPS += $(texsrc) $(figpdf) $(figvsdpdf)
DVIDEPS =  $(figeps) $(figvsdeps)


#------------------------------------------------------------------------------
# правила для сборки
#
# это правило должно быть обработано первым, чтобы быть главной целью
# semiall - это локальное правило для каждого документа
all: semiall

allbw: semiallbw

reallyall: all allbw

# отключим неявное правило сборки dvi из tex
%.dvi: %.tex 

%.pdf: %.svg 
	$(anytopdf)

%.eps: %.svg
	$(anytoeps)

%.eps: %.jpg
	$(anytoeps)

%.eps: %.png
	$(anytoeps)


# это правило для сборки eps из vsd через заранее заготовленные pdf
%.eps: %.vsd %.pdf
	$(vsdtoeps)

# автоматически скомпилировать pdf из vsd не получится
# поэтому просто выбросим ошибку
%.pdf: %.vsd
	$(error ERROR!. File $@ is out of date. Manually convert it from $<)


#------------------------------------------------------------------------------
# набор абстрактных целей для вставки в локальные мейкфайлы, либо для
# персонального вызова по мере надобности в каталоге с главным файлом 
# документа
# По умолчанию будут собираться только цветные документы
# Цели *.touch.tmp используются в тех случаях, когда заранее не известно,
# какие файлы будут собраны в результате отработки правила
#
# доки для вычитки
reading: pdf hyperref
	cp $(document).pdf $(document)__reading.pdf
	
# документ для выкладывания на сервер
pdf: $(document).pdf

# документ с гиперссылками
hyperref: $(document)__reading-hyperref.pdf

# документ в формате dvi. Наверное не нужен, но пусть валяется
dvi: $(document).dvi

# документ для печати в бумажный архив
archive: $(document)__archive.touch.tmp

# документ для печати в бумажный архив (black'n'white)
archivebw: $(document)__archive-bw.touch.tmp

# документ для поставки
distrib: $(document)__distrib.touch.tmp

# документ для поставки (black'n'white)
distribbw: $(document)__distrib-bw.touch.tmp

# буклет
booklet: $(document)__distrib-booklet-1.pdf $(document)__distrib-booklet-2.pdf

# буклет (black'n'white)
bookletbw: $(document)__distrib-bw-booklet-1.pdf $(document)__distrib-bw-booklet-2.pdf



# правило инсталляции. Должно зависеть как минимуму от цели "pdf"
install: pdf
	-cp -uv *.pdf $(OUTPUT_DIR)



#------------------------------------------------------------------------------
# эти правила будут собирать из главного файла непосредственно документы
#
$(document).pdf: $(DEPS)
	$(PDFTEXIFY) $(document).tex

reading_index.tmp:
	echo "\href{$(document).pdf}{Документ для вычитки и для сервера}" > reading_index.tmp

$(document).dvi: $(DEPS) $(DVIDEPS)
	$(TEXIFY) $(document).tex

# сборка со ссылками через hyperref
$(document)__reading-hyperref.pdf: $(DEPS)
	-find -name '*.aux' | xargs rm -f
	sed -e '/^%/d' $(document).tex |\
	sed -e 's/\\begin{document}/\\usepackage[$(HYPERREFOPTS)]{hyperref}\\begin\{document}/' > $(document)__reading-hyperref.tex;\
	$(PDFTEXIFY) $(document)__reading-hyperref.tex;\
	rm -f $(document)__reading-hyperref.tex

# для бумажного архива
$(document)__archive.touch.tmp: $(document)__archive.pdf
	$(SCRIPTS_DIR)/a3extract.sh $(document)__archive &&\
	$(SCRIPTS_DIR)/preprint.sh $(document)__archive &&\
	touch $(document)__archive.touch.tmp 

$(document)__archive.pdf: $(DEPS)
	sed -e '/^%/d' $(document).tex |\
	sed -e 's/\\begin{document}/\\setboolean{reading}{false}\\begin\{document}/' > $(document)__archive.tex &&\
	$(PDFTEXIFY) $(document)__archive.tex &&\
	rm -f $(document)__archive.tex

# для бумажного архива (black'n'white)
$(document)__archive-bw.touch.tmp: $(document)__archive-bw.pdf
	$(SCRIPTS_DIR)/a3extract.sh $(document)__archive-bw &&\
	$(SCRIPTS_DIR)/preprint.sh $(document)__archive-bw &&\
	touch $(document)__archive-bw.touch.tmp 

$(document)__archive-bw.pdf: $(DEPS)
	sed -e '/^%/d' $(document).tex |\
	sed -e 's/\\begin{document}/\\setboolean{reading}{false}\\setboolean{blacknwhite}{true}\\begin\{document}/' > $(document)__archive-bw.tex &&\
	$(PDFTEXIFY) $(document)__archive-bw.tex &&\
	rm -f $(document)__archive-bw.tex


# для поставки
$(document)__distrib.touch.tmp: $(document)__distrib.pdf
	$(SCRIPTS_DIR)/a3extract.sh $(document)__distrib &&\
	touch $(document)__distrib.touch.tmp 

$(document)__distrib.pdf: $(DEPS)
	$(SCRIPTS_DIR)/distrib.sh $(document) |\
	sed -e 's/\\begin{document}/\\setboolean{reading}{false}\\begin\{document}/' > $(document)__distrib.tex &&\
	$(PDFTEXIFY) $(document)__distrib.tex &&\
	rm -f $(document)__distrib.tex


# для поставки (black'n'white)
$(document)__distrib-bw.touch.tmp: $(document)__distrib-bw.pdf
	$(SCRIPTS_DIR)/a3extract.sh $(document)__distrib-bw &&\
	touch $(document)__distrib-bw.touch.tmp 

$(document)__distrib-bw.pdf: $(DEPS)
	$(SCRIPTS_DIR)/distrib.sh $(document) |\
	sed -e 's/\\begin{document}/\\setboolean{reading}{false}\\setboolean{blacknwhite}{true}\\begin\{document}/' > $(document)__distrib-bw.tex &&\
	$(PDFTEXIFY) $(document)__distrib-bw.tex &&\
	rm -f $(document)__distrib-bw.tex

# сборка двух файлов буклета
$(document)__distrib-booklet-1.pdf $(document)__distrib-booklet-2.pdf : $(document)__distrib.pdf
	$(SCRIPTS_DIR)/booklet.sh $(document)__distrib

# сборка двух файлов буклета (black'n'white)
$(document)__distrib-bw-booklet-1.pdf $(document)__distrib-bw-booklet-2.pdf : $(document)__distrib-bw.pdf
	$(SCRIPTS_DIR)/booklet.sh $(document)__distrib-bw





#------------------------------------------------------------------------------
# очистки
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
