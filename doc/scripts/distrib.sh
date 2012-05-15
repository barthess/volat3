#!/bin/sh
# Usage: 
# distrib filename_without_extension
# results will send to stdout

DOCUMENT=$1
n=0

# delete commented lines (may be commented %\documentclass
((n++))
sed -e '/^%/d' $DOCUMENT.tex > _tmp$n.tex


# delete change sheet
((n++))
sed -e '/input{.*change_sheet.*}/d' _tmp$((n-1)).tex > _tmp$n.tex

# switch in twoside mode (if already twoside - nothing bad)
((n++))
sed -e 's/\\documentclass\[/\\documentclass\[twoside,vpadding=17.4mm,/' _tmp$((n-1)).tex > _tmp$n.tex

# switch to plain style
# and remove unnecessary fields from title sheet
((n++))
sed -e 's/\\begin{document}/\
	\\usepackage[numberbottom,numberright]{eskdplain}\
	\\renewcommand{\\ESKDtheTitleFieldX}{}\
	\\renewcommand{\\ESKDtheTitleFieldVIII}{}\
	\\renewcommand{\\ESKDtheTitleFieldIII}{}\
	\\ESKDcompany{\\includegraphics[width=2cm]{kamerton_logo}\\par\\medskip\\nameManufacturer}\
	\\begin{document}/' _tmp$((n-1)).tex > _tmp$n.tex


((n++))
sed -e '/\\usepackage{passportkamertonstyle/d' _tmp$((n-1)).tex > _tmp$n.tex
sed -e 's/\\makekamertontitle/\\maketitle/'  _tmp$n.tex 


# clean 
rm _tmp*.*

