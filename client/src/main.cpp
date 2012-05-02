#include <qstringlist.h>
#include <QtGui\qapplication.h>
#include "tcclientctrl.h"
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QStringList args = a.arguments();

    tcClientCtrl ctrl(&a);
    if(args.size() == 2)
    {
        ctrl.initialize(args[1]);
    }

    return a.exec();
}
