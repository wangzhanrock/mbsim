#include <mbxmlutils/preprocess.h>
#include "../config.h" // preprocess.h/octeval.h will undefine macro, hence include config.h after this file
#include <iostream>
#include <boost/filesystem.hpp>
#include <mbsimxml/mbsimflatxml.h>
#include <mbxmlutilshelper/dom.h>
#include <mbxmlutilshelper/getinstallpath.h>
#include <mbsim/objectfactory.h>
#include <mbsim/dynamic_system_solver.h>
#include <mbsimxml/mbsimxml.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem/fstream.hpp"

#include "zip.h"

#include <../general/valueReferenceMap_impl.h>

using namespace std;
using namespace boost::filesystem;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;
using namespace MBSimFMI;

namespace {
  enum ScalarVarType {
    Input,
    Output
  };

  void addScalarVariable(DOMElement *modelVars, size_t vr, string name, const string &desc, ScalarVarType type);

#ifdef _WIN32
  string SHEXT(".dll");
  #ifdef _WIN64
  string FMIOS("win64");
  #else
  string FMIOS("win32");
  #endif
#else
  string SHEXT(".so");
  #ifdef __x86_64__
  string FMIOS("linux64");
  #else
  string FMIOS("linux32");
  #endif
#endif
}

int main(int argc, char *argv[]) {
  // help
  if(argc!=2) {
    cout<<"Usage: "<<argv[0]<<" <MBSim Project XML File>"<<endl;
    cout<<endl;
    cout<<"Create mbsim.fmu in the current directory."<<endl;
    return 0;
  }

  // create octave
  vector<boost::filesystem::path> dependencies;
  OctEval octEval(&dependencies);
  // create parser
  boost::shared_ptr<DOMParser> parser=DOMParser::create(true);
  MBSim::generateMBSimXMLSchema(".mbsimxml.xsd", getInstallPath()/"share"/"mbxmlutils"/"schema");
  parser->loadGrammar(".mbsimxml.xsd");
  // create FMU zip file
  CreateZip fmuFile("mbsim.fmu");

  // load MBSim project XML document
  path mbsimxmlfile=argv[1];
  boost::shared_ptr<xercesc::DOMDocument> modelDoc=parser->parse(mbsimxmlfile);
  DOMElement *modelEle=modelDoc->getDocumentElement();

  // preprocess XML file
  Preprocess::preprocess(parser, octEval, dependencies, modelEle);

  // save preprocessed model to FMU
  string ppModelStr;
  DOMParser::serializeToString(modelEle, ppModelStr, false);
  fmuFile.add(path("resources")/"Model.mbsimprj.flat.xml", ppModelStr);

  // load all plugins
  MBSimXML::loadPlugins();

  // create object for DynamicSystemSolver
  boost::shared_ptr<DynamicSystemSolver> dss;
  dss.reset(ObjectFactory::createAndInit<DynamicSystemSolver>(modelEle->getFirstElementChild()));

  // build list of value references
  ValueReferenceMap::VRMap vrUnion;
  ValueReferenceMap::create(dss.get(), vrUnion);

  // initialize dss
  dss->initialize();

  // create DOM of modelDescription.xml
  boost::shared_ptr<DOMDocument> modelDescDoc(parser->createDocument());
  DOMElement *modelDesc=D(modelDescDoc)->createElement("fmiModelDescription");
  modelDescDoc->appendChild(modelDesc);
#ifdef _WIN32
  E(modelDesc)->setAttribute("author", getenv("USERNAME"));
#else
  E(modelDesc)->setAttribute("author", getenv("USER"));
#endif
  E(modelDesc)->setAttribute("description", "FMI export of a MBSim-XML model");
  E(modelDesc)->setAttribute("fmiVersion", "1.0");
  E(modelDesc)->setAttribute("generationDateAndTime",
    boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time())+"Z");
  E(modelDesc)->setAttribute("generationTool", string("MBSimFMI Version ")+VERSION);
  E(modelDesc)->setAttribute("version", "1.0");
  E(modelDesc)->setAttribute("guid", "mbsimfmi_guid");
  E(modelDesc)->setAttribute("modelIdentifier", "mbsim");
  path desc=mbsimxmlfile.filename();
  desc.replace_extension();
  desc.replace_extension();
  E(modelDesc)->setAttribute("modelName", desc.string());
  E(modelDesc)->setAttribute("numberOfContinuousStates", boost::lexical_cast<string>(dss->getzSize()));
  E(modelDesc)->setAttribute("numberOfEventIndicators", boost::lexical_cast<string>(dss->getsvSize()));
  E(modelDesc)->setAttribute("variableNamingConvention", "structured");
    // DefaultExperiment
    DOMElement *defaultExp=D(modelDescDoc)->createElement("DefaultExperiment");
    modelDesc->appendChild(defaultExp);
    E(defaultExp)->setAttribute("startTime", boost::lexical_cast<string>(0));
    E(defaultExp)->setAttribute("stopTime", boost::lexical_cast<string>(2));
    E(defaultExp)->setAttribute("tolerance", boost::lexical_cast<string>(1e-5));
    // ModelVariables
    DOMElement *modelVars=D(modelDescDoc)->createElement("ModelVariables");
    modelDesc->appendChild(modelVars);
    size_t vr=0;
    for(ValueReferenceMap::VRMap::iterator it=vrUnion.begin(); it!=vrUnion.end(); ++it, ++vr) {
      // ScalarVariable
      switch(it->first) {
        case ValueReferenceMap::GeneralizedIO_h:
          addScalarVariable(modelVars, vr, it->second.generalizedIO->getPath()+"/h", "ExternGeneralizedIO force", Input);
          break;
        case ValueReferenceMap::GeneralizedIO_x:
          addScalarVariable(modelVars, vr, it->second.generalizedIO->getPath()+"/x", "ExternGeneralizedIO position", Output);
          break;
        case ValueReferenceMap::GeneralizedIO_v:
          addScalarVariable(modelVars, vr, it->second.generalizedIO->getPath()+"/v", "ExternGeneralizedIO velocity", Output);
          break;
        case ValueReferenceMap::SignalSource:
          addScalarVariable(modelVars, vr, it->second.generalizedIO->getPath(), "ExternSignalSource", Input);
          break;
        case ValueReferenceMap::SignalSink:
          addScalarVariable(modelVars, vr, it->second.generalizedIO->getPath(), "ExternSignalSink", Output);
          break;
      }
    }
  // add modelDescription.xml file to FMU
  string modelDescriptionStr;
  DOMParser::serializeToString(modelDescDoc.get(), modelDescriptionStr);
  fmuFile.add("modelDescription.xml", modelDescriptionStr);

  // add binaries to FMU
  fmuFile.add(path("binaries")/FMIOS/("mbsim"+SHEXT), getInstallPath()/"lib"/("mbsimxml_fmi"+SHEXT));

  fmuFile.close();

  return 0;
}

namespace {
  void addScalarVariable(DOMElement *modelVars, size_t vr, string name, const string &desc, ScalarVarType type) {
    DOMElement *scalarVar=D(modelVars->getOwnerDocument())->createElement("ScalarVariable");
    modelVars->appendChild(scalarVar);
      DOMElement *varType=D(scalarVar->getOwnerDocument())->createElement("Real");
      scalarVar->appendChild(varType);
      boost::replace_all(name, "/", ".");
      E(scalarVar)->setAttribute("name", name.substr(1));
      E(scalarVar)->setAttribute("description", desc);
      E(scalarVar)->setAttribute("valueReference", boost::lexical_cast<string>(vr));
      switch(type) {
        case Input:
          E(scalarVar)->setAttribute("causality", "input");
          E(scalarVar)->setAttribute("variability", "continuous");
          E(varType)->setAttribute("start", boost::lexical_cast<string>(0)); // default value is 0, see also fmiinstance.cc
          break;
        case Output:
          E(scalarVar)->setAttribute("causality", "output");
          E(scalarVar)->setAttribute("variability", "continuous");
          break;
      }
  }

}