#include <signal.h>
#include<sys/stat.h>
#include <pwd.h>
#include "dbus.h"
#include "notifywidget.h"
#include "notifyarea.h"

bool debugMode=false;
void catchSighup(int);
NotifyArea *area;

void createConfigIfNotExists(char *config, char *homedir)
{
FILE *f;
char configdir[255];
strcpy(configdir,homedir);
strcat(configdir,"/.config/qtnotifydaemon/");
mkdir(configdir, S_IREAD | S_IWRITE | S_IEXEC | S_IRGRP | S_IROTH);
if(!fopen(config,"r"))
	{
	f=fopen(config,"w");
	char s[1024]="#Widget style\n\
GeneralStyle = margin: 0px; background: black; border: 3px solid white; color: lime; border-radius: 15px; font-size: 14px;\n\
\n\
#Style of notifications with low urgency\n\
UrgencyTag1 = <b>\n\
\n\
#Style of notifications with normal urgency\n\
UrgencyTag2 = <b><u>\n\
\n\
#Style of notifications with critical urgency\n\
UrgencyTag3 = <b><u><font color=red>\n\
\n\
#Opacity floating point value: 0-1\n\
Opacity = 0.8\n\
MaxIconSize = 80\n\
\n\
#Widgets position: 0 - bottomRight, 1 - bottomRight, 2 - topLeft, 3 - topRight\n\
#Positions of two widgets must differ\n\
MessageWidgetPosition = 0\n\
NotificationWidgetPosition = 2\n\
";
	fprintf(f,"%s",s);
	fclose(f);
	}
}

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
char config[255]="";

struct passwd *pw = getpwuid(getuid());
strcpy(config,pw->pw_dir);
strcat(config,"/.config/qtnotifydaemon/qtnotifydaemon.conf");

createConfigIfNotExists(config, pw->pw_dir);

for(int i=0;i<argc;i++)
	{
	if(strcmp(argv[i],"--help")==0)
		{
		printf("qtnotifydaemon v0.1\nUsage: %s [OPTIONS] \"header\" \"text\"\n",argv[0]);
		printf("Option:\n-c	-	config filename\n--debug	-	debug mode");	
		return 0;
		}
	if(strcmp(argv[i],"-c")==0)
		{
		i++;
		strcpy(config,argv[i]);
		continue;
		}
	if(strcmp(argv[i],"--debug")==0)
		{
		debugMode=true;
		continue;
		}
	}

int j=fork();
if(j<0)
	{
	fprintf(stderr,"Fork error.");
	}
	else if(j>0)
	{
	return 0;
	}

QApplication app(argc,argv);
area = new NotifyArea(config, debugMode);
QMyDBusAbstractAdaptor adaptor(&app, area);
QDBusConnection connection = QDBusConnection::connectToBus(QDBusConnection::SessionBus, "org.freedesktop.Notifications");
if (connection.isConnected()) printf("Connection established\n");
if (!connection.registerService("org.freedesktop.Notifications")) {
	printf("Cant register service. Is another instance of this application running?\n");
	return 1;
	}
if (!connection.registerObject("/org/freedesktop/Notifications", &adaptor, QDBusConnection::ExportAllContents)) {
	printf("Cant register object\n");
	return 1;
	}


struct sigaction *sa;
sa=new struct sigaction;
sa->sa_handler = catchSighup;
sigaction(SIGHUP, sa, 0);

adaptor.Notify("qtnotifydaemon", 0, "", "qtnotifydaemon started", "Daemon started successfully", QStringList(), QVariantMap(), 3000);

app.exec();
return 0;
}
///////////////////////////////////////////////////////////////

void catchSighup(int param)
{
area->ReReadConfig();
}

