#include "statistics.h"

QMutex mutex;  //используется для синхронизации доступа потока GUI и рабочего потока к данным statisticList
DirsStatistic::DirsStatistic(Statistics *statisticList, QString path, bool* stopFlag) :
    extraStop(stopFlag)
{
    statisticList->clear();
    this->statisticList = statisticList;
    this->startPath = path;
}

DirsStatistic::~DirsStatistic()
{

}

void DirsStatistic::calculateSubFolderesCount(QString fullFolderName)
{
    QDir dir;
    QStringList namesOfDirectories;
    dir.setPath(fullFolderName);
    namesOfDirectories = dir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs );//Получили список имен файлов
    mutex.lock();
    statisticList->oneLevelDownSubFoldersCount = namesOfDirectories.count();
    mutex.unlock();
}

void DirsStatistic::addFileStatistic(QString fileExtention, int fileSize)
{
    //т.к. сюда заходим только если хотть один файл однозначно найден, то деления на 0 не модет произойти
    mutex.lock();   //гарантирует согласованность данных по статистике группы
    if (statisticList->fileGroups.contains(fileExtention) )
    {
        ++statisticList->fileGroups[fileExtention].filesCount;
        statisticList->fileGroups[fileExtention].filesTotalSize += fileSize;
        statisticList->fileGroups[fileExtention].filesMediumSize =
                statisticList->fileGroups[fileExtention].filesTotalSize/statisticList->fileGroups[fileExtention].filesCount;
    }
    else
    {
        FileGroupStatistic newItem;
        newItem.filesCount = 1;
        newItem.filesTotalSize = fileSize;
        newItem.filesMediumSize = newItem.filesTotalSize/newItem.filesCount;
        statisticList->fileGroups.insert(fileExtention,  newItem);
        emit newFilesGroupDeteckted();  //уведомляем всех о том что найдена новая группа и нужно срочно обновиться.
    }
    mutex.unlock();
}

#include <QDebug>
void DirsStatistic::recursiveStatisticCounter(QString fullFolderName)
{
    if (*extraStop)
        return;

    QDir dir;
    QStringList namesOfDirectories;
    dir.setPath(fullFolderName);
    namesOfDirectories = dir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs );//Получили список имен директорий
    QStringList namesOfFiles;
    namesOfFiles = dir.entryList(QDir::Files);  //Получили список имен файлов
    QFileInfo file;
    for (auto it = namesOfFiles.begin(); it!=namesOfFiles.end(); it++)
    {
        if (*extraStop)  //дополнительная проверка, на случай большого количества папок
            return;
        QString extention = tr("no extension");
        file.setFile(dir, *it);
        //Возможна ситуация, когда файл будет удален, проверим.
        if (file.exists())
        {
            int dotIndex = (*it).lastIndexOf('.');

            if ( dotIndex != -1)
                extention = (*it).right((*it).size()-1 - dotIndex);

            int fileSize = file.size();
            addFileStatistic(extention,fileSize);
        }
    }

    for (auto it = namesOfDirectories.begin(); it!=namesOfDirectories.end(); it++)
        recursiveStatisticCounter(fullFolderName +"/"+ *it);
}

void DirsStatistic::startExequte()
{
    calculateSubFolderesCount(startPath);
    recursiveStatisticCounter(startPath);
    if (*extraStop)
        statisticList->clear(); //т.к. экстренный выход, то данные недостоверны, очистим.
    emit finished();
}


//----------------------------------------------------------------------
Session::Session(Statistics *statisticList, QString path)
{
    worker = new DirsStatistic(statisticList, path, &stopWorker);    //передалть на сонстантную ссыку
    thread = new QThread;
    worker->moveToThread(thread);
    //управление работой потока с воркером
    connect(thread, SIGNAL(started()), worker, SLOT(startExequte()));

    //остановка потока и удаление (выполнятся в таком порядке)
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(destroyed(QObject*)), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
}


Session::~Session()
{
    stopThread();
}

void Session::startThread()
{
    stopThread();
    stopWorker = false;
    thread->start();
}

void Session::stopThread()
{
    stopWorker = true;
}

//----------------------------------------------------------------------
