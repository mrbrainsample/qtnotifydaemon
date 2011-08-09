#include "notifyarea.h"
#include <QPixmap>

Message::Message()
{
timeout = 3000;
urgency=1;
id = 0;
app_name = "";
text = "";
header = "";
isComplete = false;
}

Message::Message(const Message &t)
{
icon = new QPixmap;
icon = t.icon;
action = t.action;
timeout = t.timeout;
urgency = t.urgency;
app_name = t.app_name;
text = t.text;
header = t.header;
isComplete = t.isComplete;
hints = t.hints;
}

Message::~Message()
{
if(icon!=NULL)
	{
	delete icon;
	icon = NULL;
	}
}


