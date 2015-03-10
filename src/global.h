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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "logstream.h"
#include "logmanager.h"

#include <QString>
#include <QByteArray>
#include <QDebug>
#include <QSet>

class QObject;

extern QSet<QString> gDebugfilterTagSet;

#ifndef QT_NO_DEBUG_OUTPUT
#define TRACE_QOBJECT_Q  Log4Qt::LogManager::logger("Qt")->trace()<<this->objectName()<<QString("%1::%2").arg(this->metaObject()->className()).arg(__FUNCTION__)
#define TRACE_Q Log4Qt::LogManager::logger("Qt")->trace()<<__FUNCTION__
#else
#define TRACE_QOBJECT_Q qDebug()
#define TRACE_Q qDebug()
#endif

#define DEBUG_QOBJECT_Q qDebug()<<this->objectName()<<QString("%1::%2").arg(this->metaObject()->className()).arg(__FUNCTION__)
#define DEBUG_Q qDebug()<<__FUNCTION__

#ifndef QT_NO_WARNING_OUTPUT
#define INFO_QOBJECT_Q Log4Qt::LogManager::logger("Qt")->info()<<this->objectName()<<QString("%1::%2").arg(this->metaObject()->className()).arg(__FUNCTION__)
#define INFO_Q Log4Qt::LogManager::logger("Qt")->info()<<__FUNCTION__
#else
#define INFO_QOBJECT_Q qDebug()
#define INFO_Q qDebug()
#endif

#define WARNING_QOBJECT_Q qWarning()<<this->objectName()<<QString("%1::%2").arg(this->metaObject()->className()).arg(__FUNCTION__)
#define WARNING_Q qWarning()<<__FUNCTION__

#ifndef QT_NO_DEBUG_OUTPUT
#define TRACE_QOBJECT_T(TAG) \
    if(gDebugfilterTagSet.isEmpty() || gDebugfilterTagSet.contains(#TAG)) Log4Qt::LogManager::logger(#TAG)->trace()<<this->objectName()<<QString("%1::%2").arg(this->metaObject()->className()).arg(__FUNCTION__)
#define TRACE_T(TAG) \
    if(gDebugfilterTagSet.isEmpty() || gDebugfilterTagSet.contains(#TAG)) Log4Qt::LogManager::logger(#TAG)->trace()<<__FUNCTION__

#define DEBUG_QOBJECT_T(TAG) \
    if(gDebugfilterTagSet.isEmpty() || gDebugfilterTagSet.contains(#TAG)) Log4Qt::LogManager::logger(#TAG)->debug()<<this->objectName()<<QString("%1::%2").arg(this->metaObject()->className()).arg(__FUNCTION__)
#define DEBUG_T(TAG) \
    if(gDebugfilterTagSet.isEmpty() || gDebugfilterTagSet.contains(#TAG)) Log4Qt::LogManager::logger(#TAG)->debug()<<__FUNCTION__
#else
#define TRACE_QOBJECT_T(TAG) qDebug()
#define TRACE_T(TAG) qDebug()
#define DEBUG_QOBJECT_T(TAG) qDebug()
#define DEBUG_T(TAG) qDebug()
#endif

#ifndef QT_NO_WARNING_OUTPUT
#define INFO_QOBJECT_T(TAG) \
    if(gDebugfilterTagSet.isEmpty() || gDebugfilterTagSet.contains(#TAG)) Log4Qt::LogManager::logger(#TAG)->info()<<this->objectName()<<QString("%1::%2").arg(this->metaObject()->className()).arg(__FUNCTION__)
#define INFO_T(TAG) \
    if(gDebugfilterTagSet.isEmpty() || gDebugfilterTagSet.contains(#TAG)) Log4Qt::LogManager::logger(#TAG)->info()<<__FUNCTION__

#define WARNING_QOBJECT_T(TAG) \
    if(gDebugfilterTagSet.isEmpty() || gDebugfilterTagSet.contains(#TAG)) Log4Qt::LogManager::logger(#TAG)->warn()<<this->objectName()<<QString("%1::%2").arg(this->metaObject()->className()).arg(__FUNCTION__)
#define WARNING_T(TAG) \
    if(gDebugfilterTagSet.isEmpty() || gDebugfilterTagSet.contains(#TAG)) Log4Qt::LogManager::logger(#TAG)->warn()<<__FUNCTION__
#else
#define INFO_QOBJECT_T(TAG) qDebug()
#define INFO_T(TAG) qDebug()
#define WARNING_QOBJECT_T(TAG) qWarning()
#define WARNING_T(TAG) qWarning()
#endif

QByteArray GetMac(const QString& ethName, char sp = '\x00');
QStringList GetMac();

//获取/proc/cmdline中的键值
QString GetCmdLineValue(const QString &key);

//长名字转短名，中间替换为...
QString LongNameToShortName(QString name, int maxWidth = 24);

//获取Qt对象的全名（包括类名）
QString GetFullObjectName(const QObject *pObject);

//局部事件循环+定时器延时
void Delay(unsigned int t);

//局部事件循环+sleep延时
void Delay2(unsigned int t);

//将二进制数据转换为16进制的字符串
QString DataToHex(const QByteArray &data);
//将二进制数据转换为ascii的字符串
QString DatatoLatin1(const QByteArray& data);
//将16进制的形式的字符串转换为值数据
//如 16进制的形式的字符串 01 02 会转换成对因的1和2两个值数据
QByteArray StringToData(const QString &str);

//3rd
QString Bin2Hex(const char* buf, uint len);
QString Bin2Ascii(const char* buf, uint len);

//quint32 GetLocalIp();
//quint32 GetLocalNetmask();
//quint32 GetGateway();
//int SetGateway(unsigned int gw);

//int LinkStatus(const char* devname);

#endif // GLOBAL_H
