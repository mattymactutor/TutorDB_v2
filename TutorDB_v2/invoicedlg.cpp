#include "invoicedlg.h"
#include "ui_invoicedlg.h"

InvoiceDlg::InvoiceDlg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InvoiceDlg)
{
    ui->setupUi(this);
}

InvoiceDlg::~InvoiceDlg()
{
    delete ui;
}

void InvoiceDlg::receiveData(Client * c, vector<Entry *> e)
{
    qDebug() << "Invoice for " << c->getName();
    double total = 0;
    double totalHrs = 0;
    QString inv = "------------------------------\n";
    // 6/29/22 [11:00-12:00] 1hrs - *note*
    for(unsigned int i = 0; i < e.size(); i++){
        //calc the end time first
        QTime endTime = QTime::fromString(e[i]->getTime(),"hh:mma");
        endTime = endTime.addSecs(e[i]->getLength() * 60 * 60);
        inv += e[i]->getDate() + " [ " + e[i]->getTime() + " - " + endTime.toString("hh:mma") + " ] " + QString::number(e[i]->getLength())
                + "hrs - " + e[i]->getNote() + "\n";
        total += e[i]->getAmount();
        totalHrs += e[i]->getLength();
    }
    inv += "\n\nTotal: $" + QString::number(total) + " ( " + QString::number(totalHrs) + "hrs @ " + QString::number(c->getRate()) + "/hr )\n";

    ui->txtInvoice->setPlainText(inv);

}
