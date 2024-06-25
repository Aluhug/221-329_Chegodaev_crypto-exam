#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    listWidget = findChild<QListWidget*>("listWidget");
    openButton = findChild<QPushButton*>("openButton");

    connect(openButton, &QPushButton::clicked, this, &MainWindow::openFile);

    loadTransactions("transactions.json");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", "", "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        loadTransactions(fileName);
    }
}

void MainWindow::loadTransactions(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка!", "Не удалось открыть файл");
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::warning(this, "Ошибка!", "Неверный формат файла");
        return;
    }

    listWidget->clear();

    transactions = doc.object().value("transactions").toArray();
    QString previousHash = "";
    for (int i = 0; i < transactions.size(); ++i) {
        QJsonObject obj = transactions[i].toObject();
        QString amount = obj.value("amount").toString();
        QString wallet = obj.value("wallet").toString();
        QString date = obj.value("date").toString();
        QString hash = obj.value("hash").toString();

        QString sum = amount + wallet + date + previousHash;

        QByteArray computedHash = QCryptographicHash::hash(sum.toUtf8(), QCryptographicHash::Sha256);

        bool isInvalid = (computedHash.toHex() != hash);
        listWidget->addItem(createListItem(amount, wallet, date, hash, isInvalid));

        if (!isInvalid) {
            previousHash = hash;
        }
    }
}

QString MainWindow::computeHash(const QString &amount, const QString &wallet, const QString &date, const QString &previousHash) {
    QByteArray data = (amount + wallet + date + previousHash).toUtf8();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return hash.toHex();
}

QListWidgetItem* MainWindow::createListItem(const QString &amount, const QString &wallet, const QString &date, const QString &hash, bool isInvalid) {
    QString text = QString("Amount: %1, Wallet: %2, Date: %3, Hash: %4").arg(amount).arg(wallet).arg(date).arg(hash);
    auto *item = new QListWidgetItem(text);
    if (isInvalid) {
        item->setBackground(Qt::red);
    }
    return item;
}
