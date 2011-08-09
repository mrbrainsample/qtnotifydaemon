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


#include <QToolTip> //alex
#include <QDesktopWidget>
#include <fstream>
#include "notifyarea.h"
#include "notifywidget.h"
#include "dbus.h"

NotifyArea::NotifyArea(char *configPath, bool debug)
{
debugMode = debug;
config = new char[255];
strcpy(config, configPath);

this->ReReadConfig();
messageWidget = new NotifyWidget("MessageWidget",&messageStack, this);
if(debugMode) fprintf(stderr,"MessageWidget created.\n");

notificationWidget = new NotifyWidget("NotificationWidget",&notificationStack, this);
if(debugMode) fprintf(stderr,"NotificationWidget created.\n");
index = (unsigned int) 1;
}

/////////////////////////////////////////////////////////////////////////////////
void NotifyArea::ReReadConfig()
{
convertSpecialSymbols = this->readConfigString("ConvertSpecialSymbols").toInt();
if(convertSpecialSymbols < 1 || convertSpecialSymbols > 2) convertSpecialSymbols = 1;

maxIconSize = this->readConfigString("MaxIconSize").toInt();
maxOpacity = this->readConfigString("Opacity").toFloat();
if(maxOpacity<=0.1)
	{
	maxOpacity=0.7;
	}
widgetStyle = this->readConfigString("GeneralStyle");
if(widgetStyle=="")
	{
	widgetStyle = "margin: 0px; background: black; border: 3px solid white; color: lime; border-radius: 15px;";
	}
//alex:
if (widgetStyle.contains("qtfg") || widgetStyle.contains("qtbg"))
{
	QPalette pal = QToolTip::palette();
	QColor bg = pal.color(QPalette::ToolTipBase);
	QColor fg = pal.color(QPalette::ToolTipText);
	widgetStyle.replace("qtbg",bg.name());
	widgetStyle.replace("qtfg",fg.name());
}
	
for(int i=1;i<4;i++) UrgencyTag[i] = this->readConfigString("UrgencyTag"+QString::number(i));

}
/////////////////////////////////////////////////////////////////////////////////

QString NotifyArea::readConfigString(QString param)
{
if(debugMode) fprintf(stderr,"Reading config string %s ...\n",param.toStdString().c_str());
QString result="";
std::string tmp;
std::ifstream file;
file.open(config);
if(!file.fail())
	{
	while(!file.eof())
		{
		std::getline( file, tmp, '\n' );
		if(tmp.find("#")==0) continue; //skip comments
		if(tmp.find(param.toStdString().c_str())!=std::string::npos)
			{
			result=tmp.substr(tmp.find("=")+1).c_str();
			}
		}
	file.close();
	}
	else
	{
	fprintf(stderr,"Cant open config %s!",config);
	}
if(debugMode) fprintf(stderr,"Returning config string: '%s' = '%s'.\n", param.toStdString().c_str(), result.toStdString().c_str());
return result.trimmed();
}

void NotifyArea::CloseNotification(unsigned id)
{
if(debugMode) fprintf(stderr,"Closing Notification with id=%d...\n",(int)id);
for(std::vector<Message*>::iterator iter=messageStack.begin(); iter != messageStack.end(); iter++)
	{
	if((*iter)->id == id)
		{
		delete *iter;
		messageStack.erase(iter);
		messageWidget->timer->stop();
		messageWidget->checkIfNeedToShow();
		}
	}
for(std::vector<Message*>::iterator iter=notificationStack.begin(); iter != notificationStack.end(); iter++)
	{
	if((*iter)->id == id)
		{
		delete *iter;
		notificationStack.erase(iter);
		notificationWidget->timer->stop();
		notificationWidget->checkIfNeedToShow();
		}
	}
if(debugMode) fprintf(stderr,"Notification %d closed.\n",(int)id);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QPoint NotifyArea::getWidgetPosition(char *widgetName, QSize widgetSize)
{
QPoint p;

if(debugMode) fprintf(stderr,"Calculating widget position...\n");

QDesktopWidget *desktop;
desktop = new QDesktopWidget();
QRect desktopGeometry = desktop->availableGeometry();

if(debugMode) fprintf(stderr,"Got desktop geometry.\n");

QString position = this->readConfigString(QString(widgetName)+"Position");
if(debugMode) fprintf(stderr,"Got widget position from config.\n");

	if(position=="0" || position=="BL")
		p = desktopGeometry.bottomLeft();
	else if(position=="1" || position=="BR")
		p = desktopGeometry.bottomRight();
	else if(position=="2" || position=="TR")
		p = desktopGeometry.topRight();
	else if(position=="3" || position=="TL")
		p = desktopGeometry.topLeft();
	else if(position=="4" || position=="C")
		{
		p = desktopGeometry.bottomRight();
		p.setX(p.x()/2 + widgetSize.width()/2);
		p.setY(p.y()/2 + widgetSize.height()/2);
		}
	else if(position=="5" || position=="RC")
		{
		p = desktopGeometry.bottomRight();
		p.setY(p.y()/2 + widgetSize.height()/2);
		}
	else if(position=="6" || position=="TC")
		{
		p = desktopGeometry.topRight();
		p.setX(p.x()/2 + widgetSize.width()/2);
		}
	else if(position=="7" || position=="LC")
		{
		p = desktopGeometry.bottomLeft();
		p.setY(p.y()/2 + widgetSize.height()/2);
		}
	else if(position=="8" || position=="BC")
		{
		p = desktopGeometry.bottomRight();
		p.setX(p.x()/2 + widgetSize.width()/2);
		}
	else
		p = desktopGeometry.bottomRight();	


delete desktop;
desktop = NULL;
if(debugMode) fprintf(stderr,"Returning widget coordinates.\n");
return p;
}
