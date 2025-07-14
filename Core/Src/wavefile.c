/*
 * wavefile.c
 *
 * Created: 11/1/2023 6:13:30 PM
 *  Author: Tatsiana
 */ 


#include <string.h>
#include <stdio.h>
#include "parameters.h"
#include "printf.h"
#include "fatfs.h"

extern FRESULT write_result(FIL* file, float mtime);

char tem_str[1024];

typedef struct tWAVEFORMATEX {
	int16_t  wFormatTag;
	int16_t  nChannels;
	int32_t nSamplesPerSec;
	int32_t nAvgBytesPerSec;
	int16_t  nBlockAlign;
	int16_t  wBitsPerSample;
	int16_t  cbSize;
} WAVEFORMATEX;

typedef struct {
	int32_t           dwChannelMask;      /* which channels are  present in stream  */
	int32_t			SubFormat_Data1;	/* GUID    SubFormat;	00000001-0000-0010-80-00-00-aa-00-38-9b-71 */
	int16_t			SubFormat_Data2;
	int16_t			SubFormat_Data3;
	int8_t			SubFormat_Data4[8];
} EXTENSIBLE_FOR;

int8_t wave_header[68];

FRESULT create_header(FIL* file, short nChannels, int nSamplesPerSecond)
{
	FRESULT res=0;
	
	int pos=0;
	bool bExFormat=false;
	if (nChannels>2)
		bExFormat=true;
		
	WAVEFORMATEX file_format;
	EXTENSIBLE_FOR	file_FormatEx;
	
	//parent chunk	
	wave_header[0]='R';
	wave_header[1]='I';
	wave_header[2]='F';
	wave_header[3]='F';
	
	wave_header[4]=0;
	wave_header[5]=0;
	wave_header[6]=0;
	wave_header[7]=0;
	
	wave_header[8]='W';
	wave_header[9]='A';
	wave_header[10]='V';
	wave_header[11]='E';
	
	//child chunk
	wave_header[12]='f';
	wave_header[13]='m';
	wave_header[14]='t';
	wave_header[15]=' ';
	
	int sizeof_WAVEFORMATEX=18;  //function sizeof(WAVEFORMATEX) defines it incorrect (=20)  :(
	
	int len;
	if (!bExFormat)
	 len = sizeof_WAVEFORMATEX - sizeof(file_format.cbSize );
	else
	 len = sizeof_WAVEFORMATEX +sizeof(WORD) + sizeof(file_FormatEx);
	
	pos=16;
	memcpy(wave_header+pos,&len,4);
	pos=20;
	
	//format chunk	
	file_format.wFormatTag		=	1;   //WAVE_FORMAT_PCM;
	file_format.nChannels		=	nChannels;
	file_format.nSamplesPerSec	=	nSamplesPerSecond;
	file_format.wBitsPerSample	=	16;
	file_format.nAvgBytesPerSec	=	file_format.nSamplesPerSec*(file_format.wBitsPerSample/8)*file_format.nChannels;
	file_format.nBlockAlign		=	file_format.nChannels     *(file_format.wBitsPerSample/8);
	file_format.cbSize =0;
	
	if (bExFormat)
	{		
		file_format.wFormatTag		=	0xFFFE;	//WAVE_FORMAT_EXTENSIBLE;
		file_format.cbSize =22;
	}	
	memcpy(wave_header+pos,&file_format, sizeof_WAVEFORMATEX);
	
	//
	if (bExFormat)
	{
		pos=pos+sizeof_WAVEFORMATEX;

		switch(nChannels)
		{
			case 1:	file_FormatEx.dwChannelMask =0x1;	break;
			case 2:	file_FormatEx.dwChannelMask =0x1|0x2; break;
			case 3:	file_FormatEx.dwChannelMask =0x1|0x2|0x4; break;
			default: file_FormatEx.dwChannelMask =0x1|0x2|0x4; break;
		}
		file_FormatEx.SubFormat_Data1 = 0x1;
		file_FormatEx.SubFormat_Data2 = 0x0;
		file_FormatEx.SubFormat_Data3 = 0x10;
		file_FormatEx.SubFormat_Data4[0] = 0x80 ;
		file_FormatEx.SubFormat_Data4[1] = 0x0  ;
		file_FormatEx.SubFormat_Data4[2] = 0x0  ;
		file_FormatEx.SubFormat_Data4[3] = 0xAA ;
		file_FormatEx.SubFormat_Data4[4] = 0x0  ;
		file_FormatEx.SubFormat_Data4[5] = 0x38 ;
		file_FormatEx.SubFormat_Data4[6] = 0x9b ;
		file_FormatEx.SubFormat_Data4[7] = 0x71 ;
		
		memcpy(wave_header+pos,&file_format.wBitsPerSample, sizeof(file_format.wBitsPerSample));
		pos=pos+sizeof(file_format.wBitsPerSample);
		memcpy(wave_header+pos,&file_FormatEx, sizeof(file_FormatEx));
		pos=pos+sizeof(file_FormatEx);
	}
	else
		pos=pos+sizeof_WAVEFORMATEX-sizeof(file_format.cbSize);//not extend format
		
	//data chunk
	wave_header[pos]='d';
	wave_header[pos+1]='a';
	wave_header[pos+2]='t';
	wave_header[pos+3]='a';
	pos=pos+4;
	
	len=0;
	memcpy(wave_header+pos,&len,4);
	pos=pos+4;
	
	//save to file
	unsigned int bw=0;
	res = f_write(file, wave_header, pos, &bw);		/* Write the data to the file */
	return res;	
}

FRESULT update_header(FIL* file, short nChannels, DWORD size)
{
	FRESULT res;
	unsigned int bw=0;
	
	if (nChannels>2)
		size=size+68;
	else
		size=size+44;
	
	int pos=4;
	// write value = size-8
	f_lseek(file,pos);
	
	DWORD s=size-8;
	res = f_write(file, &s, 4, &bw);		/* Write the data to the file */
	if (res)
		return res;
		
		
	if (nChannels>2)
	{
		s=size-68;
		pos=68-4;
	}
	else
	{
		s=size-44;
		pos=44-4;
	}	
	//write value = s to position pos
	f_lseek(file,pos);
	res = f_write(file, &s, 4, &bw);		/* Write the data to the file */
	return res;
}

FRESULT write_result(FIL* file, float mtime)	//13.11  //nRecordMode
{
	FRESULT res;

	UINT bw;
	//=====================================================================	
	switch (g_demo_parameters.nMeasMode)
	{
		case 0:		snprintf_(tem_str, 256,		"	<EVS Report>\r\n"); break; //evs
		case 1:		snprintf_(tem_str, 256,		"	<Blast Report>\r\n"); break;//blast
	}
		
	snprintf_(tem_str, 256,
	"%s\r\n"\
	"Model:    %s \r\n"\
	"Serial:   %s \r\n"\
	"Operator: %s \r\n"\
	"Start:   %s\r\n", tem_str,g_demo_parameters.opt_info.str_model, g_demo_parameters.opt_info.str_sernumber, g_demo_parameters.opt_info.str_operator, str_start_rec);
	
	switch (g_demo_parameters.opt_rec.nRecordMode)
	{
		case 0: snprintf_(tem_str, 256,"%sRecType:	manual\r\n",tem_str); break;
		case 1: snprintf_(tem_str, 256,"%sRecType:	single\r\n",tem_str); break;
		case 2: snprintf_(tem_str, 256,"%sRecType:	contin\r\n",tem_str); break;
	}
	
	snprintf_(tem_str, 256,
	"%s"\
	"Record(sec):	%.3f\r\n"\
	"TriggerLevel:	%.3f\r\n"\
	"Integration:	%.3f\r\n"\
	"Sampling:	%d hz\r\n",
	tem_str,
	mtime,
	g_demo_parameters.opt_trig[g_demo_parameters.nMeasMode].fLevel,
	g_demo_parameters.opt_analyz[g_demo_parameters.nMeasMode].fIntegrTime,
	g_demo_parameters.nFS_vib);
	
	res = f_write(file, tem_str, strlen(tem_str), &bw);	
	memset(tem_str,0,256);
	
	snprintf_(tem_str, 256,
	"Software Vers:	%s\r\n"\
	"\r\n",	
	g_demo_parameters.vers_sf
	);
	
	switch (g_demo_parameters.nMeasMode)
	{
		case 0:  break; //evs
		case 1: //blast
			snprintf_(tem_str, 256,
			"%s"\
			"PeakVel:	%8.2f %8.2f %8.2f\r\n"\
			"PeakAcc:	%8.2f %8.2f %8.2f\r\n"\
			"PeakVect:	%8.2f\r\n"\
			"MaxSound: -\r\n"\
			"\r\n",
			tem_str,
			g_algo.BLSOUTStruct.VelocityPeakHold[0],    g_algo.BLSOUTStruct.VelocityPeakHold[1],          g_algo.BLSOUTStruct.VelocityPeakHold[2],
			g_algo.BLSOUTStruct.AccPeak[0],   g_algo.BLSOUTStruct.AccPeak[1],     g_algo.BLSOUTStruct.AccPeak[2],
			g_algo.BLSOUTStruct.PeakVectorSum
			); 
			break;
	}
	
	res = f_write(file, tem_str, strlen(tem_str), &bw);	

	//=====================================================================	
	memset(tem_str,0,256);
	snprintf_(tem_str, 256,
	"TRANSDUCERS: \r\n"\
	"Amp.Gain:   \t%8.2f %8.2f %8.2f\r\n"\
	"Sensitivity:\t%8.2f %8.2f %8.2f\r\n"\
	"Comp.Gain:  \t%8.2f %8.2f %8.2f\r\n"\
	"\r\n",
	g_demo_parameters.opt_transd[0].fAmplGain,	g_demo_parameters.opt_transd[1].fAmplGain,	g_demo_parameters.opt_transd[2].fAmplGain,
	g_demo_parameters.opt_transd[0].fSenset ,	g_demo_parameters.opt_transd[1].fSenset  ,	g_demo_parameters.opt_transd[2].fSenset  ,
	g_demo_parameters.opt_transd[0].fCalibr ,	g_demo_parameters.opt_transd[1].fCalibr  ,	g_demo_parameters.opt_transd[2].fCalibr  	
	);
	res = f_write(file, tem_str, strlen(tem_str), &bw);	
	
	//=====================================================================
	memset(tem_str,0,512);
	switch (g_demo_parameters.nMeasMode)
	{
		case 0:	//EVS
				snprintf_(tem_str, 512,	//pop
				"------------------ The Last Data  ---------------------\r\n"\
				"LeqIn:  %8.2f\r\n"\
				"LeqAv:  %8.2f\r\n"\
				"Lmax:   %8.2f\r\n"\
				"Lmin:   %8.2f\r\n\r\n"\
				"Lmax:   %8.2f %8.2f %8.2f\r\n"\
				"Lv(In): %8.2f %8.2f %8.2f\r\n"\
				"Lv(Av): %8.2f %8.2f %8.2f\r\n"\
				"L10:       -         -    %8.2f\r\n"
				"\r\n"\
				"Time(s) | Vib_X_Axis | Vib_Y_Axis | Vib_Z_Axis \r\n",
				g_algo.EVSOUTStruct.SoundOutput[0],
				g_algo.EVSOUTStruct.SoundOutput[1],
				g_algo.EVSOUTStruct.SoundOutput[2],
				g_algo.EVSOUTStruct.SoundOutput[3],
				g_algo.EVSOUTStruct.fLmax[0] , g_algo.EVSOUTStruct.fLmax[1] ,g_algo.EVSOUTStruct.fLmax[2] ,
				g_algo.EVSOUTStruct.fLveq[0] , g_algo.EVSOUTStruct.fLveq[1] ,g_algo.EVSOUTStruct.fLveq[2] , 
				g_algo.EVSOUTStruct.fLvAvg[0], g_algo.EVSOUTStruct.fLvAvg[1],g_algo.EVSOUTStruct.fLvAvg[2],
																			 g_algo.EVSOUTStruct.LnOut[3]		 //13.11 - only for z-channel
				);
				break;
			case 1:  //Blast
				snprintf_(tem_str, 512,  //pop
				"------------------ The Last Data  ---------------------\r\n"\
				"LeqIn:  %8.2f\r\n"\
				"LeqAv:  %8.2f\r\n"\
				"Lmax:   %8.2f\r\n"\
				"Lmin:   %8.2f\r\n\r\n"\
				"VelPeak(Inst): %8.2f %8.2f %8.2f\r\n"\
				"VelPeak(Hold): %8.2f %8.2f %8.2f\r\n"\
				"Lmax:          %8.2f %8.2f %8.2f\r\n"\
				"L10:           %8.2f %8.2f %8.2f\r\n"\
				"\r\n"\
				"Time(s) | Vib_X_Axis | Vib_Y_Axis | Vib_Z_Axis | VectorSum \r\n", 
				//g_algo.EVSOUTStruct.SoundOutput[0],
				//g_algo.EVSOUTStruct.SoundOutput[1],
				//g_algo.EVSOUTStruct.SoundOutput[2],
				//g_algo.EVSOUTStruct.SoundOutput[3],
				
					g_algo.BLSOUTStruct.SoundOutput[0],  //sameer: 02/07/2024 
					g_algo.BLSOUTStruct.SoundOutput[1],
					g_algo.BLSOUTStruct.SoundOutput[2],
					g_algo.BLSOUTStruct.SoundOutput[3],
				
				g_algo.BLSOUTStruct.VelocityPeakInst[0],g_algo.BLSOUTStruct.VelocityPeakInst[1],g_algo.BLSOUTStruct.VelocityPeakInst[2], 
				g_algo.BLSOUTStruct.VelocityPeakHold[0],g_algo.BLSOUTStruct.VelocityPeakHold[1],g_algo.BLSOUTStruct.VelocityPeakHold[2],//,	 //13.11
				g_algo.BLSOUTStruct.fLmaxBLS [0],		g_algo.BLSOUTStruct.fLmaxBLS [1],		g_algo.BLSOUTStruct.fLmaxBLS [2],
				g_algo.BLSOUTStruct.LnOutBLS [0],		g_algo.BLSOUTStruct.LnOutBLS [1],		g_algo.BLSOUTStruct.LnOutBLS [2]
				);
				break;
	}
	
	res = f_write(file, tem_str, strlen(tem_str), &bw);		/* Write the data to the file */
	
	//buffer
	memset(tem_str,0,1024);
	float d=1.0f/(float)g_demo_parameters.nFS_vib;  //pop
	float t_start=mtime-(float)BLS_BUFLEN/(float)g_demo_parameters.nFS_vib;  //pop
	int k=0, m=0, count=0;  //pop
	
	while (k < BLS_BUFLEN /*g_demo_parameters.nFS_vib*/)  //pop
	{
		switch(g_demo_parameters.nMeasMode)
		{
			case 0:
			m=snprintf_(tem_str+count, 256,  "%8.4f | %8.2f | %8.2f | %8.2f\r\n",        t_start + d*(k+1), g_algo.EVSStruct.EVSOut[0][k],g_algo.EVSStruct.EVSOut[1][k],g_algo.EVSStruct.EVSOut[2][k]);  //pop
			break;
			case 1:
			m=snprintf_(tem_str+count, 256,  "%8.4f | %8.2f | %8.2f | %8.2f | %8.2f\r\n",t_start + d*(k+1),g_algo.BLSStruct.VELOut[0][k],g_algo.BLSStruct.VELOut[1][k],g_algo.BLSStruct.VELOut[2][k], g_algo.BLSStruct.VectSum[k]);  //pop
			break;
		}
		k=k+1; //to make result file in 2 times smaller	//pop
		res = f_write(file, tem_str, m, &bw);		/* Write the data to the file */
		/*count=count+m;
		if (count>(1024-m))
		{
			res = f_write(file, tem_str, count, &bw);		// Write the data to the file /
			count=0;
			//memset(tem_str,0,1024);
		}*/
	}
	//res = f_write(file, tem_str, count, &bw);

	return res;
	//t_start + d*(k+1)
}
