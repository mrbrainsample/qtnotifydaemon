#include <QTimer>
#include <QLabel>
#include <QWidget>

struct message;
class QClickLabel;
class NotifyArea;

class NotifyWidget : public QLabel
{
	Q_OBJECT
public:
	NotifyWidget(const char*, std::vector<message> *, NotifyArea *);
	QTimer *timer;
	QClickLabel *text;
	QClickLabel *icon;
	void checkIfNeedToShow();
	void appendMsg(); //append msg's that can be appended
	std::vector<message> *messageStack;
public slots:
	void showWidget();
	void hideWidget();
	void fadeWidget();
private:
NotifyArea *parent;
QTimer *fadeTimer;
char widgetName[255];

};
