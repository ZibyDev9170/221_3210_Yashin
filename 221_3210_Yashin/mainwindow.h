#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QPushButton>

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
    QVector<QString> promoCodes;
    QVector<QPushButton*> promoCard;

    void createPromoCards();
    void addNewPromoCard();
    QString generateRandomCode();
};
#endif // MAINWINDOW_H
