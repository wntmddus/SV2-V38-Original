/**
 * \file
 *
 * \brief SAM toolkit demo parameter header file.
 *
 */

#ifndef PARAMETERS_H_INCLUDED
#define PARAMETERS_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include <define_1634HzFs.h>
#include "fatfs.h"

//algo files
#include <BLSProcess.h>
#include <EVSProcess.h>
#include <SLMCali.h>
#include <SLMProcess.h>
#include <VLMCali.h>

//#define TIMING_DEBUG

//#define SSC_BUFFER_SIZE		1200 //decimate = 4 -> 300*4 => 300-300-300-1200 for 0.1sec
//#define SamplFreqVibro	3000
#define SamplFreqSound	24000
//#define VIB_BUFFER_SIZE		160 // 160 for 0.1sec
//#define SamplFreqVibro	1600
//#define SamplFreqSound	12000

#define SamplFreqVibro	1634
/** Button. */
#define  F5   5 //menu
#define  BOT   3 //bottom
#define  F2   2 //ok
#define  UP   1 //up
#define  F6   6 //left
#define  F7   7 //right
#define  F8   8 //
#define  FREC   9 //rec
#define  FBL   10 //BL

/* TWI0 instance */
//typedef void *freertos_twihs0_if;

/* TWI1 instance */
//typedef void *freertos_twihs2_if;

/* --------EVENT----------- */

/** Geometric type describing a point or vector on screen, relative or absolute. */
struct win_point {
	uint32_t x;
	uint32_t y;
};

/** Valid pointer event types. */
enum win_pointer_event_type {
	/** One or more buttons have been pressed somewhere. */
	WIN_POINTER_PRESS,
	/** Pointer has moved, with or without buttons. */
	WIN_POINTER_MOVE,
	/** One or more buttons have been released somewhere. */
	WIN_POINTER_RELEASE,
};

/** Event data for all pointer events, e.g. touch screen or mouse. */
typedef struct win_pointer_event {
	/** Position of press/release, or destination of move. */
	struct win_point pos;
	/** True if position vector is relative. */
	bool is_relative;
	/** Absolute position of last event, set by window system. */
	struct win_point last_pos;
	/** "Enter" key pressed or ..... */
	uint8_t buttons;
	/** Type of this event. */
	enum win_pointer_event_type type;
} win_pointer_event_t;

typedef void (* win_event_handler_t)(win_pointer_event_t const * event);

/** Window attribute data. */
struct win_attributes {
	/** Event handler callback, or NULL. */
	win_event_handler_t event_handler;
	/** Custom window data, e.g. link to associated widget. */
	void *custom;
};


struct DateTime
{
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t weekday;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
	uint32_t lpyr;
};


//extern int read_time_date(uint32_t *hour1, uint32_t *minute1, uint32_t *second1,
//uint32_t *year1, uint32_t *month1, uint32_t *day1, uint32_t *weekday1, uint32_t *lpyr1);

extern struct win_attributes  win_attributes_root;
extern osMessageQueueId_t queue_pointer_eventHandle; //extern xQueueHandle queue_pointer_event;
extern osMessageQueueId_t ConnectionEventHandle;
extern osSemaphoreId_t fmc_write_notification_semaphoreHandle;
extern osSemaphoreId_t ad7124_notification_semaphoreHandle;
extern osSemaphoreId_t ads127l11_notification_semaphoreHandle;
extern osSemaphoreId_t battery_i2c_read_notification_semaphoreHandle;

/**
 * \brief Special window mode ID.
 */
typedef enum demo_special_mode_id {
	DEMO_MODE_EVS = 0,	
	DEMO_MODE_BLAST,
	DEMO_MODE_TRANSD,
	
	
	DEMO_MENU_MAIN=10, 
	
	DEMO_OPT_ANALYZ_EVS=20,
	DEMO_OPT_ANALYZ_BLAST,
	DEMO_TRIGGER_EVS,
	DEMO_TRIGGER_BLAST,
	DEMO_OPT_TRANSD,
	DEMO_OPT_RECORD,
	DEMO_OPT_FTP,
	DEMO_INPUTBOX,
	DEMO_SELECTBOX,
	DEMO_SEND_FILE,
	DEMO_OPT_COMMON,
	
	DEMO_LEQ_MODE = 30,	  //kt	
	
	
	DEMO_SPEC_MODE_MAX	//kt
}demo_special_mode_id;

extern void evs_win_KEY_handler(uint32_t key_pad); 
void app_widget_launch_evs(void);
void draw_evs_update(float val, _EVS_OUT evs_out);

extern void blast_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_blast(void);
void draw_blast_update(float val, _BLS_OUT bls_out);

extern void opt_an_evs_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_opt_an_evs(void);

extern void opt_an_bls_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_opt_an_bls(void);

extern void trigger_evs_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_trigger_evs(void);

extern void trigger_bls_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_trigger_bls(void);

extern void transd_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_transd(void);
void app_widget_launch_transd_back(bool save, float out_val);

extern void opt_rec_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_opt_rec(void);

extern void opt_ftp_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_opt_ftp(void);
void app_widget_launch_ftp_back(bool save, float out_val, const char* str);

extern void send_file_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_send_file(void);

extern void menu_main_win_KEY_handler(uint32_t key_pad); 
void app_widget_launch_menu_main(void);

extern void opt_common_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_opt_common(void);
void app_widget_launch_common_back(bool save, float out_val, const char* str);

extern void inputbox_KEY_handler(uint32_t key_pad);
void app_widget_launch_inputbox(demo_special_mode_id id, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* str, int max_pos_n, int type);

extern void selectbox_KEY_handler(uint32_t key_pad);
void app_widget_launch_selectbox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, float val);

extern void LEQ_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_leq(uint8_t);
//kt void draw_leq();



extern int draw_file_name;
void draw_rec_time(int sec);
void set_rec_button(void);
void refresh_system_time(void);

typedef struct
{
	int nSamplFreqVibro;	//1024
	int nBufSizeVibro;		//128
	int nSamplFreqSound;	//16384
	int nBufSizeSound;		//2048
	
	float fIntegrTime;		//"F"=0.100;  "S"=1.0f;		//was nIntegralTime   //22.05
	int   nFreqWeight;		//0-Z_WT;   1-A_WT			//was WeightChoice1
	int   nTimeWeight;		//"F" =0 (always)			// in algo SLMStruct->TimeWeight = F=0 (0.5s)  and S=1(1.0s)
	float fVibroIntegrTime; //"F"=0.100;  "S"=1.0f;		 //was fIntTime  - for EVS only   //22.05
	float fDbRefAcc;		//1.0f, 10.0f
//	bool  bCalcLmax;		//for Blast only: calculate Lmax&L10(for z-axis)
} opt_analyzing;

typedef struct  
{
	float	fAmplGain;
	float	fSenset;//mV/g   (for lib need m/ss)
	float   fNormRMS;//m/s2
	float   fNormFreq;
	float	fCalibr;	
} opt_transducer;

typedef struct
{
	float	fLevel;			//evs - db,  blast - mm/s
	float	fPostTrigTime;	//1,2,3,5,10s,1,5m,1h  timer in continuous and single modes
	int     nDelay;			//delay time( in sec)for next recording in continuous mode
	int		nUseSoundTrigger;   //0-no,  1-yes
	float   fLevelSoundTrigger;
	
	//float   fTimerManualSec;//timer in manual recording (evs,Blast): no,1,5,10m,1h 
	
	//int		nChan;		//=2
	//float	fPreTrigTime;	//=0.250f
	
} opt_trigger;

typedef struct
{
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
} rect;

typedef struct
{
	uint16_t ip[4];
	int port;
	char user_name[32];
	char password[32];
	char path[64];	
} opt_ftp_connect;

typedef struct
{
	uint16_t file_type;  //0-wav, 1-txt, 2-wav+txt
	int timer;  //0-not use, other values-in sec
	int nRecordMode;	//0-manual;  1-Single;  2- Cont
	
} opt_recording;

typedef struct
{
	char str_sernumber[32];
	char str_model[32];
	char str_operator[32];
} opt_common;

void set_special_mode_status( demo_special_mode_id id, uint32_t status );	//kt
uint32_t get_special_mode_status( demo_special_mode_id id );				//kt

#define DEMO_PARAMETERS_MAGIC 0x4c4d5441

/** This structure is used to save application parameters in Internal Flash */
typedef struct
{
	uint32_t magic ;	

	int nMeasMode;						//0-evs,  1-blast.
	int nFS_vib;
	opt_analyzing	opt_analyz[2];		//0-evs, 1-blast
	opt_trigger		opt_trig[2]	;		//0-evs, 1-blast
	opt_transducer	opt_transd[4];		//0,1,2 - for vibro channels;   3-for sound
	opt_ftp_connect	opt_ftp;
	opt_recording	opt_rec;				
	opt_common		opt_info;
			
	int32_t ser_numb;
	char ser_numb_const[7];
	
	char vers_sf[8];
} demo_param_t ;

extern demo_param_t g_demo_parameters;
extern demo_param_t g_demo_parameters1;

extern short flag_lcd_update; //-1: no update,  0-evs update, 1-bls update
extern short flag_arm_rec;   //0-arming mode, 1-rec mode  = it's reaction on FREC button press
extern short g_flagCalibr;   //1- start Calibration,  0-finished
extern short g_flagCalibrChan; //current channel for calibration , 0,1,2 - vibro, 3-sound  //kt25

uint32_t demo_parameters_initialize( void ) ;
uint32_t demo_parameters_commit_changes( void ) ;

//-----------------------  ALGO  --------------

typedef struct
{
	_SLM SLMStruct;
	//_LPSLM SLMStructPtr = &SLMStruct;
	
	_BLS BLSStruct;
	//_LPBLS BLSStructPtr = &BLSStruct;
	_BLS_OUT BLSOUTStruct;

	_EVS EVSStruct;
	//_LPEVS EVSStructPtr = &EVSStruct;
	_EVS_OUT EVSOUTStruct;	
	
	_SLMCALI SLMCaliStruct;
	//_LPSLMCALI SLMCaliStrucPtr = &SLMCaliStruct;

	_VLMCALI VLMCaliStruct;
	//_LPVLMCALI VLMCaliStrucPtr = &VLMCaliStruct;
} opt_algo;

extern opt_algo		g_algo;		//for communication with algo part

/* --------FILE----------- */
extern char send_file_name[32];
/*
extern FIL file_send;
extern FIL file_wave;
extern FIL file_wave3ch;

extern FRESULT create_header(FIL* file, short nChannels, int nSamplesPerSecond);
extern FRESULT update_header(FIL* file, short nChannels, DWORD size);
extern FIL file_txt;
extern FRESULT write_result(FIL* file, float time);
*/
extern FRESULT write_result(FIL* file, float time);
extern void DrawSD(void);


//extern int save_text_data_by_hold(void);
extern uint32_t mountsin;
extern uint32_t mounted;
extern char str_start_rec[24];

extern void power_off_mode(void);

inline portTickType get_elapsed_ticks(portTickType ticks_start, portTickType ticks_end)
{
	return (ticks_end >= ticks_start ? ticks_end - ticks_start : portMAX_DELAY - ticks_start + ticks_end + 1);
}

//extern int write_time_date(uint32_t hour1, uint32_t minute1, uint32_t second1,
//uint32_t year1, uint32_t month1, uint32_t day1, uint32_t weekday1);

#define MAX_LTEMES_BUF_SIZE	128
extern char ltestrBuffer[MAX_LTEMES_BUF_SIZE];
extern TaskHandle_t xlte_task;

#define POWERON_START 1
#define POWERON_FINISH 2
#define CPIN 3
#define CPIN_ERROR 4
#define CSQ 5
#define CGREG 7
#define COPS 9
#define CPSI 11
#define CFTPSSTART 13
#define CFTPSSINGLEIP 15
#define CFTPSLOGIN 17
#define CFTPSLOGIN_ERROR 18
#define FSCD_F 19
#define CFTPSPUTFILE 21
#define CFTPSLOGOUT 23
#define CFTPSSTOP 25
#define CFTPSCWD 27
#define CFTPSCWD_ERROR 28
#define F_READ_START 29
#define F_READ_FINISH 31
#define CFTRANRX_START 33
#define CFTRANRX_FINISH 35
#define CFTRANRX_ERROR 36
#define FSDEL 37
#define FSMEM 38
#define FSLS 39
#define CNMP 40
#define POWEROFF_START 61
#define POWEROFF_FINISH 62

extern uint32_t ltestate;
extern char tempf[64][128];

extern char tempftp[68];
extern uint32_t flag_ftp_update;
extern uint32_t flag_ftp_start;

extern char file_ftp[32];

extern uint32_t* send_buff;
extern void adc_start(void);
extern void adc_stop(void);



typedef  struct bq2588x {
	int vbus_volt;
	int vbat_volt;
	int vsys_volt;
	int ibus_curr;
	int ichg_curr;
	int die_temp;
	int ts_temp;
} bq2588x_t;

extern int twihs2state;
extern uint32_t bluescreen;
extern uint32_t dsp_start_event;

#define LTE_MAX_F_SIZE 91000

#endif /* PARAMETERS_H_INCLUDED */
