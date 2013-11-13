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

#ifndef _BASIC_PROPERTIES_H_
#define _BASIC_PROPERTIES_H_

#include <string>
#include "utils.h"
#include "extended_properties.h"
#include "basic_widgets.h"
#include "property_property_dialog.h"

class Element;
class Frame;
class Contour;
class Object;
class Link;
class RigidBody;
class Signal;

namespace MBXMLUtils {
  class TiXmlElement;
  class TiXmlNode;
}

class LocalFrameOfReferenceProperty : public Property {
  protected:
    Frame *framePtr;
    Element* element;
  public:
    LocalFrameOfReferenceProperty(const std::string &name="", const std::string &frame_="", Element* element_=0);
    virtual Property* clone() const {return new LocalFrameOfReferenceProperty(*this);}
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element); 
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    void setFrame(const std::string &str);
    std::string getFrame() const;
    Frame* getFramePtr() const { return framePtr; }
    Widget* createWidget() { return new LocalFrameOfReferenceWidget(element,element); }
};

class ParentFrameOfReferenceProperty : public Property {
  protected:
    Frame *framePtr;
    Element* element;
  public:
    ParentFrameOfReferenceProperty(const std::string &name="", const std::string &frame_="", Element* element_=0);
    virtual Property* clone() const {return new ParentFrameOfReferenceProperty(*this);}
    virtual MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void initialize();
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    void setFrame(const std::string &str);
    std::string getFrame() const;
    Frame* getFramePtr() const { return framePtr; }
    Widget* createWidget() { return new ParentFrameOfReferenceWidget(element,element); }
};

class FrameOfReferenceProperty : public Property {
  protected:
    Frame *framePtr;
    Element* element;
  public:
    FrameOfReferenceProperty(const std::string &name="", const std::string &frame="", Element* element=0); 
    virtual Property* clone() const {return new FrameOfReferenceProperty(*this);}
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element); 
    void initialize();
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    void setFrame(const std::string &str);
    std::string getFrame() const;
    Frame* getFramePtr() const { return framePtr; }
    Widget* createWidget() { return new FrameOfReferenceWidget(element,0); }
};

class ContourOfReferenceProperty : public Property {
  protected:
    std::string contour;
    Contour *contourPtr;
    Element* element;
    std::string xmlName;
  public:
    ContourOfReferenceProperty(const std::string &contour_="", Element* element_=0, const std::string &xmlName_=""); 
    virtual Property* clone() const {return new ContourOfReferenceProperty(*this);}
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element); 
    void initialize();
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    void setContour(const std::string &str);
    std::string getContour() const; 
};

class RigidBodyOfReferenceProperty : public Property {
  protected:
    std::string body;
    RigidBody *bodyPtr;
    Element* element;
    std::string xmlName;
  public:
    RigidBodyOfReferenceProperty(const std::string &body_="", Element *element_=0, const std::string &xmlName_="");
    virtual Property* clone() const {return new RigidBodyOfReferenceProperty(*this);}
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element); 
    void initialize();
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    void setBody(const std::string &str);
    std::string getBody() const;
    RigidBody* getBodyPtr() const {return bodyPtr;}
};

class ObjectOfReferenceProperty : public Property {
  protected:
    std::string object;
    Object *objectPtr;
    Element* element;
    std::string xmlName;
  public:
    ObjectOfReferenceProperty(const std::string &object_="", Element *element_=0, const std::string &xmlName_=""); 
    virtual Property* clone() const {return new ObjectOfReferenceProperty(*this);}
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element); 
    void initialize();
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    void setObject(const std::string &str);
    std::string getObject() const;
};

class LinkOfReferenceProperty : public Property {
  protected:
    std::string link;
    Link *linkPtr;
    Element* element;
    std::string xmlName;
  public:
    LinkOfReferenceProperty(const std::string &link_="", Element *element_=0, const std::string &xmlName_=""); 
    virtual Property* clone() const {return new LinkOfReferenceProperty(*this);}
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element); 
    void initialize();
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    void setLink(const std::string &str);
    std::string getLink() const;
};

class SignalOfReferenceProperty : public Property {
  protected:
    std::string signal;
    Signal *signalPtr;
    Element* element;
    std::string xmlName;
  public:
    SignalOfReferenceProperty(const std::string &signal_="", Element *element_=0, const std::string &xmlName_=""); 
    virtual Property* clone() const {return new SignalOfReferenceProperty(*this);}
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element); 
    void initialize();
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    void setSignal(const std::string &str);
    std::string getSignal() const;
};

class FileProperty : public Property {

  public:
    FileProperty(const std::string &xmlName_) : xmlName(xmlName_) {}
    virtual Property* clone() const {return new FileProperty(*this);}
    virtual MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    const std::string& getFile() const {return file;}
    void setFile(const std::string &str) {file=str;}

  protected:
    std::string file;
    std::string xmlName;
};

class IntegerProperty : public Property {

  public:
    IntegerProperty(const std::string &name="", int value_=0, const std::string &xmlName_="") : Property(name,toStr(value_)), value(value_), xmlName(xmlName_) {}
    virtual Property* clone() const {return new IntegerProperty(*this);}
    virtual MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    int getInt() const {return value;}
    void setInt(int value_) {value = value_; setValue(toStr(value));}
//    std::string getValue() const { return toStr(value); }
//    void setValue(const std::string &data) { value = atoi(data.c_str()); }
    Widget* createWidget() { return new SpinBoxWidget; }

  protected:
    int value;
    std::string xmlName;
};

class TextProperty : public Property {

  public:
    TextProperty(const std::string &name="", const std::string &text="", const std::string &xmlName_="", bool quote_=false) : Property(name,text), xmlName(xmlName_), quote(quote_) {}
    virtual Property* clone() const {return new TextProperty(*this);}
    virtual MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    //const std::string& getText() const {return text;}
    //void setText(const std::string &text_) {text = text_; setValue(text);}
//    std::string getValue() const { return text; }
//    void setValue(const std::string &data) { text = data; }
    Widget* createWidget() { return new TextWidget("Text"); }

  protected:
    std::string xmlName;
    bool quote;
};

class ConnectFramesProperty : public Property {

  public:
    ConnectFramesProperty(int n, Element* element, const std::string &xmlName_=MBSIMNS"connect");
    virtual Property* clone() const {return new ConnectFramesProperty(*this);}

    void initialize();
    virtual MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    std::vector<FrameOfReferenceProperty> frame;
    std::string xmlName;
};

class ConnectContoursProperty : public Property {

  public:
    ConnectContoursProperty(int n, Element* element, const std::string &xmlName_=MBSIMNS"connect");
    virtual Property* clone() const {return new ConnectContoursProperty(*this);}

    void initialize();
    virtual MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    std::vector<ContourOfReferenceProperty> contour;
    std::string xmlName;
};

class SolverChoiceProperty : public Property {

  public:
    SolverChoiceProperty(const std::string &xmlName_) : choice("FixedPointSingle"), xmlName(xmlName_) {}
    virtual Property* clone() const {return new SolverChoiceProperty(*this);}

    virtual MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    std::string choice;
    std::string xmlName;
};

class SolverTolerancesProperty : public Property {

  public:
    SolverTolerancesProperty();
    virtual Property* clone() const {return new SolverTolerancesProperty(*this);}

    virtual MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    ExtProperty projection, g, gd, gdd, la, La;
};

class SolverParametersProperty : public Property {

  public:
    SolverParametersProperty();
    virtual Property* clone() const {return new SolverParametersProperty(*this);}

    virtual MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    ExtProperty constraintSolver, impactSolver, numberOfMaximalIterations, tolerances;
};

class EmbedProperty : public Property {

  public:
    EmbedProperty(Element *element);
    virtual Property* clone() const {return new EmbedProperty(*this);}
    virtual MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    bool hasFile() const {return false;}//(href.isActive() && static_cast<const FileProperty*>(href.getProperty())->getFile()!="");}
    std::string getFile() const {return static_cast<const FileProperty*>(href.getProperty())->getFile();}
    bool hasCounter() const {return false;}//counterName.isActive();}
    std::string getCounterName() const {return static_cast<const TextProperty*>(counterName.getProperty())->getValue();}
    bool hasParameterFile() const {return false;}//(parameterList.isActive() && static_cast<const FileProperty*>(parameterList.getProperty())->getFile()!="");}
    std::string getParameterFile() const {return static_cast<const FileProperty*>(parameterList.getProperty())->getFile();}

  protected:
    ExtProperty href, count, counterName, parameterList;

};

class SignalReferenceProperty : public Property {
  public:
    SignalReferenceProperty(Element* element);
    virtual Property* clone() const {return new SignalReferenceProperty(*this);}
    void initialize() {refSignal.initialize();}
    virtual MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
  protected:
    SignalOfReferenceProperty refSignal;
    ExtProperty factor;
};

class ColorProperty : public Property {
  protected:
    ExtProperty color;
    std::string xmlName;
  public:
    ColorProperty(const std::string &xmlName=""); 
    virtual Property* clone() const {return new ColorProperty(*this);}
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element); 
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
};

#endif

