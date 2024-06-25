#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "encryptor.h"
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QMessageBox>

//key: 6b86b273ff34fce19d6b804eff5a3f5747ada4eaa22f1d49c01e52ddb7875b4b
//iv: c4ca4238a0b923820dcc509a6f75849b

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    listWidget = findChild<QListWidget*>("listWidget");
    openButton = findChild<QPushButton*>("openButton");

    connect(openButton, &QPushButton::clicked, this, &MainWindow::openFile);

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

//Проверка траназкции на длину и хэш
bool checkTransaction(QJsonObject obj, QString &previousHash) {
    QString amount = obj.value("amount").toString();
    QString wallet = obj.value("wallet").toString();
    QString date = obj.value("date").toString();
    QString hash = obj.value("hash").toString();
    QString sum = amount + wallet + date + previousHash;

    qDebug() << previousHash;


    if (amount.length() != 7){
        return false;
    }
    if (wallet.length() != 6){
        return false;
    }
    if (date.length() != 19) {
        return false;
    }


    QByteArray computedHash = QCryptographicHash::hash(sum.toUtf8(), QCryptographicHash::Sha256);
    previousHash = computedHash.toHex();
    // qDebug() << hash + " || " + computedHash.toHex();

    if (computedHash.toHex() != hash)
        return false;
    return true;
}

//открытие файла с транзакциями
void MainWindow::loadTransactions(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    QByteArray encrypted = QByteArray::fromHex(file.readAll());
    QByteArray data;

    Encryptor::getInstance().decrypt(encrypted, data,
                                     QCryptographicHash::hash("1", QCryptographicHash::Sha256),
                                     QCryptographicHash::hash("1", QCryptographicHash::Md5));

    qDebug() << data;

    QJsonDocument doc(QJsonDocument::fromJson(data));
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::warning(this, "Ошибка", "Неверный формат файла");
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

        bool valid = !checkTransaction(obj, previousHash);
        qDebug() << i << " " << valid;

        listWidget->addItem(createListItem(amount, wallet, date, hash, valid));

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

void MainWindow::on_pushButton_clicked()
{
    QFile pinFile("pin.txt");

    if (!pinFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл пина");
        return;
    }

    QByteArray pinFromFile = QByteArray::fromHex(pinFile.readAll());
    QByteArray pinFromUser = ui->lineEdit->text().toUtf8();

    QByteArray pinHash = QCryptographicHash::hash(pinFromUser, QCryptographicHash::Md5);

    qDebug() << pinHash << " || " << pinFromFile;

    if (pinHash != pinFromFile) {
        qDebug() << "Пин неверен!";
    } else {
        qDebug() << "Пин верен!";
        ui->stackedWidget->setCurrentWidget(ui->page);
        loadTransactions("transactions.json");
    }
}


void MainWindow::on_showAddMenu_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_3);
}


void MainWindow::on_addBtn_clicked()
{
    QString amount = ui->amountEdit->text();
    QString wallet = ui->walletEdit->text();
    QString date = ui->dateEdit->text();
    QString prevHash = "";

    if (transactions.size() > 0)
        prevHash = transactions[transactions.size() - 1].toObject().value("hash").toString();

    qDebug() << "prevHash: " << prevHash;

    QJsonObject obj;
    obj.insert("amount", amount);
    obj.insert("wallet", wallet);
    obj.insert("date", date);

    QString sum = amount + wallet + date + prevHash;

    QString hash = QCryptographicHash::hash(sum.toUtf8(), QCryptographicHash::Sha256).toHex();

    obj.insert("hash", hash);

    transactions.append(obj);

    qDebug() << obj;

    ui->stackedWidget->setCurrentWidget(ui->page);

    listWidget->clear();

    QString previousHash = "";
    for (int i = 0; i < transactions.size(); ++i) {
        QJsonObject obj = transactions[i].toObject();
        QString amount = obj.value("amount").toString();
        QString wallet = obj.value("wallet").toString();
        QString date = obj.value("date").toString();
        QString hash = obj.value("hash").toString();

        bool valid = !checkTransaction(obj, previousHash);
        qDebug() << i << " " << valid;

        listWidget->addItem(createListItem(amount, wallet, date, hash, valid));

    }
}


void MainWindow::on_saveButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", "", "JSON Files (*.json)");

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    QJsonObject obj;
    obj.insert("transactions", transactions);

    QJsonDocument doc(obj);

    QByteArray encrypted;
    QByteArray data = doc.toJson(QJsonDocument::Indented);



    Encryptor::getInstance().encrypt(data, encrypted,
                                     QCryptographicHash::hash("1", QCryptographicHash::Sha256),
                                     QCryptographicHash::hash("1", QCryptographicHash::Md5));

    QTextStream out(&file);
    out << encrypted.toHex();
    file.close();

}


void MainWindow::on_openButton_clicked()
{

}

void MainWindow::checkRecordFields()
{
    if (ui->amountEdit->text().length() != 7) {
        ui->addBtn->setEnabled(false);
        return;
    }
    if (ui->walletEdit->text().length() != 6) {
        ui->addBtn->setEnabled(false);
        return;
    }
    if (ui->dateEdit->text().length() != 19) {
        ui->addBtn->setEnabled(false);
        return;
    }
    ui->addBtn->setEnabled(true);
    return;
}

void MainWindow::on_amountEdit_textChanged(const QString &arg1)
{
    checkRecordFields();
}


void MainWindow::on_walletEdit_textChanged(const QString &arg1)
{
    checkRecordFields();
}


void MainWindow::on_dateEdit_textChanged(const QString &arg1)
{
    checkRecordFields();
}

