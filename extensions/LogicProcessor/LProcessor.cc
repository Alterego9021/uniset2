#include <iostream>
#include "Configuration.h"
#include "LProcessor.h"
// -------------------------------------------------------------------------
using namespace std;
using namespace UniSetTypes;
// -------------------------------------------------------------------------
LProcessor::LProcessor()
{
	sleepTime = conf->getArgPInt("--sleepTime", 200);
}

LProcessor::~LProcessor()
{
}
// -------------------------------------------------------------------------
void LProcessor::execute( const string lfile )
{
	build(lfile);

	while(1)
	{
		try
		{
			step();
		}
		catch( LogicException& ex )
		{
			cerr << "(LProcessor::execute): " << ex << endl;
		}
		catch( Exception& ex )
		{
			cerr << "(LProcessor::execute): " << ex << endl;
		}
		catch(...)
		{
			cerr << "(LProcessor::execute): catch...\n";
		}
		msleep(sleepTime);
	}
}
// -------------------------------------------------------------------------
void LProcessor::step()
{
	getInputs();
	processing();
	setOuts();
}		
// -------------------------------------------------------------------------
void LProcessor::build( const string& lfile )
{
	sch.read(lfile);
	
	// ���������� ����� ������� ������
	// ������, ��� � ���� name �������� �������� �������
	for( Schema::EXTiterator it=sch.extBegin(); it!=sch.extEnd(); ++it )
	{
		UniSetTypes::ObjectId sid = conf->getSensorID(it->name);
		if( sid == DefaultObjectId )
		{
			cerr << "�� ������ ������������� �������: " << it->name << endl;
			continue;	
		}
		
		EXTInfo ei;
		ei.sid = sid;
		ei.state = false;
		ei.lnk = &(*it);
#warning ���� ��� ������ �������������
		ei.iotype = UniversalIO::DigitalInput;
		extInputs.push_front(ei);
	}
	
	for( Schema::OUTiterator it=sch.outBegin(); it!=sch.outEnd(); ++it )
	{
		UniSetTypes::ObjectId sid = conf->getSensorID(it->name);
		if( sid == DefaultObjectId )
		{
			cerr << "�� ������ ������������� ������: " << it->name << endl;
			continue;
		}

		EXTOutInfo ei;
		ei.sid = sid;
		ei.lnk = &(*it);
#warning ���� ��� ������ �������������
		ei.iotype = UniversalIO::DigitalOutput;

		extOuts.push_front(ei);
	}
	
}
// -------------------------------------------------------------------------
/*!
	����� ���� ��������. ��������� ������� ��� ���������� ���������.
���������� ���������� �� �������. �.�. ���� �� ������� �������� ���� �� ����
������, �� �������� ������ ����� ��������. ����� ����� ����� �������� �� ���, ��� ����

*/
void LProcessor::getInputs()
{
	for( EXTList::iterator it=extInputs.begin(); it!=extInputs.end(); ++it )
	{
//		try
//		{
			it->state = ui.getState(it->sid);
//		}
	}
}
// -------------------------------------------------------------------------
void LProcessor::processing()
{
	// ��c������� ��� ������� �����
	for( EXTList::iterator it=extInputs.begin(); it!=extInputs.end();++it )
		it->lnk->to->setIn(it->lnk->numInput,it->state);

	// �������� �� ���� ���������
	for( Schema::iterator it=sch.begin(); it!=sch.end(); ++it )
		it->second->tick();
}
// -------------------------------------------------------------------------
void LProcessor::setOuts()
{
	// ��c������� ������
	for( OUTList::iterator it=extOuts.begin(); it!=extOuts.end(); ++it )
	{
		try
		{
			switch(it->iotype)
			{
				case UniversalIO::DigitalInput:
					ui.saveState(it->sid,it->lnk->from->getOut(),it->iotype);
				break;

				case UniversalIO::DigitalOutput:
					ui.setState(it->sid,it->lnk->from->getOut());
				break;
				
				default:
					cerr << "(LProcessor::setOuts): ���������������� ��� iotype=" << it->iotype << endl;
					break;
			}
		}
		catch( Exception& ex )
		{
			cerr << "(LProcessor::setOuts): " << ex << endl;
		}
		catch(...)
		{
			cerr << "(LProcessor::setOuts): catch...\n";
		}
	}
}
// -------------------------------------------------------------------------
