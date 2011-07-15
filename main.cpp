/*
 * Copyright (C) 2011 Drogolov Roman Olegovich <drull@drull.org.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>
 *
 */




#include <signal.h>
#include <sys/stat.h>
#include <pwd.h>
#include "dbus.h"
#include "notifywidget.h"
#include "notifyarea.h"
#include <QDBusMessage>


char configcontent[1024]="\
#Widget style\n\
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
\n\
#Convert special symbols. \"&lt;\" to <, \"&gt;\" to >, \"&apos\"; to ', \"&quot;\" to \", 1 = yes, 2 = no\n\
ConvertSpecialSymbols = 1\n\
";


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
	fprintf(f,"%s",configcontent);
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
		printf("qtnotifydaemon v1.0.9\nUsage: %s [OPTIONS]\n",argv[0]);
		printf("Option:\n\t-c	-	config filename\n\t--debug	-	debug mode\n");	
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
QMyDBusAbstractAdaptor *adaptor = new QMyDBusAbstractAdaptor(&app,area);
QDBusConnection connection = QDBusConnection::connectToBus(QDBusConnection::SessionBus, "org.freedesktop.Notifications");
if (connection.isConnected()) printf("Connection established\n");
if (!connection.registerService("org.freedesktop.Notifications")) {
	printf("Cant register service. Is another instance of this application running?\n");
	return 1;
	}
if (!connection.registerObject("/org/freedesktop/Notifications", &app, QDBusConnection::ExportAdaptors)) {
	printf("Cant register object\n");
	return 1;
	}

struct sigaction *sa;
sa=new struct sigaction;
sa->sa_handler = catchSighup;
sigaction(SIGHUP, sa, 0);

adaptor->Notify("qtnotifydaemon", 0, "", "qtnotifydaemon started", "Daemon started successfully", QStringList(), QVariantMap(), 3000);

app.exec();
return 0;
}
///////////////////////////////////////////////////////////////

void catchSighup(int param)
{
area->ReReadConfig();
}

