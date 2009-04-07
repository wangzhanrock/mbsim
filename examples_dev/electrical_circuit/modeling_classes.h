#ifndef _MODELING_CLASSES_H
#define _MODELING_CLASSES_H

#include "mbsim/element.h"
#include <string>
#include <vector>

using namespace std;

class Component;

class Pin : public MBSim::Element {
  vector<Pin*> connectedPin;
  int flag;
  Component* parent;
  public:
    Pin(const string &name) : Element(name), flag(0), parent(0) {}
    void addConnectedPin(Pin* pin);
    void setFlag(int f) { flag = f; }
    int getFlag() const { return flag; }
    void go(Pin* callingPin);
    void setParent(Component* p) { parent = p; }
    Component* getParent() const { return parent; }
};

void connectPin(Pin *pin1, Pin *pin2);

class Component : public MBSim::Element {
  protected:
    vector<Pin*> pin;
    int qSize, qInd;
    int uSize, uInd;
    int hSize, hInd;
  public:
    Component(const string &name) : Element(name), qSize(0), qInd(0), uSize(0), uInd(0) {}
    void addPin(Pin *pin);
    void addPin(const string &str);
    Pin* getPin(const string &name, bool check=true);
    virtual void calcqSize() {};
    void setqInd(int qInd_) {qInd = qInd_;}
    int getqSize() const {return qSize;}
    virtual void calcuSize() {};
    void setuInd(int uInd_) {uInd = uInd_;}
    int getuSize() const {return uSize;}
    void sethInd(int hInd_) {hInd = hInd_;}
    int getuInd() const {return uInd;}
    int gethInd() const {return hInd;}
    int gethSize() const {return hSize;}
    void sethSize(int hSize_) {hSize = hSize_;}
    void buildListOfPins(std::vector<Pin*> &pin, bool recursive = true);
};

class CompVoltageSource : public Component {
  public:
    CompVoltageSource(const string &name);
};

class CompInductor : public Component {
  public:
    CompInductor(const string &name);
};

class CompResistor : public Component {
  public:
    CompResistor(const string &name);
};

class CompCapacitor : public Component {
  public:
    CompCapacitor(const string &name);
};


#endif
