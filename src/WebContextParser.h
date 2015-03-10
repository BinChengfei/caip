#ifndef WEBCONTEXTPARSER_H
#define WEBCONTEXTPARSER_H

#include "common.h"

#include <QObject>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class WebContext : public QObject
{
    Q_OBJECT
public:
    explicit WebContext(QObject *parent = 0);
    explicit WebContext(const QString& url, QObject *parent = 0);
    ~WebContext();

    inline QString url() const {
        return _url;
    }
    inline void setUrl(const QString& url){
        _url = url;
    }

    inline QByteArray data() const {
        return _data;
    }
    inline bool isUpdating() const {
        return _running;
    }

    inline QNetworkAccessManager* networkAccessManager() const{
        return _pNetworkManager;
    }
    inline void setNetworkAccessManager(QNetworkAccessManager *pManager){
        _pNetworkManager = pManager;
    }

    inline static QNetworkAccessManager* defaultNetworkAccessManager(){
        return _pDefaultNetworkManager;
    }

public slots:
    bool flush();

protected slots:
    void httpRead();
    void httpFinish();
    void httpError(QNetworkReply::NetworkError e);

signals:
    void flushed(const QByteArray& data);
    void error(const QString& msg);

private:
    QString _url;
    bool _running;
    QNetworkReply *_pReply;
    QByteArray _data;

    QNetworkAccessManager *_pNetworkManager;

    static QNetworkAccessManager *_pDefaultNetworkManager;
};

class WebContextParser : public QObject
{
    Q_OBJECT
public:
    explicit WebContextParser(QObject *parent = 0);
    ~WebContextParser();

    inline bool isRunning() const {
        return _running;
    }

    inline void setNetworkAccessManager(QNetworkAccessManager *pManager){
        _pNetworkManager = pManager;
    }


public slots:
    virtual bool parser() = 0;
    inline virtual void abort() {
        _abort = true;
    }

signals:
    void error(const QString& msg);
    void finish(int s);

protected:
    bool _running;
    bool _abort;
    QNetworkAccessManager *_pNetworkManager;
};

class ResultPageParser : public WebContextParser
{
    Q_OBJECT
public:
    using WebContextParser::parser;
    using WebContextParser::finish;

    explicit ResultPageParser(const QString& name,int oldCount = 0, QObject *parent = 0);


    inline QString name() const
    {
        return _name;
    }
    inline int totalResultCount() const {
        return _totalResultCount;
    }

public slots:
    bool parser();
    bool parser(int oldCount);

protected slots:
    void parserTopPage();
    void parserOtherPage();
    void pageParserFinish();
    void contextError(const QString& msg);

signals:
    void finish(const QList<CaiNode>& nodeList);

protected:
    bool _parserPage(const QByteArray& data);

private:
    QString _name;
    QString _url;

    int _oldCount;
    int _totalPageCount;
    int _totalResultCount;
    int _oldTotalPageCount;
    int _oldTotalResultCount;
    int _nextParserPage;
    int _currentParserCount;
    int _errorCount;
    WebContext *_pWebContext;

    QList<CaiNode> _lastNodeList;
    static const QString _codecName;
    static const int _perPageNodeCount;
};

#endif // WEBCONTEXTPARSER_H
