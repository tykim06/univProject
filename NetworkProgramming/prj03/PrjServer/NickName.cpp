#include "NickName.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#define FD_SETSIZE 64

int nTotalNickNames = 0;
char *nickNames[FD_SETSIZE];

int find_nickname(char *nickname) {
	for (int i = 0; i < nTotalNickNames; i++) {
		if (nickNames[i] == NULL) continue;
		if (strcmp(nickNames[i], nickname) == 0) return i;
	}
	return -1;
}

void add_nickname(char *nickname, int index) {
	if(nickNames[index] == NULL) 
		nickNames[index] = new char[24]();

	strcpy(nickNames[index], nickname);
	nTotalNickNames++;
}

void remove_nickname(int index) {
	delete nickNames[index];

	if (index != nTotalNickNames - 1)
		nickNames[index] = nickNames[nTotalNickNames-1];
	nTotalNickNames--;
}

char *get_nickname(int index) {
	return nickNames[index];
}