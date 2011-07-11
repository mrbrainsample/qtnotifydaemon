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

int convertSpecialSymbols;
float maxOpacity;
int maxIconSize;
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
