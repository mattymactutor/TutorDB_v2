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

void InvoiceDlg::receiveData(Client * c, vector<Entry> e)
{
    qDebug() << "Invoice for " << c->getName();
    double total = 0;
    double totalHrs = 0;
    QString inv = "------------------------------\n";
    // 6/29/22 [11:00-12:00] 1hrs - *note*
    for(unsigned int i = 0; i < e.size(); i++){
        //calc the end time first
        //first get the start time
        QTime endTime = QTime::fromString(e[i].getTime(),"hh:mma");
        //convert the decimal length to seconds so you can add it to the start time
        //if it's 1.8 it won't get the time right it will do 2:00-3:48 instead of 3:50
        //because the decimals get dropped when the length is calculated
       // endTime = endTime.addSecs();
        int minutes = e[i].getLength() * 60;
        //if the minutes is not divisible by
        if (minutes % 5 != 0 ){
            //get the remainder and that tells you how much you need to add
            // 48 % 5 gives 3
            //we should add 2 so 5 - 3
            minutes += 5 - (minutes % 5);
        }
        endTime = endTime.addSecs(minutes * 60);
        inv += e[i].getDate() + " [ " + e[i].getTime() + " - " + endTime.toString("hh:mma") + " ] " + QString::number(e[i].getLength())
                + "hrs - " + e[i].getNote() + "\n";
        total += e[i].getAmount();
        totalHrs += e[i].getLength();
    }
    inv += "\n\nTotal: $" + QString::number(total) + " ( " + QString::number(totalHrs) + "hrs @ " + QString::number(c->getRate()) + "/hr )\n";

    ui->txtInvoice->setPlainText(inv);

}
