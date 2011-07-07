#include <QtCore>
#include <vector>
#include <string.h>

class NotifyWidget;
struct Message
{
	public:
int timeout;
int urgency;
unsigned id;
QPixmap *icon;
QString app_name;
QString text;
QVariantMap hints;
QString header;
bool isComplete;
};



class NotifyArea : public QObject
{
Q_OBJECT
public:

NotifyArea(char*, bool);
QString readConfigString(QString);
QPoint getWidgetPosition(char *);
void ReReadConfig();

NotifyWidget *messageWidget;
NotifyWidget *notificationWidget;
unsigned int index;
char *config;
bool debugMode;

float maxOpacity;
QString widgetStyle;
QString UrgencyTag[4];

public slots:
void CloseNotification(unsigned);

private: 
std::vector<Message> messageStack;
std::vector<Message> notificationStack;

signals:
void NotificationClosed(unsigned,unsigned);

};
