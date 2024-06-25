#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <QByteArray>
#include <QDataStream>
#include <QTextStream>
#include <QIODevice>
#include <QFile>
#include <QDebug>
#include <openssl/evp.h>

class Encryptor
{
public:
    static Encryptor& getInstance()
    {
        static Encryptor singleton;
        return singleton;
    }

    // ������� ���������
    int encrypt(QByteArray &in, QByteArray &out, QByteArray QBAkey, QByteArray QBAiv);
    // ������� �����������
    int decrypt(QByteArray &in, QByteArray &out, QByteArray QBAkey, QByteArray QBAiv);
private:
    Encryptor(){};
    Encryptor(const Encryptor& root) = delete;
    Encryptor& operator=(const Encryptor&) = delete;
};

#endif // ENCRYPTOR_H
