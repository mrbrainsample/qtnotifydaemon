#include <QtCore>
#include <vector>
#include <string.h>

struct message;
class NotifyWidget;

class NotifyArea
{
public:

NotifyArea(char*, bool);
QString readConfigString(QString);
void NotificationClosed(unsigned, unsigned);
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

private: 
std::vector<message> messageStack;
std::vector<message> notificationStack;


};
