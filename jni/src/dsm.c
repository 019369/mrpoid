#include <stdio.h>
#include <android/log.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <linux/time.h>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>
#include <asm-generic/fcntl.h>
#include <dirent.h>
#include <stdint.h>
#include <zlib.h>


#include "mrporting.h"
#include "FileLib.h"
#include "dsm.h"
#include "mr_helper.h"
#include "Engine.h"


extern int gbToUCS2BE(unsigned char *gbCode, unsigned char *unicode, int bufSize);

int showApiLog = TRUE; 


/////////////// ��C�� ///////////////////////
void *mr_malloc(uint32 len)
{
	return malloc(len);
}

void mr_free(void* p, uint32 len)
{
	free(p);
}

void *mr_realloc(void *p, uint32 oldlen, uint32 newlen)
{
	return realloc(p, newlen);
}

void *mr_memcpy(void *dst, const void *src, int len)
{
	return memcpy(dst, src, (size_t) len);
}

void *mr_memmove(void *dst, const void *src, int len)
{
	return memmove(dst, src, (size_t) len);
}

char *mr_strcpy(char *dst, const char *src)
{
	return strcpy(dst, src);
}

char *mr_strncpy(char *dst, const char *src, int len)
{
	return strncpy(dst, src, (size_t) len);
}

char *mr_strcat(char *dst, const char *src)
{
	return strcat(dst, src);
}

char *mr_strncat(char *dst, const char *src, int len)
{
	return strncat(dst, src, (size_t) len);
}

int mr_memcmp(const void *dst, const void *src, int len)
{
	return memcmp(dst, src, (size_t) len);
}

int mr_strcmp(const char *dst, const char *src)
{
	return strcmp(dst, src);
}

int mr_strncmp(const char *dst, const char *src, int len)
{
	return strncmp(dst, src, (size_t) len);
}

int mr_strcoll(const char *dst, const char *src)
{
	return strcoll(dst, src);
}

void* mr_memchr(const void *s, int c, int len)
{
	return memchr(s, c, (size_t) len);
}

void* mr_memset(void *s, int c, int len)
{
	return memset(s, c, (size_t) len);
}

int mr_strlen(const char *s)
{
	return strlen(s);
}

char *mr_strstr(const char *s1, const char *s2)
{
	return strstr(s1, s2);
}

int mr_sprintf(char *buf, const char *fmt, ...)
{
	__va_list vars;
	int ret;

	va_start(vars, fmt);
	ret = vsprintf(buf, fmt, vars);
	va_end(vars);

	return ret;
}

int mr_atoi(const char *s)
{
	return atoi(s);
}

unsigned long mr_strtoul(const char *nptr, char **endptr, int base)
{
	return strtoul(nptr, endptr, base);
}

void mr_sand(uint32 seed)
{
	return srand(seed);
}

int mr_rand(void)
{
	return rand();
}

void mr_printf(const char *format, ...)
{
	__va_list params;

	va_start(params, format);
	__android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, format, params);
	va_end(params);
}

static void sigroutine(int signo)
{
	switch (signo)
	{
	case SIGALRM:
		if(showApiLog) LOGI("timer out");
		mr_timer();
		break;
	}
}

/****************************************************************************
 ������:int32 mr_timerStart(uint16 t)
 ��  ��:����dsm��ʱ��
 ��  ��:t:��ʱ�����ʱ��(ms)
 ��  ��:MR_SUCCESS,MR_FAILED
 ****************************************************************************/
int32 mr_timerStart(uint16 t)
{
	struct itimerval tick;

	/*��setitimer()��ִ�е�timerʱ�䵽�˻����SIGALRM signal��
	  ��signal()��Ҫִ�е� function ָ����SIGALRM��*/
	signal(SIGALRM, sigroutine);
	
	memset(&tick, 0, sizeof(tick));
	//itimerval.it_value�趨��һ��ִ��function���ӳٵ�����
	//tick.it_value.tv_sec = t/1000;
	tick.it_value.tv_usec = t;

	//����2������ ���� ��һ��ִ�к�ÿ�������ִ�У�����Ҫ�ظ���������
	//tick.it_interval.tv_sec = 1; //��ʱ��������ÿ��1�뽫ִ����Ӧ�ĺ���
	//tick.it_interval.tv_usec = t;

	//ITIMER_REAL����ʾ��real-time��ʽ����timer����timeoutʱ���ͳ�SIGALRM signal
	if (setitimer(ITIMER_REAL, &tick, NULL) == -1){
		LOGE("setitimer err!");
		return MR_FAILED;
	}

	if(showApiLog) LOGI("mr_timerStart(t:%d)", t);

	return MR_SUCCESS;
}

/*ֹͣ��ʱ����*/
int32 mr_timerStop(void)
{
	return MR_FAILED;
}

/*ȡ��ʱ�䣬��λms*/
uint32 mr_getTime(void)
{
	struct timeval t;
	int ret = gettimeofday(&t, NULL );

	return (ret == 0 ? t.tv_usec : 0);
}

/*��ȡϵͳ����ʱ�䡣*/
int32 mr_getDatetime(mr_datetime* datetime)
{
	struct tm *time;
	time_t tt = 0;

	if (!datetime)
		return MR_FAILED;

	time = localtime(&tt);
	datetime->year = time->tm_year;
	datetime->month = time->tm_mon;
	datetime->day = time->tm_mday;
	datetime->hour = time->tm_hour;
	datetime->minute = time->tm_min;
	datetime->second = time->tm_sec;

	return MR_SUCCESS;
}

/*ȡ���ֻ������Ϣ��*/
int32 mr_getUserInfo(mr_userinfo* info)
{
	if (!info)
		return MR_FAILED;

	memset(info, 0, sizeof(mr_userinfo));
	memcpy(info->IMEI, "\x1\x2\x3\x4\x5", 5);
	memcpy(info->IMSI, "\x2\x3\x4\x5", 5);
	strcpy(info->manufactory, "mrpej");
	memcpy(info->spare, "E�翪���Ŷ�", 11);
	strcpy(info->type, "android");
	info->ver = 2012;

	return MR_SUCCESS;
}

/*����˯�ߣ���λms*/
int32 mr_sleep(uint32 ms)
{
	sleep(ms);

	return MR_SUCCESS;
}

/*ƽ̨��չ�ӿ�*/
int32 mr_plat(int32 code, int32 param)
{
	return MR_FAILED;
}

/*��ǿ��ƽ̨��չ�ӿ�*/
int32 mr_platEx(int32 code, uint8* input, int32 input_len, uint8** output, int32* output_len, MR_PLAT_EX_CB *cb)
{
	switch (code)
	{
	case 0:
		break;

	}

	return MR_FAILED;
}

///////////////////////// �ļ������ӿ� //////////////////////////////////////
#define DSM_ROOT_PATH  "mythroad"

static char dsmWorkPath[DSM_MAX_FILE_LEN + 1] = DSM_ROOT_PATH; /*·������gb ����*/
static char dsmLaunchPath[DSM_MAX_FILE_LEN + 1] = DSM_ROOT_PATH; /*·������gb ����*/
static uint8 dsmLaunchDrv = 'c'; /*ÿ��Ӧ��Ĭ�ϵ��̷��������ǲ�һ����*/
static uint8 dsmWorkDrv = 'c';

static void SetDsmWorkDrv(U8 drv)
{
	dsmWorkDrv = drv;
}

U8 GetDsmWorkDrv(void)
{
	return dsmWorkDrv;
}

static void SetDsmWorkPath(char *path)
{
	memcpy(dsmWorkPath, path, strlen(path) + 1);
}

char *GetDsmWorkPath(void)
{
	return dsmWorkPath;
}

/****************************************************************************
 ������:static void dsmRestoreRootDir(void)
 ��  ��:����VM��Ŀ¼
 ��  ��:��
 ��  ��:��
 ****************************************************************************/
void dsmRestoreRootDir(void)
{
	dsmWorkDrv = 'c';
	memcpy(dsmWorkPath, DSM_ROOT_PATH, strlen(DSM_ROOT_PATH) + 1);
}

/****************************************************************************
 ������:static void dsmToLaunchDir(void)
 ��  ��:������·�����ص�������ʱ���·��
 ��  ��:��
 ��  ��:��
 ****************************************************************************/
static void dsmToLaunchDir(void)
{
	dsmWorkDrv = dsmLaunchDrv;
	memcpy(dsmWorkPath, dsmLaunchPath, strlen(dsmLaunchPath) + 1);
}

/****************************************************************************
 ������:char* get_filename(char* outputbuf,const char *filename)
 ��  ��:�����·�����ļ����ӳɾ���·����
 ��  ��:filename:���·�����ļ���
 outputbuf:ת���õľ���·���ļ���(outputbuf�Ĵ�СҪ���ڵ���DSM_MAX_FILE_LEN * ENCODING_LENGTH)
 ��  ��:����·�����ļ���
 ****************************************************************************/
char* get_filename(char * outputbuf, const char *filename)
{
	char *p = outputbuf;

	if (strlen((char *) GetDsmWorkPath()) == 1) //��Ŀ¼ /
		p += sprintf(p, "/mnt/sdcard");
	else
		p += sprintf(p, "/mnt/sdcard/%s", GetDsmWorkPath());

	if (strlen(filename) > 0)
		p += sprintf(p, "/%s", filename);

	//kal_prompt_trace(MOD_MMI, "outputbuf = %s", outputbuf);

	return outputbuf;
}
/****************************************************************************
 ������:MR_FILE_HANDLE mr_open(const char* filename,  uint32 mode)
 ��  ��:��һ���ļ�
 ��  ��:filename:�ļ���
 mode:�򿪷�ʽ
 ��  ��:�ļ����
 ****************************************************************************/ ///
MR_FILE_HANDLE mr_open(const char* filename, uint32 mode)
{
	int f;
	int new_mode = 0;
	char* fullpathname[DSM_MAX_FILE_LEN] = { 0 };

	if (mode & MR_FILE_RDONLY)
		new_mode = O_RDONLY;
	if (mode & MR_FILE_WRONLY)
		new_mode = O_WRONLY;
	if (mode & MR_FILE_RDWR)
		new_mode = O_RDWR;
	if (mode & MR_FILE_CREATE)
		new_mode |= O_CREAT;
	//if(mode & MR_FILE_COMMITTED)
	//	new_mode |= FS_COMMITTED;
	//if(mode & MR_FILE_SHARD)
	//	new_mode  |= FS_OPEN_SHARED;

	//kal_prompt_trace(MOD_MMI, "------mr_open");

	f = open(get_filename((char *) fullpathname, filename), new_mode);
	if (f < 0)
	{
		LOGE("mr_open fail.");
		return MR_FAILED;
	}

	return (MR_FILE_HANDLE) f;
}

/****************************************************************************
 ������:int32 mr_close(MR_FILE_HANDLE f)
 ��  ��:�ر�һ���ļ�
 ��  ��:f:Ҫ�رյ��ļ��þ��
 ��  ��:NR_SUCCESS,MR_FAILED
 ****************************************************************************/
int32 mr_close(MR_FILE_HANDLE f)
{
	int ret;

	ret = close(f);

	if (ret == -1)
	{
		LOGE("mr_close fail.");
		return MR_FAILED;
	}

	return MR_SUCCESS;
}

/****************************************************************************
 ������:int32 mr_read(MR_FILE_HANDLE f,void *p,uint32 l)
 ��  ��:��ȡ�ļ��е�����
 ��  ��:f:Ҫ�����ļ��þ��
 p:�����ָ��
 l:����ô�С
 ��  ��:
 ****************************************************************************/
int32 mr_read(MR_FILE_HANDLE f, void *p, uint32 l)
{
	size_t readnum;

	readnum = read(f, p, (size_t) l);

	if (readnum < 0)
	{
		LOGE("mr_read fail.");
		return MR_FAILED;
	}

	return (int32) readnum;
}

/****************************************************************************
 ������:int32 mr_write(MR_FILE_HANDLE f,void *p,uint32 l)
 ��  ��:��һ���ļ���д������
 ��  ��:f:Ҫд����ļ��þ��
 p:�����ָ��
 l:Ҫд�����ݵô�С
 ��  ��:
 ****************************************************************************/
int32 mr_write(MR_FILE_HANDLE f, void *p, uint32 l)
{
	size_t writenum = 0;

	writenum = write(f, p, (size_t) l);

	if (writenum < 0)
	{
		LOGE("mr_write fail.");
		return MR_FAILED;
	}

	return writenum;
}

/****************************************************************************
 ������:int32 mr_seek(MR_FILE_HANDLE f, int32 pos, int method)
 ��  ��:ƫ���ļ���дָ��
 ��  ��:f     :�ļ����
 pos   :Ҫƫ�Ƶ�����
 method:ƫ�������λ��
 ��  ��:MR_SUCCESS,MR_FAILED
 ****************************************************************************/
int32 mr_seek(MR_FILE_HANDLE f, int32 pos, int method)
{
	off_t ret;

	ret = lseek(f, (off_t) pos, method);

	if (ret < 0)
		return MR_FAILED;
	else
		return MR_SUCCESS;
}

/****************************************************************************
 ������:int32 mr_info(const char* filename)
 ��  ��:�õ�һ���ļ���Ϣ
 ��  ��:filename
 ��  ��:���ļ�:MR_IS_FILE
 ��Ŀ¼:MR_IS_DIR
 ��Ч:  MR_IS_INVALID
 ****************************************************************************/
int32 mr_info(const char* filename)
{
	char fullpathname[DSM_MAX_FILE_LEN] = { 0 };
	struct stat s1;
	int ret;

	//���� 0 �ɹ�
	ret = stat(get_filename(fullpathname, filename), &s1);

	if (ret != 0)
		return MR_IS_INVALID;

	if (s1.st_mode & S_IFDIR)
		return MR_IS_DIR;
	else if (s1.st_mode & S_IFREG)
		return MR_IS_FILE;
	else
		return MR_IS_INVALID;
}

/****************************************************************************
 ������:int32 mr_remove(const char* filename)
 ��  ��:ɾ��һ���ļ�
 ��  ��:filename:Ҫ��ɾ�����ļ����ļ���
 ��  ��:MR_SUCCESS,MR_FAILED
 ****************************************************************************/
int32 mr_remove(const char* filename)
{
	char fullpathname[DSM_MAX_FILE_LEN] = { 0 };
	int ret;

	ret = remove(get_filename(fullpathname, filename));

	if (ret == 0)
		return MR_SUCCESS;
	else
		return MR_FAILED;
}

/****************************************************************************
 ������:int32 mr_rename(const char* oldname, const char* newname)
 ��  ��:��һ���ļ�����������
 ��  ��:oldname:ԭ�ļ���
 newname:���ļ���
 ��  ��:MR_SUCCESS,MR_FAILED
 ****************************************************************************/
int32 mr_rename(const char* oldname, const char* newname)
{
	char fullpathname_1[DSM_MAX_FILE_LEN] = { 0 };
	char fullpathname_2[DSM_MAX_FILE_LEN] = { 0 };
	int ret;

	ret = rename(get_filename(fullpathname_1, oldname), get_filename(fullpathname_2, newname));

	if (ret == 0)
		return MR_SUCCESS;
	else
		return MR_FAILED;
}

/****************************************************************************
 ������:int32 mr_mkDir(const char* name)
 ��  ��:����һ��Ŀ¼
 ��  ��:name:Ŀ¼��
 ��  ��:MR_SUCCESS,MR_FAILED
 ****************************************************************************/
int32 mr_mkDir(const char* name)
{
	char fullpathname[DSM_MAX_FILE_LEN] = { 0 };
	int ret;

	ret = mkdir(get_filename(fullpathname, name), 0777);

	if (ret == 0)
		return MR_SUCCESS;
	else
		return MR_FAILED;
}

/****************************************************************************
 ������:int32 mr_rmDir(const char* name)
 ��  ��:ɾ��һ��Ŀ¼
 ��  ��:name:��ɾ����Ŀ¼��
 ��  ��:MR_SUCCESS,MR_FAILED
 ****************************************************************************/
int32 mr_rmDir(const char* name)
{
	char fullpathname[DSM_MAX_FILE_LEN] = { 0 };
	int ret;

	ret = rmdir(get_filename(fullpathname, name));

	if (ret == 0)
		return MR_SUCCESS;
	else
		return MR_FAILED;
}

/****************************************************************************
 ������:MR_FILE_HANDLE mr_findStart(const char* name, char* buffer, uint32 len)
 ��  ��:��ʼ��һ���ļ�Ŀ¼�������������ص�һ������
 ��  ��:name	 :Ҫ������Ŀ¼��
 buffer:�����һ�����������buf
 len   :buf�Ĵ�С
 ��  ��:�ɹ�:��һ����������ľ��
 ʧ��:MR_FAILED
 ****************************************************************************/
typedef struct
{
	DIR *pDir;
} T_MR_SEARCHDIR, *PT_MR_SEARCHDIR;

MR_FILE_HANDLE mr_findStart(const char* name, char* buffer, uint32 len)
{
	PT_MR_SEARCHDIR t = malloc(sizeof(T_MR_SEARCHDIR));
	char fullpathname[DSM_MAX_FILE_LEN] = { 0 };

	if (!t)
		return MR_FAILED;

	memset(t, 0, sizeof(T_MR_SEARCHDIR));
	memset(buffer, 0, len);
	t->pDir = opendir(get_filename(fullpathname, name));
	if (!t->pDir)
	{
		free(t);
		return MR_FAILED;
	}

	return (MR_FILE_HANDLE) t;
}

/****************************************************************************
 ������:int32 mr_findGetNext(MR_FILE_HANDLE search_handle, char* buffer, uint32 len)
 ��  ��:����Ŀ¼����һ�����
 ��  ��:search_handle :Ŀ¼�ľ��
 buffer        :������������buf
 len           :buf�Ĵ�С
 ��  ��:MR_SUCCESS,MR_FAILED
 ****************************************************************************/
int32 mr_findGetNext(MR_FILE_HANDLE search_handle, char* buffer, uint32 len)
{
	PT_MR_SEARCHDIR t = (PT_MR_SEARCHDIR) search_handle;
	struct dirent *pDt;

	if (!t)
		return MR_FAILED;

	memset(buffer, 0, len);
	pDt = readdir(t->pDir);
	if (!pDt)
		return MR_FAILED;
	strcpy(buffer, pDt->d_name);

	return MR_SUCCESS;
}

/****************************************************************************
 ������:int32 mr_findStop(MR_SEARCH_HANDLE search_handle)
 ��  ��:ֹͣ��ǰ������
 ��  ��:search_handle:�������
 ��  ��:MR_SUCCESS,MR_FAILED
 ****************************************************************************/
int32 mr_findStop(MR_SEARCH_HANDLE search_handle)
{
	PT_MR_SEARCHDIR t = (PT_MR_SEARCHDIR) search_handle;

	if (!t)
		return MR_FAILED;

	closedir(t->pDir);
	free(t);

	return MR_SUCCESS;
}

/****************************************************************************
 ������:int32 mr_ferrno(void)
 ��  ��:�ú������ڵ���ʹ�ã����ص������һ�β����ļ�ʧ�ܵĴ�����Ϣ�����صĴ���
 ��Ϣ���庬����ƽ̨��ʹ�õ��ļ�ϵͳ�йء�
 ��  ��:��
 ��  ��:MR_SUCCESS,MR_FAILED
 ****************************************************************************/
int32 mr_ferrno(void)
{
	return (int32) MR_FAILED;
}

/****************************************************************************
 ������:int32 mr_getLen(const char* filename)
 ��  ��:�õ�ָ���ļ��ô�С
 ��  ��:filename:��ָ�����ļ���
 ��  ��:�ɹ������ļ���С
 ʧ�ܷ���:MR_FAILED
 ****************************************************************************/
int32 mr_getLen(const char* filename)
{
	char fullpathname[DSM_MAX_FILE_LEN] = { 0 };
	struct stat s1;
	int ret;

	ret = stat(get_filename(fullpathname, filename), &s1);

	if (ret != 0)
		return -1;

	return s1.st_size;
}

/****************************************************************************
 ������:int32 mr_exit(void)
 ��  ��:dsm�˳�֪ͨ����
 ��  ��:��
 ��  ��:��
 ****************************************************************************/
int32 mr_exit(void)
{
	exit(0);

	//StopTimer(DSM_TIMER_MAX);
	//ClearAllKeyHandler();
	//StartTimer(DSM_TIMER_MAX, 50, ExitDsmScr);

	return MR_SUCCESS;
}

// 2012/9/11
void mr_md5_init(md5_state_t *pms)
{

}

void mr_md5_append(md5_state_t *pms, const md5_byte_t *data, int nbytes)
{

}

void mr_md5_finish(md5_state_t *pms, md5_byte_t digest[16])
{

}

int32 mr_load_sms_cfg(void)
{
	return MR_FAILED;
}

int32 mr_save_sms_cfg(int32 f)
{
	return MR_FAILED;
}

static void
fix_x0y0x1y1(int *x0, int *y0, int *x1, int *y1){
	if(*x0 > *x1){ //����
		int tmp = *x0;
		*x0 = *x1;
		*x1 = tmp;
	}

	if(*y0 > *y1){ //����
		int tmp = *y0;
		*y0 = *y1;
		*y1 = tmp;
	}
}

 int
clip_rect(int *x0, int *y0, int *x1, int *y1, int r, int b){
	fix_x0y0x1y1(x0, y0, x1, y1);

	//���������
	if(*x0>r || *y0>b
		|| *x1<0 || *y1<0)
		return 1;

	//����Clip������� x y r b 
	*x0 = MAX(*x0, 0);
	*y0 = MAX(*y0, 0);
	*x1 = MIN(*x1, r);
	*y1 = MIN(*y1, b);

	return 0;
}

 
void mr_drawBitmap(uint16* bmp, int16 x, int16 y, uint16 w, uint16 h)
{
	if(showApiLog) LOGI("mr_drawBitmap(bmp:0x%08x, x:%d, y:%d, w:%d, h:%d)", bmp, x, y, w, h);

	drawBitmap(bmp, x, y, w, h);
}

const char *mr_getCharBitmap(uint16 ch, uint16 fontSize, int *width, int *height)
{
	if(showApiLog) LOGI("mr_getCharBitmap(ch:%04x)", ch);

	tsf_charWidthHeight(ch, width, height);
	//��һ���ֽ� �ֿ� �ڶ����ֽ� ���ֽ���
	return (char*)(tsf_getCharBitmap(ch)+2);
}

int32 mr_DispUpEx(int16 x, int16 y, uint16 w, uint16 h)
{

}

void mr_DrawPoint(int16 x, int16 y, uint16 nativecolor)
{
	uint16 * p = w_getScreenBuffer();
	int32 w, h;

	mr_getScreenSize(&w, &h);
	if(x<0 || y<0 || x>w-1 || y>h-1)
		return;
	*(p + w*y + x) = nativecolor;
}

void mr_DrawBitmap(uint16* p, int16 x, int16 y, 
				   uint16 w, uint16 h, 
				   uint16 rop, 
				   uint16 transcoler, 
				   int16 sx, int16 sy, 
				   int16 mw)
{

}

void mr_DrawBitmapEx(mr_bitmapDrawSt* srcbmp, mr_bitmapDrawSt* dstbmp, 
					 uint16 w, uint16 h, 
					 mr_transMatrixSt* pTrans, uint16 transcoler)
{

}

void mr_DrawRect(int16 sx, int16 sy, int16 w, int16 h, 
				uint8 cr, uint8 cg, uint8 cb)
{
	uint16 c = MAKERGB(cr, cg, cb);
	int32 sw, sh;
	int x, y, r, b, x1, y1, i, j;
	uint16 * p = w_getScreenBuffer();

	mr_getScreenSize(&sw, &sh);
	x = sx, y = sy, r = sw-1, b = sh-1;
	x1 = sx+w-1, y1 = sy+h-1;
	
	if(clip_rect(&x, &y, &x1, &y1, r, b))
		return;

	h = y1-y+1;
	w = x1-x+1;
	for (i=y; i<y1; i++){
		for(j=x; j<x1; j++)
			*(p + i*sw + j) = c;
	}

	if(showApiLog) LOGI("mr_DrawRect(x:%d, y:%d, w:%d, h:%d)",
		x, y, w, h);
}

int32 mr_DrawText(char* pcText, int16 x, int16 y, 
				  uint8 r, uint8 g, uint8 b, int is_unicode, uint16 font)
{
	mr_colourSt c;

	c.r = r, c.g = g, c.b = b;
	if(!is_unicode){
		uint8 *out = (uint8 *)mr_c2u(pcText, NULL, NULL);
		tsf_drawText(out, x, y, c, 0);
		mr_free(out, 0);
	}else{
		tsf_drawText((uint8*)pcText, x, y, c, 0);
	}

	if(showApiLog) LOGI("mr_DrawText(text:%s, x:%d, y:%d, )", 
		pcText, x, y);

	return MR_SUCCESS;
}

int mr_BitmapCheck(uint16*p, int16 x, int16 y, uint16 w, uint16 h, 
				   uint16 transcoler, uint16 color_check)
{

}

int mr_wstrlen(char * str)
{
	int lenth=0;
	unsigned char * ss=(unsigned char*)str;

	while(((*ss<<8)+*(ss+1))!=0)
	{
		lenth+=2;
		ss+=2;
	}

	return lenth;
}

int32 mr_DrawTextEx(char* pcText, int16 x, int16 y, mr_screenRectSt rect, mr_colourSt colorst, int flag, uint16 font)
{
	tsf_drawTextLeft((uint8*)pcText, x, y, rect, colorst, flag);
	return MR_SUCCESS;
}

int32 mr_EffSetCon(int16 x, int16 y, int16 w, int16 h, int16 perr, int16 perg, int16 perb)
{

}

int32 mr_TestCom(int32 L, int input0, int input1)
{

}

int32 mr_TestCom1(int32 L, int input0, char* input1, int32 len)
{

}

uint16* mr_c2u(char *cp, int32 *err, int32 *size)
{
	int l = mr_strlen(cp);
	unsigned char *out = mr_malloc(l+2);
	mr_memset(out, 0, l+2);
	l = gbToUCS2BE(cp, out, l);
	if(size) *size = l;
	
	return (uint16*)out;
}

int32 mr_div(int32 a, int32 b)
{
	return a/b;
}

int32 mr_mod(int32 a, int32 b)
{
	return a%b;
}

int32 mr_unzip(uint8* inputbuf, int32 inputlen, uint8** outputbuf, int32* outputlen)
{
	int ret;
	uint8 *data_org = inputbuf;
	uint8 *data_out;
	uint32 size_out;

	if(!inputbuf || inputlen<=0 || !outputbuf
		|| inputbuf[0] != 0x1f
		|| inputbuf[1] != 0x8b)
		return MR_FAILED;
	
	size_out = *(uint32*)(data_org + inputlen - 4);
	data_out = mr_malloc(size_out);

	ret = ungzipdata(data_out, &size_out, data_org, inputlen);
	if (Z_OK == ret) {
		*outputbuf = data_out;
		if (outputlen) *outputlen = size_out;

		return MR_SUCCESS;
	}else{
		mr_free(data_out, size_out);
	}

	return MR_FAILED;
}

uint32 mrc_updcrc(uint8 *s, uint32 n)
{
	return 0;
}

void *mr_readFile(const char* filename, int *filelen, int lookfor)
{
	int32 len;
	uint8* data = mr_readFileFromMrp(filename, &len, lookfor);
	if(filelen) *filelen = len;
	return (void*)data;
}

int32 mr_startShake(int32 ms){
	return MR_FAILED;

}
int32 mr_stopShake(){
	return MR_FAILED;
}
int32 mr_playSound(int type, const void* data, uint32 dataLen, int32 loop){
	return MR_FAILED;

}
int32 mr_stopSound (int type){
	return MR_FAILED;

}
int32 mr_sendSms(char* pNumber, char*pContent, int32 encode){
	return MR_FAILED;

}
void mr_call(char *number){}
int32 mr_getNetworkID(void){
	return MR_FAILED;

}
void mr_connectWAP(char* wap){}
int32 mr_menuCreate(const char* title, int16 num){
	return MR_FAILED;

}
int32 mr_menuSetItem(int32 menu, const char *text, int32 index){
	return MR_FAILED;

}
int32 mr_menuShow(int32 menu){
	return MR_FAILED;

}
int32 mr_menuSetFocus(int32 menu, int32 index){
	return MR_FAILED;

}
int32 mr_menuRelease(int32 menu){
	return MR_FAILED;

}
int32 mr_menuRefresh(int32 menu){
	return MR_FAILED;

}
int32 mr_dialogCreate(const char * title, const char * text, int32 type){
	return MR_FAILED;

}
int32 mr_dialogRelease(int32 dialog){
	return MR_FAILED;

}
int32 mr_dialogRefresh(int32 dialog, const char * title, const char * text, int32 type){
	return MR_FAILED;

}

int32 mr_textCreate(const char * title, const char * text, int32 type){
	return MR_FAILED;
}
int32 mr_textRelease(int32 text){
	return MR_FAILED;
}
int32 mr_textRefresh(int32 handle, const char * title, const char * text){
	return MR_FAILED;
}


int32 mr_editCreate(const char * title, const char * text, int32 type, int32 max_size){
	return MR_FAILED;

}
int32 mr_editRelease(int32 edit){
	return MR_FAILED;

}
const char* mr_editGetText(int32 edit){
	return NULL;

}
int32 mr_winCreate(void){
	return MR_FAILED;

}
int32 mr_winRelease(int32 win){
	return MR_FAILED;

}

int32 mr_initNetwork(MR_INIT_NETWORK_CB cb, const char *mode){
	return MR_FAILED;
}
int32 mr_closeNetwork(void){
	return MR_FAILED;
}
int32 mr_getHostByName(const char *name, MR_GET_HOST_CB cb){
	return MR_FAILED;
}
int32 mr_socket(int32 type, int32 protocol){
	return MR_FAILED;
}
int32 mr_connect(int32 s, int32 ip, uint16 port, int32 type){
	return MR_FAILED;
}
int32 mr_closeSocket(int32 s){
	return MR_FAILED;
}
int32 mr_recv(int32 s, char *buf, int len){
	return MR_FAILED;
}
int32 mr_recvfrom(int32 s, char *buf, int len, int32 *ip, uint16 *port){
	return MR_FAILED;
}
int32 mr_send(int32 s, const char *buf, int len){
	return MR_FAILED;
}
int32 mr_sendto(int32 s, const char *buf, int len, int32 ip, uint16 port){
	return MR_FAILED;
}