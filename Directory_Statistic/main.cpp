#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

//Описание программы.
//Программа осуществляет сбор статистики использования файловой системы
//Пользователь может осуществлять навигацию по дереву файловой системы, открывая и закрывая
//интересующие его каталоги. При двойном клике на папке начинается сбор статстики по выбранной
//папке. Статистика отбражается в таблице.
//При выборе папки приложение собирает:
//- количество файлов, общий размер и средний размер для каждой группы файлов (группировать
//  по расширению и отдельной строкой - для всех) - рекурсивно по всей папке.
//- количество подкаталогов - только для данной папки.
//В приложении имеется прогресс бар, который сигнализируует о том что сбор статистики в процессе.
//По завершении сбора статистики прогресс бар будет полностью заполнен, над прогресс баром будет
//надпись "завершено".

//Для навигации по файловой системе используется QTreeView + QFileSystemModel.
//QFileSystemModel - использует отдельный поток.
//Для подсчета статистики используется класс DirsStatistic, обернутый в оболочку Session - таким
//образом обеспечивается сбор статистики в фоновом потоке и отсутсвие блокировки.

//В основное окно добавлен таймер обновляющий таблицу статистики раз в секунду, т.к. нецелесообразно
//обновлять таблицу после обработки каждого файла + такой подход позволяет использовать в структуре
//с данными QMap. Использование QMap - упрощает и ускоряет код класса DirsStatistic.
//При обнаружении файла с новым расширением  - происходит немедленное обновление таблицы статистики.
//Этого достаточно для комфортной работы пользователя.

//В качестве модели с которой работает таблица статистик используется класс
//StatisticModel унаследованный от QAbstractTableModel , отказался от использования QStandardItemModel
//в связи с тем что она медленная.
//Для отбражения данных в таблице использую QTableView , т.к QTableWidget - не надежен.

//В приложение добавлены в качестве ресурсов переводы - локализация русскоязычного интерфейса пользователя.

//Прервать сбор статистики дострочно невозможно, кроме случая когда пользователь совершит двойной щелчок
//на другой папке.

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //просто выводим все надписи по - русски, что бы увидеть по английски - добавьте обработчик
    //либо закомментируйте три следующие строки.
    QTranslator translator;
    if (translator.load(":/directoryStatistic.qm"))
        qApp->installTranslator(&translator);

    MainWindow w;
    w.show();

    return a.exec();
}