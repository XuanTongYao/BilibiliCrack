#include "BilibiliCache.h"
#include <QFile>
#include <QFileInfo>
#include <QtDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegExp>

BilibiliCache::BilibiliCache(const QString &video, const QString &audio, const QString &path)
{
    videoName = video;
    audioName = audio;
    this->path = path;
    QFile file(path + "\\" + video);
    if (file.open(QIODevice::ReadOnly)) {
        if (file.read(2) == "00")
            hasPrefix = true;
        else
            hasPrefix = false;
    } else {
        hasPrefix = false;
    }
    file.close();

    if (QFile::exists(path + "/.videoInfo")) {
        // PC缓存
        QFile info(path + "/.videoInfo");
        if (info.open(QIODevice::ReadOnly))
            name = QJsonDocument::fromJson(info.readAll())["title"].toString();
    } else if (QFile::exists(path + "/../entry.json")) {
        // 安卓缓存
        QFile info(path + "/../entry.json");
        if (info.open(QIODevice::ReadOnly))
            name = QJsonDocument::fromJson(info.readAll())["page_data"].toObject().value("part").toString();
    } else if (name == "") {
        name = video;
    }
    // 替换windows中不能作为文件名的字符
    QRegExp reg("[\\/:*?\"<>|]");
    name.replace("\"", "“");
    name.replace(reg, "");
}

QString BilibiliCache::getVideoFilePath() const
{
    return path + "\\" + videoName;
}

QString BilibiliCache::getAudioFilePath() const
{
    return path + "\\" + audioName;
}

QString BilibiliCache::getPath() const
{
    return path;
}

QString BilibiliCache::getName() const
{
    return name;
}


bool BilibiliCache::isHasPrefix() const
{
    return hasPrefix;
}
