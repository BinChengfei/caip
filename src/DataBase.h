/*
* Copyright (c) 2013,长沙联远电子有限公司
* All rights reserved.
*
*
* 简述：数据库操作相关类
* 说明：
* 作   者：aFei
* 完成日期： 2013.01.10
*/

#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

class ScopedTransaction
{
public:
    ScopedTransaction(QSqlDatabase &db);
    ~ScopedTransaction();

    bool exec(const QString &query);
    bool execFile(const QString &fileName);
    QSqlQuery& query() {
        return _query;
    }

private:
    QSqlDatabase &_db;
    QSqlQuery _query;
    bool _error;
};

class DbReader : public QObject
{
    Q_OBJECT

public:
    DbReader(QObject *parent = 0);
    ~DbReader();

    void stop();

public slots:
    void initialize(const QSqlDatabase &db);
    void execute(const QString &query, const QStringList &bindings, void *pUserData = 0);
    QList<QSqlRecord> readRecord(QSqlQuery &query);

signals:
    void dataReady(DbReader *reader, const QList<QSqlRecord> &data, void *pUserData);

private:
    QSqlDatabase _db;
    volatile bool _stop;
};

#endif // DATABASE_H
