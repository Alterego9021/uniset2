#!/bin/sh
USERID=0
BASEOMNIPORT=2809

# �������� ��� ���������� ����� ������������
function get_userid()
{
	USERID=$(expr $UID + 50000)
}

# usage: standart_control {1/0} - {standart port/debug port}
function standart_control()
{
	if [ -z $TMPDIR ]
	then
		TMPDIR=$HOME/tmp
		echo �� ���������� ���������� ��������� TMPDIR. ���������� $TMPDIR
	else
		echo ���������� TMPDIR=$TMPDIR
	fi

	if [ $1 = 1 ]; then
		TMPDIR=/var/tmp
		echo ���������� ����������� ���� Omni: $BASEOMNIPORT � ��������� ������� $TMPDIR
	else
		get_userid
		if [ $USERID = 0 ]
		then
			echo �� ��������� ��������� ������������ $(whoami) � uid=$UID
			exit 0
		fi
	fi
}

function set_omni_port
{
	while [ -n "$1" ]; do
		case "$1" in
			-p|--port) 
				shift
					OMNIPORT=$1;
					echo  "set OMNIPORT=$1" 
				shift; 
				break;
			;;

			*)
				shift
			;;
		esac
	done
}

function set_omni 
{
	# ������� ��� �������� ������� omniORB
	OMNILOG=$TMPDIR/omniORB

	# ���� ��� �������� ������� ���������� � ������� ������ ���������
	RANSERVICES=$OMNILOG/ran.list
	touch $RANSERVICES

	OMNINAME=omniNames

	OMNIPORT=$(expr $USERID + $BASEOMNIPORT)

	if [ $(grep "$OMNIPORT/" /etc/services | wc -l) \> 0 ]
	then
		if [ $USERID = 0 ]
		then
			echo INFO: ������ � ����� $OMNIPORT ������������ � /etc/services.
		else
			echo ��������, ���� $OMNIPORT ��� ������������ � /etc/services.
			echo ������ omniNames ����������.
			echo �����������
			exit 0
		fi
	fi
	[ -e $(which $OMNINAME) ]  || { echo Error: ������� $OMNINAME �� ������� ; exit 0; }

}


function runOmniNames()
{
	RETVAL=1
	omniTest=0
	if [ $std = 1 ]; then
		omniTest=$(ps -ax | grep $OMNINAME | grep -v grep | grep -v $0 | wc -l);
	else
		omniTest=$(ps -aux | grep $OMNINAME | grep $USER  | grep -v grep | grep -v $0 | wc -l);
	fi

	if [ $omniTest \> 0 ]; 
	then	 
		echo $OMNINAME ��� �������. #���������.
		return 0;
	fi

	if [ ! -d $OMNILOG ]
	then
		mkdir -p $OMNILOG
		echo ������ omniNames ������ ��� � ������ $OMNIPORT
		$OMNINAME -start $OMNIPORT -logdir $OMNILOG &>$OMNILOG/background.output &
		pid=$!
		echo �������� ��������� ����������� ��������
	else
		echo ������� ������ omniNames. ���� ���� ��������, ������� $OMNILOG
		$OMNINAME -logdir $OMNILOG &>$OMNILOG/background.output &
		pid=$!
	fi
	RET=$?
	if [ $RET = 0 ]; then
		if [ $WITH_PID = 1 ]; then
			echo $pid >"$RUNDIR/$OMNINAME.pid" # ������� pid-����
		fi;
	else
		echo ������ omniNames �� ������
		return 1;
	fi
	#echo $! $OMNINAME >>$RANSERVICES
	
	if [ $(grep $OMNINAME $RANSERVICES | wc -l) \= 0 ]
	then
		echo 0 $OMNINAME >>$RANSERVICES
	fi

	# �������� �� ������ omniNames -�
	yes=$(echo $* | grep omniNames )
	if [ -n "$yes" ]; then
		echo ������ omniNames [ OK ]
		$RETVAL=0
	fi

	return $RETVAL
}