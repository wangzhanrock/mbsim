/*
   MBSimGUI - A fronted for MBSim.
   Copyright (C) 2012 Martin Förg

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
   */

#include <config.h>
#include "link.h"
#include "objectfactory.h"
#include "mainwindow.h"
#include "embed.h"

using namespace std;
using namespace MBXMLUtils;
using namespace boost;
using namespace xercesc;

namespace MBSimGUI {

  Link::Link(const string &str, Element *parent) : Element(str, parent) {
  }

  Link::~Link() {
  }

  Link* Link::readXMLFile(const string &filename, Element *parent) {
    shared_ptr<DOMDocument> doc=MainWindow::parser->parse(filename);
    DOMElement *e=doc->getDocumentElement();
//    Link *link=ObjectFactory::getInstance()->createLink(e, parent);
    Link *link=Embed<Link>::createAndInit(e,parent);
    if(link) {
//      link->initializeUsingXML(e);
      link->initialize();
    }
    return link;
  }

  //void Link::initializeUsingXML(DOMElement *element) {
  //}

  //DOMElement* Link::writeXMLFile(DOMNode *parent) {    
  //}

}
