bin_PROGRAMS=  fileBrowser

CONFD_DIR = /opt/confd
INCLUDES = -I../inc -I$(CONFD_DIR)/include

fileBrowser_SOURCES = fileBrowserConfd.cpp fileBrowserMain.cpp fileBrowser.cpp fileBrowserVnf.cpp fileBrowserSession.cpp fileBrowserDiskOper.cpp fileBrowserConfdCB.cpp fileBrowserFile.cpp fileBrowserEntry.cpp

fileBrowser_LDFLAGS= -L$(CONFD_DIR)/lib

fileBrowser_LDADD = -lconfd

fileBrowser_CPPFLAGS =
