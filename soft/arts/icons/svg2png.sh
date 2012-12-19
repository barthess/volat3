#!/bin/bash

TMPFILE=/tmp/$$.svg

if [[ $2 == "" ]]
then
	echo "ERROR: you must pass maximum dimension of picture in pixels"
	echo "For example: '$0 pic.svg 60'"
	exit 1
else
	MAXDIM=$2
fi

# main code
filename="${1%.*}"
if [[ $3 == "" ]]
then
	cp $filename.svg $TMPFILE
else
	SEDSCRIPT="s/#ffffff/#$3/g"
	sed -e $SEDSCRIPT $filename.svg > $TMPFILE
fi

W=`inkscape $filename.svg -W`
H=`inkscape $filename.svg -H`
if [[ $W > $H ]]
then
	echo "W"
	inkscape $TMPFILE -y 0 -w $MAXDIM -e $filename.png
else
	echo "H"
	inkscape $TMPFILE -y 0 -h $MAXDIM -e $filename.png
fi

rm -f $TMPFILE
