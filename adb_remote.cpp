#include <map>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <termios.h>
#include "adb_remote.h"
using namespace std;



void init_map(map < int, ADB_KEYCODE > *, key_pair *, int);
int prepair_adb_cmd(char *, int, map < int, ADB_KEYCODE >);
int handle(int, map < int, ADB_KEYCODE >);

int main(void)
{
	map < int, ADB_KEYCODE > kmap;
	FILE *fp;
	struct termios new_setting;
	struct termios init_setting;
	fp = fopen(ttyname(1), "r");
	if (fp == NULL)
	{
		fprintf(stderr, "tty open error.\n");
		exit(-1);
	}

	//turn off echo
	tcgetattr(0, &init_setting);
	new_setting = init_setting;
	new_setting.c_lflag &= ~ECHO;
	new_setting.c_lflag &= ~ICANON;
	tcsetattr(0, TCSANOW, &new_setting);

	// init key map
	init_map(&kmap, kmp, sizeof(kmp) / sizeof(key_pair));

	puts("Type 'q' or 'Q' to exit. \n");
	int ch;
	while (true)
	{
		fscanf(fp, "%c", &ch);
		if ((char) ch == 'q' || (char) ch == 'Q')

		{
			//turn on echo
			tcsetattr(0, TCSANOW, &init_setting);
			fclose(fp);
			return 0;
		}
		if (handle((char) ch, kmap))
		{
			tcsetattr(0, TCSANOW, &init_setting);
			fclose(fp);
			return 0;
		}
		fflush(stdout);
	}
}

int handle(int cmd_id, map < int, ADB_KEYCODE > kmap)
{
	static int status = 0;
	char cmd_str[256] = { 0 };
	int i = 0;
	switch (status)
	{
	case 0:
		if ((char) cmd_id == 27)
		{
			status = 1;
			return 0;
		}
		break;
	case 1:
		if ((char) cmd_id == 91)
		{
			status = 2;
			return 0;
		}
		if ((char) cmd_id == 27)
		{
			return 0;
		}
		else
		{
			status = 0;
			break;
		}
	case 2:
		if ((char) cmd_id == 27)
		{
			status = 1;
			return 0;
		}
		switch (cmd_id)
		{
		case 65:				//up
			cmd_id = 0;
			break;
		case 66:				//down
			cmd_id = 1;
			break;
		case 67:				//right
			cmd_id = 3;
			break;
		case 68:				//left
			cmd_id = 2;
			break;
		default:;
		}
		status = 0;
		break;
	}
	if (prepair_adb_cmd(cmd_str, cmd_id, kmap))
	{
		fprintf(stderr, "unknown\n");
		return 1;
	}

	printf("send command :\033[1;40;33m %s\033[0m\n", cmd_str);
	i = system(cmd_str);

	return i;
}

void init_map(map < int, ADB_KEYCODE > *kmap, key_pair * keypair, int count)
{
	int i;
	for (i = 0; i < count; i++)
	{
		(*kmap)[keypair[i].key] = keypair[i].value;
	}
}

int prepair_adb_cmd(char *cmd_str, int cmd_id, map < int, ADB_KEYCODE > kmap)
{
	//assert(cmd_str);
	map < int, ADB_KEYCODE >::iterator iter;
	iter = kmap.find(cmd_id);
	if (iter != kmap.end())
	{
		sprintf(cmd_str, "adb shell input keyevent %d", iter->second);
		return 0;
	}
	return 1;
}
