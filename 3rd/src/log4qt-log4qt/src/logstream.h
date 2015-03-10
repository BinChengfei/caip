/******************************************************************************
 *
 * package:     Log4Qt
 * file:        logstream.h
 * created:     March, 2011
 * author:      Tim Besard
 *
 *
 * Copyright 2011 Tim Besard
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

#ifndef LOG4QT_LOGSTREAM_H
#define LOG4QT_LOGSTREAM_H


/******************************************************************************
 * Dependencies
 ******************************************************************************/
#include <QtCore/QTextStream>
#include <QtCore/QString>
#include <QVariant>
#include <QtCore/qalgorithms.h>
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qstring.h>
#include <QtCore/qvector.h>
#include <QtCore/qset.h>
#include <QtCore/qcontiguouscache.h>

#include "level.h"


/******************************************************************************
 * Declarations
 ******************************************************************************/

namespace Log4Qt
{
class Logger;

class LOG4QT_EXPORT LogStream
{

private:
    struct Stream {
        Stream() : ts(&buffer, QIODevice::WriteOnly), space(true), ref(1) {}
        QTextStream ts;
        QString buffer;
        bool space;
        int ref;
    } *stream;

public:
    inline LogStream(const Logger& iLogger, Level iLevel) : stream(new Stream()),  mLogger(iLogger), mLevel(iLevel)
    {
    }
    inline LogStream(const LogStream &o):stream(o.stream),mLogger(o.mLogger),mLevel(o.mLevel)
    {
        ++stream->ref;
    }
    ~LogStream();

    //add by aFei
    inline LogStream &space() { stream->space = true; stream->ts << ' '; return *this; }
    inline LogStream &nospace() { stream->space = false; return *this; }
    inline LogStream &maybeSpace() { if (stream->space) stream->ts << ' '; return *this; }
    bool autoInsertSpaces() const { return stream->space; }
    void setAutoInsertSpaces(bool b) { stream->space = b; }

    inline LogStream &operator<<(QChar t) { stream->ts << '\'' << t << '\''; return maybeSpace(); }
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    inline LogStream &operator<<(QBool t) { stream->ts << (bool(t != 0) ? "true" : "false");return maybeSpace(); }
#endif
    inline LogStream &operator<<(bool t) { stream->ts << (t ? "true" : "false"); return maybeSpace(); }
    inline LogStream &operator<<(char t) { stream->ts << t; return maybeSpace(); }
    inline LogStream &operator<<(signed short t) { stream->ts << t; return maybeSpace(); }
    inline LogStream &operator<<(unsigned short t) { stream->ts << t; return maybeSpace(); }
    inline LogStream &operator<<(signed int t) { stream->ts << t; return maybeSpace(); }
    inline LogStream &operator<<(unsigned int t) { stream->ts << t; return maybeSpace(); }
    inline LogStream &operator<<(signed long t) { stream->ts << t; return maybeSpace(); }
    inline LogStream &operator<<(unsigned long t) { stream->ts << t; return maybeSpace(); }
    inline LogStream &operator<<(qint64 t) { stream->ts << QString::number(t); return maybeSpace(); }
    inline LogStream &operator<<(quint64 t) { stream->ts << QString::number(t); return maybeSpace(); }
    inline LogStream &operator<<(float t) { stream->ts << t; return maybeSpace(); }
    inline LogStream &operator<<(double t) { stream->ts << t; return maybeSpace(); }
    inline LogStream &operator<<(const char* t) { stream->ts << QString::fromLatin1(t); return maybeSpace(); }
    inline LogStream &operator<<(const QString & t) { stream->ts << t ; return maybeSpace(); }
    inline LogStream &operator<<(const QStringRef & t) { return operator<<(t.toString()); }
    inline LogStream &operator<<(const QLatin1String &t) { stream->ts << t.latin1(); return maybeSpace(); }
    inline LogStream &operator<<(const QByteArray & t) { stream->ts  << t; return maybeSpace(); }
    inline LogStream &operator<<(const void * t) { stream->ts << t; return maybeSpace(); }
    inline LogStream &operator<<(QTextStreamFunction f) { stream->ts << f; return maybeSpace(); }
    inline LogStream &operator<<(QTextStreamManipulator m) { stream->ts << m; return maybeSpace(); }

private:
    const Logger& mLogger;
    Level mLevel;
};

//add by aFei
template <class T>
inline LogStream operator<<(LogStream debug, const QList<T> &list)
{
    debug.nospace() << '(';
    for (typename QList<T>::size_type i = 0; i < list.count(); ++i) {
        if (i)
            debug << ", ";
        debug << list.at(i);
    }
    debug << ')';
    return debug.space();
}

template <typename T>
inline LogStream operator<<(LogStream debug, const QVector<T> &vec)
{
    debug.nospace() << "QVector";
    return operator<<(debug, vec.toList());
}

template <class aKey, class aT>
inline LogStream operator<<(LogStream debug, const QMap<aKey, aT> &map)
{
    debug.nospace() << "QMap(";
    for (typename QMap<aKey, aT>::const_iterator it = map.constBegin();
         it != map.constEnd(); ++it) {
        debug << '(' << it.key() << ", " << it.value() << ')';
    }
    debug << ')';
    return debug.space();
}

template <class aKey, class aT>
inline LogStream operator<<(LogStream debug, const QHash<aKey, aT> &hash)
{
    debug.nospace() << "QHash(";
    for (typename QHash<aKey, aT>::const_iterator it = hash.constBegin();
         it != hash.constEnd(); ++it)
        debug << '(' << it.key() << ", " << it.value() << ')';
    debug << ')';
    return debug.space();
}

template <class T1, class T2>
inline LogStream operator<<(LogStream debug, const QPair<T1, T2> &pair)
{
    debug.nospace() << "QPair(" << pair.first << ',' << pair.second << ')';
    return debug.space();
}

template <typename T>
inline LogStream operator<<(LogStream debug, const QSet<T> &set)
{
    debug.nospace() << "QSet";
    return operator<<(debug, set.toList());
}

template <class T>
inline LogStream operator<<(LogStream debug, const QContiguousCache<T> &cache)
{
    debug.nospace() << "QContiguousCache(";
    for (int i = cache.firstIndex(); i <= cache.lastIndex(); ++i) {
        debug << cache[i];
        if (i != cache.lastIndex())
            debug << ", ";
    }
    debug << ')';
    return debug.space();
}

template <class T>
inline LogStream operator<<(LogStream debug, const QFlags<T> &flags)
{
    debug.nospace() << "QFlags(";
    bool needSeparator = false;
    for (uint i = 0; i < sizeof(T) * 8; ++i) {
        if (flags.testFlag(T(1 << i))) {
            if (needSeparator)
                debug.nospace() << '|';
            else
                needSeparator = true;
            debug.nospace() << "0x" << QByteArray::number(typename QFlags<T>::Int(1) << i, 16).constData();
        }
    }
    debug << ')';
    return debug.space();
}

LogStream operator<<(LogStream, const QVariant &);
LogStream operator<<(LogStream, const QVariant::Type);
}

#endif // LOG4QT_LOGSTREAM_H
