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
void infiniteLoop()
{
    while(1) {
        sleep(1);
    }
}

static
int startFileBrowser()
{
    try {
        fileBrowser *pFileBrowser = fileBrowser::getInstance();

        pFileBrowser->init();
        
        printf("fileBrowser start successfully!\n");

        infiniteLoop();
    }

    catch (...) {
         printf("Exception: fileBrowser exit!\n");
    }

    return 0;
}

int main(int argc, char *argv[])
{
    startFileBrowser();

    return 0;
}



