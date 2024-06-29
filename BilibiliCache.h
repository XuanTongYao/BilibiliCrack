#ifndef BILIBILICACHE_H
#define BILIBILICACHE_H
#include <QString>

class BilibiliCache
{
public:
    explicit BilibiliCache(const QString & video, const QString & audio, const QString &path);
    QString getVideoFilePath() const;
    QString getAudioFilePath() const;
    QString getPath() const;
    QString getName() const;
    bool isHasPrefix() const;

private:
    QString videoName;
    QString audioName;
    QString path;
    QString name;
    bool hasPrefix;
};

#endif // BILIBILICACHE_H
