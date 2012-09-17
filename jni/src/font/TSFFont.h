// TSF�����ֿ�ģ�� [4/30/2012 JianbinZhu]
// �ȿ��ֿ� ��ASCIIΪGB ��ȵ�һ��

#ifndef	_TS_FONT_H_
#define _TS_FONT_H_



//���������ʽ
#define	TS_FT_NORMAL		0x0000	//���棨�����û���0�ʹ�����ͨ���ƣ�
#if 0 //δʵ��
#define	TS_FT_BLOD			0x0002	//����
#define	TS_FT_ELASTIC		0x0004	//б��
#endif
#define TS_FT_AUTONEWLINE	0x0008	//�����������Զ�����
#define TS_FT_CRLFNEWLINE	0x0010	//ʶ�� \r \n �Զ�����

//�ַ���ඨ��
#define TS_FONT_HMARGIN		0	//���ַ���ˮƽ���
#define TS_FONT_VMARGIN		2	//���ַ��䴹ֱ���


//���㷵��ֵ�е�����
#define TS_FONT_GET_LINE(i) (((unsigned int)((i)&0xFFF00000))>>20)

//���㷵��ֵ�е�offֵ
#define TS_FONT_GET_OFF(i) ((unsigned int)((i)&0x000FFFFF))


/**
 * �������һ��ַ���,ֻ֧��Unicode����
 * 
 * ����:
 * pText:   ������Unicode������ַ���
 * x,y:       ��ʾ�ı������Ͻ�x,y����
 * r:      �����������(λ�ڿ���������Ĳ��ֽ�������ʾ)
 * c:   ���廭����ɫ
 * flag: ȡֵ������:(�����û������'|'����ʾ������ʽ)
 *
 * ����:
 * a) һ��32λ��intֵ:
 *					  TS_FONT_GET_LINE(i) - ռ������(δ�����Ե��ַ�����������,��������һ��)
 *					  TS_FONT_GET_OFF(i) - ��һ�������Ե��ַ���offֵ,��off==len,��˵��ȫ���ַ�������������
 *											   �����Ե��ַ���ָ: ���ַ���ʼ��֮��������ַ�,���������ڿ���������.
 * b) -1 ʧ��
 */
int32 tsf_drawTextLeft(uint8 *pText, int16 x, int16 y, mr_screenRectSt r, mr_colourSt c, uint16 flag);

/**
 * ���Ƶ����ı�
 *
 * �������з���������
 *
 * flag ��Ч
 */
int32 tsf_drawText(uint8 *pText, int16 x, int16 y,  mr_colourSt colorst);

/**
 * ��ȡ�����ı����
 * 
 * ���룺
 *		showWidth������ʾ�ı�����Ŀ�ȣ������ݸÿ�ȶ��ı�����
 *
 * �����
 *		width�������ı�����һ��
 *		height���������ܸ߶ȣ������м�� TS_FONT_VMARGIN��
 *		lines��������
 *
 * ���أ�-1 ʧ�ܣ�0 �ɹ�
 */
int32 tsf_textWidthHeightLines(uint8 *pcText, uint16 showWidth, 
							   int32 *width, int32 *height, int32 *lines);

/**
 * ��ȡ�����ı����
 * 
 * �����
 *		width�������ı�����һ��
 *		height���������ܸ߶ȣ�ע�⣺�������м�� TS_FONT_VMARGIN��
 */
int32 tsf_textWidthHeight(uint8 *pcText, int32 *width, int32 *height);

int32 tsf_charWidthHeight(uint16 chU, int32 *width, int32 *height);

uint8* tsf_getCharBitmap(uint16 ch);

int32 tsf_init(void);
#endif