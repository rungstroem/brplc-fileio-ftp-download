#pragma once

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h> 
#endif

#include <fileIO.h>
#include <string>

/**
* dn = device name
* dh = device handle
* p = parameter string
* fn = file name
* d = data
* srcdl = source device link
* srcfn = source file name
* desdl = destination device link
* desfn = destination file name
*/

class fileIOClass {

	public:
	fileIOClass();
	~fileIOClass();

	UINT fileOpen(std::string fn, std::string dn);
	UINT fileCreate(std::string fn, std::string dn);
	UINT fileWrite(std::string d);
	UINT fileClose();
	UINT fileDelete(std::string fn, std::string dn);
	
	UINT fileCopy(std::string srcfn, std::string desfn);
	UINT fileCopy(std::string srcdl, std::string srcfn, std::string desdl, std::string desfn);
	
	bool createLocalFile(std::string fn, std::string dn);
	
	UINT createLocalDevlink(std::string dn, std::string p);
	UDINT getLocalDevlinkHandle();
	
	UINT createRemoteDevlink(std::string dn, std::string p);
	UDINT getRemoteDevlinkHandle();	

	UINT unlinkDevlink(UDINT dh);

	std::string return_error_string();
	
	protected:
	//Nessecary variables
	UDINT localDevlinkHandle;
	std::string localDevName;
	UDINT remoteDevlinkHandle;
	std::string remoteDevName;
	UDINT fileIdentity;	

	std::string error_id_string;
	
	std::string local_file_name;
	std::string remote_file_name;

};

