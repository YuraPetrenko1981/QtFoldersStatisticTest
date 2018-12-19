#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QTimer>
#include "statistics.h"
#include "statisticmodel.h"
//----------------------------------------------------------------------

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void refreshStatisticView();        //обновляет таблицу статистики

protected slots:
    void startCollectStatistic(const QModelIndex &path);    // старт сбора статистики
    void stopCollectStatistic(const QModelIndex &path);     // досрочное завершение сбора статистики и запуск нового сбора статистики
                                                            // двойной щелчок (должен завершать то что уже работате и запускать новое)
    void finishCollect();      // перенастройка соединений и т.п. в момент когда поток сбора статистки уничтожен

    void delayedStart();       // отложенный запуск сбора статистики. нужен в случае если пользователь
                               // решил посмотреть другую папку. Т.к. необходимо обеспеыить срабатываение слота
                               // stopCollectStatistic, дождаться завершения работающего фонового потока и только после этого
                               // можем стартовать новый подсчет

private:
    Ui::MainWindow *ui;
    QModelIndex currentPath;            //Текущий путь к папке для которой производится (либо производился последний раз)
                                        //подсчет статистики
    QFileSystemModel  *dirModel;        //модель файловой системы
    Statistics *currentFolderStatistic; //Данные по статистике
    StatisticModel *statModel;          //Модель данных по статистике
    Session *dirStaisticCounter; //Сборщик статистики, завернутый в облочку обеспечивающий его работу
                                 //в фоновом потоке уничтожает сам себя по завершении работы
    bool isCollecting;                  //Флаг того что запущен алгоритм сбора статистики
    const int tableStringHeight = 20;   //высота строк в таблице статистики

    //обновление таблицы производится
    //          1) когда будет обнаружена новая группа файлов - немедленно
    //          2) с интервалом в одну секунду.
    //в случае прекращения сбора статистики - обновление таблицы - прекращается.
    QTimer refresher;
    const int refreshInterval = 1000;   //милисекунды
};

//----------------------------------------------------------------------

#endif // MAINWINDOW_H
