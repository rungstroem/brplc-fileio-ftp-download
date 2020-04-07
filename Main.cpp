unsigned long bur_heap_size = 0xFFFF;
/***** Header files *****/
#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h> 
#endif

#include <bur/plctypes.h>
#include "fileIOClass.h"
//'DEVICE="C:/softwareUpdate"'

USINT step = 0;
int code = 1;
_INIT void Init(void){
	step = 0;
}
 fileIOClass test;

/***** Cyclic part *****/
_CYCLIC void Cyclic(void){
	switch (step){	//File operation statemachine - always create device link first!
		case 0:
			//Idle state
			break;
		
		case 1:
			if(test.createLocalDevlink("FLASH3", "/DEVICE=C:/") == 0){
				//setOutput5 = true;
				step = 2;
			}
			break;
		
		case 2:
			if(test.createRemoteDevlink("NET", "/SIP=192.168.1.10 /PROTOCOL=ftp /USER=ftpuser /PASSWORD=12345678") == 0){
				//setOutput6 = true;
				step = 5;
			}
			break;
		
		case 3:
			if(test.createLocalFile("testfile.txt", "FLASH")){
				setOutput6 = true;	
			}
			step = 0;
			break;
		
		case 4: 
			if(test.fileDelete("testfile4.txt", "FLASH") == 0){
				setOutput6 = true;	
			}
			step = 0;
			break;
		
		case 5:
			if(test.fileCopy("arnbc.xml", "arnbc.xml") == 0){
				setOutput5 = true;
			}
			step = 0;
			break;
		
		case 6:
			if(test.unlinkDevlink(test.getLocalDevlinkHandle()) == 0){
				setOutput5 = false;
				setOutput6 = false;
			} else{
				setOutput1 = true;
				setOutput3 = true;
				setOutput6 = true;
			}
			step = 0;
			break;
		
		case 7:
			if(test.unlinkDevlink(test.getRemoteDevlinkHandle()) == 0){
				setOutput5 = true;
				setOutput6 = false;
			} else{
				setOutput2 = true;
				setOutput4 = true;
				setOutput5 = true;					
			}
			step = 0;
			break;
	}	
}

