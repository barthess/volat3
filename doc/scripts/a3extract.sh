#!/bin/bash
# ������ ��������� ��� �������� ����� ��� ����������
# ��� ������ ��� ��������� ��� ���������� 
# �� ������ ��������� 2 ����� � �������
# ${DOCUMENT}_a3.pdf � ${DOCUMENT}_a4.pdf

# ��� ��������������� ������ ��� ����������, �� � ��� ���������� �����
LOG=a3extract.log

# ����� ������ ����������
DOCUMENT=$1

# ������� ��������� ������
catch() 
{
	ERR=$?
	if [ "$ERR" != 0 ] ; then 
		echo "*** ERROR!" ; exit $ERR
	fi
}


# �������� ��� ���
echo "" > $LOG

# ������ ��� ����������
test -f ${DOCUMENT}.log
if [ "$?" != 0 ] 
then
	echo "*** ERROR! ${DOCUMENT}.log not found." | tee -a $LOG 
	echo "*** �re you forget to compile document?" ; exit 1
fi
 
# ���� �� ������������� �����
cat ${DOCUMENT}.log | grep -q -e '^eskdfreesize: sheet(s) with size'
if [ "$?" != 0 ] 
	then echo "*** Nothing to to extract. Exiting..." | tee -a $LOG ; exit 0
fi

# �������� ������ ���� ������� 
echo "*** generating list of all sheets" >> $LOG
seq `cat ${DOCUMENT}.log | grep -e "^Output written on ${DOCUMENT}.pdf (.*pages" | sed 's/^.*(//' | sed 's/ .*$//'` \
| tee -a $LOG > all_pages_list.tmp

# �������� ����� ������� ��� ���������
echo "*** generating list of non standard sheets" >> $LOG
cat ${DOCUMENT}.log | grep -e '^eskdfreesize: sheet(s) with size' | sed -e 's/^.*\[//' | sed 's/\]$//' \
| tee -a $LOG > non_standard_pages_list.tmp

# ������������ ������ �������� ������
echo "*** extracting non standard sheets to ${DOCUMENT}_a3.pdf" >> $LOG
pdftk ${DOCUMENT}.pdf cat \
`cat non_standard_pages_list.tmp | tr '\n' ' '` \
output ${DOCUMENT}-A3.pdf dont_ask verbose #| tee -a $LOG
catch


# ����� �������������� ������� ��������, ������ ���� ��������� ������
# �����������, ��� ����� ��������� ��� ��������� ��������
cat non_standard_pages_list.tmp | sed -e '/-/d' > unroll_list.tmp && \
cat non_standard_pages_list.tmp | grep '-' | sed -e 's/-/ /' | xargs -n 2 seq >> unroll_list.tmp
catch

echo "*** extracting standard sheets to ${DOCUMENT}_a4.pdf" >> $LOG
pdftk ${DOCUMENT}.pdf cat \
`cat unroll_list.tmp all_pages_list.tmp | sort -n | uniq -u | tr '\n' ' '` \
output ${DOCUMENT}-A4.pdf dont_ask verbose #| tee -a $LOG
catch

# cleanup
rm -f *list.tmp
# rm -f ${DOCUMENT}.pdf
