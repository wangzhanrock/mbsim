#include "zip.h"
#include <exception>
#include <iostream>
#include <archive.h>
#include <archive_entry.h>
#include <boost/filesystem/fstream.hpp>

using namespace std;
using namespace boost::filesystem;

namespace MBSimFMI {

CreateZip::CreateZip(const path &zipFile_) : closed(false), zipFile(zipFile_) {
  a=archive_write_new();
  if(!a)
    throw runtime_error("Unable to create archive struct.");
  if(archive_write_set_format_zip(a))
    throw runtime_error("Unable to set zip format on archive.");
  if(archive_write_open_filename(a, zipFile.string().c_str()))
    throw runtime_error("Unable to open archive file "+zipFile.string()+".");
  entry=archive_entry_new();
  if(!entry)
    throw runtime_error("Unable to create archive entry struct.");
}

CreateZip::~CreateZip() {
  try {
    if(!closed)
      close();
  }
  catch(const exception &ex) {
    cerr<<"Exception from CreateZip::close() in destructor of CreateZip (ZIP filename "<<zipFile<<") catched:"<<endl
        <<ex.what()<<endl;
  }
  catch(...) {
    cerr<<"Unknown exception from CreateZip::close() in destructor of CreateZip (ZIP filename '"<<zipFile<<"') catched."<<endl;
  }
  archive_entry_free(entry);
  archive_write_free(a);
}

void CreateZip::close() {
  closed=true;
  if(archive_write_close(a))
    throw runtime_error("Unable to close archive file "+zipFile.string()+".");
}

void CreateZip::add(const path &filenameInZip, const path &srcFilename) {
  if(closed)
    throw runtime_error("ZIP file "+zipFile.string()+" is already closed.");
  if(!archive_entry_clear(entry))
    throw runtime_error("Unable to clear archive entry.");
  vector<char> buf(1024*1024*10); // read/write file content in 10MB blocks
  archive_entry_set_pathname(entry, filenameInZip.string().c_str());
  archive_entry_set_size(entry, file_size(srcFilename));
  archive_entry_set_filetype(entry, AE_IFREG);
  archive_entry_set_perm(entry, 0644);
  if(archive_write_header(a, entry))
    throw runtime_error("Unable to write entry header to archive "+zipFile.string()+".");
  boost::filesystem::ifstream f(srcFilename);
  do {
    f.read(&buf[0], buf.size());
    if(archive_write_data(a, &buf[0], f.gcount())!=f.gcount())
      throw runtime_error("Unable to write entry data.");
  }
  while(f.gcount()==buf.size());
  f.close();
}

void CreateZip::add(const path &filenameInZip, const string &textContent) {
  if(closed)
    throw runtime_error("ZIP file "+zipFile.string()+" is already closed.");
  if(!archive_entry_clear(entry))
    throw runtime_error("Unable to clear archive entry.");
  archive_entry_set_pathname(entry, filenameInZip.string().c_str());
  archive_entry_set_size(entry, textContent.size());
  archive_entry_set_filetype(entry, AE_IFREG);
  archive_entry_set_perm(entry, 0644);
  if(archive_write_header(a, entry))
    throw runtime_error("Unable to write entry header to archive "+zipFile.string()+".");
  if(archive_write_data(a, textContent.c_str(), textContent.size())!=textContent.size())
    throw runtime_error("Unable to write entry data.");
}

}
