#include <sstream>
#include <iostream>
#include "Extensions.h"
#include "Schema.h"
// -------------------------------------------------------------------------
using namespace std;
using namespace UniSetExtensions;
// -------------------------------------------------------------------------

Schema::Schema()
{
}

Schema::~Schema()
{
}
// -------------------------------------------------------------------------
void Schema::link( Element::ElementID rootID, Element::ElementID childID, int numIn )
{
	std::shared_ptr<Element> e1;
	std::shared_ptr<Element> e2;

	auto it = emap.find(rootID);

	if( it == emap.end() )
	{
		ostringstream msg;
		msg << "Schema: элемент id=" << rootID << " NOT FOUND!";
		throw LogicException(msg.str());
	}

	e1 = it->second;

	it = emap.find(childID);

	if( it == emap.end() )
	{
		ostringstream msg;
		msg << "Schema: элемент id=" << childID << " NOT FOUND!";
		throw LogicException(msg.str());
	}

	e2 = it->second;

	e1->addChildOut(e2, numIn);

	// сохраняем в список соединений
	inLinks.push_front(INLink(e1, e2, numIn));
}
// -------------------------------------------------------------------------
void Schema::unlink( Element::ElementID rootID, Element::ElementID childID )
{
	std::shared_ptr<Element> e1;
	std::shared_ptr<Element> e2;

	auto it = emap.find(rootID);

	if( it == emap.end() )
	{
		ostringstream msg;
		msg << "Schema: элемент id=" << rootID << " NOT FOUND!";
		throw LogicException(msg.str());
	}

	e1 = it->second;

	it = emap.find( childID );

	if( it == emap.end() )
	{
		ostringstream msg;
		msg << "Schema: element id=" << childID << " NOT FOUND!";
		throw LogicException(msg.str());
	}

	e2 = it->second;

	e1->delChildOut(e2);

	// удаляем из списка соединений
	for( auto && lit = inLinks.begin(); lit != inLinks.end(); ++lit )
	{
		if( lit->from == e1 && lit->to == e2 )
		{
			inLinks.erase(lit);
			break;
		}
	}
}
// -------------------------------------------------------------------------
void Schema::extlink( const string& name, Element::ElementID childID, int numIn )
{
	auto it = emap.find(childID);

	if( it == emap.end() )
	{
		ostringstream msg;
		msg << "Schema: element id=" << childID << " NOT FOUND!";
		throw LogicException(msg.str());
	}

	auto el(it->second);

	//     добавляем новое соединение
	//    el->addInput(numIn);
	//    уже должен быть

	// заносим в список
	extLinks.push_front( EXTLink(name, el, numIn) );
}
// -------------------------------------------------------------------------
std::shared_ptr<Element> Schema::manage( std::shared_ptr<Element> el )
{
	dinfo << "Schema: manage new element id=" << el->getId()
		  << " type=" << el->getType()
		  << " inputs=" << el->inCount() << endl;

	emap[el->getId()] = el;
	return el;
}
// -------------------------------------------------------------------------
void Schema::remove( std::shared_ptr<Element> el )
{
	for( auto && it = emap.begin(); it != emap.end(); ++it )
	{
		if( it->second == el )
		{
			emap.erase(it);
			break;
		}
	}

	// помечаем внутренние связи
	for( auto && lit : inLinks )
	{
		if( lit.from == el )
			lit.from = 0;

		if( lit.to == el )
			lit.to = 0;
	}

	// помечаем внешние связи
	for( auto && lit : extLinks )
	{
		if( lit.to == el )
			lit.to = 0;
	}

}
// -------------------------------------------------------------------------
void Schema::setIn( Element::ElementID ID, int inNum, bool state )
{
	auto it = emap.find(ID);

	if( it != emap.end() )
		it->second->setIn(inNum, state);
}
// -------------------------------------------------------------------------
bool Schema::getOut( Element::ElementID ID )
{
	auto it = emap.find(ID);

	if( it != emap.end() )
		return it->second->getOut();

	ostringstream msg;
	msg << "Schema: element id=" << ID << " NOT FOUND!";
	throw LogicException(msg.str());
}
// -------------------------------------------------------------------------
std::shared_ptr<Element> Schema::find( Element::ElementID id )
{
	auto it = emap.find(id);

	if( it != emap.end() )
		return it->second;

	return nullptr;
}
// -------------------------------------------------------------------------
std::shared_ptr<Element> Schema::findExtLink( const string& name )
{
	// помечаем внешние связи
	for( const auto& it : extLinks )
	{
		if( it.name == name )
			return it.to;
	}

	return nullptr;
}
// -------------------------------------------------------------------------
std::shared_ptr<Element> Schema::findOut( const string& name )
{
	// помечаем внешние связи
	for( const auto& it : outList )
	{
		if( it.name == name )
			return it.from;
	}

	return nullptr;
}
// -------------------------------------------------------------------------
