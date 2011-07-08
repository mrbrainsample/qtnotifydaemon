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




#include <QApplication>
#include <QtDBus>
#include <QWidget>
#include <QIcon>
#include <QPixmap>
#include <QDBusArgument>

#define MAX_NOTIFICATION_ID 2048

class NotifyArea;

struct Message;

class QMyDBusAbstractAdaptor: public QDBusAbstractAdaptor
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")
private:
	NotifyArea *notificationArea;

public:
	QMyDBusAbstractAdaptor(QApplication *application, NotifyArea *area);

	Q_INVOKABLE QStringList GetCapabilities();

	Q_INVOKABLE QString GetServerInformation(QString& vendor, QString& version, QString& spec_version);

	QPixmap getPixmapFromHint(QVariant argument);

	Q_INVOKABLE unsigned Notify(QString app_name, unsigned id, QString icon, QString summary, QString body, QStringList actions, QVariantMap hints, int timeout);

signals:

	Q_INVOKABLE void NotificationClosed(unsigned id, unsigned reason);
};
