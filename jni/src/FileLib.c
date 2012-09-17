#include "FileLib.h"

#include "mrporting.h"
#include "dsm.h"

#include <zlib.h>


//-----------------------------------------
int32 getMrpFileInfo(const char *path, const char *name, int32 *offset, int32 *length)
{
	int32 fd;
	int32 flStar = 0, flEnd = 0; //MRP �ļ��б���ֹλ��
	int32 fnLen = 0, fLen; //mrp ���ļ�������,�ļ�����
	char fName[128] = { 0 }; //�ļ���
	int32 off;

	fd = mr_open(path, MR_FILE_RDONLY);
	if (fd)
	{
		//��ȡ�ļ��б��յ�λ��
		mr_seek(fd, MR_SEEK_SET, 4);
		mr_read(fd, &flEnd, 4);
		flEnd += 8;

		//��ȡ�ļ��б���ʼλ��
		mr_seek(fd, 12, MR_SEEK_SET);
		mr_read(fd, &flStar, 4);

		while (flStar < flEnd)
		{
			//1.��ȡ�ļ���
			mr_seek(fd, flStar, MR_SEEK_SET);
			mr_read(fd, &fnLen, 4); //��ȡ�ļ�������
			mr_read(fd, fName, fnLen); //��ȡ�ļ���

			if (0 != mr_strcmp(fName, name))
			{ //�ҵ���
				goto NEXT;
			}

			//2.��ȡ�ļ����ȡ�ƫ��
			mr_read(fd, &off, 4);
			mr_read(fd, &fLen, 4);
			if (offset)
				*offset = off;
			if (length)
				*length = fLen;

			return MR_SUCCESS;

NEXT:
			//3.׼����ȡ��һ���ļ�
			flStar = flStar + fnLen + 16; //�����¸��ļ�
			fnLen = 0;
		}

		//��ȡ��ϼ�¼����
		mr_close(fd);
	}

	return MR_FAILED;
}

// -------------- ��mrp��ȡ�ļ����� for Mrpoid 2012-9-9 eleqian --------------------
/*
��ѹgzip����
��ע��
�ı���zlib��uncompress���� 2012-9-9 eleqian
����ֵ��
Z_OK - �ɹ�
Z_MEM_ERROR - �ڴ治��
Z_BUF_ERROR - �������������
Z_DATA_ERROR - ���ݴ���
*/
int ungzipdata(uint8 *dest, uint32 *destLen, const uint8 *source, uint32 sourceLen)
{
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	stream.next_out = (Bytef*)dest;
	stream.avail_out = (uInt)*destLen;
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	err = inflateInit2(&stream, MAX_WBITS + 16);
	if (err != Z_OK)
		return err;

	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		inflateEnd(&stream);
		if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
			return Z_DATA_ERROR;
		return err;
	}

	*destLen = stream.total_out;
	err = inflateEnd(&stream);

	return err;
}

// ��ȡmrp�ļ�
// ������mrp·������ȡ�ļ�����ȡλ��(����)����ȡ��С(���أ���ѹ��)����ȡ������(���أ����Խ�ѹ)
// ���أ��ɹ���ʧ��
int32 readMrpFileEx(const char *path, const char *name, int32 *offset, int32 *length, uint8 **data)
{
	int32 fd = 0;
	int32 flStar, flEnd; //MRP �ļ��б���ֹλ��
	int32 fnLen = 0, fLen; //mrp ���ļ�������,�ļ�����
	char fName[128] = {0}; //�ļ���

	fd = mr_open(path, MR_FILE_RDONLY);
	if (0 == fd)
		goto err;

	//��ȡ�ļ��б���ʼλ��
	mr_seek(fd, 12, MR_SEEK_SET);
	mr_read(fd, &flStar, 4);

	//��ȡ�ļ��б��յ�λ��
	mr_seek(fd, 4, MR_SEEK_SET);
	mr_read(fd, &flEnd, 4);
	flEnd += 8;

	while (flStar < flEnd)
	{
		//1.��ȡ�ļ���
		mr_seek(fd, flStar, MR_SEEK_SET);
		mr_read(fd, &fnLen, 4); //��ȡ�ļ�������
		mr_read(fd, fName, fnLen); //��ȡ�ļ���

		if (0 == mr_strcmp(fName, name))
		{
			int32 fOffset;

			//2.��ȡ�ļ����ȡ�ƫ��
			mr_read(fd, &fOffset, 4);
			mr_read(fd, &fLen, 4);

			if (NULL != offset)
				*offset = fOffset;

			// ��ȡ�ļ���С
			if (NULL != length)
			{
				uint8 magic[2];

				mr_seek(fd, fOffset, MR_SEEK_SET);
				mr_read(fd, magic, 2);
				if (magic[0] == 0x1f && magic[1] == 0x8b) {
					mr_seek(fd, fOffset + fLen - 4, MR_SEEK_SET);
					mr_read(fd, length, 4);
				} else {
					*length = fLen;
				}
			}

			// ��ȡ����
			if (NULL != data)
			{
				int ret;
				uint8 *data_org;
				uint8 *data_out;
				uint32 size_out;

				data_org = mr_malloc(fLen);
				mr_seek(fd, fOffset, MR_SEEK_SET);
				mr_read(fd, data_org, fLen);
				size_out = *(uint32*)(data_org + fLen - 4);
				data_out = mr_malloc(size_out);

				ret = ungzipdata(data_out, &size_out, data_org, fLen);
				if (Z_OK == ret) {
					*data = data_out;
					free(data_org);
					if (NULL != length)
						*length = size_out;
				} else if (Z_DATA_ERROR == ret) {
					*data = data_org;
					free(data_out);
				}
			}

			goto ok;
		}

		//3.׼����ȡ��һ���ļ�
		flStar = flStar + fnLen + 16; //�����¸��ļ�
		fnLen = 0;
	}

ok:
	if (0 != fd)
		mr_close(fd);

	return MR_SUCCESS;

err:
	if (0 != fd)
		mr_close(fd);

	return MR_FAILED;
}
