#ifndef FIREBASEHANDLER_H
#define FIREBASEHANDLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <functional>
#include <QListWidget>
#include "Client.h"

typedef function<void(vector<Client>,QString)> load_funct_ptr ;
typedef function<void()> addClient_funct_ptr ;



class FirebaseHandler : public QObject
{
    Q_OBJECT
public:
    explicit FirebaseHandler( QObject *parent = nullptr);
    ~FirebaseHandler();
    void addEntry(QString UID, double length, double amt, QString date, QString time, QString note,function<void(Entry e)>);

    void updateEntry(Entry * e);
    void updateEntryAttribute(QString CID, QString EID, QVariantMap m);
    void deleteEntry(Entry * e, function<void(Entry * e, int)> onComplete, int clickedIdx);

    void updateClient(Client * c);
    void updateClientAttribute(QString CID, QVariantMap);
    void deleteClient(QString UID, function<void(void)> onComplete);

    void getClientData(QString selected = "");
    void addClient(QString name,QString phone, QString email, QString paytype, QString payInfo, double amtOwed, double hourly);
    bool hasLoaded();
    void showClients(QListWidget * w);
    void signUpUser(QString email,QString password);
    void signInUser(QString email,QString password, load_funct_ptr loadFunc);

signals:

public slots:
   void getRequestResult();



private:
    QNetworkAccessManager * networkManager;
    QNetworkReply * networkReply;
    vector<Client> dbClients;
    bool isLoaded;
    load_funct_ptr loadFunc;
    QString loggedIn_UID;
    QString idToken;

    void requestPost(QString url, QVariantMap data, function<void(QNetworkReply*)> onComplete);
    void requestUpdate(QString url, QVariantMap data, function<void(QNetworkReply*)> onComplete);
    void requestGet(QString url, function<void(QNetworkReply*)> onComplete);
    void requestDelete(QString url, function<void(QNetworkReply*)> onComplete);
    void showErrorBox(QString msg);
};

#endif // FIREBASEHANDLER_H
