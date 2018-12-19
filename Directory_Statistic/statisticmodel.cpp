#include "statisticmodel.h"
#include <QMutex>
extern QMutex mutex;
StatisticModel::StatisticModel(QObject *parent)
{
    //установка цветов покраски ячеек таблицы
    brushHeader.setStyle(Qt::SolidPattern);
    brushHeader.setColor(QColor(225,225,225));

    brushBody.setStyle(Qt::SolidPattern);
    brushBody.setColor(QColor(160,160,160));
}

StatisticModel::~StatisticModel()
{

}

void StatisticModel::setDataSource(Statistics *dataSource)
{
    this->dataSource = dataSource;
}

QModelIndex StatisticModel::createIndex(int row, int column, void *data) const
{
    return QAbstractTableModel::createIndex(row, column, data);
}

QVariant StatisticModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant res;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        switch (section)
        {
            case enCaption                : res = tr("Specification");   break;    // Описание
            case enCount                  : res = tr("Amount");          break;    //Количество
            case enTotalFilesGroupSize    : res = tr("General size");    break;    //Общий размер
            case enTotalMediumGroupSize   : res = tr("Medium size");     break;    //Средний размер
        }
    return res;
}

QVariant StatisticModel::data(const QModelIndex &index, int role) const
{
    QVariant result;
    StatisticModel::RowColor rowType = getGroupType(index);
    switch (role)
    {
        case Qt::DisplayRole:           result = getCellData(index, rowType);         break;
        case Qt::TextAlignmentRole:     result = getTextAlignment(index, rowType);    break;
        case Qt::BackgroundRole:        result = getRowColor(rowType);                break;
    }
    return result;
}

int StatisticModel::rowCount(const QModelIndex &parent) const
{
    mutex.lock();
    int result = alwaysShownRowsNumber+dataSource->fileGroups.count();
    mutex.unlock();
    return result;
}

int StatisticModel::columnCount(const QModelIndex &parent) const
{
    return ColumnNameCount;
}

Statistics *StatisticModel::getDataSource()
{
    return dataSource;
}

#include <QDebug>
bool StatisticModel::insertRows(int position, int count, const QModelIndex  & parent)
{
//  не совсем понятно по
    beginInsertRows(parent,position,position+count-1);
    bool result = true;
    endInsertRows();
    return result;
}

void StatisticModel::groupDetected()
{
    //т.к. в данных используем QMap, то необходимо перерисовать всю область
    int lastRaw = rowCount();
    //bool temp = insertRow(lastRaw);
    bool temp = insertRows(lastRaw,1);
    QModelIndex top = createIndex(0,0);
    QModelIndex bottom = createIndex(rowCount()-1, columnCount()-1);
    qDebug() << "groupDetected " << lastRaw << "insert " << temp;

    emit dataChanged(top, bottom);
}

QVariant StatisticModel::getDirData(int column) const
{
    QVariant result;
    switch (column)
    {
        case enCaption               : result = tr("subdirs");
                                       break;
        case enCount                 : mutex.lock();
                                       result = dataSource->oneLevelDownSubFoldersCount;
                                       mutex.unlock();
                                       break;
        case enTotalFilesGroupSize   : result = "";  break;
        case enTotalMediumGroupSize  : result = "";  break;
    }
    return result;
}

QVariant StatisticModel::getFileData(int row, int column) const
{
    QVariant result;
    mutex.lock();
    switch (column)
    {
        case enCaption               : result = dataSource->fileGroups.keys().at(row);                    break;
        case enCount                 : result = dataSource->fileGroups.values().at(row).filesCount;       break;
        case enTotalFilesGroupSize   : result = dataSource->fileGroups.values().at(row).filesTotalSize;   break;
        case enTotalMediumGroupSize  : result = dataSource->fileGroups.values().at(row).filesMediumSize;  break;
    }
    mutex.unlock();
    return result;
}

QVariant StatisticModel::getCellData(const QModelIndex &index, RowColor groupType) const
{
    QVariant result ;
    switch (groupType)
    {
        case enDirHeader        : result = QString(tr("Subfolderes statistic"));           break;
        case enFileGroupHeader  : result = QString(tr("File groups statistic"));           break;
        case enDirBody          : result = getDirData(index.column());                 break;
        case enFileBody         : result = getFileData(index.row()-alwaysShownRowsNumber, index.column()); break;
    }
    return result;
}

QVariant StatisticModel::getRowColor(RowColor groupType) const
{
    if (groupType == enDirHeader || groupType == enFileGroupHeader)
        return brushHeader;
    else
        return brushBody;
}

QVariant StatisticModel::getTextAlignment(const QModelIndex &index, RowColor groupType) const
{
    QVariant result ;
    switch (groupType)
    {
        case enDirHeader        :
        case enFileGroupHeader  :
                                    return Qt::AlignCenter;

        case enDirBody          :
        case enFileBody         :
                                    if (index.column() == 0)
                                        return (QVariant) (Qt::AlignLeft | Qt::AlignBottom);
                                    else
                                        return (QVariant) (Qt::AlignRight | Qt::AlignBottom);
        break;
    }
    return result;
}

StatisticModel::RowColor StatisticModel::getGroupType(const QModelIndex &index) const
{
    if (index.row() == enDirHeader) return enDirHeader;
    else if (index.row() == enDirBody)  return enDirBody;
    else if (index.row() == enFileGroupHeader)  return enFileGroupHeader;
    else return enFileBody;
}
