#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QJsonArray>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QListWidget *listWidget;
    QPushButton *openButton;
    QJsonArray transactions;

    void loadTransactions(const QString &fileName);
    QString computeHash(const QString &amount, const QString &wallet, const QString &date, const QString &previousHash);
    QListWidgetItem* createListItem(const QString &amount, const QString &wallet, const QString &date, const QString &hash, bool isInvalid);
};

#endif // MAINWINDOW_H
