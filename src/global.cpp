/*
* Copyright (c) 2013,长沙联远电子有限公司
* All rights reserved.
*
*
* 简述：全局的辅助函数和类型
* 说明：
* 作   者：aFei
* 完成日期：2011.12.20
*/

#include "global.h"

#include <QEventLoop>
#include <QTimer>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QRegExp>
#include <QThread>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QFile>
#include <QStringList>

#include <string.h>
#include <cstdio>

#include <QNetworkInterface>
#include <QDebug>

static bool GlobalInit()
{
    static bool isInit = false;
    if (!isInit) {
        qSetMessagePattern("[%{threadid}][%{type}]: %{function} %{message}"
                            "%{if-warning}  \n#%{file}::%{line} %{endif}"
                           "%{if-critical}  \n#%{file}::%{line} %{endif}"
                           "%{if-fatal}  \n#%{file}::%{line} %{endif}"
                           ) ;
        isInit = true;
    }

    return true;
}

 QSet<QString> gDebugfilterTagSet;

/*!
 * \brief  确保一些初始化，可以在进入main()函数之前完成。
 */
bool gGlobalInit = GlobalInit();


QStringList GetMac()
{
    QNetworkInterface thisNet;
    QList<QNetworkInterface> netList = QNetworkInterface::allInterfaces();
    QStringList macList;
    for(int i = 0;i < netList.size(); i++){
        if(netList.at(i).isValid()){
            thisNet = netList.at(i);
            macList.push_back(thisNet.hardwareAddress());
        }
    }
    return macList;
}


QString LongNameToShortName(QString name, int maxWidth)
{
    int width = 0;
    int begPos = name.size() - 1;
    int endPos = 0;
    int midWidth = maxWidth / 2;
    for (int i = 0; i < name.size(); ++i) {
        width += (name.at(i) < 0xff) ? 1 : 2;
        if (width >= midWidth) {
            begPos = i;
            break;
        }
    }

    width = 0;
    for (int i = name.size() - 1; i >= 0; --i) {
        width += (name.at(i) < 0xff) ? 1 : 2;
        if (width >= midWidth) {
            endPos = i;
            break;
        }
    }

    if (name.size() && endPos > begPos)
        name.replace(begPos,  endPos - begPos + 1, "...");

    return name;
}


QString GetFullObjectName(const QObject *pObject)
{
    if (!pObject)
        return QString();

    return QString(pObject->metaObject()->className()) + "::" + pObject->objectName();
}

void Delay(unsigned int t)
{
    QEventLoop eventLoop;
    QTimer::singleShot(t, &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

void Delay2(unsigned int t)
{
    QElapsedTimer timer;
    timer.start();
    do {
        QCoreApplication::processEvents(QEventLoop::AllEvents, t);
        QThread::msleep(10);
    } while (timer.elapsed() < t);
}

QString DataToHex(const QByteArray &data)
{
    QByteArray hexData = data.toHex();
    QString tmpStr;
    tmpStr.reserve(hexData.size() + hexData.size() / 2);
    for (int i = 0; i < hexData.size(); ++i) {
        if ((i > 0) && (i % 2 == 0))
            tmpStr.push_back(' ');

        tmpStr.push_back(hexData.at(i));
    }

    return tmpStr;
}

QByteArray StringToData(const QString &str)
{
    QString tmpStr = str;
    tmpStr.remove(QRegExp("[^0-9a-fA-F]"));
    QByteArray tmpData;
    for (int i = 0; i < tmpStr.size(); i += 2) {
        tmpData.push_back(tmpStr.mid(i, 2).toUShort(0, 16));
    }
    return tmpData;
}

QString DatatoLatin1(const QByteArray& data)
{
    QString tmpStr;
    for (int i = 0; i < data.size(); ++i) {
        if ((static_cast<quint8>(data.at(i)) < 128) && QChar::fromLatin1(data.at(i)).isPrint())
            tmpStr.push_back(data.at(i));
        else
            tmpStr.push_back('.');
    }

    return tmpStr;
}

QNetworkAddressEntry Ipv4NetworkEntry()
{
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    for (int i = 0; i < interfaceList.size(); ++i) {
        QList<QNetworkAddressEntry> entryList = interfaceList.at(i).addressEntries();
        for (int j = 0; j < entryList.size(); ++j) {
            if (entryList.at(j).ip().protocol() == QAbstractSocket::IPv4Protocol) {
                if (entryList.at(j).ip().toString() != "127.0.0.1") {
                    return entryList.at(j);
                }
            }
        }
    }

    return QNetworkAddressEntry();
}

quint32 GetLocalIp()
{
    return  Ipv4NetworkEntry().ip().toIPv4Address();
}

quint32 GetLocalNetmask()
{
    return  Ipv4NetworkEntry().netmask().toIPv4Address();
}

quint32 GetLocalBroadcast()
{
    return Ipv4NetworkEntry().broadcast().toIPv4Address();
}


QString Bin2Hex(const char* buf, uint len)
{
    static const char*  _hexTab = "0123456789ABCDEF";
    char* tmp = new char[len * 3];

    const char* s = buf;
    const char* e = buf + len;
    char* d = tmp;

    if (s && d)
    {
        while (s < e)
        {
            char c = *s++;
            *d++ = _hexTab[(c>>4)&0xF];
            *d++ = _hexTab[c & 0xF];
            *d++ = ' ';
        }
        *d = 0;
    }

    QString res(tmp);

    delete tmp;

    return res;
}

QString Bin2Ascii(const char* buf, uint len)
{
    char* tmp = new char[len + 1];

    const char* s = buf;
    const char* e = buf + len;
    char* d = tmp;

    if (s && d)
    {
        while (s < e)
        {
            char c = *s++;
            *d++ = isprint((uchar)c) ? c : '.';
        }

        *d = 0;
    }

    QString res(tmp);

    delete tmp;

    return res;
}
