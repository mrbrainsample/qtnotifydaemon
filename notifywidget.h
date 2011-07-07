#include <QTimer>
#include <QLabel>
#include <QWidget>

struct Message;
class QClickLabel;
class NotifyArea;

class NotifyWidget : public QLabel
{
	Q_OBJECT
public:
	NotifyWidget(const char*, std::vector<Message> *, NotifyArea *);
	QTimer *timer;
	QClickLabel *text;
	QClickLabel *icon;
	void checkIfNeedToShow();
	void appendMsg(); //append msg's that can be appended
	std::vector<Message> *messageStack;
public slots:
	void showWidget();
	void hideWidget();
	void fadeWidget();
signals:
	void NotificationClosed(unsigned,unsigned);
private:
NotifyArea *parent;
QTimer *fadeTimer;
char widgetName[255];

};
