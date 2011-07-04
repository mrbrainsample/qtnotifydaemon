#include <QLabel>
#include <QMouseEvent>
#include <QPainter>

class QClickLabel : public QLabel
{
	Q_OBJECT
	public:
QClickLabel(QWidget *parent = 0) : QLabel(parent){};
	signals:
void clicked();
	protected:
void mouseReleaseEvent(QMouseEvent *e)
	{
	if(e->button() == Qt::LeftButton || e->button() == Qt::RightButton)
		{
		emit clicked();
		}
	}
};
