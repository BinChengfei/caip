/*
* Copyright (c) 2013,长沙联远电子有限公司
* All rights reserved.
*
* 简述： 系统配置，描述系统具体情况
* 说明：
* 作   者：aFei
* 完成日期：2013.01.10
*/

#include "Config.h"
#include "global.h"

#include <QMutex>
#include <QFile>
#include <QCoreApplication>

Config *Config::_pInstance = 0;

Config::Config()
    : _settings(QCoreApplication::applicationDirPath() + "/caip.conf", QSettings::IniFormat)
{
    _settings.setIniCodec("UTF-8");
    if (_settings.status() != QSettings::NoError) {
        WARNING_T(core) << "load cfg faile!"
                         << "fileName =" << _settings.fileName()
                         << "QSettingsState =" << _settings.status();
    } else {
        DEBUG_T(core) << "load cfg." << _settings.fileName();
    }

    init();
}

Config* Config::instance()
{
    static QMutex mutex;

    if (!_pInstance) {
        QMutexLocker locker(&mutex);
        if (!_pInstance)
            _pInstance = new Config;
    }

    return _pInstance;
}

void Config::init()
{
}
