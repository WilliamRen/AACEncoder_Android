/* DO NOT EDIT THIS FILE - it is machine generated */
/* Header for class com_hsae_audiorecorder_AACEncode */
/*
 * Class:     com_hsae_audiorecorder_AACEncoder
 * Signature: ()V
 */


/*CopyRight  hsae yangzhou RD dep.
*audthor turkeyzhu<turkeyzhu@gmail.com>
*create 201406
*/

#include "com_hsae_audiorecorder_aac.h"
#include "android/log.h"
#include <stdbool.h>

static const char *TAG="AAC";
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO,TAG,fmt,##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG,TAG,fmt,##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR,TAG,fmt,##args)

#define BOOL int







FILE						*infile, *outfile;
int 						t1, t2;
VO_AUDIO_CODECAPI			AudioAPI;
VO_MEM_OPERATOR 			moper;
VO_CODEC_INIT_USERDATA		useData;
VO_HANDLE					hCodec;
VO_CODECBUFFER				inData;
VO_CODECBUFFER				outData;
VO_AUDIO_OUTPUTINFO 		outInfo;
int 						firstWrite = 1;
int 						eofFile = 0;
int 						*info=(int*)inBuf;
int 						bytesLeft, nRead;
int 						EncoderdFrame = 0;
int 						total = 0;
int 						isOutput = 1;
int 						returnCode;
AACENC_PARAM				aacpara;
void						*handle;
void						*pfunc;
VOGETAUDIODECAPI			pGetAPI;
const char					*infileName = NULL;
const char					*outfileName = NULL;
char                        *memframe_l;
char                        *memframe_r;
int                         framecout_l=0;
int                         framecout_r=0;
BOOL                        exchage = 0;









int encodeInit(void)
{
	aacpara.adtsUsed = 1;
	aacpara.bitRate = 0;
	aacpara.nChannels = 2;
	aacpara.sampleRate = 11025;
	//aacpara.sampleRate=22050;

	//AAC_ENCODER *hAacEnc;

	// set memory operators;
		moper.Alloc = cmnMemAlloc;
		moper.Copy = cmnMemCopy;
		moper.Free = cmnMemFree;
		moper.Set = cmnMemSet;
		moper.Check = cmnMemCheck;
		useData.memflag = VO_IMF_USERMEMOPERATOR;
		useData.memData = (VO_PTR)(&moper);
		// open encoder so;
		handle = dlopen("libstagefright.so", RTLD_NOW);
		if(handle == 0)
		{
			printf("open so error......");
			LOGD("open so error----------------------");
			return -1;
		}
		// Get API;
		pfunc = dlsym(handle, "voGetAACEncAPI");
		if(pfunc == 0)
		{
			printf("open voGetAACEncAPI error......");
			return -1;
		}
		pGetAPI = (VOGETAUDIODECAPI)pfunc;
		returnCode	= pGetAPI(&AudioAPI);
		if(returnCode)
			return -1;


	if(aacpara.bitRate == 0)
	{
		int scale = 441;
		if(aacpara.sampleRate%8000 == 0)
			scale = 480;
		aacpara.bitRate = 640*aacpara.nChannels*aacpara.sampleRate/scale;
	}

	//init encode section
	returnCode = AudioAPI.Init(&hCodec, VO_AUDIO_CodingAAC, &useData);
	if(returnCode != VO_ERR_NONE)
	{
		printf("#### VOI_Error2:fail to initialize the Encoderr###\n");
		LOGD("#### VOI_Error2:fail to initialize the Encoderr###\n");
		return -1;
	}
	LOGD("get aacencode AudioAPI------------------------init over--%d--\n",hCodec);

	returnCode = AudioAPI.SetParam(hCodec, VO_PID_AAC_ENCPARAM, &aacpara);
	if(returnCode != VO_ERR_NONE)
	{
		LOGD("returnCode = AudioAPI.SetParam---------failed--------\n");
		return -1;
	}
	LOGD("get aacencode AudioAPI------------------------init over----\n");
	
	memframe_l= (void *)malloc(1280*10);
	return 0;
}


int ReadFile2Buf(FILE* infile,unsigned char* dest,int readSize)
{
	//LOGD("ReadFile2Buf------------0-------------%p-\n",infile);
	int readBytes = 0;
	readBytes = fread(dest, 1, readSize, infile);
	//LOGD("ReadFile2Buf------------1-------------%p-\n",infile);
	return readBytes;
}



int Readbuf2buf(unsigned char* src,unsigned char* dest,int readSize)
{
	LOGD("Readbuf2buf------------0-------------%p-\n",readSize);
	//int readBytes = 0;
	//readBytes = read(dest, src, readSize);
	memcpy(dest,src,readSize);
	
	//return readBytes;
	return readSize;
}





void buf_transfer()
{

	//bytesLeft = Readbuf2buf(pcmbuffer,inData.Buffer,/*READ_SIZE*/0x500);

	LOGD("_AACEncoder_encode----------------------------------1.11------%d--%p--%s",bytesLeft,inData.Buffer,inData.Buffer);

	do{
	returnCode = AudioAPI.SetInputData(hCodec,&inData);
		LOGD("_AACEncoder_encode----------------------------------1.2------%d",returnCode);
			do {
				outData.Buffer   = outBuf;
				outData.Length = 1024;
				returnCode = AudioAPI.GetOutputData(hCodec,&outData, &outInfo);
		LOGD("_AACEncoder_encode----------------------------------1.3------%d",returnCode);
				if (returnCode==VO_ERR_INPUT_BUFFER_SMALL)
					LOGD("AAC---GetoutputData------VO_ERR_INPUT_BUFFER_SMALL----------------------\n");
				if (returnCode==VO_ERR_OUTPUT_BUFFER_SMALL)
					LOGD("AAC---GetoutputData------VO_ERR_OUTPUT_BUFFER_SMALL----------------------\n");

				if(returnCode == 0)
					EncoderdFrame++;
				if(returnCode == VO_ERR_LICENSE_ERROR)
					break;
	#if VO_AAC_E_OUTPUT
				if (isOutput && returnCode == 0)
				{
					LOGD("_AACEncoder_encode----------------------------------1.4------%d",returnCode);
					fwrite(outData.Buffer, 1, outData.Length, outfile);
				}
	#endif
			} while(returnCode != (VO_ERR_INPUT_BUFFER_SMALL));

			if(returnCode == VO_ERR_LICENSE_ERROR)
				break;

			if (!eofFile) {
				LOGD("_AACEncoder_encode----------------------------------1.5------%d",returnCode);
//				nRead = Readbuf2buf(pcmbuffer, inBuf,/*READ_SIZE*/pcmsize);
				bytesLeft = nRead;
				inData.Buffer = inBuf;
				//if (eof(pcmbuffer)))
					eofFile = 1;
			}
			LOGD("_AACEncoder_encode----------------------------------1.6------%d",returnCode);
	}	 while (!eofFile && returnCode);
}



JNIEXPORT jint JNICALL Java_com_hsae_audiorecorder_AACEncoder_init(
		JNIEnv *env, jclass cls, jint inSamplerate, jint outChannel,
		jint outSamplerate, jint outBitrate, jint quality) 
{
	encodeInit();
	if(isOutput)
	{
		outfileName="/sdcard/my.acc";
		outfile = fopen(outfileName, "wb");
		if (!outfile) {
			printf("Open output file fail...");
			return -1;
		}
	}
}


JNIEXPORT jint JNICALL Java_com_hsae_audiorecorder_AACEncoder_encode(JNIEnv *env, jclass cls,jshortArray pcm_buf,jsize pcm_size, jbyteArray aac_buf) 
{
	//printf("_AACEncoder_encode--------------------------------------0-----");
	/*
	inData.Buffer = (*env)->GetByteArrayElements(env, pcm_buf, NULL);
	inData.Length = (*env)->GetArrayLength(env, pcm_buf);
	outData.Buffer = (*env)->GetByteArrayElements(env, aac_buf, NULL);
	outData.Length= (*env)->GetArrayLength(env, aac_buf);
	jint accbuf_size = outData.Length;
	LOGD("_AACEncoder_encode-----------0----pcm_size=%d, inData.Length=%d, inData.Buffer-is-%p,--hCodec-is-%p\n",pcm_size, inData.Length,inData.Buffer,hCodec);
//	LOGD("_AACEncoder_encode-------------------------------1--%d", sizeof(hCodec));
	returnCode = AudioAPI.SetInputData(hCodec,&inData);
	if(returnCode!=0)
		{
			LOGD("AAC---SetInputData----------------------------failed----");
		}
	returnCode = AudioAPI.GetOutputData(hCodec,&outData, &outInfo);
	if(returnCode!=0)
		{
		if (returnCode==VO_ERR_INPUT_BUFFER_SMALL)
			LOGD("AAC---GetoutputData------VO_ERR_INPUT_BUFFER_SMALL----------------------\n");
		if (returnCode==VO_ERR_OUTPUT_BUFFER_SMALL)
			LOGD("AAC---GetoutputData------VO_ERR_OUTPUT_BUFFER_SMALL----------------------\n");
		LOGD("AAC---GetoutputData----------------------------failed----%d\n",returnCode);
		}
	//LOGD("_AACEncoder_encode--------------------------------------3------ pcm_size=%d, outData.Length=%d, sizeof()=%d*, [NN] test=%d", pcm_size, outData.Length, sizeof(outData), test);
	(*env)->ReleaseShortArrayElements(env, pcm_buf, inData.Buffer, 0);
	(*env)->ReleaseByteArrayElements(env, aac_buf,outData.Buffer, 0);
	LOGD("_AACEncoder_encode----------------------------------3.1------%d",accbuf_size);
	return accbuf_size;
*/
	eofFile=0;
	inData.Buffer = inBuf;
	//memset(&inData, 0, sizeof(VO_CODECBUFFER));
	char* pcmbuffer = (*env)->GetByteArrayElements(env, aac_buf, NULL);
	jint pcmsize=(*env)->GetArrayLength(env, pcm_buf);
	LOGD("_AACEncoder_encode----------------------------------1.1------%d--%d",pcmsize,framecout_l);
	
	//memframe_l[framecout_l]=(char*)(*env)->GetByteArrayElements(env, aac_buf, NULL);
	memcpy(memframe_l+framecout_l*pcmsize,pcmbuffer,pcmsize);
	framecout_l++;
	if(framecout_l==8){
		framecout_l=0;
		exchage=1;
		//inData.Buffer=memframe_l;
		memcpy(inData.Buffer,memframe_l,1280*8);
		returnCode = AudioAPI.SetInputData(hCodec,&inData);
			if(returnCode!=0)
				{
					LOGD("AAC---SetInputData----------------------------failed----");
				}
			returnCode = AudioAPI.GetOutputData(hCodec,&outData, &outInfo);
			if(returnCode!=0)
				{
				if (returnCode==VO_ERR_INPUT_BUFFER_SMALL)
					LOGD("AAC---GetoutputData------VO_ERR_INPUT_BUFFER_SMALL----------------------\n");
				if (returnCode==VO_ERR_OUTPUT_BUFFER_SMALL)
					LOGD("AAC---GetoutputData------VO_ERR_OUTPUT_BUFFER_SMALL----------------------\n");
				LOGD("AAC---GetoutputData----------------------------failed----%d\n",returnCode);
				}

	}

	//buf_transfer();

}

JNIEXPORT jint JNICALL Java_com_hsae_audiorecorder_AACEncoder_uninit()
{
	returnCode = AudioAPI.Uninit(hCodec);
	return returnCode;
}
/*
JNIEXPORT jint JNICALL Java_com_hsae_audiorecorder_SimpleLame_flush(
		JNIEnv *env, jclass cls, jbyteArray mp3buf) {
	const jsize mp3buf_size = (*env)->GetArrayLength(env, mp3buf);
	jbyte* j_mp3buf = (*env)->GetByteArrayElements(env, mp3buf, NULL);

	int result = lame_encode_flush(glf, j_mp3buf, mp3buf_size);

	(*env)->ReleaseByteArrayElements(env, mp3buf, j_mp3buf, 0);

	return result;
}
JNIEXPORT void JNICALL Java_com_hsae_audiorecorder_SimpleLame_close(
		JNIEnv *env, jclass cls) {
	lame_close(glf);
	glf = NULL;
}
*/
