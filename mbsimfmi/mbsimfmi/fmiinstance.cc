#include "../config.h"
#include <fmiinstance.h>
#include <stdexcept>
#include <mbsimxml/mbsimflatxml.h>
#include <mbxmlutilshelper/dom.h>
#include <xercesc/dom/DOMDocument.hpp>
#include <mbsim/objectfactory.h>
#include <mbsim/dynamic_system_solver.h>
#include <mbsimControl/extern_signal_source.h>
#include <mbsimControl/extern_signal_sink.h>
#include <mbsim/extern_generalized_io.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

// rethrow a catched exception after prefixing the what() string with the FMI variable name
#define RETHROW_VR(vr) \
  catch(const exception &ex) { \
    rethrowVR(vr, ex.what()); \
  } \
  catch(...) { \
    rethrowVR(vr); \
  }

using namespace std;
using namespace boost::filesystem;
using namespace MBSim;
using namespace MBSimControl;
using namespace MBXMLUtils;

namespace MBSimFMI {

  PreVariable::PreVariable(Type type, char datatypeChar, const std::string &defaultValue) :
    Variable("dummy", "dummy", type, datatypeChar) {
    if(!defaultValue.empty()) {
      switch(datatypeChar) {
        case 'r': doubleValue =boost::lexical_cast<double>(defaultValue); break;
        case 'i': integerValue=boost::lexical_cast<int>   (defaultValue); break;
        case 'b': booleanValue=boost::lexical_cast<bool>  (defaultValue); break;
        case 's': stringValue =boost::lexical_cast<string>(defaultValue); break;
      }
    }
  }

  const double& PreVariable::getValue(const double&) {
    if(datatypeChar!='r') throw runtime_error("Internal error: Variable datatype differ.");
    return doubleValue;
  }

  const int& PreVariable::getValue(const int&) {
    if(datatypeChar!='i') throw runtime_error("Internal error: Variable datatype differ.");
    return integerValue;
  }

  const bool& PreVariable::getValue(const bool&) {
    if(datatypeChar!='b') throw runtime_error("Internal error: Variable datatype differ.");
    return booleanValue;
  }

  const string& PreVariable::getValue(const string&) {
    if(datatypeChar!='s') throw runtime_error("Internal error: Variable datatype differ.");
    return stringValue;
  }

  void PreVariable::setValue(const double &v) {
    if(datatypeChar!='r') throw runtime_error("Internal error: Variable datatype differ.");
    if(type==Output) throw runtime_error("Setting this variable is not allowed.");
    doubleValue=v;
  }

  void PreVariable::setValue(const int &v) {
    if(datatypeChar!='i') throw runtime_error("Internal error: Variable datatype differ.");
    if(type==Output) throw runtime_error("Setting this variable is not allowed.");
    integerValue=v;
  }

  void PreVariable::setValue(const bool &v) {
    if(datatypeChar!='b') throw runtime_error("Internal error: Variable datatype differ.");
    if(type==Output) throw runtime_error("Setting this variable is not allowed.");
    booleanValue=v;
  }

  void PreVariable::setValue(const std::string &v) {
    if(datatypeChar!='s') throw runtime_error("Internal error: Variable datatype differ.");
    if(type==Output) throw runtime_error("Setting this variable is not allowed.");
    stringValue=v;
  }

  // A MBSIM FMI instance
  FMIInstance::FMIInstance(fmiString instanceName_, fmiString GUID, fmiCallbackFunctions functions, fmiBoolean loggingOn) :
    instanceName(instanceName_),
    logger(functions.logger),
    infoBuffer (logger, this, instanceName, fmiOK,      "info"),
    warnBuffer (logger, this, instanceName, fmiWarning, "warning"),
    debugBuffer(logger, this, instanceName, fmiOK,      "debug") {

    // use the per FMIInstance provided buffers for all subsequent fmatvec::Atom objects
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Info,  boost::make_shared<ostream>(&infoBuffer));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Warn,  boost::make_shared<ostream>(&warnBuffer));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Debug, boost::make_shared<ostream>(&debugBuffer));
    // also use these streams for this object.
    // Note: we can not create a FMIInstance object with the correct streams but we can adopt the streams now!
    adoptMessageStreams(); // note: no arg means adopt the current (static) message streams (set above)
    // Now we can use msg(...)<< to print messages using the FMI logger

    // set debug stream according loggingOn
    setMessageStreamActive(Debug, loggingOn);
    msg(Debug)<<"Enabling debug logging."<<endl;

    // check GUID: we use currently a constant GUID
    if(string(GUID)!="mbsimfmi_guid")
      throw runtime_error("GUID provided by caller and internal GUID does not match.");

    // load modelDescription XML file
    parser=DOMParser::create(false);
    msg(Debug)<<"Read modelDescription file."<<endl;
    path modelDescriptionXMLFile=getSharedLibDir().parent_path().parent_path()/"modelDescription.xml";
    boost::shared_ptr<xercesc::DOMDocument> doc=parser->parse(modelDescriptionXMLFile);

    // create FMI variables from modelDescription.xml file
    msg(Debug)<<"Generate variables used before fmiInitialize."<<endl;
    size_t vr=0;
    for(xercesc::DOMElement *scalarVar=E(doc->getDocumentElement())->getFirstElementChildNamed("ModelVariables")->getFirstElementChild();
        scalarVar; scalarVar=scalarVar->getNextElementSibling(), ++vr) {
      msg(Debug)<<"Generate variable '"<<E(scalarVar)->getAttribute("name")<<"'"<<endl;
      if(vr!=boost::lexical_cast<size_t>(E(scalarVar)->getAttribute("valueReference")))
        throw runtime_error("Internal error: valueReference missmatch!");
      // get type
      Type type;
           if(E(scalarVar)->getAttribute("causality")=="internal" && E(scalarVar)->getAttribute("variability")=="parameter")  type=Parameter;
      else if(E(scalarVar)->getAttribute("causality")=="input"    && E(scalarVar)->getAttribute("variability")=="continuous") type=Input;
      else if(E(scalarVar)->getAttribute("causality")=="output"   && E(scalarVar)->getAttribute("variability")=="continuous") type=Output;
      else throw runtime_error("Internal error: Unknwon variable type.");
      // get datatype
      char datatypeChar;
           if(E(scalarVar)->getFirstElementChildNamed("Real")) datatypeChar='r';
      else if(E(scalarVar)->getFirstElementChildNamed("Integer") ||
              E(scalarVar)->getFirstElementChildNamed("Enumeration")) datatypeChar='i';
      else if(E(scalarVar)->getFirstElementChildNamed("Boolean")) datatypeChar='b';
      else if(E(scalarVar)->getFirstElementChildNamed("String")) datatypeChar='s';
      else throw runtime_error("Internal error: Unknown variable datatype.");
      // get default
      string defaultValue=E(scalarVar->getFirstElementChild())->getAttribute("start");
      // create preprocessing variable
      var.push_back(boost::make_shared<PreVariable>(type, datatypeChar, defaultValue));
    }
  }

  // destroy a MBSim FMI instance
  FMIInstance::~FMIInstance() {
  }

  // print exceptions using the FMI logger
  void FMIInstance::logException(const exception &ex) {
    logger(this, instanceName.c_str(), fmiError, "error", ex.what());
  }

  // FMI wrapper functions

  void FMIInstance::setDebugLogging(fmiBoolean loggingOn) {
    if(!loggingOn)
      msg(Debug)<<"Disabling debug logging."<<endl;
    // set debug stream according loggingOn
    setMessageStreamActive(Debug, loggingOn);
    if(loggingOn)
      msg(Debug)<<"Enabling debug logging."<<endl;
  }

  void FMIInstance::setTime(fmiReal time_) {
    time=time_;
  }

  void FMIInstance::setContinuousStates(const fmiReal x[], size_t nx) {
    for(size_t i=0; i<nx; ++i)
      z(i)=x[i];
  }

  // is called when the current state is a valid/accepted integrator step.
  // (e.g. not a intermediate Runge-Kutta step of a step which will be rejected due to
  // the error tolerances of the integrator)
  void FMIInstance::completedIntegratorStep(fmiBoolean* callEventUpdate) {
    *callEventUpdate=false;

    switch(predefinedVar.plotMode) {
      case EverynthCompletedStep:
        completedStepCounter++;
        if(completedStepCounter==predefinedVar.plotEachNStep) {
          completedStepCounter=0;
          dss->plot(z, time);
        }
        break;
      case NextCompletedStepAfterSampleTime:
        if(time>=nextPlotTime) {
          nextPlotTime += predefinedVar.plotStepSize * (floor((time-nextPlotTime)/predefinedVar.plotStepSize)+1);
          dss->plot(z, time);
        }
        break;
      case SampleTime:
        break;
    }
  }

  // set a real/integer/boolean/string variable
  template<typename CppDatatype, typename FMIDatatype>
  void FMIInstance::setValue(const fmiValueReference vr[], size_t nvr, const FMIDatatype value[]) {
    for(size_t i=0; i<nvr; ++i) {
      if(vr[i]>=var.size())
        throw runtime_error("No such value reference "+boost::lexical_cast<string>(vr[i]));
      try { var[vr[i]]->setValue(CppDatatype(value[i])); } RETHROW_VR(vr[i])
    }
  }
  // explicitly instantiate all four FMI types
  template void FMIInstance::setValue<double, fmiReal   >(const fmiValueReference vr[], size_t nvr, const fmiReal    value[]);
  template void FMIInstance::setValue<int,    fmiInteger>(const fmiValueReference vr[], size_t nvr, const fmiInteger value[]);
  template void FMIInstance::setValue<bool,   fmiBoolean>(const fmiValueReference vr[], size_t nvr, const fmiBoolean value[]);
  template void FMIInstance::setValue<string, fmiString >(const fmiValueReference vr[], size_t nvr, const fmiString  value[]);

  void FMIInstance::initialize(fmiBoolean toleranceControlled, fmiReal relativeTolerance, fmiEventInfo* eventInfo) {
    // after the ctor call another FMIInstance ctor may be called, hence we need to reset the message streams here
    // use the per FMIInstance provided buffers for all subsequent fmatvec::Atom objects
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Info,  boost::make_shared<ostream>(&infoBuffer));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Warn,  boost::make_shared<ostream>(&warnBuffer));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Debug, boost::make_shared<ostream>(&debugBuffer));

    // set eventInfo (except next event time)
    eventInfo->iterationConverged=true;
    eventInfo->stateValueReferencesChanged=false;
    eventInfo->stateValuesChanged=false;
    eventInfo->terminateSimulation=false;

    // get the model file
    path mbsimflatxmlfile=getSharedLibDir().parent_path().parent_path()/"resources"/"Model.mbsimprj.flat.xml";

    // load all plugins
    msg(Debug)<<"Load MBSim plugins."<<endl;
    MBSimXML::loadPlugins();
  
    // load MBSim project XML document
    msg(Debug)<<"Read MBSim XML model file."<<endl;
    boost::shared_ptr<xercesc::DOMDocument> doc=parser->parse(mbsimflatxmlfile);
  
    // create object for DynamicSystemSolver
    msg(Debug)<<"Create DynamicSystemSolver."<<endl;
    dss.reset(ObjectFactory::createAndInit<DynamicSystemSolver>(doc->getDocumentElement()->getFirstElementChild()));

    // build list of value references (variables)
    msg(Debug)<<"Create all FMI variables."<<endl;
    std::vector<boost::shared_ptr<Variable> > varSim; // do not overwrite var here, use varSim (see below)
    createAllVariables(dss.get(), varSim, predefinedVar);

    // save the current dir and change to outputDir -> MBSim will create output the current dir
    msg(Debug)<<"Write MBSim output files to "<<predefinedVar.outputDir<<endl;
    path savedCurDir=current_path();
    current_path(predefinedVar.outputDir);
    // initialize dss
    msg(Debug)<<"Initialize DynamicSystemSolver."<<endl;
    dss->initialize();
    // restore current dir (normally we are not allowed to change the current dir at all)
    current_path(savedCurDir);

    // Till now (between fmiInstantiateModel and fmiInitialize) we have only used objects of type PreVariable's in var.
    // Now we copy all values from val to varSim (generated above).
    if(var.size()!=varSim.size())
      throw runtime_error("Internal error: The number of parameters differ.");
    vector<boost::shared_ptr<Variable> >::iterator varSimIt=varSim.begin();
    size_t vr=0;
    for(vector<boost::shared_ptr<Variable> >::iterator varIt=var.begin(); varIt!=var.end(); ++varIt, ++varSimIt, ++vr) {
      try {
        if((*varSimIt)->getType()!=(*varIt)->getType())
          throw runtime_error("Internal error: Variable type does not match.");
        if((*varIt)->getType()==Output) // outputs are not allowed to be set -> skip
          continue;
        msg(Debug)<<"Copy variable '"<<(*varSimIt)->getName()<<"' from preprocessing space to simulation space."<<endl;
        switch((*varIt)->getDatatypeChar()) {
          case 'r': (*varSimIt)->setValue((*varIt)->getValue(double())); break;
          case 'i': (*varSimIt)->setValue((*varIt)->getValue(int())); break;
          case 'b': (*varSimIt)->setValue((*varIt)->getValue(bool())); break;
          case 's': (*varSimIt)->setValue((*varIt)->getValue(string())); break;
        }
      }
      RETHROW_VR(vr)
    }
    // var is now no longer needed since we use varSim now.
    var=varSim;

    // initialize state
    z.resize(dss->getzSize());
    zd.resize(dss->getzSize());
    dss->initz(z);
    dss->computeInitialCondition();

    // initialize stop vector
    sv.resize(dss->getsvSize());
    svLast.resize(dss->getsvSize());
    jsv.resize(dss->getsvSize(), fmatvec::INIT, 0); // init with 0 = no shift in all indices
    // initialize last stop vector with initial stop vector state (see eventUpdate for details)
    dss->getsv(z, svLast, time);

    // plot initial state
    dss->plot(z, time);

    // handling of plot mode
    switch(predefinedVar.plotMode) {
      case EverynthCompletedStep:
        // init
        completedStepCounter=0;
        // no next time event
        eventInfo->upcomingTimeEvent=false;
        break;
      case NextCompletedStepAfterSampleTime:
        // init
        nextPlotTime=time+predefinedVar.plotStepSize;
        // next time event
        eventInfo->upcomingTimeEvent=false;
        break;
      case SampleTime:
        // init
        nextPlotTime=time+predefinedVar.plotStepSize;
        // next time event
        eventInfo->upcomingTimeEvent=true;
        eventInfo->nextEventTime=nextPlotTime;
        break;
    }
  }

  void FMIInstance::getDerivatives(fmiReal derivatives[], size_t nx) {
    // calcualte MBSim zd and return it
    dss->zdot(z, zd, time);
    for(size_t i=0; i<nx; ++i)
      derivatives[i]=zd(i);
  }

  void FMIInstance::getEventIndicators(fmiReal eventIndicators[], size_t ni) {
    // calcualte MBSim stop vector and return it
    dss->getsv(z, sv, time);
    for(size_t i=0; i<ni; ++i)
      eventIndicators[i]=sv(i);
  }

  // get a real/integer/boolean/string variable
  template<typename CppDatatype, typename FMIDatatype>
  void FMIInstance::getValue(const fmiValueReference vr[], size_t nvr, FMIDatatype value[]) {
    for(size_t i=0; i<nvr; ++i) {
      if(vr[i]>=var.size())
        throw runtime_error("No such value reference "+boost::lexical_cast<string>(vr[i]));
      try { value[i]=var[vr[i]]->getValue(CppDatatype()); } RETHROW_VR(vr[i])
    }
  }
  // explicitly instantiate all four FMI types
  template void FMIInstance::getValue<double, fmiReal   >(const fmiValueReference vr[], size_t nvr, fmiReal value[]);
  template void FMIInstance::getValue<int,    fmiInteger>(const fmiValueReference vr[], size_t nvr, fmiInteger value[]);
  template void FMIInstance::getValue<bool,   fmiBoolean>(const fmiValueReference vr[], size_t nvr, fmiBoolean value[]);
  // explicitly specialization for string
  template<>
  void FMIInstance::getValue<string, fmiString>(const fmiValueReference vr[], size_t nvr, fmiString value[]) {
    for(size_t i=0; i<nvr; ++i) {
      if(vr[i]>=var.size())
        throw runtime_error("No such value reference "+boost::lexical_cast<string>(vr[i]));
      try { value[i]=var[vr[i]]->getValue(string()).c_str(); } RETHROW_VR(vr[i])
    }
  }

  // MBSim shift
  // Note: The FMI interface does not provide a jsv integer vector as e.g. the LSODAR integrator does.
  // Since MBSim required this information we have to generate it here. For this we:
  // * store the stop vector (sv) of the initial state (see initialize(...)) or the
  //   last event (shift point) in the variable svLast.
  // * compare the current sv with the sv of the last event (or initial state) svLast
  // * set all entries in jsv to 1 if the corresponding entries in sv and svLast have a sign change.
  void FMIInstance::eventUpdate(fmiBoolean intermediateResults, fmiEventInfo* eventInfo) {
    // initialize eventInfo fields (except next time event)
    eventInfo->iterationConverged=true;
    eventInfo->stateValueReferencesChanged=false;
    eventInfo->stateValuesChanged=false;
    eventInfo->terminateSimulation=false;

    // state event = root = stop vector event

    // MISSING: event handling must be conform to FMI and modellica!!!???
    // get current stop vector
    dss->getsv(z, sv, time);
    // compare last (svLast) and current (sv) stop vector: build jsv and set shiftRequired
    bool shiftRequired=false;
    for(int i=0; i<sv.size(); ++i) {
      jsv(i)=(svLast(i)*sv(i)<0); // on sign change in svLast and sv set jsv to 1 (shift this index i)
      if(jsv(i))
        shiftRequired=true; // set shiftRequired: a MBSim shift call is required (at least 1 entry in jsv is 1)
                            // (the environment may call eventUpdate of this system due to an event (shift) of another
                            // system (FMU, ...). Hence not all eventUpdate must generate a shift of this system)
    }
    if(shiftRequired) {
      // shift MBSim system
      dss->shift(z, jsv, time);
      // A MBSim shift always changes state values (non-smooth-mechanic)
      // This must be reported to the environment (the integrator must be resetted in this case).
      eventInfo->stateValuesChanged=true;
    }

    // time event (currently only for plotting)

    switch(predefinedVar.plotMode) {
      case EverynthCompletedStep:
      case NextCompletedStepAfterSampleTime:
        // no next time event
        eventInfo->upcomingTimeEvent=false;
        break;
      case SampleTime:
        // next time event
        eventInfo->upcomingTimeEvent=true;
        eventInfo->nextEventTime=nextPlotTime;

        // next event wenn plotting with sample time and we currently match that time
        if(fabs(time-nextPlotTime)<1.0e-10) {
          // plot
          dss->plot(z, time);
          // next time event
          nextPlotTime=time+predefinedVar.plotStepSize;
          eventInfo->nextEventTime=nextPlotTime;
        }
        break;
    }
  }

  void FMIInstance::getContinuousStates(fmiReal states[], size_t nx) {
    for(size_t i=0; i<nx; ++i)
      states[i]=z(i);
  }

  void FMIInstance::getNominalContinuousStates(fmiReal x_nominal[], size_t nx) {
    // we do not proved a nominal value for states in MBSim -> just return 1 as nominal value.
    for(size_t i=0; i<nx; ++i)
      x_nominal[i]=1;
  }

  void FMIInstance::getStateValueReferences(fmiValueReference vrx[], size_t nx) {
    // we do not assign any MBSim state to a FMI valueReference -> return fmiUndefinedValueReference for all states
    for(size_t i=0; i<nx; ++i)
      vrx[i]=fmiUndefinedValueReference;
  }

  void FMIInstance::terminate() {
    // plot end state
    dss->plot(z, time);

    // delete DynamicSystemSolver (requried here since after terminate a call to initialize is allowed without
    // calls to fmiFreeModelInstance and fmiInstantiateModel)
    dss.reset();
  }

  void FMIInstance::rethrowVR(size_t vr, const std::string &what) {
    throw runtime_error(string("In variable #")+var[vr]->getDatatypeChar()+boost::lexical_cast<string>(vr)+"#: "+what);
  }

}