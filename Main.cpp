unsigned long bur_heap_size = 0xFFFF;
/***** Header files *****/
#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h> 
#endif

//Needed libraries
#include <bur/plctypes.h>
#include <fileIO.h>
#include <asZip.h>
#include <arProject.h>
#include <string>

//Variables
UINT status = 0;
USINT step = 0;
UINT counter = 0;
char LocalDevName[] = "FLASH";
char RemoteDevName[] = "NET";
char RemoteFolder[] = "/softwareUpdate";
char LocalFolder[] = "/softwareUpdate";
char pipRemoteFolder[] = "/softwareUpdate/Default_X20CP04xx/pipconfig.xml";
char pipLocalFolder[] = "/softwareUpdate/Default_X20CP04xx/pipconfig.xml";
char pipServerVersion[32];
char pipLocalVersion[32];
char serverParam[] = "/SIP=152.115.180.90 /SNAME=otaupdate.kobots.dk /PROTOCOL=ftp /USER=amigo /PASSWORD=12345";
char localParam[] = "/DEVICE=C:/";

enum states{
	Idle,
	startProcess,
	CreateRemoteDev, 
	CreateLocalDev,
	InspectRemoteVersion,
	InspectLocalVersion,
	CopyInstallFiles,
	WaitForUser,
	InstallUpdate,
	DestroyRemoteDev,
	DestroyLocalDev
};

_INIT void Init(void){
	step = startProcess;
}

_CYCLIC void Cyclic(void){
	//File transfer and installation state-machine
	switch (step){
		case Idle:
			//Task running idle
			break;
		case startProcess:
			//For testing purpose
			if(start == true){
				step = CreateRemoteDev;
				break;
			}
			break;
		case CreateRemoteDev:
			//Create remote devlink
			status = 1;
			DevLink_REMOTE.enable = 1;
			DevLink_REMOTE.pDevice = (UDINT) &RemoteDevName[0];
			DevLink_REMOTE.pParam = (UDINT) &serverParam[0];
	
			DevLink(&DevLink_REMOTE);
			status = DevLink_REMOTE.status;
			if(status == 0){
				step = CreateLocalDev;
				break;
			} else if(status == 20736){
				//Network error!!!
				counter++;
				if(counter > 10){
					step = startProcess;
					DevLink_REMOTE.enable = 0;
					counter = 0;
				}
			} else if(status == 20730){
				//Error Device name already exists...
			}
			break;
		case CreateLocalDev:
			//Create local devlink
			status = 1;
			DevLink_LOCAL.enable = 1;
			DevLink_LOCAL.pDevice = (UDINT) &LocalDevName[0];
			DevLink_LOCAL.pParam = 	(UDINT) &localParam[0];
			
			DevLink(&DevLink_LOCAL);
			status = DevLink_LOCAL.status;
			if(status == 0){
				step = InspectRemoteVersion;
				break;
			}
			break;
		case InspectRemoteVersion:
			//Check installation file version on server
			status = 1;
			strcpy(FCheckServer.DeviceName , RemoteDevName);	//Test here
			strcpy(FCheckServer.FilePath, pipRemoteFolder);
			FCheckServer.Execute = 1;
			
			ArProjectGetPackageInfo(&FCheckServer);
			if(FCheckServer.Done == true){
				strcpy(pipServerVersion, FCheckServer.ConfigurationVersion);
				step = InspectLocalVersion;
				FCheckServer.Execute = 0;
				break;
			}
			break;
		case InspectLocalVersion:
			//Check installation version on target
			status = 1;
			FCheckLocal.Execute = 1;
			ArProjectGetInfo(&FCheckLocal);
			if(FCheckLocal.Done == true){
				strcpy(pipLocalVersion, FCheckLocal.ConfigurationVersion);
				if(strcmp(pipServerVersion, pipLocalVersion) == 0){
					step = DestroyRemoteDev;	
				} else{
					step = CopyInstallFiles;	
				}
				FCheckLocal.Execute = 0;
				break;
			}
			break;
		case CopyInstallFiles:
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
				step = WaitForUser;
				DCopy.enable = 0;
				break;
			}
			break;
		case WaitForUser:
			//Prompt user for update installation
			if(userInput == true){
				step = InstallUpdate;
			} else if(userInput == false){
				//waste cycles for now...	
			}
			break;
		case InstallUpdate:
			//Install the update
			status = 1;
			strcpy(FInstall.DeviceName, LocalDevName);
			strcpy(FInstall.FilePath, pipLocalFolder);
			FInstall.Execute = 1;
			
			ArProjectInstallPackage(&FInstall);
			if(FInstall.Done == true){
				step = DestroyRemoteDev;
				FInstall.Execute = 0;
				break;
			}
			break;
		case DestroyRemoteDev:
			//Unlink remote devlink
			status = 1;
			DevLink_REMOTE.enable = 0;		//Disable function-block before unlinking
			
			DevLink_UNLINK.enable = 1;
			DevLink_UNLINK.handle = DevLink_REMOTE.handle;
			DevUnlink(&DevLink_UNLINK);
			status = DevLink_UNLINK.status;
			if(status == 0){
				DevLink_UNLINK.enable = 0;
				step = DestroyLocalDev;
				break;
			} else if(status == 20731){
				//Invalid handle - device never got created
				DevLink_UNLINK.enable = 0;
				step = startProcess;
				break;
			} else if(status == 20798){
				//Device still in use
			}
			break;
		case DestroyLocalDev:
			//Unlink local devlink
			status = 1;
			DevLink_LOCAL.enable = 0;		//Disable functionblock before unlinking
			
			DevLink_UNLINK.enable = 1;
			DevLink_UNLINK.handle = DevLink_LOCAL.handle;
			DevUnlink(&DevLink_UNLINK);
			status = DevLink_UNLINK.status;
			if(status == 0){
				DevLink_UNLINK.enable = 0;
				step = Idle;
				break;
			} else if(status == 20731){
				//Invelid handle - device nevet got created
				DevLink_UNLINK.enable = 0;
				step = startProcess;
			} else if(status == 20798){
				//Device still in use
			}
			break;
	}
}

