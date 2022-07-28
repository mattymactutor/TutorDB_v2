#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>
#include "Entry.h"
#include <QJsonObject>
#include <QJsonArray>
using namespace std;



class Client{
private:
    QString name;
    QString email;
    QString payinfo;
    QString paytype;
    QString phone;
    QString UID;
    double rate;
    double amtOwed;
    vector<Entry> logs;
public:
    Client(QString name, QString email, QString payinfo, QString paytype, QString phone, double rate, double amtOwed){
        this->name = name;
        this->email = email;
        this->payinfo = payinfo;
        this->paytype = paytype;
        this->phone = phone;
        this->rate = rate;
        this->amtOwed = amtOwed;
    }
    Client(QJsonObject in){
        this->name = in.value(KEY_NAME).toString();
        this->email = in.value(KEY_EMAIL).toString();
        this->payinfo = in.value(KEY_PAYINFO).toString();
        this->paytype = in.value(KEY_PAYTYPE).toString();
        this->phone = in.value(KEY_PHONE).toString();
        this->rate = in.value(KEY_RATE).toDouble();
        this->amtOwed = in.value(KEY_AMT_OWED).toDouble();
        QJsonArray entries = in.value(KEY_LOGS).toArray();
    }
    void setName(QString name) { this->name = name; }
    QString getName() const { return this->name; }
    void setEmail(QString email) { this->email = email; }
    QString getEmail() { return this->email; }
    void setPayinfo(QString payinfo) { this->payinfo = payinfo; }
    QString getPayinfo() { return this->payinfo; }
    void setPaytype(QString paytype) { this->paytype = paytype; }
    QString getPaytype() { return this->paytype; }
    void setPhone(QString phone) { this->phone = phone; }
    QString getPhone() { return this->phone; }
    void setRate(double rate) { this->rate = rate; }
    double getRate() { return this->rate; }
    void setAmtOwed(double amtOwed) { this->amtOwed = amtOwed; }
    double getAmtOwed() { return this->amtOwed; }

    Entry* getLog(int idx){
        return &(logs[idx]);
    }

    vector<Entry>& getLogs(){
        return logs;
    }
    void addLogs(vector<Entry> logs){
        this->logs = logs;
    }

    void addLog(Entry e){
        this->logs.push_back(e);
        //sort by date TODO
    }

    void setUID(QString UID) {this->UID = UID;}
    QString getUID() {return this->UID;}

    //string getLogs() { return this->logs; }
    static const QString KEY_NAME;
    static const QString KEY_PHONE;
    static const QString KEY_PAYTYPE;
    static const QString KEY_PAYINFO;
    static const QString KEY_EMAIL;
    static const QString KEY_AMT_OWED;
    static const QString KEY_RATE;
    static const QString KEY_LOGS;

    static bool sortByName(const Client lhs, const Client rhs){
        return lhs.getName() < rhs.getName();
    }

};

#endif // CLIENT_H



