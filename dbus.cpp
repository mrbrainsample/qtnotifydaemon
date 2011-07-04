#include "dbus.h"
#include "notifyarea.h"
#include "notifywidget.h"


QMyDBusAbstractAdaptor::QMyDBusAbstractAdaptor(QApplication *application, NotifyArea *Area) : QDBusAbstractAdaptor(application)
	{
	widget = Area;
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
unsigned QMyDBusAbstractAdaptor::Notify(QString app_name, unsigned id, QString icon, QString summary, QString body, QStringList actions, QVariantMap hints, int timeout, QString &return_id)
{
if(widget->debugMode) fprintf(stderr," *** Receiving new notification...\n");
struct message msg;
bool ok, foundSynchronous=false, foundById=false;
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
		widget->index = widget->index + 1;
		if(widget->index > MAX_NOTIFICATION_ID)	widget->index = (unsigned int)1;
		id = widget->index;
	}
msg.id = id;

//		----------------------------------------------------------------------------	
	msg.urgency=hints["urgency"].toInt(&ok)+1;
	if(!ok) msg.urgency = 1;

//		-----------------------------------------------------------------------------
//							Set icon
if(widget->debugMode) fprintf(stderr," *** Attempt to set icon...\n");
int maxiconsize = widget->readConfigString("MaxIconSize").toInt();
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
	else if(!QIcon(":/images/"+icon+".svg").isNull())
		{
		*msg.icon = QIcon(":/images/"+icon+".svg").pixmap(maxiconsize);
		}

if(widget->debugMode) fprintf(stderr," *** Icon set successfully.\n");

			
msg.header = summary;
msg.text = body;

//		-----------------------------------------------------------------------------
//								Search by ID
if(widget->debugMode) fprintf(stderr," *** Attempt to search basic notifications by id...\n");
for(std::vector<message>::iterator iter=widget->messageWidget->messageStack->begin(); iter != widget->messageWidget->messageStack->end(); iter++)
	{
	if(iter->id == id && id > 0)
		{
		*iter=msg;
		foundById=true;
		break;
		}
	}
if(widget->debugMode) fprintf(stderr," *** ... done!\n");
if(widget->debugMode) fprintf(stderr," *** Attempt to search private-synchronous notifications by id...\n");

for(std::vector<message>::iterator iter=widget->notificationWidget->messageStack->begin(); iter != widget->notificationWidget->messageStack->end(); iter++)
	{
	if(iter->id == id && id > 0)
		{
		*iter=msg;
		foundById=true;
		break;
		}
	}
if(widget->debugMode) fprintf(stderr," *** Search by id finished.\n");


//		-----------------------------------------------------------------------------
//						Handle synchronous notifications

if(widget->debugMode) fprintf(stderr," *** Take care of private-synchronous notifications.\n");
		
if(hints["x-canonical-private-synchronous"].toString().size()>0)
	{
	msg.header = hints["x-canonical-private-synchronous"].toString()+": "+QString::number(hints["value"].toInt());
	for(std::vector<message>::iterator iter=widget->notificationWidget->messageStack->begin(); iter != widget->notificationWidget->messageStack->end(); iter++)
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
if(widget->debugMode) fprintf(stderr," *** ... done.\n");

//		-----------------------------------------------------------------------------

if(widget->debugMode) fprintf(stderr," *** Pushing notification to stack...");

if(!foundById)
if(!foundSynchronous)
	{
	if(hints["x-canonical-private-synchronous"].toString().size()>0)
		{
		widget->notificationWidget->messageStack->push_back(msg);
		}
		else
		{
		widget->messageWidget->messageStack->push_back(msg);
		}
	}

if(widget->debugMode) fprintf(stderr," *** ... done!\n");

widget->messageWidget->checkIfNeedToShow();
widget->notificationWidget->checkIfNeedToShow();
if(widget->debugMode) fprintf(stderr," *** Notification received.\n");
return_id=QString::number(msg.id);
return msg.id;
}



/////////////////////////////////////////////////////////////////////////////////
Q_INVOKABLE void QMyDBusAbstractAdaptor::NotificationClosed(unsigned id, unsigned reason)
{
if(widget->debugMode) fprintf(stderr," *** Received signal to close notification.\n");
widget->NotificationClosed(id, reason);
if(widget->debugMode) fprintf(stderr," *** Notification closed.\n");
}

///////////////////////////////////////////////////////////////////////////////

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


