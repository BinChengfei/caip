#include "global.h"
#include "qtsingleapplication.h"
#include "log4qt.h"
#include "propertyconfigurator.h"
#include "MainWindow.h"


#include <QApplication>
#include <QThread>
#include <QTimer>

int main(int argc, char *argv[])
{
    QtSingleApplication app(argc, argv);

    QThread::currentThread()->setObjectName("main");

    Log4Qt::PropertyConfigurator::configure(
                QtSingleApplication::applicationDirPath()  + "/log4qt.conf"
                );
    if (app.isRunning()) {
        WARNING_T(core) << "BA System is already running, exit!";

        return -1;
    }

    MainWindow window;
    window.show();

    int re = app.exec();

    return re;
}
