#include <QApplication>
#include <QtDBus>
#include <QWidget>
#include <QIcon>
#include <QPixmap>
#include <QDBusArgument>

#define MAX_NOTIFICATION_ID 2048

class NotifyArea;

struct message
	{
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




class QMyDBusAbstractAdaptor: public QDBusAbstractAdaptor
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")
private:
	NotifyArea *widget;

public:
	QMyDBusAbstractAdaptor(QApplication *application, NotifyArea *area);

	Q_INVOKABLE QStringList GetCapabilities();

	Q_INVOKABLE QString GetServerInformation(QString& vendor, QString& version, QString& spec_version);

	Q_INVOKABLE void NotificationClosed(unsigned id, unsigned reason);

	QPixmap getPixmapFromHint(QVariant argument);

	Q_INVOKABLE unsigned Notify(QString app_name, unsigned id, QString icon, QString summary, QString body, QStringList actions, QVariantMap hints, int timeout, QString &return_id);
};
