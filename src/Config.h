/*
* Copyright (c) 2013,长沙联远电子有限公司
* All rights reserved.
*
* 简述： 系统配置，描述系统具体情况
* 说明：
* 作   者：aFei
* 完成日期：2013.01.10
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

class Config
{
public:
    inline QVariant value(const QString& name,  const QVariant& defaultValue = QVariant()) {
        return _settings.value(name, defaultValue);
    }

    static Config *instance();

private:
    explicit Config();
    void init();

private:
    QSettings _settings;

    static Config *_pInstance;
};

#endif // CONFIG_H
