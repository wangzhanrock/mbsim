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
#include "basic_properties.h"
#include "frame.h"
#include "contour.h"
#include "rigidbody.h"
#include "signal_.h"
#include "basic_widgets.h"
#include "variable_widgets.h"
#include "kinematics_widgets.h"
#include "extended_widgets.h"
#include <QDir>
#include <mbxmlutilstinyxml/tinyxml.h>
#include <mbxmlutilstinyxml/tinynamespace.h>

using namespace std;
using namespace MBXMLUtils;

extern bool absolutePath;
extern QDir mbsDir;

LocalFrameOfReferenceProperty::LocalFrameOfReferenceProperty(const std::string &name, const std::string &frame, Element* element_) : Property(name), framePtr(element_->getFrame(frame.substr(6, frame.length()-7))), element(element_) {
  setValue(frame);
}

void LocalFrameOfReferenceProperty::setFrame(const std::string &str) {
  setValue(str);
  framePtr=element->getFrame(value.substr(6, value.length()-7));
}

std::string LocalFrameOfReferenceProperty::getFrame() const {
  return framePtr?("Frame[" + framePtr->getName() + "]"):value;
}

TiXmlElement* LocalFrameOfReferenceProperty::initializeUsingXML(TiXmlElement *parent) {
  setFrame(parent->Attribute("ref"));
  return parent;
}

TiXmlElement* LocalFrameOfReferenceProperty::writeXMLFile(TiXmlNode *parent) {
  static_cast<TiXmlElement*>(parent)->SetAttribute("ref", getFrame());
  return 0;
}

void LocalFrameOfReferenceProperty::fromWidget(QWidget *widget) {
//  setFrame(static_cast<LocalFrameOfReferenceWidget*>(widget)->getFrame().toStdString());
}

void LocalFrameOfReferenceProperty::toWidget(QWidget *widget) {
//  static_cast<LocalFrameOfReferenceWidget*>(widget)->setFrame(QString::fromStdString(frame),framePtr);
//  static_cast<LocalFrameOfReferenceWidget*>(widget)->updateWidget();
}

ParentFrameOfReferenceProperty::ParentFrameOfReferenceProperty(const std::string &name, const std::string &frame, Element* element_) : Property(name), framePtr(element_->getParent()->getFrame(frame.substr(9, frame.length()-10))), element(element_) {
  setValue(frame);
}

void ParentFrameOfReferenceProperty::initialize() {
  framePtr=element->getParent()->getFrame((value.substr(0,2)=="..")?value.substr(9, value.length()-10):value.substr(6, value.length()-7));
}

void ParentFrameOfReferenceProperty::setFrame(const std::string &str) {
  setValue(str);
  framePtr=element->getParent()->getFrame(value.substr(9, value.length()-10));
}

std::string ParentFrameOfReferenceProperty::getFrame() const {
  return framePtr?("../Frame[" + framePtr->getName() + "]"):value;
}

TiXmlElement* ParentFrameOfReferenceProperty::initializeUsingXML(TiXmlElement *parent) {
  setFrame(parent->Attribute("ref"));
  return parent;
}

TiXmlElement* ParentFrameOfReferenceProperty::writeXMLFile(TiXmlNode *parent) {
  static_cast<TiXmlElement*>(parent)->SetAttribute("ref", getFrame());
  return 0;
}

void ParentFrameOfReferenceProperty::fromWidget(QWidget *widget) {
//  setFrame(static_cast<ParentFrameOfReferenceWidget*>(widget)->getFrame().toStdString());
}

void ParentFrameOfReferenceProperty::toWidget(QWidget *widget) {
//  static_cast<ParentFrameOfReferenceWidget*>(widget)->setFrame(QString::fromStdString(value),framePtr);
//  static_cast<ParentFrameOfReferenceWidget*>(widget)->updateWidget();
}

FrameOfReferenceProperty::FrameOfReferenceProperty(const std::string &name, const std::string &frame, Element* element_) : Property(name), framePtr(element_->getByPath<Frame>(frame)), element(element_) {
  setValue(frame);
}

void FrameOfReferenceProperty::initialize() {
  framePtr=element->getByPath<Frame>(value);
}

void FrameOfReferenceProperty::setFrame(const std::string &str) {
  setValue(str);
  framePtr=element->getByPath<Frame>(value);
}

std::string FrameOfReferenceProperty::getFrame() const {
  return framePtr?framePtr->getXMLPath(element,true):value;
}

TiXmlElement* FrameOfReferenceProperty::initializeUsingXML(TiXmlElement *parent) {
  setFrame(parent->Attribute("ref"));
  return parent;
}

TiXmlElement* FrameOfReferenceProperty::writeXMLFile(TiXmlNode *parent) {
  static_cast<TiXmlElement*>(parent)->SetAttribute("ref", getFrame());
  return 0;
}

void FrameOfReferenceProperty::fromWidget(QWidget *widget) {
//  setFrame(static_cast<FrameOfReferenceWidget*>(widget)->getFrame().toStdString());
}

void FrameOfReferenceProperty::toWidget(QWidget *widget) {
//  static_cast<FrameOfReferenceWidget*>(widget)->setFrame(QString::fromStdString(value),framePtr);
//  static_cast<FrameOfReferenceWidget*>(widget)->updateWidget();
}

ContourOfReferenceProperty::ContourOfReferenceProperty(const std::string &contour_, Element* element_, const std::string &xmlName_) : contour(contour_), contourPtr(element_->getByPath<Contour>(contour)), element(element_), xmlName(xmlName_) {
}

void ContourOfReferenceProperty::initialize() {
  contourPtr=element->getByPath<Contour>(contour);
}

void ContourOfReferenceProperty::setContour(const std::string &str) {
  contour = str;
  contourPtr=element->getByPath<Contour>(contour);
}

std::string ContourOfReferenceProperty::getContour() const {
  return contourPtr?contourPtr->getXMLPath(element,true):contour;
}

TiXmlElement* ContourOfReferenceProperty::initializeUsingXML(TiXmlElement *parent) {
  TiXmlElement *e = parent->FirstChildElement(xmlName);
  if(e) contour=e->Attribute("ref");
  return e;
}

TiXmlElement* ContourOfReferenceProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = new TiXmlElement(xmlName);
  ele->SetAttribute("ref", getContour());
  parent->LinkEndChild(ele);
  return 0;
}

void ContourOfReferenceProperty::fromWidget(QWidget *widget) {
  setContour(static_cast<ContourOfReferenceWidget*>(widget)->getContour().toStdString());
}

void ContourOfReferenceProperty::toWidget(QWidget *widget) {
  static_cast<ContourOfReferenceWidget*>(widget)->setContour(QString::fromStdString(contour),contourPtr);
  static_cast<ContourOfReferenceWidget*>(widget)->updateWidget();
}

RigidBodyOfReferenceProperty::RigidBodyOfReferenceProperty(const std::string &body_, Element *element_, const std::string &xmlName_) : body(body_), bodyPtr(element_->getByPath<RigidBody>(body)), element(element_), xmlName(xmlName_) {
}

void RigidBodyOfReferenceProperty::initialize() {
  bodyPtr=element->getByPath<RigidBody>(body);
}

void RigidBodyOfReferenceProperty::setBody(const std::string &str) {
  body = str;
  bodyPtr=element->getByPath<RigidBody>(body);
}

std::string RigidBodyOfReferenceProperty::getBody() const {
  return bodyPtr?bodyPtr->getXMLPath(element,true):body;
}

TiXmlElement* RigidBodyOfReferenceProperty::initializeUsingXML(TiXmlElement *parent) {
  TiXmlElement *e = (xmlName=="")?parent:parent->FirstChildElement(xmlName);
  if(e) body=e->Attribute("ref");
  return e;
}

TiXmlElement* RigidBodyOfReferenceProperty::writeXMLFile(TiXmlNode *parent) {
  if(xmlName=="") 
    static_cast<TiXmlElement*>(parent)->SetAttribute("ref", getBody());
  else {
    TiXmlElement *ele = new TiXmlElement(xmlName);
    ele->SetAttribute("ref", getBody());
    parent->LinkEndChild(ele);
  }
  return 0;
}

void RigidBodyOfReferenceProperty::fromWidget(QWidget *widget) {
  setBody(static_cast<RigidBodyOfReferenceWidget*>(widget)->getBody().toStdString());
}

void RigidBodyOfReferenceProperty::toWidget(QWidget *widget) {
  static_cast<RigidBodyOfReferenceWidget*>(widget)->setBody(QString::fromStdString(body),bodyPtr);
  static_cast<RigidBodyOfReferenceWidget*>(widget)->updateWidget();
}

ObjectOfReferenceProperty::ObjectOfReferenceProperty(const std::string &object_, Element *element_, const std::string &xmlName_) : object(object_), objectPtr(element_->getByPath<Object>(object)), element(element_), xmlName(xmlName_) {
}

void ObjectOfReferenceProperty::initialize() {
  objectPtr=element->getByPath<Object>(object);
}

void ObjectOfReferenceProperty::setObject(const std::string &str) {
  object = str;
  objectPtr=element->getByPath<Object>(object);
}

std::string ObjectOfReferenceProperty::getObject() const {
  return objectPtr?objectPtr->getXMLPath(element,true):object;
}

TiXmlElement* ObjectOfReferenceProperty::initializeUsingXML(TiXmlElement *parent) {
  TiXmlElement *e = parent->FirstChildElement(xmlName);
  if(e) object=e->Attribute("ref");
  return e;
}

TiXmlElement* ObjectOfReferenceProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = new TiXmlElement(xmlName);
  ele->SetAttribute("ref", getObject());
  parent->LinkEndChild(ele);
  return 0;
}

void ObjectOfReferenceProperty::fromWidget(QWidget *widget) {
  setObject(static_cast<ObjectOfReferenceWidget*>(widget)->getObject().toStdString());
}

void ObjectOfReferenceProperty::toWidget(QWidget *widget) {
  static_cast<ObjectOfReferenceWidget*>(widget)->setObject(QString::fromStdString(object),objectPtr);
  static_cast<ObjectOfReferenceWidget*>(widget)->updateWidget();
}

LinkOfReferenceProperty::LinkOfReferenceProperty(const std::string &link_, Element *element_, const std::string &xmlName_) : link(link_), linkPtr(element_->getByPath<Link>(link)), element(element_), xmlName(xmlName_) {
}

void LinkOfReferenceProperty::initialize() {
  linkPtr=element->getByPath<Link>(link);
}

void LinkOfReferenceProperty::setLink(const std::string &str) {
  link = str;
  linkPtr=element->getByPath<Link>(link);
}

std::string LinkOfReferenceProperty::getLink() const {
  return linkPtr?linkPtr->getXMLPath(element,true):link;
}

TiXmlElement* LinkOfReferenceProperty::initializeUsingXML(TiXmlElement *parent) {
  TiXmlElement *e = parent->FirstChildElement(xmlName);
  if(e) link=e->Attribute("ref");
  return e;
}

TiXmlElement* LinkOfReferenceProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = new TiXmlElement(xmlName);
  ele->SetAttribute("ref", getLink());
  parent->LinkEndChild(ele);
  return 0;
}

void LinkOfReferenceProperty::fromWidget(QWidget *widget) {
  setLink(static_cast<LinkOfReferenceWidget*>(widget)->getLink().toStdString());
}

void LinkOfReferenceProperty::toWidget(QWidget *widget) {
  static_cast<LinkOfReferenceWidget*>(widget)->setLink(QString::fromStdString(link),linkPtr);
  static_cast<LinkOfReferenceWidget*>(widget)->updateWidget();
}

SignalOfReferenceProperty::SignalOfReferenceProperty(const std::string &signal_, Element *element_, const std::string &xmlName_) : signal(signal_), signalPtr(element_->getByPath<Signal>(signal)), element(element_), xmlName(xmlName_) {
}

void SignalOfReferenceProperty::initialize() {
  signalPtr=element->getByPath<Signal>(signal);
}

void SignalOfReferenceProperty::setSignal(const std::string &str) {
  signal = str;
  signalPtr=element->getByPath<Signal>(signal);
}

std::string SignalOfReferenceProperty::getSignal() const {
  return signalPtr?signalPtr->getXMLPath(element,true):signal;
}

TiXmlElement* SignalOfReferenceProperty::initializeUsingXML(TiXmlElement *parent) {
  TiXmlElement *e = parent->FirstChildElement(xmlName);
  if(e) signal=e->Attribute("ref");
  return e;
}

TiXmlElement* SignalOfReferenceProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = new TiXmlElement(xmlName);
  ele->SetAttribute("ref", getSignal());
  parent->LinkEndChild(ele);
  return 0;
}

void SignalOfReferenceProperty::fromWidget(QWidget *widget) {
  setSignal(static_cast<SignalOfReferenceWidget*>(widget)->getSignal().toStdString());
}

void SignalOfReferenceProperty::toWidget(QWidget *widget) {
  static_cast<SignalOfReferenceWidget*>(widget)->setSignal(QString::fromStdString(signal),signalPtr);
  static_cast<SignalOfReferenceWidget*>(widget)->updateWidget();
}

TiXmlElement* FileProperty::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e=element->FirstChildElement(xmlName);
  if(e) {
    TiXmlText *text = e->FirstChildText();
    if(text) {
      file = text->Value();
      file = file.substr(1,file.length()-2);
      QFileInfo fileInfo(mbsDir.absoluteFilePath(QString::fromStdString(file)));
      file = fileInfo.canonicalFilePath().toStdString();
      return e;
    }
  }
  return 0;
}

TiXmlElement* FileProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele0 = new TiXmlElement(xmlName);
  string filePath = string("\"")+(absolutePath?mbsDir.absoluteFilePath(QString::fromStdString(file)).toStdString():mbsDir.relativeFilePath(QString::fromStdString(file)).toStdString())+"\"";
  TiXmlText *text = new TiXmlText(filePath);
  ele0->LinkEndChild(text);
  parent->LinkEndChild(ele0);

  return 0;
}

void FileProperty::fromWidget(QWidget *widget) {
  file = static_cast<FileWidget*>(widget)->getFile().toStdString();
}

void FileProperty::toWidget(QWidget *widget) {
  static_cast<FileWidget*>(widget)->blockSignals(true);
  static_cast<FileWidget*>(widget)->setFile(QString::fromStdString(file));
  static_cast<FileWidget*>(widget)->blockSignals(false);
}

TiXmlElement* IntegerProperty::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e=element->FirstChildElement(xmlName);
  if(e) {
    TiXmlText *text = e->FirstChildText();
    if(text) {
      value = atoi(text->Value());
      return e;
    }
  }
  return 0;
}

TiXmlElement* IntegerProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele0 = new TiXmlElement(xmlName);
  TiXmlText *text= new TiXmlText(toStr(value));
  ele0->LinkEndChild(text);
  parent->LinkEndChild(ele0);

  return 0;
}

void IntegerProperty::fromWidget(QWidget *widget) {
  value = static_cast<IntegerWidget*>(widget)->getInt();
}

void IntegerProperty::toWidget(QWidget *widget) {
  static_cast<IntegerWidget*>(widget)->setInt(value);
}

TiXmlElement* TextProperty::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e=(xmlName=="")?element:element->FirstChildElement(xmlName);
  if(e) {
    TiXmlText *text_ = e->FirstChildText();
    if(text_) {
      setValue(text_->Value());
      if(quote)
        setValue(getValue().substr(1,getValue().length()-2));
      return e;
    }
  }
  return 0;
}

TiXmlElement* TextProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele0;
  if(xmlName!="") {
    ele0 = new TiXmlElement(xmlName);
    parent->LinkEndChild(ele0);
  }
  else
    ele0 = (TiXmlElement*)parent;
  TiXmlText *text_ = new TiXmlText(quote?("\""+getValue()+"\""):getValue());
  ele0->LinkEndChild(text_);
  return 0;
}

void TextProperty::fromWidget(QWidget *widget) {
  setValue(static_cast<BasicTextWidget*>(widget)->getText().toStdString());
}

void TextProperty::toWidget(QWidget *widget) {
  static_cast<BasicTextWidget*>(widget)->setText(QString::fromStdString(getValue()));
}

ConnectFramesProperty::ConnectFramesProperty(int n, Element *element, const std::string &xmlName_) : xmlName(xmlName_)  {

  for(int i=0; i<n; i++) {
    string xmlName = MBSIMNS"ref";
    if(n>1)
      xmlName += toStr(i+1);
    frame.push_back(FrameOfReferenceProperty("","",element));//,xmlName));
  }
}

void ConnectFramesProperty::initialize() {
  for(unsigned int i=0; i<frame.size(); i++)
    frame[i].initialize();
}

TiXmlElement* ConnectFramesProperty::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e = element->FirstChildElement(xmlName);
  if(e) {
    for(unsigned int i=0; i<frame.size(); i++) {
      string xmlName = "ref";
      if(frame.size()>1)
        xmlName += toStr(int(i+1));
      if(!e->Attribute(xmlName))
        return 0;
      frame[i].setFrame(e->Attribute(xmlName.c_str()));
    }
  }
  return e;
}

TiXmlElement* ConnectFramesProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = new TiXmlElement(xmlName);
  for(unsigned int i=0; i<frame.size(); i++) {
    string xmlName = "ref";
    if(frame.size()>1)
      xmlName += toStr(int(i+1));
    ele->SetAttribute(xmlName, frame[i].getFrame()); 
  }
  parent->LinkEndChild(ele);
  return ele;
}

void ConnectFramesProperty::fromWidget(QWidget *widget) {
  for(unsigned int i=0; i<frame.size(); i++)
    frame[i].fromWidget(static_cast<ConnectFramesWidget*>(widget)->widget[i]);
}

void ConnectFramesProperty::toWidget(QWidget *widget) {
  for(unsigned int i=0; i<frame.size(); i++)
    frame[i].toWidget(static_cast<ConnectFramesWidget*>(widget)->widget[i]);
  static_cast<ConnectFramesWidget*>(widget)->update();
}

ConnectContoursProperty::ConnectContoursProperty(int n, Element *element, const std::string &xmlName_) : xmlName(xmlName_) {

  for(int i=0; i<n; i++) {
    string xmlName = MBSIMNS"ref";
    if(n>1)
      xmlName += toStr(i+1);
    contour.push_back(ContourOfReferenceProperty("",element,xmlName));
  }
}

void ConnectContoursProperty::initialize() {
  for(unsigned int i=0; i<contour.size(); i++)
    contour[i].initialize();
}

TiXmlElement* ConnectContoursProperty::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e = element->FirstChildElement(xmlName);
  if(e) {
    for(unsigned int i=0; i<contour.size(); i++) {
      string xmlName = "ref";
      if(contour.size()>1)
        xmlName += toStr(int(i+1));
      if(!e->Attribute(xmlName))
        return 0;
      contour[i].setContour(e->Attribute(xmlName.c_str()));
    }
  }
  return e;
}

TiXmlElement* ConnectContoursProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = new TiXmlElement(xmlName);
  for(unsigned int i=0; i<contour.size(); i++) {
    string xmlName = "ref";
    if(contour.size()>1)
      xmlName += toStr(int(i+1));
    ele->SetAttribute(xmlName, contour[i].getContour()); 
  }
  parent->LinkEndChild(ele);
  return ele;
}

void ConnectContoursProperty::fromWidget(QWidget *widget) {
  for(unsigned int i=0; i<contour.size(); i++)
    contour[i].fromWidget(static_cast<ConnectContoursWidget*>(widget)->widget[i]);
}

void ConnectContoursProperty::toWidget(QWidget *widget) {
  for(unsigned int i=0; i<contour.size(); i++)
    contour[i].toWidget(static_cast<ConnectContoursWidget*>(widget)->widget[i]);
  static_cast<ConnectContoursWidget*>(widget)->update();
}

TiXmlElement* SolverChoiceProperty::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e=element->FirstChildElement(xmlName);
  if (e) {
    if (e->FirstChildElement(MBSIMNS"FixedPointSingle"))
      choice = "FixedPointSingle";
    else if (e->FirstChildElement(MBSIMNS"GaussSeidel"))
      choice = "GaussSeidel";
    else if (e->FirstChildElement(MBSIMNS"LinearEquations"))
      choice = "LinearEquations";
    else if (e->FirstChildElement(MBSIMNS"RootFinding"))
      choice = "RootFinding";
    return e;
  }
  return 0;
}

TiXmlElement* SolverChoiceProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *e=new TiXmlElement(xmlName);
  parent->LinkEndChild(e);
  if(choice=="FixedPointTotal")
    e->LinkEndChild(new TiXmlElement( MBSIMNS"FixedPointTotal" ));
  else if(choice=="FixedPointSingle")
    e->LinkEndChild(new TiXmlElement( MBSIMNS"FixedPointSingle" ));
  else if(choice=="GaussSeidel")
    e->LinkEndChild(new TiXmlElement( MBSIMNS"GaussSeidel" ));
  else if(choice=="LinearEquations")
    e->LinkEndChild(new TiXmlElement( MBSIMNS"LinearEquations" ));
  else if(choice=="RootFinding")
    e->LinkEndChild(new TiXmlElement( MBSIMNS"RootFinding" ));
  return e;
}

void SolverChoiceProperty::fromWidget(QWidget *widget) {
  choice = static_cast<SolverChoiceWidget*>(widget)->getSolver().toStdString();
}

void SolverChoiceProperty::toWidget(QWidget *widget) {
  static_cast<SolverChoiceWidget*>(widget)->setSolver(QString::fromStdString(choice));
}

SolverTolerancesProperty::SolverTolerancesProperty() : projection(0,false), g(0,false), gd(0,false), gdd(0,false), la(0,false), La(0,false) {

  vector<PhysicalVariableProperty> input;
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1e-15"), "-", MBSIMNS"projection"));
  projection.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1e-8"), "-", MBSIMNS"g"));
  g.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1e-10"), "-", MBSIMNS"gd"));
  gd.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1e-12"), "-", MBSIMNS"gdd"));
  gdd.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1e-12"), "-", MBSIMNS"la"));
  la.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1e-10"), "-", MBSIMNS"La"));
  La.setProperty(new ExtPhysicalVarProperty(input));
}

TiXmlElement* SolverTolerancesProperty::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e=element->FirstChildElement(MBSIMNS"tolerances");
  if(e) {
    projection.initializeUsingXML(e);
    g.initializeUsingXML(e);
    gd.initializeUsingXML(e);
    gdd.initializeUsingXML(e);
    la.initializeUsingXML(e);
    La.initializeUsingXML(e);
  }
  return e;
}

TiXmlElement* SolverTolerancesProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *e=new TiXmlElement(MBSIMNS"tolerances");
  parent->LinkEndChild(e);
  projection.writeXMLFile(e);
  g.writeXMLFile(e);
  gd.writeXMLFile(e);
  gdd.writeXMLFile(e);
  la.writeXMLFile(e);
  La.writeXMLFile(e);
  return e;
}

void SolverTolerancesProperty::fromWidget(QWidget *widget) {
  projection.fromWidget(static_cast<SolverTolerancesWidget*>(widget)->projection);
  g.fromWidget(static_cast<SolverTolerancesWidget*>(widget)->g);
  gd.fromWidget(static_cast<SolverTolerancesWidget*>(widget)->gd);
  gdd.fromWidget(static_cast<SolverTolerancesWidget*>(widget)->gdd);
  la.fromWidget(static_cast<SolverTolerancesWidget*>(widget)->la);
  La.fromWidget(static_cast<SolverTolerancesWidget*>(widget)->La);
}

void SolverTolerancesProperty::toWidget(QWidget *widget) {
  projection.toWidget(static_cast<SolverTolerancesWidget*>(widget)->projection);
  g.toWidget(static_cast<SolverTolerancesWidget*>(widget)->g);
  gd.toWidget(static_cast<SolverTolerancesWidget*>(widget)->gd);
  gdd.toWidget(static_cast<SolverTolerancesWidget*>(widget)->gdd);
  la.toWidget(static_cast<SolverTolerancesWidget*>(widget)->la);
  La.toWidget(static_cast<SolverTolerancesWidget*>(widget)->La);
}

SolverParametersProperty::SolverParametersProperty() : constraintSolver(0,false), impactSolver(0,false), numberOfMaximalIterations(0,false), tolerances(0,false) {
  constraintSolver.setProperty(new SolverChoiceProperty(MBSIMNS"constraintSolver"));
  impactSolver.setProperty(new SolverChoiceProperty(MBSIMNS"impactSolver"));

  vector<PhysicalVariableProperty> input;
  input.push_back(PhysicalVariableProperty(new ScalarProperty("10000"), "", MBSIMNS"numberOfMaximalIterations"));
  numberOfMaximalIterations.setProperty(new ExtPhysicalVarProperty(input));

  tolerances.setProperty(new SolverTolerancesProperty);
}

TiXmlElement* SolverParametersProperty::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e=element->FirstChildElement(MBSIMNS"solverParameters");
  if(e) {
    constraintSolver.initializeUsingXML(e);
    impactSolver.initializeUsingXML(e);
    numberOfMaximalIterations.initializeUsingXML(e);
    tolerances.initializeUsingXML(e);
  }
  return e;
}

TiXmlElement* SolverParametersProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *e=new TiXmlElement(MBSIMNS"solverParameters");
  parent->LinkEndChild(e);
  constraintSolver.writeXMLFile(e);
  impactSolver.writeXMLFile(e);
  numberOfMaximalIterations.writeXMLFile(e);
  tolerances.writeXMLFile(e);
  return e;
}

void SolverParametersProperty::fromWidget(QWidget *widget) {
  constraintSolver.fromWidget(static_cast<SolverParametersWidget*>(widget)->constraintSolver);
  impactSolver.fromWidget(static_cast<SolverParametersWidget*>(widget)->impactSolver);
  numberOfMaximalIterations.fromWidget(static_cast<SolverParametersWidget*>(widget)->numberOfMaximalIterations);
  tolerances.fromWidget(static_cast<SolverParametersWidget*>(widget)->tolerances);
}

void SolverParametersProperty::toWidget(QWidget *widget) {
  constraintSolver.toWidget(static_cast<SolverParametersWidget*>(widget)->constraintSolver);
  impactSolver.toWidget(static_cast<SolverParametersWidget*>(widget)->impactSolver);
  numberOfMaximalIterations.toWidget(static_cast<SolverParametersWidget*>(widget)->numberOfMaximalIterations);
  tolerances.toWidget(static_cast<SolverParametersWidget*>(widget)->tolerances);
}

EmbedProperty::EmbedProperty(Element *element) : href(0,false), count(0,false), counterName(0,false), parameterList(0,false) {
  href.setProperty(new FileProperty(""));
  static_cast<FileProperty*>(href.getProperty())->setFile(element->getName()+".xml");
  count.setProperty(new IntegerProperty("",1,""));
  counterName.setProperty(new TextProperty("","n",""));
  parameterList.setProperty(new FileProperty(""));
}

TiXmlElement* EmbedProperty::initializeUsingXML(TiXmlElement *parent) {
//  if(parent->Attribute("href")) {
//    href.setActive(true);
//    string file = parent->Attribute("href");
//    static_cast<FileProperty*>(href.getProperty())->setFile(file);
//  }
//  if(parent->Attribute("count")) {
//    count.setActive(true);
//    static_cast<IntegerProperty*>(count.getProperty())->setInt(atoi(parent->Attribute("count")));
//  }
//  if(parent->Attribute("counterName")) {
//    counterName.setActive(true);
//    static_cast<TextProperty*>(counterName.getProperty())->setValue(parent->Attribute("counterName"));
//  }
//  TiXmlElement *ele = parent->FirstChildElement(PVNS+string("localParameter"));
//  if(ele) {
//    parameterList.setActive(true);
//    string file = ele->Attribute("href");
//    static_cast<FileProperty*>(parameterList.getProperty())->setFile(file);
//  }
}

TiXmlElement* EmbedProperty::writeXMLFile(TiXmlNode *parent) {
//  TiXmlElement *ele0=new TiXmlElement(PVNS+string("embed"));
//  if(href.isActive())
//    ele0->SetAttribute("href", static_cast<FileProperty*>(href.getProperty())->getFile());
//  if(count.isActive())
//    ele0->SetAttribute("count", static_cast<IntegerProperty*>(count.getProperty())->getValue());
//  if(counterName.isActive())
//    ele0->SetAttribute("counterName", static_cast<TextProperty*>(counterName.getProperty())->getValue());
//  if(parameterList.isActive()) {
//    TiXmlElement *ele1=new TiXmlElement(PVNS+string("localParameter"));
//    string filePath = absolutePath?mbsDir.absoluteFilePath(QString::fromStdString(static_cast<FileProperty*>(parameterList.getProperty())->getFile())).toStdString():mbsDir.relativeFilePath(QString::fromStdString(static_cast<FileProperty*>(parameterList.getProperty())->getFile())).toStdString();
//    ele1->SetAttribute("href", filePath);
//    ele0->LinkEndChild(ele1);
//  }
//  parent->LinkEndChild(ele0);
//  return ele0;
}

void EmbedProperty::fromWidget(QWidget *widget) {
  href.fromWidget(static_cast<EmbedWidget*>(widget)->href);
  count.fromWidget(static_cast<EmbedWidget*>(widget)->count);
  counterName.fromWidget(static_cast<EmbedWidget*>(widget)->counterName);
  parameterList.fromWidget(static_cast<EmbedWidget*>(widget)->parameterList);
}

void EmbedProperty::toWidget(QWidget *widget) {
  href.toWidget(static_cast<EmbedWidget*>(widget)->href);
  count.toWidget(static_cast<EmbedWidget*>(widget)->count);
  counterName.toWidget(static_cast<EmbedWidget*>(widget)->counterName);
  parameterList.toWidget(static_cast<EmbedWidget*>(widget)->parameterList);
}

SignalReferenceProperty::SignalReferenceProperty(Element* element) : refSignal("",element) {
  vector<PhysicalVariableProperty> input;
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "", MBSIMCONTROLNS"factor"));
  factor.setProperty(new ExtPhysicalVarProperty(input));
} 

TiXmlElement* SignalReferenceProperty::initializeUsingXML(TiXmlElement *ele) {
  factor.initializeUsingXML(ele);
  refSignal.setSignal(ele->Attribute("ref"));
  return ele;
}

TiXmlElement* SignalReferenceProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = (TiXmlElement*)parent;
  factor.writeXMLFile(ele);
  ele->SetAttribute("ref", refSignal.getSignal());
  return ele;
}

void SignalReferenceProperty::fromWidget(QWidget *widget) {
  factor.fromWidget(static_cast<SignalReferenceWidget*>(widget)->factor);
  refSignal.fromWidget(static_cast<SignalReferenceWidget*>(widget)->refSignal);
}

void SignalReferenceProperty::toWidget(QWidget *widget) {
  factor.toWidget(static_cast<SignalReferenceWidget*>(widget)->factor);
  refSignal.toWidget(static_cast<SignalReferenceWidget*>(widget)->refSignal);
}

ColorProperty::ColorProperty(const std::string &xmlName_) : xmlName(xmlName_) {
  vector<PhysicalVariableProperty> input;
  vector<string> vec(3);
  vec[0] = "0.666667"; vec[1] = "1"; vec[2] = "1";
  input.push_back(PhysicalVariableProperty(new VecProperty(vec), "", ""));
  color.setProperty(new ExtPhysicalVarProperty(input));
}

TiXmlElement* ColorProperty::initializeUsingXML(TiXmlElement *parent) {
  TiXmlElement *e = parent->FirstChildElement(xmlName);
  color.initializeUsingXML(e);
  return e;
}

TiXmlElement* ColorProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = new TiXmlElement(xmlName);
  color.writeXMLFile(ele);
  parent->LinkEndChild(ele);
  return 0;
}

void ColorProperty::fromWidget(QWidget *widget) {
  color.fromWidget(static_cast<ColorWidget*>(widget)->color);
}

void ColorProperty::toWidget(QWidget *widget) {
  color.toWidget(static_cast<ColorWidget*>(widget)->color);
  static_cast<ColorWidget*>(widget)->updateWidget();
}

