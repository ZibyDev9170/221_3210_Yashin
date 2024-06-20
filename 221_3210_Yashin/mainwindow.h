#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QPushButton>
#include <QString>
#include <QByteArray>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleLogin();
    void handleOpenPromoCode();

private:
    Ui::MainWindow *ui;
    QVector<QByteArray> encryptedPromoCodes;
    QVector<QPushButton*> promoCard;
    QByteArray encryptionKey;

    void createPromoCards();
    void addNewPromoCard();
    QString generateRandomCode();
    QByteArray encryptPromoCode(const QString &promoCode);
    QString decryptPromoCode(const QByteArray &encryptedCode);
    QByteArray generateEncryptionKey(const QString &pin);
    QString hashPin(const QString &pin);
    void savePromoCodes();
    void loadPromoCodes();

protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
