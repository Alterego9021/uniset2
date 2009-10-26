#!/bin/sh

SIG=-TERM
ARGS=$1

# �������� ��� ������ ������ ��� ������ ��������,
# ������� ����� ţ �� ������� :) (��. makefile :install)
. uniset-functions.sh

std=0
standart_control $std
set_omni

case $1 in
	--kill|-k)
		SIG="-KILL"
		;;
esac

checkPID=$(echo "$1" | grep pidfile=)
if [ -n "$checkPID" ]; then
	PID=$( echo $(cat $RUNDIR/${1#--pidfile=}) )
	echo "KILL PID: $PID "
	kill $PID
	exit 1;
fi


if [ ! -e $RANSERVICES ]
then
	echo �� ���������� $RANSERVICES � ����������� ���������
	exit -1
fi

for i in $(tac $RANSERVICES | cut -d " " -f 2)
do
	TOKILL=$(basename $i)
	echo -n ��������� $TOKILL... 
	if [ $(ps ax | grep $TOKILL | wc -l) = 0 ]
	then
		echo " already stoppped [ OK ]"
	else
		killall $SIG $TOKILL
		echo " [ OK ]"
	fi
done

rm -f $RANSERVICES

echo "[ OK ]"
