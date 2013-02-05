#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "g711.h"
#include "splib_quality.h"

using namespace std;

typedef struct WaveHeaderTag
{
    unsigned short formatTag;
    unsigned short channelNumber;
    unsigned int samplesPerSec;
    unsigned int avgBytesPerSec;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
    unsigned short cbSize;
} WaveHeader;

FILE * openWave(char * path, WaveHeader * pHeader, int * sampleCnt)
{
    FILE * fptr = NULL;
    char chuckName[5] = {'0'};
    int ret;
    unsigned int iNum;
    unsigned short sNum;

    if(pHeader==NULL) return NULL;

    fptr = fopen(path,"rb");
    if(fptr)
    {
        ret = fread(chuckName,1,4,fptr);
        if(ret!=4 || strcmp(chuckName,"RIFF")) return NULL;
        fseek(fptr,4,SEEK_CUR);
        ret = fread(chuckName,1,4,fptr);
        if(ret!=4 || strcmp(chuckName,"WAVE")) return NULL;
        ret = fread(chuckName,1,4,fptr);
        if(ret!=4 || strcmp(chuckName,"fmt ")) return NULL;
        int fmtSize;
        ret = fread(&fmtSize,4,1,fptr);
        if(ret!=1) return NULL;
        ret = fread(&sNum,2,1,fptr);
        if(ret!=1) return NULL;
        pHeader->formatTag = sNum;
        ret = fread(&sNum,2,1,fptr);
        if(ret!=1) return NULL;
        pHeader->channelNumber = sNum;
        ret = fread(&iNum,4,1,fptr);
        if(ret!=1) return NULL;
        pHeader->samplesPerSec = iNum;
        ret = fread(&iNum,4,1,fptr);
        if(ret!=1) return NULL;
        pHeader->avgBytesPerSec = iNum;
        ret = fread(&sNum,2,1,fptr);
        if(ret!=1) return NULL;
        pHeader->blockAlign = sNum;
        ret = fread(&sNum,2,1,fptr);
        if(ret!=1) return NULL;
        pHeader->bitsPerSample = sNum;
        if(pHeader->formatTag==18U)
        {
            ret = fread(&sNum,2,1,fptr);
            if(ret!=1) return NULL;
            pHeader->cbSize = sNum;
        }
        else pHeader->cbSize = 0;
        ret = fread(chuckName,1,4,fptr);
        if(ret!=4 || strcmp(chuckName,"data")) return NULL;
        int dataSize;
        ret = fread(&dataSize,4,1,fptr);
        if(ret!=1) return NULL;
        (*sampleCnt) = dataSize/pHeader->blockAlign;
    }
    return fptr;
}

void * SaveFile(char * path, WaveHeader pHeader, int sampleCnt, short * data)
{
    FILE * fptr=NULL;
    char chuckName[5]={'0'};
    unsigned int iNum;
    unsigned short sNum;

    fptr = fopen(path,"wb");
    if(fptr)
    {
        strcpy(chuckName,"RIFF");
        fwrite(chuckName,1,4,fptr);
        iNum=(unsigned int)(sampleCnt*2+36);
        fwrite(&iNum,4,1,fptr);
        strcpy(chuckName,"WAVE");
        fwrite(chuckName,1,4,fptr);
        strcpy(chuckName,"fmt ");
        fwrite(chuckName,1,4,fptr);
        if(pHeader.cbSize==0) iNum=16;
        else iNum=18;
        fwrite(&iNum,4,1,fptr);
        sNum=1;
        fwrite(&sNum,2,1,fptr);//formatTag
        sNum=1;
        fwrite(&sNum,2,1,fptr);//channelNum
        iNum=pHeader.samplesPerSec;
        fwrite(&iNum,4,1,fptr);//samplesPerSec
        iNum*=2;
        fwrite(&iNum,4,1,fptr);//avgBytesPerSec
        sNum=2;
        fwrite(&sNum,2,1,fptr);//blockAlign
        sNum=16;
        fwrite(&sNum,2,1,fptr);//bitsPerSample
        if(pHeader.cbSize!=0)//cbSize;
        {
            sNum=pHeader.cbSize;
            fwrite(&sNum,2,1,fptr);
        }
        strcpy(chuckName,"data");
        fwrite(chuckName,1,4,fptr);
        iNum=sampleCnt*2;
        fwrite(&iNum,4,1,fptr);
        fflush(fptr);
        fwrite(data,2,sampleCnt,fptr);
        fflush(fptr);
        fclose(fptr);
    }

    else printf("File %s could not write.",path);
}

void ShowHelp()
{
    printf("pWave [options] WaveFilePath\n");
    printf("options:\n");
    printf("   -ck: Check quality.\n");
    printf("   -ch time: cur head time ms.\n");
    printf("   -ct time: cut tail time ms.\n");
}

int main(int argc, char* argv[])
{
    //unsigned char aLawValue=0x8F;
    FILE * fptr=NULL;
    WaveHeader waveHeader;
    int sampleCnt;
    char * filePath = "";
    bool isCheck=false;
    int tailCutTime=0;
    int headCutTime=0;

    for(int i=1; i<argc; i++)
    {
        if(!strcmp(argv[i],"-ck")) isCheck=true;
        else if(!strcmp(argv[i],"-ct"))
        {
            i++;
            sscanf(argv[i],"%d",&tailCutTime);
        }
        else if(!strcmp(argv[i],"-ch"))
        {
            i++;
            sscanf(argv[i],"%d",&headCutTime);
        }
        else filePath=argv[i];
    }
    if(strlen(filePath)==0)
    {
        cout<<"No File"<<endl;;
        return -1;
    }
    fptr = openWave(filePath,&waveHeader,&sampleCnt);
    if(!fptr)
    {
        cout<<"File \""<<filePath<<"\" not open."<<endl;
        return -1;
    }

    if(waveHeader.channelNumber!=1)
    {
        cout<<"Not mono."<<endl;
        return -1;
    }
    short * data=NULL;
    if(waveHeader.formatTag==6)//A law
    {
        //printf("[%d]\n",sampleCnt);
        data=new short[sampleCnt];
        unsigned char aLawValue;
        for(int i=0; i<sampleCnt; i++)
        {
            fread(&aLawValue,1,1,fptr);
            data[i]=(short)alaw2linear(aLawValue);
        }
        //printf("[%d]\n",sampleCnt);
    }
    else if(waveHeader.formatTag==7)//u law
    {
        data=new short[sampleCnt];
        unsigned char uLawValue;
        for(int i=0; i<sampleCnt; i++)
        {
            fread(&uLawValue,2,1,fptr);
            data[i]=(short)ulaw2linear(uLawValue);
        }
    }
    else if(waveHeader.formatTag==1)//linear;
    {
        data=new short[sampleCnt];
        if(waveHeader.bitsPerSample==8)
        {
            char value;
            for(int i=0; i<sampleCnt; i++)
            {
                fread(&value,2,1,fptr);
                data[i]=(short)value;
            }
        }
        else if(waveHeader.bitsPerSample==16)
        {
            fread(data,2,sampleCnt,fptr);
        }
        else
        {
            delete[] data;
            cout<<"Unsupport bits per sample."<<endl;
            return -1;
        }
    }
    else//Unsupport
    {
        cout<<"Bad format."<<endl;
        return -1;
    }

    if(fptr) fclose(fptr);
    if(!data)
    {
        cout<<"#Bad data"<<endl;
        return 0;
    }
    int cutTailSampleNumber  = waveHeader.samplesPerSec*tailCutTime/1000;
    if(sampleCnt>cutTailSampleNumber) sampleCnt-=cutTailSampleNumber;//Remove the tailing sound.
    else cutTailSampleNumber=0;
    int cutHeadSampleNumber  = waveHeader.samplesPerSec*headCutTime/1000;
    if(sampleCnt>cutHeadSampleNumber) sampleCnt-=cutHeadSampleNumber;//Remove the heading sound.
    else cutHeadSampleNumber=0;
    if(isCheck)
    {
        SpLib_Quality Chk_Qlt(waveHeader.samplesPerSec, 16, 0.01);

        Chk_Qlt.feed(data+cutHeadSampleNumber, sampleCnt);
        if (!Chk_Qlt.isCLPOK()) cout<<"#CLPOK"<<endl;           //too loud.
        else if(!Chk_Qlt.isSNROK()) cout<<"#SNROK"<<endl;       //too quite
        else if(Chk_Qlt.isSilence()) cout<<"#isSilnece"<<endl;  //is Silence.
        else if(Chk_Qlt.isNoise()) cout<<"#isNoize"<<endl;      //is Noise.
        else
        {
            SaveFile(filePath,waveHeader,sampleCnt,data+cutHeadSampleNumber);
            cout<<"#OK"<<endl;
        }
    }
    else
    {
        SaveFile(filePath,waveHeader,sampleCnt,data+cutHeadSampleNumber);
        cout<<"#OK"<<endl;
    }
    delete[] data;
    //system("PAUSE");
    return 0;
}


