/*##############################################################################
             Copyright(C) 2023-24, Svib Software technologies Pvt Ltd, INDIA.
                Svib tech Confidential Proprietary.
                          All rights reserved.
--------------------------------------------------------------------------------
          Project name :  Lightweight Environmental Analyzer (SV2)
--------------------------------------------------------------------------------
Filename    : define.h

Author(s)   : Sameer Ahamed P.S

Description : defines 

Context     :

Caution     : None

*****************************MODIFICATION HISTORY*******************************
                |      |          |
 Date			| Ver  | Author   | Description
----------------|------|----------|---------------------------------------------
                |      |          |
 1stJune2023	| 0.1  | sameer   | Fs=22050Hz for sound, FS=2205/CH for VIB
  6thJuly2023	| 0.2  | sameer   | Fs=2205Hz, output structures separated
 9thNov2023     | 0.3  | sameer   | Fs=2756Hz, calibration corrections
##############################################################################*/

#ifndef RTADEFINE_H
#define RTADEFINE_H

#include <math.h>
#include <stdint.h>

#define DSPTYPE float
#ifndef complex
#	define complex COMPLEX
#endif // complex

#ifndef PI
#	define PI (4.0*atan(1.0))
#endif /* PI */

#ifndef TWOPI
#	define TWOPI (2.0*PI)
#endif /* TWOPI*/

#ifndef pi
#	define pi PI
#endif /* pi */

#ifndef twopi
#	define twopi TWOPI
#endif /* twopi */

#ifndef SQR
#	define SQR(a) ((a)*(a))
#endif /*SQR*/

#ifndef QUAD
#	define QUAD(a) SQR(SQR(a))
#endif

#ifndef ABS
#	define ABS(a) ((a)>0?(a):(-a))
#endif /* ABS */

#ifndef MAX
#	define MAX(a, b) ((a)>(b)?(a):(b))
#endif /*MAX*/

#ifndef MIN
#	define MIN(a, b) ((a)>(b)?(b):(a))
#endif /* MIN */

// Round of Positive number
#ifndef ROUND_P
#	define ROUND_P(a) (((a)-floor((a)))>=0.5)?(floor((a))+1):(floor((a)))
#endif // ROUND_P

// Round of Negative number
#ifndef ROUND_N
#	define ROUND_N(a) ( (((a)-floor(a))>=0.5)? (floor(a)+1):(floor(a)) )
#endif //ROUND

// Round of all number
#ifndef ROUND
#	define ROUND(a) (((a)>=0)? ROUND_P((a)):ROUND_N((a)))
#endif //ROUND

#ifndef SIGN
#	define SIGN(a) ((a)>0?(1):(-1))
#endif //SIGN

#ifndef NORMDATA
#	define NORMDATA(a) ((a>0)?(a/SHRT_MAX):(-a/SHRT_MIN))
#endif //NORMDATA

#ifndef FREEMEM
#	define FREEMEM(a) if(a) {free(a); a=NULL;}
#endif //FREEMEM

#ifndef DELETEMEM
#	define DELETEMEM(a) { if(a) {delete a; a=NULL;} }
#endif //DELETEMEM

#ifndef DELETEARRAY
#	define DELETEARRAY(a) { if(a) {delete [] a; a=NULL;} }
#endif //DELETEARRAY

#ifndef MEMZERO
#	define MEMZERO(a, b) { if(a) { memset(a, 0, b); } }
#endif // MEMZERO

#ifndef ISINTEGER
#	define ISINTEGER(a) ( ((((double)(a))-floor(((double)(a))) > 0) ? false:true) )
#endif //ISINTEGER

typedef enum FilterType {
	LOWPASS, HIGHPASS, BANDPASS, BANDSTOP
} FILTERTYPE;

#ifndef LP
#	define LP	LOWPASS
#endif // LP

#ifndef HP
#	define HP	HIGHPASS
#endif // HP

#ifndef BP
#	define BP	BANDPASS
#endif // BP

#ifndef BS
#	define BS	BANDSTOP
#endif	//BS

#ifndef SCALEFACTOR
#	define SCALEFACTOR -1
#endif //SCALEFACTOR

#ifndef OVERLOADLEVEL
#	define OVERLOADLEVEL (int)(32767*4.95/5)
#endif // OVERLOADLEVEL


/* using STL for allocating buffer actively */
//using namespace std;
//typedef vector<DSPTYPE> BUFFER ;

typedef enum ProcessMode {
	SOUNDCALI,			 //0
	VIBCALI,			 //1
	BLSMODE,			 //2
	EVSMODE,			 //3
	PRESSURELEVELMODE,	 //4
} PROCESSMODE;

/*alex*/
//#define QUANTIZEMAXVALUE  				32768.0f  // 16 bit : -32768 ~ 32767
#define QUANTIZEMAXVALUE  				      16384.0f  // 
//#define QUANTIZEMAXVALUE					   524287.0f  // 20 bit : -524,288 ~ 524,287
#define INPUTMAXVOLTAGE						   (DSPTYPE)5.0f    // Input Voltage of Sound Input of iPAQ 5500 : 100mV
#define CALIBARION_ERROR_INPUT_TOO_SAMLL       0x0001   // -1    the input is too small   
#define CALIBARION_ERROR_SYSTEM_ERROR          0x0002   // -2    unknown error system error	
#define CALIBARION_ERROR_OVERLOAD              0x0004   // -4    input overload
#define CALIBARION_ERROR_CACULATION_ERROR      0x0008   // -8    calculation error
#define CALIBARION_ERROR_REFERENCE_FREQUENCY   0x0020;   // -32   the ref frequency maybe wrong 


#define SLM_MAX_DATA_FEED_LENGTH				2400  //kt  2205  //4096
//#define VLM_TMSG_SCALE_CONSTANT				0.15258789f            //  5000.0f / 32768.0f
///#define VLM_TMSG_SCALE_CONSTANT				0.019073486f           // 16.0f * 4.0f * 5000.0f / 16777216.0f (2^24)
//#define VLM_TMSG_SCALE_CONSTANT					1.4901e-04f           // 16.0f * 4.0f * 5000.0f / 2.1475e+09f (2^31)

//#define VLM_TMSG_SCALE_CONSTANT				1.164153218e-06f           // 2500.0f / 2.1475e+09f (2^31)
//#define VLM_TMSG_SCALE_CONSTANT				0.23283064365e-05f           // 5000.0f / 2.1475e+09f (2^31)

#define fUNDER_RANGE_LIMIT						(2.0f*10e-5f)
#define sOVER_RANGE_LIMIT						520000	  //32392            //32000;

#define SLM_CALI_SCALE_CONSTANT					0.35f       //Sameer: June, 2025
#define VLM_CALI_SCALE_CONSTANT				    0.011f       //Sameer: June, 2025


//#define SLM_BUFLEN								1200	//kt  2205	//Fixed for this project
//#define SLM_SAMPLERATE							12000   //kt  22050 //25600 //22050	//16384  //Fixed for this project
//#define GCONST									9.81    //Fixed for this project
//#define SLM_MAX_DATA_LEN						1200	//kt  2205  //4096
//#define FLT_MAX									140

//#define EVS_BUFLEN							300		//kt   275		//Fixed for this project
//#define EVS_SAMPLERATE						3000	//kt   2756	//25600 //22050	//16384  //Fixed for this project
//#define EVS_MAX_DATA_LEN						3000	//kt   2756  //4096 //here Sameer used 2205, I think it's mistake

//#define BLS_BUFLEN							300		//kt   275	//Fixed for this project
//#define BLS_SAMPLERATE						3000	//kt   2756 //25600 //22050	//16384  //Fixed for this project
//#define BLS_MAX_DATA_LEN						3000	//kt   2756  //4096
//#define VIB_MAX_DATA_LEN						3000	//kt   2756  //4096
//#define VLM_MAX_DATA_FEED_LENGTH				3000	//kt   2756  //4096

#define EVS_BUFLEN								164
#define EVS_SAMPLERATE							1634	
#define EVS_MAX_DATA_LEN						1634	

#define BLS_BUFLEN								164
#define BLS_SAMPLERATE							1634	
#define BLS_MAX_DATA_LEN						1634	
#define VIB_MAX_DATA_LEN						1634	
#define VLM_MAX_DATA_FEED_LENGTH				1634	




#define SLM_BUFLEN								2400	//kt  2205	//Fixed for this project
#define SLM_SAMPLERATE							24000   //kt  22050 //25600 //22050	//16384  //Fixed for this project
#define GCONST									9.81    //Fixed for this project
#define SLM_MAX_DATA_LEN						2400	//kt  2205  //4096
#define FLT_MAX									140

#endif //RTADEFINE_H
