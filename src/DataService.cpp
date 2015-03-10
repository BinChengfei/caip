#include "DataService.h"
#include "global.h"
#include "DataBase.h"
#include "global.h"
#include "WebContextParser.h"

#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlField>
#include <QMetaType>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>
#include <QSqlError>
#include <QCoreApplication>
#include <QDir>
#include <QThread>
#include <QSqlError>
#include <QMetaObject>
#include <QUuid>


static void configDbPragma(QSqlDatabase db)
{
    QSqlQuery query(db);
    query.exec("PRAGMA journal_mode=WAL");
    query.clear();
    query.exec("PRAGMA synchronous=NORMAL");
    //query.clear();
    //query.exec("PRAGMA locking_mode=EXCLUSIVE");
    query.clear();
    query.exec("PRAGMA temp_store=MEMORY");
    query.clear();
    query.exec("PRAGMA cache_size=81960");
}

const int DataService::_resultEnumTotalCount = 91125;

DataService::DataService(const QString& name, QObject *parent) :
    QObject(parent),
    _delayInit(false),
    _parserCount(0),
    _ignoreCount(0),
    _dbDir(QCoreApplication::applicationDirPath() + "/data"),
    _name(name),
    _pParser(0),
    _cancel(false),
    _lock(QReadWriteLock::Recursive)

{
    if (!QCoreApplication::instance())
        WARNING_QOBJECT_T(core) << "please create QCoreApplication instance first!";

    qRegisterMetaType<QList<QSqlRecord> >("QList<QSqlRecord>");
    qRegisterMetaType<QSqlDatabase>("QSqlDatabase");
    qRegisterMetaType<QSqlQuery>("QSqlQuery");
    qRegisterMetaType<DbReader *>("DbReader *");
    qRegisterMetaType<void *>();
    qRegisterMetaType<CaiNode>("CaiNode");
    qRegisterMetaType<QList<CaiNode> >("QList<CaiNode>");

    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        WARNING_QOBJECT_T(core) << "he SQLITE driver is unavailable";
        return;
    }

    QString dbDirPath = QFileInfo(dbPath()).absolutePath();
    QDir dir(dbDirPath);
    if (!dir.exists()) {
        if (!dir.mkpath(dbDirPath)) {
            WARNING_QOBJECT_T(core) << "create " << dbDirPath << "faile!";
            return;
        }
    }

    _mainDb = QSqlDatabase::addDatabase("QSQLITE", _name);
    _mainDb.setDatabaseName(dbPath());
    if (!_mainDb.open()) {
        WARNING_QOBJECT_T(core) << "Failed to open SQLITE database!"
                                << endl << "dataName =" << dbPath()
                                << endl << "errorString =" << _mainDb.lastError().text();
        return;
    }

    configDbPragma(_mainDb);

    if (_mainDb.tables().isEmpty()) {
        ScopedTransaction transaction(_mainDb);
        if (!transaction.execFile(":/sql")) {
            WARNING_QOBJECT_T(core) << "create table faile!"
                                    << endl << "dbPath =" << dbPath()
                                    << endl << "errorString =" << _mainDb.lastError().text();

            return;
        }

        INFO_QOBJECT_T(core) << "create table ok!";
    }

    DEBUG_QOBJECT_T(core) << "init ok! dbPath =" << dbPath();
}


int DataService::totalResultCount() const {
    QReadLocker locker(&_lock);
    return _pParser && _pParser->totalResultCount()
            ? _pParser->totalResultCount() : 0xffffff;
}

void DataService::initIfNeed() {
    if(_delayInit) return;

    INFO_QOBJECT_T(data);
    if(thread() != QCoreApplication::instance()->thread()) {
        _db = QSqlDatabase::cloneDatabase(_mainDb, QUuid::createUuid().toString());
        if (!_db.open()) {
            WARNING_QOBJECT_T(core) << "failed to open database!"
                                    << _db.lastError().text();
        }
    }
    else
        _db = _mainDb;

    configDbPragma(_db);

    _delayInit = true;

}

void DataService::abort(){
    DEBUG_QOBJECT_T(data);
    _cancel = true;
    if(_pParser)
        _pParser->abort();
}

bool DataService::update()
{
    _cancel = false;
    if(!_lock.tryLockForRead()) {
        emit message("DataService::reset db is busy, try again!");
        return false;
    }
    _lock.unlock();
    QReadLocker locker(&_lock);

    initIfNeed();

    if(!_pParser) {
        QSqlQuery query(_db);
        if(!query.exec(QString("SELECT COUNT(*) FROM result_enum"))){
            WARNING_QOBJECT_T(data) << "sql SELECT faile!"
                                    << "sqlStr =" << query.lastQuery()
                                    << "err =" << query.lastError().text();
            return false;
        }
        int totalCount = 0;
        if(query.next())
            totalCount = query.record().value(0).toInt();
        if(totalCount != _resultEnumTotalCount ){
            return QMetaObject::invokeMethod(this, "reset", Qt::QueuedConnection);
        }

        locker.unlock();
        QWriteLocker locker2(&_lock);
        if(_pParser && _pParser->isRunning()) {
            WARNING_QOBJECT_T(data)<<"parser is running";
            return false;
        }

        DEBUG_QOBJECT_T(data)<<_name;

        query.clear();
        if(!query.exec(QString("SELECT COUNT(*) FROM result"))){
            WARNING_QOBJECT_T(data) << "sql SELECT faile!"
                                    << "sqlStr =" << query.lastQuery()
                                    << "err =" << query.lastError().text();
            return false;
        }

        if(query.next())
            _parserCount = query.record().value(0).toInt();

        if(!_pParser){
            _pParser = new ResultPageParser(_name, 0, this);
            connect(_pParser,SIGNAL(finish(QList<CaiNode>)), this, SLOT(updateResult(QList<CaiNode>)));
        }

        query.clear();
        if(!query.exec(QString("SELECT * FROM result ORDER BY id limit 1"))){
            WARNING_QOBJECT_T(data) << "sql SELECT faile!"
                                    << "sqlStr =" << query.lastQuery()
                                    << "err =" << query.lastError().text();
            return false;
        }
        if(query.next()){
            _firstNode.dateTime = query.record().value("dateTime").toString();
            _firstNode.result = query.record().value("result").toString();
            _firstNode.sequence = query.record().value("sequence").toString();\

            query.clear();
            if(!query.exec(QString("SELECT * FROM result ORDER BY id DESC limit 1"))){
                WARNING_QOBJECT_T(data) << "sql SELECT faile!"
                                        << "sqlStr =" << query.lastQuery()
                                        << "err =" << query.lastError().text();
                return false;
            }
            if(query.next()){
                _lastNode.dateTime = query.record().value("dateTime").toString();
                _lastNode.result = query.record().value("result").toString();
                _lastNode.sequence = query.record().value("sequence").toString();
            }
        }

        INFO_QOBJECT_T(core)<<"_parserCount ="<<_parserCount
                           <<"_ignoreCount ="<<_ignoreCount;

        emit updateFinish();

        return true;
    }
    INFO_QOBJECT_T(core)<<"_parserCount ="<<_parserCount
                       <<"_ignoreCount ="<<_ignoreCount;

    QMetaObject::invokeMethod(_pParser, "parser", Qt::QueuedConnection, Q_ARG(int,_parserCount + _ignoreCount));

    return true;
}

static QString nextEnum(const QString& currentEnum = QString())
{
    if(currentEnum.isEmpty())
        return "01 01 01";

    QString tmpCurrentEnum = currentEnum;
    tmpCurrentEnum.remove(' ');

    if(tmpCurrentEnum.size() != 6) {
        WARNING_T(data)<<"invliad arg"<<tmpCurrentEnum;
        return QString();
    }

    QString newEnum = tmpCurrentEnum;
    for(int i = 5; i >= 0; --i) {
        if(!tmpCurrentEnum.at(i).isNumber()){
            WARNING_T(data)<<"invliad arg"<<tmpCurrentEnum;
            return QString();
        }

        if(tmpCurrentEnum.at(i)  <  (i % 2 ? '9' : '8')) {
            newEnum[i] = newEnum.at(i).toLatin1()+ 1;
            if(i < 5) {
                int j = i + 1;
                if(i % 2 == 0) {
                    newEnum[j] = newEnum.at(i).toLatin1()+ 1;
                    ++j;
                }

                for(; j <= 5; ++j)
                    newEnum[j] = (j % 2) ? '1' : '0';
            }
            newEnum.insert(2,' ');
            newEnum.insert(5,' ');
            return newEnum;
        }
    }

    WARNING_T(data)<<"is last one, not next enum"<<tmpCurrentEnum;

    return QString();
}

bool DataService::reset()
{    
    _cancel = false;
    if(!_lock.tryLockForWrite()) {
        emit message("DataService::reset db is busy, try again!");
        return false;
    }
    _lock.unlock();
    QWriteLocker locker(&_lock);

    initIfNeed();

    INFO_QOBJECT_T(data)<<_name;

    DEBUG_QOBJECT_T(data)<<"clear data"<<_name;

    if(_pParser && _pParser->isRunning()) _pParser->abort();
    QSqlQuery query(_db);
    if(!query.exec(QString("DELETE FROM result_enum"))){
        emit message("DataService::reset delete result_enumerror!");
        WARNING_QOBJECT_T(data) << "sql delete faile!"
                                << endl << "sqlStr =" << query.lastQuery()
                                << endl << "err =" << query.lastError().text();
        return false;
    }

    query.clear();
    if(!query.exec(QString("DELETE FROM result"))){
        emit message("DataService::reset delete result error!");
        WARNING_QOBJECT_T(data) << "sql delete faile!"
                                << endl << "sqlStr =" << query.lastQuery()
                                << endl << "err =" << query.lastError().text();

        return false;
    }

    QString sqlStr = QString("INSERT INTO result_enum (enumResult) VALUES (:enumResult)");
    QString enumResult = nextEnum();

    DEBUG_QOBJECT_T(data)<<"init enum result"<<_name;
    while(enumResult.size() && !_cancel) {
        query.clear();
        DEBUG_QOBJECT_T(data)<<"add enum"<<enumResult;

        emit message(QString("DataService::reset add enum %2").arg(enumResult));
        if (!query.prepare(sqlStr)){
            WARNING_QOBJECT_T(data) << "sql insert prepare faile!"
                                    << endl << "sqlStr =" << query.lastQuery()
                                    << endl << "err =" << query.lastError().text();

            return false;
        }

        query.bindValue(":enumResult", enumResult);

        if (!query.exec()) {
            WARNING_QOBJECT_T(data) << "sql insert faile!"
                                    << endl << "sqlStr =" << query.lastQuery()
                                    << endl << "err =" << query.lastError().text();
            return false;
        }

        enumResult = nextEnum(enumResult);
    }
    _parserCount = 0;
    _firstNode = CaiNode();
    _lastNode = CaiNode();
    emit  updateFinish();

    emit message("DataService::reset finish.");

    DEBUG_QOBJECT_T(data)<<"finish."<<_name;

    return true;
}

void DataService::updateResult(const QList<CaiNode>& nodeList)
{
    if(_cancel) return;

    QWriteLocker lock(&_lock);

    DEBUG_QOBJECT_T(data);
    int updateCount = 0;
    for(const CaiNode& resultNode : nodeList) {
        if(_cancel) break;

        DEBUG_QOBJECT_T(data)<< resultNode.result<<resultNode.sequence;
        ScopedTransaction transaction(_db);
        QSqlQuery &query = transaction.query();
        if (!query.exec(QString("SELECT * FROM result WHERE sequence = '%1'")
                        .arg(resultNode.sequence))) {
            emit message("DataService::updateResult db error!");

            WARNING_QOBJECT_T(data) << "sql select faile!"
                                    << endl << "sqlStr =" << query.lastQuery()
                                    << endl << "err =" << query.lastError().text();
            return;
        }else{
            if(query.next()) continue;
        }

        ++updateCount;
        emit message(QString("DataService::updateResult: update result %1 %2")
                     .arg(resultNode.result)
                     .arg(resultNode.sequence));

        query.clear();
        if (!query.exec(QString("INSERT INTO result (result, dateTime, sequence)"
                                " VALUES ('%1', '%2', '%3')")
                        .arg(resultNode.result)
                        .arg(resultNode.dateTime)
                        .arg(resultNode.sequence))) {
            emit message("DataService::updateResult db error!");
            WARNING_QOBJECT_T(data) << "sql insert faile!"
                                    << endl << "sqlStr =" << query.lastQuery()
                                    << endl << "err =" << query.lastError().text();
            return;
        }

        //front
        QHash<QString, EnumResultNode> enumResultNodeHash;
        if(!query.exec(QString("SELECT * FROM result_enum WHERE enumResult LIKE '%1'")
                       .arg(QString("%%1% __ __").arg(resultNode.result.at(0))))) {
            WARNING_QOBJECT_T(data) << "sql select faile!"
                                    << "sqlStr =" << query.lastQuery()
                                    << "err =" << query.lastError().text();
            emit message("DataService::updateResult db error!");
            return;
        }

        while(query.next()) {
            EnumResultNode tmpNode;
            QSqlRecord record =  query.record();
            tmpNode.enumResult = record.value("enumResult").toString();
            tmpNode.lastSelectId = record.value("frontLastSelectId").toInt();
            tmpNode.selectCount = record.value("frontSelectCount").toInt();
            tmpNode.maxNotSelectCount =  record.value("frontMaxNotSelectCount").toInt();
            tmpNode.maxSelectCount =  record.value("frontMaxSelectCount").toInt();
            enumResultNodeHash.insert(tmpNode.enumResult, tmpNode);
        }

        query.clear();
        if(!query.exec(QString("SELECT * FROM result_enum WHERE enumResult LIKE '%1'")
                       .arg(QString("__ %%1% __").arg(resultNode.result.at(1))))) {
            WARNING_QOBJECT_T(data) << "sql select faile!"
                                    << "sqlStr =" << query.lastQuery()
                                    << "err =" << query.lastError().text();
            emit message("DataService::updateResult db error!");
            return;
        }
        while(query.next()) {
            EnumResultNode tmpNode;
            QSqlRecord record =  query.record();
            tmpNode.enumResult = record.value("enumResult").toString();
            tmpNode.lastSelectId = record.value("frontLastSelectId").toInt();
            tmpNode.selectCount = record.value("frontSelectCount").toInt();
            tmpNode.maxNotSelectCount =  record.value("frontMaxNotSelectCount").toInt();
            tmpNode.maxSelectCount =  record.value("frontMaxSelectCount").toInt();
            enumResultNodeHash.insert(tmpNode.enumResult, tmpNode);
        }
        query.clear();
        if(!query.exec(QString("SELECT * FROM result_enum WHERE enumResult LIKE '%1'")
                       .arg(QString("__ __ %%1%").arg(resultNode.result.at(2))))) {
            WARNING_QOBJECT_T(data) << "sql select faile!"
                                    << "sqlStr =" << query.lastQuery()
                                    << "err =" << query.lastError().text();
            return;
        }
        while(query.next()) {
            EnumResultNode tmpNode;
            QSqlRecord record =  query.record();
            tmpNode.enumResult = record.value("enumResult").toString();
            tmpNode.lastSelectId = record.value("frontLastSelectId").toInt();
            tmpNode.selectCount = record.value("frontSelectCount").toInt();
            tmpNode.maxNotSelectCount =  record.value("frontMaxNotSelectCount").toInt();
            tmpNode.maxSelectCount =  record.value("frontMaxSelectCount").toInt();
            enumResultNodeHash.insert(tmpNode.enumResult, tmpNode);
        }
        for(const EnumResultNode& tmpNode : enumResultNodeHash){
            query.clear();
            int selectCount = (tmpNode.lastSelectId == _parserCount ?  tmpNode.selectCount + 1 : 1);
            if(!query.exec(QString("UPDATE result_enum SET frontMaxNotSelectCount = %1, frontLastSelectId = %2,"
                                   "frontSelectCount = %3, frontMaxSelectCount = '%4' WHERE enumResult = '%5'")
                           .arg(tmpNode.maxNotSelectCount < _parserCount -  tmpNode.lastSelectId
                                ? _parserCount -  tmpNode.lastSelectId : tmpNode.maxNotSelectCount)
                           .arg(_parserCount + 1)
                           .arg(selectCount)
                           .arg(tmpNode.lastSelectId == _parserCount && tmpNode.selectCount < selectCount
                                ? selectCount : tmpNode.selectCount)
                           .arg(tmpNode.enumResult))) {
                WARNING_QOBJECT_T(data) << "sql select faile!"
                                        << "sqlStr =" << query.lastQuery()
                                        << "err =" << query.lastError().text();
                emit message("DataService::updateResult db error!");
                return;
            }
        }

        //back
        query.clear();
        enumResultNodeHash.clear();
        if(!query.exec(QString("SELECT * FROM result_enum  WHERE enumResult LIKE '%1'")
                       .arg(QString("%%1% __ __").arg(resultNode.result.at(2))))) {
            WARNING_QOBJECT_T(data) << "sql select faile!"
                                    << "sqlStr =" << query.lastQuery()
                                    << "err =" << query.lastError().text();

            emit message("DataService::updateResult db error!");
            return;
        }
        while(query.next()) {
            EnumResultNode tmpNode;
            QSqlRecord record =  query.record();
            tmpNode.enumResult = record.value("enumResult").toString();
            tmpNode.lastSelectId = record.value("backLastSelectId").toInt();
            tmpNode.selectCount = record.value("backSelectCount").toInt();
            tmpNode.maxNotSelectCount =  record.value("backMaxNotSelectCount").toInt();
            tmpNode.maxSelectCount =  record.value("backMaxSelectCount").toInt();
            enumResultNodeHash.insert(tmpNode.enumResult, tmpNode);
        }
        query.clear();
        if(!query.exec(QString("SELECT * FROM result_enum WHERE enumResult LIKE '%1'")
                       .arg(QString("__ %%1% __").arg(resultNode.result.at(3))))) {
            WARNING_QOBJECT_T(data) << "sql select faile!"
                                    << "sqlStr =" << query.lastQuery()
                                    << "err =" << query.lastError().text();
            emit message("DataService::updateResult db error!");
            return;
        }
        while(query.next()) {
            EnumResultNode tmpNode;
            QSqlRecord record =  query.record();
            tmpNode.enumResult = record.value("enumResult").toString();
            tmpNode.lastSelectId = record.value("backLastSelectId").toInt();
            tmpNode.selectCount = record.value("backSelectCount").toInt();
            tmpNode.maxNotSelectCount =  record.value("backMaxNotSelectCount").toInt();
            tmpNode.maxSelectCount =  record.value("backMaxSelectCount").toInt();
            enumResultNodeHash.insert(tmpNode.enumResult, tmpNode);
        }
        query.clear();
        if(!query.exec(QString("SELECT * FROM result_enum WHERE enumResult LIKE '%1'")
                       .arg(QString("__ __ %%1%").arg(resultNode.result.at(4))))) {
            WARNING_QOBJECT_T(data) << "sql select faile!"
                                    << "sqlStr =" << query.lastQuery()
                                    << "err =" << query.lastError().text();
            emit message("DataService::updateResult db error!");
            return;
        }
        while(query.next()) {
            EnumResultNode tmpNode;
            QSqlRecord record =  query.record();
            tmpNode.enumResult = record.value("enumResult").toString();
            tmpNode.lastSelectId = record.value("backLastSelectId").toInt();
            tmpNode.selectCount = record.value("backSelectCount").toInt();
            tmpNode.maxNotSelectCount =  record.value("backMaxNotSelectCount").toInt();
            tmpNode.maxSelectCount =  record.value("backMaxSelectCount").toInt();
            enumResultNodeHash.insert(tmpNode.enumResult, tmpNode);
        }
        for(const EnumResultNode& tmpNode : enumResultNodeHash){
            query.clear();
            int selectCount = (tmpNode.lastSelectId == _parserCount ?  tmpNode.selectCount + 1 : 1);
            if(!query.exec(QString("UPDATE result_enum SET backMaxNotSelectCount = %1, backLastSelectId = %2,"
                                   "backSelectCount = '%3', backMaxSelectCount = '%4' WHERE enumResult = '%5'")
                           .arg(tmpNode.maxNotSelectCount <  _parserCount -  tmpNode.lastSelectId
                                ?_parserCount -  tmpNode.lastSelectId : tmpNode.maxNotSelectCount)
                           .arg(_parserCount + 1)
                           .arg(selectCount)
                           .arg(tmpNode.lastSelectId == _parserCount && tmpNode.selectCount < selectCount
                                ? selectCount : tmpNode.selectCount)
                           .arg(tmpNode.enumResult))) {
                WARNING_QOBJECT_T(data) << "sql select faile!"
                                        << "sqlStr =" << query.lastQuery()
                                        << "err =" << query.lastError().text();
                emit message("DataService::updateResult db error!");
                return;
            }
        }
        if(_firstNode.result.isEmpty())
            _firstNode = resultNode;
        _lastNode = resultNode;

        ++_parserCount;

        INFO_QOBJECT_T(data)<<"node ="<<_lastNode.result<<_lastNode.sequence<<_lastNode.dateTime
                           <<"_parserCount ="<<_parserCount;
    }

    if(!updateCount) {
        if(_parserCount + _ignoreCount < _pParser->totalResultCount()) {
            WARNING_QOBJECT_T(data) << "should not happen! ignore auto +1"
                                    <<"_parserCount ="<<_parserCount
                                   <<"_ignoreCount ="<<_ignoreCount
                                  <<"totalCount ="<<_pParser->totalResultCount();
            emit message("data error!");
            ++_ignoreCount;
            INFO_QOBJECT_T(data) << "new _ignoreCount = "<<_ignoreCount;
            lock.unlock();
            emit updateFinish();
        }else{
            emit message("no update!");
            INFO_QOBJECT_T(data) << "not find can update node!"<<"_parserCount ="<<_parserCount
                                 <<"totalResultCount ="<<_pParser->totalResultCount();
        }
    }
    else{
        lock.unlock();
        emit updateFinish();
    }
}
