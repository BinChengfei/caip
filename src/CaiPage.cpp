#include "CaiPage.h"
#include "ui_CaiPage.h"
#include "WebContextParser.h"
#include "DataService.h"
#include "global.h"
#include "Config.h"

#include <QThread>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QTimer>

const QString CaiPage::_lastQueryStr = "SELECT result,sequence,dateTime FROM result ORDER BY sequence DESC LIMIT 0,%1";

//const QString CaiPage::_frontQueryStr = "SELECT enumResult,"
//        "(%1 - frontLastSelectId) AS currentNotSelectCount,"
//        "frontMaxNotSelectCount "
//        "FROM result_enum ORDER BY currentNotSelectCount DESC LIMIT 0,%2";

//const QString CaiPage::_backQueryStr =  "SELECT enumResult,"
//        "(%1 - backLastSelectId) AS currentNotSelectCount,"
//        "backMaxNotSelectCount "
//        "FROM result_enum ORDER BY currentNotSelectCount DESC LIMIT 0,%2";

const QString CaiPage::_frontQueryStr = "SELECT enumResult,"
        "(SELECT CASE WHEN frontLastSelectId == %1 THEN frontSelectCount ELSE 0 END) AS selectCount,"
        "frontMaxSelectCount "
        "FROM result_enum ORDER BY selectCount DESC LIMIT 0,%2";

const QString CaiPage::_backQueryStr =  "SELECT enumResult,"
        "(SELECT CASE WHEN backLastSelectId == %1 THEN backSelectCount ELSE 0 END) AS selectCount,"
        "backMaxSelectCount "
        "FROM result_enum ORDER BY selectCount DESC LIMIT 0,%2";
CaiPage::CaiPage(const QString& name, int minIgnoreCount, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaiPage),
    _name(name),
    _pWorkThread(0),
    _pDataService(0),
    _pUpdateTimer(0),
    _topCount(30)
{
    setObjectName(_name);
    ui->setupUi(this);

    _pWorkThread = new QThread(this);
    _pWorkThread->setObjectName(_name);
    _pDataService = new DataService(_name);
    int ignoreCount = Config::instance()->value(QString("%1IgnoreCount").arg(_name)).toInt();
    ignoreCount = qMax(minIgnoreCount, ignoreCount);
    INFO_QOBJECT_T(ui)<<"ignoreCount ="<<ignoreCount;
    _pDataService->setIgnoreCount(ignoreCount);
    _pDataService->moveToThread(_pWorkThread);
    connect(_pWorkThread, SIGNAL(started()), _pDataService, SLOT(update()));
    connect(_pDataService, SIGNAL(updateFinish()), this, SLOT(updateFinish()));
    connect(_pDataService, SIGNAL(message(QString)), this, SIGNAL(message(QString)));
    _pWorkThread->start();

    _pUpdateTimer = new QTimer(this);
    int updateTime = Config::instance()->value(QString("%1UpdateTime").arg(_name), 60 * 3).toInt() * 1000;
    updateTime = qBound(1000 * 60 * 1, updateTime, 1000 * 60 * 5);
    INFO_QOBJECT_T(ui)<<"updateTime ="<<updateTime;

    _pUpdateTimer->setInterval(updateTime);
    connect(_pUpdateTimer, SIGNAL(timeout()), this, SLOT(update()));
    _pUpdateTimer->start();

    ui->lastSequenceTableView->setModel(&_lastModel);
    ui->enumSequenceFrontTableView->setModel(&_enumFrontModel);
    ui->enumSequenceBackTableView->setModel(&_enumBackModel);

    _topCount = Config::instance()->value("topCount", 30).toInt();
    _topCount = qBound(1, _topCount, 256);
    INFO_QOBJECT_T(ui)<<"topCount ="<<_topCount;


    connect(ui->updatePushButton, SIGNAL(clicked()), this, SLOT(update()));
}

CaiPage::~CaiPage()
{
    _pUpdateTimer->stop();
    delete ui;
    _pDataService->abort();
    _pDataService->deleteLater();
    _pWorkThread->quit();
    _pWorkThread->wait(9000);
}

void CaiPage::reset()
{
    DEBUG_QOBJECT_T(ui);
    QMetaObject::invokeMethod(_pDataService, "reset", Qt::QueuedConnection);
}

void CaiPage::update()
{
    DEBUG_QOBJECT_T(ui);
    QMetaObject::invokeMethod(_pDataService, "update", Qt::QueuedConnection);
}

void CaiPage::updateFinish()
{
    DEBUG_QOBJECT_T(ui);
    if(!_pDataService->lock()->tryLockForRead()){
        WARNING_QOBJECT_T(ui)<<"db is busy!";
        return;
    }

    ui->resultRangeLabel->setText(QStringLiteral("(%1 ~ %2 期数:%3)")
                                  .arg(_pDataService->firstNode().sequence)
                                  .arg(_pDataService->lastNode().sequence)
                                  .arg(_pDataService->parserCount()));

    _lastModel.setQuery(_lastQueryStr.arg(_topCount), _pDataService->mainDb());
    _lastModel.setHeaderData(0, Qt::Horizontal, QStringLiteral("开奖结果"));
    _lastModel.setHeaderData(1 , Qt::Horizontal,QStringLiteral("开奖期数"));
    _lastModel.setHeaderData(2, Qt::Horizontal,QStringLiteral("开奖时间"));

    ui->lastSequenceTableView->setColumnWidth(0,60);
    ui->lastSequenceTableView->setColumnWidth(1,90);
    ui->lastSequenceTableView->setColumnWidth(2,140);

    int toatolCount = _pDataService->parserCount();
    _enumFrontModel.setQuery(_frontQueryStr
                             .arg(toatolCount)
                             .arg(_topCount), _pDataService->mainDb());
    _enumFrontModel.setHeaderData(0, Qt::Horizontal, QStringLiteral("万 千 百"));
    _enumFrontModel.setHeaderData(1 , Qt::Horizontal,QStringLiteral("当前"));
    _enumFrontModel.setHeaderData(2, Qt::Horizontal,QStringLiteral("历史"));

    ui->enumSequenceFrontTableView->setColumnWidth(0,60);
    ui->enumSequenceFrontTableView->setColumnWidth(1,40);
    ui->enumSequenceFrontTableView->setColumnWidth(2,40);

    _enumBackModel.setQuery(_backQueryStr
                            .arg(toatolCount)
                            .arg(_topCount), _pDataService->mainDb());
    _enumBackModel.setHeaderData(0, Qt::Horizontal,   QStringLiteral("百 十 个"));
    _enumBackModel.setHeaderData(1 , Qt::Horizontal,QStringLiteral("当前"));
    _enumBackModel.setHeaderData(2, Qt::Horizontal,QStringLiteral("历史"));

    ui->enumSequenceBackTableView->setColumnWidth(0,60);
    ui->enumSequenceBackTableView->setColumnWidth(1,40);
    ui->enumSequenceBackTableView->setColumnWidth(2,40);

    if(_pDataService->parserCount() < _pDataService->totalResultCount())
        QMetaObject::invokeMethod(_pDataService, "update", Qt::QueuedConnection);

    _pDataService->lock()->unlock();
}
