#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    isCollecting(false)
{
    ui->setupUi(this);
    currentFolderStatistic = new Statistics();
//  QDirModel is obsolete
//  QFileSystemModel uses a separate thread to populate itself
//  so it will not cause the main thread to hang as the file system is being queried.
    dirModel = new QFileSystemModel ;
    dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    dirModel->setRootPath("");
    statModel = new StatisticModel();

    statModel->setDataSource(currentFolderStatistic);
    ui->fileTreeView->setModel(dirModel);

    //объединение таблиц, левый верхний угол + смещение
    ui->statisticView->setSpan(0, 0, 1, 4); //строка - заголовок, для статистики по директориям
    ui->statisticView->setSpan(2, 0, 1, 4); //строка - заголовок, для статистики по группам файлов
    ui->statisticView->verticalHeader()->setDefaultSectionSize(tableStringHeight); //вот так странно тут ставится высота ряда по умолчанию
    ui->statisticView->setModel(statModel);

    connect(&refresher, &QTimer::timeout, this, &MainWindow::refreshStatisticView);
    connect(ui->fileTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(startCollectStatistic(QModelIndex)) );
    //готовы к первомы запуску
    //обновление талицы по таймеру вкл.
    //по двойному щелчку слот старта потока вкл, слот остановки потока по двойному щелчку выкл.
}

MainWindow::~MainWindow()
{
    disconnect(&refresher, &QTimer::timeout, this, &MainWindow::refreshStatisticView);
    delete currentFolderStatistic;
    delete dirModel;
    delete statModel;
    delete ui;
}

void MainWindow::startCollectStatistic(const QModelIndex& path)
{
    //предусловие должно быть состояние:
    //  по двойному щелчку слот старта потока вкл, слот остановки потока по двойному щелчку выкл.
    if (!isCollecting)
    {
        currentFolderStatistic->clear();
        ui->statisticView->setModel(nullptr);
        ui->statisticView->setModel(statModel);

        currentPath = path;
        disconnect(ui->fileTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(startCollectStatistic(QModelIndex)) );
        connect(ui->fileTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(stopCollectStatistic(QModelIndex)) );
        //перейдем в состояние ожидания досрочного прерывания
        //  слот остановки потока по двойному щелчку вкл, по двойному щелчку слот старта потока выкл.
        ui->statusLabel->setText(tr("Collecting statistic"));
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(0);
        isCollecting = true;

        //создадим поток сбора статистики
        dirStaisticCounter = new Session(currentFolderStatistic, dirModel->filePath(path));
        //настроим отображение статистики в таблице
        connect(dirStaisticCounter->getWorker(), &DirsStatistic::newFilesGroupDeteckted, statModel, &StatisticModel::groupDetected);
        connect(dirStaisticCounter->getWorker(), &DirsStatistic::newFilesGroupDeteckted, this, &MainWindow::refreshStatisticView);
        refresher.start(1000);

        //настроим ожидание штатного завершения потока
        connect(dirStaisticCounter, &DirsStatistic::destroyed, this, &MainWindow::finishCollect);
        //стартуем поток сбора статистики
        dirStaisticCounter->startThread();
    }
    else
        throw std::logic_error("Collecting staistic thread is allready working.");
}

void MainWindow::finishCollect()
{
    if (isCollecting)
    {
        disconnect(ui->fileTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(stopCollectStatistic(QModelIndex)) );
        ui->statusLabel->setText(tr("Ready"));
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(1);
        ui->progressBar->setValue(1);
        refresher.stop();
        isCollecting = false;
        connect(ui->fileTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(startCollectStatistic(QModelIndex)) );
        //обеспечили условия для повторного старта сбора статистики
        //по двойному щелчку слот старта потока вкл, слот остановки потока по двойному щелчку выкл.
    }
    else
        throw std::logic_error("Collecting staistic thread is allready stoped.");
}

void MainWindow::delayedStart()
{
    //dirStaisticCounter - уничтожен
    //предусловия для работы startCollectStatistic выполнены
    //т.к. слот finishCollect подключен к сигналу destroyed от dirStaisticCounter прежде чем delayedStart.
    startCollectStatistic(currentPath);
}

void MainWindow::stopCollectStatistic(const QModelIndex& path)
{
    currentPath = path;
    connect(dirStaisticCounter, SIGNAL(destroyed(QObject*)), this, SLOT (delayedStart()));
    //готовы к новому автостарту старту после полного уничтожения работающего потока сбора статистики
    dirStaisticCounter->stopThread();

}

void MainWindow::refreshStatisticView()
{
    QModelIndex top = statModel->createIndex(0,0);
    QModelIndex bottom = statModel->createIndex(ui->statisticView->model()->rowCount()-1, ui->statisticView->model()->columnCount()-1);
    emit statModel->dataChanged(top, bottom);
}

