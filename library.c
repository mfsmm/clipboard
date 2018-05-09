#include "clipboard.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>


// gcc -Wall -o lib.o -c library.c  && gcc -o app_test.o app_teste.c socketlib.o library.c




/**
 *
 * @param clipboard_dir
 * @return  socket file descriptor on sucess / -1 on error
 */
int clipboard_connect(char * clipboard_dir){			//Must use clipboard_dir
	//Creating
	int sock ;
	if((sock = createSocket(AF_UNIX,SOCK_STREAM)) == -1){
		return -1;
	}
	//Connect
	if(UnixClientSocket(sock,SOCK_LOCAL_ADDR)==-1){
		return -1;
	}
	printf("\t[Clipboard Connect] connected successfully\n");
	return sock;
}

/**
 *
 * @param clipboard_id
 * @param region
 * @param buf
 * @param count
 * @return  numBytes sent on success / 0 on error
 */
int clipboard_copy(int clipboard_id, int region, void *buf, size_t count){
    //Check parameters:
    if(checkParams(clipboard_id,region) != 0){
        return  0;
    }
    //Send Data
    //printf("\t[Clipboard copy] buf = %s\n",buf);
	char * bytestream = malloc(sizeof(struct metaData));
	struct metaData info;
    size_t sentBytes = 0;
	info.region=region;
	info.action=0;
	info.msg_size=count;
	memcpy(bytestream,&info,sizeof(struct metaData));
	if(handShake(clipboard_id,bytestream,sizeof(struct metaData))!=0){
        return 0;
    }
	if((sentBytes = sendData(clipboard_id,count,buf))==-1){
		return 0;
	}
	printf("\t[Clipboard copy] Copy successful\n");
	return sentBytes;
}

/**
 *
 * @param clipboard_id
 * @param region
 * @param buf
 * @param count
 * @return  numBytes copyied to buf / 0 on error / -1 on empty region
 */
int clipboard_paste(int clipboard_id, int region, void *buf, size_t count){
    //Check parameters:
    if(checkParams(clipboard_id,region) != 0){
        return  0;
    }
	char * bytestream = malloc(sizeof(struct metaData));
	struct metaData info;
    void * received;

    //Request Data
	info.region=region;
	info.action=1;
	info.msg_size=-1;
	memcpy(bytestream,&info,sizeof(struct metaData));
	if(handShake(clipboard_id,bytestream,sizeof(struct metaData))!= 0){
        return 0;
    }

    //Get size of data
    if((bytestream = handleHandShake(clipboard_id,sizeof(struct metaData)))==NULL){
        return 0;
    }
    memcpy(&info,bytestream,sizeof(struct metaData));
    received = malloc(info.msg_size);                                   //Check of malloc is necessary, i dont think so

    //Get data
	if((received = receiveData(clipboard_id,info.msg_size))==NULL){
		return 0;
	}
    //Handle data
    if(info.msg_size > count ){
        printf("[clipboard_paste] Given buffer is not big enough\n");
        return count;                      //FIXME - devia copiar o que pode para o buffer. Talvez memcpy só com count
    } else if(info.msg_size > 0){
        //printf("\t[clipboard_paste] Paste successful\n");
        memcpy(buf,received,info.msg_size);
        return info.msg_size;
    } else{
        //printf("\t[clipboard_paste] Region was empty \n ");
        return -1;   //Buf unchanged, region was empty
    }
}

/**
 *
 * @param clipboard_id
 * @param region
 * @param buf
 * @param count
 * @return
 */
int clipboard_wait(int clipboard_id, int region, void *buf, size_t count){

}

/**
 *
 * @param clipboard_id
 */
void clipboard_close(int clipboard_id){

}

/***This should be in another file*/
int checkParams(int clipboard_id, int region){
    if(region<0 || region >REGION_SIZE){
        return -1;
    }
    if(clipboard_id < 0){       //FIXME - O que fazer mais?
        return -1;
    }
    return 0;
}
