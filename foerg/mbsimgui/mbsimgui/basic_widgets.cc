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
#include "basic_widgets.h"
#include "rigidbody.h"
#include "frame.h"
#include "contour.h"
#include "group.h"
#include "parameter.h"
#include "dialogs.h"
#include <string>
#include <QtGui>

using namespace std;

LocalFrameOfReferenceWidget::LocalFrameOfReferenceWidget(const string &xmlName_, Element *element_, Frame* omitFrame_) : element(element_), selectedFrame(0), omitFrame(omitFrame_), xmlName(xmlName_) {
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setMargin(0);
  setLayout(layout);

  frame = new QComboBox;
  layout->addWidget(frame);
  selectedFrame = element->getFrame(0);
  connect(frame,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(setFrame(const QString&)));
}

void LocalFrameOfReferenceWidget::update() {
  frame->blockSignals(true);
  frame->clear();
  int oldindex = 0;
  for(int i=0, k=0; i<element->getContainerFrame()->childCount(); i++) {
    if(omitFrame!=element->getFrame(i)) {
      frame->addItem("Frame["+element->getFrame(i)->getName()+"]");
      if(element->getFrame(i) == selectedFrame)
        oldindex = k;
      k++;
    }
  }
  frame->setCurrentIndex(oldindex);
  frame->blockSignals(false);
}

void LocalFrameOfReferenceWidget::setFrame(Frame* frame_) {
  selectedFrame = frame_; 
}

void LocalFrameOfReferenceWidget::setFrame(const QString &str) {
  string str_ = str.toStdString();
  selectedFrame = element->getFrame(str_.substr(6, str_.length()-7));
}

bool LocalFrameOfReferenceWidget::initializeUsingXML(TiXmlElement *parent) {
  TiXmlElement *e = parent->FirstChildElement(xmlName);
  if(e) {
    string refF="";
    refF=e->Attribute("ref");
    refF=refF.substr(6, refF.length()-7);
    setFrame(refF==""?element->getFrame(0):element->getFrame(refF));
  }
}

TiXmlElement* LocalFrameOfReferenceWidget::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = new TiXmlElement(xmlName);
  QString str = QString("Frame[") + getFrame()->getName() + "]";
  ele->SetAttribute("ref", str.toStdString());
  parent->LinkEndChild(ele);
  return 0;
}

ParentFrameOfReferenceWidget::ParentFrameOfReferenceWidget(const string &xmlName_, Element *element_, Frame* omitFrame_) : element(element_), selectedFrame(0), omitFrame(omitFrame_), xmlName(xmlName_) {
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setMargin(0);
  setLayout(layout);

  frame = new QComboBox;
  layout->addWidget(frame);
  selectedFrame = element->getParentElement()->getFrame(0);
  connect(frame,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(setFrame(const QString&)));
}

void ParentFrameOfReferenceWidget::update() {
  frame->blockSignals(true);
  frame->clear();
  int oldindex = 0;
  for(int i=0, k=0; i<element->getParentElement()->getContainerFrame()->childCount(); i++) {
    if(omitFrame!=element->getParentElement()->getFrame(i)) {
      frame->addItem("../Frame["+element->getParentElement()->getFrame(i)->getName()+"]");
      if(element->getParentElement()->getFrame(i) == selectedFrame)
        oldindex = k;
      k++;
    }
  }
  frame->setCurrentIndex(oldindex);
  frame->blockSignals(false);
}

void ParentFrameOfReferenceWidget::initialize() {
  if(saved_frameOfReference!="") {
    string refF = saved_frameOfReference.toStdString();
    refF=refF.substr(9, refF.length()-10);
    setFrame(element->getParentElement()->getFrame(refF));
  }
}

void ParentFrameOfReferenceWidget::setFrame(Frame* frame_) {
  selectedFrame = frame_; 
}

void ParentFrameOfReferenceWidget::setFrame(const QString &str) {
  string str_ = str.toStdString();
  selectedFrame = element->getParentElement()->getFrame(str_.substr(9, str_.length()-10));
}

bool ParentFrameOfReferenceWidget::initializeUsingXML(TiXmlElement *parent) {
  TiXmlElement *e = parent->FirstChildElement(xmlName);
  if(e) {
    saved_frameOfReference = e->Attribute("ref");
    //setFrame(refF==""?element->getParentElement()->getFrame(0):element->getParentElement()->getFrame(refF));
    return true;
  }
}

TiXmlElement* ParentFrameOfReferenceWidget::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = new TiXmlElement(xmlName);
  QString str = QString("../Frame[") + getFrame()->getName() + "]";
  ele->SetAttribute("ref", str.toStdString());
  parent->LinkEndChild(ele);
  return 0;
}

FrameOfReferenceWidget::FrameOfReferenceWidget(const string &xmlName_, Element *element_, Frame* selectedFrame_) : element(element_), selectedFrame(selectedFrame_), xmlName(xmlName_) {
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(0);
  setLayout(layout);

  frame = new QLineEdit;
  frame->setReadOnly(true);
  if(selectedFrame)
    frame->setText(selectedFrame->getXMLPath());
  frameBrowser = new FrameBrowser(element->treeWidget(),selectedFrame,this);
  connect(frameBrowser,SIGNAL(accepted()),this,SLOT(setFrame()));
  layout->addWidget(frame);
  QPushButton *button = new QPushButton(tr("Browse"));
  connect(button,SIGNAL(clicked(bool)),frameBrowser,SLOT(show()));
  layout->addWidget(button);
}

void FrameOfReferenceWidget::initialize() {
  if(saved_frameOfReference!="")
    setFrame(element->getByPath<Frame>(saved_frameOfReference));
}

void FrameOfReferenceWidget::update() {
  frameBrowser->update(selectedFrame);
  if(selectedFrame) {
    setFrame();
  }
}

void FrameOfReferenceWidget::setFrame() { 
  if(frameBrowser->getFrameList()->currentItem())
    selectedFrame = (Frame*)static_cast<ElementItem*>(frameBrowser->getFrameList()->currentItem())->getElement();
  else
    selectedFrame = ((Group*)element->getParentElement())->getFrame(0);
  frame->setText(selectedFrame->getXMLPath());
}

void FrameOfReferenceWidget::setFrame(Frame* frame_) {
  selectedFrame = frame_; 
  frame->setText(selectedFrame->getXMLPath());
}

bool FrameOfReferenceWidget::initializeUsingXML(TiXmlElement *parent) {
  TiXmlElement *e = parent->FirstChildElement(xmlName);
  if(e)
    saved_frameOfReference=e->Attribute("ref");
}

TiXmlElement* FrameOfReferenceWidget::writeXMLFile(TiXmlNode *parent) {
  if(getFrame()) {
    TiXmlElement *ele = new TiXmlElement(xmlName);
    ele->SetAttribute("ref", getFrame()->getXMLPath(element,true).toStdString());
    parent->LinkEndChild(ele);
  }
  return 0;
}

ContourOfReferenceWidget::ContourOfReferenceWidget(const string &xmlName_, Element *element_, Contour* selectedContour_) : element(element_), selectedContour(selectedContour_), xmlName(xmlName_) {
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(0);
  setLayout(layout);

  contour = new QLineEdit;
  contour->setReadOnly(true);
  if(selectedContour)
    contour->setText(selectedContour->getXMLPath());
  contourBrowser = new ContourBrowser(element->treeWidget(),selectedContour,this);
  connect(contourBrowser,SIGNAL(accepted()),this,SLOT(setContour()));
  layout->addWidget(contour);
  QPushButton *button = new QPushButton(tr("Browse"));
  connect(button,SIGNAL(clicked(bool)),contourBrowser,SLOT(show()));
  layout->addWidget(button);
}

void ContourOfReferenceWidget::initialize() {
  if(saved_contourOfReference!="")
    setContour(element->getByPath<Contour>(saved_contourOfReference));
}

void ContourOfReferenceWidget::update() {
  contourBrowser->update(selectedContour);
  if(selectedContour) {
    setContour();
  }
}

void ContourOfReferenceWidget::setContour() { 
  if(contourBrowser->getContourList()->currentItem())
    selectedContour = (Contour*)static_cast<ElementItem*>(contourBrowser->getContourList()->currentItem())->getElement();
  else
    selectedContour = ((Group*)element->getParentElement())->getContour(0);
  contour->setText(selectedContour->getXMLPath());
}

void ContourOfReferenceWidget::setContour(Contour* contour_) {
  selectedContour = contour_; 
  contour->setText(selectedContour->getXMLPath());
}

bool ContourOfReferenceWidget::initializeUsingXML(TiXmlElement *parent) {
  TiXmlElement *e = parent->FirstChildElement(xmlName);
  if(e)
    saved_contourOfReference=e->Attribute("ref");
}

TiXmlElement* ContourOfReferenceWidget::writeXMLFile(TiXmlNode *parent) {
  if(getContour()) {
    TiXmlElement *ele = new TiXmlElement(xmlName);
    ele->SetAttribute("ref", getContour()->getXMLPath(element,true).toStdString());
    parent->LinkEndChild(ele);
  }
  return 0;
}

RigidBodyOfReferenceWidget::RigidBodyOfReferenceWidget(const string &xmlName_, Element *element_, RigidBody* selectedBody_) : element(element_), selectedBody(selectedBody_), xmlName(xmlName_) {
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(0);
  setLayout(layout);

  body = new QLineEdit;
  body->setReadOnly(true);
  if(selectedBody)
    body->setText(selectedBody->getXMLPath());
  bodyBrowser = new RigidBodyBrowser(element->treeWidget(),0,this);
  connect(bodyBrowser,SIGNAL(accepted()),this,SLOT(setBody()));
  layout->addWidget(body);
  QPushButton *button = new QPushButton(tr("Browse"));
  connect(button,SIGNAL(clicked(bool)),bodyBrowser,SLOT(show()));
  layout->addWidget(button);
}

void RigidBodyOfReferenceWidget::initialize() {
  blockSignals(true);
  if(saved_bodyOfReference!="")
    setBody(element->getByPath<RigidBody>(saved_bodyOfReference));
  blockSignals(false);
}

void RigidBodyOfReferenceWidget::update() {
  bodyBrowser->update(selectedBody); 
  if(selectedBody) {
    setBody();
  }
}

void RigidBodyOfReferenceWidget::setBody() {
  if(bodyBrowser->getRigidBodyList()->currentItem())
    selectedBody = static_cast<RigidBody*>(static_cast<ElementItem*>(bodyBrowser->getRigidBodyList()->currentItem())->getElement());
  else
    selectedBody = 0;
  body->setText(selectedBody?selectedBody->getXMLPath():"");
  emit bodyChanged();
}

void RigidBodyOfReferenceWidget::setBody(RigidBody* body_) {
  selectedBody = body_;
  body->setText(selectedBody->getXMLPath());
  emit bodyChanged();
}

bool RigidBodyOfReferenceWidget::initializeUsingXML(TiXmlElement *parent) {
  TiXmlElement *e = parent->FirstChildElement(xmlName);
  if(e)
    saved_bodyOfReference=e->Attribute("ref");
}

TiXmlElement* RigidBodyOfReferenceWidget::writeXMLFile(TiXmlNode *parent) {
  if(getBody()) {
    TiXmlElement *ele = new TiXmlElement(xmlName);
    ele->SetAttribute("ref", getBody()->getXMLPath(element,true).toStdString());
    parent->LinkEndChild(ele);
  }
  return 0;
}

FileWidget::FileWidget(const string &xmlName_, const QString &description_, const QString &extensions_) : xmlName(xmlName_), description(description_), extensions(extensions_) {
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(0);
  setLayout(layout);

  fileName = new QLineEdit;
  fileName->setReadOnly(true);
  layout->addWidget(fileName);
  QPushButton *button = new QPushButton("Browse");
  layout->addWidget(button);
  connect(button,SIGNAL(clicked(bool)),this,SLOT(selectFile()));
}

void FileWidget::selectFile() {
  QString file=QFileDialog::getOpenFileName(0, description, QString("./"), extensions);
  if(file!="")
    fileName->setText(QString("\"")+file+"\"");
}

bool FileWidget::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e=element->FirstChildElement(xmlName);
  if(e) {
    TiXmlText *text = dynamic_cast<TiXmlText*>(e->FirstChild());
    if(text) {
      fileName->setText(text->Value());
      return true;
    }
  }
  return false;
}

TiXmlElement* FileWidget::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele0 = new TiXmlElement(xmlName);
  TiXmlText *text = new TiXmlText(fileName->text().toStdString());
  ele0->LinkEndChild(text);
  parent->LinkEndChild(ele0);

  return 0;
}

NameWidget::NameWidget(Element* ele, bool renaming) : element(ele) {
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(0);
  setLayout(layout);

  ename = new QLineEdit;
  ename->setReadOnly(true);
  ename->setText(element->getName());
  layout->addWidget(ename);
  if(renaming) {
    QPushButton *button = new QPushButton("Rename");
    layout->addWidget(button);
    connect(button,SIGNAL(clicked(bool)),this,SLOT(rename()));
  }
}

void NameWidget::rename() {
  QString text;
  do {
    text = QInputDialog::getText(0, tr("Rename"), tr("Name:"), QLineEdit::Normal, getName());
    if(((QTreeWidgetItem*)element)->parent() == 0)
      break;
    Element* ele = ((Container*)(((QTreeWidgetItem*)element)->parent()))->getChild(text.toStdString(),false);
    if(ele==0 || ele==element) {
      break;
    } 
    else {
      QMessageBox msgBox;
      msgBox.setText(QString("The name ") + text + " does already exist.");
      msgBox.exec();
    }
  } while(true);
  if(text!="")
    element->setName(text);
  ((Element*)element->treeWidget()->topLevelItem(0))->update();
}

bool NameWidget::initializeUsingXML(TiXmlElement *parent) {
  return true;
}

TiXmlElement* NameWidget::writeXMLFile(TiXmlNode *parent) {
  ((TiXmlElement*)parent)->SetAttribute("name", getName().toStdString());
  return 0;
}

ConnectFramesWidget::ConnectFramesWidget(int n, Element *element_) : element(element_) {
  
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setMargin(0);
  setLayout(layout);

  for(int i=0; i<n; i++) {
    QString xmlName = MBSIMNS"ref";
    QString subname = "Frame";
    if(n>1) {
      subname += QString::number(i+1);
      //layout->addWidget(new QLabel(QString("Frame") + QString::number(i+1) +":"));
      xmlName += QString::number(i+1);
    }
    widget.push_back(new FrameOfReferenceWidget(xmlName.toStdString(),element,0));
    QWidget *subwidget = new ExtXMLWidget(subname,widget[i]);
    layout->addWidget(subwidget);
  }
}

void ConnectFramesWidget::initialize() {
  for(unsigned int i=0; i<widget.size(); i++)
    widget[i]->initialize();
}

void ConnectFramesWidget::update() {
  for(unsigned int i=0; i<widget.size(); i++)
    widget[i]->update();
}

bool ConnectFramesWidget::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e = element->FirstChildElement(MBSIMNS"connect");
  if(e) {
    for(unsigned int i=0; i<widget.size(); i++) {
      QString xmlName = "ref";
      if(widget.size()>1)
        xmlName += QString::number(i+1);
      if(!e->Attribute(xmlName.toStdString()))
        return false;
      widget[i]->setSavedFrameOfReference(e->Attribute(xmlName.toAscii().data()));
    }
  }
  return true;
}

TiXmlElement* ConnectFramesWidget::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = new TiXmlElement(MBSIMNS"connect");
  for(unsigned int i=0; i<widget.size(); i++) {
      QString xmlName = "ref";
      if(widget.size()>1)
        xmlName += QString::number(i+1);
    if(widget[i]->getFrame())
      ele->SetAttribute(xmlName.toAscii().data(), widget[i]->getFrame()->getXMLPath(element,true).toStdString()); 
  }
  parent->LinkEndChild(ele);
  return ele;
}

ConnectContoursWidget::ConnectContoursWidget(int n, Element *element_) : element(element_) {
  
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setMargin(0);
  setLayout(layout);

  for(int i=0; i<n; i++) {
    QString xmlName = MBSIMNS"ref";
    QString subname = "Contour";
    if(n>1) {
      subname += QString::number(i+1);
      //layout->addWidget(new QLabel(QString("Contour") + QString::number(i+1) +":"));
      xmlName += QString::number(i+1);
    }
    widget.push_back(new ContourOfReferenceWidget(xmlName.toStdString(),element,0));
    QWidget *subwidget = new ExtXMLWidget(subname,widget[i]);
    layout->addWidget(subwidget);
  }
}

void ConnectContoursWidget::initialize() {
  for(unsigned int i=0; i<widget.size(); i++)
    widget[i]->initialize();
}

void ConnectContoursWidget::update() {
  for(unsigned int i=0; i<widget.size(); i++)
    widget[i]->update();
}

bool ConnectContoursWidget::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e = element->FirstChildElement(MBSIMNS"connect");
  if(e) {
    for(unsigned int i=0; i<widget.size(); i++) {
      QString xmlName = "ref";
      if(widget.size()>1)
        xmlName += QString::number(i+1);
      widget[i]->setSavedContourOfReference(e->Attribute(xmlName.toAscii().data()));
    }
  }
}

TiXmlElement* ConnectContoursWidget::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = new TiXmlElement(MBSIMNS"connect");
  for(unsigned int i=0; i<widget.size(); i++) {
      QString xmlName = "ref";
      if(widget.size()>1)
        xmlName += QString::number(i+1);
    if(widget[i]->getContour())
      ele->SetAttribute(xmlName.toAscii().data(), widget[i]->getContour()->getXMLPath(element,true).toStdString()); 
  }
  parent->LinkEndChild(ele);
  return ele;
}

DependenciesWidget::DependenciesWidget(const string &xmlName_, Element *element_) : element(element_), xmlName(xmlName_) {
  QHBoxLayout *layout = new QHBoxLayout;
  setLayout(layout);
  layout->setMargin(0);
  bodyList = new QListWidget;
  bodyList->setContextMenuPolicy (Qt::CustomContextMenu);
  bodyList->setMinimumWidth(bodyList->sizeHint().width()/3);
  bodyList->setMaximumWidth(bodyList->sizeHint().width()/3);
  layout->addWidget(bodyList);
  stackedWidget = new QStackedWidget;
  //connect(bodyList,SIGNAL(currentRowChanged(int)),this,SLOT(changeCurrent(int)));
  connect(bodyList,SIGNAL(currentRowChanged(int)),stackedWidget,SLOT(setCurrentIndex(int)));
  connect(bodyList,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(openContextMenu(const QPoint &)));
  connect(this,SIGNAL(bodyChanged()),this,SLOT(updateGeneralizedCoordinatesOfBodies()));
  layout->addWidget(stackedWidget,0,Qt::AlignTop);
}

void DependenciesWidget::openContextMenu(const QPoint &pos) {
 if(bodyList->itemAt(pos)) {
   QMenu menu(this);
   QAction *add = new QAction(tr("Remove"), this);
   connect(add, SIGNAL(triggered()), this, SLOT(removeDependency()));
   menu.addAction(add);
   menu.exec(QCursor::pos());
 }
 else {
   QMenu menu(this);
   QAction *add = new QAction(tr("Add"), this);
   connect(add, SIGNAL(triggered()), this, SLOT(addDependency()));
   menu.addAction(add);
   menu.exec(QCursor::pos());
 }
}

void DependenciesWidget::initialize() {
  for(unsigned int i=0; i<refBody.size(); i++)
    refBody[i]->initialize();
}

void DependenciesWidget::update() {
  for(unsigned int i=0; i<refBody.size(); i++)
    refBody[i]->update();
}

void DependenciesWidget::updateGeneralizedCoordinatesOfBodies() {
  for(unsigned int i=0; i<refBody.size(); i++) {
    if(selectedBody[i]) {
      selectedBody[i]->setConstrained(false);
      selectedBody[i]->resizeGeneralizedPosition();
      selectedBody[i]->resizeGeneralizedVelocity();
    }
    selectedBody[i] = refBody[i]->getBody();
    if(selectedBody[i]) {
      selectedBody[i]->setConstrained(true);
      selectedBody[i]->resizeGeneralizedPosition();
      selectedBody[i]->resizeGeneralizedVelocity();
      connect(selectedBody[i],SIGNAL(sizeChanged()),this,SIGNAL(bodyChanged()));
    }
  }
}

void DependenciesWidget::updateList() {
  emit bodyChanged();
  for(int i=0; i<bodyList->count(); i++)
    if(refBody[i]->getBody())
      bodyList->item(i)->setText(refBody[i]->getBody()->getName());
}

void DependenciesWidget::addDependency() {
  int i = refBody.size();
  selectedBody.push_back(0);
  refBody.push_back(new RigidBodyOfReferenceWidget(MBSIMNS"dependentRigidBody",element,0));
  connect(refBody[i],SIGNAL(bodyChanged()),this,SLOT(updateList()));
  bodyList->addItem("Undefined");
  stackedWidget->addWidget(refBody[i]);
  update();
}

void DependenciesWidget::removeDependency() {
  int i = bodyList->currentRow();
  if(selectedBody[i]) {
    selectedBody[i]->setConstrained(false);
    selectedBody[i]->resizeGeneralizedPosition();
    selectedBody[i]->resizeGeneralizedVelocity();
  }
  selectedBody.pop_back();

  stackedWidget->removeWidget(refBody[i]);
  delete refBody[i];
  refBody.erase(refBody.begin()+i);
  delete bodyList->takeItem(i);

  updateList();
}

bool DependenciesWidget::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e = element->FirstChildElement(xmlName);
  if(e) {
    TiXmlElement *ee=e->FirstChildElement();
    blockSignals(true);
    while(ee) {
      addDependency();
      refBody[refBody.size()-1]->setSavedBodyOfReference(ee->Attribute("ref"));
      ee=ee->NextSiblingElement();
    }
    blockSignals(false);
    return true;
  }
  return false;
}

TiXmlElement* DependenciesWidget::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele = new TiXmlElement(xmlName);
  for(int i=0; i<getSize(); i++) {
    if(getBody(i))
      refBody[i]->writeXMLFile(ele);
  }
  parent->LinkEndChild(ele);
  return ele;
}

ParameterNameWidget::ParameterNameWidget(Parameter* parameter_, bool renaming) : parameter(parameter_) {
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(0);
  setLayout(layout);

  ename = new QLineEdit;
  ename->setReadOnly(true);
  ename->setText(parameter->getName());
  layout->addWidget(ename);
  if(renaming) {
    QPushButton *button = new QPushButton("Rename");
    layout->addWidget(button);
    connect(button,SIGNAL(clicked(bool)),this,SLOT(rename()));
  }
}

void ParameterNameWidget::rename() {
  QString text;
  do {
    text = QInputDialog::getText(0, tr("Rename"), tr("Name:"), QLineEdit::Normal, getName());
    if(!getChild(parameter->treeWidget()->invisibleRootItem(), text))
      break;
    else {
      QMessageBox msgBox;
      msgBox.setText(QString("The name ") + text + " does already exist.");
      msgBox.exec();
    }
  } while(true);
  if(text!="")
    parameter->setName(text);
  //((Parameter*)parameter->treeWidget()->topLevelItem(0))->update();
}

bool ParameterNameWidget::initializeUsingXML(TiXmlElement *parent) {
  return true;
}

TiXmlElement* ParameterNameWidget::writeXMLFile(TiXmlNode *parent) {
  ((TiXmlElement*)parent)->SetAttribute("name", getName().toStdString());
  return 0;
}

ParameterValueWidget::ParameterValueWidget(PhysicalStringWidget *var) {
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(0);
  setLayout(layout);

  vector<PhysicalStringWidget*> input;
  input.push_back(var);
  widget = new ExtPhysicalVarWidget(input);
  QPushButton *button = new QPushButton("Set");

  layout->addWidget(widget);
  layout->addWidget(button);
  connect(button,SIGNAL(clicked(bool)),this,SLOT(parameterChanged()));
}

void ParameterValueWidget::parameterChanged() {
  emit parameterChanged(getValue().c_str());
}

SolverTolerances::SolverTolerances() {
  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);
  layout->setMargin(0);

  vector<PhysicalStringWidget*> input;
  input.push_back(new PhysicalStringWidget(new ScalarWidget("1e-15"), MBSIMNS"projection", noUnitUnits(), 1));
  projection = new ExtXMLWidget("Projection",new ExtPhysicalVarWidget(input),true);
  layout->addWidget(projection);

  input.clear();
  input.push_back(new PhysicalStringWidget(new ScalarWidget("1e-8"), MBSIMNS"g", noUnitUnits(), 1));
  g = new ExtXMLWidget("g",new ExtPhysicalVarWidget(input),true);
  layout->addWidget(g);

  input.clear();
  input.push_back(new PhysicalStringWidget(new ScalarWidget("1e-10"), MBSIMNS"gd", noUnitUnits(), 1));
  gd = new ExtXMLWidget("gd",new ExtPhysicalVarWidget(input),true);
  layout->addWidget(gd);

  input.clear();
  input.push_back(new PhysicalStringWidget(new ScalarWidget("1e-12"), MBSIMNS"gdd", noUnitUnits(), 1));
  gdd = new ExtXMLWidget("gdd",new ExtPhysicalVarWidget(input),true);
  layout->addWidget(gdd);

  input.clear();
  input.push_back(new PhysicalStringWidget(new ScalarWidget("1e-12"), MBSIMNS"la", noUnitUnits(), 1));
  la = new ExtXMLWidget("la",new ExtPhysicalVarWidget(input),true);
  layout->addWidget(la);

  input.clear();
  input.push_back(new PhysicalStringWidget(new ScalarWidget("1e-10"), MBSIMNS"La", noUnitUnits(), 1));
  La = new ExtXMLWidget("La",new ExtPhysicalVarWidget(input),true);
  layout->addWidget(La);
}

bool SolverTolerances::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e=element->FirstChildElement(MBSIMNS"tolerances");
  if(e) {
    projection->initializeUsingXML(e);
    g->initializeUsingXML(e);
    gd->initializeUsingXML(e);
    gdd->initializeUsingXML(e);
    la->initializeUsingXML(e);
    La->initializeUsingXML(e);
    return true;
  }
  return false;
}

TiXmlElement* SolverTolerances::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *e=new TiXmlElement(MBSIMNS"tolerances");
  parent->LinkEndChild(e);
  projection->writeXMLFile(e);
  g->writeXMLFile(e);
  gd->writeXMLFile(e);
  gdd->writeXMLFile(e);
  la->writeXMLFile(e);
  La->writeXMLFile(e);
  return e;
}

SolverParameters::SolverParameters() {
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setMargin(0);
  setLayout(layout);
  tolerances = new ExtXMLWidget("Tolerances",new SolverTolerances,true);
  layout->addWidget(tolerances);
}

bool SolverParameters::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e=element->FirstChildElement(MBSIMNS"solverParameters");
  if(e) {
    tolerances->initializeUsingXML(e);
    return true;
  }
  return false;
}

TiXmlElement* SolverParameters::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *e=new TiXmlElement(MBSIMNS"solverParameters");
  parent->LinkEndChild(e);
  tolerances->writeXMLFile(e);
  return e;
}

PlotFeature::PlotFeature(const string &name_) : name(name_) {
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(0);
  setLayout(layout);
  status = new QComboBox;
  status->addItem("enabled");
  status->addItem("disabled");
  layout->addWidget(status);
}

bool PlotFeature::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e=element->FirstChildElement(MBSIMNS"plotFeature");
  if(e) {
    if(string(e->Attribute("feature")).substr(1)==name) {
      status->setCurrentIndex(e->Attribute("feature")[0]=='+'?0:1);
      return true;
    }
  }
  return false;
}

TiXmlElement* PlotFeature::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *e=new TiXmlElement(MBSIMNS"plotFeature");
  parent->LinkEndChild(e);
  e->SetAttribute("feature",(status->currentIndex()==0?"+":"-")+name);
  return e;
}
