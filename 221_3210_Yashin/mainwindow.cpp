#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QRandomGenerator>
#include <QGridLayout>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lineEditPin->setEchoMode(QLineEdit::Password);

    connect(ui->pushButtonLogin, &QPushButton::clicked, this, &MainWindow::handleLogin);
    connect(ui->pushButtonOpenPromo, &QPushButton::clicked, this, &MainWindow::handleOpenPromoCode);
    ui->stackedWidget->setCurrentIndex(0);

    createPromoCards();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Авторизация
void MainWindow::handleLogin()
{
    const QString pinFilePath = "C:/Users/nikya/Documents/Programming/221_3210_Yashin/221_3210_Yashin/pin_hash.txt";

    // QString enteredPin = ui->lineEditPin->text();
    // QString enteredHash = hashPin(enteredPin);
    // qDebug() << enteredHash; be180d34dddf670bded23c372ef94f41d135935bf9ddeaca77d11c1ac53a6bf3

    QFile file(pinFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл с хешем PIN-кода");
        return;
    }

    QTextStream in(&file);
    QString storedHash = in.readLine();
    file.close();

    QString enteredPin = ui->lineEditPin->text();
    QString enteredHash = hashPin(enteredPin);

    if (storedHash == enteredHash) {
    // if ("be180d34dddf670bded23c372ef94f41d135935bf9ddeaca77d11c1ac53a6bf3" == enteredHash) {
        // encryptionKey = generateEncryptionKey(enteredPin);
        ui->stackedWidget->setCurrentIndex(1);
    } else {
        QMessageBox::warning(this, "Ошибка", "Неверный PIN-код");
    }
}

// Открыть карточку с промокодом
void MainWindow::handleOpenPromoCode()
{
    int randomIndex = QRandomGenerator::global()->bounded(promoCard.size());
    QString decryptedCode = decryptPromoCode(encryptedPromoCodes[randomIndex]);
    promoCard[randomIndex]->setText(decryptedCode);
    promoCard[randomIndex]->setStyleSheet("color:blue");

    addNewPromoCard();
}

// Создание карточек с промокодом
void MainWindow::createPromoCards()
{
    QGridLayout *layout = new QGridLayout;
    ui->promoCodesPage->setLayout(layout);

    for (int i = 0; i < 9; ++i) {
        addNewPromoCard();
    }
}

// Добавление нового промокода
void MainWindow::addNewPromoCard()
{
    QString newCode = generateRandomCode();
    QByteArray encryptedCode = encryptPromoCode(newCode); // Шифрование
    encryptedPromoCodes.append(encryptedCode);

    QPushButton *newPromoCard = new QPushButton("Промокод");
    newPromoCard->setFixedSize(200, 100); // Ширина в 2 раза больше высоты

    int row = promoCard.size() / 4;
    int col = promoCard.size() % 4;
    QGridLayout *layout = qobject_cast<QGridLayout*>(ui->gridLayout->layout());
    if (layout) {
        layout->addWidget(newPromoCard, row, col);
    }

    promoCard.append(newPromoCard);
}

// Генерация случайного кода
QString MainWindow::generateRandomCode()
// {
//     const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
//     const int randomStringLength = 4;

//     QString randomString;
//     for (int i = 0; i < randomStringLength; ++i) {
//         int index = QRandomGenerator::global()->bounded(possibleCharacters.length());
//         QChar nextChar = possibleCharacters.at(index);
//         randomString.append(nextChar);
//     }

//     return randomString;
// }
{
    const int codeLength = 4;
    QString randomCode;

    for (int i = 0; i < codeLength; ++i) {
        int randomCharType = QRandomGenerator::global()->bounded(3);
        QChar nextChar;

        if (randomCharType == 0) {
            nextChar = QChar('0' + QRandomGenerator::global()->bounded(10));
        } else if (randomCharType == 1) {
            nextChar = QChar('A' + QRandomGenerator::global()->bounded(26));
        } else {
            nextChar = QChar('a' + QRandomGenerator::global()->bounded(26));
        }

        randomCode.append(nextChar);
    }

    return randomCode;
}

// Шифрование промокода
QByteArray MainWindow::encryptPromoCode(const QString &promoCode)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        // Обработка ошибки инициализации
        return QByteArray();
    }

    QByteArray encryptedCode;
    int len;
    int ciphertext_len;
    unsigned char iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, AES_BLOCK_SIZE); // Генерируем случайный вектор инициализации

    encryptedCode.resize(promoCode.size() + AES_BLOCK_SIZE);

    // Инициализируем шифрование
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, reinterpret_cast<const unsigned char*>(encryptionKey.data()), iv)) {
        // Обработка ошибки инициализации
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    // Обновляем буфер с зашифрованными данными
    if (1 != EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(encryptedCode.data()), &len, reinterpret_cast<const unsigned char*>(promoCode.toUtf8().data()), promoCode.size())) {
        // Обработка ошибки обновления
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    ciphertext_len = len;

    // Завершаем шифрование
    if (1 != EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(encryptedCode.data()) + len, &len)) {
        // Обработка ошибки завершения
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    ciphertext_len += len;

    encryptedCode.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);
    return encryptedCode;
}

QString MainWindow::decryptPromoCode(const QByteArray &encryptedCode)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        // Обработка ошибки инициализации
        return QString();
    }

    QByteArray decryptedCode;
    int len;
    int plaintext_len;
    unsigned char iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, AES_BLOCK_SIZE); // Генерируем случайный вектор инициализации

    decryptedCode.resize(encryptedCode.size());

    // Инициализируем дешифрование
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, reinterpret_cast<const unsigned char*>(encryptionKey.data()), iv)) {
        // Обработка ошибки инициализации
        EVP_CIPHER_CTX_free(ctx);
        qDebug() << "1";
        return QString();
    }

    // Обновляем буфер с расшифрованными данными
    if (1 != EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(decryptedCode.data()), &len, reinterpret_cast<const unsigned char*>(encryptedCode.data()), encryptedCode.size())) {
        // Обработка ошибки обновления
        EVP_CIPHER_CTX_free(ctx);
        qDebug() << "2";
        return QString();
    }
    plaintext_len = len;

    // Завершаем дешифрование
    if (1 != EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(decryptedCode.data()) + len, &len)) {
        // Обработка ошибки завершения
        EVP_CIPHER_CTX_free(ctx);
        qDebug() << "3";
        return QString();
    }
    plaintext_len += len;

    decryptedCode.resize(plaintext_len);
    EVP_CIPHER_CTX_free(ctx);
    return QString::fromUtf8(decryptedCode);
}

// Функция для генерации ключа шифрования
QByteArray MainWindow::generateEncryptionKey(const QString &pin)
{
    QByteArray key(SHA256_DIGEST_LENGTH, 0);
    SHA256(reinterpret_cast<const unsigned char*>(pin.toUtf8().data()), pin.size(), reinterpret_cast<unsigned char*>(key.data()));
    key.resize(AES_BLOCK_SIZE);
    return key;
}

// Функция генерации хэша для пина
QString MainWindow::hashPin(const QString &pin)
{
    QByteArray hash(SHA256_DIGEST_LENGTH, 0);
    SHA256(reinterpret_cast<const unsigned char*>(pin.toUtf8().data()), pin.size(), reinterpret_cast<unsigned char*>(hash.data()));
    // qDebug() << QString::fromLatin1(hash.toHex());
    return QString::fromLatin1(hash.toHex());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    savePromoCodes();
    event->accept();
}

// Функция сохранения
void MainWindow::savePromoCodes()
{
    QJsonArray promoArray;
    for (int i = 0; i < encryptedPromoCodes.size(); ++i) {
        QJsonObject promoObject;
        promoObject["code"] = QString(encryptedPromoCodes[i].toHex());
        promoObject["state"] = promoCard[i]->text();
        promoArray.append(promoObject);
    }

    QJsonObject rootObject;
    rootObject["promos"] = promoArray;

    QJsonDocument doc(rootObject);
    QByteArray jsonData = doc.toJson();

    // Шифруем JSON данные
    QByteArray encryptedData = encryptPromoCode(QString(jsonData));

    QFile file("C:/Users/nikya/Documents/Programming/221_3210_Yashin/221_3210_Yashin/json/promos.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(encryptedData);
        file.close();
    }
}
