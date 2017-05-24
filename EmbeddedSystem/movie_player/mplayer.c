#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <linux/input.h>
#include <dirent.h>
#include <stdbool.h>

#define MDIR "/movie/"

#define EVENT_DEVICE "/dev/input/event1"
#define EVENT_TYPE_PUSH_PULL 1
#define EVENT_TYPE_POS 3
#define EVENT_CODE_X 53
#define EVENT_CODE_Y 54

#define DEFAULT_VOL 3

#define CLCD_BUF_SIZE 32

#define SET_VOL_CMD1 "echo \"set_property volume "
#define SET_VOL_CMD2 "\" > /usr/bin/mplayerfifo"
#define PLAY_PAUSE_CMD "echo \"pause\" > /usr/bin/mplayerfifo"

//movie dir
char dir[30] = MDIR;

int root_pid, movie_pid, touch_pid, key_pid;
int volume;

//system cmd
char cmd[50];

typedef struct {
    char name[20];
} movie_t;
movie_t movielist[5];
int movie_count;
int movie_current_index=0;
bool is_movie_playing = false;
typedef enum {
    MOVIE_OP_NEXT_FILE,
    MOVIE_OP_PREV_FILE,
    MOVIE_OP_SYSTEM,
    MOVIE_OP_EXIT
} movie_op_t;
movie_op_t movie_op;
static bool *is_random_mode = false;

int clcdFd = -1;
char clcd_buf[33];

int fndFd = -1;
char fnd_buf[4] = { 0,0,0,0 };

void set_volume(void) {
	printf("set volume : %d\n", volume);
    char vol_buf[4];
	sprintf(vol_buf, "%d", volume);
	sprintf(cmd, "%s%s%s", SET_VOL_CMD1, vol_buf, SET_VOL_CMD2);
    movie_op = MOVIE_OP_SYSTEM;
	system(cmd);
}

void play_pause()
{
    is_movie_playing = (is_movie_playing) ? false : true;
    movie_op = MOVIE_OP_SYSTEM;
    system("echo \"pause\" > /usr/bin/mplayerfifo");
}

typedef enum {
    CLCD_LINE_FIRST,
    CLCD_LINE_SECOND
} clcd_line_t;

void write_clcd(char *str, clcd_line_t cl) {
    switch(cl) {
        case CLCD_LINE_FIRST:
            memset(clcd_buf, ' ', 16);
            memcpy(clcd_buf, str, strlen(str));
            break;
        case CLCD_LINE_SECOND:
            memset(&clcd_buf[16], ' ', 16);
            memcpy(&clcd_buf[16], str, strlen(str));
            break;
    }
    write(clcdFd, clcd_buf, CLCD_BUF_SIZE);
}

void write_fnd() {
    memset(fnd_buf, 0, 4);
    fnd_buf[1] = movie_current_index + 1;
    fnd_buf[3] = movie_count;
    write(fndFd, &fnd_buf, sizeof(fnd_buf));
}

int play_movie()
{
    write_clcd(movielist[movie_current_index].name, CLCD_LINE_SECOND);
    write_fnd();
    char volumeBuf[4];
	sprintf(volumeBuf, "%d", volume);
    is_movie_playing = true;

	dir[7] = '\0';
	char *argv[] = {"mplayer", "-volume", volumeBuf, "-slave", "-quiet", "-input", "file=/usr/bin/mplayerfifo", "-ao", "alsa:device=hw=1.0", "-vo", "fbdev:/dev/fb0", strcat(dir, movielist[movie_current_index].name), (char *)0 };

	if ((movie_pid = fork()) == 0) {
		if (execv("/usr/bin/mplayer", argv) <0) {
			perror("execv");
			return 0;
		}
	}
	else {
		return movie_pid;
	}
	return movie_pid;
}

void execute_ts_fn(int xpos, int ypos) {
	if(ypos < 150) {
		if(xpos < 512) {
			printf("Volume Down!\n");
            write_clcd("Volume Down", CLCD_LINE_FIRST);
			volume = (volume == 0) ? 0 : volume - 1;
			set_volume();
		} else {
			printf("Volume Up!\n");
            write_clcd("Volume Up", CLCD_LINE_FIRST);
			volume = (volume == 100) ? 100 : volume + 1;
			set_volume();
		}
	}
	else if(ypos < 450) {
		if(xpos < 256) {
			printf("Random Play!\n");
            write_clcd("Random Play", CLCD_LINE_FIRST);
            *is_random_mode = !*is_random_mode;
		} else if(xpos < 768) {
			printf("Play Pause!\n");
			play_pause();
            if(is_movie_playing) write_clcd("Play", CLCD_LINE_FIRST);
            else write_clcd("Pause", CLCD_LINE_FIRST);
		} else {
			printf("Exit Program!\n");
            write_clcd("Exit", CLCD_LINE_FIRST);
            kill(movie_pid, SIGINT);
            kill(root_pid, SIGINT);
		}
	}
	else {
		if(xpos < 512) {
			printf("Prev File!\n");
            write_clcd("Prev File", CLCD_LINE_FIRST);
            movie_op = MOVIE_OP_PREV_FILE;
            kill(movie_pid, SIGINT);
		}
		else {
			printf("Next File!\n");
            write_clcd("Next File", CLCD_LINE_FIRST);
            kill(movie_pid, SIGINT);
		}
	}
}

void str_sort() {
	int i,j;
	char temp[20];

	for(i = 0; i < movie_count; i++) {
		for(j = i; j < movie_count; j++) {
			if(movielist[i].name[0] > movielist[j].name[0]) {
				strcpy(temp, movielist[i].name);
				strcpy(movielist[i].name, movielist[j].name);
				strcpy(movielist[j].name, temp);
			}
		}
	}
}

int makelist(void) {
	DIR *dp;
	struct dirent *dep;
	int count = 0;
	if((dp = opendir("/movie/")) == NULL) {
		perror("Directory Open Fail.");
		exit(1);
	}

	memset(movielist, 0, sizeof(movielist));
	while(dep = readdir(dp)) {
		if(strcmp(dep->d_name, ".") == 0) continue;
		else if(strcmp(dep->d_name, "..") == 0) continue;
		else {
			strcpy(movielist[count].name, dep->d_name);
            printf("music[%d]:%s\n", count, movielist[count].name);
			count++;
		}
	}
	closedir(dp);

	return count;
}

void SigHandler(int signo){
	if(signo == SIGINT) {
		exit(0);
	}
	else if(signo == SIGCHLD) {
		printf("SIGCHLD recieved!\n");
        switch(movie_op) {
            case MOVIE_OP_NEXT_FILE:
                movie_current_index++;
                break;
            case MOVIE_OP_PREV_FILE:
                movie_current_index--;
                movie_op = MOVIE_OP_NEXT_FILE;
                break;
            case MOVIE_OP_SYSTEM:
                movie_op = MOVIE_OP_NEXT_FILE;
                return;
            case MOVIE_OP_EXIT:
                return;
        }
        printf("random mode %d\n", *is_random_mode);
        if(*is_random_mode) movie_current_index = rand() % movie_count;
        else if(movie_current_index == movie_count) movie_current_index = 0;
        else if(movie_current_index == -1) movie_current_index = movie_count - 1;
        movie_pid = play_movie();
    }
}

int main() {
	printf("(Example Code)\nProgram Start!\n");

    is_random_mode = mmap(NULL, sizeof(bool) , PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	signal(SIGCHLD, SigHandler);
	struct input_event ev;
	int tsFd, ev_size;
	int xpos = 0;
	int ypos = 0;
	int size;
	int i, j;

	char keyValue[9] = { 0,0,0,0,0,0,0,0,0 };
	int key = 0;
	int keyFd = -1;
	bool isPressed = 0;

    root_pid = getpid();
	srand(time(NULL));
	movie_count = makelist();
	if(movie_count > 1) {
		str_sort();
	}
	volume = DEFAULT_VOL;
	tsFd = open(EVENT_DEVICE, O_RDONLY);
	if (tsFd == -1) {
		printf("%s is not a vaild device\n", EVENT_DEVICE);
		return EXIT_FAILURE;
	}
	if ((clcdFd = open("/dev/fpga_text_lcd", O_WRONLY)) < 0) {
		printf("Device open error : /dev/fpga_text_lcd\n");
		exit(-1);
	}
	if ((keyFd = open("/dev/fpga_push_switch", O_RDONLY)) < 0) {
		printf("Device open error : /dev/fpga_push_switch\n");
		exit(-1);
	}
	if ((fndFd = open("/dev/fpga_fnd", O_WRONLY)) < 0) {
		printf("Device open error : /dev/fpga_fnd\n");
		exit(-1);
	}

    write_clcd("play", CLCD_LINE_FIRST);
	movie_pid = play_movie();

    key_pid = fork();
    if(key_pid == 0) {
        bool is_key_pressed = false;
        while(1) {
            // KEYPAD
            read(keyFd, &keyValue, sizeof(keyValue));
            if (keyValue[0] == 1) {
                if(is_key_pressed) continue;

                printf("Play Pause!\n");
                play_pause();
                if(is_movie_playing) write_clcd("Play", CLCD_LINE_FIRST);
                else write_clcd("Pause", CLCD_LINE_FIRST);

                is_key_pressed = true;
            }
            else if (keyValue[1] == 1) {
                if(is_key_pressed) continue;

                printf("Random Play!\n");
                write_clcd("Random Play", CLCD_LINE_FIRST);
                *is_random_mode = !*is_random_mode;
                printf("random mode %d\n", *is_random_mode);

                is_key_pressed = true;
            }
            else if (keyValue[2] == 1) {
                if(is_key_pressed) continue;

                printf("Exit Program!\n");
                write_clcd("Exit", CLCD_LINE_FIRST);
                kill(movie_pid, SIGINT);
                kill(root_pid, SIGINT);

                is_key_pressed = true;
            } else {
                if(is_key_pressed) is_key_pressed = false;
            }

            usleep(10000);
        }
    } else if(key_pid > 0) {
        while(1) {
            // TFT
            ev_size = sizeof(ev);
            size = read(tsFd, &ev, ev_size);
            if (size < ev_size) {
                printf("Error size when reading\n");
                close(tsFd);
                return EXIT_FAILURE;
            }

            if(ev.type == EVENT_TYPE_PUSH_PULL){
                if(isPressed == 0) {
                    isPressed = 1;
                } else {
                    isPressed = 0;
                    execute_ts_fn(xpos, ypos);
                }
            } else if(ev.type == EVENT_TYPE_POS){
                if(ev.code == EVENT_CODE_X) {
                    xpos = ev.value;
                } else {
                    ypos = ev.value;
                }
            }
        }
    }

	close(tsFd);
	close(keyFd);
	close(fndFd);
	close(clcdFd);
	printf("Program Exit!\n");

	return 0;
}

