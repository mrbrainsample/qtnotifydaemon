#include <pwd.h>
#include <signal.h>
#include "dbus.h"
#include "notifywidget.h"
#include "notifyarea.h"

bool debugMode=false;
void catchSighup(int);
NotifyArea *area;

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
char config[255]="";
struct passwd *pw = getpwuid(getuid());
strcpy(config,pw->pw_dir);
strcat(config,"/.config/qtnotifydaemon/qtnotifydaemon.conf");

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

QString return_id;
adaptor.Notify("qtnotifydaemon", 0, "", "qtnotifydaemon started", "Daemon started successfully", QStringList(), QVariantMap(), 3000, return_id);

app.exec();
return 0;
}
///////////////////////////////////////////////////////////////

void catchSighup(int param)
{
area->ReReadConfig();
}

