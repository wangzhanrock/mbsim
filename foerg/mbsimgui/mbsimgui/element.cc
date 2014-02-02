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
#include "element.h"
#include <QtGui/QMenu>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <cmath>
#include <boost/shared_ptr.hpp>
#include "frame.h"
#include "contour.h"
#include "solver.h"
#include "object.h"
#include "link.h"
#include "observer.h"
#include "mainwindow.h"

extern MainWindow *mw;
extern bool absolutePath;

using namespace std;
using namespace MBXMLUtils;
using namespace boost;
using namespace xercesc;

int Element::IDcounter=0;
string Element::unit="";
string Element::evaluation="";

Element::Element(const string &name_, Element *parent_) : parent(parent_), embed(0,false), name(name_), type("Element") {
//  property.push_back(new TextProperty("name",name_,""));
  embed.setProperty(new EmbedProperty(this));
  ID=toStr(IDcounter++);
}

string Element::getPath() {
 return parent?(parent->getPath()+"."+getValue()):getValue();
}

void Element::writeXMLFile(const string &name) {
  shared_ptr<DOMDocument> doc=MainWindow::parser->createDocument();
  writeXMLFile(doc.get());
  QFileInfo info(QString::fromStdString(name));
  QDir dir;
  if(!dir.exists(info.absolutePath()))
    dir.mkpath(info.absolutePath());
  DOMParser::serialize(doc.get(), (name.length()>4 && name.substr(name.length()-4,4)==".xml")?name:name+".xml");
}

void Element::initializeUsingXML(DOMElement *element) {
//  for(unsigned int i=0; i<plotFeature.size(); i++)
//    plotFeature[i]->initializeUsingXML(element);
}

DOMElement* Element::writeXMLFile(DOMNode *parent) {
  DOMDocument *doc=parent->getNodeType()==DOMNode::DOCUMENT_NODE ? static_cast<DOMDocument*>(parent) : parent->getOwnerDocument();
  DOMElement *ele0=D(doc)->createElement(getNameSpace()%getName());
  //name->writeXMLFile(ele0);
  E(ele0)->setAttribute("name", getValue());
//  for(unsigned int i=0; i<plotFeature.size(); i++)
//    plotFeature[i]->writeXMLFile(ele0);
  parent->insertBefore(ele0, NULL);
  return ele0;
}

void Element::initializeUsingXMLEmbed(DOMElement *element) {
  embed.initializeUsingXML(element);
//  embed.setActive(true);
}

DOMElement* Element::writeXMLFileEmbed(DOMNode *parent) {
  DOMElement *ele = embed.writeXMLFile(parent);
  if(!static_cast<const EmbedProperty*>(embed.getProperty())->hasFile())
    writeXMLFile(ele);
  else 
    //writeXMLFile(absolutePath?(mw->getUniqueTempDir().toStdString()+"/"+static_cast<const EmbedProperty*>(embed.getProperty())->getFile()):(static_cast<const EmbedProperty*>(embed.getProperty())->getFile()));
    writeXMLFile(absolutePath?(mw->getUniqueTempDir().generic_string()+"/"+static_cast<const EmbedProperty*>(embed.getProperty())->getFile()):(static_cast<const EmbedProperty*>(embed.getProperty())->getFile()));
  return ele;
}

string Element::getXMLPath(Element *ref, bool rel) {
  if(rel) {
    vector<Element*> e0, e1;
    Element* element = ref;
    e0.push_back(element);
    while(!dynamic_cast<Solver*>(element)) {
      element = element->getParent();
      e0.push_back(element);
    }
    element = parent;
    e1.push_back(element);
    while(!dynamic_cast<Solver*>(element)) {
      element = element->getParent();
      e1.push_back(element);
    }
    int imatch=0;
    for(vector<Element*>::iterator i0 = e0.end()-1, i1 = e1.end()-1 ; (i0 != e0.begin()-1) && (i1 != e1.begin()-1) ; i0--, i1--) 
      if(*i0 == *i1) imatch++;
    string type;
    if(dynamic_cast<Frame*>(this))
      type = "Frame";
    else if(dynamic_cast<Contour*>(this))
      type = "Contour";
    else if(dynamic_cast<Group*>(this))
      type = "Group";
    else if(dynamic_cast<Object*>(this))
      type = "Object";
    else if(dynamic_cast<Link*>(this))
      type = "Link";
    else if(dynamic_cast<Observer*>(this))
      type = "Observer";
    else 
      type = getName();
    string str = type + "[" + getValue() + "]";
    for(vector<Element*>::iterator i1 = e1.begin() ; i1 != e1.end()-imatch ; i1++) {
      if(dynamic_cast<Group*>(*i1))
        str = string("Group[") + (*i1)->getValue() + "]/" + str;
      else if(dynamic_cast<Object*>(*i1))
        str = string("Object[") + (*i1)->getValue() + "]/" + str;
      else if(dynamic_cast<Link*>(*i1))
        str = string("Link[") + (*i1)->getValue() + "]/" + str;
      else if(dynamic_cast<Observer*>(*i1))
        str = string("Observer[") + (*i1)->getValue() + "]/" + str;
      else
        str = "";
    }
    for(int i=0; i<int(e0.size())-imatch; i++)
      str = "../" + str;
    return str;
  } else {
    string type;
    if(dynamic_cast<Frame*>(this))
      type = "Frame";
    else if(dynamic_cast<Contour*>(this))
      type = "Contour";
    else if(dynamic_cast<Group*>(this))
      type = "Group";
    else if(dynamic_cast<Object*>(this))
      type = "Object";
    else if(dynamic_cast<Link*>(this))
      type = "Link";
    else if(dynamic_cast<Observer*>(this))
      type = "Observer";
    else 
      type = getName();
    string str = type + "[" + getValue() + "]";
    Element* element = parent;
    while(!dynamic_cast<Solver*>(element)) {
      if(dynamic_cast<Group*>(element))
        str = string("Group[") + element->getValue() + "]/" + str;
      else if(dynamic_cast<Object*>(element))
        str = string("Object[") + element->getValue() + "]/" + str;
      else if(dynamic_cast<Link*>(element))
        str = string("Link[") + element->getValue() + "]/" + str;
      else if(dynamic_cast<Observer*>(element))
        str = string("Observer[") + element->getValue() + "]/" + str;
      else
        str = "";
      element = element->getParent();
    }
    str = "/" + str;
    return str;
  }
}

ParameterList Element::getParameterList(bool addCounter) const {
  ParameterList list;
  const EmbedProperty *e = static_cast<const EmbedProperty*>(embed.getProperty());
  if(parent)
    list.addParameterList(parent->getParameterList(false));
  if(isEmbedded() && e->hasParameterFile())
    list.readXMLFile(e->getParameterFile());
  if(addCounter && e->hasCounter())
    list.addParameter(e->getCounterName(),"1","scalarParameter"); 
  return list;
}

void Element::update() {
  cout << "updating element" << name << endl;
  for(int i=0; i<property.size(); i++)
    property[i]->update();
  for(int i=0; i<getNumberOfObjects(); i++)
    getObject(i)->update();
}


