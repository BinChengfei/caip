#ifndef CAIPAGE_H
#define CAIPAGE_H

#include <QWidget>
#include <QSqlQueryModel>

namespace Ui {
class CaiPage;
}

class QThread;
class DataService;
class QTimer;
class CaiPage : public QWidget
{
    Q_OBJECT

public:
    explicit CaiPage(const QString& name, int minIgnoreCount = 0, QWidget *parent = 0);
    ~CaiPage();

public slots:
    void reset();
    void update();

protected slots:
    void updateFinish();

signals:
    void message(const QString& msg);

private:
    Ui::CaiPage *ui;
    QString _name;
    QSqlQueryModel _enumFrontModel;
    QSqlQueryModel _enumBackModel;
    QSqlQueryModel _lastModel;
    QThread *_pWorkThread;
    DataService *_pDataService;
    QTimer *_pUpdateTimer;
    int _topCount;

    static const QString _frontQueryStr;
    static const QString _backQueryStr;
    static const QString _lastQueryStr;
};

#endif // CAIPAGE_H
