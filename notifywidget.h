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




#include <QTimer>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QSignalMapper>

class Message;
class QClickLabel;
class NotifyArea;

class NotifyWidget : public QLabel
{
	Q_OBJECT
public:
	NotifyWidget(const char*, std::vector<Message*> *, NotifyArea *);
	QTimer *timer;
	QClickLabel *text;
	QClickLabel *icon;
	QPushButton *actionButton[10];
	void checkIfNeedToShow();
	void appendMsg(); //append msg's that can be appended
	std::vector<Message*> *messageStack;
	QSignalMapper *signalMap;

public slots:
	void showWidget();
	void hideWidget(int reason = 4);
	void fadeWidget();
	void emitActionInvoked(QString);
signals:
	void NotificationClosed(unsigned,unsigned);
	void ActionInvoked(unsigned int, QString);
private:
NotifyArea *parent;
QTimer *fadeTimer;
char widgetName[255];

};
