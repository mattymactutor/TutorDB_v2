#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "Client.h"
#include "firebasehandler.h"
#include "invoicedlg.h"
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool isEventInside(QTextEdit * edit, QWheelEvent * event);



signals:
    void sendToInvoiceDlg(Client *, vector<Entry>);

private slots:
    void on_lstClients_itemClicked(QListWidgetItem *item);
    void on_srchClient_textChanged();
    void on_tblLogs_cellChanged(int row, int column);
    void on_tblLogs_cellClicked(int row, int column);
    void on_actionClear_triggered();
    void on_actionNew_Client_triggered();
    void on_edtName_textChanged();
    void on_btnSave_clicked();
    void on_actionDelete_Client_triggered();
    void on_btnAddLog_clicked();
    void wheelEvent(QWheelEvent * event);
    void handleRightClick(const QPoint& pos);
    void on_actionDelete_triggered();
    void on_actionMakeInvoice_triggered();
    void on_actionSendToPhone_triggered();


    void on_actionMarkAsSent_triggered();

    void on_actionMarkAsPaid_triggered();

    void on_edtLength_textChanged();

private:
    Ui::MainWindow *ui;
    FirebaseHandler * db;
    InvoiceDlg * invoiceDlg;

};
#endif // MAINWINDOW_H
