// TSF�����ֿ�ģ�� [4/30/2012 JianbinZhu]
#include "..\mrporting.h"
#include "..\mr_helper.h"
#include "..\Engine.h"
#include "TSFFont.h"


#define TSF_LOG LOGI


//������Ϣ�ṹ��
typedef struct fontPoint {
	int32 uIndexOff;	//unicode�ֿ����������ֿ��ļ��е�ƫ��
	int32 uIndexLen;	//unicode�ֿ���������
	uint8 *uIndexBuf;	//unicode�ֿ�������������ַ���ֿ��������ǻ���ص��ڴ��

	int32 PointOff;		//�ֿ�������ֿ��ļ��е�ƫ��
	int32 PointLen;		//�ֿ���󳤶�
	uint8 *PointBuf;	//�ֿ���󻺳������ڴ���ط�ʽ��

	int32 GBWidth;		//GB�����ַ����
	int32 AsciiWidth;	//ascii�����ַ����
	int32 fontHeight;	//�����ַ��߶�
}T_FONT_INFO, *PT_FONT_INFO;


#define FONT_DES		128

//�����ַ�����
#define CHR_SPACE	' '		//�ո�
#define CHR_TAB		'	'	//�Ʊ��


static T_FONT_INFO	g_nowUse;	//��ǰʹ�õ��ֿ�
static uint8		Buf[FONT_DES];  //���������Ϣ����
static uint8		enable;			//�Ƿ���ñ�־
static uint8		gMemLoad;		//�ڴ���ر�־

static uint16		*pscn;			//��Ļ��������ַ
static int32		scnw, scnh;		//��Ļ�ߴ�


#define MAKERGB(r, g, b) \
	(uint16) ( ((uint32)(r>>3) << 11) + ((uint32)(g>>2) << 5) + ((uint32)(b>>3)) )

static const unsigned char masks[] = {
	0x80, //1000 0000
	0x40, //0100 0000
	0x20, //0010 0000
	0x10, //0001 0000
	0x08, //0000 1000
	0x04, //0000 0100
	0x02, //0000 0010
	0x01  //0000 0001
}; 


//��ȡһ���ַ����ֿ�����е�����
static 
int32 GetOffSet(uint16 chr)
{
	uint16 iM = 0;//(һ�������16λUNICODEֵ16λƫ����)
	uint16 iB = 1;
	uint16 iE = 0;
	int32 UValue = 0;
	int32 indexLen = 0;
	uint8 *buf = NULL;

	indexLen = g_nowUse.uIndexLen;
	buf = g_nowUse.uIndexBuf;

	//2012��5��9 ���� iE = (uint16)indexLen /5;
	iE = indexLen /5;
	while(iB <= iE)
	{
		iM = (iB + iE)/2;
		UValue = buf[(iM-1)*5];
		UValue = UValue << 8;
		UValue += buf[(iM-1)*5+1];

		if(chr == UValue)
		{
			UValue = buf[(iM-1)*5+2];
			UValue = UValue << 8;
			UValue += buf[(iM-1)*5+3];
			UValue = UValue << 8;
			UValue += buf[(iM-1)*5+4];

			return UValue;
		}
		else if(chr > UValue)
		{
			iB = iM+1;
		}
		else
		{
			iE = iM-1;
		}
	}

	return 0;
}

//��ȡ�ַ�����λͼ 
//��һ���ֽ� �ֿ� �ڶ����ֽ� ���ֽ���
uint8* tsf_getCharBitmap(uint16 ch)
{
	int32 offset = GetOffSet(ch);

	mr_memset(Buf, 0, FONT_DES);

	if(offset == 0) {
		offset = GetOffSet(0x53e3);	//���� �� 
		if(!offset)
			return (uint8*)Buf;
	}

	//��һ���ֽ� �ֿ� �ڶ����ֽ� ���ֽ���
	if(gMemLoad){
		mr_memcpy(Buf, g_nowUse.PointBuf + offset, FONT_DES);
	}
	
	return (uint8*)Buf;
}

//���л���
int32 tsf_drawText(uint8 *chr, int16 x, int16 y,  mr_colourSt colorst)
{
	if (!enable || !chr ) {
		return -1;
	}

	{
		int totalPoint,totalIndex,index_I,index_J;
		uint16 *tempBuf= (uint16 *)chr;
		uint16 ch=0;
		int32 temp_mr_screen_w;
		int32 X1,Y1,chx,chy;
		const uint8 *current_bitmap;
		uint8  *p=(uint8*)tempBuf;
		uint8 temp = 0;
		uint16 color=MAKERGB(colorst.r, colorst.g, colorst.b);
		int32 fw, fh = g_nowUse.fontHeight, flen;
		int32 tx,ty;
		

		mr_getScreenSize((int32*)&scnw, (int32*)&scnh);
		pscn = w_getScreenBuffer();

		temp_mr_screen_w = scnw;

		ch = (uint16) ((*p<<8) + *(p+1));
			
		chx = x;
		chy = y;
		while(ch)
		{
			X1 = Y1 = 0;
			totalIndex = totalPoint = 0;
			
			if ((ch == 0x0a) || (ch == 0x0d))//����ֱ�ӷ���
			{
				return 1;
			}
			else if(ch == CHR_SPACE)//�ո���ո�
			{
				chx += g_nowUse.AsciiWidth;
				//������Ļ��Χ���
				if((chx) > temp_mr_screen_w) 
					return 1;
				goto next;
			}
			else if(ch == CHR_TAB)
			{
				chx += 4*g_nowUse.AsciiWidth;
				//������Ļ��Χ���
				if((chx) > temp_mr_screen_w) 
					return 1;
				goto next;
			}
			else
			{
				current_bitmap = tsf_getCharBitmap(ch);

				fw = *current_bitmap;
				flen = *(current_bitmap+1);
				current_bitmap += 2;
				if(fw == 0) fw = g_nowUse.GBWidth;
			}

			//������Ļ��Χ���
			if((chx + fw ) > temp_mr_screen_w) 
				return 1;

			//���Ƶ���
			totalPoint = fh * fw;
			totalIndex = 0;
			for ( index_I = 0; index_I < flen; index_I++)
			{
				temp = current_bitmap[index_I];
				
				for (index_J = 0; index_J < 8; index_J++)
				{
					tx = chx + X1, ty = chy + Y1;
					totalIndex++;

					if(tx < 0 || ty < 0 || tx > scnw-1 || ty > scnh-1){
		
					}else if (temp & masks[index_J])
					{
						*(pscn + (chy+Y1)*scnw + (chx+X1)) = color;
					}
					X1++;
					if ((totalIndex % fw) == 0)
					{
						Y1++;
						X1= 0;
					}
					if (totalIndex >= totalPoint)
						break;
				}
			}

			chx =chx +  fw + TS_FONT_HMARGIN;//�ּ��Ϊ 4
next:
			p += 2;
			ch = (uint16) ((*p<<8) + *(p+1));
		}
	}

	return 1;
}

//�������һ��л���
int32 tsf_drawTextLeft(uint8 *pcText, int16 x, int16 y, 
					   mr_screenRectSt rect, mr_colourSt colorst, uint16 flag)
{
	if (!enable || !pcText || rect.w == 0 || rect.h == 0) {
		return -1;
	}

	{
		uint16 ch;
		const char *current_bitmap;
		uint8 *p = (uint8*)pcText;
		int16 chx = x,chy = y;
		int32 totalIndex,totalPoint,X1,Y1,index_I,index_J;
		uint8 temp = 0;
		uint16 color=MAKERGB(colorst.r, colorst.g, colorst.b);
		int32 right = rect.x+rect.w, btm = rect.y+rect.h;
		int32 fw, fh = g_nowUse.fontHeight, flen;
		int32 lines = 0;
		int32 tx, ty;


		mr_getScreenSize((int32*)&scnw, (int32*)&scnh);
		pscn = w_getScreenBuffer();

		//����unicode/GB����ֵ
		ch = (uint16) ((*p<<8)+*(p+1));
		while(ch)
		{
			if ((ch == 0x0a) || (ch == 0x0d))
			{//���д���
				if(ch == 0x0d)	//�Ƴ��ڶ������з�
					p += 2;

				if(flag & TS_FT_CRLFNEWLINE) {
					chy += (fh + TS_FONT_VMARGIN);
					chx = x;
					lines++;
				}else {
					goto end;
				}

				goto next;
			}
			else if(ch == CHR_SPACE || ch == CHR_TAB)
			{//�ո��Ʊ�� ����
				chx += (ch == CHR_SPACE? g_nowUse.AsciiWidth : 4*g_nowUse.AsciiWidth);
				
				if( (chx > right)) {	//�Զ���������
					if((TS_FT_AUTONEWLINE & flag)){
						chy += (fh + TS_FONT_VMARGIN);
						chx = x;
						lines++;
					}else
						goto end;					
				}

				goto next;
			}
			else
			{//�����ַ�
				current_bitmap = (char*)tsf_getCharBitmap(ch);
								
				fw = *current_bitmap;		//�ַ���
				flen = *(current_bitmap + 1);	//�����ֽ���
				current_bitmap += 2;
			}

			//if(chx > right && chy > btm)	//�����˻�������
			//	goto end;

			//��������
			if(((chx + fw) > right))
			{
				if(flag & TS_FT_AUTONEWLINE){
					chy += (fh + TS_FONT_VMARGIN);
					chx = x;
					lines++;
				}else
					goto end;				
			}

			//������
			//if( ((chx + fw) <= right) 
			//	&& (chx >= rect.x) 
			//	&& ((chy + fh) <= btm) 
			//	&& (chy >= rect.y) 
			//	&& chx >= 0 
			//	&& chy >= 0)
			{
				totalPoint = fh * fw;
				X1 = Y1 = 0;
				totalIndex = 0;
				
				for ( index_I = 0; index_I < flen; index_I++)	//����ռ�ֽ���
				{
					temp = current_bitmap[index_I];
					
					for (index_J = 0; index_J < 8; index_J++)
					{
						tx = chx + X1, ty = chy + Y1;
						totalIndex++;
						
						if(tx < rect.x || ty < rect.y || tx > right || ty > btm){
							//
						}else if (temp & masks[index_J]) {	//��Ļ���
							*(pscn + ty*scnw + tx) = color;
						}
						X1++;
						if ((totalIndex % fw) == 0) {	//������
							Y1++;
							X1= 0;
						}
						if (totalIndex >= totalPoint)
							break;
					}
				}
			}

			//�ۼƿ��
			chx = chx + fw + TS_FONT_HMARGIN;
next:
			p += 2; //��һ���ַ�
			ch = (uint16)((*p<<8)+*(p+1));
		}

end:
		return ( ((lines<<20)&0xFFF00000) | (p-pcText) );
	}

	return 0;
}

//��ȡ�����ı����
int32 tsf_textWidthHeightLines(uint8 *pcText, uint16 showWidth, 
							  int32 *width, int32 *height, int32 *lines)
{
	uint16 chU;
	int32 tempAdd = 0, tempWidth = 0;
	uint8* tempChr = (uint8*)pcText;
	uint8 *bmpPoint = NULL;
	int32 linewidth = showWidth;


	*width = *height = *lines = 0;

	if(!enable || !tempChr || showWidth == 0) {
		return -1;
	}

	chU = (uint16) ((*tempChr<<8)+*(tempChr+1));
		
	while(chU)
	{
		if(chU == CHR_SPACE)	//�ո�
		{
			tempAdd = g_nowUse.AsciiWidth;
						
			goto LineCheck;
		}
		else if(chU == CHR_TAB){
			tempAdd = 4*g_nowUse.AsciiWidth;

			goto LineCheck;
		}
		else if(chU == 0x0a || chU == 0x0d)
		{
			if(chU == 0x0d)	//�Ƴ��ڶ������з�
				tempChr += 2;
			goto NewLine;				
		}
		else
		{
			bmpPoint = (uint8*)tsf_getCharBitmap(chU);
			tempAdd = (*bmpPoint + TS_FONT_HMARGIN);
		}
				
LineCheck:
		if(tempWidth + tempAdd > linewidth)
		{
NewLine:
			*width = (tempWidth>*width)? tempWidth : *width;
			*height += g_nowUse.fontHeight + TS_FONT_VMARGIN;

			(*lines)++;
			tempWidth = 0;
		}else{
			tempWidth += tempAdd;
		}

		//��һ���ַ�
		tempChr +=  2;
		chU = (uint16) ((*tempChr<<8)+*(tempChr+1));
	}
	
	*height += g_nowUse.fontHeight + TS_FONT_VMARGIN;
	*width = (tempWidth>*width)? tempWidth : *width;
	(*lines)++;

	return 0;
}

int32 tsf_charWidthHeight(uint16 chU, int32 *width, int32 *height)
{
	if(height) *height = g_nowUse.fontHeight;
	if(width) *width = *tsf_getCharBitmap(chU);
}

//��ȡ�����ı����
int32 tsf_textWidthHeight(uint8 *pcText, int32 *width, int32 *height)
{
	uint16 chU;
	uint8* p = (uint8*)pcText;
	uint8 *bmpPoint = NULL;
	int32 w=0;

	if(!enable || !p) {
		return -1;
	}

	chU = (uint16) ((*p<<8)+*(p+1));
	while(chU)
	{
		if(chU == CHR_SPACE)	//�ո�	//|| (chU == 0x0a) || (chU == 0x0d)
		{
			w += g_nowUse.AsciiWidth;
		}else if(chU == CHR_TAB){	//�Ʊ�� 4�ո����
			w += 4*g_nowUse.AsciiWidth;
		}else{
			bmpPoint = (uint8*)tsf_getCharBitmap(chU);
			w += *bmpPoint + TS_FONT_HMARGIN;
		}
		
		p +=  2;
		chU = (uint16) ((*p<<8)+*(p+1));
	}
	
	if(height) *height = g_nowUse.fontHeight;// + TS_FONT_VMARGIN;	//�������
	if(width) *width = w;
	
	return MR_SUCCESS;
}

extern unsigned char font16_st[399063];

//��ʼ���ֿ�
int32 tsf_init(void){
	uint8 *head = font16_st;

	enable = 0;	//���ñ�־Ϊ0
	memset(&g_nowUse, sizeof(T_FONT_INFO), 0);

	//��ȡunicode��������Ϣ
	g_nowUse.uIndexOff = *(int32*)(head+12);
	g_nowUse.uIndexLen = *(int32*)(head+16);
	g_nowUse.uIndexBuf = head + g_nowUse.uIndexOff;


	//��ȡ�������Ϣ
	g_nowUse.PointOff = *(int32*)(head+20);
	g_nowUse.PointLen = *(int32*)(head+24);
	g_nowUse.PointBuf = head + g_nowUse.PointOff;
	gMemLoad = TRUE;

	//����ߴ���Ϣ
	g_nowUse.GBWidth = head[28];
	g_nowUse.AsciiWidth = head[29];
	g_nowUse.fontHeight = head[30];

	enable = 1;	//���سɹ�������

	return MR_SUCCESS;
}