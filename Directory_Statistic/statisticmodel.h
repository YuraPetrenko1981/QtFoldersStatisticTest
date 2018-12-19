#ifndef STATISTICMODEL_H
#define STATISTICMODEL_H

#include <QAbstractTableModel>
#include <QBrush>
#include "statistics.h"

//класс модели сбора статистики
//наследуется от QAbstractTableModel , т.к. стандартные модели - тормознутые
//собственно далее идет реализация данного интерфейса.
//помним что первые три строки в таблице показываеются при любом результате
// (заголовки и статистика по группе будут всегда)
class StatisticModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ColumnName
    {
        enCaption=0,              //для папок выводим dirs
                                  //для группы файлов - выводим раширение (например *.txt)
        enCount,                  //для папок выводим количество подпапок
                                  //количество файлов (для директории - количество папок)
        //эти два столбца использются только для групп файлов
        enTotalFilesGroupSize,
        enTotalMediumGroupSize,
        ColumnNameCount
    };

    enum RowColor
    {
        enDirHeader =0,
        enDirBody,
        enFileGroupHeader,
        enFileBody
    };

    enum Roles
    {
        ColorRole = Qt::UserRole + 1,
        EndOfRoles
    };

public:
    StatisticModel(QObject *parent = 0);
    ~StatisticModel();
    void setDataSource(Statistics *dataSource);
    QModelIndex createIndex(int row, int column, void *data=0) const;
    QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    bool insertRows(int position, int count, const QModelIndex &parent = QModelIndex());

    Statistics * getDataSource();   //проверить где используетя, возможно удалить

public slots:
    void groupDetected();

private:
    Statistics* dataSource;
    const int alwaysShownRowsNumber = 3;    //количество рядов в таблице, которые показываются всегда
                                            //два заголовка () и статистика по поддиректориям
    QVariant getDirData(int column) const;
    QVariant getFileData(int row, int column) const;
    QVariant getCellData(const QModelIndex &index, RowColor groupType) const;
    QVariant getRowColor(RowColor groupType) const;
    QVariant getTextAlignment(const QModelIndex &index, RowColor groupType) const;
    StatisticModel::RowColor getGroupType(const QModelIndex &index) const;

    //цвета в которые окрашивается таблица
    QBrush brushHeader;
    QBrush brushBody;
};

#endif // STATISTICMODEL_H
