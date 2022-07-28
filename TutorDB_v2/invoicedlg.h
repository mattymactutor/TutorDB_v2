#ifndef INVOICEDLG_H
#define INVOICEDLG_H

#include <QWidget>
#include "Client.h"
#include "Entry.h"

namespace Ui {
class InvoiceDlg;
}

class InvoiceDlg : public QWidget
{
    Q_OBJECT

public:
    explicit InvoiceDlg(QWidget *parent = nullptr);
    ~InvoiceDlg();

private slots:
    void receiveData(Client *, vector<Entry *>);

private:
    Ui::InvoiceDlg *ui;
};

#endif // INVOICEDLG_H
