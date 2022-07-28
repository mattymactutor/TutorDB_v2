#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <QJsonObject>
#include <QVariantMap>
using namespace std;

class Entry{
private:
    QString CID; //client id
    QString LID; //log id
    QString date; //the dater and time of lesson
    QString time;
    double length; //length of time in hours
    double amount; //total amount for the lesson
    QString status; //UNPAID, SENT or PAID
    QString note; //what happened in the lesson or the work log

public:
    Entry( QString CID, QString date,double length,double amount,QString status,QString note){
        this->CID = CID;
        this->date = date.split(" ")[0];
        this->time = date.split(" ")[1];
        this->length = length;
        this->amount = amount;
        this->status = status;
        this->note = note;
        this->LID = "NEW";
    }

    Entry( QJsonObject in, QString LID, QString CID = ""){
        this->CID = CID; //cid will be passed in from the client
        this->LID = LID;
        QString dateTime = in.value(KEY_DATE).toString();
        this->date = dateTime.split(" ")[0];
        this->time = dateTime.split(" ")[1];
        this->length = in.value(KEY_LENGTH).toDouble();
        this->amount = in.value(KEY_AMT).toDouble();
        this->status = in.value(KEY_STATUS).toString();
        this->note = in.value(KEY_NOTE).toString();
    }

    Entry( QVariantMap in){
        this->note = in[KEY_NOTE].toString();
        this->amount = in[KEY_AMT].toDouble();
        this->length = in[KEY_LENGTH].toDouble();
        QString dateTime = in[KEY_DATE].toString();
        this->date = dateTime.split(" ")[0];
        this->time = dateTime.split(" ")[1];
        this->status = in[KEY_STATUS].toString();
    }

    void setCID(QString CID) { this->CID = CID; }
    QString getCID() const{ return this->CID; }
    void setDate(QString date) { this->date = date; }
    QString getDate() const{ return this->date; }
    void setLength(double length) { this->length = length; }
    double getLength() const{ return this->length; }
    void setAmount(double amount) { this->amount = amount; }
    double getAmount() const{ return this->amount; }
    void setStatus(QString status) { this->status = status; }
    QString getStatus() const{ return this->status; }
    void setNote(QString note) { this->note = note; }
    QString getNote() const{ return this->note; }
    QString getLID(){ return this->LID;}
    void setLID(QString lid){ LID = lid; }

    QString toString(){
        return date + " " + time + " " + QString::number(length) + " " + QString::number(amount) + " " + status + " |" + note;
    }

    static const QString KEY_CID;
    static const QString KEY_DATE;
    static const QString KEY_LENGTH;
    static const QString KEY_AMT;
    static const QString KEY_NOTE;
    static const  QString KEY_STATUS;

    const QString &getTime() const{ return time;}
    void setTime(const QString &newTime){ time = newTime;}

    static bool sortByDate_asc(const Entry lhs, const Entry rhs){
        //convert to dates
        QString date = lhs.getDate();
        QString time = lhs.getTime();
        QDateTime d1 = QDateTime::fromString(date + " " + time, "MM-dd-yyyy hh:mma");
        QDateTime d2 = QDateTime::fromString(rhs.getDate() + " " + rhs.getTime(), "MM-dd-yyyy hh:mma");
        return d1 < d2;
    }
    static bool sortByDate_dec(const Entry lhs, const Entry rhs){
        //convert to dates
       // QDateTime d1 = QDateTime::fromString(lhs.getDate() + " " + lhs.getTime(), "MM-dd-yyyy hh:mma");
       // QDateTime d2 = QDateTime::fromString(rhs.getDate() + " " + rhs.getTime(), "MM-dd-yyyy hh:mma");
        return !sortByDate_asc(lhs,rhs);
    }
};



#endif // ENTRY_H
