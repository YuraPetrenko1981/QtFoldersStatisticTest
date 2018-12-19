#ifndef STAISTICMODEL_H
#define STAISTICMODEL_H

//----------------------------------------------------------------------
//структура для хранения статистике по группе файлов
struct FileGroupStatistic
{
    int filesTotalSize;
    int filesCount;
    int filesMediumSize;
};

#include <QMap>
//структура данных для хранения полной статистике повыбранной папке
struct Statistics
{
    Statistics():
        oneLevelDownSubFoldersCount(0)
    {}

    void clear()
    {
        oneLevelDownSubFoldersCount=0;
        fileGroups.clear();
    }
    int oneLevelDownSubFoldersCount;
    QMap<QString, FileGroupStatistic> fileGroups;
};


//----------------------------------------------------------------------
#include <QObject>
#include <QDir>
#include <QMutex>

//класс, который производит непосредсвенно сбор информации по выбранной папке
//должен работать в отдельном потоке.
//использует глобальный мьютекс QMutex mutex
//остутсвует защита от переполнения стека рекурсией!!! - добавить обработчик
class DirsStatistic : public QObject
{
    Q_OBJECT
public:
    DirsStatistic(Statistics* statisticList, QString path, bool* stopFlag);
    ~DirsStatistic();

private slots:
    void startExequte();

private:
    void recursiveStatisticCounter(QString fullFolderName);
    void calculateSubFolderesCount(QString fullFolderName);
    void addFileStatistic(QString fileExtention, int fileSize);

private:
    Statistics *statisticList;  //объект для хранения результатов работы.
    QString startPath;
    bool *extraStop;    //останавливает построитель статистики (должна корректно останавливать поток, разблокировать
                        //файлы и мьютексы завершать поток)

signals:
    void newFilesGroupDeteckted();
    void finished();
protected:
};

//----------------------------------------------------------------------
#include <QThread>
class Session : public QObject
{
    Q_OBJECT
public:
    Session(Statistics *statisticList, QString path);
    ~Session();
    DirsStatistic* getWorker() {return worker;}

public slots:
    void startThread();
    void stopThread();

private:
    DirsStatistic* worker;
    QThread*       thread;
    bool   stopWorker;
};

//----------------------------------------------------------------------

#endif // STAISTICMODEL_H
