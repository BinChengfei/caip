#include "WebContextParser.h"
#include "global.h"
#include "DataService.h"

#include <QRegExp>
#include <QStringList>
#include <QTextCodec>
#include <QMetaObject>
#include <QThread>

QNetworkAccessManager *WebContext::_pDefaultNetworkManager;

WebContext::WebContext(const QString& url, QObject *parent)
    :QObject(parent),
      _url(url),
      _running(false),
      _pReply(0),
      _pNetworkManager(0)
{
    if(!_pDefaultNetworkManager)
        _pDefaultNetworkManager = new QNetworkAccessManager;
}

WebContext::WebContext(QObject *parent)
    :WebContext(QString(), parent)
{

}

WebContext::~WebContext()
{
    if(_pReply) delete _pReply;
}

void WebContext::httpRead()
{
    TRACE_QOBJECT_T(web);

    _data.append(_pReply->readAll());
}

void WebContext::httpFinish()
{
    if (!_pReply || _pReply->error() != QNetworkReply::NoError)
        return;

    TRACE_QOBJECT_T(web);

    _pReply->deleteLater();
    _pReply = 0;
    _running = false;

    emit flushed(_data);
}

void WebContext::httpError(QNetworkReply::NetworkError e)
{
    _data.clear();

    if(!_pReply) return;
    WARNING_QOBJECT_T(web)<<e<<_pReply->errorString();
    QString errStr = _pReply->errorString();
    _pReply->deleteLater();
    _pReply = 0;
    _running = false;

    emit error(errStr);
}

bool WebContext::flush()
{
    if(_running) {
        DEBUG_QOBJECT_T(web) <<"already is running!";
        return false;
    }

    if(_url.isEmpty()) {
        WARNING_QOBJECT_T(web)<<"url is empty!";
        error("WebContext::flush(), url is empty!");
        return false;
    }

    if(!_pNetworkManager) _pNetworkManager = _pDefaultNetworkManager;

    _pReply = _pNetworkManager->get(QNetworkRequest(_url));
    connect(_pReply, SIGNAL(finished()),  this, SLOT(httpFinish()));
    connect(_pReply, SIGNAL(readyRead()), this, SLOT(httpRead()));
    connect(_pReply, SIGNAL(error(QNetworkReply::NetworkError)),  this, SLOT(httpError(QNetworkReply::NetworkError)));

    _data.clear();

    DEBUG_QOBJECT_T(web) <<_url;

    return true;
}


WebContextParser::WebContextParser(QObject *parent)
    :QObject(parent),
      _running(false),
      _abort(false),
      _pNetworkManager(0)
{
}


WebContextParser::~WebContextParser()
{
    if(_pNetworkManager)
        _pNetworkManager->deleteLater();
}

const QString ResultPageParser::_codecName = "gb2312";
const int ResultPageParser::_perPageNodeCount = 30;


ResultPageParser::ResultPageParser(const QString& name,int oldCount, QObject *parent)
    :WebContextParser(parent),
      _name(name),
      _url(QString("http://kaijiang.cjcp.com.cn/%1ssc/").arg(name)),
      _oldCount(oldCount),
      _totalPageCount(0),
      _totalResultCount(0),
      _oldTotalPageCount(0),
      _oldTotalResultCount(0),
      _nextParserPage(0),
      _currentParserCount(0),
      _errorCount(0),
      _pWebContext(0)
{
}


bool ResultPageParser::parser() {
    if(_running){
        WARNING_QOBJECT_T(web)<<"already in parser!";
        return false;
    }

    DEBUG_QOBJECT_T(web)<<"_oldCount"<<_oldCount;

    _running = true;
    _abort = false;
    _errorCount = 0;
    if(!_pNetworkManager)
        _pNetworkManager = new QNetworkAccessManager(this);
    parserTopPage();

    return true;
}

bool ResultPageParser::parser(int oldCount)
{
    if(_running){
        WARNING_QOBJECT_T(web)<<"already in parser!";
        return false;
    }
    _oldCount = oldCount;
    return parser();
}

void ResultPageParser::parserTopPage()
{
    if(_abort) {
        _running = false;
        return;
    }

    _currentParserCount = 0;
    _nextParserPage = -1;
    _totalPageCount = 0;
    _totalResultCount = 0;
    _oldTotalPageCount = 0;
    _oldTotalResultCount = 0;
    _lastNodeList.clear();
    _pWebContext = new WebContext(_url, this);
    _pWebContext->setNetworkAccessManager(_pNetworkManager);
    connect(_pWebContext, &WebContext::flushed, this, &ResultPageParser::_parserPage);
    connect(_pWebContext, &WebContext::error, this, &ResultPageParser::contextError);
    _pWebContext->flush();

    DEBUG_QOBJECT_T(web);
}

void ResultPageParser::contextError(const QString &msg)
{
    ++_errorCount;
    _pWebContext->disconnect(this);
    _pWebContext->deleteLater();
    _pWebContext = 0;
    if(_errorCount < 9){
        QThread::msleep(1000 + qrand() % 3000);
        if(_nextParserPage <= 1)
            parserTopPage();
        else
            parserOtherPage();
    }else{
        _errorCount = 0;
        _abort = false;
        _running =  false;
    }
    WARNING_QOBJECT_T(web)<<msg<<"_errorCount"<<_errorCount;
}

void ResultPageParser::parserOtherPage()
{
    if(_abort) {
        _running = false;
        return;
    }

    _pWebContext = new WebContext(_url + QString("index.php?topage=%1").arg(_nextParserPage), this);
    if(_pNetworkManager)
        _pWebContext->setNetworkAccessManager(_pNetworkManager);
    connect(_pWebContext, &WebContext::flushed, this, &ResultPageParser::_parserPage);
    connect(_pWebContext, &WebContext::error, this, &ResultPageParser::contextError);
    _pWebContext->flush();

    DEBUG_QOBJECT_T(web)<<"_nextParserPage ="<<_nextParserPage
                       <<"_oldCount ="<<_oldCount;
}


void ResultPageParser::pageParserFinish()
{
    _pWebContext->disconnect(this);
    _pWebContext->deleteLater();
    _pWebContext = 0;
    _errorCount = 0;

    if(_totalPageCount <= 0
            || _totalResultCount <= 0
            || _lastNodeList.isEmpty()){
        WARNING_QOBJECT_T(web)<<"have error! _totalPageCount=" <<_totalPageCount
                             <<"_totalResultCount =" <<_totalResultCount
                            <<"nodeList ="<<_lastNodeList.size() ;
        _running = false;
        emit finish(1);
        return;
    }

    if(_nextParserPage < 0
            || _oldTotalResultCount != _totalResultCount) {
        _nextParserPage = _totalPageCount
                - qMax(0, _oldCount -  _totalResultCount % _perPageNodeCount) / _perPageNodeCount
                - ((_oldCount >= _totalResultCount % _perPageNodeCount) ? 1 : 0);

        if(_nextParserPage > 1) {
            _oldTotalPageCount = _totalPageCount;
            _oldTotalResultCount = _totalResultCount;
            parserOtherPage();
            return;
        }
    }

    _running = false;

    if(_nextParserPage != _totalPageCount && _lastNodeList.size() != _perPageNodeCount) {
        WARNING_QOBJECT_T(web)<<"not parser all node!_lastNodeList.size() ="
                             <<_lastNodeList.size()<<"_nextParserPage ="<<_nextParserPage
                            <<"_totalPageCount ="<<_totalPageCount;
    }

    emit finish(_lastNodeList);
    emit finish(0);
}

bool ResultPageParser::_parserPage(const QByteArray& rawData)
{
    if(_abort) {
        _running = false;
        return false;
    }

    _totalResultCount = 0;
    _totalPageCount = 0;
    if(rawData.isEmpty()){
        WARNING_QOBJECT_T(web)<<"not get data!";
        pageParserFinish();
        return false;
    }

    QString data = QTextCodec::codecForName(_codecName.toLatin1())->toUnicode(rawData);

    QRegExp totalResultRegExp(QStringLiteral("总数：<b>[0-9]{1,}<"));
    totalResultRegExp.setMinimal(true);
    int offset = totalResultRegExp.lastIndexIn(data);
    if(offset != -1){
        QRegExp totalResultRegExp2("[0-9]{1,}");
        totalResultRegExp2.indexIn(totalResultRegExp.cap());
        offset += totalResultRegExp.cap().size();
        _totalResultCount =totalResultRegExp2.cap().toInt();
        DEBUG_QOBJECT_T(web)<<"totalResultCount ="<<_totalResultCount;
    }else{
        WARNING_QOBJECT_T(web)<<"not parser total result!";
        pageParserFinish();
        return false;
    }

    QRegExp totalPageRegExp(QStringLiteral("页次：<b><font color=\"red\">[0-9]{1,}</font>/[0-9]{1,}<"));
    totalPageRegExp.setMinimal(true);
    if(totalPageRegExp.indexIn(data, offset) != -1){
        QRegExp totalPageRegExp2("/[0-9]{1,}");
        totalPageRegExp2.indexIn(totalPageRegExp.cap());
        _totalPageCount =totalPageRegExp2.cap().mid(1).toInt();
        DEBUG_QOBJECT_T(web)<<"totalPageCount ="<<_totalPageCount;
    }else{
        WARNING_QOBJECT_T(web)<<"not parser total page!";
        pageParserFinish();
        return false;
    }

    QRegExp caiQihaoReg(QStringLiteral("\"qihao\">[0-9]{11,11}期<"));
    caiQihaoReg.setMinimal(true);
    offset = caiQihaoReg.indexIn(data, 0);
    _lastNodeList.clear();
    while(offset != -1){
        CaiNode node;

        QString caiNodeStr = caiQihaoReg.cap();
        node.sequence = caiNodeStr.mid(8,caiNodeStr.size() - 10);

        QRegExp caiTimeReg("\"time\">[0-9]{4,4}-[0-9]{2,2}-[0-9]{2,2} [0-9]{2,2}:[0-9]{2,2}:[0-9]{2,2}<");
        caiTimeReg.setMinimal(true);
        offset = caiTimeReg.indexIn(data, offset + caiNodeStr.size());
        if(offset == -1) break;
        caiNodeStr = caiTimeReg.cap();
        node.dateTime = caiNodeStr.mid(7,caiNodeStr.size() - 8);

        QRegExp resultReg("value=\"[0-9]\"");
        resultReg.setMinimal(true);
        for(int i = 0; i < 5; ++i){
            offset = resultReg.indexIn(data, offset + caiNodeStr.size());
            if(offset == -1) break;
            caiNodeStr = resultReg.cap();
            node.result.push_back(caiNodeStr.mid(7,1));
        }
        if(offset == -1) break;
        _lastNodeList.push_front(node);

        qDebug()<< node.sequence<<node.dateTime<<node.result;

        offset = caiQihaoReg.indexIn(data, offset + caiNodeStr.size());
    }

    INFO_QOBJECT_T(web)<<"_oldCount ="<<_oldCount<<"nodeCount ="<<_lastNodeList.size();

    pageParserFinish();

    return true;
}

