#include <rtthread.h>
#include <dfs.h>
#include <dfs_posix.h>
#include "file_txt.h"

static char txt_buffer[FILE_TXT_BUFFER_SIZE];
static int txt_rd_pos = 0;
static int txt_left = 0;
static int txt_fd = -1;

int TXT_FileOpen(char * fn)
{
	txt_fd = open(fn, O_RDONLY, 0);
	if(txt_fd >= 0)return 0;
	else return 1;
}

void TXT_FileClose(void)
{
	if(txt_fd >= 0)
		close(txt_fd);
	txt_fd = -1;
}

int TXT_GetString(char * str)
{
	int len = 0;
	if(txt_fd < 0)return 0;
	else
	{
		while(1)
		{
			char c;
			if(txt_left == 0)
			{
				int size_t = FILE_TXT_BUFFER_SIZE - txt_rd_pos;
				txt_left += read(txt_fd, &txt_buffer[txt_rd_pos], size_t);
				txt_left += read(txt_fd, &txt_buffer[0], txt_rd_pos);
				if(txt_left == 0) break;/* file end */
			}
			c = txt_buffer[txt_rd_pos++];
			txt_rd_pos &= (FILE_TXT_BUFFER_SIZE-1);
			txt_left--;
			if(c == 0x0A)continue;
			else str[len++] = c;
			if(c == 0x0D)break;
		}
	}
	str[len] = '\0';
	return len;
}