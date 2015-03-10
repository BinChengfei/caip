#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class CaiPage;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void resetDb();
    void showHelpPage();
    void updateMessage(const QString& msg);

protected slots:
    void tabChange(int t);

private:
    Ui::MainWindow *_pUi;

    CaiPage *_pCqPage;
    CaiPage *_pJxPage;
    CaiPage *_pTjPage;
    CaiPage *_pXjPage;
    CaiPage *_pHljPage;
    CaiPage *_pYnPage;
    CaiPage *_pFjPage;

    CaiPage *_pCurrentPage;

    QTimer *_pUpdateTimer;
};

#endif // MAINWINDOW_H
