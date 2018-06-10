#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>
#include <list>
#include <fstream>

#include "fileBrowserConfd.h"
#include "fileBrowser.h"

static
int writePid(const char* file)
{
    char filename[256] = {0};
    char pid[256] = {0};
    int pid_len;
    int fd;

    snprintf(filename, sizeof(filename)-1, "/var/lock/%s.pid", file);
    pid_len = snprintf(pid, sizeof(pid)-1, "%lu", (unsigned long)getpid());

    if ((fd=open(filename, O_RDWR|O_CREAT|O_TRUNC, 0644)) < 0) {
        return -1;
    }

    if (write(fd, pid, pid_len)!=pid_len) {
        close(fd);
        unlink(filename);
        return -1;
    }
    close(fd);

    return 0;
}

static
void infiniteMsgLoop()
{
    while(1) {
        sleep(1);
    }
}

static
int startFileBrowserMgr()
{
    try {
        fileBrowser *fileBrowserMain = fileBrowser::getInstance();

        fileBrowserMain->init();
        
        printf("fileBrowser start successfully!\n");

        infiniteMsgLoop();
    }

    catch (...) {
         printf("Exception: fileBrowser exit!\n");
    }

    return 0;
}

int main(int argc, char *argv[])
{
    writePid("fileBrowser");
    signal(SIGPIPE, SIG_IGN); 

    startFileBrowserMgr();

    return 0;
}



