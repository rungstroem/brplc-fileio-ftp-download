unsigned long bur_heap_size = 0xFFFF;
/***** Header files *****/
#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h> 
#endif


#include <bur/plctypes.h>
#include <fileIO.h>
#include <asZip.h>
#include <arProject.h>
#include <string>


//Variables
UINT status = 0;
USINT step = 0;
char LocalDevName[] = "FLASH3";
char RemoteDevName[] = "NET";
char RemoteFolder[] = "/softwareUpdate";
char LocalFolder[] = "/softwareUpdate";
char pipRemoteFolder[] = "/softwareUpdate/Default_X20CP04xx/pipconfig.xml";
char pipLocalFolder[] = "/softwareUpdate/Default_X20CP04xx/pipconfig.xml";
char pipServerVersion[32];
char pipLocalVersion[32];
char serverParam[] = "/SIP=192.168.1.10 /PROTOCOL=ftp /USER=ftpuser /PASSWORD=12345678";
char localParam[] = "/DEVICE=C:/";

_INIT void Init(void){
	step = 0;
}


_CYCLIC void Cyclic(void){
	//File transfer and installation state-machine
	switch (step){
		case 0:
			break;
		case 1:
			//Create remote devlink
			status = 1;
			DevLink_REMOTE.enable = 1;
			DevLink_REMOTE.pDevice = (UDINT) &RemoteDevName[0];			//(UDINT) &devname[0];
			DevLink_REMOTE.pParam = (UDINT) &serverParam[0];			//(UDINT) "/SIP=192.168.1.10 /PROTOCOL=ftp /USER=ftpuser /PASSWORD=12345678";				//(UDINT) &param[0];
	
			DevLink(&DevLink_REMOTE);
			status = DevLink_REMOTE.status;
			if(status == 0){
				step = 2;
				break;
			}
			break;
		case 2:
			//Create local devlink
			status = 1;
			DevLink_LOCAL.enable = 1;
			DevLink_LOCAL.pDevice = (UDINT) &LocalDevName[0];			//(UDINT) &devname[0];
			DevLink_LOCAL.pParam = 	(UDINT) &localParam[0];				//(UDINT) "/DEVICE=C:/";				//(UDINT) &param[0];
			
			DevLink(&DevLink_LOCAL);
			status = DevLink_LOCAL.status;
			if(status == 0){
				step = 3;
				break;
			}
			break;
		case 3:
			//Check installation file version on server
			status = 1;
			strcpy(FCheckServer.DeviceName , RemoteDevName);
			strcpy(FCheckServer.FilePath, pipRemoteFolder);
			FCheckServer.Execute = 1;
			
			ArProjectGetPackageInfo(&FCheckServer);
			if(FCheckServer.Done == true){
				strcpy(pipServerVersion, FCheckServer.ConfigurationVersion);
				step = 4;
				FCheckServer.Execute = 0;
				break;
			}
			break;
		case 4:
			//Check installation version on target
			status = 1;
			FCheckLocal.Execute = 1;
			ArProjectGetInfo(&FCheckLocal);
			if(FCheckLocal.Done == true){
				strcpy(pipLocalVersion, FCheckLocal.ConfigurationVersion);
				if(strcmp(pipServerVersion, pipLocalVersion) == 0){
					setOutput1 = true;
					step = 9;	
				} else{
					step = 5;	
				}
				FCheckLocal.Execute = 0;
				break;
			}
			break;
		case 5:
			//Copy installation files if server version is newer
			status = 1;
			
			//Copy an entire folder
			DCopy.enable = 1;
			DCopy.pSrcDev = (UDINT) &RemoteDevName[0];
			DCopy.pSrcDir = (UDINT) &RemoteFolder[0];
			DCopy.pDestDev = (UDINT) &LocalDevName[0];
			DCopy.pDestDir = (UDINT) &LocalFolder[0];
			DCopy.option = fiOVERWRITE;
			
			DirCopy(&DCopy);
			status = DCopy.status;
			if(status == 0){
				step = 8;
				DCopy.enable = 0;
				break;
			}
			
			/*
			//Copy a single file - "Used to copy a zipped archive"
			FCopy.enable = 1;
			FCopy.pSrcDev = (UDINT) &RemoteDevName[0];					//(UDINT) &srcdl[0];
			FCopy.pSrc = (UDINT) "softwareUpdate.tar.gz";	//(UDINT) &srcfn[0];
			FCopy.pDestDev = (UDINT) &LocalDevName[0];				//(UDINT) &desdl[0];
			FCopy.pDest = (UDINT) "softwareUpdate.tar.gz";	//(UDINT) &desfn[0];
			FCopy.option = fiOVERWRITE;

			FileCopy(&FCopy);
			status = FCopy.status;
			if(status == 0){
				setOutput3 = true;
				step = 6;
				FCopy.enable = 0;
				break;	
			}
			*/

			break;
		case 6:
			//Unzip files tar.gz files if update is downloaded as a zipped archive - maybe not needed
			status = 1;
			FUnzip.enable = 1;
			FUnzip.pArchiveDevice = (UDINT) &LocalDevName[0];				//(UDINT) &devname[0];
			FUnzip.pArchiveFile = (UDINT) "softwareUpdate.tar.gz";	//(UDINT) &filename[0];
			FUnzip.pOutDevice = (UDINT) &LocalDevName[0];					//(UDINT) &devname[0];
			FUnzip.pOutFolder = (UDINT) "/";

			zipExtract(&FUnzip);
			status = FUnzip.status;
			if(status == 0){
				setOutput4 = true;
				step = 7;
				FUnzip.enable = 0;
				break;	
			}
			break;
		case 7:
			//Deletes the tar.gz archive
			status = 1;
			FDelete.enable = 1;
			FDelete.pDevice = (UDINT) &LocalDevName[0];
			FDelete.pName = (UDINT) "softwareUpdate.tar.gz";
			
			FileDelete(&FDelete);
			status = FDelete.status;
			if(status == 0){
				setOutput5 = true;
				step = 8;
				FDelete.enable = 0;
				break;
			}
			break;
		case 8:
			//Install the update
			status = 1;
			strcpy(FInstall.DeviceName, LocalDevName);
			strcpy(FInstall.FilePath, pipLocalFolder);
			FInstall.Execute = 1;
			
			ArProjectInstallPackage(&FInstall);
			if(FInstall.Done == true){
				step = 9;
				FInstall.Execute = 0;
				break;
			}
			break;
		case 9:
			//Unlink remote devlink
			status = 1;
			DevLink_REMOTE.enable = 0;		//Disable function-block before unlinking
			
			DevLink_UNLINK.enable = 1;
			DevLink_UNLINK.handle = DevLink_REMOTE.handle;
			DevUnlink(&DevLink_UNLINK);
			status = DevLink_UNLINK.status;
			if(status == 0){
				DevLink_UNLINK.enable = 0;
				step = 10;
				break;
			}
			break;
		case 10:
			//Unlink local devlink
			status = 1;
			DevLink_LOCAL.enable = 0;		//Disable functionblock before unlinking
			
			DevLink_UNLINK.enable = 1;
			DevLink_UNLINK.handle = DevLink_LOCAL.handle;
			DevUnlink(&DevLink_UNLINK);
			status = DevLink_UNLINK.status;
			if(status == 0){
				setOutput6 = true;
				DevLink_UNLINK.enable = 0;
				step = 0;
				break;
			}
			break;
	}
}

