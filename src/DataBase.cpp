#include "DataBase.h"
#include "global.h"

#include <QFile>
#include <QStringList>
#include <QSqlError>
#include <QMetaType>
#include <QUuid>
#include <QSqlRecord>
#include <QDebug>

ScopedTransaction::ScopedTransaction(QSqlDatabase &db)
    : _db(db),
      _query(_db),
      _error(false)
{
    _db.transaction();
}

ScopedTransaction::~ScopedTransaction()
{
    if (!_error)
        _db.commit();
    else
        _db.rollback();
}

bool ScopedTransaction::exec(const QString &query)
{
    if (_error) {
        WARNING_T(core) << "already have erro!" << endl
                        << query;
        return false;
    }

    if (!_query.exec(query)) {
        WARNING_T(core) << "faile!" << endl
                        << query << endl
                        << "error =" << _query.lastError().text();
        _error = true;
        return false;
    }

    return true;
}

bool ScopedTransaction::execFile(const QString &fileName)
{
    if (_error) {
        WARNING_T(core) << "already have errro!" << endl
                        << fileName ;
        return false;
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        WARNING_T(core) << " faile!" << endl
                        << fileName;
        return false;
    }

    QString fileContents = file.readAll();
    QStringList queries = fileContents.simplified().split(";", QString::SkipEmptyParts);
    foreach(const QString & query, queries) {
        if (!_query.exec(query)) {
            WARNING_T(core) << "faile!" << endl
                            << fileName << endl
                            << "query =" << query << endl
                            << "error =" << _query.lastError().text();
            _error = true;
            return false;
        }
    }

    return true;
}

DbReader::DbReader(QObject *parent)
    : QObject(parent),
      _stop(false)
{
}

DbReader::~DbReader()
{
    QString connectionName = _db.connectionName();
    if (connectionName.size()) {
        _db.close();
        _db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
    }
}

void DbReader::stop()
{
    _stop = true;
}

void DbReader::initialize(const QSqlDatabase &db)
{
    _db = QSqlDatabase::cloneDatabase(db, QUuid::createUuid().toString());
    if (!_db.open())
        WARNING_T(core) << "opening database faile!" << endl
                        << "errStr =" << _db.lastError().text();
}

void DbReader::execute(const QString &queryStr, const QStringList &bindings, void *pUserData)
{
    QSqlQuery query(_db);
    query.setForwardOnly(true);
    query.prepare(queryStr);
    foreach(const QString & binding, bindings)
    query.addBindValue(binding);

    if (!query.exec()) {
        WARNING_T(core) << "faile!" << queryStr << endl
                        << "errStr =" << query.lastError().text();
        return;
    }

    QList<QSqlRecord> data = readRecord(query);

    if (!_stop)
        emit dataReady(this, data, pUserData);
}

QList<QSqlRecord> DbReader::readRecord(QSqlQuery &query)
{
    QList<QSqlRecord> data;
    while (query.next() && !_stop)
        data.append(query.record());

    return data;
}

