/*
 * Copyright (C) 2011 Drogolov Roman Olegovich <drull@drull.org.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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



#include "dbus.h"
#include "notifyarea.h"
#include "notifywidget.h"


QMyDBusAbstractAdaptor::QMyDBusAbstractAdaptor(QApplication *application, NotifyArea *Area) : QDBusAbstractAdaptor(application)
	{
	notificationArea = Area;
	connect(notificationArea->messageWidget, SIGNAL(NotificationClosed(unsigned,unsigned)), this, SIGNAL(NotificationClosed(unsigned,unsigned)));
	connect(notificationArea->notificationWidget, SIGNAL(NotificationClosed(unsigned,unsigned)), this, SIGNAL(NotificationClosed(unsigned,unsigned)));

	}


////////////////////////////////////////////////////////////////////////////////////////////////
QPixmap QMyDBusAbstractAdaptor::getPixmapFromHint(QVariant argument)
{
	int width, height, rowstride, bitsPerSample, channels;
    bool hasAlpha;
    QByteArray data;

	const QDBusArgument arg=argument.value<QDBusArgument>();
	arg.beginStructure();
	arg >> width;
	arg >> height;
	arg >> rowstride;
	arg >> hasAlpha;
	arg >> bitsPerSample;
	arg >> channels;
	arg >> data;
	arg.endStructure();
	QImage img=QImage((uchar*)data.constData(),width,height, QImage::Format_ARGB32).rgbSwapped();
	QPixmap p;
	p.convertFromImage(img);
	return p;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
unsigned QMyDBusAbstractAdaptor::Notify(QString app_name, unsigned id, QString icon, QString summary, QString body, QStringList actions, QVariantMap hints, int timeout)
{
if(notificationArea->debugMode) fprintf(stderr," *** Receiving new notification...\n");
Message msg;
bool ok, foundSynchronous=false, foundById=false;

body.replace("&lt;",   "<",     Qt::CaseSensitive); //replace <>"' \n symbols
body.replace("&gt;",   ">",     Qt::CaseSensitive);
body.replace("&quot;", "\"",    Qt::CaseSensitive);
body.replace("&apos;", "'",     Qt::CaseSensitive);
body.replace("\n",     "<br>",  Qt::CaseSensitive);

msg.app_name = app_name;
msg.isComplete = false;
msg.hints = hints;
if(timeout>0)
	{
		msg.timeout = timeout;
	}
	else
	{
		msg.timeout = 3000;
	}
	if(id == 0) // Calculate new id. If old id==0 --> new id = current+1, if old id!=0 --> new id = old id
	{
		notificationArea->index = notificationArea->index + 1;
		if(notificationArea->index > MAX_NOTIFICATION_ID)	notificationArea->index = (unsigned int)1;
		id = notificationArea->index;
	}
msg.id = id;

//		----------------------------------------------------------------------------	
	msg.urgency=hints["urgency"].toInt(&ok)+1;
	if(!ok) msg.urgency = 1;

//		-----------------------------------------------------------------------------
//							Set icon
if(notificationArea->debugMode) fprintf(stderr," *** Attempt to set icon...\n");
int maxiconsize = notificationArea->readConfigString("MaxIconSize").toInt();
msg.icon = new QPixmap();

if(!QPixmap(icon).isNull()) //Check if icon file is available, then check if it standart icon.
	{
	*msg.icon = QIcon(icon).pixmap(maxiconsize);
	}
	else if(!hints["icon_data"].isNull())
		{
		*msg.icon = getPixmapFromHint(hints["icon_data"]);
		}
	else if(!hints["image_data"].isNull())
		{
		*msg.icon = getPixmapFromHint(hints["image_data"]);
		}
	else if(!QIcon::fromTheme(icon).pixmap(maxiconsize).isNull())
		{
		*msg.icon = QIcon::fromTheme(icon).pixmap(maxiconsize);
		}
	else if(QFile::exists(":/images/"+icon+".svg") && !QIcon(":/images/"+icon+".svg").isNull())
		{
		*msg.icon = QIcon(":/images/"+icon+".svg").pixmap(maxiconsize);
		}

if(notificationArea->debugMode) fprintf(stderr," *** Icon set successfully.\n");

			
msg.header = summary;
msg.text = body;

//		-----------------------------------------------------------------------------
//								Search by ID
if(notificationArea->debugMode) fprintf(stderr," *** Attempt to search basic notifications by id...\n");
for(std::vector<Message>::iterator iter=notificationArea->messageWidget->messageStack->begin(); iter != notificationArea->messageWidget->messageStack->end(); iter++)
	{
	if(iter->id == id && id > 0)
		{
		*iter=msg;
		foundById=true;
		break;
		}
	}
if(notificationArea->debugMode) fprintf(stderr," *** ... done!\n");
if(notificationArea->debugMode) fprintf(stderr," *** Attempt to search private-synchronous notifications by id...\n");

for(std::vector<Message>::iterator iter=notificationArea->notificationWidget->messageStack->begin(); iter != notificationArea->notificationWidget->messageStack->end(); iter++)
	{
	if(iter->id == id && id > 0)
		{
		*iter=msg;
		foundById=true;
		break;
		}
	}
if(notificationArea->debugMode) fprintf(stderr," *** Search by id finished.\n");


//		-----------------------------------------------------------------------------
//						Handle synchronous notifications

if(notificationArea->debugMode) fprintf(stderr," *** Take care of private-synchronous notifications.\n");
		
if(hints["x-canonical-private-synchronous"].toString().size()>0)
	{
	msg.header = hints["x-canonical-private-synchronous"].toString()+": "+QString::number(hints["value"].toInt());
	for(std::vector<Message>::iterator iter=notificationArea->notificationWidget->messageStack->begin(); iter != notificationArea->notificationWidget->messageStack->end(); iter++)
		{
		if(iter->hints["x-canonical-private-synchronous"] == hints["x-canonical-private-synchronous"])
			{
			iter->header = msg.header;
			iter->isComplete=false;
			foundSynchronous=true;
			break;
			}
		}

	}
if(notificationArea->debugMode) fprintf(stderr," *** ... done.\n");

//		-----------------------------------------------------------------------------

if(notificationArea->debugMode) fprintf(stderr," *** Pushing notification to stack...");

if(!foundById)
if(!foundSynchronous)
	{
	if(hints["x-canonical-private-synchronous"].toString().size()>0)
		{
		notificationArea->notificationWidget->messageStack->push_back(msg);
		}
		else
		{
		notificationArea->messageWidget->messageStack->push_back(msg);
		}
	}

if(notificationArea->debugMode) fprintf(stderr," *** ... done!\n");

notificationArea->messageWidget->checkIfNeedToShow();
notificationArea->notificationWidget->checkIfNeedToShow();
if(notificationArea->debugMode) fprintf(stderr," *** Notification received.\n");
return msg.id;
}



/////////////////////////////////////////////////////////////////////////////////

Q_INVOKABLE QString QMyDBusAbstractAdaptor::GetServerInformation(QString& vendor, QString& version, QString& spec_version)
{
vendor = "Drull notification daemon";
version = "0.1";
spec_version = "0.1";
return "";
}
///////////////////////////////////////////////////////////////////////////
Q_INVOKABLE QStringList QMyDBusAbstractAdaptor::GetCapabilities()
{
QStringList lst;
lst << "body" << "body-markup" << "body-hyperlinks" << "body-images" << "icon-static" << "image/svg+xml" << "private-synchronous" << "append"  << "x-canonical-private-synchronous";
return lst;
}


