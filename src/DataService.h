#ifndef DATASERVICE_H
#define DATASERVICE_H

#include "common.h"

#include <QObject>
#include <QSqlDatabase>
#include <QStringList>
#include <QThread>
#include <QMap>
#include <QReadWriteLock>

class ResultPageParser;

class DataService : public QObject
{
    Q_OBJECT
public:
    struct EnumResultNode{
        EnumResultNode()
            : lastSelectId(0),
              selectCount(0),
              maxNotSelectCount(0),
              maxSelectCount(0)
        {}

        inline bool isValid() const{
            return !enumResult.isEmpty();
        }

        QString enumResult;
        int lastSelectId;
        int selectCount;
        int maxNotSelectCount;
        int maxSelectCount;
    };


    explicit DataService(const QString& name, QObject *parent = 0);

    inline QString dbPath() const {
        return QString("%1/%2.db").arg(_dbDir).arg(_name);
    }

    inline QSqlDatabase mainDb(){
        return _mainDb;
    }

    inline QSqlDatabase db(){
        return _db;
    }

    inline QString name() const
    {
        return _name;
    }

    inline CaiNode firstNode() const {
        QReadLocker locker(&_lock);
        return _firstNode;
    }

    inline CaiNode lastNode() const {
        QReadLocker locker(&_lock);
        return _lastNode;
    }

    inline int parserCount() const {
        QReadLocker locker(&_lock);
        return _parserCount;
    }

    int totalResultCount() const;

    inline int ignoreCount() const {
        QReadLocker locker(&_lock);
        return _ignoreCount;
    }
    inline void setIgnoreCount(int c) {
        QWriteLocker locker(&_lock);
        _ignoreCount = qMax(0,c);
    }

    inline QReadWriteLock* lock() {
        return &_lock;
    }

public slots:
    bool reset();
    bool update();
    void abort();

protected slots:
    void updateResult(const QList<CaiNode>& nodeList);

protected:
    void initIfNeed();

signals:
    void updateFinish();
    void message(const QString& msg);

private:
    bool _delayInit;
    int _parserCount;
    int _ignoreCount;
    QString _dbDir;
    QString _name;
    QSqlDatabase _db;
    QSqlDatabase _mainDb;
    ResultPageParser *_pParser;
    CaiNode _firstNode;
    CaiNode _lastNode;
    bool _cancel;
    mutable QReadWriteLock _lock;

    static const int _resultEnumTotalCount;
};

#endif // DATASERVICE_H
