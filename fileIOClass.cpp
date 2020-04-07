#include "fileIOClass.h"
#include <string>

fileIOClass::fileIOClass(){
}

UINT fileIOClass::createRemoteDevlink(std::string devname, std::string param){
	UINT status;
	while(1){
		DevLink_REMOTE.enable = 1;
		DevLink(&DevLink_REMOTE);
		DevLink_REMOTE.pDevice = (UDINT) &devname[0];
		DevLink_REMOTE.pParam = (UDINT) &param[0];
		status = DevLink_REMOTE.status;
		if(status != 65535){
			remoteDevlinkHandle = DevLink_REMOTE.handle;
			remoteDevName = devname;
			break;
		}
	}
	return status;
}

UINT fileIOClass::createLocalDevlink(std::string devname, std::string param){
	UINT status;
	while(1){
		DevLink_LOCAL.enable = 1;
		DevLink(&DevLink_LOCAL);
		DevLink_LOCAL.pDevice = (UDINT) &devname[0];
		DevLink_LOCAL.pParam = (UDINT) &param[0];
		status = DevLink_LOCAL.status;
		if(status != 65535){
			localDevlinkHandle = DevLink_LOCAL.handle;
			localDevName = devname;
			break;	
		}
	}
	return status;
}

UDINT fileIOClass::getLocalDevlinkHandle(){
	return localDevlinkHandle;
}

UDINT fileIOClass::getRemoteDevlinkHandle(){
	return remoteDevlinkHandle;
}

UINT fileIOClass::unlinkDevlink(UDINT DevlinkHandle){	//This function might violate maximum cycle time
	UINT status;
	while(1){
		DevLink_UNLINK.enable = 1;
		DevUnlink(&DevLink_UNLINK);
		DevLink_UNLINK.handle = DevlinkHandle;
		status = DevLink_UNLINK.status;
		if(status == 0){
			break;	
		}
	}
	return status;
}

//Open existing file
UINT fileIOClass::fileOpen(std::string filename, std::string devname){
	UINT status;
	while(1){	//Poll function-block until status flag != 65535
		FOpen.enable = 1;
		FOpen.pDevice 	= (UDINT) &devname[0];			//(UDINT) &local_dev_name;	//(UDINT) local_dev_name;
		FOpen.pFile     = (UDINT) &filename[0];
		FOpen.mode = FILE_RW;
		FileOpen(&FOpen);
		fileIdentity = FOpen.ident;
		//while(1){
		status = FOpen.status;
		if(status != 65535){
			break;	
		}
	}
	return status;
}

//Create new file
UINT fileIOClass::fileCreate(std::string filename, std::string devname){
	UINT status;
	while(1){	//Poll function-block until status flag != 65535
		FCreate.enable = 1;
		FCreate.pDevice = (UDINT) &devname[0]; 			//&local_dev_name;	//(UDINT) local_dev_name;
		FCreate.pFile   = (UDINT) &filename[0];
		FileCreate(&FCreate);
		fileIdentity = FCreate.ident;
		status = FCreate.status;
		if(status != 65535){
			break;	
		}
	}
	return status;
}

//Write data to file
UINT fileIOClass::fileWrite(std::string data){		//Problem - too few bytes written to file, only writes 4 bytes at the moment
	UINT status = 0;
	while(1){	//Poll function-block until status flag != 65535
		FWrite.enable = 1;
		FWrite.ident = fileIdentity;
		FWrite.offset = 0;
		FWrite.pSrc = (UDINT) &data[0];
		FWrite.len = sizeof (data);
		FileWrite(&FWrite);
		status = FWrite.status;
		if(status != 65535){
			break;	
		}
	}
	return status;
}

//Close file
UINT fileIOClass::fileClose(){
	UINT status = 0;
	while(1){	//Poll function-block until status flag != 65535
		FClose.enable = 1;
		FClose.ident = fileIdentity;
		FileClose(&FClose);
		status = FClose.status;
		if(status != 65535){
			break;	
		}
	}
	return status;
}

//Delete file
UINT fileIOClass::fileDelete(std::string filename, std::string devname){
	UINT status = 0;
	while(1){	//Poll function-block until status-flag != 65535
		FDelete.enable = 1;
		FDelete.pDevice = (UDINT) &devname[0];
		FDelete.pName 	= (UDINT) &filename[0];
		FileDelete(&FDelete);
		status = FDelete.status;
		if(status != 65535){
			break;	
		}
	}
	return status;
}

UINT fileIOClass::fileCopy(std::string srcfn, std::string desfn){
	return fileCopy(this->remoteDevName, srcfn, this->localDevName, desfn);
}

//Copy file
UINT fileIOClass::fileCopy(std::string srcdl, std::string srcfn, std::string desdl, std::string desfn){
	UINT status = 0;
	while(1){
		FCopy.enable = 1;
		FCopy.pSrcDev = (UDINT) &srcdl[0];
		FCopy.pSrc = (UDINT) &srcfn[0];
		FCopy.pDestDev = (UDINT) &desdl[0];
		FCopy.pDest = (UDINT)&desfn[0];
		FCopy.option = fiOVERWRITE;
		status = FCopy.status;
		if(status != 65535){
			break;	
		}
	}
	return status;
}

bool fileIOClass::createLocalFile(std::string filename, std::string devname){
	UINT status = 0;
	USINT step = 1;
	USINT error_level = 0;
	
	while(1){
		switch (step) {
			case 0:
				if(error_level == 1) setOutput1 = true;
				if(error_level == 2) setOutput2 = true;
				if(error_level == 3) setOutput3 = true;
				if(error_level == 4) setOutput4 = true;
				//Error step
				return false;
				break;
			case 1:
				status = fileOpen(filename, this->localDevName);
				if(status == 0){
					step = 3;
				} else if(status == 20708){
					step = 2;
				} else {
					error_level = 1;
					step = 0;
				}
				break;
			case 2:
				status = fileCreate(filename, this->localDevName);
				if(status == 0){
					step = 1;
				} else {
					error_level = 2;
					step = 0;
				}
				break;
			case 3:
				status = fileWrite("HelloWorld!");
				if(status == 0){
					step = 4;
				} else{
					error_level = 3;
					step = 0;
				}
				break;
			case 4:
				status = fileClose();
				if(status == 0){
					return true;
				} else{
					error_level = 4;
					step = 0;
				}
				break;
		}
	}
}

std::string fileIOClass::return_error_string(){
	return error_id_string;
}

fileIOClass::~fileIOClass(){
}

