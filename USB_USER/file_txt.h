#ifndef _FILE_TXT_H_
#define _FILE_TXT_H_

#define FILE_TXT_BUFFER_SIZE 	(512)

int TXT_FileOpen(char * fn);
void TXT_FileClose(void);
int TXT_GetString(char * str);

#endif
