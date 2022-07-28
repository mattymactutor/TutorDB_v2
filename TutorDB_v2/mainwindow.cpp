#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "firebasehandler.h"
#include <QMessageBox>
#include <QClipboard>
#include <QTextEdit>
#include <QWheelEvent>
#include <vector>
#include "Entry.h"


/*
 TODO
 The total amt owed did not update as new lessons came in
 Need a way to mark all the highlighted rows as sent and maybe just auto do it from the create invoice screen, make sent button
 */

/*
 * Compile Web Assembly
go to cmd prompt
cd C:\emsdk
emsdk activate 1.39.8

apparently "./emsdk activate 1.39.8 --permanent" makes it permanent

run "em++ -- version" to make sure you can access it

from here make a wasm build folder
buildWebAsm_PROJECT_NAME

navigate to this folder from same terminal
run the wasm32 qmake on the folder where the actual QTCreator Project file is
C:\Qt\6.2.3\wasm_32\bin\qmake C:\users\matty\Documents\QT_Projects\PROJECT_NAME

This creates a makefile in the build directory
run "mingw32-make" in the build directory and it should compile the webasm

To host the website you need
app.html
qtloader.js
app.js
app.wasm
 */

const QString Client::KEY_NAME = "Name";
const QString Client::KEY_PHONE = "Phone";
const QString Client::KEY_PAYTYPE = "PayType";
const QString Client::KEY_PAYINFO = "PayInfo";
const QString Client::KEY_EMAIL = "Email";
const QString Client::KEY_AMT_OWED = "AmtOwed";
const QString Client::KEY_RATE = "Rate";
const QString Client::KEY_LOGS = "Logs";

const QString Entry::KEY_CID = "CID";
const QString Entry::KEY_DATE = "Date";
const QString Entry::KEY_LENGTH = "Length";
const QString Entry::KEY_AMT = "Amt";
const QString Entry::KEY_NOTE = "Note";
const QString Entry::KEY_STATUS = "Paid";

const int COL_DATE = 0;
const int COL_TIME = 1;
const int COL_LENGTH = 2;
const int COL_AMT = 3;
const int COL_STATUS = 4;
const int COL_NOTE = 5;

bool loadedClient = false;
vector<Client> dbClients;
vector<Client*> displayClients;
Client * selectedClient = nullptr;
int idxSelClient = -1;
Ui::MainWindow * ui2;
bool sortLogsAsc = true;
double totalAmtOwed = 0;

QString emailSignedIn;

void showClientList(){
    ui2->lstClients->clear();
    for(unsigned int i = 0; i < displayClients.size(); i++){
        QString display = displayClients[i]->getName() + ((displayClients[i]->getAmtOwed() > 0) ? "***" : "");
        ui2->lstClients->addItem(display);
    }
    //select what was selected
    if (idxSelClient != -1){
        ui2->lstClients->setCurrentRow(idxSelClient);
    }
}

void updateClient(Client * c){
    for(unsigned int i =0; i < dbClients.size(); i++){
        if (c->getUID() == dbClients[i].getUID()){
            dbClients[i] = *c;
            break;
        }
    }

}

void filter(QString q){
    displayClients.clear();
    for(unsigned int i = 0; i < dbClients.size(); i++){
        if (dbClients[i].getName().contains(q) || (q == "*" && dbClients[i].getAmtOwed() > 0)){
            displayClients.push_back(&dbClients[i]);
        }
    }
}

void firstLoad(vector<Client> loadClients, QString selected){
    dbClients = loadClients;

    //sort the clients in alphabetical order
    sort(dbClients.begin(),dbClients.end(), Client::sortByName);
    displayClients.clear();
    for (unsigned int i = 0; i < dbClients.size() ; i++ ) {
        displayClients.push_back(&dbClients[i]);
        totalAmtOwed += dbClients[i].getAmtOwed();
    }
    showClientList();
    //if new client highlight them
    if (idxSelClient == -2){
        for (unsigned int i = 0; i < displayClients.size() ; i++ ) {
            if (displayClients[i]->getUID() == selected){
                ui2->lstClients->setCurrentRow(i);
                idxSelClient = i;
                selectedClient = displayClients[i];
                break;
            }
        }
    }
    ui2->lblTotalAmtOwed->setText(("Total Owed: $" + QString::number(totalAmtOwed)));

   }


//Go through local database and update the total amount owed
void updateTotalAmtOwed(){
    double total = 0;
    for(int i = 0; i < dbClients.size(); i++){
        total += dbClients[i].getAmtOwed();
    }
    ui2->lblTotalAmtOwed->setText(("Total Owed: $" + QString::number(total)));

}

void showLogs(){
    loadedClient = false;
    if (selectedClient != nullptr){
        double amtOwed = 0;
        //ui2->tblLogs->clearContents();
        ui2->tblLogs->model()->removeRows(0, ui2->tblLogs->rowCount());
       // vector<Entry> selectedClient->getLogs() =  selectedClient->getLogs();

        //show the logs sorted
        if (sortLogsAsc){
            sort(selectedClient->getLogs().begin(),selectedClient->getLogs().end(),Entry::sortByDate_asc);
        } else {
            sort(selectedClient->getLogs().begin(),selectedClient->getLogs().end(),Entry::sortByDate_dec);
        }

        for(unsigned int i = 0; i < selectedClient->getLogs().size(); i++){
            Entry c = selectedClient->getLogs()[i];
            ui2->tblLogs->insertRow(i);
            QString date = c.getDate();
            QString time = c.getTime();
            ui2->tblLogs->setItem(i,0, new QTableWidgetItem(date));
            ui2->tblLogs->setItem(i,1, new QTableWidgetItem(time));
            ui2->tblLogs->setItem(i,2, new QTableWidgetItem(QString::number(c.getLength())));
            ui2->tblLogs->setItem(i,3, new QTableWidgetItem(QString::number(c.getAmount())));
            ui2->tblLogs->setItem(i,4, new QTableWidgetItem(c.getStatus()));
            ui2->tblLogs->setItem(i,5, new QTableWidgetItem(c.getNote()));
            if (c.getStatus() != "PAID"){
                amtOwed += c.getAmount();
            }
        }
        ui2->edtAmtOwed->setText(QString::number(amtOwed));
        selectedClient->setAmtOwed(amtOwed);
    }
    loadedClient = true;
    updateTotalAmtOwed();
}




/*
 * This function upates the amount owed on firebase for one client
 * Fires after changing a cell that affects the amount owed
 */
double updateAmtOwed(vector<Entry> logs, FirebaseHandler * db){
    double total=0;
    for(auto it = logs.begin(); it != logs.end(); it++){
        if (it->getStatus() != "PAID"){
         total += it->getAmount();
        }
    }

    selectedClient->setAmtOwed(total);
    ui2->edtAmtOwed->setText(QString::number(total));
    updateTotalAmtOwed();
    QVariantMap updateData;
    updateData[Client::KEY_AMT_OWED] = total;
    db->updateClientAttribute(selectedClient->getUID(), updateData);
    showClientList();
    return total;
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui2 = ui;
    db = new FirebaseHandler();

    //signUpUser("mattymactutor@gmail.com","password");

    //in the future pull this from a prefs file
    emailSignedIn = "mattymactutor@gmail.com";
    db->signInUser(emailSignedIn,"password", firstLoad);
    setWindowTitle("Tutor Database - " + emailSignedIn);


   // db->signInUser("test@gmail.com","password");

    //make all the edit texts fire off the same text changed listener - this enables the save button
    connect(ui->edtPayType,&QTextEdit::textChanged,this, &MainWindow::on_edtName_textChanged);
    connect(ui->edtPayInfo,&QTextEdit::textChanged,this, &MainWindow::on_edtName_textChanged);
    connect(ui->edtEmail,&QTextEdit::textChanged,this, &MainWindow::on_edtName_textChanged);
    connect(ui->edtPhone,&QTextEdit::textChanged,this, &MainWindow::on_edtName_textChanged);
    connect(ui->edtRate,&QTextEdit::textChanged,this, &MainWindow::on_edtName_textChanged);

    //connect the right click of the table to the function
    connect(ui->tblLogs, &QTableWidget::customContextMenuRequested, this, &MainWindow::handleRightClick);

    //set the onclick for the date header
    auto header = ui->tblLogs->horizontalHeader();
    connect(header, &QHeaderView::sectionClicked, [this](int idx){
        QString text = ui->tblLogs->horizontalHeaderItem(idx)->text();
        qDebug() << idx << text;
        if (idx == 0){
            sortLogsAsc = !sortLogsAsc;
            showLogs();
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_lstClients_itemClicked(QListWidgetItem *item)
{
    //QModelIndexList selection = ui->lstClients->selectionModel()->selectedIndexes();
    int idx = ui->lstClients->row(item);
    selectedClient = displayClients[idx];
    for(unsigned int i =0; i < dbClients.size(); i++){
        if (dbClients[i].getUID() == selectedClient->getUID()){
            idxSelClient = i;
            break;
        }
    }

    ui->edtName->setText(selectedClient->getName());
    ui->edtPhone->setText(selectedClient->getPhone());
    ui->edtPayInfo->setText(selectedClient->getPayinfo());
    ui->edtPayType->setText(selectedClient->getPaytype());
    ui->edtEmail->setText(selectedClient->getEmail());
    ui->edtRate->setText(QString::number(selectedClient->getRate()));
    ui->edtAmtOwed->setText(QString::number(selectedClient->getAmtOwed()));

    showLogs();
    //create the defaults for one lesson
    ui->edtLength->setText("1");
    //TODO copy this later
    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime = date.toString("MM-dd-yyyy");
    ui->edtDate->setText(formattedTime);

    QTime qtimeObj = QTime::currentTime();
    //round back to the nearest hour
    int mins = qtimeObj.minute();
    //subtract that many seconds from the time
    qtimeObj = qtimeObj.addSecs(-60 * mins);
    QString strTime = qtimeObj.toString("hh:mma");
    ui->edtTime->setText(strTime);
    ui->edtAmount->setText(QString::number( selectedClient->getRate()));
}


void MainWindow::on_srchClient_textChanged()
{
    QString query = ui->srchClient->toPlainText();
    filter(query);
    showClientList();
}


void MainWindow::on_tblLogs_cellChanged(int row, int column)
{

    if (loadedClient){
        QTableWidgetItem * cell = ui->tblLogs->item(row,column);
        QTableWidgetItem * changeCell;

        Entry * logToChange = displayClients[idxSelClient]->getLog(row);
        double l, amt;
        QVariantMap updateData;
         //change the data in the current client
        switch(column){
            //the date and time are combined on firebase so have to update that for both
            case COL_DATE:
                logToChange->setDate(cell->text());
                updateData[Entry::KEY_DATE] = cell->text() + " " + logToChange->getTime();
                break;
            case COL_TIME:
                logToChange->setTime(cell->text());
                updateData[Entry::KEY_DATE] = logToChange->getDate() + " " + cell->text();
                break;
            case COL_AMT:
                logToChange->setAmount(cell->text().toDouble());
                updateData[Entry::KEY_AMT] = cell->text().toDouble();
                if (logToChange->getStatus() != "PAID"){
                    updateAmtOwed(displayClients[idxSelClient]->getLogs(),db);
                }
                break;
            case COL_LENGTH:
                //also need to update the amount
                l = cell->text().toDouble(); //get length from table
                logToChange->setLength(l); //set it in local db
                amt = ceil(l * dbClients[idxSelClient].getRate()); //calc new amt
                logToChange->setAmount(amt); //set new amt
                changeCell = ui->tblLogs->item(row,COL_AMT); //grab the amt cell
                changeCell->setText(QString::number(amt)); //set amt cell text

                //firebase update length and amount
                updateData[Entry::KEY_LENGTH] = l;
                updateData[Entry::KEY_AMT] = amt;
                if (logToChange->getStatus() != "PAID"){
                   updateAmtOwed(displayClients[idxSelClient]->getLogs(),db);
                }
                break;
            case COL_STATUS:
                logToChange->setStatus(cell->text());
                updateData[Entry::KEY_STATUS] = cell->text();
                //if it changes to paid or unpaid we need to recalculte, no change when unpaid to sent
                if (logToChange->getStatus() != "SENT"){
                   updateAmtOwed(displayClients[idxSelClient]->getLogs(),db);
                }
                break;
            case COL_NOTE:
                logToChange->setNote(cell->text());
                updateData[Entry::KEY_NOTE] = cell->text();
                break;
        }

        db->updateEntryAttribute(logToChange->getCID(),logToChange->getLID(),updateData);
    }
}


void MainWindow::on_tblLogs_cellClicked(int row, int column)
{
    if (column == COL_STATUS){
        QTableWidgetItem * cell = ui->tblLogs->item(row,column);

        if (cell->text() == "UNPAID"){
            cell->setText("SENT");
        } else if (cell->text() == "SENT"){
            cell->setText("PAID");
        } else if (cell->text() == "PAID"){
            cell->setText("UNPAID");
        }
        //this gets saved because it counts as a changed cell

    }
}

void clearFields(){
    ui2->edtAmount->setText("");
    ui2->edtAmtOwed->setText("");
    ui2->edtDate->setText("");
    ui2->edtEmail->setText("");
    ui2->edtLength->setText("");
    ui2->edtName->setText("");
    ui2->edtPayInfo->setText("");
    ui2->edtPayType->setText("");
    ui2->edtPhone->setText("");
    ui2->edtRate->setText("");
    ui2->edtTime->setText("");
    ui2->tblLogs->setRowCount(0);

    selectedClient = nullptr;
    idxSelClient = -1;
    ui2->lstClients->clearSelection();
}

void MainWindow::on_actionClear_triggered()
{
    clearFields();
}


void MainWindow::on_actionNew_Client_triggered()
{
    QMessageBox msgBox;
    msgBox.setText("Make a new client from the current information?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Save){
        QString name = ui->edtName->toPlainText();
        QString phone = ui->edtPhone->toPlainText();
        QString email = ui->edtEmail->toPlainText();
        QString payType = ui->edtPayType->toPlainText();
        QString payInfo =  ui->edtPayInfo->toPlainText();
        double amtOwed = ui->edtAmtOwed->toPlainText().toDouble();
        double hourly = ui->edtRate->toPlainText().toDouble();
        idxSelClient = -2;
        db->addClient(name,phone,email,payType,payInfo,amtOwed,hourly);
    }
}


void MainWindow::on_edtName_textChanged()
{
    if (loadedClient){
        ui->btnSave->setEnabled(true);
    }
}


void MainWindow::on_btnSave_clicked()
{
    QString name = ui->edtName->toPlainText();
    QString phone = ui->edtPhone->toPlainText();
    QString email = ui->edtEmail->toPlainText();
    QString payType = ui->edtPayType->toPlainText();
    QString payInfo =  ui->edtPayInfo->toPlainText();
    double amtOwed = ui->edtAmtOwed->toPlainText().toDouble();
    double hourly = ui->edtRate->toPlainText().toDouble();

    //if the name you are about to put in is different than what is there
    //you need to reload the client list
    QString prevName = selectedClient->getName();
    //change to possible new name
    selectedClient->setName(name);
    selectedClient->setPayinfo(payInfo);
    selectedClient->setPhone(phone);
    selectedClient->setEmail(email);
    selectedClient->setPaytype(payType);
    selectedClient->setAmtOwed(amtOwed);
    selectedClient->setRate(hourly);
    db->updateClient(selectedClient);
    ui->btnSave->setEnabled(false);

    //check if the name was changed and reload client list if so
    if (prevName != name){
        showClientList();
    }
}


void MainWindow::on_actionDelete_Client_triggered()
{
    if (selectedClient != nullptr){

        QMessageBox msgBox;
        msgBox.setText("Are you sure you want to delete " + selectedClient->getName() + "?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes){
            db->deleteClient(selectedClient->getUID(), [](){
                clearFields();
            });
        }
    }
}


void MainWindow::on_btnAddLog_clicked()
{
    if (selectedClient != nullptr){
        double length = ui->edtLength->toPlainText().toDouble();
        double amt = ui->edtAmount->toPlainText().toDouble();
        QString date = ui->edtDate->toPlainText();
        QString time = ui->edtTime->toPlainText();

        db->addEntry(selectedClient->getUID(),length,amt,date,time,"", [this](Entry e){
            selectedClient->addLog(e);
            //show the new log that was just added
            showLogs();
            //update the information on firebase
            QVariantMap updateData;
            updateData[Client::KEY_AMT_OWED] = selectedClient->getAmtOwed();
            db->updateClientAttribute(e.getCID(), updateData);
            //calc the new amt owed
            updateTotalAmtOwed();
        });
    }
}

bool MainWindow::isEventInside(QTextEdit * edit, QWheelEvent * event){

    int x = event->position().x();
    int y = event->position().y();

    int widgetX = edit->geometry().x();
    int widgetY = edit->geometry().y() + 10; //i think you need a little offset because of the file menu bar?

    //if this is the length or the date you have to add the x of the group to get the actual real x
    if (edit == ui->edtLength || edit == ui->edtTime || edit == ui->edtDate){
        widgetX += ui->grpAddLesson->geometry().x();
        widgetY += ui->grpAddLesson->geometry().y();       
    }

    int width = edit->geometry().width();
    int height = edit->geometry().height();

    //qDebug("x: %d  y: %d    tX: %d  tY: %d      wid: %d  height: %d",x,y,widgetX,widgetY,width,height);
    if ( widgetX <= x && x <= widgetX + width && widgetY <= y && y <= widgetY + height ){
        return true;
    } else {
        return false;
    }

}

void MainWindow::wheelEvent(QWheelEvent * event){

    if ( isEventInside(ui->edtLength,event)){

        //get the length and increase it by 0.25 on scrolss
        double length = ui->edtLength->toPlainText().toDouble();
        double degrees = event->angleDelta().y();

        if (degrees > 0){
            length += 0.25;
        } else {
            length -= 0.25;
        }

        ui->edtLength->setPlainText(QString::number(length) );
        double rate = ui->edtRate->toPlainText().toDouble();
        ui->edtAmount->setPlainText( QString::number( ceil(length * rate) ) );
    } else if (isEventInside(ui->edtDate, event)){

        QString time = ui->edtDate->toPlainText();
        QDate date = QDate::fromString(time, "MM-dd-yyyy");

        double degrees = event->angleDelta().y();
        if (degrees > 0) {
            date = date.addDays(1);
        } else {
            date = date.addDays(-1);
        }

        ui->edtDate->setPlainText(date.toString("MM-dd-yyyy"));
    } else if (isEventInside(ui->edtTime, event)){

        QString timeStr = ui->edtTime->toPlainText();
        QTime time = QTime::fromString(timeStr, "hh:mma");

        double degrees = event->angleDelta().y();
        if (degrees > 0) {
            time = time.addSecs(60 * 15); //add 15 mins
        } else {
            time = time.addSecs(60 * -15); //subtract 15 mins
        }

        ui->edtTime->setPlainText(time.toString("hh:mma"));
    }



}
//Right Click Menu
void MainWindow::handleRightClick(const QPoint& pos)
{

    //create delete action
    QAction * delAct = new QAction("Delete",this);//tr("&New"), this);
    delAct->setStatusTip(tr("Delete this entry"));
    //connect the triggered of the delete action to the on delete slot
    connect(delAct, &QAction::triggered, this, &MainWindow::on_actionDelete_triggered);

    //create the make invoice action
    QAction * invAct = new QAction("Make Invoice",this);
    invAct->setStatusTip(tr("Make an invoice from the highlighted logs"));
    connect(invAct, &QAction::triggered, this, &MainWindow::on_actionMakeInvoice_triggered);

    //create the send to phone action
    QAction * sendToPhoneAct = new QAction("Send to Phone",this);
    sendToPhoneAct->setStatusTip(tr("Copy this lesson to clipboard"));
    connect(sendToPhoneAct, &QAction::triggered, this, &MainWindow::on_actionSendToPhone_triggered);

    //create the mark as sent action
    QAction * markAsSentAct = new QAction("Mark As Sent",this);
    markAsSentAct->setStatusTip(tr("Mark these lessons as sent"));
    connect(markAsSentAct, &QAction::triggered, this, &MainWindow::on_actionMarkAsSent_triggered);

    //create the mark as paid action
    QAction * markAsPaidAct = new QAction("Mark As Paid",this);
    markAsPaidAct->setStatusTip(tr("Mark these lessons as paid"));
    connect(markAsPaidAct, &QAction::triggered, this, &MainWindow::on_actionMarkAsPaid_triggered);

    QMenu menu(this);
    menu.addAction(delAct);
    menu.addAction(invAct);
    menu.addAction(sendToPhoneAct);
    menu.addAction(markAsSentAct);
    menu.addAction(markAsPaidAct);
    //the position that was clicked is originally based on the local x-y of the table
    //we have to map that to the overall xy point so we can show the menu in the correct spot
    menu.exec(ui->tblLogs->mapToGlobal(pos));

}


void onComplete_deleteEntry(Entry * e, int clickedRow){
    //the entry is already deleted from firebase and the local dbClients in firebase manager
    //get rid of the log here in the table and also the local data

    for(auto it = selectedClient->getLogs().begin(); it != selectedClient->getLogs().end(); it++){
        if (it->getLID() == e->getLID()){
            selectedClient->getLogs().erase(it);
            break;
        }
    }
    //update the visual
    ui2->tblLogs->removeRow(clickedRow);
}

void MainWindow::on_actionDelete_triggered()
{


    //are you sure pop up
    QMessageBox msgBox;
    msgBox.setText("Are you sure you want to delete this log(s)?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if (ret == QMessageBox::No){
        return;
    }

    QModelIndexList selection = ui->tblLogs->selectionModel()->selectedIndexes();

    // Multiple rows can be selected, but you have to delete them in reverse order
    //say you have
    //entry1
    //entry2
    //if you delete index 0 first then when you go to delte index1 that no longer exists
    for(int i=selection.count()-1; i >= 0; i--)
    {
        QModelIndex index = selection.at(i);      
        //get pointer to this log
        db->deleteEntry(selectedClient->getLog(index.row()), onComplete_deleteEntry,index.row() );
    }


}


void MainWindow::on_actionMakeInvoice_triggered()
{
    qDebug() << "create invoice for " << selectedClient->getName();
    QModelIndexList selection = ui->tblLogs->selectionModel()->selectedIndexes();

    vector<Entry*> invoiceEntries;

    // Multiple rows can be selected, but you have to delete them in reverse order
    //say you have
    //entry1
    //entry2
    //if you delete index 0 first then when you go to delte index1 that no longer exists
    for(int i = 0; i < selection.count(); i++)
    {
        QModelIndex index = selection.at(i);
        qDebug() << index.row();
        invoiceEntries.push_back(selectedClient->getLog(index.row()));
    }

    //send the current client and the selected logs to the invoice dialog
    invoiceDlg = new InvoiceDlg();
    connect(this, SIGNAL(sendToInvoiceDlg(Client*,vector<Entry*>)), invoiceDlg, SLOT(receiveData(Client*,vector<Entry*>)));
    invoiceDlg->show();
    emit sendToInvoiceDlg(selectedClient,invoiceEntries);

}


void MainWindow::on_actionSendToPhone_triggered()
{

    QModelIndexList selection = ui->tblLogs->selectionModel()->selectedIndexes();
    QString output = "";

    // Multiple rows can be selected, but you have to delete them in reverse order
    //say you have
    //entry1
    //entry2
    //if you delete index 0 first then when you go to delte index1 that no longer exists
    for(int i = 0; i < selection.count(); i++)
    {
        QModelIndex index = selection.at(i);
        Entry * e = selectedClient->getLog(index.row());
        output += QString::number(e->getLength()) + "hrs " + e->getDate().split(" ")[0] + " | ";
    }

    //the last 3 chars can come off
    if (output.length() > 0){
        output = output.left( output.length() - 3);
        qDebug() << output;
    }
    QClipboard * clipboard = QApplication::clipboard();
    clipboard->setText(output);
    qDebug() << "copied to clipboard";
}

void markAs(QString s){
    QModelIndexList selection = ui2->tblLogs->selectionModel()->selectedIndexes();
    for(int i = 0; i < selection.count(); i++)
    {
        QModelIndex index = selection.at(i);
        Entry * e = selectedClient->getLog(index.row());
        e->setStatus(s);
    }
}

void MainWindow::on_actionMarkAsSent_triggered()
{
    markAs("SENT");
    showLogs();
}




void MainWindow::on_actionMarkAsPaid_triggered()
{
    markAs("PAID");
    showLogs();
}
