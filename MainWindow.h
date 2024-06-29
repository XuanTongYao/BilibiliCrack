#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <BilibiliCache.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void run();
    bool combine(const BilibiliCache& cache);




private:
    Ui::MainWindow *ui;
    bool delTmpFile;
    QString outputPath;
};
#endif // MAINWINDOW_H
