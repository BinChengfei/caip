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

/******************************************************************************
 * Dependencies
 ******************************************************************************/

#include "logstream.h"
#include "logger.h"

#include <QDebug>
#include <QBuffer>

namespace Log4Qt {

/**************************************************************************
   * Class implementation: Logger
   **************************************************************************/

LogStream::~LogStream()
{
    if (!--stream->ref) {
        switch (mLevel.toInt())
        {
        case Level::TRACE_INT:
            mLogger.trace(stream->buffer);
            break;
        case  Level::DEBUG_INT:
            mLogger.debug(stream->buffer);
            break;
        case  Level::INFO_INT:
            mLogger.info(stream->buffer);
            break;
        case Level::WARN_INT:
            mLogger.warn(stream->buffer);
            break;
        case  Level::ERROR_INT:
            mLogger.error(stream->buffer);
            break;
        case  Level::FATAL_INT:
            mLogger.fatal(stream->buffer);
            break;
        }
        delete stream;
    }
}

//add by aFei
LogStream operator<<(LogStream dbg, const QVariant &v)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QDebug *pTmpDbg = new QDebug(&buffer);
    (*pTmpDbg).nospace()<<v;
    delete pTmpDbg;
    pTmpDbg = 0;
    dbg.nospace()<<buffer.data();
    buffer.close();

    return dbg.space();
}

LogStream operator<<(LogStream dbg, const QVariant::Type p)
{
    dbg.nospace() << "QVariant::"
                  << (int(p) != int(QMetaType::UnknownType)
            ? QMetaType::typeName(p)
            : "Invalid");
    return dbg.space();
}
} // namespace Log4Qt
