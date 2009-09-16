/* This file is part of the UniSet project
 * Copyright (c) 2002 Free Software Foundation, Inc.
 * Copyright (c) 2002 Vitaly Lipatov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
// --------------------------------------------------------------------------
/*! \file
 *  \author Vitaly Lipatov
 *  \date   $Date: 2006/12/20 10:39:04 $
 *  \version $Id: UniXML.h,v 1.9 2006/12/20 10:39:04 vpashka Exp $
 *  \par
 
 *	\bug �� �������� ������� findNode. (�� ���� �� ���� name, ���� ������)
 */
// --------------------------------------------------------------------------

// ����� ��� ������ � ������� � XML, ������� � ���������������

#ifndef UniXML_H_
#define UniXML_H_
 
#include <assert.h>
#include <string>

#include <libxml/parser.h>
#include <libxml/tree.h>

class UniXML
{
public:

	inline xmlNode* getFirstNode()
	{
		return xmlDocGetRootElement(doc);
	}

	// ��������� ��������� ����
	void open(const std::string filename);

	void close();
	inline bool isOpen(){ return doc!=0; }
	UniXML(const std::string filename);

	UniXML();

	~UniXML();

	xmlNode* cur;
	xmlDoc* doc;
	std::string filename;
	
	// ��������� ���������� (� ���������) � ������� (�������� � XML)
	static const std::string InternalEncoding;
	static const std::string ExternalEncoding;
	static const std::string xmlEncoding;
	

	// �������������� ��������� ������ �� XML � ������ ������ ����������� �������������
	static std::string xml2local(const xmlChar* xmlText);

	// �������������� ��������� ������ �� ������ ����������� ������������� � ������ ��� XML
	// ���������� ��������� �� ��������� �����, ������� ���� �� ��� ������ �������.
	static const xmlChar* local2xml(std::string text);
	
	// ������� ����� XML-��������
	void newDoc(const std::string& root_node, std::string xml_ver="1.0");

	// �������� �������� name ���������� ���� node
	static std::string getProp(xmlNode* node, const std::string name);
	static std::string getPropUtf8(xmlNode* node, const std::string name);
	static int getIntProp(xmlNode* node, const std::string name);
	/// if value if not positive ( <= 0 ), returns def
	static int getPIntProp(xmlNode* node, const std::string name, int def);
	
	// ���������� �������� name ���������� ���� node
	static void setProp(xmlNode* node, const std::string name, const std::string text);
	
	// �������� ����� �������� ����
	static xmlNode* createChild(xmlNode* node, const std::string title, const std::string text);
	
	// �������� ��������� ����
	static xmlNode* createNext(xmlNode* node, const std::string title, const std::string text);
	
	// ������� ��������� ���� � ��� ��������� ����
	static void removeNode(xmlNode* node);
	
	// ������� ��������� ���� � ��� ��������� ����
	static xmlNode* copyNode(xmlNode* node, int recursive=1);
	
	// ��������� � ����, ���� �������� �� ������, ��������� � ��� ����
	// ������� ��� �������� ���������.
	bool save(const std::string filename="", int level = 2);

	// ����������� ��������� � ���������� ����
	static xmlNode* nextNode(xmlNode* node);

	// ����� �������� ��������� ����������� �������� �� �������,
	// ��������� ->parent
	xmlNode* findNode(xmlNode* node, const std::string searchnode, const std::string name = "");

	xmlNode* extFindNode(xmlNode* node, int depth, int width, const std::string searchnode, const std::string name = "", bool top=true );


protected:
	static int recur;

};

class UniXML_iterator
{
	public:
		UniXML_iterator(xmlNode* node) :
			curNode(node)
		{}
		UniXML_iterator() {}

		std::string getProp(const std::string name);
		std::string getPropUtf8(const std::string name);
		int getIntProp(const std::string name);
		/// if value if not positive ( <= 0 ), returns def
		int getPIntProp(const std::string name, int def);
		void setProp(const std::string name, const std::string text);
		
		/*! ������� � ���������� ����. ���������� false, ���� ������ ������� */
		bool goNext();

		/*! ������� �������� � ���������� ����. ���������� false, ���� ������ ������� */
		bool goThrowNext();
		
		/*! ������� � ����������� ���� */
		bool goPrev();
		
		bool canPrev();
		bool canNext();
		
		// ������� � ���������� ����
		void operator ++()
		{
			goNext();
		}
		
		// ������� � ����������� ����
		void operator --()
		{
			goPrev();
		}
		
		/*! ������� �� ���� ������� ���� 
			\note ���� ������� �� �������, �������� �������� ��������� �� ������� ����
		*/
		bool goParent();
		
		/*! ������� �� ���� ������� ���� 
			\note ���� ������� �� �������, �������� �������� ��������� �� ������� ����
		*/
		bool goChildren();
		
		// �������� ������� ����
		xmlNode* getCurrent() const
		{
			return curNode;
		}

		// �������� �������� �������� ����
		const std::string getName() const
		{
			if( curNode )
				return (char*) curNode->name;
			else
				return "";
		}

		operator xmlNode*()
		{
			//unideb << "current\n";
			return curNode;
		}

		inline void goBegin()
		{
			while(canPrev()){goPrev();}
		}

		inline void goEnd()
		{
			while(canNext()){goNext();}
		}

	private:
		xmlNode* curNode;
};


#endif
