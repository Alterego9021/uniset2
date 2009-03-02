# This file is a part of the NCS project. (c) 1999-2002 All rights reserved.
# $Id: idl.mk,v 1.2 2006/10/14 16:53:22 vpashka Exp $

# ����� ���� ��� IDL
# ����� ����� � ����, ��� ����� ����������� omniidl �����
# ����� ���-���� ��������, ��� ���������������� ������
# ��� ��������� ��������� IDL

IDLFLAGS = -I$(top_builddir)/IDL

# ��������� ������� ������������ ������
HHTARG=$(patsubst %.idl, ${HHDIR}/%.hh, ${IDLFILES})
CCTARG=$(patsubst %.idl, ${CCDIR}/%SK.cc, ${IDLFILES})

########################################################################

all: ${HHTARG} ${CCTARG}

dynamic: all

${HHTARG} ${CCTARG}: ${IDLFILES}
	for i in $^; do ${IDL} -v -bcxx ${IDLFLAGS} $$i; done
	mv --target-directory=${HHDIR} *.hh
	mv --target-directory=${CCDIR} *.cc

.PHONY: clean depend
clean:
	${RM} ${HHTARG} ${CCTARG}

depend:

install: