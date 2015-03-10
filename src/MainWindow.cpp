#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CaiPage.h"
#include "WebContextParser.h"
#include "DataService.h"
#include "global.h"
#include "Config.h"

#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _pUi(new Ui::MainWindow),
    _pCqPage(0),
    _pJxPage(0),
    _pTjPage(0),
    _pXjPage(0),
    _pHljPage(0),
    _pYnPage(0),
    _pFjPage(0),
    _pCurrentPage(0)
{
    _pUi->setupUi(this);


    QStringList cityList = Config::instance()->value("cityList").toStringList();
    if(cityList.isEmpty())
        cityList<<"cq";

    INFO_QOBJECT_T(ui)<<"cityList ="<<cityList;

    if(cityList.contains("cq")) {
        _pCqPage = new CaiPage("cq",0, this);
        _pUi->contextWidget->addTab(_pCqPage, QStringLiteral("重庆"));
    }

    if(cityList.contains("jx")) {
        _pJxPage = new CaiPage("jx",48, this);
        _pUi->contextWidget->addTab(_pJxPage, QStringLiteral("江西"));
    }

    if(cityList.contains("tj")) {
        _pTjPage = new CaiPage("tj", 1, this);
        _pUi->contextWidget->addTab(_pTjPage, QStringLiteral("天津"));
    }

    //    if(cityList.contains("xj")) {
    //        _pXjPage = new CaiPage("xj",this);
    //        _pUi->contextWidget->addTab(_pXjPage, QStringLiteral("新疆"));
    //    }

    //    if(cityList.contains("hlj")) {
    //        _pHljPage = new CaiPage("hlj", this);
    //        _pUi->contextWidget->addTab(_pHljPage, QStringLiteral("黑龙江"));
    //    }


    //    if(cityList.contains("yn")) {
    //        _pYnPage = new CaiPage("yn",this);
    //        _pUi->contextWidget->addTab(_pYnPage, QStringLiteral("云南"));
    //    }

    //    if(cityList.contains("fj")) {
    //        _pFjPage = new CaiPage("fj", this);
    //        _pUi->contextWidget->addTab(_pFjPage, QStringLiteral("福建"));
    //}

    connect(_pUi->contextWidget,  SIGNAL(currentChanged(int)), this, SLOT(tabChange(int)));
    connect(_pUi->quitAction, SIGNAL(triggered()), QApplication::instance() , SLOT(quit()));
    connect(_pUi->dataReset, SIGNAL(triggered()), this , SLOT(resetDb()));
    connect(_pUi->helpAction, SIGNAL(triggered(bool)), this , SLOT(showHelpPage()));

    _pCurrentPage = qobject_cast<CaiPage *>(_pUi->contextWidget->currentWidget());
    connect(_pCurrentPage, SIGNAL(message(QString)), this , SLOT(updateMessage(QString)));
}

MainWindow::~MainWindow()
{
    delete _pUi;
}

void MainWindow::tabChange(int t)
{
    CaiPage *pPage = qobject_cast<CaiPage *>(_pUi->contextWidget->widget(t));
    disconnect(_pCurrentPage, SIGNAL(message(QString)), this, SLOT(updateMessage(QString)));
    updateMessage(QString());
    connect(pPage, SIGNAL(message(QString)), this , SLOT(updateMessage(QString)));
    _pCurrentPage = pPage;
}

void MainWindow::updateMessage(const QString& msg)
{
    _pUi->messageLabel->setText(msg);
}

void MainWindow::resetDb()
{
    DEBUG_QOBJECT_T(ui);
    int re = QMessageBox::warning(this,
                                  QStringLiteral("数据重置"),
                                  QStringLiteral("该操作比较耗时,确认操作？"),
                                  QMessageBox::Ok | QMessageBox::Cancel);

    if(re == QMessageBox::Ok){
        INFO_QOBJECT_T(ui);

        CaiPage *pPage = qobject_cast<CaiPage *>(_pUi->contextWidget->currentWidget());
        QMetaObject::invokeMethod(pPage, "reset", Qt::QueuedConnection);
    }
}

void MainWindow::showHelpPage()
{

}
