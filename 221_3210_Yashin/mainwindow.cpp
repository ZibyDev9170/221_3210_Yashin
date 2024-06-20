#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QRandomGenerator>
#include <QGridLayout>
#include <openssl/sha.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lineEditPin->setEchoMode(QLineEdit::Password);

    connect(ui->pushButtonLogin, &QPushButton::clicked, this, &MainWindow::handleLogin);
    connect(ui->pushButtonOpenPromo, &QPushButton::clicked, this, &MainWindow::handleOpenPromoCode);

    createPromoCards();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Авторизация
void MainWindow::handleLogin()
{
    const QString correctPin = "7850"; // Верный PIN-код

    if (ui->lineEditPin->text() == correctPin) {
        encryptionKey = generateEncryptionKey(correctPin); // Генерирую код для шифрования и дешифрования
        ui->stackedWidget->setCurrentIndex(1);
    } else {
        ui->label->setText("Неверный пин!");
        ui->label->setStyleSheet("color:red");
    }
}

// Открыть карточку с промокодом
void MainWindow::handleOpenPromoCode()
{
    // Выбираем случайную карточку и отображаем ее промокод
    int randomIndex = QRandomGenerator::global()->bounded(promoCard.size());
    promoCard[randomIndex]->setText(promoCodes[randomIndex]);
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
    promoCodes.append(encryptedCode);

    QPushButton *newPromoCard = new QPushButton("Промокод");
    newPromoCard->setFixedSize(200, 100); // Ширина в 2 раза больше высоты

    int row = promoCard.size() / 4;
    int col = promoCard.size() % 4;
    QGridLayout *layout = qobject_cast<QGridLayout*>(ui->promoCodesPage->layout());
    if (layout) {
        layout->addWidget(newPromoCard, row, col);
    }

    promoCard.append(newPromoCard);
}

// Генерация случайного кода
QString MainWindow::generateRandomCode()
{
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int randomStringLength = 4;

    QString randomString;
    for (int i = 0; i < randomStringLength; ++i) {
        int index = QRandomGenerator::global()->bounded(possibleCharacters.length());
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }

    return randomString;
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

//QString MainWindow::decryptPromoCode(const QByteArray &encryptedCode)
//{

//}

// Функция для генерации ключа шифрования
QByteArray MainWindow::generateEncryptionKey(const QString &pin)
{
    QByteArray key(SHA256_DIGEST_LENGTH, 0);
    SHA256(reinterpret_cast<const unsigned char*>(pin.toUtf8().data()), pin.size(), reinterpret_cast<unsigned char*>(key.data()));
    key.resize(AES_BLOCK_SIZE);
    return key;
}
