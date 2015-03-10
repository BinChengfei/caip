//#include "WebContextParser.h"
//#include "global.h"
//#include "qtsingleapplication.h"
//#include "log4qt.h"
//#include "propertyconfigurator.h"
//#include "DataService.h"

//#include <QApplication>
//#include <QTextCodec>
//#include <QThread>
//#include <QDebug>
//#include <QTimer>

//int main(int argc, char *argv[])
//{
//    QtSingleApplication app(argc, argv);

//    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

//    QThread::currentThread()->setObjectName("main");

//    Log4Qt::PropertyConfigurator::configure(
//                QtSingleApplication::applicationDirPath()  + "/log4qt.conf"
//                );
//    if (app.isRunning()) {
//        WARNING_T(core) << "BA System is already running, exit!";

//        return -1;
//    }


//    qDebug()<<QString("%%1%").arg("x");

//    return 0;
//}
