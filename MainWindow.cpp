#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QMessageBox>

#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->DownloadPath->installEventFilter(this);
    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::run);

    if (!QDir("output").exists())
        QDir(".").mkdir("output");
    outputPath = QDir("output").absolutePath();


    qDebug() << outputPath;
}

MainWindow::~MainWindow()
{
    delete ui;
}


// 拖拽文件夹
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QString path = event
        ->mimeData()
        ->urls()
        .at(0)
        .toLocalFile();
    auto currentText = ui->VideoPathList->toPlainText();

    if (currentText == "将单个缓存的下载路径拖拽到此" || currentText == "")
        ui->VideoPathList->setPlainText(path);
    else
        ui->VideoPathList->appendPlainText(path);
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->VideoPathList)
        qDebug() << event << "事件";

    if (event->type() == QEvent::Drop && obj == ui->DownloadPath) {
        auto dropEvent = dynamic_cast<QDropEvent *>(event);
        QString path = dropEvent
            ->mimeData()
            ->urls()
            .at(0)
            .toLocalFile();
        ui->DownloadPath->setText(path);
        return true;
    } else if (event->type() == QEvent::DragEnter) {
        auto DragEnterEvent = dynamic_cast<QDragEnterEvent *>(event);
        if (DragEnterEvent->mimeData()->hasUrls())
            DragEnterEvent->acceptProposedAction();
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::run()
{
    delTmpFile = ui->delCheckBox->checkState();
    auto totalDownloadPath = ui->DownloadPath->text();
    auto allPaths = ui->VideoPathList->toPlainText().trimmed().split("\n");

    // 1.找到所有单个缓存的目录，目录内必定有.videoInfo或index.json文件
    if (QFileInfo::exists(totalDownloadPath)) {
        QDir dir(totalDownloadPath, "*.videoInfo index.json");
        dir.setFilter(QDir::Files);
        QDirIterator it(dir, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            allPaths.append(QFileInfo(it.next()).absolutePath());
        }
    }
    // 2.遍历单个缓存的目录，找出对应的视频音频文件。id-xxx.m4s
    allPaths.removeDuplicates();
    auto allCaches = QVector<BilibiliCache>();
    for (auto path : allPaths) {
        QDir dir(path, "*.m4s", QDir::Name | QDir::Size, QDir::Files);
        if (dir.exists()) {
            auto fileList = dir.entryList();
            if (fileList.length() == 2)
                allCaches.append(BilibiliCache(fileList.at(0), fileList.at(1), path));
        }
    }

    // 3.合并音视频
    ui->progressBar->setRange(0, allCaches.length());
    int numberCompleted = 0;
    int numberSuccesses = 0;
    for (const auto& cache:allCaches) {
        numberCompleted += 1;
        if (combine(cache))
            numberSuccesses += 1;
        ui->progressBar->setValue(numberCompleted);
    }
    if (allCaches.length() == 0) {
        QMessageBox::information(this, "未检测到视频缓存", "目录中未检测到视频缓存");
    } else {
        QMessageBox::information(this, "合并完成",
            QString("检测到%1个视频缓存，成功合并%2个")
            .arg(QString::number(allCaches.length()), QString::number(numberSuccesses)));
    }
}

bool MainWindow::combine(const BilibiliCache &cache)
{
    // 如果音视频有乱码前缀则移除乱码
    QString videoFilePath = cache.getVideoFilePath();
    QString audioFilePath = cache.getAudioFilePath();

    if (cache.isHasPrefix()) {
        QFile video(videoFilePath);
        QFile audio(audioFilePath);
        if (!video.open(QIODevice::ReadOnly))
            return false;
        if (!audio.open(QIODevice::ReadOnly))
            return false;
        video.seek(0x09);
        audio.seek(0x09);

        auto namePath = cache.getPath() + "/" + cache.getName();
        videoFilePath = namePath + ".mp4";
        audioFilePath = namePath + ".mp3";
        QFile tmpVideo(videoFilePath);
        if (!tmpVideo.open(QIODevice::WriteOnly))
            return false;
        tmpVideo.write(video.readAll());
        tmpVideo.close();

        QFile tmpAudio(audioFilePath);
        if (!tmpAudio.open(QIODevice::WriteOnly))
            return false;
        tmpAudio.write(audio.readAll());
        tmpAudio.close();

        video.close();
        audio.close();
    }
    // 合并
    QString outputFilePath = "output/" + cache.getName() + ".mp4";
    if (QFile::exists(outputFilePath)) {
        auto userSelected = QMessageBox::question(this, "检测到同名文件", cache.getName() + "\n输出文件已存在，是否覆盖？");
        if (userSelected == QMessageBox::Ok)
            QFile::remove(outputFilePath);
        else
            return true;
    }

    QProcess process;
    QString cmd = "ffmpeg -i \"" + videoFilePath + "\" -i \"" + audioFilePath + "\" -c:v copy -c:a copy \"" + outputFilePath + "\"";
    process.start(cmd);
    if (!process.waitForFinished())
        return false;

    // 删除临时文件
    if (delTmpFile && cache.isHasPrefix()) {
        QFile::remove(videoFilePath);
        QFile::remove(audioFilePath);
    }
    return true;
}
