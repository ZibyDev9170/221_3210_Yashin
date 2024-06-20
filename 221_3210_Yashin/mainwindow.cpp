#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QRandomGenerator>
#include <QGridLayout>

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
    promoCodes.append(newCode);

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
