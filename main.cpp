#include "Widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   Widget w;
   w.show();
   w.wakeupTime = QDateTime::currentDateTime();
   return a.exec();
}
